#include "rtmp_ctrl.h"
#include "types.h"
#include "rtmp.h"
#include "common.h"
#include "usb_io.h"
#include "asic_ctrl.h"


VOID RT28XXDMAEnable(
	IN PRTMP_ADAPTER 	pAd)
{	
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x4);

	if (AsicWaitPDMAIdle(pAd, 200, 1000) == FALSE) { 
	} 
}


VOID RTMPDrvOpen(
	IN PRTMP_ADAPTER	pAd)
{
	/* Now Enable RxTx*/
	RTMPEnableRxTx(pAd);
	{
		UINT32 reg = 0;
		RTMP_IO_READ32(pAd, 0x1300, &reg);  /* clear garbage interrupts*/
	}

	AsicWaitPDMAIdle(pAd, 5, 10);
}


VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 rx_filter_flag;

	DBGPRINT(RT_TRACE, ("[TRACE]--->%s\n", __FUNCTION__));

	RT28XXDMAEnable(pAd);

	/* enable RX of MAC block*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		rx_filter_flag = NO_LINK_FILTER;
else IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	rx_filter_flag = STANORMAL|0x8;   	// LINK_FILTER;// STANORMAL;
else		
	rx_filter_flag = STANORMAL|0x8;     /* Staion not drop control frame will fail WiFi Certification.*/

	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
}


VOID RTMPDisableRxTx(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG val;
	
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &val);
	val &= 0xFFFFFF3F;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, val);
}


VOID RTMPSetLinkFliter(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN			bTurnOn)
{
	RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x20);
	
	if (bTurnOn == TRUE)
	{
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, LINK_FILTER);
	}
	else
	{
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, NO_LINK_FILTER);
	}
}

VOID RTMPSetPowerSuspend(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			off)
{
	if (off)
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	else
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xC);
}


