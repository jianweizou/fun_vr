#ifndef __MT7601_CTRL_H__
#define __MT7601_CTRL_H__

#include "types.h"
#include "rtmp.h"
#include "rtmp_if.h"

#define MT7601_VALID_EEPROM_VERSION			0x0C


VOID NICInitMT7601BbpRegisters(
	IN	PRTMP_ADAPTER 	pAd);


VOID NICInitMT7601MacRegisters(
	IN PRTMP_ADAPTER 	pAd);


VOID NICInitMT7601RFRegisters(
   IN PRTMP_ADAPTER    pAd);


VOID MT7601_InitTemperatureCompensation(
	IN PRTMP_ADAPTER	pAd);


VOID MT7601_INIT_CAL(
	IN PRTMP_ADAPTER 	pAd);


VOID MT7601_RXDC_CAL(
	IN PRTMP_ADAPTER 	pAd);


VOID MT7601_TssiDcGainCalibration(
	IN PRTMP_ADAPTER 		pAd);


VOID MT7601_AsicTemperatureCompensation(
	IN PRTMP_ADAPTER 	pAd,
	IN BOOLEAN			bPowerOn);


VOID MT7601_AsicAntennaDefaultReset(
	IN PRTMP_ADAPTER			pAd,
	IN PEEPROM_ANTENNA_STRUC 	pAntenna);


VOID MT7601_AsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd);


VOID MT7601_ChipSwitchChannel(	
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel,
	IN BOOLEAN	 		bScan);


VOID MT7601_ChipSwitchChannel_ex(
   IN PRTMP_ADAPTER	   pAd,
   IN UCHAR 		   Channel,
   IN BOOLEAN		   bScan);


VOID MT7601_ReadTxPwrPerRate(
	IN PRTMP_ADAPTER 	pAd);


INT MT7601_ReadChannelPwr(
	IN PRTMP_ADAPTER 	pAd);


VOID MT7601_SetMacRegisterOfPBF(
	IN UINT32 	ulValue);


NDIS_STATUS MT7601_BBP_read(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			regID,
	IN PUCHAR 			pValue);


NDIS_STATUS MT7601_BBP_write(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			regID,
	IN UCHAR 			value);


BOOLEAN MT7601_Bootup_Read_Temperature(
	IN	PRTMP_ADAPTER	pAd, 
	OUT	PUCHAR			pTemperature);


VOID MT7601_SetEDCCA(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			enable);


VOID MT7601_MacInit(
	IN PRTMP_ADAPTER 	pAd);


#endif
