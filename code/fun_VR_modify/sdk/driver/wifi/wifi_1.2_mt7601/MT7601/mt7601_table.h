#ifndef __SN93360_WIFI_TABLE_H
#define __SN93360_WIFI_TABLE_H

#include "rtmp.h"

typedef struct _MT7601_FREQ_ITEM {
	UINT8 Channel;
	UINT8 K_R17;
	UINT8 K_R18;
	UINT8 K_R19;
	UINT8 N_R20;
} MT7601_FREQ_ITEM;


/* 
	ASIC register initialization sets
*/
const __align(32) RTMP_REG_PAIR MACRegTable[] = {
#ifndef MT7601
#if defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x200)
	{BCN_OFFSET0,			0xf8f0e8e0}, /* 0x3800(e0), 0x3A00(e8), 0x3C00(f0), 0x3E00(f8), 512B for each beacon */
	{BCN_OFFSET1,			0x6f77d0c8}, /* 0x3200(c8), 0x3400(d0), 0x1DC0(77), 0x1BC0(6f), 512B for each beacon */
#elif defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x100)
	{BCN_OFFSET0,			0xece8e4e0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
	{BCN_OFFSET1,			0xfcf8f4f0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
#endif /* HW_BEACON_OFFSET */
#endif /* MT7601 */

	{LEGACY_BASIC_RATE,		0x00000001}, /*  Basic rate set bitmap*///{LEGACY_BASIC_RATE,		0x0000013f}, /*  Basic rate set bitmap*/
	{HT_BASIC_RATE,		0x00008003}, /* Basic HT rate set , 20M, MCS=3, MM. Format is the same as in TXWI.*/
	{MAC_SYS_CTRL,		0x00}, /* 0x1004, , default Disable RX*/
	{RX_FILTR_CFG,		0x1FFFF}, /*0x1400  , RX filter control,  */
	{BKOFF_SLOT_CFG,	0x209}, /* default set short slot time, CC_DELAY_TIME should be 2	 */
	/*{TX_SW_CFG0,		0x40a06},  Gary,2006-08-23 */
	{TX_SW_CFG0,		0x0}, 		/* Gary,2008-05-21 for CWC test */
	{TX_SW_CFG1,		0x80606}, /* Gary,2006-08-23 */
	{TX_LINK_CFG,		0x1020},		/* Gary,2006-08-23 */
	/*{TX_TIMEOUT_CFG,	0x00182090},	 CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT*/
	{TX_TIMEOUT_CFG,	0x000a2090},	/* CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT , Modify for 2860E ,2007-08-01*/
	{MAX_LEN_CFG,		MAX_AGGREGATION_SIZE | 0x00001000},	/* 0x3018, MAX frame length. Max PSDU = 16kbytes.*/
#ifndef MT7601
	{LED_CFG,		0x7f031e46}, /* Gary, 2006-08-23*/
#endif /* MT7601 */

#ifdef RLT_MAC

#ifdef MT7601
#ifdef CONFIG_MULTI_CHANNEL
	{TX_MAX_PCNT,		0x1f1f1f1f/*0x1fbf1f1f */},
#else
	{TX_MAX_PCNT,		0x1fbf1f1f},
#endif /* CONFIG_MULTI_CHANNEL */
	{RX_MAX_PCNT,		0x9f},
#else
	{TX_MAX_PCNT,		0xbfbf3f1f},
	{RX_MAX_PCNT,		0x9f},
#endif

// TODO: shiang-6590, need set this in FPGA mode
#ifdef RTMP_MAC_USB
#endif /* RTMP_MAC_USB */
#else
#ifdef INF_AMAZON_SE
	{PBF_MAX_PCNT,			0x1F3F6F6F}, 	/*iverson modify for usb issue, 2008/09/19*/
											/* 6F + 6F < total page count FE*/
											/* so that RX doesn't occupy TX's buffer space when WMM congestion.*/
#else
	{PBF_MAX_PCNT,			0x1F3FBF9F}, 	/*0x1F3f7f9f},		Jan, 2006/04/20*/
#endif /* INF_AMAZON_SE */
#endif /* RLT_MAC */

	/*{TX_RTY_CFG,			0x6bb80408},	 Jan, 2006/11/16*/
/* WMM_ACM_SUPPORT*/
/*	{TX_RTY_CFG,			0x6bb80101},	 sample*/
	{TX_RTY_CFG,			0x47d00c0c},	/* Jan, 2006/11/16, Set TxWI->ACK =0 in Probe Rsp Modify for 2860E ,2007-08-03*/
	
	{AUTO_RSP_CFG,			0x00000013},	/* Initial Auto_Responder, because QA will turn off Auto-Responder*/
	{CCK_PROT_CFG,			0x05740003 /*0x01740003*/},	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */
	{OFDM_PROT_CFG,			0x05740003 /*0x01740003*/},	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */
#ifdef RTMP_MAC_USB
#ifndef MT7601
	{PBF_CFG, 				0xf40006}, 		/* Only enable Queue 2*/
	{WPDMA_GLO_CFG,			0x00000030},		// MT7601U not support WPDMA
#endif /* MT7601 */
	{MM40_PROT_CFG,			0x3F44084},		/* Initial Auto_Responder, because QA will turn off Auto-Responder*/
#endif /* RTMP_MAC_USB */
	{GF20_PROT_CFG,			0x01744004},    /* set 19:18 --> Short NAV for MIMO PS*/
	{GF40_PROT_CFG,			0x03F44084},    
	{MM20_PROT_CFG,			0x01744004},    
	{TXOP_CTRL_CFG,			0x0000583f, /*0x0000243f*/ /*0x000024bf*/},	/*Extension channel backoff.*/
	{TX_RTS_CFG,			0x01092b20},		// enable RTS fall back

	{EXP_ACK_TIME,			0x002400ca},	/* default value */
	{TXOP_HLDR_ET, 			0x00000002},

	/* Jerry comments 2008/01/16: we use SIFS = 10us in CCK defaultly, but it seems that 10us
		is too small for INTEL 2200bg card, so in MBSS mode, the delta time between beacon0
		and beacon1 is SIFS (10us), so if INTEL 2200bg card connects to BSS0, the ping
		will always lost. So we change the SIFS of CCK from 10us to 16us. */
	{XIFS_TIME_CFG,			0x33a41010},
#if defined(RT65xx) || defined(MT7601)
	{PWR_PIN_CFG,			0x00000000},
#else
	{PWR_PIN_CFG,			0x00000003},	/* patch for 2880-E*/
#endif /* defined(RT65xx) || defined(MT7601) */
	
	 {LG_FBK_CFG0,  			0xEDCBA983},
	 {LG_FBK_CFG1,  			0x2111},
};




//static 
const __align(32) RTMP_REG_PAIR	MT7601_MACRegTable[] = {
	//{TSO_CTRL, 				0x16058},		// must set when enable HDR_TRANS_SUPPORT
	{TSO_CTRL, 				0x06050},		// must set when enable HDR_TRANS_SUPPORT
	{BCN_OFFSET0,			0x18100800}, 
	{BCN_OFFSET1,			0x38302820}, 
	{PBF_SYS_CTRL,			0x80c00},
	{PBF_CFG,				0x7F723c1f},
	{FCE_CTRL,				0x1},
	{0xA38,					0x0},			// For bi-direction RX fifo overflow issue.
	{TX0_RF_GAIN_CORR,		0x003B0005},		// 20120806 Jason Huang
	{TX0_RF_GAIN_ATTEN,		0x00006900},		// 20120806 Jason Huang
	{TX0_BB_GAIN_ATTEN,		0x00000400},		// 20120806 Jason Huang
	{TX_ALC_VGA3,			0x00060006},		// 20120806 Jason Huang
	//{TX_SW_CFG0,			0x400},			// 20120822 Gary
	{TX_SW_CFG0,			0x402},			// 20121017 Jason Huang
	{TX_SW_CFG1,			0x0},			// 20120822 Gary
	{TX_SW_CFG2,			0x0},			// 20120822 Gary

#ifdef HDR_TRANS_SUPPORT
	{HEADER_TRANS_CTRL_REG, 0x2},
#else
	{HEADER_TRANS_CTRL_REG, 0x0},	
#endif /* HDR_TRANS_SUPPORT */

#ifdef CONFIG_RX_CSO_SUPPORT
	{CHECKSUM_OFFLOAD,	0x30f},
	{FCE_PARAM,			0x00256f0f},
#else
	{CHECKSUM_OFFLOAD,	0x200},
	{FCE_PARAM,			0x00254f0f},
#endif /* CONFIG_RX_CSO_SUPPORT */
};
//static 
UCHAR MT7601_NUM_MAC_REG_PARMS = (sizeof(MT7601_MACRegTable) / sizeof(RTMP_REG_PAIR));



//static 
const __align(32) RTMP_REG_PAIR MT7601_BBP_InitRegTb[] = {
	/* TX/RX Controls */
	{BBP_R1, 0x04},
	{BBP_R4, 0x40},
	{BBP_R20, 0x06},
	{BBP_R31, 0x08},
	/* CCK Tx Control  */
	{BBP_R178, 0xFF},
	/* AGC/Sync controls */
	//{BBP_R65, 0x2C},
	{BBP_R66, 0x14},
	{BBP_R68, 0x8B},
	{BBP_R69, 0x12},
	{BBP_R70, 0x09},
	{BBP_R73, 0x11},
	{BBP_R75, 0x60},
	{BBP_R76, 0x44},
	{BBP_R84, 0x9A},
	{BBP_R86, 0x38},
	{BBP_R91, 0x07},
	{BBP_R92, 0x02},	
	/* Rx Path Controls */
	{BBP_R99, 0x50},	
	{BBP_R101, 0x00},
	{BBP_R103, 0xC0},
	{BBP_R104, 0x92},
	{BBP_R105, 0x3C},
	{BBP_R106, 0x03},
	{BBP_R128, 0x12},
	/* Change RXWI content: Gain Report */
	{BBP_R142, 0x04},
	{BBP_R143, 0x37},
	/* Change RXWI content: Antenna Report */
	{BBP_R142, 0x03},
	{BBP_R143, 0x99},
	/* Calibration Index Register */
	/* CCK Receiver Control */
	{BBP_R160, 0xEB},
	{BBP_R161, 0xC4},
	{BBP_R162, 0x77},
	{BBP_R163, 0xF9},
	{BBP_R164, 0x88},
	{BBP_R165, 0x80},
	{BBP_R166, 0xFF},
	{BBP_R167, 0xE4},
	/* Added AGC controls */
	/* These AGC/GLRT registers are accessed through R195 and R196. */
	/* 0x00 */
	{BBP_R195, 0x00},
	{BBP_R196, 0x00},
	/* 0x01 */
	{BBP_R195, 0x01},
	{BBP_R196, 0x04},
	/* 0x02 */
	{BBP_R195, 0x02},
	{BBP_R196, 0x20},
	/* 0x03 */
	{BBP_R195, 0x03},
	{BBP_R196, 0x0A},
	/* 0x06 */
	{BBP_R195, 0x06},
	{BBP_R196, 0x16},
	/* 0x07 */
	{BBP_R195, 0x07},
	{BBP_R196, 0x05},
	/* 0x08 */
	{BBP_R195, 0x08},
	{BBP_R196, 0x37},
	/* 0x0A */
	{BBP_R195, 0x0A},
	{BBP_R196, 0x15},
	/* 0x0B */
	{BBP_R195, 0x0B},
	{BBP_R196, 0x17},
	/* 0x0C */
	{BBP_R195, 0x0C},
	{BBP_R196, 0x06},
	/* 0x0D */
	{BBP_R195, 0x0D},
	{BBP_R196, 0x09},
	/* 0x0E */
	{BBP_R195, 0x0E},
	{BBP_R196, 0x05},
	/* 0x0F */
	{BBP_R195, 0x0F},
	{BBP_R196, 0x09},
	/* 0x10 */
	{BBP_R195, 0x10},
	{BBP_R196, 0x20},
	/* 0x20 */
	{BBP_R195, 0x20},
	{BBP_R196, 0x17},
	/* 0x21 */
	{BBP_R195, 0x21},
	{BBP_R196, 0x06},
	/* 0x22 */
	{BBP_R195, 0x22},
	{BBP_R196, 0x09},
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x17},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x06},
	/* 0x25 */
	{BBP_R195, 0x25},
	{BBP_R196, 0x09},
	/* 0x26 */
	{BBP_R195, 0x26},
	{BBP_R196, 0x17},
	/* 0x27 */
	{BBP_R195, 0x27},
	{BBP_R196, 0x06},
	/* 0x28 */
	{BBP_R195, 0x28},
	{BBP_R196, 0x09},
	/* 0x29 */
	{BBP_R195, 0x29},
	{BBP_R196, 0x05},
	/* 0x2A */
	{BBP_R195, 0x2A},
	{BBP_R196, 0x09},
	/* 0x80 */
	{BBP_R195, 0x80},
	{BBP_R196, 0x8B},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x12},
	/* 0x82 */
	{BBP_R195, 0x82},
	{BBP_R196, 0x09},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x17},
	/* 0x84 */
	{BBP_R195, 0x84},
	{BBP_R196, 0x11},
	/* 0x85 */
	{BBP_R195, 0x85},
	{BBP_R196, 0x00},
	/* 0x86 */
	{BBP_R195, 0x86},
	{BBP_R196, 0x00},
	/* 0x87 */
	{BBP_R195, 0x87},
	{BBP_R196, 0x18},
	/* 0x88 */
	{BBP_R195, 0x88},
	{BBP_R196, 0x60},
	/* 0x89 */
	{BBP_R195, 0x89},
	{BBP_R196, 0x44},
	/*     */
	{BBP_R195, 0x8A},
	{BBP_R196, 0x8B},
	{BBP_R195, 0x8B},
	{BBP_R196, 0x8B},
	{BBP_R195, 0x8C},
	{BBP_R196, 0x8B},
	{BBP_R195, 0x8D},
	{BBP_R196, 0x8B},
	/*     */
	{BBP_R195, 0x8E},
	{BBP_R196, 0x09},
	{BBP_R195, 0x8F},
	{BBP_R196, 0x09},
	{BBP_R195, 0x90},
	{BBP_R196, 0x09},
	{BBP_R195, 0x91},
	{BBP_R196, 0x09},
	/*     */
	{BBP_R195, 0x92},
	{BBP_R196, 0x11},
	{BBP_R195, 0x93},
	{BBP_R196, 0x11},
	{BBP_R195, 0x94},
	{BBP_R196, 0x11},
	{BBP_R195, 0x95},
	{BBP_R196, 0x11},
	// PPAD
	{BBP_R47, 0x80},
	{BBP_R60, 0x80},
	{BBP_R150, 0xD2},
	{BBP_R151, 0x32},
	{BBP_R152, 0x23},
	{BBP_R153, 0x41},
	{BBP_R154, 0x00},
	{BBP_R155, 0x4F},
	{BBP_R253, 0x7E},
	{BBP_R195, 0x30},
	{BBP_R196, 0x32},
	{BBP_R195, 0x31},
	{BBP_R196, 0x23},
	{BBP_R195, 0x32},
	{BBP_R196, 0x45},
	{BBP_R195, 0x35},
	{BBP_R196, 0x4A},
	{BBP_R195, 0x36},
	{BBP_R196, 0x5A},
	{BBP_R195, 0x37},
	{BBP_R196, 0x5A},
};
//static 
UCHAR MT7601_BBP_InitRegTb_Size = (sizeof(MT7601_BBP_InitRegTb) / sizeof(RTMP_REG_PAIR));






