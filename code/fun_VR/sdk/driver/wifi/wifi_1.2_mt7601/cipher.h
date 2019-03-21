#ifndef __CIPHER_H__
#define __CIPHER_H__

#include "types.h"
#include "rtmp_if.h"
#include "rtmp.h"
#include "mgmtentry.h"


VOID RTMPSetCipherAlg(
	IN PRTMP_ADAPTER 	pAd);


VOID RTMPSetChipHWKey(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			key, 
	IN UCHAR 			len, 
	IN UINT 			enc_mode);


VOID RTMPSetWEPCfg(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			item, 
	IN PUCHAR 			key, 
	IN UCHAR 			len);


VOID RTMPUpdateWPAInfo(
	IN PRTMP_ADAPTER 	pAd,
	IN PUCHAR			ssid,
	IN UCHAR			len);



VOID RTMPSetWPACfg(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			item, 
	IN PUCHAR 			key, 
	IN UCHAR 			len);


VOID RTMPSetWcidSecurityInfo(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			BssIdx,
	IN UINT8			KeyIdx,
	IN UINT8			CipherAlg,
	IN UINT8			Wcid,
	IN UINT8			KeyTabFlag);


USHORT RTMPCheckValidateRSNIE(
	IN PRTMP_ADAPTER    pAd,
	IN PMGMTENTRY		pEntry,
	IN PUCHAR			pRsnIe,
	IN UCHAR			rsnie_len);


NDIS_STATUS RTMPApCheckRxError(
	IN PRTMP_ADAPTER 	pAd,
	IN PRXINFO_STRUC 	pRxInfo,
	IN UCHAR 			Wcid);


VOID RTMPAPRxDErrorHandle(
	IN PRTMP_ADAPTER 	pAd, 
	IN PRXINFO_STRUC 	pRxInfo, 
	IN PRXWI_STRUC 		pRxWI);


NDIS_STATUS	RTMPStaCheckRxError(
	IN PRTMP_ADAPTER 	pAd,
	IN PHEADER_802_11 	pHeader,	
	IN PRXWI_STRUC 		pRxWI,	
	IN PRXINFO_STRUC 	pRxInfo);

#if 0 // TKIP unused code..
BOOLEAN RTMPCheckTkipMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHEADER_802_11	pWifiHdr,
	IN 	PUCHAR			pData,
	IN	UINT32			DataSize,
	IN	CHAR			UserPriority);
#endif

VOID RTMPGetCipherInfo(
	IN PUCHAR 			pIEData, 
	IN INT 				len, 
	OUT PUCHAR 			auth,		/* Auth mode*/ 
	OUT PUCHAR 			uenc,		/* Unicast enc mode */
	OUT PUCHAR			menc);		/* Multicast enc mode */


VOID RTMPAddWEPPairwise(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			aid);

#endif /* __CIPHER_H__ */
