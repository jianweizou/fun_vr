#include "nl80211.h"
#include "nl80211_hdr.h"
#include "common.h"
#include "tx.h"
#include "asic_ctrl.h"
#include "cipher.h"
#include "MT7601/mt7601_ctrl.h"
#include "mlme.h"
#include "wifi_api.h"

static UCHAR RALINK_OUI[]  	= {0x00, 0x0c, 0x43};
//static UCHAR BROADCOM_OUI[] = {0x00, 0x90, 0x4c};
//static UCHAR WPS_OUI[] 		= {0x00, 0x50, 0xf2, 0x04};


const UCHAR SsidIe 			= IE_SSID;
const UCHAR SupRateIe 		= IE_SUPP_RATES;
const UCHAR DsIe 			= IE_DS_PARM;
const UCHAR IbssIe 			= IE_IBSS_PARM;
const UCHAR ExtRateIe 		= IE_EXT_SUPP_RATES;
const UCHAR HtCapIe 		= IE_HT_CAP;
const UCHAR AddHtInfoIe 	= IE_ADD_HT;
const UCHAR RSNIe 			= IE_WPA;
const UCHAR RSNIe2 			= IE_WPA2;


VOID BuildCommon802_11MgtMacHeader(
	IN PRTMP_ADAPTER		pAd, 
	INOUT PHEADER_802_11 	pHdr80211, 
	IN UCHAR 				SubType, 
	IN UCHAR 				ToDs, 
	IN PUCHAR 				pDA, 
	IN PUCHAR 				pBssid)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	
	*(PUSHORT)pHdr80211 = (SubType << 4) | (ToDs << 8) ; 

	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
	COPY_MAC_ADDR(pHdr80211->Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
	
	pHdr80211->FC.MoreData = 1;
}


VOID BuildCommon802_11DataHeader(
	IN PRTMP_ADAPTER		pAd,
	INOUT PHEADER_802_11	pHeader80211,
	IN PUCHAR				pAddr,
	IN BOOLEAN				encrypt)
{
	PHEADER_802_11 pHeader_802_11;
	/* normal wlan header size : 24 octets */

	pHeader_802_11 = pHeader80211;

	NdisZeroMemory(pHeader_802_11, sizeof (HEADER_802_11));	
	
	pHeader_802_11->FC.FrDs = 0;
	pHeader_802_11->FC.ToDs = 0;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pHeader_802_11->FC.FrDs = 1;
	else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pHeader_802_11->FC.ToDs = 1;
		
	pHeader_802_11->FC.Type 	= BTYPE_DATA;
	pHeader_802_11->FC.SubType 	= SUBTYPE_DATA;
	pHeader_802_11->FC.Wep 	= encrypt;
	pHeader_802_11->FC.PwrMgmt 	= 0;
	pHeader_802_11->Sequence 	= pAd->DataSeq;
	pAd->DataSeq = (pAd->DataSeq + 1) & MAXSEQ;
	pHeader_802_11->Frag 		= 0;
	pHeader_802_11->FC.MoreData = 1 ; 

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		NdisMoveMemory(pHeader_802_11->Addr1, pAddr, MAC_ADDR_LEN);
		NdisMoveMemory(pHeader_802_11->Addr2, pAd->CurrentAddress, MAC_ADDR_LEN );
		NdisMoveMemory(pHeader_802_11->Addr3, pAd->essid, MAC_ADDR_LEN);
	}
	else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		NdisMoveMemory(pHeader_802_11->Addr3, pAddr, MAC_ADDR_LEN);
		NdisMoveMemory(pHeader_802_11->Addr2, pAd->CurrentAddress, MAC_ADDR_LEN );
		NdisMoveMemory(pHeader_802_11->Addr1, pAd->essid, MAC_ADDR_LEN);
	}
}


BOOLEAN MakeIbssBeacon(
	IN PRTMP_ADAPTER 	pAd,
	IN PUCHAR			pAVCmd,
	IN USHORT			AVCmdLen)
{
	UCHAR TIM[6] = { 0x5, 0x4, 0x0, 0x1, 0x1, 0xff };	/* FIXME: This is hard code */
	UCHAR DsLen = 1;
	UCHAR LocalErpIe[3] = { IE_ERP, 1, 0x0 };
	HEADER_802_11 BcnHdr;
	USHORT CapabilityInfo;
	LARGE_INTEGER FakeTimestamp;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI = &pAd->BeaconTxWI;  
	BOOLEAN Privacy = pAd->WEPEnabled;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen = 0;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen = 0;
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	UCHAR ubWifiDataGap = 0;
	PAP_ADMIN_CONFIG pApCfg = NULL; 

	pAd->BeaconBuf = pAd->TxMgmBase;

	if ((pAd->CommonCfg.PhyMode == PHY_11B)
	    && (pAd->CommonCfg.Channel <= 14)) 
	{
		SupRate[0] = 0x82;	/* 1 mbps */
		SupRate[1] = 0x84;	/* 2 mbps */
		SupRate[2] = 0x8b;	/* 5.5 mbps */
		SupRate[3] = 0x96;	/* 11 mbps */
		SupRateLen = 4;
		ExtRateLen = 0;
	} 
	else if (pAd->CommonCfg.Channel > 14) 
	{
		SupRate[0] = 0x8C;	/* 6 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
		SupRate[2] = 0x98;	/* 12 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
		SupRate[4] = 0xb0;	/* 24 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
		SupRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
		SupRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
		SupRateLen = 8;
		ExtRateLen = 0;
	} 
	else 
	{
		SupRate[0] = 0x82;	/* 1 mbps */
		SupRate[1] = 0x84;	/* 2 mbps */
		SupRate[2] = 0x8b;	/* 5.5 mbps */
		SupRate[3] = 0x96;	/* 11 mbps */
		SupRateLen = 4;

		ExtRate[0] = 0x0C;	/* 6 mbps, in units of 0.5 Mbps, */
		ExtRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
		ExtRate[2] = 0x18;	/* 12 mbps, in units of 0.5 Mbps, */
		ExtRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
		ExtRate[4] = 0x30;	/* 24 mbps, in units of 0.5 Mbps, */
		ExtRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
		ExtRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
		ExtRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
		ExtRateLen = 8;
	}
	pAd->StaActive.SupRateLen = SupRateLen;
	NdisMoveMemory(pAd->StaActive.SupRate, SupRate, SupRateLen);
	pAd->StaActive.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pAd->StaActive.ExtRate, ExtRate, ExtRateLen);

	/* compose IBSS beacon frame */
	BuildCommon802_11MgtMacHeader(pAd, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pAd->essid);
	
	CapabilityInfo = 
		CAP_GENERATE(1, 0, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);


	MakeOutgoingFrame(pAd->BeaconBuf, 			&FrameLen,
						sizeof(HEADER_802_11), 	&BcnHdr,
						TIME_STAMP_LEN, 		&FakeTimestamp,
						2, 						&pAd->beaconPeriod,
						2, 						&CapabilityInfo,
						1, 						&SsidIe,
						1, 						&pAd->SsidLen,
						pAd->SsidLen, 			pAd->Ssid,
						1, 						&SupRateIe,
						1, 						&SupRateLen,
						SupRateLen, 			SupRate,
						1, 						&DsIe,
						1, 						&DsLen,
						1, 						&pAd->channel,
						6, 						TIM,
			  			END_OF_ARGS);

	/* add ERP_IE and EXT_RAE IE of in 802.11g */
	if (ExtRateLen) 
	{
		ULONG tmp;
		MakeOutgoingFrame(pAd->BeaconBuf + FrameLen, 	&tmp,
							3, 							LocalErpIe,
							1, 							&ExtRateIe,
							1, 							&ExtRateLen,
							ExtRateLen, 				ExtRate,
							END_OF_ARGS);
		FrameLen += tmp;
	}

	pApCfg = &pAd->ApCfg;

	/* Append RSN_IE when  WPA OR WPAPSK, */
	if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
		(pApCfg->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pAd->BeaconBuf + FrameLen,	&TmpLen,
						  1,							&RSNIe,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  1,							&RSNIe2,
						  1,							&pApCfg->RSNIE_Len[1],
						  pApCfg->RSNIE_Len[1],			pApCfg->RSN_IE[1],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA) ||
		(pApCfg->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pAd->BeaconBuf + FrameLen,	&TmpLen,
						  1,							&RSNIe,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pApCfg->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pAd->BeaconBuf + FrameLen,	&TmpLen,
						  1,							&RSNIe2,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else {

	}
	
	if (pAVCmd != 0 && AVCmdLen != 0)
	{
		ULONG	ulTmpGap;
		UCHAR   RALINK_OUI[5]  = {0xDD, 0x0, 0x00, 0x0c, 0x43};
		UCHAR	ubTmpCharLen = 0;
 		
		ulTmpGap = FrameLen + sizeof(RALINK_OUI);
		if ((ubWifiDataGap > ulTmpGap) && ((ubWifiDataGap - ulTmpGap) > 0))
			ubTmpCharLen = ubWifiDataGap - ulTmpGap;
		else
			ubTmpCharLen = 0;
		
		RALINK_OUI[1] = AVCmdLen + 3 + ubTmpCharLen;		
		NdisMoveMemory(pAd->BeaconBuf + FrameLen, RALINK_OUI, sizeof(RALINK_OUI));
		FrameLen += sizeof(RALINK_OUI);
		if ((ubWifiDataGap > FrameLen) && ((ubWifiDataGap - FrameLen) > 0))
		{
			NdisFillMemory((pAd->BeaconBuf + FrameLen), (ubWifiDataGap - FrameLen), 0xAA);
			FrameLen += (ubWifiDataGap - FrameLen);
		}
		FrameLen += AVCmdLen;
	}

	/*beacon use reserved WCID 0xff */
	if (pAd->CommonCfg.Channel > 14) {
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						TRUE, 
						FALSE, 
						FALSE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_BEACON,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		FALSE,
			      		&pAd->CommonCfg.MlmeTransmit);
	} else {
		/* Set to use 1Mbps for Adhoc beacon. */
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						TRUE, 
						FALSE, 
						FALSE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_BEACON,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		FALSE, 
			      		&Transmit);
	}


	AsicEnableIbssSync(pAd);
	return FrameLen;
}



