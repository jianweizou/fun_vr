/* USBH */
#include "USBH.h"

#include "mlme.h"
#include "rtmp.h"

/* LWIP */
#include "netif/etharp.h"
#include "lwip/dhcp.h"
/* Default Setting */
#include "asic_ctrl.h"
#include "nl80211.h"
#include "tx.h"
#include "rx.h"
#include "usb_io.h"
#include "wpa/cmm_wpa.h"
#include "rtmp_init.h"
#include "rtmp_ctrl.h"
#include "cipher.h"
#include "wifi_api.h"
#include "ra_ctrl.h"

static BOOLEAN udhcpc_run = FALSE;
extern struct netif EMAC_if;
UCHAR WIFIInitialDone = FALSE;
extern UCHAR CXBusy;
extern UCHAR EPOUTBusy, EPINBusy;

/* TASK priority 			*/
/* DRV_WIFI_MAIN 	: 108	*/
/* DRV_WIFI_MESSAGE : 75	*/
/* DRV_WIFI_EAPOL	: 75	*/
/* DRV_WIFI_WPA		: 95	*/
/* DRV_WIFI_SCAN	: 52	*/
/* ======================== */

/* SCAN Channel sequence list definition */
static UCHAR ScanSequence[13] = {1, 4, 6, 11, 13,
								  3, 5, 9, 12,
								  2, 7, 8, 10};

static __align(32) UCHAR RateSwitchTable11BG[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0a, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x10,  2, 20, 35,
    0x05, 0x10,  3, 16, 35,
    0x06, 0x10,  4, 10, 25,
    0x07, 0x10,  5, 16, 25,
    0x08, 0x10,  6, 10, 25,
    0x09, 0x10,  7, 10, 13,
};

/* extern from USB */
USBH_Device_Structure *DEV;
extern SemaphoreHandle_t USBH_SEM_WAKEUP_WIFI;
extern SemaphoreHandle_t USBH_SEM_WAKEUP_AUTO_BKIN_2;
extern SemaphoreHandle_t USBH_SEM_AUTO_BKIN_CNT;
extern uint32_t wifi_init(uint8_t ID);
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
extern void usbh_auto_bkin_init(void);
extern void usbh_auto_bkin_uninit(void);
#endif
/* WIFI System SEMAPHORE definition */
xSemaphoreHandle xMACSendMutex			= NULL;

/* WIFI System TASK definition */
xTaskHandle xWiFiTask 					= NULL;
xTaskHandle xWiFiMessageTask_AP 		= NULL;			
xTaskHandle xWiFiMessageTask_DEV 		= NULL;			
xTaskHandle xEapol_supplicantTask		= NULL;
xTaskHandle xWiFiScanTask				= NULL;
xTaskHandle xWiFiEdccaTask				= NULL;
#ifdef	AUTO_DEL_MGMT_NODE
xTaskHandle xWiFiMgmtTask				= NULL;
#endif

/* WIFI System MESSAGE Q definition */
xQueueHandle xWifiEventQueue 			= NULL;
xQueueHandle xWifiEventQueue_DEV 		= NULL;
xQueueHandle xWifiEapolQueue			= NULL;
xQueueHandle xEzsetupEventQueue			= NULL;



RTMP_ADAPTER rtmpadapter;

PRTMP_ADAPTER ppAd;
/* */

VOID WpaEAPOLKeyAction(IN PRTMP_ADAPTER pAd, IN PUCHAR pDest_Addr, IN PUCHAR pBuf, IN USHORT	buf_len);
void WiFi_Receive(unsigned char* pData, int len);

static VOID WiFiTask(PVOID id);
static VOID WiFiMessageTask_AP(PVOID id);
static VOID WiFiMessageTask_DEV(PVOID id);
VOID WiFiScanTask(PVOID id);
#ifdef	AUTO_DEL_MGMT_NODE
static VOID WiFiMgmtTask(PVOID id);
#endif

static USHORT rx_htons(IN USHORT n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}


#define WIFI_MAIN_TASK_STACK		768//768//2048
#define WIFI_TASK_STACK				384//512
#define WIFI_SCAN_TASK_STACK		512//256
#define WIFI_EVENT_QUEUE			10//0
#define WIFI_DEVICE_ID				1
#define MLME_PERIODIC_EXEC_INTV		1000

wifi_event_rsp_hndler ext_hndl;


void WiFi_Task_Init(wifi_event_rsp_hndler hndler, unsigned char mode)
//void WiFi_Task_Init(void)
{
	xMACSendMutex			= xSemaphoreCreateMutex();
	xWiFiTask				= NULL;
	/* FIXME {*/
	xWiFiMessageTask_AP		= NULL;
	xWiFiMessageTask_DEV	= NULL;
	/* FIXME }*/
	xEapol_supplicantTask 	= NULL;
	xWiFiScanTask			= NULL;
#ifdef	AUTO_DEL_MGMT_NODE	
	xWiFiMgmtTask			= NULL;
#endif

	/* clear and reset RTMP structure */
	NdisZeroMemory(&rtmpadapter, sizeof(rtmpadapter));
	rtmpadapter.run_mode = (WIFI_RUN_MODE)mode;
	rtmpadapter.hndler = hndler;
	ext_hndl = hndler;

	/* assign to global pointer */
	ppAd = &rtmpadapter;	

	xTaskCreate(WiFiTask, "WiFiTask", WIFI_MAIN_TASK_STACK, (void *)&rtmpadapter, 6, &xWiFiTask);

}


