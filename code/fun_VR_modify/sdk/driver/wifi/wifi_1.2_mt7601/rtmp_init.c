#include "rtmp_init.h"
#include "rtmp.h"
#include "rtmp_ctrl.h"
#include "asic_ctrl.h"
#include "rtmp_andes.h"
#include "usb_io.h"
#include "MT7601/mt7601_ctrl.h"
//#include "MT7601/mt7601_table.h"





/* BBP register initialization set*/
__align(32) REG_PAIR   BBPRegTable[] = {
	{BBP_R65,		0x2C},		/* fix rssi issue*/
	{BBP_R66,		0x38},	/* Also set this default value to pAd->BbpTuning.R66CurrentValue at initial*/
	{BBP_R68,		0x0B},  /* improve Rx sensitivity. */
	{BBP_R69,		0x12},
	{BBP_R70,		0xa},	/* BBP_R70 will change to 0x8 in ApStartUp and LinkUp for rt2860C, otherwise value is 0xa*/
	{BBP_R73,		0x10},
	{BBP_R81,		0x37},
	{BBP_R82,		0x62},
	{BBP_R83,		0x6A},
	{BBP_R84,		0x99},	/* 0x19 is for rt2860E and after. This is for extension channel overlapping IOT. 0x99 is for rt2860D and before*/
	{BBP_R86,		0x00},	/* middle range issue, Rory @2008-01-28 	*/
	{BBP_R91,		0x04},	/* middle range issue, Rory @2008-01-28*/
	{BBP_R92,		0x00},	/* middle range issue, Rory @2008-01-28*/
	{BBP_R103,		0x00}, 	/* near range high-power issue, requested from Gary @2008-0528*/
	{BBP_R105,		0x05},	/* 0x05 is for rt2860E to turn on FEQ control. It is safe for rt2860D and before, because Bit 7:2 are reserved in rt2860D and before.*/
#ifdef DOT11_N_SUPPORT
	{BBP_R106,		0x35},	/* Optimizing the Short GI sampling request from Gray @2009-0409*/
#endif /* DOT11_N_SUPPORT */
};
#define	NUM_BBP_REG_PARMS	(sizeof(BBPRegTable) / sizeof(REG_PAIR))



NDIS_STATUS NICInitBBP(
	IN PRTMP_ADAPTER 	pAd)
{
	INT Index = 0;

	/* Initialize BBP register to default value*/
	for (Index = 0; Index < NUM_BBP_REG_PARMS; Index++)
	{

#ifdef MICROWAVE_OVEN_SUPPORT
#ifdef MT7601
		if (BBPRegTable[Index].Register == BBP_R65)
		{
			/* Backup BBP_R65 and B5.R6 and B5.R7 */	
			pAd->CommonCfg.MO_Cfg.Stored_BBP_R65 = BBPRegTable[Index].Value;
			DBGPRINT(RT_TRACE, ("[TRAE]%s: Stored_BBP_R65=%x\r\n", 
								__FUNCTION__, pAd->CommonCfg.MO_Cfg.Stored_BBP_R65));
		}
#endif /* MT7601 */
#endif /* MICROWAVE_OVEN_SUPPORT */

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd,
										BBPRegTable[Index].Register,
										BBPRegTable[Index].Value);
	}

	/* re-config specific BBP registers for individual chip */
	if (pAd->chipCap.pBBPRegTable)
	{
		REG_PAIR *pbbpRegTb = pAd->chipCap.pBBPRegTable;
		
		for (Index = 0; Index < pAd->chipCap.bbpRegTbSize; Index++)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd,
											pbbpRegTb[Index].Register,
											pbbpRegTb[Index].Value);
			
			DBGPRINT(RT_TRACE, ("[TRCAE]%s: BBP_R%d=0x%x\r\n",
								__FUNCTION__, pbbpRegTb[Index].Register,  pbbpRegTb[Index].Value));
		}
	}

	 NICInitMT7601BbpRegisters(pAd);
	/*
		For rt2860E and after, init BBP_R84 with 0x19. This is for extension channel overlapping IOT.
		RT3090 should not program BBP R84 to 0x19, otherwise TX will block.
		3070/71/72,3090,3090A( are included in RT30xx),3572,3390
	*/
	if (((pAd->MACVersion & 0xffff) != 0x0101) &&
		!(IS_RT30xx(pAd)|| IS_RT3572(pAd) || IS_RT5390(pAd) || IS_RT5392(pAd)  || IS_MT7601(pAd)))
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x19);


	if (pAd->MACVersion == 0x28600100)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x12);
	}

	return NDIS_SUCCESS;	
}