VOID RTMPSetPhyMode(
	IN PRTMP_ADAPTER 	pAd,
	IN ULONG 			phymode)
{
	INT i;

	/* the selected phymode must be supported by the RF IC encoded in E2PROM*/
	phymode = WMODE_GN;
	pAd->CommonCfg.PhyMode = WMODE_GN;

	DBGPRINT(RT_TRACE,("[TRACE]%s : PhyMode=%d, channel=%d \n", __FUNCTION__, pAd->CommonCfg.PhyMode, pAd->CommonCfg.Channel));

	/* sanity check user setting*/
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
			break;
	}

	if (i == pAd->ChannelListNum)
	{ 
		pAd->CommonCfg.Channel = pAd->ChannelList[0].Channel; 
	}
	
	NdisZeroMemory(pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	switch (phymode) {
		case (WMODE_B):
			pAd->CommonCfg.SupRate[0]  = 0x82; 		/* 1 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[1]  = 0x84;		/* 2 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[2]  = 0x8B;	  	/* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[3]  = 0x96;	  	/* 11 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRateLen  = 4;
			pAd->CommonCfg.ExtRateLen  = 0;
			pAd->CommonCfg.DesireRate[0]  = 2;	   	/* 1 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[1]  = 4;	   	/* 2 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[2]  = 11;    	/* 5.5 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[3]  = 22;    	/* 11 mbps, in units of 0.5 Mbps*/
			/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_CCK;  This MODE is only FYI. not use*/
			break;

		/*
			In current design, we will put supported/extended rate element in
			beacon even we are 11n-only mode.
			Or some 11n stations will not connect to us if we do not put
			supported/extended rate element in beacon.
		*/
		case (WMODE_G):
		case (WMODE_B | WMODE_G):
		case (WMODE_A | WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
		case (WMODE_GN):
		case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
		case (WMODE_B | WMODE_G | WMODE_GN):
		case (WMODE_G | WMODE_GN):
#endif /* DOT11_N_SUPPORT */
			pAd->CommonCfg.SupRate[0]  = 0x82; 		/* 1 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[1]  = 0x84;	  	/* 2 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[2]  = 0x8B;	  	/* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[3]  = 0x96;	  	/* 11 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[4]  = 0x12;	  	/* 9 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[5]  = 0x24;	  	/* 18 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[6]  = 0x48;	  	/* 36 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[7]  = 0x6c;	  	/* 54 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRateLen  = 8;
			pAd->CommonCfg.ExtRate[0]  = 0x0C;	  	/* 6 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.ExtRate[1]  = 0x18;	  	/* 12 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.ExtRate[2]  = 0x30;	  	/* 24 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.ExtRate[3]  = 0x60;	  	/* 48 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.ExtRateLen  = 4;
			pAd->CommonCfg.DesireRate[0]  = 2;	   	/* 1 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[1]  = 4;	   	/* 2 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[2]  = 11;    	/* 5.5 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[3]  = 22;    	/* 11 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[4]  = 12;    	/* 6 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[5]  = 18;    	/* 9 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[6]  = 24;    	/* 12 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[7]  = 36;    	/* 18 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[8]  = 48;    	/* 24 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[9]  = 72;    	/* 36 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[10] = 96;    	/* 48 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[11] = 108;   	/* 54 mbps, in units of 0.5 Mbps*/
			break;

		case (WMODE_A):
#ifdef DOT11_N_SUPPORT
		case (WMODE_A | WMODE_AN):
		case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
		case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		case (WMODE_A | WMODE_AN | WMODE_AC):
		case (WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
			pAd->CommonCfg.SupRate[0]  = 0x8C;	  /* 6 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[1]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[2]  = 0x98;	  /* 12 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[3]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[4]  = 0xb0;	  /* 24 mbps, in units of 0.5 Mbps, basic rate*/
			pAd->CommonCfg.SupRate[5]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[6]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.SupRateLen  = 8;
			pAd->CommonCfg.ExtRateLen  = 0;
			pAd->CommonCfg.DesireRate[0]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[1]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[2]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[3]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[4]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[5]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[6]  = 96;    /* 48 mbps, in units of 0.5 Mbps*/
			pAd->CommonCfg.DesireRate[7]  = 108;   /* 54 mbps, in units of 0.5 Mbps*/
			/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_OFDM;  This MODE is only FYI. not use*/
			break;

		default:
			break;
	}

	
#ifdef DOT11_N_SUPPORT
	RTMPSetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
}

INT	RTMPSetCommonHT(
	IN PRTMP_ADAPTER 	pAd)
{
	OID_SET_HT_PHYMODE SetHT;

	SetHT.PhyMode = (RT_802_11_PHY_MODE)pAd->CommonCfg.PhyMode;
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
	SetHT.MCS = MCS_AUTO;
	SetHT.BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
	SetHT.STBC = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
	SetHT.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;		

	RTMPSetHT(pAd, &SetHT);
	return TRUE;
}

 
VOID RTMPSetHT(
	IN PRTMP_ADAPTER 		pAd,
	IN POID_SET_HT_PHYMODE 	pHTPhyMode)
{
	UCHAR RxStream = pAd->CommonCfg.RxStream;
	INT bw;
	RT_HT_CAPABILITY *rt_ht_cap = &pAd->CommonCfg.DesiredHtPhy;
	HT_CAPABILITY_IE *ht_cap= &pAd->CommonCfg.HtCapability;
	
	DBGPRINT(RT_TRACE, ("[TRACE]%s: HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
						__FUNCTION__,
						pHTPhyMode->HtMode, pHTPhyMode->ExtOffset, 
						pHTPhyMode->MCS, pHTPhyMode->BW,
						pHTPhyMode->STBC, pHTPhyMode->SHORTGI));
			
	/* Don't zero supportedHyPhy structure.*/
	RTMPZeroMemory(ht_cap, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));
	RTMPZeroMemory(rt_ht_cap, sizeof(RT_HT_CAPABILITY));

   	if (pAd->CommonCfg.bRdg)
	{
		ht_cap->ExtHtCapInfo.PlusHTC = 1;
		ht_cap->ExtHtCapInfo.RDGSupport = 1;
	}
	else
	{
		ht_cap->ExtHtCapInfo.PlusHTC = 0;
		ht_cap->ExtHtCapInfo.RDGSupport = 0;
	}


	if (RxStream == 1)
	{
		ht_cap->HtCapParm.MaxRAmpduFactor = 2;
		rt_ht_cap->MaxRAmpduFactor = 2;
	}
	else
	{
		ht_cap->HtCapParm.MaxRAmpduFactor = 3;
		rt_ht_cap->MaxRAmpduFactor = 3;
	}

	DBGPRINT(RT_TRACE, ("[TRACE]%s: RxBAWinLimit = %d\n", __FUNCTION__, pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	/* Mimo power save, A-MSDU size, */
	rt_ht_cap->AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	rt_ht_cap->AmsduSize = (UCHAR)pAd->CommonCfg.BACapability.field.AmsduSize;
	rt_ht_cap->MimoPs = (UCHAR)pAd->CommonCfg.BACapability.field.MMPSmode;
	rt_ht_cap->MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	ht_cap->HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	ht_cap->HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	ht_cap->HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	
	DBGPRINT(RT_TRACE, ("[TRACE]%s: AMsduSize = %d, MimoPs = %d, MpduDensity = %d, MaxRAmpduFactor = %d\n",
						__FUNCTION__,
						rt_ht_cap->AmsduSize, 
						rt_ht_cap->MimoPs,
						rt_ht_cap->MpduDensity,
						rt_ht_cap->MaxRAmpduFactor));
	
	if (pHTPhyMode->HtMode == HTMODE_GF)
	{
		ht_cap->HtCapInfo.GF = 1;
		rt_ht_cap->GF = 1;
	}
	else
		rt_ht_cap->GF = 0;
	
	/* Decide Rx MCSSet*/
	switch (RxStream)
	{
		case 3:
			ht_cap->MCSSet[2] =  0xff;
		case 2:
			ht_cap->MCSSet[1] =  0xff;
		case 1:
		default:
			ht_cap->MCSSet[0] =  0xff;
			break;
	}

	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pHTPhyMode->BW == BW_40))
	{
		pHTPhyMode->BW = BW_20;
		ht_cap->HtCapInfo.Forty_Mhz_Intolerant = 1;
	}

	// TODO: shiang-6590, how about the "bw" when channel 14 for JP region???
	bw = BW_20;
	if (pHTPhyMode->BW == BW_40)
	{
		ht_cap->MCSSet[4] = 0x1; /* MCS 32*/
		ht_cap->HtCapInfo.ChannelWidth = 1;
		if (pAd->CommonCfg.Channel <= 14) 		
			ht_cap->HtCapInfo.CCKmodein40 = 1;

		rt_ht_cap->ChannelWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = (pHTPhyMode->ExtOffset == EXTCHA_BELOW)? (EXTCHA_BELOW): EXTCHA_ABOVE;
		/* Set Regsiter for extension channel position.*/
		RTMP_MACSetCtrlCH(pAd, pHTPhyMode->ExtOffset);

		/* Turn on BBP 40MHz mode now only as AP . */
		/* Sta can turn on BBP 40MHz after connection with 40MHz AP. Sta only broadcast 40MHz capability before connection.*/
		if (1)//(pAd->OpMode == OPMODE_AP) || INFRA_ON(pAd) || ADHOC_ON(pAd))
		{
			RTMP_BBPSetCtrlCH(pAd, pHTPhyMode->ExtOffset);	
			bw = BW_40;
		}
	}
	else
	{
		ht_cap->HtCapInfo.ChannelWidth = 0;
		rt_ht_cap->ChannelWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		/* Turn on BBP 20MHz mode by request here.*/
		bw = BW_20;
	}
 
	RTMP_BBPSetBW(pAd, bw);

	if (pHTPhyMode->STBC == STBC_USE)
	{
		if (pAd->Antenna.field.TxPath >= 2)
		{
			ht_cap->HtCapInfo.TxSTBC = 1;
			rt_ht_cap->TxSTBC = 1;
		}
		else
		{
			ht_cap->HtCapInfo.TxSTBC = 0;
			rt_ht_cap->TxSTBC = 0; 	
		}
		
		/*
			RxSTBC
				0: not support,
				1: support for 1SS
				2: support for 1SS, 2SS
				3: support for 1SS, 2SS, 3SS
		*/
		if (pAd->Antenna.field.RxPath >= 1)
		{
			ht_cap->HtCapInfo.RxSTBC = 1;
			rt_ht_cap->RxSTBC = 1;
		}
		else
		{
			ht_cap->HtCapInfo.RxSTBC = 0; 
			rt_ht_cap->RxSTBC = 0; 	
		}
	}
	else
	{
		rt_ht_cap->TxSTBC = 0;
		rt_ht_cap->RxSTBC = 0;
	}

	if (pHTPhyMode->SHORTGI == GI_400)
	{
		ht_cap->HtCapInfo.ShortGIfor20 = 1;
		ht_cap->HtCapInfo.ShortGIfor40 = 1;
		rt_ht_cap->ShortGIfor20 = 1;
		rt_ht_cap->ShortGIfor40 = 1;
	}
	else
	{
		ht_cap->HtCapInfo.ShortGIfor20 = 0;
		ht_cap->HtCapInfo.ShortGIfor40 = 0;
		rt_ht_cap->ShortGIfor20 = 0;
		rt_ht_cap->ShortGIfor40 = 0;
	}
	
	/* We support link adaptation for unsolicit MCS feedback, set to 2.*/
	pAd->CommonCfg.AddHTInfo.ControlChan = pAd->CommonCfg.Channel;
	/* 1, the extension channel above the control channel. */
	
	/* EDCA parameters used for AP's own transmission*/
	if (pAd->CommonCfg.APEdcaParm.bValid == FALSE)
	{
		pAd->CommonCfg.APEdcaParm.bValid = TRUE;
		pAd->CommonCfg.APEdcaParm.Aifsn[0] = 1;//3;
		pAd->CommonCfg.APEdcaParm.Aifsn[1] = 7;
		pAd->CommonCfg.APEdcaParm.Aifsn[2] = 1;
		pAd->CommonCfg.APEdcaParm.Aifsn[3] = 1;
	
		pAd->CommonCfg.APEdcaParm.Cwmin[0] = 2;//4;
		pAd->CommonCfg.APEdcaParm.Cwmin[1] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmin[2] = 3;
		pAd->CommonCfg.APEdcaParm.Cwmin[3] = 2;

		pAd->CommonCfg.APEdcaParm.Cwmax[0] = 4;//6;
		pAd->CommonCfg.APEdcaParm.Cwmax[1] = 10;
		pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

		pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[2]  = 94;	
		pAd->CommonCfg.APEdcaParm.Txop[3]  = 47;	
	}
	
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
}


VOID RTMPReadTxPwrPerRate(
	IN PRTMP_ADAPTER 	pAd)
{
	ULONG		data, Adata, Gdata;
	USHORT		i, value, value2;
	USHORT		value_1, value_2, value_3, value_4;
	INT			Apwrdelta, Gpwrdelta;
	UCHAR		t1,t2,t3,t4;
	BOOLEAN		bApwrdeltaMinus = TRUE, bGpwrdeltaMinus = TRUE;

#ifdef RT8592
	if (IS_RT8592(pAd)) {
		RT85592ReadTxPwrPerRate(pAd);
		return;
	}
#endif /* RT8592 */

#ifdef RT65xx
	if (IS_RT6590(pAd)) {
		RT6590ReadTxPwrPerRate(pAd);
		return;
	}
#endif /* RT65xx */


#ifdef MT7601
	if (IS_MT7601(pAd)) {
		MT7601_ReadTxPwrPerRate(pAd);
		return;
	}
#endif /* MT7601 */

	/* For default one, go here!! */
	{	
		
		/* Get power delta for 20MHz and 40MHz.*/
		DBGPRINT(RT_TRACE, ("Txpower per Rate\n"));
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value2);
		Apwrdelta = 0;
		Gpwrdelta = 0;

		if ((value2 & 0xff) != 0xff)
		{
			if ((value2 & 0x80))
				Gpwrdelta = (value2&0xf);
			
			if ((value2 & 0x40))
				bGpwrdeltaMinus = FALSE;
			else
				bGpwrdeltaMinus = TRUE;
		}
		if ((value2 & 0xff00) != 0xff00)
		{
			if ((value2 & 0x8000))
				Apwrdelta = ((value2&0xf00)>>8);

			if ((value2 & 0x4000))
				bApwrdeltaMinus = FALSE;
			else
				bApwrdeltaMinus = TRUE;
		}	
		DBGPRINT(RT_TRACE, ("Gpwrdelta = %x, Apwrdelta = %x .\n", Gpwrdelta, Apwrdelta));

		
		/* Get Txpower per MCS for 20MHz in 2.4G.*/
		
		for (i=0; i<5; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, value);
			data = value;

			/* use value_1 ~ value_4 for code size reduce */
			value_1 = value&0xf;
			value_2 = (value&0xf0)>>4;
			value_3 = (value&0xf00)>>8;
			value_4 = (value&0xf000)>>12;

			if (bApwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Apwrdelta)
					t1 = value_1-(Apwrdelta);
				else
					t1 = 0;
				if (value_2 > Apwrdelta)
					t2 = value_2-(Apwrdelta);
				else
					t2 = 0;
				if (value_3 > Apwrdelta)
					t3 = value_3-(Apwrdelta);
				else
					t3 = 0;
				if (value_4 > Apwrdelta)
					t4 = value_4-(Apwrdelta);
				else
					t4 = 0;
			}				
			Adata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Gpwrdelta)
					t1 = value_1-(Gpwrdelta);
				else
					t1 = 0;
				if (value_2 > Gpwrdelta)
					t2 = value_2-(Gpwrdelta);
				else
					t2 = 0;
				if (value_3 > Gpwrdelta)
					t3 = value_3-(Gpwrdelta);
				else
					t3 = 0;
				if (value_4 > Gpwrdelta)
					t4 = value_4-(Gpwrdelta);
				else
					t4 = 0;
			}				
			Gdata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
			
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, value);

			/* use value_1 ~ value_4 for code size reduce */
			value_1 = value&0xf;
			value_2 = (value&0xf0)>>4;
			value_3 = (value&0xf00)>>8;
			value_4 = (value&0xf000)>>12;

			if (bApwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Apwrdelta)
					t1 = value_1-(Apwrdelta);
				else
					t1 = 0;
				if (value_2 > Apwrdelta)
					t2 = value_2-(Apwrdelta);
				else
					t2 = 0;
				if (value_3 > Apwrdelta)
					t3 = value_3-(Apwrdelta);
				else
					t3 = 0;
				if (value_4 > Apwrdelta)
					t4 = value_4-(Apwrdelta);
				else
					t4 = 0;
			}				
			Adata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Gpwrdelta)
					t1 = value_1-(Gpwrdelta);
				else
					t1 = 0;
				if (value_2 > Gpwrdelta)
					t2 = value_2-(Gpwrdelta);
				else
					t2 = 0;
				if (value_3 > Gpwrdelta)
					t3 = value_3-(Gpwrdelta);
				else
					t3 = 0;
				if (value_4 > Gpwrdelta)
					t4 = value_4-(Gpwrdelta);
				else
					t4 = 0;
			}				
			Gdata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
			data |= (value<<16);

			/* For 20M/40M Power Delta issue */		
			pAd->Tx20MPwrCfgABand[i] = data;
			pAd->Tx20MPwrCfgGBand[i] = data;
			pAd->Tx40MPwrCfgABand[i] = Adata;
			pAd->Tx40MPwrCfgGBand[i] = Gdata;
			
			if (data != 0xffffffff)
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);
			DBGPRINT(RT_TRACE, ("20MHz BW, 2.4G band-%lx,  Adata = %lx,  Gdata = %lx \n", data, Adata, Gdata));
		}
	}

	/* Extra set MAC registers to compensate Tx power if any */
	MT7601_AsicExtraPowerOverMAC(pAd);
}