void WiFi_Task_UnInit(void)
{
	DBGPRINT(RT_TRACE, ("[TRACE]%s: Started...\r\n", __FUNCTION__));
	udhcpc_run = 0;

	WIFIInitialDone = FALSE;
	if (ppAd)
	{
		BOOLEAN Cancelled;

		RTUSB_ClearBULKINBuffer(ppAd);

		ppAd->LinkState = LS_NO_LINK;

		RTMPNICTxReleaseMem(ppAd);
		MGMTENTRY_ReleaseAll(ppAd);

		DeleteScanList(ppAd);

		IF_DEV_CONFIG_OPMODE_ON_AP(ppAd) 
		{
			WpaApStop(ppAd);
		}

		RTMPReleaseTimer(&ppAd->PeriodicTimer, &Cancelled);

		ppAd = NULL;
	}

	CXBusy = 0;
	EPOUTBusy = 0;
	EPINBusy = 0;

	/* Semaphore Release */	
	if (xMACSendMutex)
	{
		vSemaphoreDelete(xMACSendMutex);
		xMACSendMutex = NULL;
	}

	/* Queue Release */
	if (xWifiEventQueue)
	{
		vQueueUnregisterQueue(xWifiEventQueue);
		vQueueDelete(xWifiEventQueue);	
		xWifiEventQueue = NULL;
	}

	if (xWifiEventQueue_DEV)
	{
		vQueueUnregisterQueue(xWifiEventQueue_DEV);
		vQueueDelete(xWifiEventQueue_DEV);
		xWifiEventQueue_DEV = NULL;
	}

	if (xWifiEapolQueue)
	{
		vQueueUnregisterQueue(xWifiEapolQueue);
		vQueueDelete(xWifiEapolQueue);
		xWifiEapolQueue = NULL;
	}

	/*	Task Release */
	if (xWiFiMessageTask_AP)
	{
		vTaskDelete(xWiFiMessageTask_AP);
		xWiFiMessageTask_AP = NULL;
	}

	if (xWiFiMessageTask_DEV)
	{
		vTaskDelete(xWiFiMessageTask_DEV);
		xWiFiMessageTask_DEV = NULL;
	}

	if (xEapol_supplicantTask)
	{
		vTaskDelete(xEapol_supplicantTask);
		xEapol_supplicantTask = NULL;
	}

	if (xWiFiScanTask)
	{
		vTaskDelete(xWiFiScanTask);
		xWiFiScanTask = NULL;
	}

	if (xWiFiEdccaTask)
	{
		vTaskDelete(xWiFiEdccaTask);
		xWiFiEdccaTask = NULL;
	}

#ifdef	AUTO_DEL_MGMT_NODE
	if (xWiFiMgmtTask)
	{
		vTaskDelete(xWiFiMgmtTask);
		xWiFiMgmtTask = NULL;
	}
#endif	

	if (xWiFiTask)
	{
		vTaskDelete(xWiFiTask);
		xWiFiTask = NULL;
	}

	DBGPRINT(RT_TRACE, ("[TRACE]%s: Ended...\r\n", __FUNCTION__));	
}



static VOID WiFiTask(PVOID id)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)id;
	//struct netif *netif = NULL;

	DBGPRINT(RT_INFO, ("[INFO]%s: WiFi Task Start...\r\n", __FUNCTION__));

	/* It can only do one-time SemaphoreTask. */
	if (DEV == NULL)
	{
		DBGPRINT(RT_TRACE, ("[TRACE]%s: SEM_WAKEUP_WIFI...\r\n", __FUNCTION__));
		xSemaphoreTake(USBH_SEM_WAKEUP_WIFI_2,  portMAX_DELAY);
	}
	else DBGPRINT(RT_INFO, ("[INFO]%s: EHCI Ready...\r\n", __FUNCTION__));

	if (DEV == NULL)
	{
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
		DEV = (USBH_Device_Structure*)usbh_wifi_init(WIFI_DEVICE_ID);
		usbh_auto_bkin_init();	
#endif
	}

	if (MLME_AdapterInit(pAd) == NDIS_FAILURE)
		DBGPRINT(RT_ERROR, ("[ERROR]%s: Wifi Initail FAIL..\r\n", __FUNCTION__));
	else DBGPRINT(RT_INFO, ("[INFO]%s: Wifi Initail OK..\r\n", __FUNCTION__));

	/* Initialize security variable per entry, 
		1. 	pairwise key table, re-set all WCID entry as NO-security mode.
		2.	access control port status
	*/
	int i;
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		AsicRemovePairwiseKeyEntry(pAd, (UCHAR)i);
	}

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_INFO, ("[INFO]%s: WiFi AP Mode Initial Started.....\r\n", __FUNCTION__));

		xWifiEventQueue = xQueueCreate(WIFI_EVENT_QUEUE, sizeof(AP_Message));
		
		vQueueAddToRegistry(xWifiEventQueue, "WifiEvnt_AP");
		
		xTaskCreate(WiFiMessageTask_AP, "WiFiMessager_AP", WIFI_TASK_STACK, (PVOID)pAd, 3, &xWiFiMessageTask_AP);

#ifdef	AUTO_DEL_MGMT_NODE
		/* Creat Wi-Fi Mgmt Task */
		xTaskCreate(WiFiMgmtTask, "WiFiMgmtnTask_AP", WIFI_SCAN_TASK_STACK, (PVOID)pAd, 1, &xWiFiMgmtTask);
#endif
	}
	else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		DBGPRINT(RT_INFO, ("[INFO]%s: WiFi Station Mode Initial Started.....\r\n", __FUNCTION__));

		xWifiEventQueue_DEV = xQueueCreate(WIFI_EVENT_QUEUE, sizeof(xWifiStackEvent_t));
		
		vQueueAddToRegistry(xWifiEventQueue_DEV, "WifiEvnt_DEV");
		
		xTaskCreate(WiFiMessageTask_DEV, "WiFiMessager_DEV", WIFI_TASK_STACK, (PVOID)pAd, 3, &xWiFiMessageTask_DEV);
	}
	else
	{
		DBGPRINT(RT_INFO, ("[INFO]%s: WiFi Proprietary Mode nitial Started.....\r\n", __FUNCTION__));

	}
	WIFIInitialDone = TRUE;

	xSemaphoreGive(USBH_SEM_WAKEUP_AUTO_BKIN_2);

	DBGPRINT(RT_TRACE, ("[TRACE]%s: DeAuth Packet Send Broadcast..\n\r\n", __FUNCTION__));

	NdisFillMemory(pAd->MgmAddr, MAC_ADDR_LEN, 0xFF);
	for (i = 0; i < 5; i++)
	{	
		SendDeAuthReq(pAd, pAd->MgmAddr, &pAd->TxContext, REASON_RESERVED);
		vTaskDelay(100 / portTICK_RATE_MS);	
	}

	while (1)
	{
		xSemaphoreTake(USBH_SEM_AUTO_BKIN_CNT, portMAX_DELAY);
		RTUSB_IRQHandler(pAd);
	}
}