NTSTATUS  NICInitializeAdapter(
	IN PRTMP_ADAPTER 	pAd)
{

	WPDMA_GLO_CFG_STRUC	GloCfg;
	UINT8	 retry = 0, index = 0;

	DBGPRINT(RT_TRACE, ("[TRCAE]--->%s\r\n", __FUNCTION__));
	
	/* 3. Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits:*/
_retry:

	retry = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;
		
		RTMPusecDelay(1000);
		retry++;
	} while (retry < 100);

	if (retry == 100) goto _err1;

	if (NICInitializeAsic(pAd) != NDIS_SUCCESS)
	{
		if (index++ < 10)
		{
			goto _retry;
		}
		else return NDIS_FAILURE;
	}

	DBGPRINT(RT_TRACE, ("[TRCAE]<---%s\r\n", __FUNCTION__));
	
	return NDIS_SUCCESS;

_err1:
	return NDIS_FAILURE;	
}

VOID NicResetRawCounters(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32 Counter;
	
	RTMP_IO_READ32(pAd, RX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &Counter);
}


NTSTATUS NICInitializeAsic(
	IN PRTMP_ADAPTER 	pAd)
{
	ULONG			Index = 0;
	UINT32			MACValue = 0;
 	UINT32			MacCsr0 = 0;
	USB_DMA_CFG_STRUC UsbCfg;
	UINT32			Counter = 0;
	
	DBGPRINT(RT_TRACE, ("[TRCAE]---> %s\r\n", __FUNCTION__));

	/* Make sure MAC gets ready after NICLoadFirmware().*/
	
	Index = 0;
	
	do 
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);

		if ((MacCsr0 != 0x00) && (MacCsr0 != 0xFFFFFFFF))
			break;

		RTMPusecDelay(10);
	} while (Index++ < 100);

	pAd->MACVersion = MacCsr0;

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x3);
	RTMP_IO_WRITE32(pAd, USB_DMA_CFG, 0x0);
	RTMPusecDelay(100); 
	
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

#ifdef RTMP_MAC_USB
	UsbCfg.word = 0;
	
	/* USB1.1 do not use bulk in aggregation */
//	if (pAd->BulkInMaxPacketSize == 512)
	UsbCfg.field.RxBulkAggEn = 0;

	/* for last packet, PBF might use more than limited, so minus 2 to prevent from error */
	UsbCfg.field.RxBulkAggLmt = (MAX_RXBULK_SIZE /1024)-3;
	UsbCfg.field.RxBulkAggTOut = 0x80; 
	UsbCfg.field.RxBulkEn = 1;
	UsbCfg.field.TxBulkEn = 1;
	
	RTMP_IO_WRITE32(pAd, USB_DMA_CFG, UsbCfg.word);

#ifdef MT7601
	if (IS_MT7601(pAd))
	{
		UsbCfg.field.UDMA_RX_WL_DROP = 1;
		RTMP_IO_WRITE32(pAd, USB_DMA_CFG, UsbCfg.word);
	
		UsbCfg.field.UDMA_RX_WL_DROP = 0;
		RTMP_IO_WRITE32(pAd, USB_DMA_CFG, UsbCfg.word);
	}	
#endif /* MT7601 */

#ifdef RLT_MAC
	/* Select Q2 to receive command response */
	AndesFunSetOP(pAd, Q_SELECT, RX_RING1);
