#include <stdlib.h>
#include "mt7601_ctrl.h"
#include "mt7601_table.h"
#include "rtmp_andes.h"
#include "rtmp_ctrl.h"
#include "usb_io.h"

#define MT7601_E2_TEMPERATURE_SLOPE		39


#define ANDES_CALIBRATION_R				1
#define ANDES_CALIBRATION_LC			3
#define ANDES_CALIBRATION_LOFT			4
#define ANDES_CALIBRATION_TXIQ			5
#define ANDES_CALIBRATION_BW			6
#define ANDES_CALIBRATION_DPD			7
#define ANDES_CALIBRATION_RXIQ			8
#define ANDES_CALIBRATION_TXDCOC		9


#define MAX_RXDCOC_RETRY_CNT			20


#define DEFAULT_BO              		4
#define LIN2DB_ERROR_CODE       		(-10000)


enum TEMPERATURE_MODE {
	TEMPERATURE_MODE_NORMAL,
	TEMPERATURE_MODE_LOW,
	TEMPERATURE_MODE_HIGH,
};


#define	NUM_MAC_REG_PARMS				(sizeof(MACRegTable) / sizeof(RTMP_REG_PAIR))
//#define	NUM_STA_MAC_REG_PARMS			(sizeof(STAMACRegTable) / sizeof(RTMP_REG_PAIR))
//#define	NUM_BCN_SPEC_MAC_REG_PARMS		(sizeof(BcnSpecMACRegTable) / sizeof(RTMP_REG_PAIR))


static SHORT lin2dBd(USHORT linearValue)
{
	SHORT 	exp;
	UINT 	mantisa;
	INT 	app, dBd;
	
	/* Default backoff ; to enhance leading bit searching time */
	mantisa = linearValue << DEFAULT_BO;
	exp = -(DEFAULT_BO);
	/* Leading bit searching */
	if (mantisa < (0x8000)) {
		while (mantisa < (0x8000)) {
			mantisa = mantisa << 1; // no need saturation
			exp--;
			if (exp < -20) {
				return LIN2DB_ERROR_CODE;
			}
		}
	}
	else {
		while (mantisa > (0xFFFF)) {
			mantisa = mantisa >> 1; // no need saturation
			exp ++;
			if (exp > 20) {
				return LIN2DB_ERROR_CODE;
			}
		}
	}

	if (mantisa <= 47104) {
		app = (mantisa + (mantisa >> 3) + (mantisa >> 4) - 38400); //S(15,0)
		if (app < 0) {app = 0;}
	}
	else {
		app = (mantisa - (mantisa >> 3) - (mantisa >> 6) - 23040); //S(15,0)
		if (app < 0) {app = 0;}
	}

	/* since 2^15=1 here */
	dBd = ((15 + exp) << 15) + app;		
	dBd = (dBd << 2) + (dBd << 1) + (dBd >> 6) + (dBd >> 7);
	dBd = (dBd >> 10); //S10.5
	return(dBd);
}


VOID NICInitMT7601BbpRegisters(
	IN	PRTMP_ADAPTER 	pAd)
{
	DBGPRINT(RT_TRACE, ("[TRACE]--->%s\r\n", __FUNCTION__));

	//AndesBBPRandomWritePair(pAd, MT7601_BBP_InitRegTb, MT7601_BBP_InitRegTb_Size);

	char* tempMT7601_BBP_InitRegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_InitRegTb) +0xF);
	if (tempMT7601_BBP_InitRegTb == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
		return;
	}
	unsigned int tempMT7601_BBP_InitRegTbAddr = (UINT32)(tempMT7601_BBP_InitRegTb+ 0xF) & 0xFFFFFFF0;
	memcpy((void*)tempMT7601_BBP_InitRegTbAddr,MT7601_BBP_InitRegTb,sizeof(MT7601_BBP_InitRegTb));
	AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_InitRegTbAddr, MT7601_BBP_InitRegTb_Size);
	vPortFree(tempMT7601_BBP_InitRegTb);

	return;
}



VOID NICInitMT7601MacRegisters(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32 MacReg = 0;

	DBGPRINT(RT_TRACE, ("[TRACE]--->%s\r\n", __FUNCTION__));

	/*
		Enable PBF and MAC clock
		SYS_CTRL[11:10] = 0x3
	*/	
	//AndesRandomWritePair(pAd, MT7601_MACRegTable, MT7601_NUM_MAC_REG_PARMS);

	char* tempMT7601_MACRegTable = (char*)pvPortMalloc(sizeof(MT7601_MACRegTable) + 0xF);
	if (tempMT7601_MACRegTable == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
		return;
	}
	unsigned int tempMT7601_MACRegTableAddr = (UINT32)(tempMT7601_MACRegTable+ 0xF) & 0xFFFFFFF0;
	memcpy((void*)tempMT7601_MACRegTableAddr,MT7601_MACRegTable,sizeof(MT7601_MACRegTable));
	AndesRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_MACRegTableAddr, MT7601_NUM_MAC_REG_PARMS);
	vPortFree(tempMT7601_MACRegTable);
	
	/*
		Release BBP and MAC reset
		MAC_SYS_CTRL[1:0] = 0x0
	*/
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacReg);
	MacReg &= ~(0x3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacReg);
	RTMP_IO_WRITE32(pAd, AUX_CLK_CFG, 0);
}




 VOID NICInitMT7601RFRegisters(
 	IN PRTMP_ADAPTER 	pAd)
{
	DBGPRINT(RT_TRACE, ("[TRACE]--->%s\r\n", __FUNCTION__));
	
	RLT_RF_write(pAd, RF_BANK0, RF_R12, pAd->RfFreqOffset);

	//AndesRFRandomWritePair(pAd, MT7601_RF_Central_RegTb, MT7601_RF_Central_RegTb_Size);
	//AndesRFRandomWritePair(pAd, MT7601_RF_Channel_RegTb, MT7601_RF_Channel_RegTb_Size);
	//AndesRFRandomWritePair(pAd, MT7601_RF_VGA_RegTb, MT7601_RF_VGA_RegTb_Size);

	char* tempMT7601_RF_Central_RegTb = (char*)pvPortMalloc(sizeof(MT7601_RF_Central_RegTb)+0xF);
	if (tempMT7601_RF_Central_RegTb == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
		return;
	}
	unsigned int tempMT7601_RF_Central_RegTbAddr = (UINT32)(tempMT7601_RF_Central_RegTb+ 0xF) & 0xFFFFFFF0;
	memcpy((void*)tempMT7601_RF_Central_RegTbAddr,MT7601_RF_Central_RegTb,sizeof(MT7601_RF_Central_RegTb));
	AndesRFRandomWritePair(pAd, (BANK_RF_REG_PAIR*)tempMT7601_RF_Central_RegTbAddr, MT7601_RF_Central_RegTb_Size);
	vPortFree(tempMT7601_RF_Central_RegTb);

	char* tempMT7601_RF_Channel_RegTb = (char*)pvPortMalloc(sizeof(MT7601_RF_Channel_RegTb)+0xF);
	if (tempMT7601_RF_Channel_RegTb == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
		return;
	}
	unsigned int tempMT7601_RF_Channel_RegTbAddr = (UINT32)(tempMT7601_RF_Channel_RegTb+ 0xF) & 0xFFFFFFF0;
	memcpy((void*)tempMT7601_RF_Channel_RegTbAddr,MT7601_RF_Channel_RegTb,sizeof(MT7601_RF_Channel_RegTb));
	AndesRFRandomWritePair(pAd, (BANK_RF_REG_PAIR*)tempMT7601_RF_Channel_RegTbAddr, MT7601_RF_Channel_RegTb_Size);
	vPortFree(tempMT7601_RF_Channel_RegTb);

	char* tempMT7601_RF_VGA_RegTb = (char*)pvPortMalloc(sizeof(MT7601_RF_VGA_RegTb) + 0xF);
	if (tempMT7601_RF_VGA_RegTb == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
		return;
	}
	unsigned int tempMT7601_RF_VGA_RegTbAddr = (UINT32)(tempMT7601_RF_VGA_RegTb+ 0xF) & 0xFFFFFFF0;
	memcpy((void*)tempMT7601_RF_VGA_RegTbAddr,MT7601_RF_VGA_RegTb,sizeof(MT7601_RF_VGA_RegTb));
	AndesRFRandomWritePair(pAd, (BANK_RF_REG_PAIR*)tempMT7601_RF_VGA_RegTbAddr, MT7601_RF_VGA_RegTb_Size);
	vPortFree(tempMT7601_RF_VGA_RegTb);
	
	MT7601_INIT_CAL(pAd);
}