ULONG MakeOutgoingFrame(
	OUT PUCHAR 				pBuffer, 
	OUT PULONG 				pFrameLen, ...) 
{
	PUCHAR   p;
	int 	leng;
	ULONG	TotLeng;
	va_list Args;

	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, pFrameLen);
	do 
	{
		leng = va_arg(Args, int);
		if (leng == END_OF_ARGS) 
		{
			break;
		}
		p = va_arg(Args, PVOID);
		NdisMoveMemory(&pBuffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while(TRUE);

	va_end(Args); /* clean up */
	*pFrameLen = TotLeng;
	return TotLeng;
}



BOOLEAN SendIbssProbeReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PTX_CONTEXT 			pTxContext)
{	
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen = 0;
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


	NdisZeroMemory(pAd->TxMgmBase, 200);
	
	SupRate[0] = 0x82;	/* 1 mbps */
	SupRate[1] = 0x84;	/* 2 mbps */
	SupRate[2] = 0x8b;	/* 5.5 mbps */
	SupRate[3] = 0x96;	/* 11 mbps */
	SupRateLen = 4;
#if 0	
	ExtRate[0] = 0x0C;	/* 6 mbps, in units of 0.5 Mbps, */
	ExtRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
	ExtRate[2] = 0x18;	/* 12 mbps, in units of 0.5 Mbps, */
	ExtRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
	ExtRate[4] = 0x30;	/* 24 mbps, in units of 0.5 Mbps, */
	ExtRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
	ExtRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
	ExtRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
	ExtRateLen = 8;
#endif

	BuildCommon802_11MgtMacHeader(pAd, pTxContext->pWifiHeader, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
							BROADCAST_ADDR);
			
	/* this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse */
	MakeOutgoingFrame(pTxContext->pDataBuff, 		(PULONG)&pTxContext->DataLen,
						  1,                      	&SsidIe,
						  1,                       	&pAd->SsidLen,
						  pAd->SsidLen,		 		pAd->Ssid,
						  1,               			&SupRateIe,
						  1,                  		&SupRateLen,
						  SupRateLen,      			SupRate, 
						  END_OF_ARGS);

	pTxContext->DataLen += 24;
	
	RTMPWriteTxWI(pAd, 
					pTxContext->pTXWI, 
					FALSE, 
					FALSE, 
					FALSE, 
					FALSE, 
					FALSE,
					TRUE, 
					0, 
					0xff, 
					pTxContext->DataLen, 
					PID_MGMT, 
					PID_DATA_NORMALUCAST,
			 		RATE_1, 
			 		IFS_HTTXOP, 
			 		FALSE,
			      	&pAd->CommonCfg.MlmeTransmit);

	NdisZeroMemory((PUCHAR)pTxContext->pTXINF, sizeof(TXINFO_STRUC));
		
	RTMPWriteTxINFO(pTxContext, pTxContext->DataLen);
	RTMPKickOutTx(pAd, pTxContext);
	
	return TRUE;
}


BOOLEAN SendIbssProbeRsp(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext)
{
	UCHAR DsLen = 1;
	UCHAR LocalErpIe[3] = { IE_ERP, 1, 0x0 };
	HEADER_802_11 BcnHdr;
	USHORT CapabilityInfo;
	LARGE_INTEGER FakeTimestamp;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI = pTxContext->pTXWI;
	PUCHAR pRspFrame = (PUCHAR)pTxContext->pWifiHeader;
	BOOLEAN Privacy = pAd->WEPEnabled;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen = 0;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen = 0;
	PAP_ADMIN_CONFIG pApCfg = NULL; 

	NdisZeroMemory((PUCHAR)pTxContext->pTXINF, 200);

	if ((pAd->CommonCfg.PhyMode == PHY_11B)
	    && (pAd->CommonCfg.Channel <= 14)) 
	{
		SupRate[0] = 0x82;	/* 1 mbps */
		SupRate[1] = 0x84;	/* 2 mbps */
		SupRate[2] = 0x8b;	/* 5.5 mbps */
		SupRate[3] = 0x96;	/* 11 mbps */
		SupRateLen = 4;
		ExtRateLen = 0;
	} 
	else if (pAd->CommonCfg.Channel > 14) 
	{
		SupRate[0] = 0x8C;	/* 6 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
		SupRate[2] = 0x98;	/* 12 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
		SupRate[4] = 0xb0;	/* 24 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
		SupRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
		SupRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
		SupRateLen = 8;
		ExtRateLen = 0;
	} 
	else 
	{
		SupRate[0] = 0x82;	/* 1 mbps */
		SupRate[1] = 0x84;	/* 2 mbps */
		SupRate[2] = 0x8b;	/* 5.5 mbps */
		SupRate[3] = 0x96;	/* 11 mbps */
		SupRateLen = 4;

		ExtRate[0] = 0x0C;	/* 6 mbps, in units of 0.5 Mbps, */
		ExtRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
		ExtRate[2] = 0x18;	/* 12 mbps, in units of 0.5 Mbps, */
		ExtRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
		ExtRate[4] = 0x30;	/* 24 mbps, in units of 0.5 Mbps, */
		ExtRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
		ExtRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
		ExtRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
		ExtRateLen = 8;
	}

	pAd->StaActive.SupRateLen = SupRateLen;
	NdisMoveMemory(pAd->StaActive.SupRate, SupRate, SupRateLen);
	pAd->StaActive.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pAd->StaActive.ExtRate, ExtRate, ExtRateLen);

	/* compose IBSS beacon frame */
	BuildCommon802_11MgtMacHeader(pAd, &BcnHdr, SUBTYPE_PROBE_RSP, 0, pAddr, (UCHAR *)pAd->essid);

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		CapabilityInfo = 
			CAP_GENERATE(1, 0, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);
	else 
		CapabilityInfo = 
			CAP_GENERATE(0, 1, Privacy,(pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);


	MakeOutgoingFrame(pRspFrame, 				&FrameLen,
						sizeof (HEADER_802_11), &BcnHdr,
						TIME_STAMP_LEN, 		&FakeTimestamp,
						2, 						&pAd->beaconPeriod,
						2, 						&CapabilityInfo,
						1, 						&SsidIe,
						1, 						&pAd->SsidLen,
						pAd->SsidLen, 			pAd->Ssid,
						1, 						&SupRateIe,
						1, 						&SupRateLen,
						SupRateLen, 			SupRate,
						1, 						&DsIe,
						1, 						&DsLen,
						1, 						&pAd->channel,
						END_OF_ARGS);

	/* add ERP_IE and EXT_RAE IE of in 802.11g */
	if (ExtRateLen) {
		ULONG tmp;

		MakeOutgoingFrame(pRspFrame + FrameLen, &tmp,
							3, 					LocalErpIe,
							1, 					&ExtRateIe,
							1, 					&ExtRateLen,
							ExtRateLen, 		ExtRate, 
					END_OF_ARGS);
		FrameLen += tmp;
	}

	pApCfg = &pAd->ApCfg;

	/* Append RSN_IE when  WPA OR WPAPSK, */
	if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
		(pApCfg->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pRspFrame + FrameLen,			&TmpLen,
						  1,							&RSNIe,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  1,							&RSNIe2,
						  1,							&pApCfg->RSNIE_Len[1],
						  pApCfg->RSNIE_Len[1],			pApCfg->RSN_IE[1],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA) ||
		(pApCfg->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pRspFrame + FrameLen,			&TmpLen,
						  1,							&RSNIe,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pApCfg->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pRspFrame + FrameLen,			&TmpLen,
						  1,							&RSNIe2,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  END_OF_ARGS);
		FrameLen += TmpLen;

	}
	else {
		/* do nothing */
	}

	NdisZeroMemory((PUCHAR)pTxWI, sizeof(TXWI_STRUC));

	if (pAd->CommonCfg.Channel > 14) {
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						TRUE, 
						FALSE, 
						FALSE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_DATA_NORMALUCAST,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		FALSE,
			      		&pAd->CommonCfg.MlmeTransmit);
	} else {
		/* Set to use 1Mbps for Adhoc beacon. */
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						TRUE, 
						FALSE,
						FALSE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_DATA_NORMALUCAST,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		FALSE, 
			      		&Transmit);
	}

	RTMPWriteTxINFO(pTxContext, FrameLen);
	RTMPKickOutTx(pAd, pTxContext);

	return TRUE;
}