#if 0 // wmm unused code.
const __align(32) RTMP_REG_PAIR	STAMACRegTable[] =	{
	{WMM_AIFSN_CFG,		0x00002273},
	{WMM_CWMIN_CFG,	0x00002344},
	{WMM_CWMAX_CFG,	0x000034aa},
};

const __align(32) RTMP_REG_PAIR	BcnSpecMACRegTable[] =	{
	/* 	
		That means all beacon's size are 512 bytes 
		and their starting address are "0x4000, 0x4200, 0x4400, 0x4600, ....." 
		in the second(higher) 8KB shared memory . 

		The formula is : 0x4000 + BCNx_OFFSET*64
			ex : the address of BSS0 = 0x4000 + 0x00 * 64 = 0x4000
				 the address of BSS1 = 0x4000 + 0x08 * 64 = 0x4200
	*/
	{BCN_OFFSET0,			0x18100800}, 
	{BCN_OFFSET1,			0x38302820}, 
	{BCN_OFFSET2,			0x58504840}, 
	{BCN_OFFSET3,			0x78706860}, 
};

const __align(32) REG_PAIR   RT3020_RFRegTable[] = {
        {RF_R04,          0x40},
        {RF_R05,          0x03},
        {RF_R06,          0x02},
        {RF_R07,          0x60},      
        {RF_R09,          0x0F},
        {RF_R10,          0x41},
        {RF_R11,          0x21},
        {RF_R12,          0x7B},
        {RF_R14,          0x90},
        {RF_R15,          0x58},
        {RF_R16,          0xB3},
        {RF_R17,          0x92},
        {RF_R18,          0x2C},
        {RF_R19,          0x02},
        {RF_R20,          0xBA},
        {RF_R21,          0xDB},
        {RF_R24,          0x16},      
        {RF_R25,          0x03},
        {RF_R29,          0x1F},
};

