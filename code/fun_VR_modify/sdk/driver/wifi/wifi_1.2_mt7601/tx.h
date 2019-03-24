#ifndef __TX_H__
#define __TX_H__

#include "types.h"
#include "rtmp_if.h"
#include "rtmp.h"

NDIS_STATUS RTMPNICInitTxMem(
	IN PRTMP_ADAPTER 	pAd);


VOID RTMPNICTxReleaseMem(
	IN PRTMP_ADAPTER 	pAd);


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
	IN	PHTTRANSMIT_SETTING	pTransmit);


VOID RTMPWriteTxWI_Data(
	IN	PRTMP_ADAPTER	pAd,
	INOUT PTXWI_STRUC	pTxWI,
	IN	UINT8			wcid,
	IN	UINT32 			len,
	IN	UINT8			Phymode,
	IN	UINT8			McsRate,
	IN	UINT8			Fallback);


VOID RTMPWriteTxINFO(
	IN PTX_CONTEXT 		pTxContext, 
	IN ULONG 			FrameLen);


VOID RTMPWriteTxINFO_Data(
	IN	PRTMP_ADAPTER	pAd,
	INOUT PTXINFO_STRUC	pTxINFO,
	IN	UINT32 			len,
	IN  UCHAR			next);


VOID RTMPKickOutTx(
	IN PRTMP_ADAPTER 	pAd, 
	IN PTX_CONTEXT 		pTxBlk);


VOID StartEapol(
	IN PRTMP_ADAPTER 	pAd, 
	IN PMGMTENTRY 		pEntry, 
	IN PUCHAR 			ptr, 
	IN UINT32 			frameSize, 
	IN BOOLEAN 			encrypt);


VOID StartTx(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pDid, 
	IN PUCHAR 			ptr, 
	IN UINT32 			packetCount, 
	IN UINT32 			frameSize);


UCHAR ProtectTypeMatch(
	IN USHORT 			type);


#endif /* __TX_H__ */