BOOLEAN SendAuthPacket(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN UCHAR				seq,
	IN USHORT				status)
{
	HEADER_802_11 AuthHdr;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI = pTxContext->pTXWI;
	PUCHAR pRspFrame = (PUCHAR)pTxContext->pWifiHeader;	
	AUTH_IE	auth_ie;

	NdisZeroMemory(pAd->TxMgmBase, 200);
	
	/* compose IBSS beacon frame */
	BuildCommon802_11MgtMacHeader(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pAddr, (UCHAR *)pAd->essid);

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (seq == 2)
		{	
			auth_ie.auth_alg = AUTH_MODE_KEY;
			auth_ie.auth_seq = 1;
		}
		else 
		{	auth_ie.auth_alg = AUTH_MODE_OPEN;
			auth_ie.auth_seq = seq;
		}
	}
	else
	{
		auth_ie.auth_alg = AUTH_MODE_OPEN;
		auth_ie.auth_seq = seq;
	}

	auth_ie.auth_status = status;

	MakeOutgoingFrame(pRspFrame, 					&FrameLen,
						sizeof(HEADER_802_11), 		&AuthHdr,
						sizeof(AUTH_IE), 			&auth_ie,	
						END_OF_ARGS);
			
	NdisZeroMemory((PUCHAR)pTxWI, sizeof(TXWI_STRUC));
	{
		/* Set to use 1Mbps for Adhoc beacon. */
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						FALSE, 
						FALSE,TRUE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_DATA_NORMALUCAST,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		FALSE, 
			      		&Transmit);
	
	}

	RTMPWriteTxINFO(pTxContext, FrameLen);
	RTMPKickOutTx(pAd, pTxContext);

	return FrameLen;
}


BOOLEAN SendAuthRspAtSeq2Action(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN PUCHAR				pData)
{

	HEADER_802_11 AuthHdr;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI = pTxContext->pTXWI;
	PUCHAR pAuthFrame = (PUCHAR)pTxContext->pWifiHeader;
	USHORT Seq, RemoteStatus, Alg; 
	ULONG c_len = 0;
	UCHAR ChlgText[CIPHER_TEXT_LEN];
	UCHAR CyperChlgText[CIPHER_TEXT_LEN + 8 + 8];
	UCHAR ChallengeIe = IE_CHALLENGE_TEXT;
	UCHAR len_challengeText = CIPHER_TEXT_LEN;
	
	NdisZeroMemory(pAd->TxMgmBase, 200);//sizeof(ppAd->TxMgmBase));

	Alg = cpu2le16(*(USHORT *) pData);
	Seq = cpu2le16(*(USHORT *) (pData+2));
	Seq++;
	RemoteStatus =cpu2le16(*(USHORT *) (pData+4));
	NdisMoveMemory(ChlgText, pData+8,  CIPHER_TEXT_LEN);
		
	MakeOutgoingFrame(CyperChlgText, 		&c_len,
						2, 					&Alg,
						2, 					&Seq,
						2, 					&RemoteStatus,
						1, 					&ChallengeIe,
						1, 					&len_challengeText,
						len_challengeText, 	ChlgText,
						END_OF_ARGS);	


	BuildCommon802_11MgtMacHeader(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pAddr, (PUCHAR)pAd->essid);
	
	AuthHdr.FC.Wep = 1;		
	
	MakeOutgoingFrame(pAuthFrame, 				&FrameLen,
						sizeof(HEADER_802_11), 	&AuthHdr,
//	  					LEN_WEP_IV_HDR, 		iv_hdr, 
						c_len, 					CyperChlgText,
						END_OF_ARGS);
							
	NdisZeroMemory((PUCHAR)pTxWI, sizeof(TXWI_STRUC));

	{

		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						FALSE, 
						FALSE,
						TRUE,
			      		TRUE, 
			      		1, 
			      		MCAST_WCID, 
			      		FrameLen, 
			      		PID_MGMT, 
						PID_DATA_NORMALUCAST,
			      		RATE_1, 
			      		IFS_HTTXOP,
			      		FALSE, 
			      		&Transmit);
	}	
	RTMPWriteTxINFO(pTxContext, FrameLen);
	RTMPKickOutTx(pAd, pTxContext);
	
	return TRUE;			

}


