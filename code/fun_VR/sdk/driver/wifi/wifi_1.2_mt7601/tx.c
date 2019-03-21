#include "tx.h"
#include "asic_ctrl.h"
#include "rtmp_ctrl.h"
#include "common.h"
#include "usb_io.h"
#include "nl80211.h"
#include "wifi_api.h"

extern xSemaphoreHandle xMACSendMutex;
extern PRTMP_ADAPTER	ppAd;
	
static UCHAR txIndex = 0;
static UCHAR EAPOL[] = {0x88, 0x8e};



NDIS_STATUS RTMPNICInitTxMem(
	IN PRTMP_ADAPTER 	pAd)
{
	NDIS_STATUS		Status = NDIS_SUCCESS;
	PUCHAR 			pBuff;
	ULONG			val;
	int i;

	DBGPRINT(RT_TRACE, ("[TRACE]---> %s\n", __FUNCTION__));	

	/* TX Allocation */	
	pBuff = pvPortMalloc(256);//pvPortMalloc(512);
	if (pBuff == NULL)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: TX Mgm Alloaction Fail...\n", __FUNCTION__));

		return NDIS_FAILURE;
	}
	else pAd->TxMgmBase_org = pBuff;


	pAd->TxMgmBase = pBuff;
	NdisZeroMemory((PUCHAR)&pAd->TxContext, sizeof(TXINFO_STRUC));

	pAd->TxContext.pTXINF = (PTXINFO_STRUC)pBuff;
	pAd->TxContext.pBulkOUTBuff = (PUCHAR)pBuff;
	pBuff += sizeof(TXINFO_STRUC);
	pAd->TxContext.pTXWI = (PTXWI_STRUC)pBuff;
	pBuff += sizeof(TXWI_STRUC);
	pAd->TxContext.pWifiHeader = (PHEADER_802_11)pBuff;
	pBuff += sizeof(HEADER_802_11);
	pAd->TxContext.pDataBuff = pBuff;	

	for (i = 0; i < MAX_TX_RING; i++)
	{
		pBuff = pvPortMalloc(LOCAL_TXBUF_SIZE);
		if (pBuff == NULL)
		{
			/* FIXME */
			DBGPRINT(RT_ERROR, ("[ERR]%s: TX Data Alloaction Fail...%d ", __FUNCTION__, i));
			return  NDIS_FAILURE;
		}
		else pAd->tx_dma_org[i] = pBuff;
		
		val = (UINT32)pBuff;
		if (val%32)
			val = 32 - val%32;
		else val = 0;

		/* Memory alignment */
		pAd->tx_dma_pool[i] = (PUCHAR)(pBuff + val);
	}

	return Status;
}


VOID RTMPNICTxReleaseMem(
	IN PRTMP_ADAPTER 	pAd)
{
	int i = 0;
	
	for (i = 0; i < MAX_TX_RING; i++)
	{	
		if (pAd->tx_dma_org[i])
			vPortFree(pAd->tx_dma_org[i]);
		
		pAd->tx_dma_org[i] = NULL;
	}

	if (pAd->TxMgmBase_org) {
		vPortFree(pAd->TxMgmBase_org);
		pAd->TxMgmBase_org = NULL;
	}
}



VOID RTMPWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC 	pOutTxWI,
	IN	BOOLEAN			FRAG,
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,
	IN	BOOLEAN			CfAck,
	IN	PHTTRANSMIT_SETTING	pTransmit)
{		
	TXWI_STRUC 		TxWI;
	PTXWI_STRUC 	pTxWI;
	
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/
	
	//OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, sizeof(TXWI_STRUC));
	pTxWI = &TxWI;
	pTxWI->TXWI_N.FRAG	= FRAG;
	pTxWI->TXWI_N.CFACK	= CFACK;
	pTxWI->TXWI_N.TS	= InsTimestamp;
	pTxWI->TXWI_N.AMPDU = AMPDU;
	pTxWI->TXWI_N.ACK 	= Ack;
	pTxWI->TXWI_N.txop	= Txopmode;	
	pTxWI->TXWI_N.NSEQ 	= NSeq;
	
	/* John tune the performace with Intel Client in 20 MHz performance*/
	pTxWI->TXWI_N.wcid	= WCID;
	pTxWI->TXWI_N.MPDUtotalByteCnt = Length;
	pTxWI->TXWI_N.TxPktId = PID;

	/* If CCK or OFDM, BW must be 20*/

	pTxWI->TXWI_N.BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);

	/* P2P test case 6.1.12 */
	pTxWI->TXWI_N.MCS 		= 0;
	pTxWI->TXWI_N.PHYMODE 	= 0;
	pTxWI->TXWI_N.CFACK		= CfAck;