#endif /* RLT_MAC */
#endif /* RTMP_MAC_USB */

	MT7601_MacInit(pAd);

	AsicInitBcnBuf(pAd);

	/* re-set specific MAC registers for individual chip */
	NICInitMT7601MacRegisters(pAd);

	/* Before program BBP, we need to wait BBP/RF get wake up.*/
	Index = 0;
	do
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MACValue);

		if ((MACValue & 0x03) == 0)	/* if BB.RF is stable*/
			break;
		
		DBGPRINT(RT_TRACE, ("Check MAC_STATUS_CFG  = Busy = %x\r\n", MACValue));
		RTMPusecDelay(1000);
	} while (Index++ < 100);

	/* Wait to be stable.*/
	RTMPusecDelay(1000);
	pAd->LastMCUCmd = 0x72;

	NICInitBBP(pAd);

	// Clifford simplfy as:
	{
		UINT32 csr;
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &csr);
		csr |= 0x3fff;
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, csr);
	}

	
#ifdef RTMP_MAC_USB
#ifdef RLT_MAC
	/*@!RELEASE
		Reset WCID table

		In AP mode,  First WCID Table in ASIC will never be used.
		To prevent it's 0xff-ff-ff-ff-ff-ff, Write 0 here.

		p.s ASIC use all 0xff as termination of WCID table search.
	*/
	{
		UINT32 MACValue[128 * 2];

		for (Index = 0; Index < 128 * 2; Index+=2)
		{
			MACValue[Index] = 0xffffffff;
			MACValue[Index + 1] = 0x00ffffff;
		}

		AndesBurstWrite(pAd, MAC_WCID_BASE, MACValue, 128 * 2);

	}
#else
	{
	UCHAR	MAC_Value[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0,0};

	/*Initialize WCID table*/
	for(Index = 0; Index < 254;Index++)
	{
		RTUSB_MultiWrite(pAd, (USHORT)(MAC_WCID_BASE + Index * 8), MAC_Value, 8, FALSE);
	}
	}

#endif /* RLT_MAC */
#endif /* RTMP_MAC_USB */

	/* Clear raw counters*/
	NicResetRawCounters(pAd);

	/* ASIC will keep garbage value after boot*/
	/* Clear all shared key table when initial*/
	/* This routine can be ignored in radio-ON/OFF operation. */
	if (1) //This portion can be remarked to shorten initial time 		// bHardReset)
	{

		{
			UINT32 MACValue[4];

			for (Index = 0; Index < 4; Index++)
				MACValue[Index] = 0;

			AndesBurstWrite(pAd, SHARED_KEY_MODE_BASE, MACValue, 4);
		}

		/* Clear all pairwise key table when initial*/
		{
			UINT32 MACValue[256];

			for (Index = 0; Index < 256; Index++)
				MACValue[Index] = 1;

			AndesBurstWrite(pAd, MAC_WCID_ATTRIBUTE_BASE, MACValue, 256);
		}
	}
	
#ifdef RTMP_MAC_USB
	AsicDisableSync(pAd);

	/* Clear raw counters*/
	NicResetRawCounters(pAd);

	/* Default PCI clock cycle per ms is different as default setting, which is based on PCI.*/
	RTMP_IO_READ32(pAd, USB_CYC_CFG, &Counter);
	Counter &= 0xffffff00;
	Counter |= 0x000001e;
	RTMP_IO_WRITE32(pAd, USB_CYC_CFG, Counter);
#endif /* RTMP_MAC_USB */


	{
		/* for rt2860E and after, init TXOP_CTRL_CFG with 0x583f. This is for extension channel overlapping IOT.*/
		if ((pAd->MACVersion & 0xffff) != 0x0101)
			RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, 0x583f);
	}

	return NDIS_SUCCESS;
}