VOID MT7601_InitTemperatureCompensation(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 Value = 0;

	pAd->chipCap.TemperatureMode = TEMPERATURE_MODE_NORMAL;
	pAd->chipCap.CurrentTemperature = 0;
	pAd->chipCap.bPllLockProtect = FALSE;
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TARGET_POWER, Value);
	pAd->chipCap.TemperatureRef25C = (Value >> 8) & 0xFF;
	DBGPRINT(RT_TRACE, ("[TRCAE]%s: TemperatureRef25C = 0x%x\r\n", __FUNCTION__, pAd->chipCap.TemperatureRef25C));

}


VOID MT7601_INIT_CAL(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RfValue;
	UINT32 Mac_R1004;
	
	DBGPRINT(RT_TRACE, ("[TRACE]--->%s\r\n", __FUNCTION__));


	MT7601_Bootup_Read_Temperature(pAd, (PUCHAR)&pAd->chipCap.CurrentTemperBbpR49);
	pAd->chipCap.CurrentTemperature = (pAd->chipCap.CurrentTemperBbpR49 - pAd->chipCap.TemperatureRef25C) * MT7601_E2_TEMPERATURE_SLOPE;
#ifdef DPD_CALIBRATION_SUPPORT
	pAd->chipCap.TemperatureDPD = pAd->chipCap.CurrentTemperature;
#endif /* DPD_CALIBRATION_SUPPORT */

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Mac_R1004);

	/* R Calibration */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_R, 0);

	/*  VCO Cal */
	RLT_RF_read(pAd, RF_BANK0, RF_R04, &RfValue);
	RfValue = RfValue | 0x80; 
	RLT_RF_write(pAd, RF_BANK0, RF_R04, RfValue);
	RTMPusecDelay(2000);

	/* TXDC */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_TXDCOC, 0);

	/* Tx Filter BW */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_BW, 0x00001);

	/* Rx Filter BW */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_BW, 0x00000);

	/* TXLOFT */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_LOFT, 0);

	/* TXIQ */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_TXIQ, 0);

	/* RXIQ */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_RXIQ, 0);

#ifdef DPD_CALIBRATION_SUPPORT
	/* DPD-Calibration */
	AndesCalibrationOP(pAd, ANDES_CALIBRATION_DPD, pAd->chipCap.CurrentTemperature);
#endif /* DPD_CALIBRATION_SUPPORT */

	/* MT7601_RXDC_CAL */
	MT7601_RXDC_CAL(pAd);

#ifdef RTMP_INTERNAL_TX_ALC
	MT7601_TssiDcGainCalibration(pAd);
#endif /* RTMP_INTERNAL_TX_ALC */

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Mac_R1004);

	MT7601_AsicTemperatureCompensation(pAd, TRUE);

	DBGPRINT(RT_TRACE, ("[TRCAE]<---%s\r\n", __FUNCTION__));
}


BOOLEAN MT7601_Bootup_Read_Temperature(
	IN	PRTMP_ADAPTER	pAd, 
	OUT	PUCHAR			pTemperature)
{
	UINT32	MAC0504, MAC050C;
	UCHAR	BBPReg;
	int i;

	RTMP_IO_READ32(pAd, 0x50C, &MAC050C);
	RTMP_IO_READ32(pAd, 0x504, &MAC0504);

	RTMP_IO_WRITE32(pAd, 0x504, 0x0);
	RTMP_IO_WRITE32(pAd, 0x50C, 0x10);
	RTMP_IO_WRITE32(pAd, 0x504, 0x10);

	/* BBP R47[4] = 1 */
	MT7601_BBP_read(pAd, BBP_R47, &BBPReg);
	BBPReg &= (~0x7f);
	BBPReg |= 0x10;
	MT7601_BBP_write(pAd, BBP_R47, BBPReg);

	MT7601_BBP_write(pAd, BBP_R22, 0x40);

	i = 100;
	while ( i > 0 )
	{
		MT7601_BBP_read(pAd, BBP_R47, &BBPReg);
		if ( (BBPReg & 0x10) == 0)
		{
			break;
		}
		i--;
	}

	BBPReg = (BBPReg & ~0x07) | 0x04;
	MT7601_BBP_write(pAd, BBP_R47, BBPReg);
	RTMPusecDelay(500);
	MT7601_BBP_read(pAd, BBP_R49, pTemperature);
	DBGPRINT(RT_TRACE, ("[TRCAE]%s: Boot up temperature = 0x%X\r\n", 
						__FUNCTION__, *pTemperature));

	MT7601_BBP_write(pAd, BBP_R22, 0x0);
	MT7601_BBP_read(pAd, BBP_R21, &BBPReg);
	BBPReg |= 0x2;
	MT7601_BBP_write(pAd, BBP_R21, BBPReg);
	BBPReg &= ~0x2;
	MT7601_BBP_write(pAd, BBP_R21, BBPReg);

	RTMP_IO_WRITE32(pAd, 0x504, 0);
	RTMP_IO_WRITE32(pAd, 0x50C, MAC050C);
	RTMP_IO_WRITE32(pAd, 0x504, MAC0504);

	return TRUE;
}



VOID MT7601_RXDC_CAL(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT count;
	UCHAR RValue;
	UINT32 Mac_R1004;

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Mac_R1004);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x08);

	/* 1012 BBP CR */
	AndesBBPRandomWrite(pAd, 4,
						BBP_R158, 0x8D,
						BBP_R159, 0xFC,
						BBP_R158, 0x8C,
						BBP_R159, 0x4C);

	for (count = 0; count < MAX_RXDCOC_RETRY_CNT; count++)
	{
		vTaskDelay(1/portTICK_RATE_MS); 
		MT7601_BBP_write(pAd, BBP_R158, 0x8C);
		MT7601_BBP_read(pAd, BBP_R159, &RValue);

		if ( RValue == 0x0c )
			break;
	}

	if (count == MAX_RXDCOC_RETRY_CNT )
	{
		/* FIXME: do something else @Betters */
		DBGPRINT(RT_ERROR, ("[ERR]%s: MT7601_RXDC_CAL Fail!\r\n", __FUNCTION__)); 
	}

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

	/* 0725 BBP CR change DCOC sequence. */
	AndesBBPRandomWrite(pAd, 2,
						BBP_R158, 0x8D,
						BBP_R159, 0xE0);

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Mac_R1004);
}