BOOLEAN SendAssocReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext)
{
	HEADER_802_11 AssocHdr;
	USHORT ListenIntv = 0xA;
	USHORT CapabilityInfo = 0x0001;
	PUCHAR pOutBuffer =  (PUCHAR)pTxContext->pWifiHeader;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen = 0;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen = 0;

	NdisZeroMemory(pAd->TxMgmBase, 200);//sizeof(ppAd->TxMgmBase));

	pTxWI = pTxContext->pTXWI;

	SupRate[0] = 0x82;	/* 1 mbps */
	SupRate[1] = 0x84;	/* 2 mbps */
	SupRate[2] = 0x8b;	/* 5.5 mbps */
	SupRate[3] = 0x96;	/* 11 mbps */
	SupRateLen = 4;

	ExtRate[0] = 0x0C;	/* 6 mbps, in units of 0.5 Mbps, */
	ExtRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
	ExtRate[2] = 0x18;	/* 12 mbps, in units of 0.5 Mbps, */
	ExtRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
	ExtRate[4] = 0x30;	/* 24 mbps, in units of 0.5 Mbps, */
	ExtRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
	ExtRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
	ExtRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
	ExtRateLen = 8;

	BuildCommon802_11MgtMacHeader(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0,  pAddr, (PUCHAR)pAd->essid);
	
	/* Build basic frame first */
	MakeOutgoingFrame(pOutBuffer, 				&FrameLen,
						sizeof (HEADER_802_11), &AssocHdr,
						  2, 					&CapabilityInfo,
						  2, 					&ListenIntv,
						  1, 					&SsidIe,
						  1, 					&pAd->SsidLen,
						  pAd->SsidLen, 		pAd->Ssid,
						  1, 					&SupRateIe,
						  1, 					&SupRateLen,
						  SupRateLen, 			SupRate,
						  1, 					&ExtRateIe,
						  1, 					&ExtRateLen,
						  ExtRateLen,  			ExtRate,
						  END_OF_ARGS);


	/* */
	/* Let WPA(#221) Element ID on the end of this association frame. */
	/* Otherwise some AP will fail on parsing Element ID and set status fail on Assoc Rsp. */
	/* For example: Put Vendor Specific IE on the front of WPA IE. */
	/* This happens on AP (Model No:Linksys WRK54G) */
	/* */
	if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
	     (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
	     (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
	     (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2))) 
	{
		ULONG tmplen = 0;
		UCHAR RSNIe = IE_WPA;

		if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) || 
			(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)) 
		{
			RSNIe = IE_WPA2;
		}

		RTMPMakeRSNIE(pAd, pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus, BSS0);

		DBGPRINT(RT_INFO, ("[INFO]%s: Using %s - Authmode = %d, RSN len = %d\r\n", 
							__FUNCTION__, (RSNIe == IE_WPA2)?("WPA2"):("WPA"), pAd->StaCfg.AuthMode, pAd->StaCfg.RSNIE_Len));

		

		MakeOutgoingFrame(pOutBuffer + FrameLen, 	&tmplen,
								  1, 				&RSNIe,
								  1, 				&pAd->StaCfg.RSNIE_Len,
							pAd->StaCfg.RSNIE_Len,	pAd->StaCfg.RSN_IE,
							END_OF_ARGS);
		
		FrameLen += tmplen;
	}
	
	{
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;

		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						FALSE, 
						FALSE,
						TRUE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_DATA_NORMALUCAST,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		TRUE, 
			      		&Transmit);

		RTMPWriteTxINFO(pTxContext, FrameLen);
		RTMPKickOutTx(pAd, pTxContext);
	}
	return TRUE;
}


BOOLEAN SendAssociateRsp(
	IN PRTMP_ADAPTER 		pAd,
	IN PMGMTENTRY			pEntry,
	IN PTX_CONTEXT 			pTxContext,
	IN USHORT 				StatusCode)
{
	HEADER_802_11 AssocRspHdr;
	USHORT Association_ID = 0;
	USHORT CapabilityInfo = 0x0001;
	PUCHAR pOutBuffer =  (PUCHAR)pTxContext->pWifiHeader;
	ULONG FrameLen = 0;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen = 0;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen = 0;
	PAP_ADMIN_CONFIG pApCfg = &pAd->ApCfg;

	NdisZeroMemory(pAd->TxMgmBase, 200);
	
	PTXWI_STRUC pTxWI = pTxContext->pTXWI;

	SupRate[0] = 0x82;	/* 1 mbps */
	SupRate[1] = 0x84;	/* 2 mbps */
	SupRate[2] = 0x8b;	/* 5.5 mbps */
	SupRate[3] = 0x96;	/* 11 mbps */
	SupRateLen = 4;

	ExtRate[0] = 0x0C;	/* 6 mbps, in units of 0.5 Mbps, */
	ExtRate[1] = 0x12;	/* 9 mbps, in units of 0.5 Mbps */
	ExtRate[2] = 0x18;	/* 12 mbps, in units of 0.5 Mbps, */
	ExtRate[3] = 0x24;	/* 18 mbps, in units of 0.5 Mbps */
	ExtRate[4] = 0x30;	/* 24 mbps, in units of 0.5 Mbps, */
	ExtRate[5] = 0x48;	/* 36 mbps, in units of 0.5 Mbps */
	ExtRate[6] = 0x60;	/* 48 mbps, in units of 0.5 Mbps */
	ExtRate[7] = 0x6c;	/* 54 mbps, in units of 0.5 Mbps */
	ExtRateLen = 8;

	Association_ID = pEntry->Aid | 0xc000;

	DBGPRINT(RT_LOUD, ("[LOUD]%s: Assign New AID for Associate resp %d\r\n", 
						__FUNCTION__, Association_ID));

	BuildCommon802_11MgtMacHeader(pAd, &AssocRspHdr, SUBTYPE_ASSOC_RSP, 0,  pEntry->Addr, pAd->essid);

	/* Build basic frame first */
	MakeOutgoingFrame(pOutBuffer, 				&FrameLen,
						sizeof(HEADER_802_11), 	&AssocRspHdr,
						2, 						&CapabilityInfo,
						2,						&StatusCode,
						2,						&Association_ID,
						1, 						&SupRateIe,
						1, 						&SupRateLen,
						SupRateLen, 			SupRate,
						1, 						&ExtRateIe,
						1, 						&ExtRateLen,
						ExtRateLen,  			ExtRate,
				  		END_OF_ARGS);

	/* Append RSN_IE when WPA OR WPAPSK, */
	if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
		(pApCfg->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TmpLen,
						  1,							&RSNIe,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  1,							&RSNIe2,
						  1,							&pApCfg->RSNIE_Len[1],
						  pApCfg->RSNIE_Len[1],			pApCfg->RSN_IE[1],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA) ||
		(pApCfg->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TmpLen,
						  1,							&RSNIe,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if ((pApCfg->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pApCfg->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TmpLen,
						  1,							&RSNIe2,
						  1,							&pApCfg->RSNIE_Len[0],
						  pApCfg->RSNIE_Len[0],			pApCfg->RSN_IE[0],
						  END_OF_ARGS);
		FrameLen += TmpLen;

	}
	else {
		/* do nothing */
	}


	{
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;

		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						FALSE, 
						FALSE, 
						TRUE,
			      		TRUE, 
			      		0, 
			      		0xff, 
			      		FrameLen, 
			      		PID_MGMT, 
			      		PID_DATA_NORMALUCAST,
			      		RATE_1, 
			      		IFS_HTTXOP, 
			      		FALSE, 
			      		&Transmit);
		
		RTMPWriteTxINFO(pTxContext, FrameLen);
		RTMPKickOutTx(pAd, pTxContext);
	}
	return TRUE;
}


BOOLEAN SendDeAuthReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN UCHAR 				reason)
{
	HEADER_802_11 DeauthHdr;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI = pTxContext->pTXWI;
	PUCHAR pOutBuffer = (PUCHAR)pTxContext->pWifiHeader;
	UINT16 reasonCode;

	NdisZeroMemory(pAd->TxMgmBase, 200);
	
	BuildCommon802_11MgtMacHeader(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAddr, (UCHAR *)pAd->essid);

	reasonCode = reason;

	MakeOutgoingFrame(pOutBuffer, 					&FrameLen, 
						sizeof (HEADER_802_11), 	&DeauthHdr, 
							2, 						&reasonCode, 
						END_OF_ARGS);

	NdisZeroMemory((PUCHAR)pTxWI, sizeof(TXWI_STRUC));

	{
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE, 
						FALSE, 
						FALSE,
						FALSE,
				  		TRUE, 
				  		0, 
				  		0xff, 
				  		FrameLen, 
				  		PID_MGMT, 
				  		PID_DATA_NORMALUCAST,
				  		RATE_1, 
				  		IFS_HTTXOP, 
				  		FALSE, 
				  		&Transmit);
	}
	
	RTMPWriteTxINFO(pTxContext, FrameLen);
	RTMPKickOutTx(pAd, pTxContext);

	return TRUE;
}