VOID NICInitAsicFromEEPROM(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32 data = 0;
#ifdef RALINK_ATE
	USHORT value;
#endif /* RALINK_ATE */
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
	
	DBGPRINT(RT_TRACE, ("[TRACE]--->%s\r\n", __FUNCTION__));
	

	NicConfig2.word = pAd->NicConfig2.word;

	/* finally set primary ant */
	NICAntCfgInit(pAd);

	MT7601_InitTemperatureCompensation(pAd);

	NICInitMT7601RFRegisters(pAd);

	{
		/* Read Hardware controlled Radio state enable bit*/
		if (NicConfig2.field.HardwareRadioControl == 1)
		{
			BOOLEAN radioOff = FALSE;
			pAd->StaCfg.bHardwareRadio = TRUE;

#ifdef RT3290
			if (IS_RT3290(pAd))
			{
				// Read GPIO pin0 as Hardware controlled radio state
				RTMP_IO_FORCE_READ32(pAd, WLAN_FUN_CTRL, &data);
				if ((data & 0x100) == 0)
					radioOff = TRUE;
			}
			else
#endif /* RT3290 */
			{
				/* Read GPIO pin2 as Hardware controlled radio state*/
				RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
				if ((data & 0x04) == 0)
					radioOff = TRUE;
			}

			if (radioOff)
			{
				pAd->StaCfg.bHwRadio = FALSE;
				pAd->StaCfg.bRadio = FALSE;
			}
		}
		else
			pAd->StaCfg.bHardwareRadio = FALSE;
	}

	/* Turn off patching for cardbus controller*/
	if (NicConfig2.field.CardbusAcceleration == 1)
	{
//		pAd->bTest1 = TRUE;
	}

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

#ifdef RTMP_INTERNAL_TX_ALC
	/*
	    Internal Tx ALC support is starting from RT3370 / RT3390, which combine PA / LNA in single chip.
	    The old chipset don't have this, add new feature flag RTMP_INTERNAL_TX_ALC.
	 */

	/* Internal Tx ALC */
	if (((NicConfig2.field.DynamicTxAgcControl == 1) && 
            (NicConfig2.field.bInternalTxALC == 1)) ||
            ((!IS_RT3390(pAd)) && (!IS_RT3350(pAd)) &&
            (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)) && (!IS_RT5390(pAd)) && (!IS_RT3290(pAd)) && (!IS_MT7601(pAd))))
	{
		/*
			If both DynamicTxAgcControl and bInternalTxALC are enabled,
			it is a wrong configuration.
			If the chipset does not support Internal TX ALC, we shall disable it.
		*/
			pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}
	else
	{
		if (NicConfig2.field.bInternalTxALC == 1)
		{
			pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		}
		else
		{
			pAd->TxPowerCtrl.bInternalTxALC = FALSE;
		}
	}

	
	/* Old 5390 NIC always disables the internal ALC */	
	if (pAd->MACVersion == 0x53900501)
	{
		pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}

	DBGPRINT(RT_TRACE, ("[TRACE]%s: pAd->TxPowerCtrl.bInternalTxALC = %d\r\n", 
						__FUNCTION__,  pAd->TxPowerCtrl.bInternalTxALC));
#endif /* RTMP_INTERNAL_TX_ALC */

	RTMP_BBPSetRxPath(pAd, pAd->Antenna.field.RxPath);
	{
		/* Handle the difference when 1T*/
		{
			if (pAd->Antenna.field.TxPath == 1)
				RTMP_BBPSetTxDAC(pAd, 0);
			else
				RTMP_BBPSetTxDAC(pAd, 2);
		}

		DBGPRINT(RT_TRACE, ("[TRCAE]%s: Use Hw Radio Control Pin=%d; if used Pin=%d;\r\n", 
							__FUNCTION__,
							pAd->StaCfg.bHardwareRadio, pAd->StaCfg.bHardwareRadio));
	}

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
		/*
			Only for RT3593, RT5390 (Maybe add other chip in the future)
			Sometimes the frequency will be shift, we need to adjust it.
		*/
		if (pAd->StaCfg.AdaptiveFreq == TRUE) /*Todo: iwpriv and profile support.*/
		pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration = TRUE;

		DBGPRINT(RT_TRACE, ("[TRACE]%s: pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration = %d\r\n", 
							__FUNCTION__, 
							pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration));

#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

	DBGPRINT(RT_TRACE, ("[TRACE]%s: TxPath = %d, RxPath = %d, RFIC=%d\r\n", 
						__FUNCTION__, 
						pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType));

	DBGPRINT(RT_TRACE, ("[TRACE]<---%s\r\n", __FUNCTION__));
}