#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.bMIMOPSEnable)
	{
		if ((pAd->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > 7))
		{
			/* Dynamic MIMO Power Save Mode*/
			pTxWI->TXWI_N.MIMOps = 1;
		}
		else if (pAd->MmpsMode == MMPS_STATIC)
		{
			/* Static MIMO Power Save Mode*/
			if (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > 7)
			{
				pTxWI->TXWI_N.MCS = 7;
				pTxWI->TXWI_N.MIMOps = 0;
			}
		}
	}
	{
		pTxWI->TXWI_N.MpduDensity = pAd->CommonCfg.DesiredHtPhy.MpduDensity;
	}
#endif /* DOT11_N_SUPPORT */

	pTxWI->TXWI_N.TxPktId = pTxWI->TXWI_N.MCS;
	NdisMoveMemory(pOutTxWI, &TxWI, sizeof(TXWI_STRUC));
}


VOID RTMPWriteTxWI_Data(
	IN	PRTMP_ADAPTER	pAd,
	INOUT PTXWI_STRUC	pTxWI,
	IN	UINT8			wcid,
	IN	UINT32 			len,
	IN	UINT8			Phymode,
	IN	UINT8			McsRate,
	IN	UINT8			Fallback)
{
	NdisZeroMemory(pTxWI, sizeof(TXWI_STRUC));

// WORD 0
	pTxWI->TXWI_N.MCS 		= McsRate;
	pTxWI->TXWI_N.PHYMODE 	= Phymode;
	pTxWI->TXWI_N.BW 		= 0;
	pTxWI->TXWI_N.ShortGI 	= 0;
	pTxWI->TXWI_N.Autofallback = (Fallback)? 1 : 0;
	pTxWI->TXWI_N.txop		= 0;

// WORD 1
	if (pAd->DataAddr[0] & 0x1) 
	{	
		/* b/m frame packet */
		pTxWI->TXWI_N.MCS 		= MCS_RATE_1;
		pTxWI->TXWI_N.PHYMODE 	= MODE_CCK;

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			pTxWI->TXWI_N.ACK = 0;
		else
			pTxWI->TXWI_N.ACK = 1;
	}
	else 
		pTxWI->TXWI_N.ACK = 1;
	
	pTxWI->TXWI_N.MPDUtotalByteCnt = len - sizeof(TXINFO_STRUC) - sizeof(TXWI_STRUC);	
	pTxWI->TXWI_N.BAWinSize = 0x0;
	pTxWI->TXWI_N.wcid = wcid;
	pTxWI->TXWI_N.TxPktId = pTxWI->TXWI_N.MCS;
}


VOID RTMPWriteTxINFO(
	IN PTX_CONTEXT 	pTxContext, 
	IN ULONG 		FrameLen)
{
	int infoLen;
	
	NdisZeroMemory((PUCHAR)pTxContext->pTXINF, sizeof(TXINFO_STRUC));
	infoLen = FrameLen + sizeof(TXWI_STRUC);

	if (infoLen % 4)
		infoLen += (4 - infoLen%4);

	pTxContext->pTXINF->txinfo_nmac_pkt.pkt_len = infoLen;
	pTxContext->pTXINF->txinfo_nmac_pkt.pkt_80211 = 1;
	pTxContext->pTXINF->txinfo_nmac_pkt.QSEL = FIFO_MGMT;
	pTxContext->BulkOUTLen = infoLen + sizeof(TXINFO_STRUC) + 4;
}