UCHAR NUM_RF_3020_REG_PARMS = (sizeof(RT3020_RFRegTable) / sizeof(REG_PAIR));
#endif


// 20121122 RF CR
/* Bank	Register Value(Hex) */
//static 
const __align(32) BANK_RF_REG_PAIR MT7601_RF_Central_RegTb[] = {
/*
	Bank 0 - For central blocks: BG, PLL, XTAL, LO, ADC/DAC
*/
	{RF_BANK0,	RF_R00, 0x02},
	{RF_BANK0,	RF_R01, 0x01},
	{RF_BANK0,	RF_R02, 0x11},
	{RF_BANK0,	RF_R03, 0xFF},
	{RF_BANK0,	RF_R04, 0x0A},
	{RF_BANK0,	RF_R05, 0x20},
	{RF_BANK0,	RF_R06, 0x00},

	/*
		BG
	*/
	{RF_BANK0,	RF_R07, 0x00},
	{RF_BANK0,	RF_R08, 0x00}, 
	{RF_BANK0,	RF_R09, 0x00},
	{RF_BANK0,	RF_R10, 0x00},
	{RF_BANK0,	RF_R11, 0x21},

	/*
		XO
	*/
	//{RF_BANK0,	RF_R12, 0x00},		// By EEPROM
	{RF_BANK0,	RF_R13, 0x00},		// 40MHZ xtal
	//{RF_BANK0,	RF_R13, 0x13},		// 20MHZ xtal
	{RF_BANK0,	RF_R14, 0x7C},
	{RF_BANK0,	RF_R15, 0x22},
	{RF_BANK0,	RF_R16, 0x80},


	/*
		PLL
	*/
	{RF_BANK0,	RF_R17, 0x99},
	{RF_BANK0,	RF_R18, 0x99},
	{RF_BANK0,	RF_R19, 0x09},
	{RF_BANK0,	RF_R20, 0x50},
	{RF_BANK0,	RF_R21, 0xB0},
	{RF_BANK0,	RF_R22, 0x00},
	{RF_BANK0,	RF_R23, 0xC5},
	{RF_BANK0,	RF_R24, 0xFC},
	{RF_BANK0,	RF_R25, 0x40},
	{RF_BANK0,	RF_R26, 0x4D},
	{RF_BANK0,	RF_R27, 0x02},
	{RF_BANK0,	RF_R28, 0x72},
	{RF_BANK0,	RF_R29, 0x01},
	{RF_BANK0,	RF_R30, 0x00},
	{RF_BANK0,	RF_R31, 0x00},

	/*
		Test Ports
	*/
	{RF_BANK0,	RF_R32, 0x00},
	{RF_BANK0,	RF_R33, 0x00},
	{RF_BANK0,	RF_R34, 0x23},
	{RF_BANK0,	RF_R35, 0x01}, /* Change setting to reduce spurs */
	{RF_BANK0,	RF_R36, 0x00},
	{RF_BANK0,	RF_R37, 0x00},	

	/*
		ADC/DAC
	*/
	{RF_BANK0,	RF_R38, 0x00},
	{RF_BANK0,	RF_R39, 0x20},
	{RF_BANK0,	RF_R40, 0x00},
	{RF_BANK0,	RF_R41, 0xD0},
	{RF_BANK0,	RF_R42, 0x1B},
	{RF_BANK0,	RF_R43, 0x02},
	{RF_BANK0,	RF_R44, 0x00},
};
//static 
UINT32 MT7601_RF_Central_RegTb_Size = (sizeof(MT7601_RF_Central_RegTb) / sizeof(BANK_RF_REG_PAIR));

