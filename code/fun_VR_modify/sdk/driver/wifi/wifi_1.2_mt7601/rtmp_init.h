#ifndef __RTMP_INIT_H__
#define __RTMP_INIT_H__

#include "rtmp_if.h"
#include "types.h"

NDIS_STATUS NICInitBBP(
	IN PRTMP_ADAPTER 	pAd);


NTSTATUS  NICInitializeAdapter(
	IN PRTMP_ADAPTER 	pAd);


VOID NicResetRawCounters(
	IN PRTMP_ADAPTER 	pAd);


NTSTATUS NICInitializeAsic(
	IN PRTMP_ADAPTER 	pAd);


VOID NICInitAsicFromEEPROM(
	IN	PRTMP_ADAPTER	pAd);


VOID NICAntCfgInit(
	IN  PRTMP_ADAPTER   pAd);


VOID NICReadEEPROMParameters(
	IN PRTMP_ADAPTER 	pAd);



#endif