VOID RTMPWriteTxINFO_Data(
	IN	PRTMP_ADAPTER		pAd,
	INOUT PTXINFO_STRUC		pTxINFO,
	IN	UINT32 				len,
	IN  UCHAR				next)
{
	NdisZeroMemory(pTxINFO, sizeof(TXINFO_STRUC));

	pTxINFO->txinfo_nmac_pkt.pkt_80211 	= 1;
	pTxINFO->txinfo_nmac_pkt.QSEL 		= FIFO_EDCA;
	pTxINFO->txinfo_nmac_pkt.pkt_len	= len - sizeof(TXINFO_STRUC);
	pTxINFO->txinfo_nmac_pkt.next_vld	= next;	
}


/* Need to check return value !!!! @Betters */
VOID RTMPKickOutTx(
	IN PRTMP_ADAPTER 	pAd, 
	IN PTX_CONTEXT 		pTxBlk)
{
	RTUSB_EPOUThndl(pAd, 4, pTxBlk->pBulkOUTBuff, pTxBlk->BulkOUTLen) ;
}

#ifdef PS_TX_QUEUE_DATA
pBufferCell AllocateNewCell(
	IN	PMGMTENTRY 	pNode,
	IN	ULONG 		pktlen)
{
	pBufferCell pNewCell;

	pNewCell = (pBufferCell) pvPortMalloc(sizeof(BufferCell));

	if(pNewCell == NULL)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s:TX PS Cell Allocate Fail...\n", __FUNCTION__));
		return NULL;
	}
	else
	{
		pNewCell->pPacket = pvPortMalloc(pktlen);
		if(pNewCell->pPacket == NULL )
		{
			vPortFree(pNewCell);
			DBGPRINT(RT_ERROR, ("[ERR]%s:TX PS Packet Allocate Fail...\n", __FUNCTION__));
			return NULL;
		}
		else
		{
			pNewCell->pNextCell = NULL;

			if(pNode->pPSTxCell)
			{
				pBufferCell pTmpCell;

				pTmpCell = pNode->pPSTxCell;

				while (pTmpCell->pNextCell != NULL)
					pTmpCell = pTmpCell->pNextCell;

				pTmpCell->pNextCell = pNewCell;
				return pNewCell;
			}
			else
			{
				pNode->pPSTxCell = pNewCell;
				return pNewCell;
			}
		}
	}
}

VOID SendAllPSCell(
	IN	PRTMP_ADAPTER 	pAd,
	IN	PMGMTENTRY 		pNode)
{
	pBufferCell pNextCell, pTmpCell;
	int cnt = 0;

	pTmpCell = pNode->pPSTxCell;
	while(pTmpCell != NULL)
	{
		RTUSB_EPOUThndl(pAd, 4, pTmpCell->pPacket, pTmpCell->packetLen);
		cnt++;
		vPortFree(pTmpCell->pPacket);
		pNextCell = pTmpCell->pNextCell;
		vPortFree(pTmpCell);
		pTmpCell = pNextCell;
	}

	pNode->pPSTxCell = NULL;

	if(cnt != 0)
		DBGPRINT(RT_WARN, ("[WARN]%s:Total %d packets in PS Queue....\n", __FUNCTION__, cnt));
}
#endif

