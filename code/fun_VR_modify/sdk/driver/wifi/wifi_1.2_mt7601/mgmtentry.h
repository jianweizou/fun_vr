#ifndef __MGMTENTRY_H__
#define __MGMTENTRY_H__

#include "types.h"
#include "rtmp_timer.h"
#include "rtmp_if.h"
#include "wpa/cmm_wpa.h"

//#define AUTO_DEL_MGMT_NODE
#define THRESHOLD_MS 					90000
#define RX_TIME_INTERVAL_THRESHOLD		THRESHOLD_MS/portTICK_RATE_MS

//#define PS_TX_QUEUE_DATA

#ifdef PS_TX_QUEUE_DATA
typedef struct _BufferCell{
	PUCHAR  pPacket;
	ULONG	packetLen;
	struct  _BufferCell  *pNextCell;
} BufferCell, *pBufferCell;
#endif

#define MAX_TX_RATE_INDEX			12//33		/* Maximum Tx Rate Table Index value */

struct _MGMTENTRY {
	UCHAR	Addr[6];
	BOOLEAN	bActive;
	ULONG 	rx_time;
	CHAR	rx_stregth;
	USHORT 	Aid;
	UCHAR 	ANonce[LEN_KEY_DESC_NONCE];
	UCHAR 	SNonce[LEN_KEY_DESC_NONCE];
	UCHAR 	R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR 	PTK[64];
	USHORT 	CapabilityInfo;
	CIPHER_KEY PairwiseKey;
	UCHAR	Linkstat;
	UCHAR 	ReTryCounter;
	UCHAR 	WpaState;
	UCHAR 	GTKState;
	RTMP_SONIX_TIMER RetryTimer;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	PVOID	pAd;
	UCHAR 	CMTimerRunning;
#ifdef PS_TX_QUEUE_DATA	
	pBufferCell  	pPSTxCell;
#endif
	struct _MGMTENTRY *next_node;
	struct _MGMTENTRY *pre_node;

	/* Rate adapt */
	UCHAR	CurrTxRateIndex;
	UCHAR	CurrTxRate;
	UCHAR	PhyMode;
	UCHAR	*pTable;	/* Pointer to this entry's Tx Rate Table */
	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT	TxQuality[MAX_TX_RATE_INDEX + 1];
	UCHAR	TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
};


PMGMTENTRY MGMTENTRY_GetNode(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr);

PMGMTENTRY MGMTENTRY_SearchNodeByWCID(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			wcid);

VOID MGMTENTRY_Init(
	IN PRTMP_ADAPTER 	pAd);

BOOLEAN MGMTENTRY_Add(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr);

BOOLEAN MGMTENTRY_Del(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr);

BOOLEAN MGMTENTRY_SetLinkStat(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr, 
	IN UCHAR 			stat);

BOOLEAN MGMTENTRY_UpdateInfo(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr, 
	IN ULONG 			rx_time, 
	IN UCHAR 			rx_stregth);


VOID MGMTENTRY_ReleaseAll(
	IN PRTMP_ADAPTER 	pAd);

VOID MGMTENTRY_GetAllInfo(
	IN PRTMP_ADAPTER 	pAd);

#endif /* __MGMTENTRY_H__ */