VOID NICAntCfgInit(
	IN  PRTMP_ADAPTER   pAd)
{
#ifdef TXRX_SW_ANTDIV_SUPPORT
	/* EEPROM 0x34[15:12] = 0xF is invalid, 0x2~0x3 is TX/RX SW AntDiv */
	DBGPRINT(RT_OFF, ("%s: bTxRxSwAntDiv %d\r\n", __FUNCTION__, pAd->chipCap.bTxRxSwAntDiv));
	if (pAd->chipCap.bTxRxSwAntDiv)  
	{																	  
		DBGPRINT(RT_OFF, ("Antenna word %X/%d, AntDiv %d\r\n", 
					pAd->Antenna.word, pAd->Antenna.field.BoardType, pAd->NicConfig2.field.AntDiversity));
	}
#endif /* TXRX_SW_ANTDIV_SUPPORT */

	{
		if (pAd->NicConfig2.field.AntOpt == 1) /* ant selected by efuse */
		{	
			if (pAd->NicConfig2.field.AntDiversity == 0) /* main */
			{
				pAd->RxAnt.Pair1PrimaryRxAnt = 0;
				pAd->RxAnt.Pair1SecondaryRxAnt = 1;
			}
			else/* aux */
			{
				pAd->RxAnt.Pair1PrimaryRxAnt = 1;
				pAd->RxAnt.Pair1SecondaryRxAnt = 0;
			}
		}
		else if (pAd->NicConfig2.field.AntDiversity == 0) /* Ant div off: default ant is main */
		{
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
		else if (pAd->NicConfig2.field.AntDiversity == 1)/* Ant div on */
		{/* eeprom on, but sw ant div support is not enabled: default ant is main */
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
	}
}



VOID NICReadEEPROMParameters(
	IN PRTMP_ADAPTER 	pAd)
{
	USHORT i, value, value2;
	EEPROM_TX_PWR_STRUC Power;
	EEPROM_VERSION_STRUC Version;
	EEPROM_ANTENNA_STRUC Antenna;
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
	USHORT  Addr01,Addr23,Addr45 ;
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;


	DBGPRINT(RT_TRACE, ("[TRCAE]--->%s\r\n", __FUNCTION__));	

	RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
	RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
	RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);

	pAd->PermanentAddress[0] = (UCHAR)(Addr01 & 0xff);
	pAd->PermanentAddress[1] = (UCHAR)(Addr01 >> 8);
	pAd->PermanentAddress[2] = (UCHAR)(Addr23 & 0xff);
	pAd->PermanentAddress[3] = (UCHAR)(Addr23 >> 8);
	pAd->PermanentAddress[4] = (UCHAR)(Addr45 & 0xff);
	pAd->PermanentAddress[5] = (UCHAR)(Addr45 >> 8);

	/*more conveninet to test mbssid, so ap's bssid &0xf1*/			
	DBGPRINT(RT_TRACE, ("[TRCAE]%s: E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\r\n",
						__FUNCTION__, PRINT_MAC(pAd->PermanentAddress)));
	
	COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress); //!<Hann
	
	/* Set the current MAC to ASIC */	
	csr2.field.Byte0 = pAd->CurrentAddress[0];
	csr2.field.Byte1 = pAd->CurrentAddress[1];
	csr2.field.Byte2 = pAd->CurrentAddress[2];
	csr2.field.Byte3 = pAd->CurrentAddress[3];
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW0, csr2.word);
	csr3.word = 0;
	csr3.field.Byte4 = pAd->CurrentAddress[4];
	{
		csr3.field.Byte5 = pAd->CurrentAddress[5];
		csr3.field.U2MeMask = 0xff;
	}
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW1, csr3.word);

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW0, csr2.word);
	csr3.word &= 0xff00ffff;
	csr3.word |= 0x00410000;				// HW test code
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW1, csr3.word);
#endif /* HDR_TRANS_SUPPORT */

	DBGPRINT(RT_INFO,("[INFO]%s: Current MAC: =%02x:%02x:%02x:%02x:%02x:%02x\r\n",
						__FUNCTION__, PRINT_MAC(pAd->CurrentAddress)));

	/* if E2PROM version mismatch with driver's expectation, then skip*/
	/* all subsequent E2RPOM retieval and set a system error bit to notify GUI*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_VERSION_OFFSET, Version.word);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	DBGPRINT(RT_TRACE, ("[TRACE]%s: E2PROM: Version = %d, FAE release #%d\r\n", 
						__FUNCTION__, Version.field.Version, Version.field.FaeReleaseNumber));

#ifdef MT7601
	if (IS_MT7601(pAd))
	{
		if (Version.field.Version > MT7601_VALID_EEPROM_VERSION)
		{
			DBGPRINT(RT_WARN, ("[WARN]%s: MT7601 E2PROM: WRONG VERSION 0x%x, should be %d\r\n",
								__FUNCTION__, Version.field.Version, MT7601_VALID_EEPROM_VERSION));
		}
	}
	else
#endif /* MT7601 */
	if (Version.field.Version > VALID_EEPROM_VERSION)
	{
		DBGPRINT(RT_WARN, ("[WARN]%s: E2PROM: WRONG VERSION 0x%x, should be %d\r\n",
							__FUNCTION__, Version.field.Version, VALID_EEPROM_VERSION));
	}

	/* Read BBP default value from EEPROM and store to array(EEPROMDefaultValue) in pAd*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] = value;

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] = value;

	/* if not return early. cause fail at emulation.*/
	/* Init the channel number for TX channel power*/


	MT7601_ReadChannelPwr(pAd);
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_COUNTRY_REGION, value);	/* Country Region*/
		pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
	}

	/* We have to parse NIC configuration 0 at here.*/
	/* If TSSI did not have preloaded value, it should reset the TxAutoAgc to false*/
	/* Therefore, we have to read TxAutoAgc control beforehand.*/
	/* Read Tx AGC control bit*/
	Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];

	// TODO: shiang, why we only check oxff00??
	if ((Antenna.word & 0xFF00) == 0xFF00)
		MT7601_AsicAntennaDefaultReset(pAd, &Antenna);
	
	/* Choose the desired Tx&Rx stream.*/
	if ((pAd->CommonCfg.TxStream == 0) || (pAd->CommonCfg.TxStream > Antenna.field.TxPath))
		pAd->CommonCfg.TxStream = Antenna.field.TxPath;

	if ((pAd->CommonCfg.RxStream == 0) || (pAd->CommonCfg.RxStream > Antenna.field.RxPath))
	{
		pAd->CommonCfg.RxStream = Antenna.field.RxPath;

		if ((pAd->MACVersion != RALINK_3883_VERSION) &&
			(pAd->MACVersion != RALINK_2883_VERSION) &&
			(pAd->CommonCfg.RxStream > 2))
		{
			/* only 2 Rx streams for RT2860 series*/
			pAd->CommonCfg.RxStream = 2;
		}
	}

	/* EEPROM offset 0x36 - NIC Configuration 1 */
	NicConfig2.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];

	{
		if ((NicConfig2.word & 0x00ff) == 0xff)
			NicConfig2.word &= 0xff00;

		if ((NicConfig2.word >> 8) == 0xff)
			NicConfig2.word &= 0x00ff;
	}


	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;
		
	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;
	
	DBGPRINT(RT_TRACE, ("[TRACE]%s: RxPath = %d, TxPath = %d\r\n", __FUNCTION__, Antenna.field.RxPath, Antenna.field.TxPath));

	/* Save the antenna for future use*/
	pAd->Antenna.word = Antenna.word;

	/* Set the RfICType here, then we can initialize RFIC related operation callbacks*/
