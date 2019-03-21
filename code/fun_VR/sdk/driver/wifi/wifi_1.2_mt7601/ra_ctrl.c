#include "ra_ctrl.h"
#include "rtmp.h"
#include "usb_io.h"

/* Extern function */
VOID MLME_SelectTxRateTable(IN PRTMP_ADAPTER pAd,IN PMGMTENTRY pEntry,IN PUCHAR *ppTable,IN PUCHAR pTableSize,IN PUCHAR pInitTxRateIdx);
/*
	========================================================================
	
	Routine Description:
		Read Tx statistic raw counters from hardware registers and record to
		related software variables for later on query

	Arguments:
		pAd					Pointer to our adapter
		pStaTxCnt0			Pointer to record "TX_STA_CNT0" (0x170c)
		pStaTxCnt1			Pointer to record "TX_STA_CNT1" (0x1710)

	Return Value:
		None

	========================================================================
*/
VOID NICGetTxRawCounters(
	IN PRTMP_ADAPTER 		pAd,
	IN PTX_STA_CNT0_STRUC 	pStaTxCnt0,
	IN PTX_STA_CNT1_STRUC 	pStaTxCnt1)
{
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &pStaTxCnt0->word);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &pStaTxCnt1->word);
}

VOID NICGetMacFifoTxCnt(
	IN PRTMP_ADAPTER 		pAd,
	IN PMGMTENTRY 			pEntry,
	IN PWCID_TX_CNT_STRUC 	pWcidTxCnt)
{
	if (pEntry->Aid >= 1 && pEntry->Aid <= 8)
	{
		UINT32 regAddr;
		
		regAddr = WCID_TX_CNT_0 + (pEntry->Aid - 1) * 4;
		RTMP_IO_READ32(pAd, regAddr, &pWcidTxCnt->word);
	}
}

VOID MLME_SetTxRate(
	IN PRTMP_ADAPTER 		pAd,
	IN PMGMTENTRY 			pEntry,
	IN PRTMP_RA_LEGACY_TB 	pTxRate)
{
	pEntry->PhyMode = pTxRate->Mode;
	pEntry->CurrTxRate = pTxRate->CurrMCS;
}

/*
	MlmeNewTxRate - called when a new TX rate was selected. Sets TX PHY to
		rate selected by pEntry->CurrTxRateIndex in pTable;
*/
VOID MLME_NewTxRate(PRTMP_ADAPTER pAd, PMGMTENTRY pEntry)
{
	PRTMP_RA_LEGACY_TB pNextTxRate;
	PUCHAR pTable = pEntry->pTable;

	/*  Get pointer to CurrTxRate entry */
	pNextTxRate = PTX_RA_LEGACY_ENTRY(pTable, pEntry->CurrTxRateIndex);

	/*  Set new rate */
	MLME_SetTxRate(pAd, pEntry, pNextTxRate);
}

/* MlmeGetSupportedMcs - fills in the table of mcs with index into the pTable
		pAd - pointer to adapter
		pTable - pointer to the Rate Table. Assumed to be a table without mcsGroup values
		mcs - table of MCS index into the Rate Table. -1 => not supported
*/
VOID MLME_GetSupportedMcs(
	IN PRTMP_ADAPTER 	pAd,
	IN PUCHAR			pTable,
	OUT CHAR 			mcs[])
{
	CHAR	idx;
	PRTMP_RA_LEGACY_TB 	pCurrTxRate;

	for (idx=0; idx<24; idx++)
		mcs[idx] = -1;

	/*  check the existence and index of each needed MCS */
	for (idx=0; idx<RATE_TABLE_SIZE(pTable); idx++)
	{
		pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, idx);

		/*  Rate Table may contain CCK and MCS rates. Give HT/Legacy priority over CCK */
		if (pCurrTxRate->CurrMCS==MCS_0 && (mcs[0]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[0] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_1 && (mcs[1]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[1] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_2 && (mcs[2]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[2] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_3)
			mcs[3] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_4)
			mcs[4] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_5)
			mcs[5] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_6)
			mcs[6] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))
			mcs[7] = idx;
#ifdef DOT11_N_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_12)
			mcs[12] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_13)
			mcs[13] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_14)
			mcs[14] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
		{
			mcs[15] = idx;
		}
#endif /*  DOT11_N_SUPPORT */
	}
}