VOID StartEapol(
	IN PRTMP_ADAPTER 	pAd, 
	IN PMGMTENTRY 		pEntry, 
	IN PUCHAR 			ptr, 
	IN UINT32 			frameSize, 
	IN BOOLEAN 			encrypt)
{
	PHEADER_802_11	pHeader_802_11;
	PTXINFO_STRUC  	pTXINFO;
	PTXWI_STRUC		pTXWI;
	PUCHAR			pData;
	UINT32			tmpLen = 0;
	UINT8			i, padLen;
	UINT32			len = 0;
	PUCHAR			tx_org;

	tmpLen = sizeof(TXINFO_STRUC) + sizeof(TXWI_STRUC) + sizeof(HEADER_802_11);
	
	int 			alloc_len = tmpLen + frameSize + 24;
	
	tx_org = (PUCHAR)pvPortMalloc(alloc_len);
	if (tx_org == NULL) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Cannot alloc memory!!\n", __FUNCTION__));
		return;
	}
	
	NdisZeroMemory(tx_org, alloc_len);

	pTXINFO = (PTXINFO_STRUC)tx_org;
	pTXWI	= (PTXWI_STRUC)(tx_org + sizeof(TXINFO_STRUC));
	pHeader_802_11 = (PHEADER_802_11)(tx_org + sizeof(TXINFO_STRUC) + sizeof(TXWI_STRUC));
	pData = tx_org + sizeof(TXINFO_STRUC) + sizeof(TXWI_STRUC) + sizeof(HEADER_802_11);


	/* Add LLC control 0xaa 0xaa 0x03 0x00 0x00 0x00 */
	pData[0] = pData[1] = 0xAA;
	pData[2] = 0x03;
	pData[3] = pData[4] = pData[5] = 0x0;
	len = 6;
	
	/* Add EAPOL cmd */
	NdisMoveMemory(pData + len, EAPOL, 2);
	len += 2;
	
	/* Add EAPOL data */	
	NdisMoveMemory(pData + len, ptr, frameSize);
	len += frameSize;

	tmpLen += len;
	
	BuildCommon802_11DataHeader(pAd, pHeader_802_11, pEntry->Addr, encrypt);
	RTMPWriteTxWI_Data(pAd, pTXWI, pEntry->Aid, tmpLen, MODE_CCK, MCS_RATE_1, 0);

	if ((tmpLen%4) == 0)
		padLen = 4;
	else 
		padLen = (4 + 4 - tmpLen%4);

	/* Add padding */
	for (i = 0; i < padLen; i++) pAd->TxContext.pDataBuff[tmpLen + i] = 0;

	tmpLen += padLen;

	RTMPWriteTxINFO_Data(pAd, pTXINFO, tmpLen-4, 0);

	RTUSB_EPOUThndl(pAd, 4, tx_org, tmpLen);

	vPortFree(tx_org);
}

VOID StartTx(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pDid, 
	IN PUCHAR 			ptr, 
	IN UINT32 			packetCount, 
	IN UINT32 			frameSize)
{
	PHEADER_802_11	pHeader_802_11;
	PTXINFO_STRUC  	pTXINFO;
	PTXWI_STRUC		pTXWI;
	UINT32			tmpLen = 0;
	UINT8			i, padLen;
	UINT8			*pLLC;
	PMGMTENTRY		pEntry;
	UINT8			wcid = 0;
	USHORT			type;

	NdisMoveMemory(pAd->DataAddr, ptr + 42, 6);

	pTXINFO = (PTXINFO_STRUC)ptr;
	pTXWI 	= (PTXWI_STRUC)(ptr + sizeof(TXINFO_STRUC));
	pHeader_802_11 = (PHEADER_802_11)(ptr + sizeof(TXINFO_STRUC) + sizeof(TXWI_STRUC));
	pLLC = (PUCHAR)(ptr + sizeof(TXINFO_STRUC) + sizeof(TXWI_STRUC) + sizeof(HEADER_802_11));
	LLCH_SET(pLLC);


	type = (pLLC[28] << 8) | pLLC[29];
	tmpLen = frameSize + 42;
	pEntry = MGMTENTRY_GetNode(pAd, pAd->DataAddr);
	if (!pEntry) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{	
			if (pAd->ApCfg.AuthMode >= Ndis802_11AuthModeWPA) {
				GET_GroupKey_WCID(pAd, wcid, BSS0);
			}
			else
				wcid = MCAST_WCID;
		}
		else 
		{
			wcid = MCAST_WCID;
		}
	}
	else 
	{
		wcid = pEntry->Aid;
		if (ProtectTypeMatch(type))
		{
			USHORT  waitPS = 0;

			while ((pEntry->bActive == FALSE) && (waitPS++) < 1000)
			{
				DBGPRINT(RT_TRACE, ("\nPS=%d  waitPS=%d\n", pEntry->bActive, waitPS));
				vTaskDelay(1);
			}
			if (waitPS >= 1000) DBGPRINT(RT_ERROR, ("[ERR]\nType=%d Packet Expired\n", type));
		}
	}
	
	BuildCommon802_11DataHeader(pAd, pHeader_802_11, pAd->DataAddr, pAd->WEPEnabled);

	if (ProtectTypeMatch(type))
		RTMPWriteTxWI_Data(pAd, pTXWI, wcid, tmpLen, MODE_CCK, MCS_RATE_2 , 1);
	else
	{
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pEntry = pAd->pFirstLinkNode;
		}

		if (pEntry)
			RTMPWriteTxWI_Data(pAd, pTXWI, wcid, tmpLen, pEntry->PhyMode, pEntry->CurrTxRate, pAd->SetTxFallback);
		else
			RTMPWriteTxWI_Data(pAd, pTXWI, wcid, tmpLen, pAd->SetTxPHYMode, pAd->SetTxPHYRate, pAd->SetTxFallback);
	}

	if ((tmpLen%4) == 0)
		padLen = 4;
	else 
		padLen = (4 + 4 - tmpLen%4);

	for (i = 0; i < padLen; i++) ptr[tmpLen+i] = 0;

	tmpLen += padLen;

	RTMPWriteTxINFO_Data(pAd, pTXINFO, tmpLen-4, 0);
	