VOID MT7601_TssiDcGainCalibration(
	IN PRTMP_ADAPTER 		pAd)
{
	UCHAR Rf_B5_R03, Rf_B4_R39, bbp_r47;
	INT i, count;
	UCHAR RValue;
	INT16 tssi_linear;
	INT16 tssi0_db = 0, tssi0_db_hvga = 0;
	MT7601_TX_ALC_DATA *pTxALCData = &pAd->chipCap.TxALCData;

	/* Mac Bypass */
	RTMP_IO_WRITE32(pAd, 0x50C, 0x30);
	RTMP_IO_WRITE32(pAd, 0x504, 0xC0030);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	MT7601_BBP_write(pAd, BBP_R58, 0x0);
	MT7601_BBP_write(pAd, BBP_R241, 0x2);
	MT7601_BBP_write(pAd, BBP_R23, 0x8);
	MT7601_BBP_read(pAd, BBP_R47, &bbp_r47);


	/* Set VGA gain */
	RLT_RF_read(pAd, RF_BANK5, RF_R03, &Rf_B5_R03);
	RLT_RF_write(pAd, RF_BANK5, RF_R03, 0x8);

	/* Mixer disable */
	RLT_RF_read(pAd, RF_BANK4, RF_R39, &Rf_B4_R39);
	RLT_RF_write(pAd, RF_BANK4, RF_R39, 0x0);

	for( i = 0; i < 4; i++)
	{
		if ( ( i == 0 ) || ( i == 2 ) )
			RLT_RF_write(pAd, RF_BANK4, RF_R39, 0x0);		//disable PA
		else
			RLT_RF_write(pAd, RF_BANK4, RF_R39, Rf_B4_R39);	// enable PA

		if ( i < 2)
		{
 		 	MT7601_BBP_write(pAd, BBP_R23, 0x8);
			RLT_RF_write(pAd, RF_BANK5, RF_R03, 0x8);
		}
		else
		{ 	
 		 	MT7601_BBP_write(pAd, BBP_R23, 0x2);
			RLT_RF_write(pAd, RF_BANK5, RF_R03, 0x11);
		}

		/* BBP TSSI initial and soft reset */
		MT7601_BBP_write(pAd, BBP_R22  , 0x0);
		MT7601_BBP_write(pAd, BBP_R244, 0x0);	
		
		MT7601_BBP_write(pAd, BBP_R21, 0x1);
		RTMPusecDelay(1);
		MT7601_BBP_write(pAd, BBP_R21, 0x0);

		/* TSSI measurement */
		MT7601_BBP_write(pAd, BBP_R47, 0x50); 
		if ((i == 0) || (i == 2))
			MT7601_BBP_write(pAd, BBP_R22, 0x40);	// enable dc
		else
			MT7601_BBP_write(pAd, BBP_R244, 0x31);	// enable ton gen
		
		for (count = 0; count < 20; count++)
		{
			MT7601_BBP_read(pAd, BBP_R47, &RValue);

			if ((RValue & 0x10) == 0x00)
				break;
		}
		
		if (count == 20) 
		{
			/* FIXME: do something else @Betters */
			DBGPRINT(RT_ERROR, ("[ERR]%s: TssiDC0 Read Fail!\r\n", __FUNCTION__));
		}

		/* TSSI read */
		MT7601_BBP_write(pAd, BBP_R47, 0x40);
		if (i == 0)
			MT7601_BBP_read(pAd, BBP_R49, (PUCHAR)&pTxALCData->TssiDC0);
		else if (i == 2)
			MT7601_BBP_read(pAd, BBP_R49, (PUCHAR)&pTxALCData->TssiDC0_HVGA);
		else 
			MT7601_BBP_read(pAd, BBP_R49, (PUCHAR)&tssi_linear);

		tssi_linear = tssi_linear & 0xff;
		tssi_linear = (tssi_linear & 0x80) ? tssi_linear - 0x100 : tssi_linear;   

		if (i==1)
			tssi0_db = lin2dBd(tssi_linear - pTxALCData->TssiDC0);
		else if (i == 3)
		{
			tssi_linear = tssi_linear - pTxALCData->TssiDC0_HVGA;
			tssi_linear = tssi_linear * 4;
			tssi0_db_hvga = lin2dBd(tssi_linear);
		}
			
       }

	pTxALCData->TSSI0_DB = tssi0_db;
	DBGPRINT(RT_TRACE, ("[TRACE]tssi0_db_hvga = %x\r\n", tssi0_db_hvga));
	DBGPRINT(RT_TRACE, ("[TRACE]tssi0_db = %x\r\n", tssi0_db));

	pTxALCData->TSSI_DBOFFSET_HVGA = tssi0_db_hvga - tssi0_db;
	MT7601_BBP_write(pAd, BBP_R22, 0x0);
	MT7601_BBP_write(pAd, BBP_R244, 0x0);
	MT7601_BBP_write(pAd, BBP_R21, 0x1);
	RTMPusecDelay(1);
	MT7601_BBP_write(pAd, BBP_R21, 0x0);
	RTMP_IO_WRITE32(pAd, 0x504, 0x0);
	RTMP_IO_WRITE32(pAd, 0x50C, 0x0);
	RLT_RF_write(pAd, RF_BANK5, RF_R03, Rf_B5_R03);
	RLT_RF_write(pAd, RF_BANK4, RF_R39, Rf_B4_R39);
	MT7601_BBP_write(pAd, BBP_R47, bbp_r47);
	// -(tssi0_db*slope +tssi0_db) / 4096
	DBGPRINT(RT_TRACE, ("[TRACE]TssiDC0 = %d (0x%x)\r\n", pTxALCData->TssiDC0, pTxALCData->TssiDC0));
	DBGPRINT(RT_TRACE, ("[TRACE]TssiDC0_HVGA = %d (0x%x)\r\n", pTxALCData->TssiDC0_HVGA, pTxALCData->TssiDC0));
	DBGPRINT(RT_TRACE, ("[TRACE]TSSI_DBOFFSET_HVGA = %x\r\n", pTxALCData->TSSI_DBOFFSET_HVGA));
}