BOOLEAN SendDisassocReq(
	IN PRTMP_ADAPTER 	pAd,
	IN PUCHAR			pAddr,
	IN PTX_CONTEXT 		pTxContext,
	IN UCHAR 			reason)
{
	HEADER_802_11 DisassocHdr;
	PHEADER_802_11 pDisassocHdr;
	PUCHAR pOutBuffer = (PUCHAR)pTxContext->pWifiHeader;
	ULONG FrameLen = 0;
	PTXWI_STRUC pTxWI = pTxContext->pTXWI;	
	USHORT reasonCode;

	NdisZeroMemory(pAd->TxMgmBase, 200);
	
	BuildCommon802_11MgtMacHeader(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pAddr, pAd->essid);

	reasonCode = reason;
	
	MakeOutgoingFrame(pOutBuffer, 				&FrameLen,  
						sizeof(HEADER_802_11), 	&DisassocHdr, 
						2, 						&reasonCode, 
						END_OF_ARGS);

	NdisZeroMemory((PUCHAR)pTxWI, sizeof(TXWI_STRUC));

	{
		HTTRANSMIT_SETTING Transmit;
		Transmit.word = 0;
		RTMPWriteTxWI(pAd, 
						pTxWI, 
						FALSE, 
						FALSE,
						FALSE, 
						FALSE,
						FALSE,
				  		TRUE, 
				  		0, 
				  		0xff, 
				  		FrameLen, 
				  		PID_MGMT, 
				  		PID_DATA_NORMALUCAST,
				 	 	RATE_1, 
				 	 	IFS_HTTXOP, 
				 	 	FALSE, 
				 	 	&Transmit);
	}

	RTMPWriteTxINFO(pTxContext, FrameLen);
	RTMPKickOutTx(pAd, pTxContext);

	/* To patch Instance and Buffalo(N) AP */
	/* Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine */
	/* Therefore, we send both of them. */
	pDisassocHdr = (PHEADER_802_11) pOutBuffer;
	
	pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;

	RTMPKickOutTx(pAd, pTxContext);

	return TRUE;
}



BOOLEAN PeerAssocReqCmmSanity(
	IN PRTMP_ADAPTER		pAd,
	IN PHEADER_802_11		pHdr80211,
	BOOLEAN 				isReassoc,
	INT 					MsgLen,
	PIE_LISTS				pie_lists)
{
	PUCHAR			pData = NULL;
    PEID_STRUCT		eid_ptr;
    UCHAR			Sanity = 0;
    UCHAR			WPA1_OUI[4] = { 0x00, 0x50, 0xF2, 0x01 };
    UCHAR			WPA2_OUI[3] = { 0x00, 0x0F, 0xAC };

	pData = (PUCHAR)((PUCHAR)pHdr80211 + sizeof(HEADER_802_11));

	COPY_MAC_ADDR(&pie_lists->Addr2[0], pHdr80211->Addr2);

	NdisMoveMemory(&pie_lists->CapabilityInfo, pData, 2);
	pData += 2;
	NdisMoveMemory(&pie_lists->ListenInterval, pData, 2);
	pData += 2;

	if (isReassoc) 
	{
		NdisMoveMemory(&pie_lists->ApAddr[0], pData, 6);
		pData += 6;
		eid_ptr = (PEID_STRUCT) pData;
	}
	else
	{
		eid_ptr = (PEID_STRUCT) pData;
	}

    /* get variable fields from payload and advance the pointer */
    while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)pHdr80211 + MsgLen))
    {
        switch(eid_ptr->Eid)
        {
            case IE_SSID:
			if (((Sanity&0x1) == 1))
				break;

                if ((eid_ptr->Len <= MAX_LEN_OF_SSID))
                {
                    Sanity |= 0x01;
                    NdisMoveMemory(&pie_lists->Ssid[0], eid_ptr->Octet, eid_ptr->Len);
                    pie_lists->SsidLen = eid_ptr->Len;
                    DBGPRINT(RT_LOUD, ("[LOUD]%s: - SsidLen = %d  \r\n", __FUNCTION__, pie_lists->SsidLen));
                }
                else
                {
                    DBGPRINT(RT_TRACE, ("TRACE]%s: - wrong IE_SSID\r\n", __FUNCTION__));
                    return FALSE;
                }
                break;

            case IE_SUPP_RATES:
                if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) &&
					(eid_ptr->Len > 0))
                {
                    Sanity |= 0x02;
                    NdisMoveMemory(&pie_lists->SupportedRates[0], eid_ptr->Octet, eid_ptr->Len);

                    DBGPRINT(RT_LOUD, ("[LOUD]%s: - IE_SUPP_RATES., Len=%d. " "Rates[0]=%x\r\n", 
										__FUNCTION__, eid_ptr->Len, pie_lists->SupportedRates[0]));
                    DBGPRINT(RT_LOUD, ("[LOUD]%s: - Rates[1]=%x %x %x %x %x %x %x\r\n",
										__FUNCTION__,
										pie_lists->SupportedRates[1], pie_lists->SupportedRates[2],
										pie_lists->SupportedRates[3], pie_lists->SupportedRates[4],
										pie_lists->SupportedRates[5], pie_lists->SupportedRates[6],
										pie_lists->SupportedRates[7]));

                    pie_lists->SupportedRatesLen = eid_ptr->Len;
                }
                else
                {
					UCHAR RateDefault[8] = \
							{ 0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c };

                	/* HT rate not ready yet. return true temporarily. rt2860c */
                    /*DBGPRINT(RT_TRACE, ("PeerAssocReqSanity - wrong IE_SUPP_RATES\r\n")); */
                    Sanity |= 0x02;
                    pie_lists->SupportedRatesLen = 8;
					NdisMoveMemory(&pie_lists->SupportedRates[0], RateDefault, 8);

                    DBGPRINT(RT_TRACE, ("[TRACE]%s: - wrong IE_SUPP_RATES., Len=%d\r\n",
										__FUNCTION__, eid_ptr->Len));
                }
                break;

            case IE_EXT_SUPP_RATES:
                if (eid_ptr->Len + pie_lists->SupportedRatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(&pie_lists->SupportedRates[pie_lists->SupportedRatesLen], eid_ptr->Octet,
									eid_ptr->Len);
                    pie_lists->SupportedRatesLen += eid_ptr->Len;
                }
                else
                {
                    NdisMoveMemory(&pie_lists->SupportedRates[pie_lists->SupportedRatesLen], eid_ptr->Octet,
									MAX_LEN_OF_SUPPORTED_RATES - (pie_lists->SupportedRatesLen));
                    pie_lists->SupportedRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
                break;

            case IE_WPA:    /* same as IE_VENDOR_SPECIFIC */
            case IE_WPA2:

                if (NdisEqualMemory(eid_ptr->Octet, RALINK_OUI, 3) && (eid_ptr->Len == 7))
                {
					if (eid_ptr->Octet[3] != 0)
   						pie_lists->RalinkIe = eid_ptr->Octet[3];
        			else
        				pie_lists->RalinkIe = 0xf0000000; /* Set to non-zero value (can't set bit0-2) to represent this is Ralink Chip. So at linkup, we will set ralinkchip flag. */
                    break;
                }
                
                if (pAd->ApCfg.AuthMode < Ndis802_11AuthModeWPA)
                    break;
                
                /* 	If this IE did not begins with 00:0x50:0xf2:0x01,  
                	it would be proprietary. So we ignore it. */
                if (!NdisEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
                    && !NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI)))
                {
                    DBGPRINT(RT_TRACE, ("[TRACE]%s: Not RSN IE, maybe WMM IE!!!\r\n", __FUNCTION__));
                    break;                          
                }
                
                if (/*(eid_ptr->Len <= MAX_LEN_OF_RSNIE) &&*/ (eid_ptr->Len >= MIN_LEN_OF_RSNIE))
                {
					//hex_dump("Received RSNIE in Assoc-Req", (UCHAR *)eid_ptr, eid_ptr->Len + 2);
                    
					/* Copy whole RSNIE context */
                    NdisMoveMemory(&pie_lists->RSN_IE[0], eid_ptr, eid_ptr->Len + 2);
					pie_lists->RSNIE_Len = eid_ptr->Len + 2;
                }
                else
                {
                    pie_lists->RSNIE_Len = 0;
                    DBGPRINT(RT_TRACE, ("[TRACE]%s - missing IE_WPA(%d)\r\n", __FUNCTION__, eid_ptr->Len));
                    return FALSE;
                }               
                break;
				
            default:
                break;
        }

        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
    }

	if ((Sanity & 0x3) != 0x03)	 
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: - missing mandatory field\r\n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		DBGPRINT(RT_LOUD, ("[LOUD]%s: - success\r\n", __FUNCTION__));
		return TRUE;
	}
}