VOID RTMP_EdccaStatusRead(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32 period_us;

	INT percent;
	RX_STA_CNT1_STRUC RxStaCnt1;
	UINT32 ch_idle_stat = 0,  ed_stat = 0;

	period_us = pAd->ed_chk_period * 1000;
	RTMP_IO_READ32(pAd, CH_IDLE_STA, &ch_idle_stat);
	RTMP_IO_READ32(pAd, ED_CCA_TIMER, &ed_stat);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);

	pAd->false_cca_stat += RxStaCnt1.field.FalseCca;
	pAd->ed_stat = ed_stat;

	percent = (pAd->ed_stat * 100 ) / period_us;
	
	if ((pAd->ed_threshold > 0) && (period_us > 0) && (pAd->ed_block_tx_threshold > 0)) {
		percent = (pAd->ed_stat * 100 ) / period_us;
		if (percent > 100)
			percent = 100;

		if (pAd->false_cca_stat > pAd->ed_false_cca_threshold) 
		{
			pAd->ed_false_cca_cnt ++;						
		}
		else
		{
			pAd->ed_false_cca_cnt = 0;
		}

		if (percent > pAd->ed_threshold) 
		{
			pAd->ed_trigger_cnt++;
			pAd->ed_silent_cnt = 0;
		}
		else
		{
			pAd->ed_trigger_cnt = 0;
			pAd->ed_silent_cnt++;
		}
	}

	{
		if (pAd->ed_trigger_cnt > pAd->ed_block_tx_threshold) {
			if (pAd->ed_tx_stoped == FALSE) {		
				pAd->ed_tx_stoped = TRUE;
				RTMP_EdccaTxCtrl(pAd, TRUE);
				DBGPRINT(RT_TRACE, ("[TRACE]%s: EDCCCA TX STOP\n", __FUNCTION__));
			}
		}

		if (pAd->ed_silent_cnt > pAd->ed_block_tx_threshold) {
			if (pAd->ed_tx_stoped == TRUE) {
				pAd->ed_tx_stoped = FALSE;
				RTMP_EdccaTxCtrl(pAd, FALSE);
				DBGPRINT(RT_TRACE, ("[TRACE]%s: EDCCCA TX Start\n", __FUNCTION__));
			}
		}
	}
}