VOID MT7601_AsicTemperatureCompensation(
	IN PRTMP_ADAPTER 	pAd,
	IN BOOLEAN			bPowerOn)
{
	INT32	CurrentTemper;
	UCHAR	RfReg;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	INT32 high_temp_cr_threshold, low_temp_cr_threshold;

	CurrentTemper = (pChipCap->CurrentTemperBbpR49 - pChipCap->TemperatureRef25C) * MT7601_E2_TEMPERATURE_SLOPE;		// 3.9 * 10
	pChipCap->CurrentTemperature = CurrentTemper;

#ifdef DPD_CALIBRATION_SUPPORT
	/* DPD Calibration */
	if ( ((CurrentTemper - pChipCap->TemperatureDPD) > 450) || ((CurrentTemper - pChipCap->TemperatureDPD) < -450 ))
	{
		pChipCap->TemperatureDPD = CurrentTemper;

		/* DPD-Calibration */
		AndesCalibrationOP(pAd, ANDES_CALIBRATION_DPD, pChipCap->TemperatureDPD);

		RLT_RF_write(pAd, RF_BANK0, RF_R04, 0x0A);
		RLT_RF_write(pAd, RF_BANK0, RF_R05, 0x20);
		RLT_RF_read(pAd, RF_BANK0, RF_R04, &RfReg);
		RfReg = RfReg | 0x80; 
		RLT_RF_write(pAd, RF_BANK0, RF_R04, RfReg);
		RTMPusecDelay(2000);
	
		DBGPRINT(RT_TRACE, ("%s::ReCalibration DPD.\r\n", __FUNCTION__));
	}
#endif /* DPD_CALIBRATION_SUPPORT */

	/* PLL Lock Protect */
	if ( CurrentTemper < -50 )												// ( 20 - 25 ) * 10 = -50
	{
		if ( pAd->chipCap.bPllLockProtect == FALSE )
		{
			pAd->chipCap.bPllLockProtect = TRUE;
			RLT_RF_write(pAd, RF_BANK4, RF_R04, 0x06);

			RLT_RF_read(pAd, RF_BANK4, RF_R10, &RfReg);
			RfReg = RfReg & ~0x30; 
			RLT_RF_write(pAd, RF_BANK4, RF_R10, RfReg);

			DBGPRINT(RT_TRACE, ("%s::Enable PLL Lock Protect.\r\n", __FUNCTION__));
		}
	}
	else if ( CurrentTemper > 50 )											// ( 30 - 25 ) * 10 = 50
	{
		if ( pAd->chipCap.bPllLockProtect == TRUE )
		{
			pAd->chipCap.bPllLockProtect = FALSE;
			RLT_RF_write(pAd, RF_BANK4, RF_R04, 0x0);

			RLT_RF_read(pAd, RF_BANK4, RF_R10, &RfReg);
			RfReg = (RfReg & ~0x30) | 0x10;
			RLT_RF_write(pAd, RF_BANK4, RF_R10, RfReg);

			DBGPRINT(RT_TRACE, ("%s::Disable PLL Lock Protect.\r\n", __FUNCTION__));
		}
	}

	if ( bPowerOn )
	{
		high_temp_cr_threshold = 350;
		low_temp_cr_threshold = -250;
	}
	else
	{
		high_temp_cr_threshold = 400;
		low_temp_cr_threshold = -200;
	}


	/* BBP CR for H, L, N temperature */
	if ( CurrentTemper > high_temp_cr_threshold  )													// (60 - 25) * 10 = 350
	{
		if ( pChipCap->TemperatureMode != TEMPERATURE_MODE_HIGH )
		{
			pChipCap->TemperatureMode = TEMPERATURE_MODE_HIGH;

			//AndesBBPRandomWritePair(pAd, MT7601_BBP_HighTempCommonRegTb, MT7601_BBP_HighTempCommonRegTb_Size);
			
			char* tempMT7601_BBP_HighTempCommonRegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_HighTempCommonRegTb) +0xF);
			if (tempMT7601_BBP_HighTempCommonRegTb == NULL)
			{
				DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
				return;
			}
			unsigned int tempMT7601_BBP_HighTempCommonRegTbAddr = (UINT32)(tempMT7601_BBP_HighTempCommonRegTb+ 0xF) & 0xFFFFFFF0;
			memcpy((void*)tempMT7601_BBP_HighTempCommonRegTbAddr,MT7601_BBP_HighTempCommonRegTb,sizeof(MT7601_BBP_HighTempCommonRegTb));
			AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_HighTempCommonRegTbAddr, MT7601_BBP_HighTempCommonRegTb_Size);
			vPortFree(tempMT7601_BBP_HighTempCommonRegTb);

			if (pAd->CommonCfg.BBPCurrentBW == BW_20 )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_HighTempBW20RegTb, MT7601_BBP_HighTempBW20RegTb_Size);

				char* tempMT7601_BBP_HighTempBW20RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_HighTempBW20RegTb)+0xF);
				if (tempMT7601_BBP_HighTempBW20RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_HighTempBW20RegTbAddr = (UINT32)(tempMT7601_BBP_HighTempBW20RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_HighTempBW20RegTbAddr,MT7601_BBP_HighTempBW20RegTb,sizeof(MT7601_BBP_HighTempBW20RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_HighTempBW20RegTbAddr, MT7601_BBP_HighTempBW20RegTb_Size);
				vPortFree(tempMT7601_BBP_HighTempBW20RegTb);
			}
			else if (pAd->CommonCfg.BBPCurrentBW == BW_40 )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_HighTempBW40RegTb, MT7601_BBP_HighTempBW40RegTb_Size);

				char* tempMT7601_BBP_HighTempBW40RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_HighTempBW40RegTb)+0xF);
				if (tempMT7601_BBP_HighTempBW40RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_HighTempBW40RegTbAddr = (UINT32)(tempMT7601_BBP_HighTempBW40RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_HighTempBW40RegTbAddr,MT7601_BBP_HighTempBW40RegTb,sizeof(MT7601_BBP_HighTempBW40RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_HighTempBW40RegTbAddr, MT7601_BBP_HighTempBW40RegTb_Size);
				vPortFree(tempMT7601_BBP_HighTempBW40RegTb);
			}
			else
				DBGPRINT(RT_ERROR, ("[ERR]%s::Unsupported BW(%x)\r\n", __FUNCTION__, pAd->CommonCfg.BBPCurrentBW));

			DBGPRINT(RT_TRACE, ("[TRACE]%s::Change to TEMPERATURE_MODE_HIGH\r\n", __FUNCTION__));
		}
	}
	else if ( CurrentTemper > low_temp_cr_threshold )									// ( 0 - 25 ) * 10
	{
		if ( pChipCap->TemperatureMode != TEMPERATURE_MODE_NORMAL )
		{
			pChipCap->TemperatureMode = TEMPERATURE_MODE_NORMAL;

			//AndesBBPRandomWritePair(pAd, MT7601_BBP_CommonRegTb, MT7601_BBP_CommonRegTb_Size);

			char* tempMT7601_BBP_CommonRegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_CommonRegTb)+0xF);
			if (tempMT7601_BBP_CommonRegTb == NULL)
			{
				DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
				return;
			}
			unsigned int tempMT7601_BBP_CommonRegTbAddr = (UINT32)(tempMT7601_BBP_CommonRegTb+ 0xF) & 0xFFFFFFF0;
			memcpy((void*)tempMT7601_BBP_CommonRegTbAddr,MT7601_BBP_CommonRegTb,sizeof(MT7601_BBP_CommonRegTb));
			AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_CommonRegTbAddr, MT7601_BBP_CommonRegTb_Size);
			vPortFree(tempMT7601_BBP_CommonRegTb);

			if (pAd->CommonCfg.BBPCurrentBW == BW_20 )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_BW20RegTb, MT7601_BBP_BW20RegTb_Size);

				char* tempMT7601_BBP_BW20RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_BW20RegTb) + 0xF);
				if (tempMT7601_BBP_BW20RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_BW20RegTbAddr = (UINT32)(tempMT7601_BBP_BW20RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_BW20RegTbAddr,MT7601_BBP_BW20RegTb,sizeof(MT7601_BBP_BW20RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_BW20RegTbAddr, MT7601_BBP_BW20RegTb_Size);
				vPortFree(tempMT7601_BBP_BW20RegTb);
			}
			else if (pAd->CommonCfg.BBPCurrentBW == BW_40 )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_BW40RegTb, MT7601_BBP_BW40RegTb_Size);

				char* tempMT7601_BBP_BW40RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_BW40RegTb)+0xF);
				if (tempMT7601_BBP_BW40RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_BW40RegTbAddr = (UINT32)(tempMT7601_BBP_BW40RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_BW40RegTbAddr,MT7601_BBP_BW40RegTb,sizeof(MT7601_BBP_BW40RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_BW40RegTbAddr, MT7601_BBP_BW40RegTb_Size);
				vPortFree(tempMT7601_BBP_BW40RegTb);
			}
			else
				DBGPRINT(RT_ERROR, ("[ERR]%s::Unsupported BW(%x)\r\n", __FUNCTION__, pAd->CommonCfg.BBPCurrentBW));

			DBGPRINT(RT_TRACE, ("[TRACE]%s::Change to TEMPERATURE_MODE_NORMAL\r\n", __FUNCTION__));
		}
	}
	else
	{
		if ( pChipCap->TemperatureMode != TEMPERATURE_MODE_LOW )
		{
			pChipCap->TemperatureMode = TEMPERATURE_MODE_LOW;

			AndesBBPRandomWritePair(pAd, MT7601_BBP_LowTempCommonRegTb, MT7601_BBP_LowTempCommonRegTb_Size);

			if (pAd->CommonCfg.BBPCurrentBW == BW_20 )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_LowTempBW20RegTb, MT7601_BBP_LowTempBW20RegTb_Size);

				char* tempMT7601_BBP_LowTempBW20RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_LowTempBW20RegTb)+0xF);
				if (tempMT7601_BBP_LowTempBW20RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_LowTempBW20RegTbAddr = (UINT32)(tempMT7601_BBP_LowTempBW20RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_LowTempBW20RegTbAddr,MT7601_BBP_LowTempBW20RegTb,sizeof(MT7601_BBP_LowTempBW20RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_LowTempBW20RegTbAddr, MT7601_BBP_LowTempBW20RegTb_Size);
				vPortFree(tempMT7601_BBP_LowTempBW20RegTb);
			}
			else if (pAd->CommonCfg.BBPCurrentBW == BW_40 )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_LowTempBW40RegTb, MT7601_BBP_LowTempBW40RegTb_Size);

				char* tempMT7601_BBP_LowTempBW40RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_LowTempBW40RegTb) + 0xF);
				if (tempMT7601_BBP_LowTempBW40RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_LowTempBW40RegTbAddr = (UINT32)(tempMT7601_BBP_LowTempBW40RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_LowTempBW40RegTbAddr,MT7601_BBP_LowTempBW40RegTb,sizeof(MT7601_BBP_LowTempBW40RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_LowTempBW40RegTbAddr, MT7601_BBP_LowTempBW40RegTb_Size);
				vPortFree(tempMT7601_BBP_LowTempBW40RegTb);
			}
			else
				DBGPRINT(RT_ERROR, ("[ERR]%s::Unsupported BW(%x)\r\n", __FUNCTION__, pAd->CommonCfg.BBPCurrentBW));

			DBGPRINT(RT_TRACE, ("[TRACE]%s::Change to TEMPERATURE_MODE_LOW\r\n", __FUNCTION__));
		}
	}	
}



