#include "system.h"
#include "rtmp.h"
#include "rtmp_ctrl.h"
#include "usb_io.h"
#include "asic_ctrl.h"
#include "nl80211.h"
#include "MT7601/mt7601_ctrl.h"
#include "wifi_api.h"
#include "mlme.h"

extern xQueueHandle xEzsetupEventQueue;
extern xQueueHandle xWifiEventQueue_DEV;
extern xTaskHandle xWiFiEdccaTask;
extern xQueueHandle xWifiEventQueue;
extern xQueueHandle xWifiEventQueue_DEV;
extern xTaskHandle xWiFiScanTask;

extern PRTMP_ADAPTER ppAd;

extern UCHAR WIFIInitialDone;

unsigned char * wlan_get_get_mac_addr(void)
{
	if (ppAd)
		return ppAd->CurrentAddress;
	else
		return NULL;
}

PAPScanResultElement WiFi_get_scan_RouterInfo(void)
{
	PAPScanResultElement	pEntry = NULL;
	
	if (ppAd)
	{
		pEntry = ppAd->FirstElem;
		return pEntry;
	}	
	else
		return NULL;
}

unsigned char WiFiAPLinkNum(void)
{
	if (ppAd)
		return (ppAd->node_num);
	else
		return (0xff);
}


unsigned char WIFI_AP_Initial_Done()
{

	return WIFIInitialDone;

}




int prvSendEventToWiFiTask(xWifiStackEvent_t *xEventMessage)
{
	const TickType_t xDontBlock = 0;
	BaseType_t xReturn = pdFAIL;
	
	if (xWifiEventQueue_DEV) 
	{
		xReturn = xQueueSendToBack(xWifiEventQueue_DEV, (void *)xEventMessage, xDontBlock );
		if ( xReturn != pdPASS )
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Send To Queue Error ...\n", __FUNCTION__));
		}
	}
	else
		DBGPRINT(RT_ERROR, ("[ERR]%s: xWifiEventQueue_DEV is NULL...\n", __FUNCTION__));
		
	return xReturn;
}