//static 
const __align(32) BANK_RF_REG_PAIR MT7601_RF_Channel_RegTb[] = {
	{RF_BANK4,	RF_R00, 0x01},
	{RF_BANK4,	RF_R01, 0x00},
	{RF_BANK4,	RF_R02, 0x00},
	{RF_BANK4,	RF_R03, 0x00},

	/*
		LDO
	*/
	{RF_BANK4,	RF_R04, 0x00},
	{RF_BANK4,	RF_R05, 0x08},
	{RF_BANK4,	RF_R06, 0x00},

	/*
		RX
	*/
	{RF_BANK4,	RF_R07, 0x5B},
	{RF_BANK4,	RF_R08, 0x52},
	{RF_BANK4,	RF_R09, 0xB6},
	{RF_BANK4,	RF_R10, 0x57},
	{RF_BANK4,	RF_R11, 0x33},
	{RF_BANK4,	RF_R12, 0x22},
	{RF_BANK4,	RF_R13, 0x3D},
	{RF_BANK4,	RF_R14, 0x3E},
	{RF_BANK4,	RF_R15, 0x13},
	{RF_BANK4,	RF_R16, 0x22},
	{RF_BANK4,	RF_R17, 0x23},
	{RF_BANK4,	RF_R18, 0x02},
	{RF_BANK4,	RF_R19, 0xA4},
	{RF_BANK4,	RF_R20, 0x01},
	{RF_BANK4,	RF_R21, 0x12},
	{RF_BANK4,	RF_R22, 0x80},
	{RF_BANK4,	RF_R23, 0xB3},
	{RF_BANK4,	RF_R24, 0x00}, /* Reserved */
	{RF_BANK4,	RF_R25, 0x00}, /* Reserved */
	{RF_BANK4,	RF_R26, 0x00}, /* Reserved */
	{RF_BANK4,	RF_R27, 0x00}, /* Reserved */

	/*
		LOGEN
	*/
	{RF_BANK4,	RF_R28, 0x18},
	{RF_BANK4,	RF_R29, 0xEE},
	{RF_BANK4,	RF_R30, 0x6B},
	{RF_BANK4,	RF_R31, 0x31},
	{RF_BANK4,	RF_R32, 0x5D},
	{RF_BANK4,	RF_R33, 0x00}, /* Reserved */

	/*
		TX
	*/
	{RF_BANK4,	RF_R34, 0x96},
	{RF_BANK4,	RF_R35, 0x55},
	{RF_BANK4,	RF_R36, 0x08},
	{RF_BANK4,	RF_R37, 0xBB},
	{RF_BANK4,	RF_R38, 0xB3},
	{RF_BANK4,	RF_R39, 0xB3},
	{RF_BANK4,	RF_R40, 0x03},
	{RF_BANK4,	RF_R41, 0x00}, /* Reserved */
	{RF_BANK4,	RF_R42, 0x00}, /* Reserved */
	{RF_BANK4,	RF_R43, 0xC5},
	{RF_BANK4,	RF_R44, 0xC5},
	{RF_BANK4,	RF_R45, 0xC5},
	{RF_BANK4,	RF_R46, 0x07},
	{RF_BANK4,	RF_R47, 0xA8},
	{RF_BANK4,	RF_R48, 0xEF},
	{RF_BANK4,	RF_R49, 0x1A},

	/*
		PA
	*/
	{RF_BANK4,	RF_R54, 0x07},
	{RF_BANK4,	RF_R55, 0xA7},
	{RF_BANK4,	RF_R56, 0xCC},
	{RF_BANK4,	RF_R57, 0x14},
	{RF_BANK4,	RF_R58, 0x07},
	{RF_BANK4,	RF_R59, 0xA8},
	{RF_BANK4,	RF_R60, 0xD7},
	{RF_BANK4,	RF_R61, 0x10},
	{RF_BANK4,	RF_R62, 0x1C},
	{RF_BANK4,	RF_R63, 0x00}, /* Reserved */
};
//static 
UINT32 MT7601_RF_Channel_RegTb_Size = (sizeof(MT7601_RF_Channel_RegTb) / sizeof(BANK_RF_REG_PAIR));