#ifdef PS_TX_QUEUE_DATA			
	pEntry = MGMTENTRY_GetNode(pAd, pAd->DataAddr);

	if (pEntry)
	{
		if(pEntry->bActive == FALSE)  // Device in PS MODE
		{
			pBufferCell pTmpCell;

			pTmpCell = AllocateNewCell(pEntry, tmpLen);

			if(pTmpCell == NULL)
			{
				SendAllPSCell(pAd, pEntry);
				RTUSB_EPOUThndl(pAd, 4, ptr, tmpLen);
			}
			else
			{
				memcpy(pTmpCell->pPacket, ptr, tmpLen);
				pTmpCell->packetLen = tmpLen;
				pTmpCell->pNextCell = NULL;
			}
		}
		else // Device NOT in PS MODE
		{
			SendAllPSCell(pAd, pEntry);
			RTUSB_EPOUThndl(pAd, 4, ptr, tmpLen);
		}
	}
	else
#endif		
		RTUSB_EPOUThndl(pAd, 4, ptr, tmpLen);
}

UCHAR WiFi_Send(PUCHAR pDid, void *packet, UINT32 packetCount, UINT32 length)
{
	PUCHAR tx_packet, tx_org;
	UCHAR idx = 0;

	if (ppAd == NULL) return 0;
	if (WIFI_AP_Initial_Done() == FALSE) return 0;
	if (ppAd->ed_tx_stoped == TRUE) return 0;
	if (ppAd->USBPlugOut == TRUE) return 0;
	if (xMACSendMutex == NULL) return 0;
	
	if (xSemaphoreTake(xMACSendMutex, 1000/portTICK_RATE_MS) == pdFALSE){
		DBGPRINT(RT_ERROR, ("WiFi_mac_send mux time out\n"));
		return 0;
	}

	if ((ppAd->LinkState == LS_LINKED) || ppAd->OpMode == OPMODE_AP)
	{
		if (ppAd->APPowerSave)
		{
			RTMPSetPowerSuspend(ppAd, 0);
		}

		idx = txIndex++%MAX_TX_RING;
		tx_org = ppAd->tx_dma_pool[idx];
	
		NdisZeroMemory(tx_org, length+64);

		tx_packet = tx_org;
		NdisMoveMemory(tx_packet + 42, packet, length);

		StartTx(ppAd, 0, tx_packet,  1, length);

		ppAd->txTotalCount++;

		if (xMACSendMutex != NULL)
			xSemaphoreGive(xMACSendMutex);
		return 1;
	}
	if (xMACSendMutex != NULL)
		xSemaphoreGive(xMACSendMutex);
	return 0;
}



UCHAR ProtectTypeMatch(USHORT type)
{
	UCHAR i;
	
	if(ppAd->numTypeProtect)
	{
		for(i=0; i<ppAd->numTypeProtect; i++)
		{
			if(type==ppAd->valTypeProtect[i])
				return TRUE;
		}
	}

	return FALSE;
}



