#ifndef __ASIC_CTRL_H__
#define __ASIC_CTRL_H__

#include "types.h"
#include "rtmp_if.h"
#include "cipher.h"
#include "rtmp.h"


VOID AsicInitBcnBuf(
	IN PRTMP_ADAPTER 	pAd);


VOID AsicSetBssid(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pBssid);


VOID AsicSetSynInfo(
	IN PRTMP_ADAPTER 	pAd);


VOID AsicEnableIbssSync(
	IN PRTMP_ADAPTER 	pAd);



VOID AsicDisableIbssSync(			/* Maybe to be replaced with AsicDisableIbssSync */
	IN PRTMP_ADAPTER 	pAd);


VOID AsicDisableSync(				/* Driver version */
	IN PRTMP_ADAPTER 	pAd);


VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey);


VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Wcid);


VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey);


VOID AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag);


VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER 	pAd,
	IN USHORT			WCID,
	IN ULONG        	uIV,
	IN ULONG        	uEIV);


VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER 	pAd,
	IN USHORT			WCID,
	IN PUCHAR        	pAddr);



BOOLEAN AsicWaitPDMAIdle(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				round, 
	IN INT 				wait_us);


VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER 	pAd,
	IN PEDCA_PARM	 	pEdcaParm);


VOID AsicFifoExtSet(
	IN PRTMP_ADAPTER 	pAd);
#endif /*__ASIC_CTRL_H__ */