VOID MT7601_AsicAntennaDefaultReset(
	IN PRTMP_ADAPTER			pAd,
	IN PEEPROM_ANTENNA_STRUC 	pAntenna)
{
	DBGPRINT(RT_TRACE, ("[TRCAE]--->%s\r\n", __FUNCTION__));
	pAntenna->word = 0;
	pAntenna->field.RfIcType = 0xf;
	pAntenna->field.TxPath = 1;
	pAntenna->field.RxPath = 1;
}


VOID MT7601_AsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
	RTMP_IO_READ32(pAd, 0x1318, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, 0x131C, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */			
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1324, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);
	
	DBGPRINT(RT_TRACE, ("[TRACE]%s: Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\r\n",
						__FUNCTION__, (UINT)ExtraPwrOverTxPwrCfg7, (UINT)ExtraPwrOverTxPwrCfg9));
}



VOID MT7601_ChipSwitchChannel(	
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel,
	IN BOOLEAN	 		bScan)
{

	{
		RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x20);
		MT7601_ChipSwitchChannel_ex(pAd, Channel, bScan);
		DBGPRINT(RT_INFO, ("[INFO]%s: Set channel(%d)\r\n", __FUNCTION__, Channel));
	}
}


 VOID MT7601_ChipSwitchChannel_ex(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel,
	IN BOOLEAN	 		bScan)
{
	CHAR TxPwer = 0;
	UCHAR index;
	UCHAR RFValue = 0;
	UINT32 Value = 0;
#ifdef SINGLE_SKU_V2
	CHAR SkuBasePwr;
	CHAR ChannelPwrAdj;
#endif /* SINGLE_SKU_V2 */

	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	if (Channel == 0) return ;
	
	if (Channel > 14)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Can't find the Channel#%d \r\n", __FUNCTION__, Channel));
		return;
	}	
	
#ifdef SINGLE_SKU_V2
	SkuBasePwr = GetSkuChannelBasePwr(pAd, Channel);

#ifdef RTMP_INTERNAL_TX_ALC
	if (pAd->TxPowerCtrl.bInternalTxALC != TRUE)
#endif /* RTMP_INTERNAL_TX_ALC */
	{
		UINT32 value;
		if ( pAd->DefaultTargetPwr > SkuBasePwr )
			ChannelPwrAdj = SkuBasePwr - pAd->DefaultTargetPwr;
		else
			ChannelPwrAdj = 0;

		if ( ChannelPwrAdj > 31 )
			ChannelPwrAdj = 31;
		if ( ChannelPwrAdj < -32 )
			ChannelPwrAdj = -32;

		RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
		value = (value & ~0x3F) | (ChannelPwrAdj & 0x3F);
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, value);
		DBGPRINT(RT_TRACE, ("[TRACE]SkuBasePwr = 0x%x,  DefaultTargetPwr = 0x%x, ChannelPwrAdj 0x13b4: 0x%x\r\n", SkuBasePwr, pAd->DefaultTargetPwr, value));
	}

#ifdef RTMP_INTERNAL_TX_ALC
	if (pAd->TxPowerCtrl.bInternalTxALC)
	{
		TxPwer = SkuBasePwr;
		pAd->TxPower[Channel - 1].Power = TxPwer;
	}
	else
