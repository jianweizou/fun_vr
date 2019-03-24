#include "asic_ctrl.h"
#include "common.h"
#include "usb_io.h"

extern char*   CipherName[];

VOID AsicInitBcnBuf(
	IN PRTMP_ADAPTER 	pAd)
{
	int idx;
#ifdef RLT_MAC
	{
		RTMP_REG_PAIR bcn_mac_reg_tb[] = {
			{BCN_OFFSET0, 0x18100800},
			{BCN_OFFSET1, 0x38302820},
			{BCN_OFFSET2, 0x58504840},
			{BCN_OFFSET3, 0x78706860},
		};
		
		for (idx = 0; idx < 4; idx ++)
		{
			RTMP_IO_WRITE32(pAd, (USHORT)bcn_mac_reg_tb[idx].Register, 
									bcn_mac_reg_tb[idx].Value);
		}
	}
#endif /* RLT_MAC */
}



VOID AsicSetBssid(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pBssid) 
{
	ULONG		  Addr4;

	DBGPRINT(RT_TRACE, ("[TRACE]==============>%s: %02X:%02X:%02X:%02X:%02X:%02X\n",
						__FUNCTION__, PRINT_MAC(pBssid)));

	Addr4 = *(PULONG)pBssid;
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);
	Addr4 = 0;
	Addr4 = *(PULONG)(pBssid + 4);
	Addr4 &= 0xFFFF;
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

}

VOID AsicSetSynInfo(
	IN PRTMP_ADAPTER 	pAd)
{
	BCN_TIME_CFG_STRUC csr9;

	csr9.word = 0;
	
	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word);
	csr9.field.BeaconInterval = pAd->beaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr9.field.bTsfTicking = 1;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{	csr9.field.TsfSyncMode = 3; /* sync TSF in AP mode*/
		csr9.field.bBeaconGen = 1;
	}
	else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		csr9.field.TsfSyncMode = 1; /* sync TSF in DEvice mode*/
		csr9.field.bBeaconGen = 0;
	}
	else	
		csr9.field.TsfSyncMode = 2; /* sync TSF in IBSS mode*/

	csr9.field.bTBTTEnable = 1;
	
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
}



VOID AsicEnableIbssSync(
	IN PRTMP_ADAPTER 	pAd)
{
	BCN_TIME_CFG_STRUC csr9;

	PUCHAR			ptr;
	UINT i;
	USHORT			beaconLen = pAd->BeaconTxWI.TXWI_N.MPDUtotalByteCnt;
	
#ifdef SPECIFIC_BCN_BUF_SUPPORT	
	unsigned long irqFlag = 0;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word);
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);


#ifdef RTMP_MAC_USB
	/* move BEACON TXD and frame content to on-chip memory*/
	ptr = (PUCHAR)&pAd->BeaconTxWI;

	for (i = 0; i < sizeof(TXWI_STRUC); i += 2)  /* 16-byte TXWI field*/
	{
		RTUSB_MultiWrite(pAd, HW_BEACON_BASE0(pAd) + i, ptr, 2, FALSE);
		ptr += 2;
	}
 
	/* start right after the 16-byte TXWI field*/
	ptr = pAd->BeaconBuf;
	
	for (i = 0; i < beaconLen; i += 2)
	{
		RTUSB_MultiWrite(pAd, HW_BEACON_BASE0(pAd) + sizeof(TXWI_STRUC) + i, ptr, 2, FALSE);
		ptr += 2;
	}
#endif /* RTMP_MAC_USB */

	
	/* For Wi-Fi faily generated beacons between participating stations. */
	/* Set TBTT phase adaptive adjustment step to 8us (default 16us)*/
	/* don't change settings 2006-5- by Jerry*/
//	RTMP_IO_WRITE32(pAd, TBTT_SYNC_CFG, 0x00001010);
	
	/* start sending BEACON*/
	AsicSetSynInfo(pAd);

	RTMP_IO_WRITE32(pAd, MAC_WCID_BASE, 0x00);
	RTMP_IO_WRITE32(pAd, MAC_WCID_BASE+8, 0x00);
}


