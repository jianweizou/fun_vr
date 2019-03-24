#include "rx.h"
#include "rtmp.h"
#include "wifi_api.h"
#include "nl80211.h"
#include "cipher.h"
#include "mlme.h"
#include "MT7601/mt7601_ctrl.h"
#include "asic_ctrl.h"
#include "rtmp_ctrl.h"
#ifdef DEBUG
#include "usb_io.h"
#endif

extern xQueueHandle xWifiEventQueue;

static CHAR ConvertToRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi,
	IN UCHAR			RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

	/* Rssi equals to zero should be an invalid value*/
	if (Rssi == 0)
		return -99;

	LNAGain = GET_LNA_GAIN(pAd);
	if (pAd->LatchRfRegs.Channel > 14)
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
	}
	else
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
	}	
	return (-12 - RssiOffset - LNAGain - Rssi);
}


VOID STAHandleRxControlFrame(
	IN	PRTMP_ADAPTER	pAd)
{
	PRXWI_STRUC		pWI;

	pWI = (PRXWI_STRUC)(pAd->RxUSBDataPtr + sizeof(RXINFO_STRUC));
	DBGPRINT(RT_TRACE,("[TRACE]%s: ControlFrame len=%d \r\n", __FUNCTION__, pWI->MPDUtotalByteCnt));	
}


