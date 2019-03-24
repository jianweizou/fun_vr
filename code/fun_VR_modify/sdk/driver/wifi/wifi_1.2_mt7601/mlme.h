#ifndef __MLME_H__
#define __MLME_H__

#include "types.h"
#include "rtmp_if.h"
#include "common.h"
#include "system.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


VOID MLME_ProcessDataFrame(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pData, 
	IN UINT 			len);

VOID MLME_DoDisconnect(
	IN PRTMP_ADAPTER 	pAd, 
	IN BOOLEAN 			bReconnect);


VOID MLME_DoConnect(	
	IN PRTMP_ADAPTER  	pAd);


VOID MLME_Startdhcpc(
	IN PRTMP_ADAPTER  	pAd);


VOID MLME_SetScanFlag(
	IN	PRTMP_ADAPTER	pAd,
	IN 	UINT8 			Flag);


UCHAR MLME_GetScanFlag(
	IN PRTMP_ADAPTER	pAd);


VOID MLME_SetRxBcnCount(
	IN PRTMP_ADAPTER 	pAd, 
	IN ULONG 			Cnt);


ULONG MLME_GetRxBcnCount(
	IN PRTMP_ADAPTER 	pAd);


BOOLEAN MLME_StoreStaConfig(
	IN PRTMP_ADAPTER 	pAd);


BOOLEAN MLME_SetDefaultAp(
	IN PRTMP_ADAPTER 	pAd);
	

VOID MLME_WriteParamToNVARAM(
	IN SNX_DEV_INFO_ID 	id, 
	IN PVOID 			pdata, 
	IN UINT32 			len);


NDIS_STATUS MLME_ReadParamFromNVARAM(
	IN SNX_DEV_INFO_ID 	id, 
	OUT PVOID 			pdata, 
	IN UINT32 			len);


NDIS_STATUS MLME_GetConfig(
	IN PRTMP_ADAPTER 	pAd);


VOID MLME_GetOperConfig(
	IN PRTMP_ADAPTER 	pAd);


NTSTATUS MLME_InitHardwre(
	IN PRTMP_ADAPTER 	pAd);


VOID MLME_ModeInit(
	IN PRTMP_ADAPTER 	pAd);


NTSTATUS MLME_AdapterInit(
	IN PRTMP_ADAPTER 	pAd);

VOID MLME_StartAPConnect(
	IN PRTMP_ADAPTER 	pAd);

VOID WiFiScanTask(
	IN PVOID id);

VOID WiFiEdccaTask(
	IN PVOID 			id);

VOID MLME_Startdhcpc(
	IN PRTMP_ADAPTER  	pAd);


#endif