#endif /* RTMP_INTERNAL_TX_ALC */
#endif /* SINGLE_SKU_V2 */
	TxPwer = pAd->TxPower[Channel - 1].Power;

	for (index = 0; index < NUM_OF_MT7601_CHNL; index++)
	{
		if (Channel == MT7601_Frequency_Plan[index].Channel)
		{		
			/* Frequeny plan setting */
			AndesRFRandomWrite(pAd, 4,
								RF_BANK0, RF_R17, MT7601_Frequency_Plan[index].K_R17,
								RF_BANK0, RF_R18, MT7601_Frequency_Plan[index].K_R18,
								RF_BANK0, RF_R19, MT7601_Frequency_Plan[index].K_R19,
								RF_BANK0, RF_R20, MT7601_Frequency_Plan[index].N_R20);			
			break;
		}
	}

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &Value);
	Value = Value & (~0x3F3F);
	Value |= (TxPwer & 0x3F);
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, Value);

	pAd->LatchRfRegs.Channel = Channel; /* Channel latch */

	/* BBP setting */
	if (Channel <= 14)
	{
		AndesBBPRandomWrite(pAd, 3,
			BBP_R62, (0x37 - GET_LNA_GAIN(pAd)),
			BBP_R63, (0x37 - GET_LNA_GAIN(pAd)),
			BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		//RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}

	/* 
		vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. 
	*/
	RLT_RF_write(pAd, RF_BANK0, RF_R04, 0x0A);
	RLT_RF_write(pAd, RF_BANK0, RF_R05, 0x20);
	RLT_RF_read(pAd, RF_BANK0, RF_R04, &RFValue);
	RFValue = RFValue | 0x80; 
	RLT_RF_write(pAd, RF_BANK0, RF_R04, RFValue);
	RTMPusecDelay(2000);

	RTMP_BBPSetBW(pAd, pAd->CommonCfg.BBPCurrentBW);

	switch (pAd->CommonCfg.BBPCurrentBW)
	{
		case BW_20:
			if ( pChipCap->TemperatureMode == TEMPERATURE_MODE_HIGH )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_HighTempBW20RegTb, MT7601_BBP_HighTempBW20RegTb_Size);

				char* tempMT7601_BBP_HighTempBW20RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_HighTempBW20RegTb)+0xF);
				if (tempMT7601_BBP_HighTempBW20RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_HighTempBW20RegTbAddr = (UINT32)(tempMT7601_BBP_HighTempBW20RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_HighTempBW20RegTbAddr,MT7601_BBP_HighTempBW20RegTb,sizeof(MT7601_BBP_HighTempBW20RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_HighTempBW20RegTbAddr, MT7601_BBP_HighTempBW20RegTb_Size);
				vPortFree(tempMT7601_BBP_HighTempBW20RegTb);
			}
			else if ( pChipCap->TemperatureMode == TEMPERATURE_MODE_LOW )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_LowTempBW20RegTb, MT7601_BBP_LowTempBW20RegTb_Size);

				char* tempMT7601_BBP_LowTempBW20RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_LowTempBW20RegTb)+0xF);
				if (tempMT7601_BBP_LowTempBW20RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_LowTempBW20RegTbAddr = (UINT32)(tempMT7601_BBP_LowTempBW20RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_LowTempBW20RegTbAddr,MT7601_BBP_LowTempBW20RegTb,sizeof(MT7601_BBP_LowTempBW20RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_LowTempBW20RegTbAddr, MT7601_BBP_LowTempBW20RegTb_Size);
				vPortFree(tempMT7601_BBP_LowTempBW20RegTb);
			}
			else
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_BW20RegTb, MT7601_BBP_BW20RegTb_Size);

				char* tempMT7601_BBP_BW20RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_BW20RegTb) + 0xF);
				if (tempMT7601_BBP_BW20RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_BW20RegTbAddr = (UINT32)(tempMT7601_BBP_BW20RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_BW20RegTbAddr,MT7601_BBP_BW20RegTb,sizeof(MT7601_BBP_BW20RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_BW20RegTbAddr, MT7601_BBP_BW20RegTb_Size);
				vPortFree(tempMT7601_BBP_BW20RegTb);
			}

			/* Tx Filter BW */
			AndesCalibrationOP(pAd, ANDES_CALIBRATION_BW, 0x10001);
			/* Rx Filter BW */
			AndesCalibrationOP(pAd, ANDES_CALIBRATION_BW, 0x10000);
			break;
		case BW_40:
			if ( pChipCap->TemperatureMode == TEMPERATURE_MODE_HIGH )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_HighTempBW40RegTb, MT7601_BBP_HighTempBW40RegTb_Size);

				char* tempMT7601_BBP_HighTempBW40RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_HighTempBW40RegTb)+0xF);
				if (tempMT7601_BBP_HighTempBW40RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_HighTempBW40RegTbAddr = (UINT32)(tempMT7601_BBP_HighTempBW40RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_HighTempBW40RegTbAddr,MT7601_BBP_HighTempBW40RegTb,sizeof(MT7601_BBP_HighTempBW40RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_HighTempBW40RegTbAddr, MT7601_BBP_HighTempBW40RegTb_Size);
				vPortFree(tempMT7601_BBP_HighTempBW40RegTb);
			}
			else if ( pChipCap->TemperatureMode == TEMPERATURE_MODE_LOW )
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_LowTempBW40RegTb, MT7601_BBP_LowTempBW40RegTb_Size);

				char* tempMT7601_BBP_LowTempBW40RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_LowTempBW40RegTb)+0xF);
				if (tempMT7601_BBP_LowTempBW40RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_LowTempBW40RegTbAddr = (UINT32)(tempMT7601_BBP_LowTempBW40RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_LowTempBW40RegTbAddr,MT7601_BBP_LowTempBW40RegTb,sizeof(MT7601_BBP_LowTempBW40RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_LowTempBW40RegTbAddr, MT7601_BBP_LowTempBW40RegTb_Size);
				vPortFree(tempMT7601_BBP_LowTempBW40RegTb);
			}
			else
			{
				//AndesBBPRandomWritePair(pAd, MT7601_BBP_BW40RegTb, MT7601_BBP_BW40RegTb_Size);

				char* tempMT7601_BBP_BW40RegTb = (char*)pvPortMalloc(sizeof(MT7601_BBP_BW40RegTb)+0xF);
				if (tempMT7601_BBP_BW40RegTb == NULL)
				{
					DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
					return;
				}
				unsigned int tempMT7601_BBP_BW40RegTbAddr = (UINT32)(tempMT7601_BBP_BW40RegTb+ 0xF) & 0xFFFFFFF0;
				memcpy((void*)tempMT7601_BBP_BW40RegTbAddr,MT7601_BBP_BW40RegTb,sizeof(MT7601_BBP_BW40RegTb));
				AndesBBPRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMT7601_BBP_BW40RegTbAddr, MT7601_BBP_BW40RegTb_Size);
				vPortFree(tempMT7601_BBP_BW40RegTb);
			}

			/* Tx Filter BW */
			AndesCalibrationOP(pAd, ANDES_CALIBRATION_BW, 0x10101);
			/* Rx Filter BW */
			AndesCalibrationOP(pAd, ANDES_CALIBRATION_BW, 0x10100);

			break;
		default:			
			break;
	}

#ifdef MICROWAVE_OVEN_SUPPORT
	/* B5.R6 and B5.R7 */
	RLT_RF_read(pAd, RF_BANK5, RF_R06, &RFValue);
	pAd->CommonCfg.MO_Cfg.Stored_RF_B5_R6 = RFValue;
	RLT_RF_read(pAd, RF_BANK5, RF_R07, &RFValue);
	pAd->CommonCfg.MO_Cfg.Stored_RF_B5_R7 = RFValue;
#endif /* MICROWAVE_OVEN_SUPPORT */

	/* CCK CH14 OBW */
	if ( (pAd->CommonCfg.BBPCurrentBW == BW_20) && ( Channel == 14 ) )
	{
		
		UINT32	value;
		CHAR	CCK1MPwr, CCK11MPwr;
		
		MT7601_BBP_write(pAd, BBP_R4, 0x60);
		MT7601_BBP_write(pAd, BBP_R178, 0x0);
		

		value = pAd->TxCCKPwrCfg;
		CCK1MPwr = value & 0x3F;
		CCK1MPwr -= 2;
		if ( CCK1MPwr < -32 )
			CCK1MPwr = -32;
		CCK11MPwr = (value & 0x3F00) >> 8;
		CCK11MPwr -= 2;
		if ( CCK11MPwr < -32 )
			CCK11MPwr = -32;

		value = (value & ~0xFFFF) | (CCK11MPwr << 8 ) | CCK1MPwr;

		pAd->Tx20MPwrCfgGBand[0] = value;
	}
	else
	{
		UCHAR BBPValue;
		MT7601_BBP_read(pAd, BBP_R4, &BBPValue);
		BBPValue &= ~(0x20);
		AndesBBPRandomWrite(pAd, 2,
							BBP_R4, BBPValue,
							BBP_R178, 0xFF);
		pAd->Tx20MPwrCfgGBand[0] = pAd->TxCCKPwrCfg;
	}
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0, pAd->Tx20MPwrCfgGBand[0]);
}