/*
	MlmeSelectTxRate - select the MCS based on the RSSI and the available MCSs
		pAd - pointer to adapter
		pEntry - pointer to MAC table entry
		mcs - table of MCS index into the Rate Table. -1 => not supported
		Rssi - the Rssi value
		RssiOffset - offset to apply to the Rssi
*/
UCHAR MLME_SelectTxRate(
	IN PRTMP_ADAPTER 	pAd,
	IN PMGMTENTRY 		pEntry,
	IN CHAR				mcs[],
	IN CHAR				Rssi,
	IN CHAR				RssiOffset)
{
	UCHAR TxRateIdx = 0;

	{/*  Legacy mode */
		if (mcs[7]>=0 && (Rssi > -70))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > -74))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > -78))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > -82))
			TxRateIdx = mcs[4];
		else if (mcs[4] == -1)							/*  for B-only mode */
		{
			if (mcs[3]>=0 && (Rssi > -85))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > -87))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > -90))
				TxRateIdx = mcs[1];
			else 
				TxRateIdx = mcs[0];			
		}
		else if (mcs[3]>=0 && (Rssi > -85))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > -87))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > -90))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}

	return TxRateIdx;
}

/*  MlmeClearAllTxQuality - Clear both BF and non-BF TxQuality history */
VOID MLME_ClearAllTxQuality(
	IN PMGMTENTRY	pEntry)
{
	NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));
}

/*  MlmeDecTxQuality - Decrement TxQuality of specified rate table entry */
VOID MLME_DecTxQuality(
	IN PMGMTENTRY	pEntry,
	IN UCHAR		rateIndex)
{
	if (pEntry->TxQuality[rateIndex])
		pEntry->TxQuality[rateIndex]--;
}

VOID MLME_SetTxQuality(
	IN PMGMTENTRY	pEntry,
	IN UCHAR		rateIndex,
	IN USHORT		txQuality)
{
	pEntry->TxQuality[rateIndex] = txQuality;
}

USHORT MLME_GetTxQuality(
	IN PMGMTENTRY	pEntry,
	IN UCHAR		rateIndex)
{
	return pEntry->TxQuality[rateIndex];
}

/*  MlmeClearTxQuality - Clear TxQuality history only for the active BF state */
VOID MLME_ClearTxQuality(
	IN PMGMTENTRY	pEntry)
{
	NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));
}

/*
	MlmeOldRateAdapt - perform Rate Adaptation based on PER using old RA algorithm
		pEntry - the MAC table entry
		CurrRateIdx - the index of the current rate
		UpRateIdx, DownRateIdx - UpRate and DownRate index
		TrainUp, TrainDown - TrainUp and Train Down threhsolds
		TxErrorRatio - the PER

		On exit:
			pEntry->LastSecTxRateChangeAction = RATE_UP or RATE_DOWN if there was a change
			pEntry->CurrTxRateIndex = new rate index
			pEntry->TxQuality is updated
*/
VOID MLME_OldRateAdapt(
	IN PRTMP_ADAPTER 	pAd,
	IN PMGMTENTRY		pEntry,
	IN UCHAR			CurrRateIdx,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio)
{
	BOOLEAN	bTrainUp = FALSE;

	//pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
	UCHAR LastSecTxRateChangeAction = RATE_NO_CHANGE;

	/* Downgrade TX quality if PER >= Rate-Down threshold */
	if (TxErrorRatio >= TrainDown)
	{
		MLME_SetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);
		if (CurrRateIdx != DownRateIdx)
		{
			pEntry->CurrTxRateIndex = DownRateIdx;
			//pEntry->LastSecTxRateChangeAction = RATE_DOWN;
			LastSecTxRateChangeAction = RATE_DOWN;
		}
	}
	else
	{
		/* Upgrade TX quality if PER <= Rate-Up threshold */
		if (TxErrorRatio <= TrainUp)
		{
			bTrainUp = TRUE;
			MLME_DecTxQuality(pEntry, CurrRateIdx);  /* quality very good in CurrRate */

			if (pEntry->TxRateUpPenalty)
				pEntry->TxRateUpPenalty --;
			else
				MLME_DecTxQuality(pEntry, UpRateIdx);    /* may improve next UP rate's quality */
		}

		if (bTrainUp)
		{
			/* Train up if up rate quality is 0 */
			if ((CurrRateIdx != UpRateIdx) && (MLME_GetTxQuality(pEntry, UpRateIdx) <= 0))
			{
				pEntry->CurrTxRateIndex = UpRateIdx;
				//pEntry->LastSecTxRateChangeAction = RATE_UP;
				LastSecTxRateChangeAction = RATE_UP;
			}
		}
	}

	/* Handle the rate change */
	//if (pEntry->LastSecTxRateChangeAction != RATE_NO_CHANGE)
	if (LastSecTxRateChangeAction != RATE_NO_CHANGE)
	{
		pEntry->TxRateUpPenalty = 0;

		/* Update TxQuality */
		//if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		if (LastSecTxRateChangeAction == RATE_UP)
		{
			/* Clear history if normal train up */
			//if (pEntry->lastRateIdx != pEntry->CurrTxRateIndex)
			if (CurrRateIdx != pEntry->CurrTxRateIndex)
				MLME_ClearTxQuality(pEntry);
		}
		else
		{
			/* Clear the down rate history */
			MLME_SetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);
		}

		/* Update PHY rate */
		MLME_NewTxRate(pAd, pEntry);
	}
}