//static 
const __align(32) BANK_RF_REG_PAIR MT7601_RF_VGA_RegTb[] = {
	{RF_BANK5,	RF_R00, 0x47},
	{RF_BANK5,	RF_R01, 0x00},
	{RF_BANK5,	RF_R02, 0x00},
	{RF_BANK5,	RF_R03, 0x08},
	{RF_BANK5,	RF_R04, 0x04},
	{RF_BANK5,	RF_R05, 0x20},
	{RF_BANK5,	RF_R06, 0x3A},
	{RF_BANK5,	RF_R07, 0x3A},
	{RF_BANK5,	RF_R08, 0x00},
	{RF_BANK5,	RF_R09, 0x00},
	{RF_BANK5,	RF_R10, 0x10},
	{RF_BANK5,	RF_R11, 0x10},
	{RF_BANK5,	RF_R12, 0x10},
	{RF_BANK5,	RF_R13, 0x10},
	{RF_BANK5,	RF_R14, 0x10},
	{RF_BANK5,	RF_R15, 0x20},
	{RF_BANK5,	RF_R16, 0x22},
	{RF_BANK5,	RF_R17, 0x7C},
	{RF_BANK5,	RF_R18, 0x00},
	{RF_BANK5,	RF_R19, 0x00},
	{RF_BANK5,	RF_R20, 0x00},
	{RF_BANK5,	RF_R21, 0xF1},
	{RF_BANK5,	RF_R22, 0x11},
	{RF_BANK5,	RF_R23, 0x02},
	{RF_BANK5,	RF_R24, 0x41},
	{RF_BANK5,	RF_R25, 0x20},
	{RF_BANK5,	RF_R26, 0x00},
	{RF_BANK5,	RF_R27, 0xD7},
	{RF_BANK5,	RF_R28, 0xA2},
	{RF_BANK5,	RF_R29, 0x20},
	{RF_BANK5,	RF_R30, 0x49},
	{RF_BANK5,	RF_R31, 0x20},
	{RF_BANK5,	RF_R32, 0x04},
	{RF_BANK5,	RF_R33, 0xF1},
	{RF_BANK5,	RF_R34, 0xA1},
	{RF_BANK5,	RF_R35, 0x01},
	{RF_BANK5,	RF_R41, 0x00},
	{RF_BANK5,	RF_R42, 0x00},
	{RF_BANK5,	RF_R43, 0x00},
	{RF_BANK5,	RF_R44, 0x00},
	{RF_BANK5,	RF_R45, 0x00},
	{RF_BANK5,	RF_R46, 0x00},
	{RF_BANK5,	RF_R47, 0x00},
	{RF_BANK5,	RF_R48, 0x00},
	{RF_BANK5,	RF_R49, 0x00},
	{RF_BANK5,	RF_R50, 0x00},
	{RF_BANK5,	RF_R51, 0x00},
	{RF_BANK5,	RF_R52, 0x00},
	{RF_BANK5,	RF_R53, 0x00},
	{RF_BANK5,	RF_R54, 0x00},
	{RF_BANK5,	RF_R55, 0x00},
	{RF_BANK5,	RF_R56, 0x00},
	{RF_BANK5,	RF_R57, 0x00},
	{RF_BANK5,	RF_R58, 0x31},
	{RF_BANK5,	RF_R59, 0x31},
	{RF_BANK5,	RF_R60, 0x0A},
	{RF_BANK5,	RF_R61, 0x02},
	{RF_BANK5,	RF_R62, 0x00},
	{RF_BANK5,	RF_R63, 0x00},
};
//static 
UINT32 MT7601_RF_VGA_RegTb_Size = (sizeof(MT7601_RF_VGA_RegTb) / sizeof(BANK_RF_REG_PAIR));