VOID STAHandleRxMgmtFrame(
	IN	PRTMP_ADAPTER	pAd)
{
	PRXWI_STRUC			pWI;
	PHEADER_802_11		pWiFiHeader;
	PUCHAR 				pData;
	int 				len;
	UCHAR 				frmType;
	UCHAR				ie = 0, ieLen = 0; 	
    PUSHORT 			CapabilityInfo;
	
	pWI = (PRXWI_STRUC)(pAd->RxUSBDataPtr + sizeof(RXINFO_STRUC));
	pWiFiHeader = (PHEADER_802_11)((PUCHAR)pWI + sizeof(RXWI_STRUC));
	pData = (PUCHAR) ((PUCHAR)pWiFiHeader + sizeof(HEADER_802_11));	
	frmType = *(PUCHAR)pWiFiHeader;

	switch (frmType)
	{
		case PKT_BEACON:
			{
				CapabilityInfo = (PUSHORT)(pData + (TIME_STAMP_LEN + INTERVAL_LEN));
				pData = pData + (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);
				len = pWI->MPDUtotalByteCnt - sizeof(HEADER_802_11) - (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);

				if (pWiFiHeader->Sequence < 20/*15*/)
				{
					if (MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid))
						MLME_SetRxBcnCount(pAd, 1);
					break;
				}

				if (pAd->LinkState == LS_SCAN)
				{
					ParseBeacon(pAd, (PUCHAR)pWiFiHeader, len);
				}
				else if (pAd->LinkState == LS_SCAN_BSSID)
				{
					if (MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid) || RTMPCheckBeacon(pAd, pData, len))
					{
						NdisMoveMemory(pAd->essid, pWiFiHeader->Addr2, MAC_ADDR_LEN);	

						pAd->channel = RTMPGetAPChannel(pData, len);
						if (pAd->channel == 0)
						{
							pAd->channel = pAd->scanChannel;
							DBGPRINT(RT_WARN, ("[WARN]%s: No Channel Info, Set To Ch=%d", __FUNCTION__, pAd->channel));
							pAd->LinkState = LS_NO_LINK;
						}
						else
						{
							pAd->LinkState = LS_START_LINK;
							NdisMoveMemory(pAd->MgmAddr, pWiFiHeader->Addr2, MAC_ADDR_LEN);
							MLME_SetScanFlag(pAd, 0);
							MT7601_ChipSwitchChannel(pAd, pAd->channel, 0);
							AsicSetBssid(pAd, pAd->essid);
							RTMPSetLinkFliter(pAd, TRUE);
							
							/* Send dorect probe to acquie specified router information */
							SendIbssProbeReq(pAd, &pAd->TxContext);
							pAd->WEPEnabled = (*CapabilityInfo & 0x0010)? 1:0;
						}
					}
				}
				else if ((pAd->LinkState == LS_START_LINK) &&
						MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid))
				{
					/* Send dorect probe to acquie specified router information */
					SendIbssProbeReq(pAd, &pAd->TxContext);
				}
				else if ((pAd->LinkState == LS_BEACON_AUTH) &&
						MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid))
				{
					DBGPRINT(RT_INFO, ("[INFO]Auth to %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
					SendAuthPacket(pAd, pWiFiHeader->Addr2, &pAd->TxContext, 1, MLME_SUCCESS);
				}
				else if ((pAd->LinkState == LS_AUTH_ASSOCIATE) &&
						MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid))
				{
					DBGPRINT(RT_INFO, ("[INFO]Assoc to %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
					SendAssocReq(pAd, pWiFiHeader->Addr2, &pAd->TxContext);
				}			
				else if (MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid))
				{
					MLME_SetRxBcnCount(pAd, 1);
				}
			}
			break;

		case PKT_PROBE_RESP:
			{
				DBGPRINT(RT_INFO, ("[INFO]ProbeRsp from %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
				pData += (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);
				len = pWI->MPDUtotalByteCnt - sizeof(HEADER_802_11) - (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);
				if ((pAd->LinkState == LS_START_LINK) && 
					MAC_ADDR_EQUAL(pWiFiHeader->Addr1, pAd->CurrentAddress))
				{
					do
					{
						ie = pData[0];
						ieLen = pData[1];

						if (ie == 0x0 && ieLen == pAd->SsidLen)
						{
							if (NdisCmpMemory(pAd->Ssid, pData+2, ieLen) == 0)
							{
								/* go to next connection stage */
								pAd->LinkState = LS_BEACON_AUTH;
							}
						}
						len = len - 2 - ieLen;
						pData = pData + 2 + ieLen;
					} while (len > 2);
				}			
			}
			break;

		case PKT_AUTH:
			DBGPRINT(RT_INFO, ("[INFO]Auth from %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
			if ((pAd->LinkState == LS_BEACON_AUTH) && 
				MAC_ADDR_EQUAL(pWiFiHeader->Addr1, pAd->CurrentAddress))
			{
				pAUTH_IE auth;

				auth = (pAUTH_IE)pData;
				
				if ((auth->auth_alg == 0 && auth->auth_seq == 2 && auth->auth_status == 0) || 
				   (auth->auth_alg == 1 && auth->auth_seq == 4 && auth->auth_status == 0))
				{
					pAd->LinkState = LS_AUTH_ASSOCIATE;
					DBGPRINT(RT_INFO, ("[INFO]Assoc to %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
					SendAssocReq(pAd, pWiFiHeader->Addr2, &pAd->TxContext);
				}
				else if (auth->auth_alg == 0 && auth->auth_seq == 2 && auth->auth_status == 13)
					SendAuthPacket(pAd, pWiFiHeader->Addr2, &pAd->TxContext, 2, MLME_SUCCESS);
				else if (auth->auth_alg == 1 && auth->auth_seq == 2 && auth->auth_status == 0)
					SendAuthRspAtSeq2Action(pAd, pWiFiHeader->Addr2, &pAd->TxContext, pData);
				else
					DBGPRINT(RT_ERROR, ("[ERR]%s: auth->auth_status = %d\r\n",
										__FUNCTION__, auth->auth_status));
			}
			break;

		case PKT_ASSOC_RSP:
			DBGPRINT(RT_INFO, ("[INFO]Assoc from %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
			if ((pAd->LinkState == LS_AUTH_ASSOCIATE) && 
				(MAC_ADDR_EQUAL(pWiFiHeader->Addr1, pAd->CurrentAddress)))
			{
				if (RTMPCheckAssociation(pAd, pData))
				{
					pAd->LinkState = LS_LINKED;

					/* Betters Test */
					if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
						(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
						(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
						(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)) 
					{
						/* do nothing, just waiting for Eapol pakcet. */
					}
					else
					{
						MLME_Startdhcpc(pAd);
					}
				}
			}
			break;

		case PKT_AUTH_DEATH	:
		case PKT_ASSOC_DIS:
			if ((MAC_ADDR_EQUAL(pWiFiHeader->Addr2, pAd->essid)) && (pAd->LinkState == LS_LINKED))
			{
				USHORT code = *(USHORT *)&pData[0];
				DBGPRINT(RT_WARN, ("[WARN]%s: AP Send %s...code=%d\r\n", 
									__FUNCTION__, (frmType == PKT_AUTH_DEATH)? "AUTH_DEATH":"ASSOC_DIS", code));

				/* Delete entry*/
				MGMTENTRY_Del(pAd, pAd->StaCfg.RouterMac);
				/* startup reconnection !! */
				MLME_DoDisconnect(pAd, TRUE);
			}
			break;			
		default:
			break;
	}
	return;
}


VOID APHandleRxMgmtFrame(
	IN	PRTMP_ADAPTER	pAd)
{
	PRXWI_STRUC			pWI;
	PHEADER_802_11		pWiFiHeader;
	PUCHAR 				pData;
	int 				len;
	UCHAR 				frmType;
	AP_Message 			mMessage;

	pWI = (PRXWI_STRUC)(pAd->RxUSBDataPtr + sizeof(RXINFO_STRUC));
	pWiFiHeader = (PHEADER_802_11)((PUCHAR)pWI + sizeof(RXWI_STRUC));
	pData = (PUCHAR) ((PUCHAR)pWiFiHeader + sizeof(HEADER_802_11));
	len = pWI->MPDUtotalByteCnt - sizeof(HEADER_802_11) - (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);	
	frmType = *(PUCHAR)pWiFiHeader;

	/* drop some request due to MIC error protection */
	if (pAd->ApCfg.BANClass3Data == TRUE)
	{
		/* disallow new association */
		if ((pWiFiHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pWiFiHeader->FC.SubType == SUBTYPE_AUTH))
		{
			DBGPRINT(RT_TRACE, ("Disallow new Association due to CounterMeasure error, system will block until 60 seconds.\r\n"));
			return;
		}
	}

	switch (frmType)
	{
		case PKT_BEACON:
			if (MLME_GetScanFlag(pAd))
			{
				CollectScanBeacon(pAd, (PUCHAR)pWiFiHeader, len, ConvertToRssi(pAd, pWI->RSSI0, 0));
			}
			break;

		case PKT_PROBE_REQ:
			DBGPRINT(RT_LOUD, ("[LOUD]%s: ProbeReq: from %02X-%02X-%02X-%02X-%02X-%02X\r\n", 
									__FUNCTION__, PRINT_MAC(pWiFiHeader->Addr2)));
			{
				if (RTMPCheckBeacon(pAd, pData, len) == TRUE)
				{
					NdisZeroMemory(&mMessage, sizeof(AP_Message));
					NdisMoveMemory(mMessage.MgmAdd, pWiFiHeader->Addr2, MAC_ADDR_LEN);
					NdisMoveMemory(pAd->essid, pAd->CurrentAddress, MAC_ADDR_LEN);

					mMessage.message = SEND_PROBE_RESPOND;
					xQueueSendToBack(xWifiEventQueue, (void *)&mMessage, 0);
				}
			}
			break;

		case PKT_AUTH:
			DBGPRINT(RT_TRACE, ("[TRACE]Authenticate: %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
			{
				USHORT alg;

				if (PeerAuthSanity(pAd, pWiFiHeader, pWI->MPDUtotalByteCnt, &alg) == FALSE)
					return;

				NdisZeroMemory(&mMessage, sizeof(AP_Message));
				NdisMoveMemory(mMessage.MgmAdd, pWiFiHeader->Addr2, MAC_ADDR_LEN);

				if ((alg == AUTH_MODE_OPEN) && (pAd->ApCfg.AuthMode != Ndis802_11AuthModeShared)) 
				{
					if (MGMTENTRY_Add(pAd, pWiFiHeader->Addr2) != TRUE)
						return;

					mMessage.reserved = MLME_SUCCESS;
				}
				else if ((alg == AUTH_MODE_KEY) && ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeShared) ||
					((pAd->ApCfg.AuthMode == Ndis802_11AuthModeAutoSwitch))))
				{
					if (MGMTENTRY_Add(pAd, pWiFiHeader->Addr2) != TRUE)
						return;

					mMessage.reserved = MLME_SUCCESS;
				}
				else {
					mMessage.reserved = MLME_ALG_NOT_SUPPORT;
				}
				
				mMessage.message = SEND_AUTH_RESPOND;
				xQueueSendToBack(xWifiEventQueue, (void *)&mMessage, 0);
			}
			break;

		case PKT_ASSOC_REQ:
			DBGPRINT(RT_TRACE, ("[TRACE]Associate: from %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
			{
				/* allocate memory */
				PIE_LISTS ie_list;
				PMGMTENTRY pEntry = NULL;
				UINT statuscode = 0;

				if (MAC_ADDR_EQUAL(pWiFiHeader->Addr1, pAd->CurrentAddress) == 0)
					break;

				pEntry = MGMTENTRY_GetNode(pAd, pWiFiHeader->Addr2);
				if (pEntry == NULL)
					break;

				ie_list = (PIE_LISTS)pvPortMalloc(sizeof(IE_LISTS));
				if (ie_list == NULL) {
					DBGPRINT(RT_ERROR, ("%s(): mem alloc failed\r\n", __FUNCTION__));
					break;
				}
				
				NdisZeroMemory(ie_list, sizeof(IE_LISTS));
				
				if (PeerAssocReqCmmSanity(pAd, pWiFiHeader, FALSE, pWI->MPDUtotalByteCnt, ie_list) == FALSE)
				{
					vPortFree(ie_list);
					break;
				}

				if ((statuscode = RTMPCheckValidateRSNIE(pAd, pEntry, &ie_list->RSN_IE[0], ie_list->RSNIE_Len)) != SUCCESS) {
					DBGPRINT(RT_ERROR, ("[ERR]%s: AssocReq failed error code %d\r\n", __FUNCTION__, statuscode));
	
				}

				NdisZeroMemory(&mMessage, sizeof(AP_Message));
				NdisMoveMemory(mMessage.MgmAdd, pWiFiHeader->Addr2, MAC_ADDR_LEN);
				mMessage.reserved = statuscode;
				mMessage.message = SEND_ASSOCIATION_RESPOND;
				xQueueSendToBack(xWifiEventQueue, (void *)&mMessage, 0);
				DBGPRINT(RT_TRACE, ("[TRACE]%s: Device Linked: %02X-%02X-%02X-%02X-%02X-%02X\r\n", 
									__FUNCTION__, PRINT_MAC(pWiFiHeader->Addr2)));

				vPortFree(ie_list);
			}
			break;

		case PKT_ASSOC_DIS:
		case PKT_AUTH_DEATH:
			if (frmType == PKT_ASSOC_DIS)
				DBGPRINT(RT_TRACE, ("[TRACE]Associate DIS: from %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
			else
				DBGPRINT(RT_TRACE, ("[TRACE]Deauthenticate: from %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));

			if (MAC_ADDR_EQUAL(pWiFiHeader->Addr1, pAd->CurrentAddress))
			{
				PMGMTENTRY pEntry = NULL;

				pEntry = MGMTENTRY_GetNode(pAd, pWiFiHeader->Addr2);
				if (pEntry == NULL)
					break;

				if (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK || pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
				{
					if (pEntry->WpaState != AS_PTKINITDONE)
					{
						DBGPRINT(RT_WARN, ("[WARN]Doing 4-way handshake, cannot delete %02X-%02X-%02X-%02X-%02X-%02X\r\n", PRINT_MAC(pWiFiHeader->Addr2)));
						break;
					}
				}

				MGMTENTRY_Del(pAd, pWiFiHeader->Addr2);
				DBGPRINT(RT_TRACE, ("[TRACE]%s: Device Disconnect: %02X-%02X-%02X-%02X-%02X-%02X\r\n", 
										__FUNCTION__, PRINT_MAC(pWiFiHeader->Addr2)));
			}
			break;

		default:
			break;
	}

	return;
}



VOID HandleRxDataFrame(
	IN	PRTMP_ADAPTER pAd)
{
	PRXWI_STRUC			pWI;
	PHEADER_802_11		pWiFiHeader;
	PUCHAR 				pData;
	PRXINFO_STRUC		pInfo;
	int 				len;

	pInfo = (PRXINFO_STRUC)pAd->RxUSBDataPtr;
	pWI = (PRXWI_STRUC)(pAd->RxUSBDataPtr + sizeof(RXINFO_STRUC));
	pWiFiHeader = (PHEADER_802_11)((PUCHAR)pWI + sizeof(RXWI_STRUC));
	/* Using ethhdr-like format */
	pData = (PUCHAR) ((PUCHAR)pWiFiHeader + sizeof(HEADER_802_11) - 6);

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* Check for all RxD errors */
		if (RTMPApCheckRxError(pAd, pInfo, pWI->wcid) != NDIS_SUCCESS)
		{
			RTMPAPRxDErrorHandle(pAd, pInfo, pWI);

			/* Increase received error packet counter per BSS */
			if (pWiFiHeader->FC.FrDs == 0 && pInfo->U2M)
			{
				pAd->ApCfg.RxDropCount++;
				pAd->ApCfg.RxErrorCount++;
			}

			DBGPRINT(RT_TRACE, ("[TRACE]%s: BSSID:%02X-%02X-%02X-%02X-%02X-%02X, SA:%02X-%02X-%02X-%02X-%02X-%02X, DA:%02X-%02X-%02X-%02X-%02X-%02X, Error code(%d)\r\n", 
								__FUNCTION__, PRINT_MAC(pWiFiHeader->Addr1), PRINT_MAC(pWiFiHeader->Addr2), PRINT_MAC(pWiFiHeader->Addr3), pInfo->CipherErr));
			/* discard this frame */
			return;
		}
#if 0
		else {
//			UCHAR target_MAC[6] = {0xC4, 0xD9, 0x87, 0xA3, 0x3B, 0xF3};
			PUCHAR pContent = (PUCHAR)pWiFiHeader;
			int L2PAD = 0;
			int Descrypt = 0;
			int reassemble = 0;
			int Qos = 0;
			int order = 0;
			int BMCast = 0;
			
			/* 1. skip 802.11 HEADER */
			pContent += sizeof(HEADER_802_11);
			/* 2. QOS */
			if (pWiFiHeader->FC.SubType & 0x08) {
				Qos = 1;
				pContent += 2;
			}
			/* 3. Order bit: A-Ralink or HTC+ */
			if (pWiFiHeader->FC.Order) {
				order = 1;
				pContent += 4;
			}
	
			/* 4. skip HW padding */
			if (pInfo->L2PAD)
			{
				/* just move pData pointer */
				/* because DataSize excluding HW padding */
				//RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
				pContent += 2;
				L2PAD = 1;
			}
	
			if (pInfo->Bcast || pInfo->Mcast) {
				BMCast = 1;
			}
			
	
			if (!((pWiFiHeader->Frag == 0) && (pWiFiHeader->FC.MoreFrag == 0)))
			{
				reassemble = 1;
			}
	
			if ((pWiFiHeader->FC.Wep == 1) && (pInfo->Decrypted == 0))
			{	
				Descrypt = 1;
			}
			
//			if (NdisEqualMemory(target_MAC, pWiFiHeader->Addr2, MAC_ADDR_LEN)) {
				DBGPRINT(RT_TRACE, ("[TRACE]WCID:%d; len(%d)DA:%02X-%02X-%02X-%02X-%02X-%02X - (BMCast, Qos, Order, L2PAD, reassemble, Descrypt) = (%d, %d, %d, %d, %d, %d)\r\n", 
										pWI->wcid, pWI->MPDUtotalByteCnt - sizeof(HEADER_802_11), PRINT_MAC(pWiFiHeader->Addr3), BMCast, Qos, order, L2PAD, reassemble, Descrypt));
//			}
		}
#endif

		/* Drop data util 60 seconds. (MIC error protection) */
		if (pAd->ApCfg.BANClass3Data == TRUE)
		{
			DBGPRINT(RT_TRACE, ("[TRACE]%s: Meet MIC error protection\r\n", __FUNCTION__));
			return;
		}
	}

	if (pWiFiHeader->FC.SubType != SUBTYPE_NULL_FUNC)
	{
		len = pWI->MPDUtotalByteCnt - sizeof(HEADER_802_11) + 6;

		if (pWiFiHeader->FC.SubType != SUBTYPE_DATA)
		{
			DBGPRINT(RT_INFO, ("[TRACE]%s: Recv Data Packet Type = %d\r\n", __FUNCTION__, pWiFiHeader->FC.SubType));
		}
		
		/* Qos data handling */
		if (pWiFiHeader->FC.SubType & SUBTYPE_QDATA)
		{
			len -= 2;
			pData += 4;
		}

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			NdisMoveMemory((PUCHAR)pWI, pWiFiHeader->Addr3, MAC_ADDR_LEN);
			NdisMoveMemory(pData, (PUCHAR)pWI, MAC_ADDR_LEN);
			NdisMoveMemory(pData + MAC_ADDR_LEN, pWiFiHeader->Addr2, MAC_ADDR_LEN);
		}
		else
		{	
			NdisMoveMemory(pData + MAC_ADDR_LEN, pWiFiHeader->Addr3, MAC_ADDR_LEN);
			NdisMoveMemory(pData, pWiFiHeader->Addr1, MAC_ADDR_LEN);
		}
			
		MLME_ProcessDataFrame(pAd, pData, len);
	}
}



BOOLEAN RxDoneInterruptHandle(
	IN PRTMP_ADAPTER 	pAd)
{
	NDIS_STATUS 	Status = 0;
	PRXWI_STRUC		pWI;
	PRXINFO_STRUC	pInfo;
	PHEADER_802_11	pWiFiHeader;
	PMGMTENTRY		pEntry;

	pInfo = (PRXINFO_STRUC)pAd->RxUSBDataPtr;
	pWI = (PRXWI_STRUC)(pAd->RxUSBDataPtr + sizeof(RXINFO_STRUC));
	pWiFiHeader = (PHEADER_802_11)((PUCHAR)pWI + sizeof(RXWI_STRUC));

	if (MAC_ADDR_EQUAL(pWiFiHeader->Addr1, pAd->CurrentAddress) || (pWiFiHeader->Addr1[0] & 0x1));
	else return 0;

	pEntry = MGMTENTRY_GetNode(pAd, pWiFiHeader->Addr2);
	if (pEntry)
	{
		pEntry->rx_stregth = ConvertToRssi(pAd, pWI->RSSI0, 0);
		pEntry->rx_time = (ULONG)xTaskGetTickCount();
#ifdef DEBUG
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if(pEntry->bActive == pWiFiHeader->FC.PwrMgmt)
			{
				ULONG val;

				if(pWiFiHeader->FC.PwrMgmt)
				{
					RTMP_IO_READ32(pAd, TX_STA_CNT0, &val);
					DBGPRINT(RT_INFO, ("[INFO]\n\n<=--%d ", pWiFiHeader->Sequence));
				}
				else
				{
					RTMP_IO_READ32(pAd, TX_STA_CNT0, &val);
					DBGPRINT(RT_INFO, (" %d - %d=>\n\r\n", val&0xFFFF, pWiFiHeader->Sequence));
				}
			}
		}
		DBGPRINT(RT_INFO, ("[INFO]%s: (id:%d)-%02X:%02X:%02X:%02X:%02X:%02X-PWR(%s)\r\n",
				__FUNCTION__, pEntry->Aid, PRINT_MAC(pEntry->Addr), (pWiFiHeader->FC.PwrMgmt == 0)?("Active"):("Sleep")));
#endif
		pEntry->bActive = (pWiFiHeader->FC.PwrMgmt==1)? FALSE:TRUE;
	}

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{		
		pAd->ApCfg.LastSNR0 = (UCHAR)(pWI->SNR0);
		pAd->ApCfg.LastSNR1 = (UCHAR)(pWI->SNR1);
	}
	else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (RTMPStaCheckRxError(pAd, pWiFiHeader, pWI, pInfo) != NDIS_SUCCESS)
		{
			return 0;
		}
	}
	
	switch (pWiFiHeader->FC.Type) 
	{		
		
		case BTYPE_DATA:	/* CASE I, receive a DATA frame */
			{
				if (pAd->smartSetup == SmartConfigStart)
				{
					if (MATCH_MULTI(pWiFiHeader->Addr1) || MATCH_MULTI(pWiFiHeader->Addr3))
						HandleRxDataFrame(pAd);

				}
				else if (pAd->smartSetup == SmartConfigOff)
				{
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						if (pEntry) {
							HandleRxDataFrame(pAd);
						}
					}
					else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
					{
						if (pAd->LinkState == LS_LINKED)
							HandleRxDataFrame(pAd);
					}
					else { /* do nothing */ }
				}
				else { /* do nothing */ }
			}
			break;	
		
		case BTYPE_MGMT:	/* CASE II, receive a MGMT frame */
			{
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{ 
					if (pAd->smartSetup == SmartConfigCheckBeacon) 
					{
						/* Detect specified router */
						ScanSmartConfigBeacon(pAd);
					}
					else if (pAd->smartSetup == SmartConfigOff)
						STAHandleRxMgmtFrame(pAd);
					else { /* do nothing */ }
				}
				else
					APHandleRxMgmtFrame(pAd);
			}
			break;
		
		case BTYPE_CNTL:	/* CASE III. receive a CNTL frame */
			{
				if (pAd->smartSetup == SmartConfigOff)
					STAHandleRxControlFrame(pAd);
			}
			break;
		
		default:	/* discard other type */
			DBGPRINT(RT_ERROR, ("[ERR]%s:  Type Packet %d\r\n", __FUNCTION__, pWiFiHeader->FC.Type));
			pAd->rxCorrectSeq = 0xFFFF;
			break;
	}

	return Status ;
}