VOID MT7601_ReadTxPwrPerRate(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		data, Adata, Gdata;
	USHORT		i, value, value2;
	CHAR		value_1, value_2;
	CHAR		Apwrdelta, Gpwrdelta;
	CHAR		t1,t2;

	/* Get power delta for 20MHz and 40MHz.*/
	DBGPRINT(RT_TRACE, ("[TRCAE]--->%s\r\n", __FUNCTION__));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value2);
	Apwrdelta = 0;
	Gpwrdelta = 0;

	if ((value2 & 0xff) != 0xff)
	{
		if ((value2 & 0x80))
		{
			/* Unit = 0.5 dBm, Max is 4 dBm */
			Gpwrdelta = (value2&0x1f);
			if ( Gpwrdelta > 8 )
				Gpwrdelta = 8;

			if ((value2 & 0x40) == 0 )
				Gpwrdelta = -Gpwrdelta;
		}
		
	}
	if ((value2 & 0xff00) != 0xff00)
	{
		if ((value2 & 0x8000))
		{
			Apwrdelta = ((value2&0x1f00)>>8);
			if ( Apwrdelta > 8 )
				Apwrdelta = 8;

			if ((value2 & 0x4000) == 0)
				Apwrdelta = - Apwrdelta;
		}
	}
	
#ifdef SINGLE_SKU_V2
	pAd->chipCap.Apwrdelta = Apwrdelta;
	pAd->chipCap.Gpwrdelta = Gpwrdelta;
#endif /* SINGLE_SKU_V2 */
	DBGPRINT(RT_TRACE, ("[TRACE]%s: Gpwrdelta = %d, Apwrdelta = %d .\r\n", __FUNCTION__, Gpwrdelta, Apwrdelta));
	
	/* Get Txpower per MCS for 20MHz in 2.4G.*/
	
	for (i = 0; i < 5; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, value);
		data = value;

		/* signed 6-bit */		
		value_1 = (value & 0x3f);
		value_2 = (value & 0x3f00) >> 8;

		/* signed extension */
		value_1 = (value_1 > 0x1F) ? value_1 - 0x40 : value_1;
		value_2 = (value_2 > 0x1F) ? value_2 - 0x40 : value_2;

		t1 = value_1 + Gpwrdelta;
		t2 = value_2 + Gpwrdelta;

		/* boundary check */
		if (t1 > 31)
			t1 = 31;
		if (t1 < -32)
			t1 = -32;
		if (t2 > 31)
			t2 = 31;
		if (t2 < -32)
			t2 = -32;

		Gdata = (t1 & 0x3f) + ((t2 & 0x3f) << 8);

		t1 = value_1 + Apwrdelta;
		t2 = value_2 + Apwrdelta;

		/* boundary check */
		if (t1 > 31)
			t1 = 31;
		if (t1 < -32)
			t1 = -32;

		if (t2 > 31)
			t2 = 31;
		if (t2 < -32)
			t2 = -32;

		Adata = (t1 & 0x3f)  + ((t2 & 0x3f) <<8);

		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, value);

		if ( i != 4 )
		{
			/* signed 6-bit */		
			value_1 = (value & 0x3f);
			value_2 = (value & 0x3f00) >> 8;

			/* signed extension */
			value_1 = (value_1 > 0x1F) ? value_1 - 0x40 : value_1;
			value_2 = (value_2 > 0x1F) ? value_2 - 0x40 : value_2;

			t1 = value_1 + Gpwrdelta;
			t2 = value_2 + Gpwrdelta;

			/* boundary check */
			if (t1 > 31)
				t1 = 31;
			if (t1 < -32)
				t1 = -32;
			if (t2 > 31)
				t2 = 31;
			if (t2 < -32)
				t2 = -32;

			Gdata |= (((t1 & 0x3f) << 16) + ((t2 & 0x3f) << 24));

			t1 = value_1 + Apwrdelta;
			t2 = value_2 + Apwrdelta;

			/* boundary check */
			if (t1 > 31)
				t1 = 31;
			if (t1 < -32)
				t1 = -32;
			if (t2 > 31)
				t2 = 31;
			if (t2 < -32)
				t2 = -32;
		
			Adata |= (( (t1 & 0x3f ) << 16) + ( (t2 & 0x3f ) << 24));
		} 
		else
		{
			Gdata |= 0xFFFF0000;
			Adata |= 0xFFFF0000;
		}
		data |= (value << 16);

		/* For 20M/40M Power Delta issue */		
		pAd->Tx20MPwrCfgABand[i] = data;
		pAd->Tx20MPwrCfgGBand[i] = data;
		pAd->Tx40MPwrCfgABand[i] = Adata;
		pAd->Tx40MPwrCfgGBand[i] = Gdata;
		
		if (data != 0xffffffff)
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);
		
		DBGPRINT(RT_TRACE, ("20MHz BW, 2.4G band-%08x,  Adata = %08x,  Gdata = %08x \r\n", data, Adata, Gdata));
	}

	/* Extra set MAC registers to compensate Tx power if any */
	MT7601_AsicExtraPowerOverMAC(pAd);

	pAd->TxCCKPwrCfg = pAd->Tx20MPwrCfgGBand[0];
}



INT MT7601_ReadChannelPwr(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32 i, idx, ss_offset_g, MacReg;
	EEPROM_TX_PWR_STRUC Power;
	CHAR tx_pwr1, tx_pwr2;
	CHAR max_tx1_pwr;
	
#ifdef RTMP_INTERNAL_TX_ALC
	UINT16 TargetPwr = 0;
#endif

#ifdef RTMP_INTERNAL_TX_ALC
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
#endif /* RTMP_INTERNAL_TX_ALC */

	DBGPRINT(RT_TRACE, ("[TRCAE]%s()--->\r\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &MacReg);
	max_tx1_pwr = (MacReg >> 16) & 0x3F;

#if defined (RTMP_INTERNAL_TX_ALC)// || defined (SINGLE_SKU_V2)
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TARGET_POWER, TargetPwr);
	tx_pwr1 = TargetPwr & 0xFF;
	DBGPRINT(RT_TRACE, ("[TRCAE]%s: EEPROM 0xD0 = 0x%x\r\n", __FUNCTION__, tx_pwr1));

	if ( (tx_pwr1 == 0x0) || (tx_pwr1 > max_tx1_pwr) )
	{
		tx_pwr1 = 0x20;
		DBGPRINT(RT_ERROR, ("[ERR]%s: EEPROM 0xD0 Error! Use Default Target Power = 0x%x\r\n", __FUNCTION__, tx_pwr1));
	}
#endif /* defined (RTMP_INTERNAL_TX_ALC) || defined (SINGLE_SKU_V2) */

#ifdef SINGLE_SKU_V2
	pAd->DefaultTargetPwr = tx_pwr1;
#endif /* SINGLE_SKU_V2 */
		
	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

#ifdef RTMP_INTERNAL_TX_ALC
	NicConfig2.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];
	if (NicConfig2.field.bInternalTxALC)
	{
		for (i = 0; i < 7; i++)
		{
			idx = i * 2;

			pAd->TxPower[idx].Power = tx_pwr1;
			pAd->TxPower[idx + 1].Power = tx_pwr1;
			pAd->TxPower[idx].Channel = i * 2 +1;
			pAd->TxPower[idx + 1].Channel = i * 2 + 2;

			DBGPRINT(RT_TRACE, ("[TRCAE]%s: TxPower[%d].Power = 0x%02X, TxPower[%d].Power = 0x%02X\r\n", 
								__FUNCTION__, 
								i * 2, 
								pAd->TxPower[i * 2].Power, 
								i * 2 + 1, 
								pAd->TxPower[i * 2 + 1].Power));
		}

		return TRUE;
	}
#endif /* RTMP_INTERNAL_TX_ALC */

	/* 0. 11b/g, ch1 - ch 14, 1SS */
	ss_offset_g = EEPROM_G_TX_PWR_OFFSET;

	for (i = 0; i < 7; i++)
	{
		idx = i * 2;
		RT28xx_EEPROM_READ16(pAd, ss_offset_g + idx, Power.word);

		tx_pwr1 = tx_pwr2 = DEFAULT_TX_POWER;

		if ((Power.field.Byte0 <= max_tx1_pwr) && (Power.field.Byte0 >= 0))
			tx_pwr1 = Power.field.Byte0;

		if ((Power.field.Byte1 <= max_tx1_pwr) || (Power.field.Byte1 >= 0))
			tx_pwr2 = Power.field.Byte1;

		pAd->TxPower[idx].Power = tx_pwr1;
		pAd->TxPower[idx + 1].Power = tx_pwr2;
		pAd->TxPower[idx].Channel = i * 2 +1;
		pAd->TxPower[idx + 1].Channel = i * 2 + 2;

		DBGPRINT(RT_TRACE, ("[TRCAE]%s: TxPower[%d].Power = 0x%02X, TxPower[%d].Power = 0x%02X\r\n", 
							__FUNCTION__, 
							i * 2, 
							pAd->TxPower[i * 2].Power, 
							i * 2 + 1, 
							pAd->TxPower[i * 2 + 1].Power));
	}

	return TRUE;
}