const __align(32) MT7601_FREQ_ITEM MT7601_Frequency_Plan[] =
{
	/* CH,	K_R17,	K_R18,	K_R19,	N_R20 */
	{ 1,		0x99,	0x99,	0x09,	0x50},
	{ 2,		0x46,	0x44,	0x0A,	0x50},
	{ 3,		0xEC,	0xEE,	0x0A,	0x50},
	{ 4,		0x99,	0x99,	0x0B,	0x50},
	{ 5,		0x46,	0x44,	0x08,	0x51},
	{ 6,		0xEC,	0xEE,	0x08,	0x51},
	{ 7,		0x99,	0x99,	0x09,	0x51},
	{ 8,		0x46,	0x44,	0x0A,	0x51},
	{ 9,		0xEC,	0xEE,	0x0A,	0x51},
	{ 10,	0x99,	0x99,	0x0B,	0x51},
	{ 11,	0x46,	0x44,	0x08,	0x52},
	{ 12,	0xEC,	0xEE,	0x08,	0x52},
	{ 13,	0x99,	0x99,	0x09,	0x52},
	{ 14,	0x33,	0x33,	0x0B,	0x52},
};
UINT32 NUM_OF_MT7601_CHNL = (sizeof(MT7601_Frequency_Plan) / sizeof(MT7601_FREQ_ITEM));