//	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;
	pAd->RfIcType = RFIC_24GHZ;

	/* Read TSSI reference and TSSI boundary for temperature compensation. This is ugly*/
	/* 0. 11b/g*/
	{
		/* these are tempature reference value (0x00 ~ 0xFE)
		   ex: 0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
		   TssiPlusBoundaryG [4] [3] [2] [1] [0] (smaller) +
		   TssiMinusBoundaryG[0] [1] [2] [3] [4] (larger) */
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND1, Power.word);
			pAd->TssiMinusBoundaryG[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND2, Power.word);
			pAd->TssiMinusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND3, Power.word);
			pAd->TssiRefG   = Power.field.Byte0; /* reference value [0] */
			pAd->TssiPlusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND4, Power.word);
			pAd->TssiPlusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND5, Power.word);
			pAd->TssiPlusBoundaryG[4] = Power.field.Byte0;
			pAd->TxAgcStepG = Power.field.Byte1;    
			pAd->TxAgcCompensateG = 0;
			pAd->TssiMinusBoundaryG[0] = pAd->TssiRefG;
			pAd->TssiPlusBoundaryG[0]  = pAd->TssiRefG;

			/* Disable TxAgc if the based value is not right*/
			if (pAd->TssiRefG == 0xff)
				pAd->bAutoTxAgcG = FALSE;
		}

		DBGPRINT(RT_TRACE, ("[TRACE]E2PROM: G Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\r\n",
							pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
							pAd->TssiRefG,
							pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
							pAd->TxAgcStepG, pAd->bAutoTxAgcG));
	}
	
	/* 1. 11a*/
	{
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND1, Power.word);
			pAd->TssiMinusBoundaryA[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND2, Power.word);
			pAd->TssiMinusBoundaryA[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND3, Power.word);
			pAd->TssiRefA = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND4, Power.word);
			pAd->TssiPlusBoundaryA[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND5, Power.word);
			pAd->TssiPlusBoundaryA[4] = Power.field.Byte0;
			pAd->TxAgcStepA = Power.field.Byte1;    
			pAd->TxAgcCompensateA = 0;
			pAd->TssiMinusBoundaryA[0] = pAd->TssiRefA;
			pAd->TssiPlusBoundaryA[0]  = pAd->TssiRefA;

			/* Disable TxAgc if the based value is not right*/
			if (pAd->TssiRefA == 0xff)
				pAd->bAutoTxAgcA = FALSE;
		}

		DBGPRINT(RT_TRACE, ("[TRACE]E2PROM: A Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\r\n",
							pAd->TssiMinusBoundaryA[4], pAd->TssiMinusBoundaryA[3], pAd->TssiMinusBoundaryA[2], pAd->TssiMinusBoundaryA[1],
							pAd->TssiRefA,
							pAd->TssiPlusBoundaryA[1], pAd->TssiPlusBoundaryA[2], pAd->TssiPlusBoundaryA[3], pAd->TssiPlusBoundaryA[4],
							pAd->TxAgcStepA, pAd->bAutoTxAgcA));
	}	
	pAd->BbpRssiToDbmDelta = 0x0;
	
	/* Read frequency offset setting for RF*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);

	if ((value & 0x00FF) != 0x00FF)
		pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
	else
		pAd->RfFreqOffset = 0;
	
#ifdef MT7601
	if (IS_MT7601(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET_COMPERSATION, value);
		value = (value >> 8) & 0xFF;
		if ( value != 0xFF )
		{
			if ( value & 0x80 )
				pAd->RfFreqOffset -= (value & 0x7F);
			else
				pAd->RfFreqOffset += value;
		}
		
	}
#endif /* MT7601 */
	
	DBGPRINT(RT_TRACE, ("[TRACE]E2PROM: RF FreqOffset=0x%lx \r\n", pAd->RfFreqOffset));

	/*CountryRegion byte offset (38h)*/
	{
		value = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] >> 8;		/* 2.4G band*/
		value2 = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] & 0x00FF;	/* 5G band*/
	}
	
	if ((value <= REGION_MAXIMUM_BG_BAND) || (value == REGION_32_BG_BAND) || (value == REGION_33_BG_BAND))
	{
		pAd->CommonCfg.CountryRegion = ((UCHAR) value) | 0x80;
	}

	if (value2 <= REGION_MAXIMUM_A_BAND)
	{
		pAd->CommonCfg.CountryRegionForABand = ((UCHAR) value2) | 0x80;
	}
	
	/* Get RSSI Offset on EEPROM 0x9Ah & 0x9Ch.*/
	/* The valid value are (-10 ~ 10) */
	/* */
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, value);
		pAd->BGRssiOffset[0] = value & 0x00ff;
		pAd->BGRssiOffset[1] = (value >> 8);
	}