unsigned int WiFi_QueryAndSet(unsigned char item, unsigned char *pOutBuf, unsigned short *pLen)
{
	USHORT len = 0;
	ULONG val;

	if (ppAd == NULL) 
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Wifi doesn't startup!!\n", __FUNCTION__));
		return QUERY_FAIL;
	}

	if (ppAd->USBPlugOut)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Fail, wifi dongle is pluged out!!\n", __FUNCTION__));
		return QUERY_FAIL;
	}

	if (WIFIInitialDone != TRUE)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Wifi is not ready!!\n", __FUNCTION__));
		return QUERY_FAIL;
	}

	switch (item)
	{
		case QID_HW_MAC_ADDR:
			len = *pLen;
			NdisMoveMemory(pOutBuf, ppAd->PermanentAddress, 6);
			break;

		case SET_HW_MAC_ADDR:
			len = *pLen;
			NdisMoveMemory(ppAd->CurrentAddress, pOutBuf, 6);
			NdisMoveMemory(&val, pOutBuf, 4);
			RTMP_IO_WRITE32(ppAd, MAC_ADDR_DW0, val);
			RTMP_IO_READ32(ppAd, MAC_ADDR_DW1, &val);
			val &= 0xFFFF0000;
			val |=*(UINT16 *)(ppAd->CurrentAddress+4);
			RTMP_IO_WRITE32(ppAd, MAC_ADDR_DW1, val);
			break;

		case SET_HW_ESSID_ADDR:
			len = *pLen;
			NdisMoveMemory(ppAd->essid, pOutBuf, 6);
			AsicSetBssid(ppAd, ppAd->essid);
			break;	

		case SET_HW_CHANNEL:
			ppAd->channel = *pOutBuf;
			MT7601_ChipSwitchChannel(ppAd, ppAd->channel, 0);
			break;

		case QID_HW_CHANNEL:
			 *pOutBuf = ppAd->channel ;
			break;

		case SET_BEACON_ON:
			len = *pLen;
			MakeIbssBeacon(ppAd, pOutBuf, 0);
			break;

		case SET_BEACON_OFF:
			AsicDisableIbssSync(ppAd);
			break;

		case SET_BEACON_INTERVAL:
			ppAd->beaconPeriod = *pOutBuf;
			break;

		case SET_BEACON_SSID:
			len = *pLen;
			ppAd->SsidLen = len;
			NdisMoveMemory(ppAd->Ssid, pOutBuf, len);
			IF_DEV_CONFIG_OPMODE_ON_AP(ppAd)
			{
				/* If WPA & WPA2, then it need to update wpa information. */
				if (IS_WPA_CAPABILITY(ppAd->ApCfg.AuthMode)) 
				{
					RTMPUpdateWPAInfo(ppAd, (UCHAR *)ppAd->Ssid, ppAd->SsidLen);
				}
			}
			break;

		case QID_BEACON_SSID:
			len = ppAd->SsidLen;
			*pLen = len;
			NdisMoveMemory(pOutBuf, ppAd->Ssid, len);
			break;

		case QID_SECURITY_WEP:
			len = ppAd->SharedKey[0][0].KeyLen;
			*pLen = len;
			NdisMoveMemory(pOutBuf, ppAd->SharedKey[0][0].Key, len);
			break;

		case SET_SECURITY_WEP128:
		case SET_SECURITY_WEP64:
			IF_DEV_CONFIG_OPMODE_ON_AP(ppAd)
			{
				RTMPSetWEPCfg(ppAd, item, pOutBuf, *(PUCHAR)pLen);
			}	
			else IF_DEV_CONFIG_OPMODE_ON_STA(ppAd)
			{
				ppAd->StaCfg.WpaPassPhraseLen = *pLen;
				NdisZeroMemory(ppAd->StaCfg.WpaPassPhrase, 64);
				NdisMoveMemory(ppAd->StaCfg.WpaPassPhrase, pOutBuf, ppAd->StaCfg.WpaPassPhraseLen);
			}
			else
				DBGPRINT(RT_ERROR, ("[ERR]%s: unknown opmode.\n", __FUNCTION__));
			break;

		case SET_SECURITY_WEP128_DISABLED:
		case SET_SECURITY_WEP64_DISABLED:
			ppAd->WEPEnabled = FALSE;
			break;

		case SET_SECURITY_WEP_DEFAULT_KEY:
			ppAd->DefaultKeyId = (*pOutBuf)& 0x3;
			break;

		case QID_SECURITY_WPA:
		case QID_SECURITY_WPA2:
			len = ppAd->ApCfg.WpaPassPhraseLen;
			*pLen = len;
			NdisMoveMemory(pOutBuf, ppAd->ApCfg.WpaPassPhrase, len);
			break;
			
		case SET_SECURITY_WPA:
		case SET_SECURITY_WPA2:
			IF_DEV_CONFIG_OPMODE_ON_AP(ppAd)
			{
				RTMPSetWPACfg(ppAd, item, pOutBuf, *(PUCHAR)pLen);
			}
			else IF_DEV_CONFIG_OPMODE_ON_STA(ppAd)
			{
				ppAd->StaCfg.WpaPassPhraseLen = *pLen;
				NdisZeroMemory(ppAd->StaCfg.WpaPassPhrase, 64);
				NdisMoveMemory(ppAd->StaCfg.WpaPassPhrase, pOutBuf, ppAd->StaCfg.WpaPassPhraseLen);

				/* reset PMK */
				NdisZeroMemory(ppAd->StaCfg.PMK, LEN_PMK);
				RT_CfgSetWPAPSKKey(ppAd,
								(PSTRING)ppAd->StaCfg.WpaPassPhrase,
								ppAd->StaCfg.WpaPassPhraseLen,
								(PUCHAR)ppAd->Ssid,
								ppAd->SsidLen,
								ppAd->StaCfg.PMK);
			}
			else
				DBGPRINT(RT_ERROR, ("[ERR]%s: unknown opmode.\n", __FUNCTION__));	
			break;		

		case QID_LINK_STATE:
			*pOutBuf = (ppAd->LinkState==LS_LINKED)? 1:0;
			break;

		case SET_START_AP_SCAN:
			{
				xTaskCreate(WiFiScanTask, "WiFiScanTask_AP", 512, (PVOID)ppAd, 2, &xWiFiScanTask);
				DeleteScanList(ppAd);
				RTMP_IO_WRITE32(ppAd, PBF_CTRL, 0x20);
				RTMP_IO_WRITE32(ppAd, RX_FILTR_CFG, NO_LINK_FILTER);
				MLME_SetScanFlag(ppAd, 1);
			}
			break;

		case SET_STOP_AP_SCAN:
			MLME_SetScanFlag(ppAd, 0);
			RTMP_IO_WRITE32(ppAd, PBF_CTRL, 0x20);
			RTMP_IO_WRITE32(ppAd, RX_FILTR_CFG, LINK_FILTER);			
			if (xWiFiScanTask)
			{
				vTaskDelete(xWiFiScanTask);
				xWiFiScanTask = NULL;
			}
			break;

		case QID_SCAN_STATUS:		
			*pOutBuf = ppAd->scanFlag;
			if(*pOutBuf == 0)
			{
				*pLen = ppAd->nCollectNbr;
				DBGPRINT(RT_ERROR, ("ppAd->APArray.nCollectNbr = %d",ppAd->nCollectNbr));
			}
			break;	
			
		case SET_SMART_CONFIG_ON:
			ppAd->smartSetup = SmartConfigStart;
			RTMP_IO_WRITE32(ppAd, PBF_CTRL, 0x20);
			RTMP_IO_WRITE32(ppAd, RX_FILTR_CFG, SCAN_FILTER);			
			break;

		case SET_SMART_CONFIG_OFF:
			ppAd->smartSetup = SmartConfigOff;
			break;

		case SET_SMART_CONFIG_RESTART:
			ppAd->smartSetup = SmartConfigRestart;
			MLME_SetScanFlag(ppAd, 0);
			RTMP_IO_WRITE32(ppAd, PBF_CTRL, 0x20);
			RTMP_IO_WRITE32(ppAd, RX_FILTR_CFG, NO_LINK_FILTER);
			break;

		case SET_SMART_CONFIG_FAIL:
			ppAd->smartSetup = SmartConfigFail;
			break;

		case SET_TX_RETRY_COUNT:
			(*pOutBuf) = (*pOutBuf) & 0xFF;
			RTMP_IO_READ32(ppAd, TX_RTY_CFG, &val);
			val = val & 0xFFFF0000;
			val = val | (*pOutBuf) | ((*pOutBuf) << 8);
			RTMP_IO_WRITE32(ppAd, TX_RTY_CFG, val);
			break;

		case SET_TX_PHY_RATE:
			ppAd->SetTxPHYRate = (*pOutBuf) & 0xF;
			ppAd->SetTxPHYMode = ((*pOutBuf) & 0xC0) >> 6;
			break;

		case SET_TX_AUTO_FALLBACK:
			ppAd->SetTxFallback = (*pOutBuf);
			break;

		case QID_GET_TX_STATUS:
			{
				ULONG tx_ok, tx_fail;
				RTMP_IO_READ32(ppAd, TX_STA_CNT0, &tx_fail);
				RTMP_IO_READ32(ppAd, TX_STA_CNT1, &tx_ok);
				tx_fail &= 0xFFFF;
				tx_ok &= 0xFFFF;

				if((tx_fail == 0) && (tx_ok == 0))
					*pOutBuf = 0xFF;
				else
					*pOutBuf = tx_fail *100/(tx_ok+tx_fail);

				if((*pOutBuf == 0) && (tx_fail != 0))
					*pOutBuf = 1;
			}
			break;

		case QID_GET_RX_STREGTH:
			{
				PMGMTENTRY	pNode;
				
				pNode = MGMTENTRY_GetNode(ppAd, pOutBuf);
				/* clear target memory */
				NdisZeroMemory(pOutBuf, 6);
				if (pNode)
				{
					NdisMoveMemory(pOutBuf, &pNode->rx_stregth, 1);	
				}
			}
			break;

		case SET_POWER_SAVE_OFF:
			if (ppAd->APPowerSave)
				RTMPSetPowerSuspend(ppAd, 0);
			
			ppAd->APPowerSave = 0;
			break;

		case SET_POWER_SAVE_ON:
			ppAd->APPowerSave = 1;
			ppAd->PSPeriod = *pOutBuf;
			break;

		case SET_SEND_DISCONNECT:
			NdisFillMemory(ppAd->MgmAddr, 6, 0xFF);
			for (val = 0; val < 10; val++)
			{	
				SendDeAuthReq(ppAd, ppAd->MgmAddr, &ppAd->TxContext, REASON_CLS3ERR);
				vTaskDelay(100 / portTICK_RATE_MS);	
			}		
			break;
			
		case SET_MAC_TX_ONOFF:
			if (*pOutBuf)
				ppAd->mac_tx_stop = FALSE;
			else
				ppAd->mac_tx_stop = TRUE;
			
			RTMP_EdccaTxCtrl(ppAd, ppAd->mac_tx_stop);
			break;
		
		case SET_EDCCA_ONOFF:
			if (*pOutBuf)
			{	
				ppAd->ed_tx_stoped = TRUE;
				MT7601_SetEDCCA(ppAd, TRUE);
				xTaskCreate(WiFiEdccaTask, "WiFiEdccaTask", 512, NULL, 108, &xWiFiEdccaTask);
			}
			else
			{
				MT7601_SetEDCCA(ppAd, FALSE);
				if (xWiFiEdccaTask)
				{
					vTaskDelete(xWiFiEdccaTask);
					xWiFiEdccaTask = NULL;
				}
				ppAd->ed_tx_stoped = FALSE;
			}
			break;
		
		case SET_EDCCA_PERIOD:
			NdisMoveMemory(&ppAd->ed_chk_period, pOutBuf, 4);
			if (ppAd->ed_chk_period < portTICK_RATE_MS)
				ppAd->ed_chk_period = portTICK_RATE_MS;
			
			break;
		
		case SET_EDCCA_ED_TH:
			NdisMoveMemory(&ppAd->ed_threshold, pOutBuf, 4);
			break;
		
		case SET_EDCCA_FALSE_CCA_TH:
			NdisMoveMemory(&ppAd->ed_false_cca_threshold, pOutBuf, 4);
			break;
		
		case SET_EDCCA_BLOCK_CHECK_TH:
			NdisMoveMemory(&ppAd->ed_block_tx_threshold, pOutBuf, 4);
			break;
			
		case QID_GET_ENTRY_INFO:
			MGMTENTRY_GetAllInfo(ppAd);
			break;
				
		case SET_AP_GTK_TIME:
			IF_DEV_CONFIG_OPMODE_ON_AP(ppAd)
			{
				USHORT time = *pLen;
				DBGPRINT(RT_INFO,("[INFO]Set GTK time = %d sec\n", time));
				ppAd->ApCfg.WPAREKEY.ReKeyInterval = time;
			}
			else
				DBGPRINT(RT_ERROR,("[ERR]%s: Current oper mode isn't AP mode\n", __FUNCTION__));
				
			break;
			
		case QID_GET_ROUTER_INFO:
			IF_DEV_CONFIG_OPMODE_ON_STA(ppAd)
			{
				DBGPRINT(RT_INFO, ("[INFO]Router SSID %s\n", ppAd->Ssid));
				DBGPRINT(RT_INFO, ("[INFO]Router Password %s\n", ppAd->StaCfg.WpaPassPhrase));
				DBGPRINT(RT_INFO, ("[INFO]Router Auth %d\n", ppAd->StaCfg.AuthMode));
				DBGPRINT(RT_INFO, ("[INFO]Router Enc %d\n", ppAd->StaCfg.EncMode));
				DBGPRINT(RT_INFO, ("[INFO]Router channel %d\n", ppAd->channel));
			}
			break;
		
		case QID_GET_SELF_INFO:
			IF_DEV_CONFIG_OPMODE_ON_AP(ppAd)
			{
				DBGPRINT(RT_INFO, ("[INFO]AP SSID %s\n", ppAd->Ssid));
				DBGPRINT(RT_INFO, ("[INFO]AP Password %s\n", ppAd->ApCfg.WpaPassPhrase));
				DBGPRINT(RT_INFO, ("[INFO]AP Auth %d\n", ppAd->ApCfg.AuthMode));
				DBGPRINT(RT_INFO, ("[INFO]AP Enc %d\n", ppAd->ApCfg.EncMode));
				DBGPRINT(RT_INFO, ("[INFO]AP GroupKeyWepStatus %d\n", ppAd->ApCfg.GroupKeyWepStatus));
				DBGPRINT(RT_INFO, ("[INFO]AP channel %d\n", ppAd->channel));
				DBGPRINT(RT_INFO, ("[INFO]AP GTK Update time %d\n", ppAd->ApCfg.WPAREKEY.ReKeyInterval));
			}
			else IF_DEV_CONFIG_OPMODE_ON_STA(ppAd)
			{
				/* nothing */
			}
		
			break;

		case SET_PACKET_TYPE_PROTECT:
			{	
				USHORT type = 0;
				memcpy(&type, pOutBuf, 2);
				if (type == 0xFFFF)
				{
					ppAd->numTypeProtect = 0;
					memset(ppAd->valTypeProtect, 0x0, sizeof(ppAd->valTypeProtect));
				}
				else if (ppAd->numTypeProtect >= MAX_TYPE_PROTECT)
					return 0;
				else
				{
					ppAd->valTypeProtect[ppAd->numTypeProtect] = type;
					ppAd->numTypeProtect++;			
				}
			}
			break;

		case QID_AUTH_MODE:
			IF_DEV_CONFIG_OPMODE_ON_AP(ppAd)
			{
				NdisMoveMemory(pOutBuf, &ppAd->ApCfg.EncMode, 1);			
			}
			else
			{
				NdisMoveMemory(pOutBuf, &ppAd->StaCfg.EncMode, 1);
			}
			break;

		case QID_RUN_MODE:
			{
				NdisMoveMemory(pOutBuf, &ppAd->run_mode, 1);
			}
			break;

		case SET_TX_CHANNEL_POWER:
			{
				CHAR txPwer;

				RTMP_IO_READ32(ppAd, TX_ALC_CFG_0, &val);
				txPwer = val & 0x3F;
				val = val & (~0x3F3F);
				txPwer += *(char *)pOutBuf;
				if (txPwer < 0)
					txPwer = 0;
				val |= (txPwer & 0x3F);
				RTMP_IO_WRITE32(ppAd, TX_ALC_CFG_0, val);
			}
			break;

		case SET_RX_FILTER_SSID_LIMITED:
			RTMP_IO_WRITE32(ppAd, RX_FILTR_CFG, LINK_FILTER);
			break;
	
		case SET_RX_FILTER_SSID_NOT_LIMITED:
			RTMP_IO_WRITE32(ppAd, RX_FILTR_CFG, NO_LINK_FILTER);
			break;

		case SET_SUSPEND_TIME:
			{
				UINT time, tx, rx;
				memcpy(&time, pOutBuf, 4);
				memcpy(&tx, pOutBuf+4, 4);
				memcpy(&rx, pOutBuf+8, 4);				
				
				val = 0x0;
				if (tx == 1)		//Tx Enable
					val |= 0x4;
									
				if (rx == 1)		//Rx Enable
					val |= 0x8;

				RTMP_IO_WRITE32(ppAd, MAC_SYS_CTRL, val);
      			vTaskDelay(time);
      			//TX/RX enable
      			RTMP_IO_WRITE32(ppAd, MAC_SYS_CTRL, 0xC);			
			}			
			break;

		case SET_START_AP_CONNECT:
			MLME_StartAPConnect(ppAd);
			break;
			
		default:
			break;
	}
	return QUREY_SUCCESS;
}