/*
	==========================================================================
	Description:
		This routine calculates the acumulated TxPER of eaxh TxRate. And
		according to the calculation result, change CommonCfg.TxRate which
		is the stable TX Rate we expect the Radio situation could sustained.

		CommonCfg.TxRate will change dynamically within {RATE_1/RATE_6, MaxTxRate}
	Output:
		CommonCfg.TxRate -

	IRQL = DISPATCH_LEVEL

	NOTE:
		call this routine every second
	==========================================================================
 */
VOID MLME_DynamicTxRateSwitching(
	IN PRTMP_ADAPTER pAd)
{
	PUCHAR					pTable;
	UCHAR					TableSize = 0;
	UCHAR					UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx;
	ULONG					TxTotalCnt;
	ULONG					TxErrorRatio = 0;
	PMGMTENTRY				pEntry = NULL;
	PRTMP_RA_LEGACY_TB 		pCurrTxRate;
	UCHAR					InitTxRateIdx, TrainUp, TrainDown;	
	CHAR					Rssi, TmpIdx = 0;
	ULONG					TxRetransmit = 0, TxSuccess = 0, TxFailCount = 0;
	int 					i;	
	
	for (i = 0; i < pAd->node_num; i++)
	{
		if (i == 0)
			pEntry = pAd->pFirstLinkNode;
		else
		{
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				pEntry = pEntry->next_node;
			}
			else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				break;
			}
		}
		
		/* walk through MAC table, see if need to change AP's TX rate toward each entry */
	   	if (pEntry)
		{				
			MLME_SelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
			pEntry->pTable = pTable;

			Rssi = pEntry->rx_stregth;
			if (pAd->node_num == 1)			
			{
				TX_STA_CNT0_STRUC		TxStaCnt0;
				TX_STA_CNT1_STRUC		TxStaCnt1;				
				
				/* Update statistic counter */
				NICGetTxRawCounters(pAd, &TxStaCnt0, &TxStaCnt1);
			
				TxRetransmit = TxStaCnt1.field.TxRetransmit;
				TxSuccess = TxStaCnt1.field.TxSuccess;
				TxFailCount = TxStaCnt0.field.TxFailCount;
				TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;			
				if (TxTotalCnt)
					TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
				
				DBGPRINT(RT_TRACE,
						("DRS:Aid=%d, TxSuccess=%ld, TxRetransmit=%ld, TxFailCount=%ld, TxErrorRatio=%ld\n",
						pEntry->Aid, TxSuccess, TxRetransmit, TxFailCount, TxErrorRatio));
			}
			else
			{
				WCID_TX_CNT_STRUC wcidTxCnt;
				
				NICGetMacFifoTxCnt(pAd, pEntry, &wcidTxCnt);

				TxRetransmit = wcidTxCnt.field.reTryCnt;
				TxSuccess = wcidTxCnt.field.succCnt;
				TxTotalCnt = TxRetransmit + TxSuccess;
				if (TxTotalCnt)
					TxErrorRatio = (TxRetransmit * 100) / TxTotalCnt;

				DBGPRINT(RT_TRACE,
						("DRS:Aid=%d, TxSuccess=%ld, TxRetransmit=%ld, TxErrorRatio=%ld\n",
						pEntry->Aid, TxSuccess, TxRetransmit, TxErrorRatio));
			}

			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				if (TxTotalCnt)
				{
					if (TxErrorRatio == 100)
					{
						TX_RTY_CFG_STRUC	TxRtyCfg,TxRtyCfgtmp;
						ULONG	Index;
						UINT32	MACValue;

						RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg.word);
						TxRtyCfgtmp.word = TxRtyCfg.word;
						TxRtyCfg.field.LongRtyLimit = 0x0;
						TxRtyCfg.field.ShortRtyLimit = 0x0;
						RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg.word);

						RTMPusecDelay(1);

						Index = 0;
						MACValue = 0;
						do
						{
							RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);		// page count in Tx queue
							if ((MACValue & 0xffffff) == 0)
								break;
							Index++;
							RTMPusecDelay(1000);
						}while(Index < 330);

						RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg.word);
						TxRtyCfg.field.LongRtyLimit = TxRtyCfgtmp.field.LongRtyLimit;
						TxRtyCfg.field.ShortRtyLimit = TxRtyCfgtmp.field.ShortRtyLimit;
						RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg.word);
					}
				}
			}

			CurrRateIdx = pEntry->CurrTxRateIndex;

			if (CurrRateIdx >= TableSize)
				CurrRateIdx = TableSize - 1;

			UpRateIdx = DownRateIdx = CurrRateIdx;

			/*
				When switch from Fixed rate -> auto rate, the REAL TX rate might be different from pEntry->TxRateIndex.
				So need to sync here.
			*/
			pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, CurrRateIdx);
			if (pEntry->CurrTxRate != pCurrTxRate->CurrMCS)
			{
				/*
					Need to sync Real Tx rate and our record.
					Then return for next DRS.
				*/
				pEntry->CurrTxRateIndex = InitTxRateIdx;
				MLME_NewTxRate(pAd, pEntry);

				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					continue;
				}
				else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					break;
				}
			}

			/* decide the next upgrade rate and downgrade rate, if any */
			if ((pCurrTxRate->Mode <= MODE_CCK))
			{
				TmpIdx = CurrRateIdx + 1;
				if (TmpIdx < TableSize)
				{
					UpRateIdx = TmpIdx;
				}

				TmpIdx = CurrRateIdx - 1;
				if (TmpIdx >= 0)
				{
					DownRateIdx = TmpIdx;
				}
			}		
			else if ((pCurrTxRate->Mode <= MODE_OFDM))
			{
				TmpIdx = CurrRateIdx + 1;
				if (TmpIdx < TableSize)
				{
					UpRateIdx = TmpIdx;
				}

				TmpIdx = CurrRateIdx - 1;
				if (TmpIdx >= 0)
				{
					DownRateIdx = TmpIdx;
				}
			}

			pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, CurrRateIdx);