#ifdef MT7601
	if (IS_MT7601(pAd))
	{
		; // MT7601 not support BGRssiOffset[2]
	}
	else
#endif /* MT7601 */
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET+2, value);
/*		if (IS_RT2860(pAd))  RT2860 supports 3 Rx and the 2.4 GHz RSSI #2 offset is in the EEPROM 0x48*/
			pAd->BGRssiOffset[2] = value & 0x00ff;
		pAd->ALNAGain1 = (value >> 8);
	}

	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, value);
		pAd->BLNAGain = value & 0x00ff;
		pAd->ALNAGain0 = (value >> 8);
	}
	

#ifdef MT7601
	if (IS_MT7601(pAd))
	{
		;	// MT7601 not support A Band
	}
	else
#endif /* MT7601 */
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);
		pAd->ARssiOffset[0] = value & 0x00ff;
		pAd->ARssiOffset[1] = (value >> 8);
	}

#ifdef MT7601
	if (IS_MT7601(pAd))
	{
		;	// MT7601 not support A Band
	}
	else
#endif /* MT7601 */
	{
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET+2), value);
		pAd->ARssiOffset[2] = value & 0x00ff;
		pAd->ALNAGain2 = (value >> 8);
	}


	if (((UCHAR)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;
	if (((UCHAR)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	/* Validate 11a/b/g RSSI 0/1/2 offset.*/
	for (i = 0 ; i < 3; i++)
	{
		if ((pAd->BGRssiOffset[i] < -10) || (pAd->BGRssiOffset[i] > 10))
			pAd->BGRssiOffset[i] = 0;

		if ((pAd->ARssiOffset[i] < -10) || (pAd->ARssiOffset[i] > 10))
			pAd->ARssiOffset[i] = 0;
	}
	
		
	RTMPReadTxPwrPerRate(pAd);

#ifdef SINGLE_SKU
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_DEFINE_MAX_TXPWR, pAd->CommonCfg.DefineMaxTxPwr);
	}

	/*
		Some dongle has old EEPROM value, use ModuleTxpower for saving correct value fo DefineMaxTxPwr.
		ModuleTxpower will override DefineMaxTxPwr (value from EEPROM) if ModuleTxpower is not zero.
	*/
	if (pAd->CommonCfg.ModuleTxpower > 0) 
		pAd->CommonCfg.DefineMaxTxPwr = pAd->CommonCfg.ModuleTxpower;

	DBGPRINT(RT_TRACE, ("[TRACE]TX Power set for SINGLE SKU MODE is : 0x%04x \r\n", pAd->CommonCfg.DefineMaxTxPwr));
	
	pAd->CommonCfg.bSKUMode = FALSE;
	if ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF) <= 0x50)
	{
		if (IS_RT3883(pAd))
			pAd->CommonCfg.bSKUMode = TRUE;
		else if ((pAd->CommonCfg.AntGain > 0) && (pAd->CommonCfg.BandedgeDelta >= 0))
			pAd->CommonCfg.bSKUMode = TRUE;
	}
	DBGPRINT(RT_TRACE, ("[TRACE]Single SKU Mode is %s\r\n", pAd->CommonCfg.bSKUMode ? "Enable" : "Disable"));
