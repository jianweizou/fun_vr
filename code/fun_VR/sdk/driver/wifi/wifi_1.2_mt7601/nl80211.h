#ifndef __NL80211_H__
#define __NL80211_H__

#include "types.h"
#include "system.h"
#include "common.h"
#include "nl80211_hdr.h"
#include "rtmp_if.h"
#include "rtmp.h"
#include "wpa/cmm_wpa.h"


typedef struct _IE_lists {
	UCHAR	Addr2[MAC_ADDR_LEN];
	UCHAR 	ApAddr[MAC_ADDR_LEN];
	USHORT 	CapabilityInfo;
	USHORT 	ListenInterval;
	UCHAR 	SsidLen;
	UCHAR 	Ssid[MAX_LEN_OF_SSID];
	UCHAR 	SupportedRatesLen;
	UCHAR 	SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR 	RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR 	RSNIE_Len;
	BOOLEAN bWmmCapable;
	ULONG 	RalinkIe;
} IE_LISTS, *PIE_LISTS;


VOID BuildCommon802_11MgtMacHeader(
	IN PRTMP_ADAPTER		pAd, 
	IN OUT PHEADER_802_11 	pHdr80211, 
	IN UCHAR 				SubType, 
	IN UCHAR 				ToDs, 
	IN PUCHAR 				pDA, 
	IN PUCHAR 				pBssid);


VOID BuildCommon802_11DataHeader(
	IN PRTMP_ADAPTER		pAd,
	INOUT PHEADER_802_11	pHeader80211,
	IN PUCHAR				pAddr,
	IN BOOLEAN				encrypt);

BOOLEAN MakeIbssBeacon(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAVCmd,
	IN USHORT				AVCmdLen);


ULONG MakeOutgoingFrame(
	OUT PUCHAR 				pBuffer, 
	OUT PULONG	 			pFrameLen, ...);


BOOLEAN SendIbssProbeReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PTX_CONTEXT 			pTxContext);


BOOLEAN SendIbssProbeRsp(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext);


BOOLEAN SendAuthPacket(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN UCHAR				seq,
	IN USHORT				status);


BOOLEAN SendAuthRspAtSeq2Action(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN PUCHAR				pData);


BOOLEAN SendAssocReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext);


BOOLEAN SendAssociateRsp(
	IN PRTMP_ADAPTER 		pAd,
	IN PMGMTENTRY			pEntry,
	IN PTX_CONTEXT 			pTxContext,
	IN USHORT 				StatusCode);


BOOLEAN SendDeAuthReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN UCHAR 				reason);


BOOLEAN SendDisassocReq(
	IN PRTMP_ADAPTER 		pAd,
	IN PUCHAR				pAddr,
	IN PTX_CONTEXT 			pTxContext,
	IN UCHAR 				reason);


BOOLEAN PeerAssocReqCmmSanity(
	IN PRTMP_ADAPTER 		pAd,
	IN PHEADER_802_11		pHdr80211,
	BOOLEAN 				isReassoc,
	INT 					MsgLen,
	IE_LISTS 				*ie_lists);


BOOLEAN PeerAuthSanity(
    IN PRTMP_ADAPTER 		pAd, 
    IN PHEADER_802_11		pHdr80211,
    IN ULONG 				MsgLen,
    IN PUSHORT				pAlg) ;


NDIS_STATUS APCheckRxError(
	IN PRTMP_ADAPTER 		pAd,
	IN PRXINFO_STRUC 		pRxInfo,
	IN UCHAR 				Wcid);


VOID APRxDErrorHandle(
	IN PRTMP_ADAPTER 		pAd, 
	IN PRXINFO_STRUC 		pRxInfo, 
	IN PRXWI_STRUC 			pRxWI);


NDIS_STATUS	STACheckRxError(
	IN PRTMP_ADAPTER 		pAd,
	IN PHEADER_802_11 		pHeader,	
	IN PRXWI_STRUC 			pRxWI,	
	IN PRXINFO_STRUC 		pRxInfo);


BOOLEAN RTMPCheckBeacon(
	IN PRTMP_ADAPTER 		pAd, 
	IN PUCHAR 				pData, 
	IN USHORT 				len);


BOOLEAN RTMPCheckAssociation(
	IN PRTMP_ADAPTER 		pAd, 
	IN PUCHAR 				pData);


UCHAR RTMPGetAPChannel(
	IN PUCHAR 				pData, 
	IN USHORT 				len);


VOID ScanSmartConfigBeacon(
	IN	PRTMP_ADAPTER		pAd);


VOID ParseBeacon(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN USHORT 			len);

VOID CollectScanBeacon(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN USHORT 			len, 
	IN CHAR 			rssi);

VOID DeleteScanList(
	IN PRTMP_ADAPTER 	pAd);
#endif /* __FRAME_H__ */