const __align(32) RTMP_REG_PAIR MT7601_BBP_BW20RegTb[] = {
	{BBP_R69, 0x12},
	{BBP_R91, 0x07},
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x17},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x06},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x12},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x17},
};
//const 
UCHAR MT7601_BBP_BW20RegTb_Size = (sizeof(MT7601_BBP_BW20RegTb) / sizeof(RTMP_REG_PAIR));

const __align(32) RTMP_REG_PAIR MT7601_BBP_BW40RegTb[] = {
	{BBP_R69, 0x15},
	{BBP_R91, 0x04},
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x12},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x08},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x15},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x16},
};
//const 
UCHAR MT7601_BBP_BW40RegTb_Size = (sizeof(MT7601_BBP_BW40RegTb) / sizeof(RTMP_REG_PAIR));

//static 
const __align(32) RTMP_REG_PAIR MT7601_BBP_CommonRegTb[] = {
	{BBP_R75, 0x60},
	{BBP_R92, 0x02},
	{BBP_R178, 0xFF},		// For CCK CH14 OBW
	/* 0x88 */
	{BBP_R195, 0x88},
	{BBP_R196, 0x60},

};
//const static 
UCHAR MT7601_BBP_CommonRegTb_Size = (sizeof(MT7601_BBP_CommonRegTb) / sizeof(RTMP_REG_PAIR));