VOID AsicDisableIbssSync(
	IN PRTMP_ADAPTER 	pAd)
{	
	unsigned short uwLoop;
	BCN_TIME_CFG_STRUC csr9;
	
	for (uwLoop = 0; uwLoop < 60000; uwLoop++)
	{
		if (RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word) == NDIS_SUCCESS)
			break;
	}
	
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;	

	for (uwLoop = 0; uwLoop < 60000; uwLoop++)
	{
		if (RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word) == NDIS_SUCCESS)
			break;
	}
}


VOID AsicDisableSync(
	IN PRTMP_ADAPTER 	pAd) 
{
	BCN_TIME_CFG_STRUC csr;
	
	DBGPRINT(RT_TRACE, ("[TRACE]--->%s: Disable TSF synchronization\n", __FUNCTION__));

	/* 2003-12-20 disable TSF and TBTT while NIC in power-saving have side effect*/
	/*			  that NIC will never wakes up because TSF stops and no more */
	/*			  TBTT interrupts*/
//	pAd->TbttTickCount = 0;
	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
	csr.field.bBeaconGen = 0;
	csr.field.bTBTTEnable = 0;
	csr.field.TsfSyncMode = 0;
	csr.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}


VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
	INT i;
	ULONG 		offset;
	PUCHAR 		pKey = pCipherKey->Key;
	PUCHAR 		pTxMic = pCipherKey->TxMic;
	PUCHAR 		pRxMic = pCipherKey->RxMic;
	UCHAR 		CipherAlg = pCipherKey->CipherAlg;

	/* EKEY*/
	offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
#ifdef RTMP_MAC_USB
	RTUSB_MultiWrite(pAd, offset, &pCipherKey->Key[0], MAX_LEN_OF_PEER_KEY, FALSE);
#endif /* RTMP_MAC_USB */

	for (i = 0; i < MAX_LEN_OF_PEER_KEY; i+=4)
	{
		UINT32 Value;
		RTMP_IO_READ32(pAd, offset + i, &Value);
	}

	offset += MAX_LEN_OF_PEER_KEY;
	
	/*  MIC KEY*/
	if (pTxMic)
	{
#ifdef RTMP_MAC_USB
		RTUSB_MultiWrite(pAd, offset, &pCipherKey->TxMic[0], 8, FALSE);
#endif /* RTMP_MAC_USB */
	}
	offset += 8;
	if (pRxMic)
	{
#ifdef RTMP_MAC_USB
		RTUSB_MultiWrite(pAd, offset, &pCipherKey->RxMic[0], 8, FALSE);
#endif /* RTMP_MAC_USB */
	}
	
	DBGPRINT(RT_TRACE,("[TRACE]%s: WCID #%d Alg=%s\n", __FUNCTION__, WCID, CipherName[CipherAlg]));
	DBGPRINT(RT_LOUD,("[LOUD]%s: 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__, pKey[0], pKey[1], pKey[2], pKey[3], pKey[4], pKey[5], pKey[6], pKey[7], pKey[8], 
									  pKey[9],pKey[10], pKey[11], pKey[12], pKey[13], pKey[14], pKey[15]));

	if (pRxMic)
	{
		DBGPRINT(RT_LOUD, ("[LOUD]%s: 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
								__FUNCTION__, pRxMic[0], pRxMic[1], pRxMic[2], pRxMic[3], pRxMic[4], pRxMic[5], pRxMic[6], pRxMic[7]));
	}

	if (pTxMic)
	{
		DBGPRINT(RT_LOUD, ("[LOUD]%s: 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
								__FUNCTION__, pTxMic[0], pTxMic[1], pTxMic[2], pTxMic[3], pTxMic[4], pTxMic[5], pTxMic[6], pTxMic[7]));
	}
}


VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	AsicUpdateWcidAttributeEntry(pAd, 
							  BSS0,
							  0,
							  CIPHER_NONE, 
							  Wcid,
							  PAIRWISEKEYTABLE);

	DBGPRINT(RT_LOUD, ("[LOUD]%s : Wcid #%d \n", __FUNCTION__, Wcid));
}


VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	ULONG offset; /*, csr0;*/
	SHAREDKEY_MODE_STRUC csr1;

	PUCHAR		pKey = pCipherKey->Key;
	PUCHAR		pTxMic = pCipherKey->TxMic;
	PUCHAR		pRxMic = pCipherKey->RxMic;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;

	DBGPRINT(RT_TRACE, ("[TRACE]%s: BssIndex=%d, KeyIdx=%d Alg=%s key:#%d\n", 
						__FUNCTION__, BssIndex, KeyIdx, CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	
	DBGPRINT(RT_LOUD, ("[LOUD]%s: 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__, pKey[0], pKey[1], pKey[2], pKey[3], pKey[4], pKey[5], pKey[6], pKey[7], 
									  pKey[8], pKey[9], pKey[10], pKey[11], pKey[12], pKey[13], pKey[14], pKey[15]));
	if (pRxMic)
	{
		DBGPRINT(RT_LOUD, ("[LOUD]%s: 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
							__FUNCTION__, pRxMic[0], pRxMic[1], pRxMic[2], pRxMic[3], pRxMic[4], pRxMic[5], pRxMic[6], pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT(RT_LOUD, ("[LOUD]%s: 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
							__FUNCTION__, pTxMic[0], pTxMic[1], pTxMic[2], pTxMic[3], pTxMic[4], pTxMic[5], pTxMic[6], pTxMic[7]));
	}


	offset = SHARED_KEY_TABLE_BASE + (4*BssIndex + KeyIdx)*HW_KEY_ENTRY_SIZE;
	RTUSB_MultiWrite(pAd, offset, pKey, MAX_LEN_OF_SHARE_KEY, FALSE);

	offset += MAX_LEN_OF_SHARE_KEY;
	if (pTxMic)
	{
		RTUSB_MultiWrite(pAd, offset, pTxMic, 8, FALSE);
	}

	offset += 8;
	if (pRxMic)
	{
		RTUSB_MultiWrite(pAd, offset, pRxMic, 8, FALSE);
	}	
	
	/* Update cipher algorithm. WSTA always use BSS0*/
	
	RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE + 4*(BssIndex/2), &csr1.word);
	DBGPRINT(RT_LOUD, ("[LOUD]%s:(Read) SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", 
							__FUNCTION__, BssIndex,KeyIdx, csr1.word));
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss0Key3CipherAlg = CipherAlg;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss1Key3CipherAlg = CipherAlg;
	}
	DBGPRINT(RT_LOUD, ("[LOUD]%s:(Write) SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n",
							__FUNCTION__, BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE + 4*(BssIndex/2), csr1.word);
		
}


VOID AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
	WCID_ATTRIBUTE_STRUC WCIDAttri;	
	USHORT		offset;

	/* Initialize the content of WCID Attribue  */
	WCIDAttri.word = 0;

	/* Update the pairwise key security mode.
	   Use bit10 and bit3~1 to indicate the pairwise cipher mode */	
	WCIDAttri.field.PairKeyModeExt = ((CipherAlg & 0x08) >> 3);
	WCIDAttri.field.PairKeyMode = (CipherAlg & 0x07);

	/* Update the MBSS index.
	   Use bit11 and bit6~4 to indicate the BSS index */	
	WCIDAttri.field.BSSIdxExt = ((BssIdx & 0x08) >> 3);
	WCIDAttri.field.BSSIdx = (BssIdx & 0x07);
	
	/* Assign Key Table selection */		
	WCIDAttri.field.KeyTab = KeyTabFlag;

	/* Update related information to ASIC */
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid * HW_WCID_ATTRI_SIZE);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri.word);

 	DBGPRINT(RT_LOUD, ("[LOUD]%s : WCID #%d, KeyIndex #%d, Alg=%s WCIDAttri = 0x%x\n", 
						__FUNCTION__, Wcid, KeyIdx, CipherName[CipherAlg], WCIDAttri.word));
	
}


VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER 	pAd,
	IN USHORT			WCID,
	IN ULONG        	uIV,
	IN ULONG        	uEIV)
{
	ULONG	offset;

	offset = MAC_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);

	RTMP_IO_WRITE32(pAd, offset, uIV);
	RTMP_IO_WRITE32(pAd, offset + 4, uEIV);

	DBGPRINT(RT_LOUD, ("[LOUD]%s: wcid(%d) 0x%08lx, 0x%08lx \n", 
									__FUNCTION__, WCID, uIV, uEIV));	
}


VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER 	pAd,
	IN USHORT			WCID,
	IN PUCHAR        	pAddr)
{
	ULONG offset;
	ULONG Addr;
	
	offset = MAC_WCID_BASE + (WCID * HW_WCID_ENTRY_SIZE);	
	Addr = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = pAddr[4] + (pAddr[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);	
}


BOOLEAN AsicWaitPDMAIdle(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				round, 
	IN INT 				wait_us)
{
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;

	do {
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0)) {
			DBGPRINT(RT_TRACE, ("[TRCAE]--->%s: DMAIdle, GloCfg=0x%x\n", __FUNCTION__, GloCfg.word));
			return TRUE;
		}
		RTMPusecDelay(wait_us);
	} while ((i++) < round);

	DBGPRINT(RT_TRACE, ("[TRCAE]<---%s: DMABusy, GloCfg=0x%x\n", __FUNCTION__, GloCfg.word));
	
	return FALSE;
}


VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER 	pAd,
	IN PEDCA_PARM	 	pEdcaParm)
{
	EDCA_AC_CFG_STRUC   Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC    AifsnCsr;
	CWMIN_CSR_STRUC    CwminCsr;
	CWMAX_CSR_STRUC    CwmaxCsr;

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;

	/*========================================================*/
	/*      MAC Register has a copy.*/
	/*========================================================*/
	
	/* Modify Cwmin/Cwmax/Txop on queue[QID_AC_VI], Recommend by Jerry 2005/07/27*/
	/* To degrade our VIDO Queue's throughput for WiFi WMM S3T07 Issue.*/
	
	/*pEdcaParm->Txop[QID_AC_VI] = pEdcaParm->Txop[QID_AC_VI] * 7 / 10;  rt2860c need this		*/

	Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE];
	Ac0Cfg.field.Cwmin 	= pEdcaParm->Cwmin[QID_AC_BE];
	Ac0Cfg.field.Cwmax 	= pEdcaParm->Cwmax[QID_AC_BE];
	Ac0Cfg.field.Aifsn 	= pEdcaParm->Aifsn[QID_AC_BE]; /*+1;*/
	Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
	Ac1Cfg.field.Cwmin 	= pEdcaParm->Cwmin[QID_AC_BK]; /*+2; */
	Ac1Cfg.field.Cwmax 	= pEdcaParm->Cwmax[QID_AC_BK];
	Ac1Cfg.field.Aifsn 	= pEdcaParm->Aifsn[QID_AC_BK]; /*+1;*/
	Ac2Cfg.field.AcTxop = (pEdcaParm->Txop[QID_AC_VI] * 6) / 10;
	Ac2Cfg.field.Cwmin 	= pEdcaParm->Cwmin[QID_AC_VI];
	Ac2Cfg.field.Cwmax 	= pEdcaParm->Cwmax[QID_AC_VI];
	/*sync with window 20110524*/
	Ac2Cfg.field.Aifsn 	= pEdcaParm->Aifsn[QID_AC_VI] + 1; /* 5.2.27 T6 Pass Tx VI+BE, but will impack 5.2.27/28 T7. Tx VI*/

#ifdef CONFIG_STA_SUPPORT
//	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Tuning for Wi-Fi WMM S06*/
		if (pAd->CommonCfg.bWiFiTest && 
			pEdcaParm->Aifsn[QID_AC_VI] == 10)
			Ac2Cfg.field.Aifsn -= 1; 

		/* Tuning for TGn Wi-Fi 5.2.32*/
		/* STA TestBed changes in this item: conexant legacy sta ==> broadcom 11n sta*/
		if (0)//STA_TGN_WIFI_ON(pAd) && 
//				pEdcaParm->Aifsn[QID_AC_VI] == 10)
		{
			Ac0Cfg.field.Aifsn = 3;
			Ac2Cfg.field.AcTxop = 5;
		}
		
	}