#ifdef DOT11_N_SUPPORT

			/*
				when Rssi > -65, there is a lot of interference usually. therefore, the 
				algorithm tends to choose the mcs lower than the optimal one.
				by increasing the thresholds, the chosen mcs will be closer to the optimal mcs
			*/
			if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
			{
				TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
				TrainDown = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				TrainUp = pCurrTxRate->TrainUp;
				TrainDown = pCurrTxRate->TrainDown;
			}

			/*
				CASE 1. when TX samples are fewer than 15, then decide TX rate solely on RSSI
				     (criteria copied from RT2500 for Netopia case)
			*/
			if (TxTotalCnt <= 15)
			{
				UCHAR	TxRateIdx;
				CHAR	mcs[24];
				CHAR	RssiOffset = 0;

				/* Check existence and get the index of each MCS */
				MLME_GetSupportedMcs(pAd, pTable, mcs);

				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					if (pAd->LatchRfRegs.Channel <= 14)
					{
						RssiOffset = pAd->NicConfig2.field.ExternalLNAForG? 2: 5;
					}
					else
					{
						RssiOffset = pAd->NicConfig2.field.ExternalLNAForA? 5: 8;
					}
				}

				/* Select the Tx rate based on the RSSI */
				TxRateIdx = MLME_SelectTxRate(pAd, pEntry, mcs, Rssi, RssiOffset);
				DBGPRINT(RT_TRACE, ("%s(%d) TxRateIdx %d mcs %d rssi %d\n", __FUNCTION__, __LINE__, TxRateIdx, mcs, Rssi));

				if (TxRateIdx != pEntry->CurrTxRateIndex)
				{
					pEntry->CurrTxRateIndex = TxRateIdx;
					MLME_NewTxRate(pAd, pEntry);
					DBGPRINT(RT_TRACE, ("DRS: TxTotalCnt <= 15, switch MCS according to RSSI (%d), RssiOffset=%d\n", Rssi, RssiOffset));
				}

				MLME_ClearAllTxQuality(pEntry);

				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					continue;
				}
				else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					break;
				}
			}

			/* Select rate based on PER */
			MLME_OldRateAdapt(pAd, pEntry, CurrRateIdx, UpRateIdx, DownRateIdx, TrainUp, TrainDown, TxErrorRatio);	
		}
		else
			break;
	}
}