BOOLEAN PeerAuthSanity(
    IN PRTMP_ADAPTER 		pAd, 
    IN PHEADER_802_11		pHdr80211,
    IN ULONG 				MsgLen,
    IN PUSHORT				pAlg) 
{
	PUCHAR pData = NULL;
	USHORT Alg, Seq, Status;

	if (MAC_ADDR_EQUAL(pHdr80211->Addr1, pAd->CurrentAddress) == 0)
		return FALSE;

	pData = (PUCHAR)((PUCHAR)pHdr80211 + sizeof(HEADER_802_11));

    NdisMoveMemory((PUCHAR)&Alg,    pData, 2);
	pData += 2;
    NdisMoveMemory((PUCHAR)&Seq,    pData, 2);
	pData += 2;
    NdisMoveMemory((PUCHAR)&Status, pData, 2);
	pData += 2;

	DBGPRINT(RT_LOUD, ("[LOUD]%s: Alg = %d, Seq = %d, Status = %d \r\n",
						__FUNCTION__, Alg, Seq, Status));

	*pAlg = Alg;
	
    if (Alg == AUTH_MODE_OPEN) 
    {
        if (Seq == 1 || Seq == 2) 
        {
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_ERROR, ("[ERR]%s: fail - wrong Seg# (=%d)\r\n", __FUNCTION__, Seq));
            return FALSE;
        }
    } 
    else if (Alg == AUTH_MODE_KEY) 
    {
        if (Seq == 1 || Seq == 4) 
        {
            return TRUE;
        } 
        else if (Seq == 2 || Seq == 3) 
        {
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_ERROR, ("[ERR]%s: fail - wrong Seg# (=%d)\r\n", __FUNCTION__, Seq));
            return FALSE;
        }
    } 
    else 
    {
        DBGPRINT(RT_ERROR, ("[ERR]%s: fail - wrong algorithm (=%d)\r\n", __FUNCTION__, Alg));
        return FALSE;
    }

	//return TRUE;
}


NDIS_STATUS APCheckRxError(
	IN PRTMP_ADAPTER 		pAd,
	IN PRXINFO_STRUC 		pRxInfo,
	IN UCHAR 				Wcid)
{
	if (pRxInfo->Crc || pRxInfo->CipherErr)
	{
		/*
			WCID equal to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
			The incoming packet mays come from WDS or AP-Client link.
			We need them for further process. Can't drop the packet here.
		*/
		if ((pRxInfo->U2M) && (pRxInfo->CipherErr) && (Wcid == 255))
		{
			/* pass those packet for further process. */
			return NDIS_SUCCESS;
		}
		else
		{
			DBGPRINT(RT_WARN, ("[WARN]%s(): pRxInfo:Crc=%d, CipherErr=%d, U2M=%d, Wcid=%d\r\n", 
								__FUNCTION__, pRxInfo->Crc, pRxInfo->CipherErr, pRxInfo->U2M, Wcid));
			return NDIS_FAILURE;
		}
	}
	else
	{
		return NDIS_SUCCESS;
	}
}


VOID APRxDErrorHandle(
	IN PRTMP_ADAPTER 		pAd, 
	IN PRXINFO_STRUC 		pRxInfo, 
	IN PRXWI_STRUC 			pRxWI)
{
	PMGMTENTRY pEntry;
/*	
	if (pRxInfo->CipherErr)
		INC_COUNTER64(pAd->WlanCounters.WEPUndecryptableCount);
*/
	if (pRxInfo->U2M && pRxInfo->CipherErr)
	{		
		if (pRxWI->wcid < MAX_LEN_OF_MAC_TABLE)
		{
			pEntry = MGMTENTRY_SearchNodeByWCID(pAd, pRxWI->wcid);
			/*
				MIC error
				Before verifying the MIC, the receiver shall check FCS, ICV and TSC.
				This avoids unnecessary MIC failure events.
			*/
			if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) && (pRxInfo->CipherErr == 2))
			{
				HandleCounterMeasure(pAd, pEntry);
			}
		}
		DBGPRINT(RT_TRACE, ("[TRACE]%s: Rx u2me Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\r\n", 
							__FUNCTION__, pRxWI->MPDUtotalByteCnt, pRxWI->wcid, pRxInfo->CipherErr));
	}

	// pAd->Counters8023.RxErrors++;
	DBGPRINT(RT_TRACE, ("[TRACE]<---%s\r\n", __FUNCTION__));
}


NDIS_STATUS	STACheckRxError(
	IN PRTMP_ADAPTER 		pAd,
	IN PHEADER_802_11 		pHeader,	
	IN PRXWI_STRUC 			pRxWI,	
	IN PRXINFO_STRUC 		pRxInfo)
{	
	PCIPHER_KEY pWpaKey;
	INT dBm;
	
	if (pRxInfo == NULL)
		return(NDIS_FAILURE);

	/* Phy errors & CRC errors*/
	if (pRxInfo->Crc)
	{
		/* Check RSSI for Noise Hist statistic collection.*/
		dBm = (INT) (pRxWI->RSSI0) - pAd->BbpRssiToDbmDelta;

		DBGPRINT(RT_TRACE, ("[TRACE]%s: dBm = %d\r\n", __FUNCTION__, dBm));
		
		if (dBm <= -87)
			pAd->StaCfg.RPIDensity[0] += 1;
		else if (dBm <= -82)
			pAd->StaCfg.RPIDensity[1] += 1;
		else if (dBm <= -77)
			pAd->StaCfg.RPIDensity[2] += 1;
		else if (dBm <= -72)
			pAd->StaCfg.RPIDensity[3] += 1;
		else if (dBm <= -67)
			pAd->StaCfg.RPIDensity[4] += 1;
		else if (dBm <= -62)
			pAd->StaCfg.RPIDensity[5] += 1;
		else if (dBm <= -57)
			pAd->StaCfg.RPIDensity[6] += 1;
		else if (dBm > -57)
			pAd->StaCfg.RPIDensity[7] += 1;
		
		return(NDIS_FAILURE);
	}

	/* Add Rx size to channel load counter, we should ignore error counts*/
//	pAd->StaCfg.CLBusyBytes += (pRxWI->MPDUtotalByteCnt + 14);

	/* Paul 04-03 for OFDM Rx length issue*/
	if (pRxWI->MPDUtotalByteCnt > MAX_AGGREGATION_SIZE)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: received packet too long\r\n", __FUNCTION__));
		return NDIS_FAILURE;
	}

	/* Drop not U2M frames, cant's drop here because we will drop beacon in this case*/
	/* I am kind of doubting the U2M bit operation*/
	/* if (pRxD->U2M == 0)*/
	/*	return(NDIS_STATUS_FAILURE);*/

	/* drop decyption fail frame*/
	if (pRxInfo->Decrypted && pRxInfo->CipherErr)
	{
#if 0						
		if (((pRxInfo->CipherErr & 1) == 1) && INFRA_ON(pAd))
            RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);			

		if (((pRxInfo->CipherErr & 2) == 2) && INFRA_ON(pAd))
			RTMPSendWirelessEvent(pAd, IW_MIC_ERROR_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
#endif
		pWpaKey = &pAd->SharedKey[BSS0][pRxWI->key_idx];


		DBGPRINT(RT_TRACE,("[TRACE]%s: CipherErr = %d, MyBss = %d, key_idx = %d, key_type = %s\r\n",
							__FUNCTION__, pRxInfo->CipherErr, pRxInfo->MyBss, pRxWI->key_idx, (pWpaKey->Type == PAIRWISEKEY)?("Pair-Key"):("Group-Key")));
		/* MIC Error*/
		if ((pRxInfo->CipherErr == 2) && pRxInfo->MyBss)
		{
			DBGPRINT(RT_ERROR,("[ERR]%s: Rx MIC Value error in KeyIdx = %d (%s)\r\n", 
								__FUNCTION__, pRxWI->key_idx, (pWpaKey->Type == PAIRWISEKEY)?("Pair-Key"):("Group-Key")));
		}
		
		if (pRxInfo->Decrypted && (pWpaKey->CipherAlg == CIPHER_AES)) //&& (pHeader->Sequence == pAd->FragFrame.Sequence))
		{
			/* Acceptable since the First FragFrame no CipherErr problem.*/
			return(NDIS_SUCCESS);
		}

		return(NDIS_FAILURE);
	}

	return(NDIS_SUCCESS);
}