static VOID WiFiMessageTask_AP(PVOID id)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)id;
	AP_Message mMessage;

	DBGPRINT(RT_INFO, ("[INFO]%s: AP Message Start..\r\n", __FUNCTION__));	

	for (;;)
	{
		NdisZeroMemory(&mMessage, sizeof(AP_Message));

		if (xQueueReceive(xWifiEventQueue, (PVOID)&mMessage, portMAX_DELAY) == pdPASS)
		{
			if (pAd->APPowerSave)
				RTMPSetPowerSuspend(pAd, 0);

			switch (mMessage.message)
			{
				case SEND_PROBE_RESPOND:
				default:
					DBGPRINT(RT_LOUD, ("[TRACE]ProbeRsp to %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(mMessage.MgmAdd)));
					SendIbssProbeRsp(pAd, mMessage.MgmAdd, &pAd->TxContext);
					break;

				case SEND_AUTH_RESPOND:
					DBGPRINT(RT_INFO, ("[INFO]Authenticate to %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(mMessage.MgmAdd)));
					SendAuthPacket(pAd, mMessage.MgmAdd, &pAd->TxContext, 2, mMessage.reserved);
					break;

				case SEND_ASSOCIATION_RESPOND:
					{
						BOOLEAN	ret;
						PMGMTENTRY pEntry = NULL;
						DBGPRINT(RT_INFO, ("[INFO]Association to %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(mMessage.MgmAdd)));

						pEntry = MGMTENTRY_GetNode(pAd, mMessage.MgmAdd);
						if (pEntry == NULL) {
							DBGPRINT(RT_ERROR, ("[ERR]%s: Cannot find registered node %02X-%02X-%02X-%02X-%02X-%02X\r\n", 
												__FUNCTION__, PRINT_MAC(mMessage.MgmAdd)));
						}
						else {
							ret = SendAssociateRsp(pAd, pEntry, &pAd->TxContext, mMessage.reserved);
							if (ret == TRUE && mMessage.reserved == MLME_SUCCESS) {
								if (pAd->ApCfg.GroupKeyWepStatus >= Ndis802_11Encryption2Enabled)
								{
									/* Startup WPA handshake */
									WpaEAPOLStartAction(pAd, pEntry);
								}
								else
								{	
									if (pAd->ApCfg.EncMode == Ndis802_11Cipher_WEP40)
									{
										/* Just register WEP*/
										RTMPAddWEPPairwise(pAd, pEntry->Aid);
									}
								}
							}
						}
					}
					break;

				case SET_SCAN_ALL_AP:
					pAd->scanIndex = 0;
					WiFi_QueryAndSet(SET_BEACON_OFF, 0, 0);
					MLME_SetScanFlag(pAd, 1);
					break;

				case SEND_EAPOL_MESSAGE:
					{
						if (mMessage.buf_len == 0 || mMessage.pbuf == NULL) {
							DBGPRINT(RT_ERROR, ("[ERR]%s: Eapol Message is invaild\r\n", __FUNCTION__));	
						}
						else {
							WpaEAPOLKeyAction(pAd, mMessage.MgmAdd, mMessage.pbuf, mMessage.buf_len);
						}
					}
					break;
			}			
		}
		/* release memory */
		vPortFree(mMessage.pbuf);
	}
}

static VOID WiFiMessageTask_DEV(PVOID id)
{
	xWifiStackEvent_t xReceivedEvent;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)id;

	DBGPRINT(RT_TRACE, ("[TRACE]%s: DEVICE Message Start..\r\n", __FUNCTION__));

	for (;;)
	{
		NdisZeroMemory(&xReceivedEvent, sizeof(xReceivedEvent));

		if (xQueueReceive(xWifiEventQueue_DEV, ( void * ) &xReceivedEvent, portMAX_DELAY) == pdPASS)
		{

			switch ( xReceivedEvent.eEventType )
			{
				case eWifitest:
					DBGPRINT(RT_TRACE, ("[TRACE]WiFi Message: eEzsetupWifitest\r\n"));
					break;
					
				case eWifiInit:
					DBGPRINT(RT_TRACE, ("[TRACE]WiFi Message: eeEzsetupWifiInit\r\n"));
					break;
					
				case eWifiConnect:
					DBGPRINT(RT_TRACE, ("[TRACE]WiFi Message: eeEzsetupWifiConnect\r\n"));
					if (xWiFiScanTask)
					{
						vTaskDelete(xWiFiScanTask);
						xWiFiScanTask = NULL;
					}

					if(pAd->StaCfg.WepStatus  ==  Ndis802_11WEPEnabled)
					{

						pAd->WEPEnabled = TRUE;
						RTMPSetChipHWKey(pAd, pAd->StaCfg.WpaPassPhrase,  pAd->StaCfg.WpaPassPhraseLen, Ndis802_11Encryption1Enabled);
						pAd->LinkState = LS_SCAN_BSSID;

					}
					else if (pAd->StaCfg.WepStatus  ==  Ndis802_11WEPDisabled)
					{
						pAd->LinkState = LS_SCAN_BSSID;
						pAd->WEPEnabled = FALSE;
					}
					else
					{
						/*RT_CfgSetWPAPSKKey(pAd,
								(PSTRING)pAd->StaCfg.WpaPassPhrase, 
								pAd->StaCfg.WpaPassPhraseLen,
								(PUCHAR)pAd->Ssid, 
								pAd->SsidLen, 
								pAd->StaCfg.PMK);*/
		
						pAd->LinkState = LS_SCAN;
						pAd->WEPEnabled = TRUE;
					}
				break;
					
				case eWifiDisconnect:
					DBGPRINT(RT_TRACE, ("[TRACE]WiFi Message: eEzsetupWifiDisconnect\r\n"));
					/* FIXME */
					if (pAd->LinkState == LS_LINKED) {
						SendDeAuthReq(pAd, pAd->StaCfg.RouterMac, &pAd->TxContext, REASON_DEAUTH_STA_LEAVING);
						MGMTENTRY_Del(pAd, pAd->StaCfg.RouterMac);
					}
					
					MLME_DoDisconnect(pAd, FALSE);

					break;
					
				case eWifiEAPOL:
					{
						if (xReceivedEvent.epol_msg.buf_len == 0 || xReceivedEvent.epol_msg.pbuf == NULL) {
							DBGPRINT(RT_ERROR, ("[ERR]%s: Eapol Message is invaild\r\n", __FUNCTION__));	
						}
						else {
							WpaEAPOLKeyAction(pAd, xReceivedEvent.epol_msg.dst_addr, xReceivedEvent.epol_msg.pbuf, xReceivedEvent.epol_msg.buf_len);
						}
					}
					break;
					
				default :
					DBGPRINT(RT_ERROR, ("[ERR]WiFi Message: Unknown message (%d)\r\n", xReceivedEvent.eEventType));
					/* Should not get here. */
					break;
			}
		}
		/* release memory */
		vPortFree(xReceivedEvent.epol_msg.pbuf);
	}
}

VOID WiFiScanTask(
	IN PVOID id)
{
	PRTMP_ADAPTER pAd 	= (PRTMP_ADAPTER)id;
//	int i				= 0;
	UCHAR	index 		= 0;
#ifdef DEBUG	
	PAPScanResultElement	pAPEntry = NULL;
#endif

	/* If app register hander, then response the status to app. */
	if (pAd->hndler)
	{
		(pAd->hndler)(eWifiInitDone);
	}	

	while (1)
	{
		if (MLME_GetScanFlag(pAd))
		{
			vTaskDelay(DEFAULT_SCAN_PERIOD/ portTICK_RATE_MS);	// Waiting for receive packets complete in current channel.

			pAd->scanChannel = ScanSequence[pAd->scanIndex];
			DBGPRINT(RT_TRACE, ("[TRACE]%s(%d): Start Scan channel :%d\r\n", __FUNCTION__, pAd->scanIndex, pAd->scanChannel));
			if (pAd->scanChannel != pAd->channel)				// Don't switch to the same channel again.
			MT7601_ChipSwitchChannel(pAd, pAd->scanChannel, 0);
			pAd->scanIndex = (pAd->scanIndex + 1) % sizeof(ScanSequence);

			if (pAd->scanIndex == 0)
			{
				vTaskDelay(DEFAULT_SCAN_PERIOD/ portTICK_RATE_MS);

				MLME_SetScanFlag(pAd, 0);
				MT7601_ChipSwitchChannel(pAd, pAd->channel, 0);
				WiFi_QueryAndSet(SET_BEACON_ON, 0, 0);
#ifdef DEBUG
				pAPEntry = pAd->FirstElem;
				if (pAPEntry != NULL)
				{
					for (i = 0; i < pAd->nCollectNbr; i++)
					{		
						DBGPRINT(RT_INFO, ("(%d): Len=%d %s, ch=%d auth=%d enc=%d %s\r\n",						
														i,
														pAPEntry->SsIdLen,
														pAPEntry->SsId,
														pAPEntry->channel,
														pAPEntry->auth_mode,
														pAPEntry->uenc_mode,
														pAPEntry->auth));
						pAPEntry = pAPEntry->NextElem;	
					}
				}
#endif
				if (pAd->hndler)
				{
					(pAd->hndler)(eWifiScanDone);
				}
			}
		}
		else
		{
			if (pAd->APPowerSave)
			{
				if (pAd->PSPeriod != 0)
				{
					if (index % 2 == 0)
						RTMPSetPowerSuspend(pAd, 1);
					else
						RTMPSetPowerSuspend(pAd, 0);
					
					index++;
					vTaskDelay(pAd->PSPeriod / portTICK_RATE_MS);
				}
				else
				{
					RTMPSetPowerSuspend(pAd, 1);
					vTaskDelay(1000 / portTICK_RATE_MS);
				}
			}
			else
			{
#ifdef DEBUG
				UCHAR val=0;
				WiFi_QueryAndSet(QID_GET_TX_STATUS, &val, 0);
				if(val && val != 0xFF)
					DBGPRINT(RT_INFO, ("[INFO]Packet Lost=%x....\r\n", val));
#endif
				vTaskDelay(2000 / portTICK_RATE_MS);
			}
		}
	}
}



VOID WiFiEdccaTask(
	IN PVOID 	id)
{
	PRTMP_ADAPTER pAd 	= (PRTMP_ADAPTER)id;
	while(1)
	{
		vTaskDelay(pAd->ed_chk_period / portTICK_RATE_MS);
		RTMP_EdccaStatusRead(pAd);
	}	
}

#ifdef	AUTO_DEL_MGMT_NODE
static VOID WiFiMgmtTask(PVOID id)
{
	PRTMP_ADAPTER 	pAd		= (PRTMP_ADAPTER)id;
	PMGMTENTRY		pEntry	= NULL;
	ULONG			curr_time;

	DBGPRINT(RT_TRACE, ("[TRACE]%s OK ..\r\n", __FUNCTION__));	

	while(1)
	{
		pEntry = pAd->pFirstLinkNode;
		while (pEntry != NULL) 
		{
			curr_time = (ULONG)xTaskGetTickCount();
			if (curr_time > pEntry->rx_time)
			{
				if (curr_time - pEntry->rx_time > RX_TIME_INTERVAL_THRESHOLD)
				{
					DBGPRINT(RT_INFO, ("[INFO]%s: Timeout, Disconnect: %02X-%02X-%02X-%02X-%02X-%02X\r\n", __FUNCTION__, PRINT_MAC(pEntry->Addr)));
					MGMTENTRY_Del(pAd, pEntry->Addr);
					break;
				}
			}
			DBGPRINT(RT_LOUD, ("[LOUD]%d: %lu\r\n", pEntry->Aid, (curr_time - pEntry->rx_time)*portTICK_RATE_MS));
			pEntry = pEntry->next_node;
		}
		
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
#endif

VOID MLME_StartAPConnect(
	IN PRTMP_ADAPTER 	pAd)
{
	DBGPRINT(RT_TRACE, ("[TRACE]WiFi Message: eEzsetupWifiSetAPInfo\r\n"));
	if (pAd->LinkState == LS_LINKED) {
		SendDeAuthReq(pAd, pAd->StaCfg.RouterMac, &pAd->TxContext, REASON_DEAUTH_STA_LEAVING);
		MGMTENTRY_Del(pAd, pAd->StaCfg.RouterMac);
	}
	
	MLME_DoDisconnect(pAd, FALSE);
	
	DBGPRINT(RT_TRACE, ("[TRACE]%s: User Set AP information: SSID(%s); PASSWORD(%s)\r\n",
						__FUNCTION__, pAd->Ssid, pAd->StaCfg.WpaPassPhrase));

	if (xWiFiScanTask)
	{
		vTaskDelete(xWiFiScanTask);
		xWiFiScanTask = NULL;
	}
	xTaskCreate(WiFiScanTask, "WiFiScanTask_AP", WIFI_SCAN_TASK_STACK, (PVOID)pAd, 2, &xWiFiScanTask);
	/* setup to detetct target AP */
	pAd->smartSetup = SmartConfigCheckBeacon;
	MLME_SetScanFlag(pAd, 1);	
}

VOID MLME_SelectTxRateTable(
	IN PRTMP_ADAPTER 	pAd,
	IN PMGMTENTRY 		pEntry,
	IN PUCHAR 			*ppTable,
	IN PUCHAR 			pTableSize,
	IN PUCHAR 			pInitTxRateIdx)
{
	*ppTable = RateSwitchTable11BG;
	*pTableSize = RATE_TABLE_SIZE(*ppTable);
	*pInitTxRateIdx = RATE_TABLE_INIT_INDEX(*ppTable);
}

VOID MLME_PeriodicExec(IN PVOID FunctionContext)
{
    PRTMP_ADAPTER   pAd = (PRTMP_ADAPTER)pvTimerGetTimerID(FunctionContext);

    MLME_DynamicTxRateSwitching(pAd);
}

VOID MLME_ProcessDataFrame(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN UINT 			len)
{
	struct eth_hdr 	*ethhdr;

	ethhdr =(struct eth_hdr *) pData;

	switch (rx_htons(ethhdr->type)) {
		/* IP or ARP packet? */
		case ETHTYPE_IP:
		case ETHTYPE_ARP:
		case ETHTYPE_VLAN:
			/* PPPoE packet? */
		case ETHTYPE_PPPOEDISC:
		case ETHTYPE_PPPOE:
			WiFi_Receive(pData,len);
			break;

		case ETHTYPE_EAPOL:
			DBGPRINT(RT_LOUD, ("[LOUD]%s: Receive EAPOL Packet\r\n", __FUNCTION__));
			{
				{
					// skip [Dest Addr - 6B][Src Addr - 6B][CMD - 2B]
					len -= 14;
					pData += 14;

					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						AP_Message mMessage;
						
						NdisZeroMemory(&mMessage, sizeof(mMessage));
						NdisMoveMemory(mMessage.MgmAdd, ethhdr->src.addr, 6);
						
						mMessage.buf_len = len;
						
						mMessage.pbuf = (PUCHAR)pvPortMalloc(len);
						if (mMessage.pbuf == NULL)
						{
					        DBGPRINT(RT_ERROR, ("[ERR]%s!!! : no memory!!!\r\n", __FUNCTION__));
					        return;
					    }

						NdisMoveMemory(mMessage.pbuf, pData, len);
						mMessage.message = SEND_EAPOL_MESSAGE;
						xQueueSend(xWifiEventQueue, (void *)&mMessage, (portTickType)0);	
					}	
					else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
					{
						xWifiStackEvent_t xReceivedEvent;
						
						NdisZeroMemory(&xReceivedEvent.epol_msg, sizeof(xReceivedEvent.epol_msg));
						NdisMoveMemory(xReceivedEvent.epol_msg.dst_addr, ethhdr->src.addr, 6);
						
						xReceivedEvent.epol_msg.buf_len = len;
						
						xReceivedEvent.epol_msg.pbuf = (PUCHAR)pvPortMalloc(len);
						if (xReceivedEvent.epol_msg.pbuf == NULL)
						{
					        DBGPRINT(RT_ERROR, ("[ERR]%s!!! : no memory!!!\r\n", __FUNCTION__));
					        return;
					    }

						NdisMoveMemory(xReceivedEvent.epol_msg.pbuf, pData, len);
						xReceivedEvent.eEventType = eWifiEAPOL;
						xQueueSend(xWifiEventQueue_DEV, (void *)&xReceivedEvent, (portTickType)0);	
					}
					else
						DBGPRINT(RT_ERROR, ("[ERR]%s: unknown opmode.\r\n", __FUNCTION__));
				}
			}
			break;
				
		default:
			break;
	}//DummyS
}

VOID MLME_DoDisconnect(
	IN PRTMP_ADAPTER 	pAd, 
	IN BOOLEAN 			bReconnect)
{
	RTMPSetLinkFliter(pAd, FALSE);

	if (bReconnect == TRUE)
	{
		/* reconnect */
		pAd->LinkState = LS_SCAN;
		MLME_SetScanFlag(pAd, 1);
	}
	else
		pAd->LinkState = LS_NO_LINK;
	
	pAd->bss_table.BssNr = 0;
	NdisZeroMemory(pAd->bss_table.BssEntry[0].Bssid, 6);
}


VOID MLME_DoConnect(	
	IN PRTMP_ADAPTER  	pAd)
{
	MLME_SetScanFlag(pAd, 0);
	RTMPSetLinkFliter(pAd, TRUE);
	pAd->LinkState = LS_START_LINK;
}

VOID MLME_Startdhcpc(
	IN PRTMP_ADAPTER  	pAd)
{	
	int ret = 0;

	RTMPSetLinkFliter(pAd, TRUE);

	//DBGPRINT(RT_INFO, ("[INFO]%s: Start DHCP....", __FUNCTION__));
	if (udhcpc_run == FALSE)
	{
		ret = dhcp_start(&EMAC_if);
		DBGPRINT(RT_INFO, ("[INFO]Start DHCP....ret = %d\r\n", ret));
		udhcpc_run = TRUE;
	}
	else
	{
		ret = dhcp_renew(&EMAC_if);
		DBGPRINT(RT_INFO, ("[INFO]Renew DHCP....ret = %d\r\n", ret));
	}
}


VOID MLME_SetScanFlag(
	IN	PRTMP_ADAPTER	pAd,
	IN 	UINT8 			Flag)
{
	pAd->scanFlag = Flag;
}


UCHAR MLME_GetScanFlag(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR ubScanFlag;
	ubScanFlag = pAd->scanFlag;
	return ubScanFlag;
}


VOID MLME_SetRxBcnCount(
	IN PRTMP_ADAPTER 	pAd, 
	IN ULONG 			Cnt)
{
	pAd->rxBeaconCount = (Cnt == 0) ? (0) : (pAd->rxBeaconCount + 1);
}


ULONG MLME_GetRxBcnCount(
	IN PRTMP_ADAPTER 	pAd)
{
	ULONG ulCnt = 0;
	ulCnt = pAd->rxBeaconCount;
	return ulCnt;
}	


BOOLEAN MLME_SetDefaultAp(
	IN PRTMP_ADAPTER 	pAd)
{
//	CHAR ssid_prefix[MAX_LEN_OF_SSID] = "";
//	UINT mac_num = 0;
//	CHAR ssid_mac[2*MAC_ADDR_LEN] = "";
//	UINT i;

	if (pAd->SsidLen == 0)
	{
		NdisZeroMemory(pAd->Ssid, MAX_LEN_OF_SSID);

			pAd->SsidLen = snprintf((char *)pAd->Ssid, MAX_LEN_OF_SSID, "%s_%02X%02X%02X",
														SNX_AP_TITLE,	
														pAd->CurrentAddress[3], 
														pAd->CurrentAddress[4],
														pAd->CurrentAddress[5]);
		DBGPRINT(RT_INFO, ("Default AP ssid(%s), len(%d)\r\n", pAd->Ssid, pAd->SsidLen));
	}

	if (pAd->ApCfg.WpaPassPhraseLen == 0)
	{
		NdisZeroMemory(pAd->ApCfg.WpaPassPhrase, 64);

		if ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) || (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
		{
			pAd->ApCfg.WpaPassPhraseLen = 8;
			NdisMoveMemory(pAd->ApCfg.WpaPassPhrase, "12345678", pAd->ApCfg.WpaPassPhraseLen);
		}
		else if (pAd->ApCfg.EncMode == Ndis802_11Cipher_WEP40)
		{
			pAd->ApCfg.WpaPassPhraseLen = 5;
			NdisMoveMemory(pAd->ApCfg.WpaPassPhrase, "12345", pAd->ApCfg.WpaPassPhraseLen);
		}
		DBGPRINT(RT_INFO, ("Default AP key(%s), len(%d)\r\n", pAd->ApCfg.WpaPassPhrase, pAd->ApCfg.WpaPassPhraseLen));
	}

	return TRUE;
}

int u8_wc_toutf8(char *dest, ULONG ch)
{
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

NDIS_STATUS MLME_GetConfig(
	IN PRTMP_ADAPTER 	pAd)
{
	pAd->channel = DEFAULT_CHANNEL;
	pAd->beaconPeriod = DEFAULT_BCNPERIOD;
	
	if (pAd->run_mode == WIFI_RUN_MODE_AP)
		pAd->OpMode = OPMODE_AP;
	else
		pAd->OpMode = OPMODE_STA ;
	
	pAd->MmpsMode = 0;
	pAd->BeaconOffset[0] = 0xC000;
	pAd->Antenna.field.RxPath = 1;
	pAd->Antenna.field.TxPath = 1;
	pAd->Antenna.field.BoardType = BOARD_TYPE_USB_PEN;
	
	pAd->StaCfg.AdaptiveFreq = TRUE;
	pAd->StaCfg.DesiredTransmitSetting.field.MCS = 7;
	pAd->StaCfg.bAutoTxRateSwitch = FALSE;
	pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPANone;
	
	pAd->CommonCfg.CountryRegion = 7;
	pAd->CommonCfg.BBPCurrentBW = BW_20;
	pAd->CommonCfg.TxBASize = 7;
	pAd->CommonCfg.PhyMode = PHY_11BGN_MIXED;
	pAd->CommonCfg.TxStream = 1;
	pAd->CommonCfg.RxStream = 1;
	pAd->CommonCfg.Channel = pAd->channel;
	pAd->CommonCfg.bMIMOPSEnable = FALSE;
	pAd->CommonCfg.RegTransmitSetting.field.HTMODE =  HTMODE_MM;
	pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_NONE;
	pAd->CommonCfg.RegTransmitSetting.field.BW = pAd->CommonCfg.BBPCurrentBW;
	pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_NONE;
	pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	
	pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAd->CommonCfg.BBPCurrentBW;

	pAd->rxCorrectSeq = 0xFFFF;

	/* Sync with MT7601 AP linux version */
	pAd->chipCap.WcidHwRsvNum = 127;

	pAd->TxBW = 0;
	pAd->TxMCS = 7;
	pAd->TxGI = GI_800;
	pAd->SetTxPHYMode = MODE_OFDM;
	pAd->SetTxPHYRate = MCS_RATE_54;
	pAd->LinkState = LS_SCAN;
	MLME_SetScanFlag(pAd, 0);
	pAd->scanIndex = 0;

	/* EDCCA Default */
	pAd->ed_chk_period = 50;
	pAd->ed_tx_stoped = FALSE;
	pAd->mac_tx_stop = FALSE;
	pAd->ed_false_cca_threshold = 200;
	pAd->ed_threshold = 90;
	pAd->ed_block_tx_threshold = 1;

	MLME_GetOperConfig(pAd);

	return NDIS_SUCCESS;
}


VOID MLME_GetOperConfig(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT nvram_data = 0;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef NEED_STORE_STA_INFO
		MLME_ReadParamFromNVARAM(WIFI_DEV_CHANNEL_INFO, &nvram_data, 1);
		if (nvram_data == 0)
		{
			MLME_SetScanFlag(pAd, 0);
		}
		else
		{
			MLME_SetScanFlag(pAd, 0);
			pAd->channel = nvram_data;
		}

		/* Load Wifi configuration from NVRAM */
		MLME_ReadParamFromNVARAM(WIFI_DEV_SSID_INFO, (PVOID)pAd->Ssid, pAd->SsidLen);
		pAd->SsidLen = strlen((char *)pAd->Ssid);
		MLME_ReadParamFromNVARAM(WIFI_DEV_BSSID_INFO, (PVOID)pAd->essid, 6);
		MLME_ReadParamFromNVARAM(WIFI_DEV_AUTH_MODE, (PVOID)&pAd->StaCfg.AuthMode, 1);
		MLME_ReadParamFromNVARAM(WIFI_DEV_ENC_MODE,	(PVOID)&pAd->StaCfg.EncMode, 1);
		MLME_ReadParamFromNVARAM(WIFI_DEV_KEY_INFO, (PVOID)&pAd->StaCfg.WpaPassPhrase, pAd->StaCfg.WpaPassPhraseLen);
		pAd->StaCfg.WpaPassPhraseLen = strlen((char *)pAd->StaCfg.WpaPassPhrase);
		
		pAd->StaCfg.Networkmode = Ndis802_11Infrastructure;
		
		if (pAd->StaCfg.EncMode == Ndis802_11Cipher_NONE)
		{
			pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
		}
		else if ((pAd->StaCfg.EncMode == Ndis802_11Cipher_WEP40) || (pAd->StaCfg.EncMode == Ndis802_11Cipher_WEP104)) 
		{
			pAd->StaCfg.WepStatus = Ndis802_11WEPEnabled;
		}
		else if (pAd->StaCfg.EncMode == Ndis802_11Cipher_TKIP)
		{
			pAd->StaCfg.WepStatus = Ndis802_11Encryption2Enabled;
		}
		else if (pAd->StaCfg.EncMode == Ndis802_11Cipher_CCMP)
		{
			pAd->StaCfg.WepStatus = Ndis802_11Encryption3Enabled;
		}
		else {
			DBGPRINT(RT_ERROR, ("[ERR]%s: Invalid encryption type(%d)\r\n", __FUNCTION__, pAd->StaCfg.EncMode));
			pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
		}

		/* Group cipher will be updated during connection flow. If not, it should be saved for AES+TKIP case. */
		pAd->StaCfg.GroupCipher = pAd->StaCfg.WepStatus;
		pAd->StaCfg.PairCipher = pAd->StaCfg.WepStatus;
		pAd->StaCfg.bMixCipher = FALSE;

		DBGPRINT(RT_INFO, ("[INFO]%s: Router SSID %s\r\n", __FUNCTION__, pAd->Ssid));
		DBGPRINT(RT_INFO, ("[INFO]%s: Router Password %s\r\n", __FUNCTION__, pAd->StaCfg.WpaPassPhrase));
		DBGPRINT(RT_INFO, ("[INFO]%s: Router Auth %d\r\n", __FUNCTION__, pAd->StaCfg.AuthMode));
		DBGPRINT(RT_INFO, ("[INFO]%s: Router Enc %d\r\n", __FUNCTION__, pAd->StaCfg.EncMode));
		DBGPRINT(RT_INFO, ("[INFO]%s: Router channel %d\r\n", __FUNCTION__, pAd->channel));
#endif		
		pAd->LinkState = LS_NO_LINK;
	}
//#ifdef APP_AP     
	else IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{	
		pAd->channel = SetChannel;

		if (pAd->channel <= 0 || pAd->channel > 15)
		{
			pAd->channel = DEFAULT_CHANNEL;
		}

		//James add: set utf8 ssid
		int next = 0;
		int i = 0;
	
#if ADDMAC_TO_SSID
		{
			char tempSSID[15];
			snprintf((char *)tempSSID,15,"-%02x%02x%02x%02x%02x%02x\r\n",pAd->CurrentAddress[0],pAd->CurrentAddress[1],pAd->CurrentAddress[2],pAd->CurrentAddress[3],pAd->CurrentAddress[4],pAd->CurrentAddress[5]);
	
			for (i=0; i<((sizeof(SSID)-1)/2); i++)
			{
				if(SSID[i] == 0x3F) break; 
					DBGPRINT(RT_LOUD, ("\n n=%d,",next));
				next += u8_wc_toutf8(&pAd->Ssid[next], SSID[i]); 
			}
			
			pAd->SsidLen=next;
			
			NdisMoveMemory(&pAd->Ssid[pAd->SsidLen],tempSSID,13);
			pAd->SsidLen = pAd->SsidLen+13;
			pAd->Ssid[pAd->SsidLen] = '\0';
		}
		else
#endif			
		{
			for (i=0; i<((sizeof(WIFI_SSID)-1)/2); i++)
			{
				if(WIFI_SSID[i] == 0x3F) break; 
					DBGPRINT(RT_LOUD, ("\n n=%d,",next));
				next += u8_wc_toutf8((char *)&pAd->Ssid[next], WIFI_SSID[i]); 
			}
			
			pAd->SsidLen=next;	
		}

		NdisMoveMemory(pAd->ApCfg.WpaPassPhrase, WIFI_PASSWD, sizeof(WIFI_PASSWD));
		pAd->ApCfg.WpaPassPhraseLen = strlen((char *)pAd->ApCfg.WpaPassPhrase);

		if ((pAd->ApCfg.WpaPassPhraseLen >= 8) && (pAd->ApCfg.WpaPassPhraseLen <= 63))
			nvram_data = WPA2;
		else if ((pAd->ApCfg.WpaPassPhraseLen == 5) || (pAd->ApCfg.WpaPassPhraseLen == 13))
			nvram_data = WEP;
		else
			nvram_data = OPEN;

		if (nvram_data == WPA2)
		{
			pAd->ApCfg.AuthMode = Ndis802_11AuthModeWPA2PSK;
			if (pAd->ApCfg.WpaPassPhraseLen > 63 || pAd->ApCfg.WpaPassPhraseLen < 8)
				pAd->ApCfg.WpaPassPhraseLen = 0;
			pAd->ApCfg.EncMode = Ndis802_11Cipher_CCMP;
			pAd->ApCfg.WepStatus = Ndis802_11Encryption3Enabled;
			pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption3Enabled;
			pAd->ApCfg.WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
		}
		else if (nvram_data == WPA)
		{
			pAd->ApCfg.AuthMode = Ndis802_11AuthModeWPAPSK;
			if (pAd->ApCfg.WpaPassPhraseLen > 63 || pAd->ApCfg.WpaPassPhraseLen < 8)
				pAd->ApCfg.WpaPassPhraseLen = 0;
			pAd->ApCfg.EncMode = Ndis802_11Cipher_TKIP;
			pAd->ApCfg.WepStatus = Ndis802_11Encryption2Enabled;
			pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption2Enabled;
			pAd->ApCfg.WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
		}
		else if (nvram_data == WEP)
		{
			pAd->ApCfg.AuthMode = Ndis802_11AuthModeOpen;
			if (pAd->ApCfg.WpaPassPhraseLen != 5)
				pAd->ApCfg.WpaPassPhraseLen = 0;
			pAd->ApCfg.EncMode = Ndis802_11Cipher_WEP40;
			pAd->ApCfg.WepStatus = Ndis802_11Encryption1Enabled;
			pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption1Enabled;
			pAd->ApCfg.WpaMixPairCipher = MIX_CIPHER_NOTUSE;
		}
		else
		{
			/* OPEN */
			pAd->ApCfg.AuthMode = Ndis802_11AuthModeOpen;
			pAd->ApCfg.EncMode = Ndis802_11Cipher_NONE;
			pAd->ApCfg.WepStatus = Ndis802_11EncryptionDisabled;
			pAd->ApCfg.GroupKeyWepStatus = Ndis802_11EncryptionDisabled;
			pAd->ApCfg.WpaMixPairCipher = MIX_CIPHER_NOTUSE;
		}
	}
//#endif    
	else DBGPRINT(RT_ERROR, ("[ERR]:%s Configuration mode\r\n", __FUNCTION__));

	pAd->WEPEnabled = (pAd->ApCfg.EncMode == Ndis802_11Cipher_NONE)?(FALSE):(TRUE);
}



NTSTATUS MLME_InitHardwre(
	IN PRTMP_ADAPTER 	pAd)
{
	int 			index = 0;
	NTSTATUS		status;

	DBGPRINT(RT_TRACE, ("[TRACE]%s: WiFi Initial WiFi Hardware\r\n", __FUNCTION__));

	do
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &pAd->MACVersion );
	
		if ((pAd->MACVersion != 0x00) && (pAd->MACVersion != 0xFFFFFFFF))
			break;
		
		RTMPusecDelay(1);
	} while (index++ < 100);

	if (index == 100)
	{
	   DBGPRINT(RT_ERROR, ("[ERR]%s: MAC_CSR0  [ Ver:Rev=0x%08x].. Card not exist \r\r\n", 
	   						__FUNCTION__, pAd->MACVersion));
		goto _err1;
	}
	else DBGPRINT(RT_TRACE, ("[TRACE]%s: MAC_CSR0  [Ver:Rev=0x%08x]\r\n", 
							__FUNCTION__, pAd->MACVersion));

	status = RTUSB_LoadFirmwareToAndes(pAd);
	if (status != NDIS_SUCCESS)
	{
		DBGPRINT(RT_ERROR, ("[ERR]Load Firmware Fail[=0x%08x]\r\n", status));
		if (status != NDIS_SUCCESS)
		goto _err1;
	}

	status = NICInitializeAdapter(pAd);
	if (status != NDIS_SUCCESS)
	{
		DBGPRINT(RT_ERROR, ("[ERR]NICInitializeAdapter, Status[=0x%08x]\r\n", status));
		if (status != NDIS_SUCCESS)
		goto _err1;
	}	

	NICReadEEPROMParameters(pAd);
	NICInitAsicFromEEPROM(pAd);	
	RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);

	RTMPusecDelay(100);

	return NDIS_SUCCESS;

_err1:
	DBGPRINT(RT_ERROR, ("[ERR]\n<---%s",  __FUNCTION__));
	return NDIS_FAILURE;	
}


VOID MLME_ModeInit(
	IN PRTMP_ADAPTER 	pAd)
{	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MT7601_ChipSwitchChannel(pAd,  pAd->channel, 0);
		NdisMoveMemory(pAd->essid, pAd->CurrentAddress, 6);
		AsicSetBssid(pAd, pAd->essid);

		if (pAd->SsidLen == 0 || pAd->ApCfg.WpaPassPhraseLen == 0)
		{
			DBGPRINT(RT_WARN, ("[WARN]%s: Using Default SSID/KEY\r\n", __FUNCTION__));
			MLME_SetDefaultAp(pAd);
		}

		DBGPRINT(RT_INFO, ("[INFO]%s: AP SSID %s\r\n", __FUNCTION__, pAd->Ssid));
		DBGPRINT(RT_INFO, ("[INFO]%s: AP Auth %d\r\n", __FUNCTION__, pAd->ApCfg.AuthMode));
		if (pAd->ApCfg.EncMode != Ndis802_11Cipher_NONE)
		{
			DBGPRINT(RT_INFO, ("[INFO]%s: AP Password %s\r\n", __FUNCTION__, pAd->ApCfg.WpaPassPhrase));
			DBGPRINT(RT_INFO, ("[INFO]%s: AP Enc %d\r\n", __FUNCTION__, pAd->ApCfg.EncMode));
			DBGPRINT(RT_INFO, ("[INFO]%s: AP GroupKeyWepStatus %d\r\n", __FUNCTION__, pAd->ApCfg.GroupKeyWepStatus));
		}
		DBGPRINT(RT_INFO, ("[INFO]%s: AP channel %d\r\n", __FUNCTION__, pAd->channel));

		if (pAd->WEPEnabled)
		{
			RTMPSetChipHWKey(pAd, pAd->ApCfg.WpaPassPhrase, pAd->ApCfg.WpaPassPhraseLen, pAd->ApCfg.WepStatus);		
		
			if ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPAPSK) || (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
			{
				WpaApStartup(pAd);
			}
		}		
		
		MakeIbssBeacon(pAd, 0, 0);
		AsicFifoExtSet(pAd);
	}	
	else IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		MT7601_ChipSwitchChannel(pAd, pAd->channel, 0);
	}
	else
		DBGPRINT(RT_ERROR, ("[ERR]%s: Unsupport mode (%d)\r\n", __FUNCTION__, pAd->OpMode));

	/* Init mlme periodic timer*/
#ifdef TIMER_DEBUG
	RTMPInitTimer(pAd, &pAd->PeriodicTimer, (PCHAR)"MLME_PeriodicExec", MLME_PeriodicExec, pAd, TRUE);
#else
	RTMPInitTimer(pAd, &pAd->PeriodicTimer, MLME_PeriodicExec, pAd, TRUE);
#endif
	RTMPSetTimer(&pAd->PeriodicTimer, MLME_PERIODIC_EXEC_INTV);
}


NTSTATUS MLME_AdapterInit(
	IN PRTMP_ADAPTER 	pAd) 
{
	DBGPRINT(RT_TRACE, ("[TRACE]%s: Wifi param sz(%d)\r\n", __FUNCTION__, sizeof(RTMP_ADAPTER)));

	pAd->smartSetup = SmartConfigOff;

	MGMTENTRY_Init(pAd);

	if (MLME_GetConfig(pAd) != NDIS_SUCCESS)
		goto _fail;

	if (RTMPNICInitTxMem(pAd) != NDIS_SUCCESS)
		goto _fail;

	if (MLME_InitHardwre(pAd) != NDIS_SUCCESS)
		goto _fail;
	
	MLME_ModeInit(pAd);

	/* Enable Tx &Rx */
	RTMPDrvOpen(pAd);

	return NDIS_SUCCESS;

_fail:
	DBGPRINT(RT_ERROR, ("[ERR]%s: Initial FAIL..\r\n", __FUNCTION__));
	return NDIS_FAILURE;
}