VOID MT7601_MacInit(
	IN PRTMP_ADAPTER 	pAd)
{
#if defined(RTMP_MAC_USB) && defined(RLT_MAC)
	//AndesRandomWritePair(pAd, MACRegTable, NUM_MAC_REG_PARMS);

	char* tempMACRegTable = (char*)pvPortMalloc(sizeof(MACRegTable) + 0xF);
	if (tempMACRegTable == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Can't malloc memory\r\n", __FUNCTION__));
		return;
	}
	unsigned int tempMACRegTableAddr = (UINT32)(tempMACRegTable+ 0xF) & 0xFFFFFFF0;
	memcpy((void*)tempMACRegTableAddr,MACRegTable,sizeof(MACRegTable));
	AndesRandomWritePair(pAd, (RTMP_REG_PAIR*)tempMACRegTableAddr, NUM_MAC_REG_PARMS);
	vPortFree(tempMACRegTable);
#else
	ULONG			Index = 0;
	
	/* Initialize MAC register to default value*/
	for (Index = 0; Index < NUM_MAC_REG_PARMS; Index++)
	{
		RTMP_IO_WRITE32(pAd, (USHORT)MACRegTable[Index].Register, MACRegTable[Index].Value);
	}	
#endif /* defined(RTMP_MAC_USB) && defined(RLT_MAC) */

}

/*
VOID MT7601_SetMacRegisterOfPBF(
	IN UINT32 	ulValue)
{
	if (MACRegTable[13].Register == PBF_MAX_PCNT)
	{
		MACRegTable[13].Value = ulValue;
	}
}
*/

NDIS_STATUS MT7601_BBP_read(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			regID,
	IN PUCHAR 			pValue)
{
	BBP_CSR_CFG_STRUC  BbpCsr = { { 0 } };
	UINT i = 0, k = 0;
	NDIS_STATUS	 ret = NDIS_FAILURE;

	for (i = 0; i < MAX_BUSY_COUNT; i++)
	{			
		RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr.word);

		if (BbpCsr.field.Busy == BUSY)
				continue;
		
		BbpCsr.word = 0;
		BbpCsr.field.fRead = 1;
		BbpCsr.field.BBP_RW_MODE = 1;
		BbpCsr.field.Busy = 1;
		BbpCsr.field.RegNum = regID;
		
		RTMP_IO_WRITE32(pAd, BBP_CSR_CFG, BbpCsr.word);
		
		for (k = 0; k < MAX_BUSY_COUNT; k++)
		{		
			RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr.word);

			if (BbpCsr.field.Busy == IDLE)
				break;
		}
		
		if ((BbpCsr.field.Busy == IDLE) &&
			(BbpCsr.field.RegNum == regID) )
		{
			*pValue = (UCHAR)(BbpCsr.field.Value);
			break;
		}
	}

	if (BbpCsr.field.Busy == BUSY)
	{																	
		DBGPRINT(RT_ERROR, ("[ERR]%s: BBP read R%d=0x%X fail, i[%d], k[%d]\r\n", 
							__FUNCTION__, regID, BbpCsr.word,i,k));
		goto done;
	}
	ret = NDIS_SUCCESS;

done:
	return ret;
}


NDIS_STATUS MT7601_BBP_write(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			regID,
	IN UCHAR 			value)
{
	BBP_CSR_CFG_STRUC BbpCsr = {{0}};
	UINT i = 0;
	NDIS_STATUS	 ret = NDIS_FAILURE;

	do
	{
		RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr.word);

		if (!BbpCsr.field.Busy)
			break;
		
		i++;
	} while (i < MAX_BUSY_COUNT );

	if ((i == MAX_BUSY_COUNT))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Retry count exhausted or device removed!!!\r\n", __FUNCTION__));
		goto done;
	}

	BbpCsr.word = 0;
	BbpCsr.field.fRead = 0;
	BbpCsr.field.BBP_RW_MODE = 1;
	BbpCsr.field.Busy = 1;
	BbpCsr.field.Value = value;
	BbpCsr.field.RegNum = regID;

	RTMP_IO_WRITE32(pAd, BBP_CSR_CFG, BbpCsr.word);

	ret = NDIS_SUCCESS;
done:
	return ret;
}

VOID MT7601_SetEDCCA(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			enable)
{
	UINT32 mac_val;
	UCHAR bbp_val;

	if (enable) {
		RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
		mac_val |= 0x05; // enable channel status check
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);

		// BBP: enable ED_CCA and high/low threshold
//		bbp_val = 0x18; /* 0x08 *//*0x0B*/ /* 0x2e */ // bit 0~7 for high threshold
		bbp_val = 0xC; // Make some margin for cable loss when testing
		
		MT7601_BBP_write(pAd, BBP_R61, bbp_val);
		MT7601_BBP_write(pAd, BBP_R87, 0x87);
		MT7601_BBP_write(pAd, BBP_R123, 0x03);

		// MAC: enable ED_CCA/ED_2nd_CCA
		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
		mac_val |= ((1<<20) | (1<<7));
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);
	}
	else
	{
		MT7601_BBP_read(pAd, BBP_R87, &bbp_val);
		bbp_val &= (~0x80); // bit 7 for enable/disable ED_CCA
		MT7601_BBP_write(pAd, BBP_R87, bbp_val);

		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
		mac_val &= (~((1<<20) | (1<<7)));
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);         
	}

	/* Clear previous status */
	RTMP_IO_READ32(pAd, CH_IDLE_STA, &mac_val);
	RTMP_IO_READ32(pAd, CH_BUSY_STA, &mac_val);
	RTMP_IO_READ32(pAd, CH_BUSY_STA_SEC, &mac_val);
	RTMP_IO_READ32(pAd, ED_CCA_TIMER, &mac_val);
}