BOOLEAN RTMPCheckBeacon(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN USHORT 			len)
{
	UCHAR ie, ieLen;
	
	do
	{
		ie = pData[0];
		ieLen = pData[1];

		if (ie == IE_SSID)
		{
			if (ieLen == pAd->SsidLen)
			{
				if (NdisCmpMemory(pAd->Ssid, pData + 2, ieLen) == 0)
					return TRUE;
				else return FALSE;
			}
			else if (ieLen == 0)
			{
				/* cannot find specified SSID */
				return FALSE;
			}
			else return FALSE;
		}
		len = len - 2 - ieLen;
		pData = pData + 2 + ieLen;
	} while (len > 2);

	return FALSE;
}

BOOLEAN RTMPCheckAssociation(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData)
{
	USHORT	status = *(USHORT *)&pData[2];	

	if (status == 0)
	{
		DBGPRINT(RT_INFO, ("[INFO]%s: Connected to AP = %02X-%02X-%02X-%02X-%02X-%02X, channel (%d)\r\n",
							__FUNCTION__, PRINT_MAC(pAd->MgmAddr), pAd->channel));
		return TRUE;
	}
	else 
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Fail to Connect to AP = %d\r\n", 
							__FUNCTION__, status));
	}
	
	return FALSE;
}

UCHAR RTMPGetAPChannel(
	IN PUCHAR 		pData, 
	IN USHORT 		len)
{	
	UCHAR ie, ieLen;
	
	do
	{
		ie = pData[0];
		ieLen = pData[1];

		if (ie == IE_DS_PARM)
			return pData[2];		

		pData += (2 + ieLen);
		len -= (2 + ieLen);
	
	} while (len >= 2);

	return 0;
}


VOID ParseBeacon(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN USHORT 			len)
{
	int i = 0;
	UCHAR ie, ieLen;
	PHEADER_802_11 pWiFiHeader;
	NDIS_802_11_AUTHENTICATION_MODE auth; 
	NDIS_802_11_ENCRYPTION_MODE uenc;
	NDIS_802_11_ENCRYPTION_MODE menc;
	PUSHORT CapabilityInfo;

	pWiFiHeader = (PHEADER_802_11)pData;
	pData += (sizeof(HEADER_802_11) + TIME_STAMP_LEN);
	CapabilityInfo = (PUSHORT)(pData + INTERVAL_LEN);
	
	if (pAd->bss_table.BssNr < MAX_LEN_OF_BSS_TABLE)
	{
		if (pAd->bss_table.BssNr)
		{
			for (i = 0; i < pAd->bss_table.BssNr; i++)
			{
				if (NdisCmpMemory(pAd->bss_table.BssEntry[i].Bssid, pWiFiHeader->Addr3, 6) == 0)
					goto _exit;
			}
		}
		NdisMoveMemory(pAd->bss_table.BssEntry[i].Bssid, pWiFiHeader->Addr3, 6);
		pAd->bss_table.BssEntry[i].BeaconPeriod = *(U16 *)pData;
		pData += INTERVAL_LEN;
		pAd->bss_table.BssEntry[i].CapabilityInfo = *(U16 *)pData;		
		pAd->bss_table.BssEntry[i].BssType = *(U16 *)pData & 0x3;
		pData += CAPABILITY_INFO_LEN;
		do
		{
			ie = pData[0];
			ieLen = pData[1];
			pData += 2;			
			switch(ie)
			{
				case IE_SSID:

					if (pAd->SsidLen != 0) 
					{
						if ((pAd->SsidLen == ieLen) && (NdisCmpMemory(pAd->Ssid, pData, pAd->SsidLen) == 0)) 
						{
							DBGPRINT(RT_TRACE, ("[TRACE]%s: Target AP Found...\r\n", __FUNCTION__));	
							RTMPGetCipherInfo(pData + ieLen, len - 2 - ieLen, (PUCHAR)&auth, (PUCHAR)&uenc, (PUCHAR)&menc);
							if ((*CapabilityInfo & 0x0010) == 0)
							{
								pAd->LinkState = LS_SCAN_BSSID;
								goto _exit;
							}

							if (auth != 0 && uenc == 0) 
							{
								goto _exit;
							}
							
							/* FIXME: */
							if (NdisCmpMemory(pWiFiHeader->Addr3, pAd->essid, 6) != 0)
							{
								DBGPRINT(RT_ERROR, ("[ERR]%s: Storend Essid differs from the one of AP\r\n", __FUNCTION__));
							}
							
							NdisMoveMemory(pAd->MgmAddr, pWiFiHeader->Addr3, 6);
							NdisMoveMemory(pAd->DataAddr, pWiFiHeader->Addr3, 6);
							NdisMoveMemory(pAd->essid, pWiFiHeader->Addr3, 6);
							
							AsicSetBssid(pAd, pAd->essid);							

							pAd->bss_table.BssEntry[i].SsidLen = ieLen;
							NdisZeroMemory(pAd->bss_table.BssEntry[i].Ssid, ieLen);
							NdisMoveMemory(pAd->bss_table.BssEntry[i].Ssid, pData, ieLen);
							pAd->bss_table.BssNr++;
							DBGPRINT(RT_INFO, ("[INFO]%s: Found matched SSID (%s)\r\n",
												__FUNCTION__, pAd->bss_table.BssEntry[i].Ssid));

							MGMTENTRY_Add(pAd, pWiFiHeader->Addr2);

							MLME_DoConnect(pAd);
						} 
						else
						{
							goto _exit;
						}
					} 
					else 
					{
						SendIbssProbeReq(pAd, &pAd->TxContext);
					}
				break;

				case IE_DS_PARM:
					pAd->bss_table.BssEntry[i].CentralChannel = *pData;
					pAd->bss_table.BssEntry[i].Channel = *pData;
					if (pAd->channel != *pData)
					{	
						pAd->channel = *pData;
						MT7601_ChipSwitchChannel(pAd, pAd->channel, 0);
					}
					break;

				case IE_WPA:
					if (pData[0] == 0x00 && pData[1] == 0x50 && pData[2] == 0xF2 && pData[3] == 0x01)
					{
						/* FIXME: */
					}
					break;

				case IE_RSN:
					pAd->bss_table.BssEntry[i].RsnIE.IELen = ieLen + 2;
					NdisMoveMemory(pAd->bss_table.BssEntry[i].RsnIE.IE, pData - 2, ieLen + 2);					
					break;

			}	
			len = len - 2 - ieLen;
			pData = pData + ieLen;
		}while(len > 4);
	}
	else DBGPRINT(RT_ERROR, ("[ERR]%s: BSS Table Full...\r\n", __FUNCTION__));

_exit:
	return;
}