const __align(32) RTMP_REG_PAIR MT7601_BBP_HighTempBW20RegTb[] = {
	{BBP_R69, 0x12},
	{BBP_R91, 0x07},
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x17},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x06},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x12},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x17},
};
//const 
UCHAR MT7601_BBP_HighTempBW20RegTb_Size = (sizeof(MT7601_BBP_HighTempBW20RegTb) / sizeof(RTMP_REG_PAIR));

const __align(32) RTMP_REG_PAIR MT7601_BBP_HighTempBW40RegTb[] = {
	{BBP_R69, 0x15},
	{BBP_R91, 0x04},
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x12},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x08},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x15},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x16},
};
//const 
UCHAR MT7601_BBP_HighTempBW40RegTb_Size = (sizeof(MT7601_BBP_HighTempBW40RegTb) / sizeof(RTMP_REG_PAIR));

//static 
const __align(32) RTMP_REG_PAIR MT7601_BBP_HighTempCommonRegTb[] = {
	{BBP_R75, 0x60},
	{BBP_R92, 0x02},
	{BBP_R178, 0xFF},		// For CCK CH14 OBW
	/* 0x88 */
	{BBP_R195, 0x88},
	{BBP_R196, 0x60},
};
//const static 
UCHAR MT7601_BBP_HighTempCommonRegTb_Size = (sizeof(MT7601_BBP_HighTempCommonRegTb) / sizeof(RTMP_REG_PAIR));

const __align(32) RTMP_REG_PAIR MT7601_BBP_LowTempBW20RegTb[] = {
	{BBP_R69, 0x12},
	{BBP_R75, 0x5E},
	{BBP_R91, 0x07},
	{BBP_R92, 0x02},	
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x17},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x06},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x12},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x17},
	/* 0x88 */
	{BBP_R195, 0x88},
	{BBP_R196, 0x5E},
};
//const 
UCHAR MT7601_BBP_LowTempBW20RegTb_Size = (sizeof(MT7601_BBP_LowTempBW20RegTb) / sizeof(RTMP_REG_PAIR));

const __align(32) RTMP_REG_PAIR MT7601_BBP_LowTempBW40RegTb[] = {
	{BBP_R69, 0x15},
	{BBP_R75, 0x5C},
	{BBP_R91, 0x04},
	{BBP_R92, 0x03},	
	/* 0x23 */
	{BBP_R195, 0x23},
	{BBP_R196, 0x10},
	/* 0x24 */
	{BBP_R195, 0x24},
	{BBP_R196, 0x08},
	/* 0x81 */
	{BBP_R195, 0x81},
	{BBP_R196, 0x15},
	/* 0x83 */
	{BBP_R195, 0x83},
	{BBP_R196, 0x16},
	/* 0x88 */
	{BBP_R195, 0x88},
	{BBP_R196, 0x5B},
};
UCHAR MT7601_BBP_LowTempBW40RegTb_Size = (sizeof(MT7601_BBP_LowTempBW40RegTb) / sizeof(RTMP_REG_PAIR));

//static 
static __align(32) RTMP_REG_PAIR MT7601_BBP_LowTempCommonRegTb[] = {
	{BBP_R178, 0xFF},		// For CCK CH14 OBW
};
//const static 
UCHAR MT7601_BBP_LowTempCommonRegTb_Size = (sizeof(MT7601_BBP_LowTempCommonRegTb) / sizeof(RTMP_REG_PAIR));





#endif