#endif /* CONFIG_STA_SUPPORT */

	Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
	Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
	Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
	Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];

	if (pAd->CommonCfg.bWiFiTest)
	{
		if (Ac3Cfg.field.AcTxop == 102)
		{
		Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE] ? pEdcaParm->Txop[QID_AC_BE] : 10;
			Ac0Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BE]-1; /* AIFSN must >= 1 */
		Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
			Ac1Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BK];
		Ac2Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VI];
		}
	}


	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
	RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);
	RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
	RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);


	/*========================================================*/
	/*      DMA Register has a copy too.*/
	/*========================================================*/
	csr0.field.Ac0Txop = Ac0Cfg.field.AcTxop;
	csr0.field.Ac1Txop = Ac1Cfg.field.AcTxop;
	RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);

	csr1.field.Ac2Txop = Ac2Cfg.field.AcTxop;
	csr1.field.Ac3Txop = Ac3Cfg.field.AcTxop;
	RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

	CwminCsr.word = 0;
	CwminCsr.field.Cwmin0 = pEdcaParm->Cwmin[QID_AC_BE];
	CwminCsr.field.Cwmin1 = pEdcaParm->Cwmin[QID_AC_BK];
	CwminCsr.field.Cwmin2 = pEdcaParm->Cwmin[QID_AC_VI];
#ifdef CONFIG_STA_SUPPORT
//	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; /*for TGn wifi test*/
#endif /* CONFIG_STA_SUPPORT */
	RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

	CwmaxCsr.word = 0;
	CwmaxCsr.field.Cwmax0 = pEdcaParm->Cwmax[QID_AC_BE];
	CwmaxCsr.field.Cwmax1 = pEdcaParm->Cwmax[QID_AC_BK];
	CwmaxCsr.field.Cwmax2 = pEdcaParm->Cwmax[QID_AC_VI];
	CwmaxCsr.field.Cwmax3 = pEdcaParm->Cwmax[QID_AC_VO];
	RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

	AifsnCsr.word = 0;
	AifsnCsr.field.Aifsn0 = Ac0Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BE];*/
	AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BK];*/
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_USB
//	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->Antenna.field.TxPath == 1)
			AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn + 2; 	/*5.2.27 T7 Pass*/
	}
#endif /* RTMP_MAC_USB */
#endif /* CONFIG_STA_SUPPORT */
	AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VI];*/

#ifdef CONFIG_STA_SUPPORT
//	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Tuning for Wi-Fi WMM S06*/
		if (pAd->CommonCfg.bWiFiTest &&
			pEdcaParm->Aifsn[QID_AC_VI] == 10)
			AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn - 4;

		/* Tuning for TGn Wi-Fi 5.2.32*/
		/* STA TestBed changes in this item: connexant legacy sta ==> broadcom 11n sta*/
		if (0)//STA_TGN_WIFI_ON(pAd) && 
//				pEdcaParm->Aifsn[QID_AC_VI] == 10)
		{
			AifsnCsr.field.Aifsn0 = 3;
			AifsnCsr.field.Aifsn2 = 7;
		}

	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
//	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; /*pEdcaParm->Aifsn[QID_AC_VO]; for TGn wifi test*/
	}
#endif /* CONFIG_STA_SUPPORT */
	RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);

	NdisMoveMemory(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	if (0)//!ADHOC_ON(pAd))
	{
		DBGPRINT(RT_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
		DBGPRINT(RT_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
								 pEdcaParm->Aifsn[0],
								 pEdcaParm->Cwmin[0],
								 pEdcaParm->Cwmax[0],
								 pEdcaParm->Txop[0]<<5,
								 pEdcaParm->bACM[0]));
		DBGPRINT(RT_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
								 pEdcaParm->Aifsn[1],
								 pEdcaParm->Cwmin[1],
								 pEdcaParm->Cwmax[1],
								 pEdcaParm->Txop[1]<<5,
								 pEdcaParm->bACM[1]));
		DBGPRINT(RT_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
								 pEdcaParm->Aifsn[2],
								 pEdcaParm->Cwmin[2],
								 pEdcaParm->Cwmax[2],
								 pEdcaParm->Txop[2]<<5,
								 pEdcaParm->bACM[2]));
		DBGPRINT(RT_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
								 pEdcaParm->Aifsn[3],
								 pEdcaParm->Cwmin[3],
								 pEdcaParm->Cwmax[3],
								 pEdcaParm->Txop[3]<<5,
								 pEdcaParm->bACM[3]));
	}
}

VOID AsicFifoExtSet(
	IN PRTMP_ADAPTER 	pAd)
{
	RTMP_IO_WRITE32(pAd, WCID_MAPPING_0, 0x04030201);
	RTMP_IO_WRITE32(pAd, WCID_MAPPING_1, 0x08070605);
}