VOID RTMP_EdccaTxCtrl(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			stop)
{
	UINT32 macCfg,  macStatus;
	UINT32 MTxCycle;

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &macCfg);
	
	if (stop == TRUE)
	{
		macCfg &= (~0x04);	
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x00150F00);
	}
	else if((pAd->ed_tx_stoped == FALSE) && (pAd->mac_tx_stop == FALSE))
	{
		macCfg |= 0x04;				
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x00150F0F);
	}
	
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, macCfg);

	if (stop == TRUE)
	{
		for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x1)
				RTMPusecDelay(50);
			else
				break;
		}

		if (MTxCycle == 10000)
		{
			DBGPRINT(RT_OFF, ("%s(cnt=%d):stop MTx,macStatus=0x%x!\n", 
				__FUNCTION__, MTxCycle, macStatus));
		}
	}

	DBGPRINT(RT_TRACE, ("[TRACE]%s():%s tx\n", 
				__FUNCTION__, ((stop == TRUE) ? "stop" : "start")));

}

INT RTMP_BBPSetBW(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				bw)
{
	UCHAR val, old_val = 0;
	BOOLEAN bstop = FALSE;
	UINT32 Data, MTxCycle, macStatus;


	if (bw != pAd->CommonCfg.BBPCurrentBW)
		bstop = TRUE;

	if (bstop)
	{
		/* Disable MAC Tx/Rx */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
		Data &= (~0x0C);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);

		/* Check MAC Tx/Rx idle */
		for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x3)
				RTMPusecDelay(50);
			else
				break;
		}
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &old_val);
	val = (old_val & (~0x18));
	switch (bw)
	{
		case BW_20:
			val &= (~0x18);
			break;
		case BW_40:
			val |= (0x10);
			break;
		case BW_10:
			val |= 0x08;
			break;	
	}

	if (val != old_val) 
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, val);
	}

	if (bstop)
	{
		/* Enable MAC Tx/Rx */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
		Data |= 0x0C;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
	}

	pAd->CommonCfg.BBPCurrentBW = bw;
	
	return TRUE;
}