#endif /* SINGLE_SKU */

#ifdef SINGLE_SKU_V2
	InitSkuRateDiffTable(pAd);
#endif /* SINGLE_SKU_V2 */


#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT65xx
	if (IS_RT6590(pAd))
	{
		; // TODO: wait TC6008 EEPROM format
	}
	else
#endif /* RT65xx */
	{
		/*
			Internal Tx ALC support is starting from RT3370 / RT3390, which combine PA / LNA in single chip.
			The old chipset don't have this, add new feature flag RTMP_INTERNAL_TX_ALC.
		*/
		RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
		if (value == 0xFFFF) /*EEPROM is empty*/
		   	pAd->TxPowerCtrl.bInternalTxALC = FALSE;
		else if (value & 1<<13) 
		   	pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		else 
		   	pAd->TxPowerCtrl.bInternalTxALC = FALSE;

	}
	DBGPRINT(RT_TRACE, ("TXALC> bInternalTxALC = %d\r\n", pAd->TxPowerCtrl.bInternalTxALC));
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RT3290
	if (IS_RT3290(pAd))
		RT3290_eeprom_access_grant(pAd, FALSE);
#endif /* RT3290 */

	DBGPRINT(RT_TRACE, ("[TRACE]%s: pAd->Antenna.field.BoardType = %d, IS_MINI_CARD(pAd) = %d, IS_RT5390U(pAd) = %d\r\n", 
						__FUNCTION__,
						pAd->Antenna.field.BoardType,
						IS_MINI_CARD(pAd),
						IS_RT5390U(pAd)));

	DBGPRINT(RT_TRACE, ("[TRACE]<---%s\r\n", __FUNCTION__));
}