VOID ScanSmartConfigBeacon(
	IN	PRTMP_ADAPTER		pAd)
{
	PRXWI_STRUC			pWI;
	PHEADER_802_11		pWiFiHeader;
	PUCHAR 				pData;
	int 				len;
	UCHAR 				frmType;
    PUSHORT 			CapabilityInfo;

	pWI = (PRXWI_STRUC)(pAd->RxUSBDataPtr + sizeof(RXINFO_STRUC));
	pWiFiHeader = (PHEADER_802_11)((PUCHAR)pWI + sizeof(RXWI_STRUC));
	pData = (PUCHAR) ((PUCHAR)pWiFiHeader + sizeof(HEADER_802_11));
	frmType = *(PUCHAR)pWiFiHeader;

	switch(frmType)
	{
		case PKT_BEACON:
			{
				CapabilityInfo = (PUSHORT)(pData + (TIME_STAMP_LEN + INTERVAL_LEN));
				pData = pData + (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);
				len = pWI->MPDUtotalByteCnt - sizeof(HEADER_802_11) - (TIME_STAMP_LEN + INTERVAL_LEN + CAPABILITY_INFO_LEN);

				if (RTMPCheckBeacon(pAd, pData, len))
				{
					UCHAR auth, uenc, menc, chan;

					chan = RTMPGetAPChannel(pData, len);
					if (chan != pAd->scanChannel) break;	// Prevent receive CRC error packets.
					else
						pAd->channel = chan;
					if (*CapabilityInfo & 0x0010)
						RTMPGetCipherInfo(pData, len, &auth, &uenc, &menc);
					else 
					{
						auth = Ndis802_11AuthModeOpen;
						uenc = Ndis802_11Cipher_NONE;
					}
					pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel = pAd->channel;

					NdisMoveMemory(pAd->essid, pWiFiHeader->Addr2, MAC_ADDR_LEN);
					NdisMoveMemory(pAd->StaCfg.RouterMac, pWiFiHeader->Addr2, MAC_ADDR_LEN);
					
					pAd->StaCfg.AuthMode = (NDIS_802_11_AUTHENTICATION_MODE)auth;
					pAd->StaCfg.EncMode = (NDIS_802_11_ENCRYPTION_MODE)uenc;

					/* Currently, only process Infrastructure mode */
					pAd->StaCfg.Networkmode = Ndis802_11Infrastructure;

					if (uenc == Ndis802_11Cipher_NONE)
					{
						pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
					}
					else if ((uenc == Ndis802_11Cipher_WEP40) || (uenc == Ndis802_11Cipher_WEP104)) 
					{
						pAd->StaCfg.WepStatus = Ndis802_11WEPEnabled;
					}
					else if (uenc == Ndis802_11Cipher_TKIP)
					{
						pAd->StaCfg.WepStatus = Ndis802_11Encryption2Enabled;
					}
					else if (uenc == Ndis802_11Cipher_CCMP)
					{
						pAd->StaCfg.WepStatus = Ndis802_11Encryption3Enabled;
					}
					else {
						DBGPRINT(RT_ERROR, ("[ERR]%s: Invalid encryption type(%d)\r\n", __FUNCTION__, uenc));
						pAd->StaCfg.WepStatus = Ndis802_11WEPDisabled;
					}

					if (auth >= Ndis802_11AuthModeWPA &&  auth < Ndis802_11AuthModeMax)
					{
						pAd->StaCfg.GroupCipher = (NDIS_802_11_ENCRYPTION_STATUS)menc;
					}
					pAd->StaCfg.PairCipher = pAd->StaCfg.WepStatus;
					pAd->StaCfg.bMixCipher = FALSE;

					/* reset PMK */
					//NdisZeroMemory(pAd->StaCfg.PMK, LEN_PMK);

					MGMTENTRY_Add(pAd, pWiFiHeader->Addr2);
					
					MLME_SetScanFlag(pAd, 0);
					//MT7601_ChipSwitchChannel(pAd, pAd->channel, 0);

					/* reset station mode control status */
					pAd->smartSetup = SmartConfigOff;
					pAd->LinkState = LS_NO_LINK;

					DBGPRINT(RT_INFO, ("[INFO]%s: Get Beacon Info Done! Auth(%d), Enc(%d), Channel(%d)\r\n",
										__FUNCTION__, pAd->StaCfg.AuthMode, pAd->StaCfg.EncMode, pAd->channel));

					/* In current application, app will pass target router SSID and Password to wifi.	*/
					/* Hence, it won't be update in SN986X case.										*/
					/* The feature will be depended on user scenario.  (Quickly connect?!)				*/
#ifdef NEED_STORE_STA_INFO
					MLME_StoreStaConfig(pAd);
#endif

					/* Notify user*/
//					if (pAd->hndler)
//					{
//						(pAd->hndler)(eWifiGetAPDone);
//					}
					/* directly connect without APP */
					{
						xWifiStackEvent_t ev_msg;
						memset(&ev_msg, 0x0, sizeof(ev_msg));
						ev_msg.eEventType = eWifiConnect;
						prvSendEventToWiFiTask(&ev_msg);
					}
				}
			}
			break;
		default:
			/* without any process */
			break;
	}	
}


VOID CollectScanBeacon(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN USHORT 			len, 
	IN CHAR 			rssi)
{
	UCHAR ie, ieLen;
	PHEADER_802_11 pWiFiHeader;
	PUSHORT CapabilityInfo;
	PAPScanResultElement	pScanElem = NULL;
	PAPScanResultElement	pNewElem = NULL;

	pWiFiHeader = (PHEADER_802_11) pData;
	pData += (sizeof(HEADER_802_11)+ TIME_STAMP_LEN);
	pData += INTERVAL_LEN;
	CapabilityInfo = (PUSHORT)pData;
	pData += CAPABILITY_INFO_LEN;

	if (pAd->nCollectNbr < MAX_AP_SCAN_NUM)
	{
		do
		{
			ie = pData[0];
			ieLen = pData[1];
			switch(ie)
			{
				case IE_SSID:
					if (ieLen == 0) return;

					if (pAd->nCollectNbr != 0)	//	is not first elemente
					{
						pScanElem = pAd->FirstElem;
						while (pScanElem != NULL)
						{
							if ((pScanElem->SsIdLen == ieLen) &&
							   (NdisCmpMemory(pScanElem->SsId, pData+2, ieLen) == 0))
							{
								return;
							}
							else
							{
								if (pScanElem->NextElem != NULL)
									pScanElem = pScanElem->NextElem;
								else
									break;									
							}	
						}
					}

					pNewElem = (APScanResultElement *)pvPortMalloc(sizeof(APScanResultElement));
					if (pNewElem == NULL) {
						DBGPRINT(RT_ERROR, ("[ERR]%s: Add Element failed: Allocate FAIL\r\n", __FUNCTION__));
						return;
					}

					NdisZeroMemory(pNewElem, sizeof(APScanResultElement));
					pNewElem->NextElem = NULL;

					/* Assign node value */
					pNewElem->signal = rssi;
					pNewElem->SsIdLen = ieLen;
					pNewElem->channel = RTMPGetAPChannel(pData, len);
					NdisMoveMemory(pNewElem->SsId, pData+2, ieLen);
					NdisMoveMemory(pNewElem->BssId, pWiFiHeader->Addr2, 6);
					NdisZeroMemory(pNewElem->auth, sizeof(pNewElem->auth));
					if (*CapabilityInfo & 0x0010) // Privacy bit, BIT4
						RTMPGetCipherInfo(pData, len, &pNewElem->auth_mode, &pNewElem->uenc_mode, &pNewElem->menc_mode);
					else
					{
						NdisMoveMemory(pNewElem->auth, "NONE", 4);
						pNewElem->auth_mode = Ndis802_11AuthModeOpen;
						pNewElem->uenc_mode = Ndis802_11Cipher_NONE;
					}

					if (pAd->nCollectNbr == 0)	//	first element
						pAd->FirstElem = pNewElem;
					else
						pScanElem->NextElem = pNewElem;

					pAd->nCollectNbr++;					
					return ;
			}
			pData += (2+ieLen);
			len -= (2+ieLen);
		}while(len > 1);
	}
	else DBGPRINT(RT_ERROR, ("[ERR]%s: Scan Table Full...\r\n", __FUNCTION__));
	
	return;
}

VOID DeleteScanList(
	IN PRTMP_ADAPTER 	pAd)
{
	PAPScanResultElement	pScanElem = NULL;
	PAPScanResultElement	pPrevElem = NULL;

	pScanElem = pAd->FirstElem;
	if (pScanElem != NULL)
	{
		DBGPRINT(RT_TRACE, ("[TRACE]%s: free ap list\r\n", __FUNCTION__));		
	    while (pScanElem != NULL)
		{
	    	pPrevElem = pScanElem;
	        pScanElem = pScanElem->NextElem;
			vPortFree(pPrevElem);
	    }
		pAd->FirstElem = NULL;
		pAd->nCollectNbr = 0;
    }
}