INT RTMP_BBPSetCtrlCH(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				ext_ch)
{
	UCHAR val, old_val = 0;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &old_val);
	val = old_val;
	switch (ext_ch)
	{
		case EXTCHA_BELOW:
			val |= (0x20);
			break;
		case EXTCHA_ABOVE:
		case EXTCHA_NONE:
			val &= (~0x20);
			break;
	}

	if (val != old_val)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, val);

	return TRUE;
}


UCHAR RTMP_BBPSetRxPath(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			rxpath)
{
	UCHAR val = 0;
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &val);
	val &= (~0x18);
	if (rxpath == 3)
		val |= (0x10);
	else if (rxpath == 2)
		val |= (0x8);
	else if (rxpath == 1)
		val |= (0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, val);

	return TRUE;
}



INT RTMP_BBPSetTxDAC(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				tx_dac)
{
	UCHAR val, old_val = 0;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &old_val);
	val = old_val & (~0x18);
	switch (tx_dac)
	{
		case 2:
			val |= 0x10;
			break;
		case 1:
			val |= 0x08;
			break;
		case 0:
		default:
			break;
	}

	if (val != old_val) 
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, val);
	}

	return TRUE;
}


INT RTMP_MACSetCtrlCH(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				extch)
{
	UINT32 val, band_cfg;

	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x1);
	switch (extch)
	{
		case EXTCHA_ABOVE:
			val &= (~0x1);
			break;
		case EXTCHA_BELOW:
			val |= (0x1);
			break;
		case EXTCHA_NONE:
			val &= (~0x1);
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);
	
	return TRUE;
}

