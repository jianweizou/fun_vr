#ifndef __RTMP_H__
#define __RTMP_H__

#include "common.h"
#include "nl80211_hdr.h"
#include "system.h"
#include "rtmp_mac.h"
#include "USBH.h"
#include "rtmp_timer.h"
#include "mgmtentry.h"
#include "wpa/cmm_wpa.h"

#include "wifi_api.h"



typedef union _LARGE_INTEGER {
	struct {
#ifdef BIG_ENDIAN
		INT32 	HighPart;
		UINT 	LowPart;
#else
		UINT 	LowPart;
		INT32 	HighPart;
#endif
	} u;
	UINT64 	QuadPart;
} LARGE_INTEGER;


typedef struct {
#ifdef BIG_ENDIAN
	USHORT	LSIGTxopProSup:1;
	USHORT	Forty_Mhz_Intolerant:1;
	USHORT	PSMP:1;
	USHORT	CCKmodein40:1;
	USHORT	AMsduSize:1;
	USHORT	DelayedBA:1;			/* rt2860c not support */
	USHORT	RxSTBC:2;
	USHORT	TxSTBC:1;
	USHORT	ShortGIfor40:1;			/* for40MHz */
	USHORT	ShortGIfor20:1;
	USHORT	GF:1;					/* green field */
	USHORT	MimoPs:2;				/* momi power safe */
	USHORT	ChannelWidth:1;
	USHORT	AdvCoding:1;
#else
	USHORT	AdvCoding:1;
	USHORT	ChannelWidth:1;
	USHORT	MimoPs:2;				/* momi power safe */
	USHORT	GF:1;					/* green field */
	USHORT	ShortGIfor20:1;
	USHORT	ShortGIfor40:1;			/* for40MHz */
	USHORT	TxSTBC:1;
	USHORT	RxSTBC:2;
	USHORT	DelayedBA:1;			/* rt2860c not support */
	USHORT	AMsduSize:1;			/* only support as zero */
	USHORT	CCKmodein40:1;
	USHORT	PSMP:1;
	USHORT	Forty_Mhz_Intolerant:1;
	USHORT	LSIGTxopProSup:1;
#endif	/* !BIG_ENDIAN */
} HT_CAP_INFO, *PHT_CAP_INFO;

//;  HT Capability INFO field in HT Cap IE.
typedef struct {
#ifdef BIG_ENDIAN
	UCHAR 	rsv:3;					/* momi power safe */
	UCHAR	MpduDensity:3;
	UCHAR	MaxRAmpduFactor:2;
#else
	UCHAR	MaxRAmpduFactor:2;
	UCHAR	MpduDensity:3;
	UCHAR	rsv:3;					/* momi power safe */
#endif /* !BIG_ENDIAN */
} HT_CAP_PARM, *PHT_CAP_PARM;

//;  HT Capability INFO field in HT Cap IE.
typedef struct  {
	UCHAR	MCSSet[10];
	UCHAR	SupRate[2];  			/* unit : 1Mbps */
#ifdef BIG_ENDIAN
	UCHAR	rsv:3;
	UCHAR	MpduDensity:1;
	UCHAR	TxStream:2;
	UCHAR	TxRxNotEqual:1;
	UCHAR	TxMCSSetDefined:1; 
#else
	UCHAR	TxMCSSetDefined:1; 
	UCHAR	TxRxNotEqual:1;
	UCHAR	TxStream:2;
	UCHAR	MpduDensity:1;
	UCHAR	rsv:3;
#endif /* BIG_ENDIAN */
	UCHAR	rsv3[3];  
} HT_MCS_SET, *PHT_MCS_SET;

//;  HT Capability INFO field in HT Cap IE.
typedef struct  {
#ifdef BIG_ENDIAN 
	USHORT	rsv2:4;
	USHORT	RDGSupport:1;		/* reverse Direction Grant  support */
	USHORT	PlusHTC:1;			/* +HTC control field support */
	USHORT	MCSFeedback:2;		/* 0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv. */
	USHORT	rsv:5;				/* momi power safe */
	USHORT	TranTime:2;
	USHORT	Pco:1;
#else
	USHORT	Pco:1;
	USHORT	TranTime:2;
	USHORT	rsv:5;				/* momi power safe */
	USHORT	MCSFeedback:2;		/* 0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv. */
	USHORT	PlusHTC:1;			/* +HTC control field support */
	USHORT	RDGSupport:1;		/* reverse Direction Grant  support */
	USHORT	rsv2:4;
#endif /* BIG_ENDIAN */
} EXT_HT_CAP_INFO, *PEXT_HT_CAP_INFO;

//;  HT Beamforming field in HT Cap IE.
typedef struct  _HT_BF_CAP{
#ifdef BIG_ENDIAN
	ULONG	rsv:3;
	ULONG	ChanEstimation:2;
	ULONG	CSIRowBFSup:2;
	ULONG	ComSteerBFAntSup:2;
	ULONG	NoComSteerBFAntSup:2;
	ULONG	CSIBFAntSup:2;
	ULONG	MinGrouping:2;
	ULONG	ExpComBF:2;	
	ULONG	ExpNoComBF:2;
	ULONG	ExpCSIFbk:2;
	ULONG	ExpComSteerCapable:1;
	ULONG	ExpNoComSteerCapable:1;
	ULONG	ExpCSICapable:1;
	ULONG	Calibration:2;
	ULONG	ImpTxBFCapable:1;
	ULONG	TxNDPCapable:1;	
	ULONG	RxNDPCapable:1;	
	ULONG	TxSoundCapable:1;
	ULONG	RxSoundCapable:1;
	ULONG	TxBFRecCapable:1;
#else
	ULONG	TxBFRecCapable:1;
	ULONG	RxSoundCapable:1;
	ULONG	TxSoundCapable:1;
	ULONG	RxNDPCapable:1;	
	ULONG	TxNDPCapable:1;	
	ULONG	ImpTxBFCapable:1;	
	ULONG	Calibration:2;
	ULONG	ExpCSICapable:1;
	ULONG	ExpNoComSteerCapable:1;
	ULONG	ExpComSteerCapable:1;
	ULONG	ExpCSIFbk:2;	
	ULONG	ExpNoComBF:2;	
	ULONG	ExpComBF:2;	
	ULONG	MinGrouping:2;
	ULONG	CSIBFAntSup:2;
	ULONG	NoComSteerBFAntSup:2;
	ULONG	ComSteerBFAntSup:2;
	ULONG	CSIRowBFSup:2;
	ULONG	ChanEstimation:2;
	ULONG	rsv:3;
#endif /* BIG_ENDIAN */
} HT_BF_CAP, *PHT_BF_CAP;

/*  HT antenna selection field in HT Cap IE . */
typedef struct  _HT_AS_CAP{
	UCHAR	AntSelect:1; 
	UCHAR	ExpCSIFbkTxASEL:1;
	UCHAR	AntIndFbkTxASEL:1;
	UCHAR	ExpCSIFbk:1;
	UCHAR	AntIndFbk:1;
	UCHAR	RxASel:1;
	UCHAR	TxSoundPPDU:1;
	UCHAR	rsv:1;
} HT_AS_CAP, *PHT_AS_CAP;

/* The structure for HT Capability IE. */
typedef struct  _HT_CAPABILITY_IE{
	HT_CAP_INFO		HtCapInfo;
	HT_CAP_PARM		HtCapParm;
/*	HT_MCS_SET		HtMCSSet; */
	UCHAR			MCSSet[16];
	EXT_HT_CAP_INFO	ExtHtCapInfo;
	HT_BF_CAP		TxBFCap;	/* beamforming cap. rt2860c not support beamforming. */
	HT_AS_CAP		ASCap;		/*antenna selection. */
} HT_CAPABILITY_IE, *PHT_CAPABILITY_IE;

typedef union _REG_TRANSMIT_SETTING {
	struct {
		/*UINT32  PhyMode:4; */
		/*UINT32  MCS:7;                 // MCS */
		UINT32 rsv0:9;
		UINT32 ITxBfEn:1;
		UINT32 TxBF:1;
		UINT32 BW:1;			/*channel bandwidth 20MHz or 40 MHz */
		UINT32 ShortGI:1;
		UINT32 STBC:1;			/*SPACE */
		UINT32 TRANSNO:2;
		UINT32 HTMODE:1;
		UINT32 EXTCHA:2;
		UINT32 rsv:13;
	} field;
	UINT32 word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;

typedef struct  {
	UCHAR	ExtChanOffset:2;
	UCHAR	RecomWidth:1;
	UCHAR	RifsMode:1; 
	UCHAR	S_PSMPSup:1;	 	/*Indicate support for scheduled PSMP */
	UCHAR	SerInterGranu:3;	/*service interval granularity */
} ADD_HTINFO, *PADD_HTINFO;

typedef struct {
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv:1; 
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv2:11; 
} ADD_HTINFO2, *PADD_HTINFO2;


typedef struct {
	UCHAR 	bHtEnable;	 /* If we should use ht rate. */
	UCHAR 	bPreNHt;	 /* If we should use ht rate. */
	/*Substract from HT Capability IE */
	UCHAR 	MCSSet[16];
} RT_HT_PHY_INFO, *PRT_HT_PHY_INFO;

/* TODO: Need sync with spec about the definition of StbcMcs. In Draft 3.03, it's reserved. */
typedef struct {
	USHORT	StbcMcs:6;
	USHORT	DualBeacon:1;
	USHORT	DualCTSProtect:1; 
	USHORT	STBCBeacon:1;
	USHORT	LsigTxopProt:1;		/* L-SIG TXOP protection full support */
	USHORT	PcoActive:1; 
	USHORT	PcoPhase:1; 
	USHORT	rsv:4; 
} ADD_HTINFO3, *PADD_HTINFO3;

typedef struct {
	UCHAR 	ControlChan;
	UCHAR 	MCSSet[16];		/* Basic MCS set */
	ADD_HTINFO 		AddHtInfo;
	ADD_HTINFO2 	AddHtInfo2;	 
	ADD_HTINFO3 	AddHtInfo3;	 
} ADD_HT_INFO_IE, *PADD_HT_INFO_IE;


typedef struct {
    UCHAR       IELen;
    UCHAR       IE[MAX_CUSTOM_LEN];
} WPA_IE_;

typedef struct {
    UCHAR   Bssid[MAC_ADDR_LEN];
    UCHAR   Channel;
	UCHAR   CentralChannel;				/*Store the wide-band central channel for 40MHz.  .used in 40MHz AP. Or this is the same as Channel. */
    UCHAR   BssType;
    USHORT  AtimWin;
    USHORT  BeaconPeriod;
    UCHAR   SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   SupRateLen;
    UCHAR   ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   ExtRateLen;
//	HT_CAPABILITY_IE HtCapability;
	UCHAR 	HtCapabilityLen;
//	ADD_HT_INFO_IE AddHtInfo;			/* AP might use this additional ht info IE */
	UCHAR 	AddHtInfoLen;
//	EXT_CAP_INFO_ELEMENT ExtCapInfo;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */
	UCHAR 	NewExtChanOffset;
	CHAR    Rssi;
	CHAR	MinSNR;	
    UCHAR   Privacy;					/* Indicate security function ON/OFF. Don't mess up with auth mode. */
	UCHAR	Hidden;
    USHORT  DtimPeriod;
    USHORT  CapabilityInfo;
    USHORT  CfpCount;
    USHORT  CfpPeriod;
    USHORT  CfpMaxDuration;
    USHORT  CfpDurRemaining;
    UCHAR   SsidLen;
    CHAR    Ssid[MAX_LEN_OF_SSID];
	UCHAR	SameRxTimeCount;
	ULONG   LastBeaconRxTimeA; 			/* OS's timestamp */
    ULONG   LastBeaconRxTime; 			/* OS's timestamp */
	BOOLEAN	bSES;
	UCHAR	VarIEs[MAX_VIE_LEN];
	USHORT	VarIeFromProbeRspLen;
	PUCHAR	pVarIeFromProbRsp;
	/* CCX Ckip information */
    WPA_IE_ 	WpaIE;
    WPA_IE_     RsnIE;
	WPA_IE_ 	WpsIE;
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR 	CountryString[3];
	BOOLEAN		bHasCountryIE;
#endif /* EXT_BUILD_CHANNEL_LIST */
} BSS_ENTRY, *PBSS_ENTRY;

typedef struct {
    UCHAR 	BssNr;
    UCHAR   BssOverlapNr;
    BSS_ENTRY 	BssEntry[MAX_LEN_OF_BSS_TABLE];
} BSS_TABLE, *PBSS_TABLE;



typedef struct  {
	UCHAR 	NewExtChanOffset;
} NEW_EXT_CHAN_IE, *PNEW_EXT_CHAN_IE;

typedef struct {
#ifdef BIG_ENDIAN
	USHORT	rsv:5;
	USHORT	AmsduSize:1;		/* Max receiving A-MSDU size */
	USHORT	AmsduEnable:1;		/* Enable to transmit A-MSDU. Suggest disable. We should use A-MPDU to gain best benifit of 802.11n */
	USHORT	RxSTBC:2;			/* 2 bits */
	USHORT	TxSTBC:1;
	USHORT	ShortGIfor40:1;		/* for40MHz */
	USHORT	ShortGIfor20:1;
	USHORT	GF:1;				/* green field */
	USHORT	MimoPs:2;			/* mimo power safe MMPS_ */
	USHORT	ChannelWidth:1;
#else
	USHORT	ChannelWidth:1;
	USHORT	MimoPs:2;			/* mimo power safe MMPS_ */
	USHORT	GF:1;				/* green field */
	USHORT	ShortGIfor20:1;
	USHORT	ShortGIfor40:1;		/* for40MHz */
	USHORT	TxSTBC:1;
	USHORT	RxSTBC:2;			/* 2 bits */
	USHORT	AmsduEnable:1;		/* Enable to transmit A-MSDU. Suggest disable. We should use A-MPDU to gain best benifit of 802.11n */
	USHORT	AmsduSize:1;		/* Max receiving A-MSDU size */
	USHORT	rsv:5;
#endif
	/*Substract from Addiont HT INFO IE */
#ifdef BIG_ENDIAN
	UCHAR	RecomWidth:1;
	UCHAR	ExtChanOffset:2;	/* Please not the difference with following 	UCHAR	NewExtChannelOffset; from 802.11n */
	UCHAR	MpduDensity:3;	
	UCHAR	MaxRAmpduFactor:2;
#else
	UCHAR	MaxRAmpduFactor:2;
	UCHAR	MpduDensity:3;
	UCHAR	ExtChanOffset:2;	/* Please not the difference with following 	UCHAR	NewExtChannelOffset; from 802.11n */
	UCHAR	RecomWidth:1;
#endif

#ifdef BIG_ENDIAN
	USHORT	rsv2:11;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv3:1;
	USHORT	NonGfPresent:1;
	USHORT	OperaionMode:2;
#else
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv3:1;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv2:11;
#endif
	
	/* New Extension Channel Offset IE */
	UCHAR	NewExtChannelOffset;	
	/* Extension Capability IE = 127 */
	UCHAR	BSSCoexist2040;	
} RT_HT_CAPABILITY, *PRT_HT_CAPABILITY;


typedef union _BACAP_STRUC {
#ifdef BIG_ENDIAN
	struct {
		UINT32:4;
		UINT32 	b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32 	bHtAdhoc:1;				/* adhoc can use ht rate. */
		UINT32 	MMPSmode:2;				/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 	AmsduSize:1;			/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		UINT32 	AmsduEnable:1;			/*Enable AMSDU transmisstion */
		UINT32 	MpduDensity:3;
		UINT32 	Policy:2;				/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 	AutoBA:1;				/* automatically BA */
		UINT32 	TxBAWinLimit:8;
		UINT32 	RxBAWinLimit:8;
	} field;
#else
	struct {
		UINT32 	RxBAWinLimit:8;
		UINT32 	TxBAWinLimit:8;
		UINT32 	AutoBA:1;				/* automatically BA */
		UINT32 	Policy:2;				/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 	MpduDensity:3;
		UINT32 	AmsduEnable:1;			/*Enable AMSDU transmisstion */
		UINT32 	AmsduSize:1;			/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		UINT32 	MMPSmode:2;				/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 	bHtAdhoc:1;				/* adhoc can use ht rate. */
		UINT32 	b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32:4;
	} field;
#endif
	UINT32 word;
} BACAP_STRUC, *PBACAP_STRUC;


/* EDCA configuration from AP's BEACON/ProbeRsp */
typedef struct {
    UCHAR 	bValid;         /* 1: variable contains valid value */
    UCHAR 	bAdd;         	/* 1: variable contains valid value */
    UCHAR 	bQAck;
    UCHAR  	bQueueRequest;
    UCHAR 	bTxopRequest;
    UCHAR  	bAPSDCapable;
/*  BOOLEAN 	bMoreDataAck; */
    UCHAR 	EdcaUpdateCount;
    UCHAR 	Aifsn[4];       /* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
    UCHAR  	Cwmin[4];
    UCHAR  	Cwmax[4];
    USHORT 	Txop[4];      	/* in unit of 32-us */
    UCHAR  	bACM[4];      	/* 1: Admission Control of AC_BK is mandattory */
} EDCA_PARM, *PEDCA_PARM;


typedef union _HTTRANSMIT_SETTING {
#ifdef BIG_ENDIAN
	struct {
		USHORT MODE:2;	/* Use definition MODE_xxx. */
		USHORT iTxBF:1;
		USHORT rsv:1;
		USHORT eTxBF:1;
		USHORT STBC:2;	/*SPACE */
		USHORT ShortGI:1;
		USHORT BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;	/* MCS */
		USHORT BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		USHORT ShortGI:1;
		USHORT STBC:2;	/*SPACE */
		USHORT eTxBF:1;
		USHORT rsv:1;
		USHORT iTxBF:1;
		USHORT MODE:2;	/* Use definition MODE_xxx. */
	} field;
#endif
	USHORT word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;


typedef struct _MLME_QUEUE_ELEM {
	UCHAR 	Msg[MGMT_DMA_BUFFER_SIZE];	/* move here to fix alignment issue for ARM CPU */
    ULONG 	Machine;
    ULONG   MsgType;
    ULONG   MsgLen;
    LARGE_INTEGER 	TimeStamp;
    UCHAR  	Rssi0;
    UCHAR 	Rssi1;
    UCHAR  	Rssi2;
    UCHAR 	Signal;
    UCHAR 	Channel;
    UCHAR 	Wcid;
    BOOLEAN 	Occupied;
	UCHAR 	OpMode;
	ULONG 	Priv;
} MLME_QUEUE_ELEM, *PMLME_QUEUE_ELEM;


typedef struct _MLME_QUEUE {
    ULONG 	Num;
    ULONG 	Head;
    ULONG 	Tail;
	BOOLEAN 	lock;
    MLME_QUEUE_ELEM 	Entry[MAX_LEN_OF_MLME_QUEUE];
} MLME_QUEUE, *PMLME_QUEUE;

typedef struct _MLME_STRUCT {
	UCHAR 	RealRxPath;
	UCHAR 	CaliBW40RfR24;
	UCHAR 	CaliBW20RfR24;
	UCHAR 	CaliBW20RfR31;
	UCHAR 	CaliBW40RfR31;
	UCHAR	bEnableAutoAntennaCheck;
	BOOLEAN 	bRunning;
	BOOLEAN 	TaskLock;
	MLME_QUEUE 	Queue;
} MLME_STRUCT, *PMLME_STRUCT;



#ifdef BIG_ENDIAN
typedef union _BBP_R105_STRUC {
	struct
	{
		UCHAR	Reserve1:4; 							/* Reserved field */
		UCHAR	EnableSIGRemodulation:1; 				/* Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols. */
		UCHAR	MLDFor2Stream:1; 						/* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX) */
		UCHAR	IndependentFeedForwardCompensation:1; 	/* Apply independent feed-forward compensation for independent stream. */
		UCHAR	DetectSIGOnPrimaryChannelOnly:1; 		/* Under 40 MHz band, detect SIG on primary channel only. */
	} field;
	UCHAR		byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#else
typedef union _BBP_R105_STRUC {
	struct
	{
		UCHAR	DetectSIGOnPrimaryChannelOnly:1; 		/* Under 40 MHz band, detect SIG on primary channel only. */
		UCHAR	IndependentFeedForwardCompensation:1; 	/* Apply independent feed-forward compensation for independent stream. */
		UCHAR	MLDFor2Stream:1; 						/* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX) */
		UCHAR	EnableSIGRemodulation:1; 				/* Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols. */
		UCHAR 	Reserve1:4; 							/* Reserved field */
	} field;
	UCHAR		byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#endif


#define NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET	14

enum IEEE80211_BAND {
  IEEE80211_BAND_2G,
  IEEE80211_BAND_5G,
  IEEE80211_BAND_NUMS
};

/* */
/* The Tx power control using the internal ALC */
/* */
typedef struct _TX_POWER_CONTROL {
	UCHAR 	bInternalTxALC;			/* Internal Tx ALC */
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
	UCHAR 	bExtendedTssiMode; 		/* The extended TSSI mode (each channel has different Tx power if needed) */
	CHAR 	PerChTxPwrOffset[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1]; /* Per-channel Tx power offset */
	/* RT5392 temperature compensation */
	/* lookup table, 33 elements (-7, -6....0, 1, 2...25) */
	INT 	LookupTable[IEEE80211_BAND_NUMS][33];
	/* Reference temperature, from EEPROM */
	INT 	RefTempG;
	/* Index offset, -7....25. */
	INT 	LookupTableIndex;
	CHAR 	idxTxPowerTable2; 		/* The index of the Tx power table */
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */
	CHAR 	idxTxPowerTable;		/* The index of the Tx power table */
	CHAR 	RF_R12_Value;			/* RF R12[4:0]: Tx0 ALC   3390: RF R12[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC*/
	CHAR 	MAC_PowerDelta;			/* Tx power control over MAC 0x1314~0x1324 */
	INT 	RefTemp[IEEE80211_BAND_NUMS];
	UCHAR 	TssiGain[IEEE80211_BAND_NUMS];
} TX_POWER_CONTROL, *PTX_POWER_CONTROL;



typedef struct _MT7601_TX_ALC_DATA {
	INT32	PowerDiffPre;
	INT32	MT7601_TSSI_T0_Delta_Offset;
	INT16	TSSI_DBOFFSET_HVGA;
	INT16	TSSI0_DB;
	UCHAR	TssiSlope;
	CHAR	TssiDC0;
	CHAR	TssiDC0_HVGA;
	UINT32	InitTxAlcCfg1;
	BOOLEAN	TSSI_USE_HVGA;
	BOOLEAN TssiTriggered;
	CHAR	MT7601_TSSI_OFFSET[3];
} MT7601_TX_ALC_DATA, *PMT7601_TX_ALC_DATA;


typedef struct _RTMP_CHIP_CAP_ {
	/* register */
	REG_PAIR 	*pRFRegTable;
	REG_PAIR 	*pBBPRegTable;
	UCHAR 	bbpRegTbSize;
	UINT32 	MaxNumOfRfId;
	UINT32 	MaxNumOfBbpId;
#define RF_REG_WT_METHOD_NONE			0
#define RF_REG_WT_METHOD_STEP_ON		1
	UCHAR 	RfReg17WtMethod;
	/* beacon */
	UCHAR 	BcnMaxNum;					/* software use */
	UCHAR 	BcnMaxHwNum;				/* hardware limitation */
	UCHAR 	WcidHwRsvNum;				/* hardware available WCID number */
	USHORT 	BcnMaxHwSize;				/* hardware maximum beacon size */
	USHORT 	BcnBase[HW_BEACON_MAX_NUM];	/* hardware beacon base address */
	/* function */
	/* use UINT8, not bit-or to speed up driver */
	UCHAR FlgIsHwWapiSup;
	/* signal */
#define SNR_FORMULA1		0			/* ((0xeb     - pAd->StaCfg.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1			/* (pAd->StaCfg.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2			/* (pAd->StaCfg.LastSNR0) * 3) / 16; */
	UCHAR 	SnrFormula;
#ifdef RTMP_INTERNAL_TX_ALC
	UCHAR 	TxAlcTxPowerUpperBound;
	UCHAR 	TxAlcMaxMCS;
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_EFUSE_SUPPORT
	USHORT 	EFUSE_USAGE_MAP_START;
	USHORT 	EFUSE_USAGE_MAP_END;
	UCHAR 	EFUSE_USAGE_MAP_SIZE;
#endif /* RTMP_EFUSE_SUPPORT */
	UCHAR 	FlgIsVcoReCalSup;
	UCHAR 	FlgIsHwAntennaDiversitySup;
#ifdef TXRX_SW_ANTDIV_SUPPORT
	UCHAR 	bTxRxSwAntDiv;
#endif /* TXRX_SW_ANTDIV_SUPPORT */
	PUCHAR	FWImageName;
	UCHAR	IsComboChip;
	UCHAR	FlgIsSupSpecBcnBuf;
#ifdef MT7601
	CHAR	TemperatureRef25C;
	UCHAR	TemperatureMode;
	BOOLEAN	bPllLockProtect;
	UCHAR	CurrentTemperBbpR49;
#ifdef DPD_CALIBRATION_SUPPORT
	INT32	TemperatureDPD;				/* temperature when do DPD calibration */
#endif /* DPD_CALIBRATION_SUPPORT */
	INT32	CurrentTemperature;			/* (BBP_R49 - Ref25C) * offset */
#endif /* MT7601 */

#ifdef MT7601
	MT7601_TX_ALC_DATA TxALCData;
	INT16	PAModeCCK[4];
	INT16	PAModeOFDM[8];
	INT16	PAModeHT[16];
#endif /* MT7601 */	
}RTMP_CHIP_CAP, *PRTMP_CHIP_CAP;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

/* structure to store channel TX power */
typedef struct _CHANNEL_TX_POWER {
	USHORT 	RemainingTimeForUse;	/*unit: sec */
	UCHAR 	Channel;
	CHAR 	Power;
	CHAR 	Power2;
	UCHAR 	MaxTxPwr;
	UCHAR 	DfsReq;
	UCHAR 	RegulatoryDomain;
#define CHANNEL_DEFAULT_PROP	0x00
#define CHANNEL_DISABLED		0x01	// no use 
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS			0x04
#define CHANNEL_RADAR			0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20
	UCHAR 	Flags;
#ifdef RT30xx
	UCHAR 	Tx0FinePowerCtrl;	/* Tx0 fine power control in 0.1dB step */
	UCHAR 	Tx1FinePowerCtrl;	/* Tx1 fine power control in 0.1dB step */
	UCHAR 	Tx2FinePowerCtrl;	/* Tx2 fine power control in 0.1dB step */
#endif				/* RT30xx */
} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;


typedef struct _COMMON_CONFIG {
	UCHAR 	CountryRegion;				/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
	UCHAR 	CountryRegionForABand;		/* Enum of country region for A band */
	UCHAR 	PhyMode;					/* PHY_11A, PHY_11B, PHY_11BG_MIXED, PHY_ABG_MIXED */
	/* Tx & Rx Stream number selection */
	USHORT 	TxStream;
	USHORT 	RxStream;
	ABGBAND_STATE 	BandState;
	UCHAR 	Channel;
	UCHAR 	SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR 	SupRateLen;
	UCHAR 	ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR 	ExtRateLen;
	UCHAR 	DesireRate[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR 	MaxDesiredRate;
	UCHAR 	ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];
	REG_TRANSMIT_SETTING 	RegTransmitSetting;		/*registry transmit setting. this is for reading registry setting only. not useful. */
	ADD_HT_INFO_IE 	AddHTInfo;						/* Useful as AP. */
	NEW_EXT_CHAN_IE 	NewExtChanOffset;			/*7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present */
	RT_HT_CAPABILITY 	DesiredHtPhy;
	HT_CAPABILITY_IE 	HtCapability;
	UCHAR 	bRdg;
	BACAP_STRUC 	BACapability;		/* NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	BACAP_STRUC 	REGBACapability;	/* NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	UCHAR 	bForty_Mhz_Intolerant;
	UCHAR 	BBPCurrentBW;				/* BW_10,       BW_20, BW_40 */
	UCHAR 	CentralChannel;				/* Central Channel when using 40MHz is indicating. not real channel. */
	EDCA_PARM 	APEdcaParm;				/* EDCA parameters of the current associated AP */
	UCHAR 	bWiFiTest;					/* Enable this parameter for WiFi test */
	UCHAR 	bEnableTxBurst;
	UCHAR 	HT_Disable;					/* 1: disable HT function; 0: enable HT function */
	UCHAR	TxBASize;
	UCHAR	TxPreamble;
	UCHAR	bMIMOPSEnable;
	HTTRANSMIT_SETTING 	MlmeTransmit;	
	USHORT 	NumOfBulkInIRP;
} COMMON_CONFIG, *PCOMMON_CONFIG;


typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef BIG_ENDIAN
	struct {
		USHORT rsv:3;
		USHORT FixedTxMode:2;	/* If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;			/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;			/* MCS */
		USHORT PhyMode:4;
		USHORT FixedTxMode:2;	/* If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode. */
		USHORT rsv:3;
	} field;
#endif
	USHORT word;
 } DESIRED_TRANSMIT_SETTING, *PDESIRED_TRANSMIT_SETTING;

typedef struct _RTMP_RA_LEGACY_TB
{
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR	Rsv2:1;
	UCHAR	Mode:3;
	UCHAR	BW:2;
	UCHAR	ShortGI:1;
	UCHAR	STBC:1;
#else
	UCHAR	STBC:1;
	UCHAR	ShortGI:1;
	UCHAR	BW:2;
	UCHAR	Mode:3;
	UCHAR	Rsv2:1;
#endif
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
} RTMP_RA_LEGACY_TB, *PRTMP_RA_LEGACY_TB;

#define PTX_RA_LEGACY_ENTRY(pTable, idx)	((RTMP_RA_LEGACY_TB *)&(pTable[(idx+1)*5]))

typedef struct _RSSI_SAMPLE {
	CHAR 	LastRssi0;			/* last received RSSI */
	CHAR 	LastRssi1;			/* last received RSSI */
	CHAR 	LastRssi2;			/* last received RSSI */
#if 0
	CHAR 	AvgRssi0;
	CHAR 	AvgRssi1;
	CHAR 	AvgRssi2;
	SHORT 	AvgRssi0X8;
	SHORT 	AvgRssi1X8;
	SHORT 	AvgRssi2X8;
	CHAR 	LastSnr0;
	CHAR 	LastSnr1;
	CHAR 	LastSnr2;
	CHAR	AvgSnr0;
	CHAR 	AvgSnr1;
	CHAR 	AvgSnr2;
	SHORT 	AvgSnr0X8;
	SHORT 	AvgSnr1X8;
	SHORT 	AvgSnr2X8;
	CHAR 	LastNoiseLevel0;
	CHAR 	LastNoiseLevel1;
	CHAR 	LastNoiseLevel2;
#endif	
} RSSI_SAMPLE;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR 	EvaluatePeriod;			/* 0:not evalute status, 1: evaluate status, 2: switching status */
	UCHAR 	EvaluateStableCnt;
	UCHAR 	Pair1PrimaryRxAnt;		/* 0:Ant-E1, 1:Ant-E2 */
	UCHAR 	Pair1SecondaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
#ifdef CONFIG_STA_SUPPORT
	SHORT 	Pair1AvgRssi[2];		/* AvgRssi[0]:E1, AvgRssi[1]:E2 */
	SHORT 	Pair2AvgRssi[2];		/* AvgRssi[0]:E3, AvgRssi[1]:E4 */
#endif /* CONFIG_STA_SUPPORT */
	SHORT 	Pair1LastAvgRssi;		/* */
	SHORT 	Pair2LastAvgRssi;		/* */
	ULONG 	RcvPktNumWhenEvaluate;
	UCHAR 	FirstPktArrivedWhenEvaluate;
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;


typedef struct _STA_ADMIN_CONFIG {
   	UCHAR 	bAdhocN;
	UCHAR 	bHardwareRadio;		/* Hardware controlled Radio enabled */
	UCHAR   bNotFirstScan;		/* Sam add for ADHOC flag to do first scan when do initialization */
	UCHAR	bSwRadio;			/* Software controlled Radio On/Off, TRUE: On */
	UCHAR 	bHwRadio;			/* Hardware controlled Radio On/Off, TRUE: On */
	UCHAR 	bRadio;				/* Radio state, And of Sw & Hw radio state */
	UCHAR	bShowHiddenSSID;	/* Show all known SSID in SSID list get operation */
	UCHAR	AdaptiveFreq;
	BOOLEAN bConnected;

	RT_HT_PHY_INFO DesiredHtPhyInfo;
	UCHAR	bTGnWifiTest;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;
	HTTRANSMIT_SETTING HTPhyMode;
	HTTRANSMIT_SETTING MaxHTPhyMode;
	HTTRANSMIT_SETTING MinHTPhyMode;			/* For transmit phy setting in TXWI. */
	UCHAR 	bAutoTxRateSwitch;

	USHORT RPIDensity[8];	/* Array for RPI density collection */
	
	NDIS_802_11_NETWORK_INFRASTRUCTURE	Networkmode;
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_ENCRYPTION_MODE		EncMode;
	UCHAR	RouterMac[MAC_ADDR_LEN];
	
	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;							/* Indicate current Pair & Group use different cipher suites */
	UCHAR 	WpaPassPhrase[64];					/* WPA PSK pass phrase */
	INT32 	WpaPassPhraseLen;						/* the length of WPA PSK pass phrase */
	UCHAR 	PMK[LEN_PMK];							/* WPA PSK mode PMK */
	UCHAR 	PTK[LEN_PTK];							/* WPA PSK mode PTK */
	UCHAR 	GTK[MAX_LEN_GTK];						/* GTK from authenticator */
	UCHAR 	GNonce[32];							/* GNonce for WPA2PSK from authenticator */
	UCHAR 	RSNIE_Len;
	UCHAR 	RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be little-endian format. */
	UCHAR 	DefaultKeyId;
	RSSI_SAMPLE RssiSample;

	UCHAR	CipherErrCnt;
}STA_ADMIN_CONFIG, *PSTA_ADMIN_CONFIG;


typedef struct _STA_ACTIVE_CONFIG {
	USHORT 	Aid;
	USHORT 	AtimWin;		/* in kusec; IBSS parameter set element */
	USHORT 	CapabilityInfo;
	USHORT 	CfpMaxDuration;
	USHORT 	CfpPeriod;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR 	SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR 	ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR 	SupRateLen;
	UCHAR 	ExtRateLen;
	/* Copy supported ht from desired AP's beacon. We are trying to match */
	RT_HT_PHY_INFO 	SupportedPhyInfo;
	RT_HT_CAPABILITY 	SupportedHtPhy;
} STA_ACTIVE_CONFIG, *PSTA_ACTIVE_CONFIG;

typedef enum _RTMP_INF_TYPE_ {	
	RTMP_DEV_INF_UNKNOWN 	= 0,
	RTMP_DEV_INF_PCI 		= 1,
	RTMP_DEV_INF_USB 		= 2,
	RTMP_DEV_INF_RBUS 		= 4,
	RTMP_DEV_INF_PCIE 		= 5
} RTMP_INF_TYPE;


typedef struct _FREQUENCY_CALIBRATION_CONTROL {
	CHAR 	bEnableFrequencyCalibration; 	/* Enable the frequency calibration algorithm */
	CHAR 	bSkipFirstFrequencyCalibration; /* Avoid calibrating frequency at the time the STA is just link-up */
	CHAR 	bApproachFrequency; 			/* Approach the frequency */
	CHAR 	AdaptiveFreqOffset; 			/* Adaptive frequency offset */
	CHAR	LatestFreqOffsetOverBeacon; 	/* Latest frequency offset from the beacon */
	CHAR 	BeaconPhyMode; 					/* Latest frequency offset from the beacon */
} FREQUENCY_CALIBRATION_CONTROL, *PFREQUENCY_CALIBRATION_CONTROL;

typedef struct _MLME_AUX {
    UCHAR 	BssType;
    UCHAR 	Ssid[MAX_LEN_OF_SSID];
    UCHAR   SsidLen;
    UCHAR   Bssid[6];
	UCHAR	AutoReconnectSsid[MAX_LEN_OF_SSID];
	UCHAR	AutoReconnectSsidLen;
    USHORT  Alg;
    UCHAR   ScanType;
    UCHAR   Channel;
	UCHAR  	CentralChannel;
    USHORT  Aid;
    USHORT  CapabilityInfo;
    USHORT 	BeaconPeriod;
    USHORT 	CfpMaxDuration;
    USHORT 	CfpPeriod;
    USHORT  AtimWin;
	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR 	SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR	ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR	SupRateLen;
	UCHAR	ExtRateLen;
	HT_CAPABILITY_IE 	HtCapability;
	UCHAR 	HtCapabilityLen;
	ADD_HT_INFO_IE 	AddHtInfo;					/* AP might use this additional ht info IE */
//	EXT_CAP_INFO_ELEMENT ExtCapInfo; 			/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */
	UCHAR 	NewExtChannelOffset;
	/*RT_HT_CAPABILITY	SupportedHtPhy; */
    /* new for QOS */
//  QOS_CAPABILITY_PARM APQosCapability;    	/* QOS capability of the current associated AP */
    EDCA_PARM 	APEdcaParm;         			/* EDCA parameters of the current associated AP */
//    QBSS_LOAD_PARM      APQbssLoad;         	/* QBSS load of the current associated AP */
    /* new to keep Ralink specific feature */
    ULONG 	APRalinkIe;
//  BSS_TABLE 	SsidBssTab;     				/* AP list for the same SSID */
//  BSS_TABLE 	RoamTab;        				/* AP list eligible for roaming */
    ULONG 	BssIdx;
    ULONG 	RoamIdx;
//	BOOLEAN		CurrReqIsFromNdis;
//  RALINK_TIMER_STRUCT BeaconTimer, ScanTimer, APScanTimer;
//  RALINK_TIMER_STRUCT AuthTimer;
//  RALINK_TIMER_STRUCT AssocTimer, ReassocTimer, DisassocTimer;
} MLME_AUX, *PMLME_AUX;

typedef struct _RTMP_RF_REGS {
	UCHAR Channel;
	UINT32 R1;
	UINT32 R2;
	UINT32 R3;
	UINT32 R4;
} RTMP_RF_REGS, *PRTMP_RF_REGS;


typedef struct _AP_ADMIN_CONFIG {
	USHORT 		CapabilityInfo;
	UCHAR 		MaxStaNum;	/* Limit the STA connection number per BSS */
	UCHAR 		StaCount;

	/* for wpa */
	RTMP_SONIX_TIMER CounterMeasureTimer;
	RTMP_SONIX_TIMER REKEYTimer;
	ULONG 		RxErrorCount;
	ULONG 		RxDropCount;
	ULONG 		TxErrorCount;
	ULONG 		TxDropCount;
	ULONG 		MICFailureCounter;
	UCHAR 		CMTimerRunning;
	UCHAR 		BANClass3Data;
	UCHAR 		GMK[LEN_PMK];
	UCHAR		GNonce[LEN_PMK];
	UCHAR 		GTK[LEN_PMK];
	UCHAR 		PMK[LEN_PMK];
	UCHAR 		RSNIE_Len[2];
	UCHAR 		RSN_IE[2][MAX_LEN_OF_RSNIE];
	UCHAR 		WpaPassPhrase[64];						/* WPA PSK pass phrase */
	INT32 		WpaPassPhraseLen;						/* the length of WPA PSK pass phrase */	
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_ENCRYPTION_MODE		EncMode;
	NDIS_802_11_WEP_STATUS 			WepStatus;
	NDIS_802_11_WEP_STATUS 			GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER 			WpaMixPairCipher;

	/* for Group Rekey */
	RT_WPA_REKEY WPAREKEY;
	ULONG REKEYCOUNTER;
	UCHAR REKEYTimerRunning;
	UINT8 RekeyCountDown;
	
	BOOLEAN 	bHideSsid;
	UINT16 		StationKeepAliveTime;	/* unit: second */

	UCHAR LastSNR0;		/* last received BEACON's SNR */
	UCHAR LastSNR1;		/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;		/* last received BEACON's SNR for 2nd  antenna */
#endif				/* DOT11N_SS3_SUPPORT */

} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXWI_NMAC {
	/* Word 0 */
	UINT32 	PHYMODE:2;
	UINT32	Rsv1:3;	
	UINT32	STBC:2;
	UINT32	ShortGI:1;
	UINT32	BW:1;
	UINT32	MCS:7;
	UINT32	TXLUT:1;
	UINT32	TXRPT:1;
	UINT32	Autofallback:1;	/* TX rate auto fallback disable */
	UINT32	CWMIN:3;
	UINT32	txop:2;
	UINT32	MpduDensity:3;
	UINT32	AMPDU:1;
	UINT32	TS:1;
	UINT32	CFACK:1;
	UINT32	MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32	FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	
	/* Word 1 */
	UINT32	TxPktId:4;
	UINT32	MPDUtotalByteCnt:12;
	UINT32	wcid:8;
	UINT32	BAWinSize:6;
	UINT32	NSEQ:1;
	UINT32	ACK:1;

	/* Word 2 */
	UINT32	IV;
	/* Word 3 */
	UINT32	EIV;

	/* Word 4 */
	UINT32	Rsv3:9;
	UINT32	PIFS_REV:1;
	UINT32	Rsv2:1;
	UINT32	CCP:1;		/* Channel Check Packet */
	UINT32	TxPwrAdj:4;
	UINT32	TxStreamMode:8;
	UINT32	TxEAPId:8;
}	TXWI_NMAC, *PTXWI_NMAC;
#else
typedef	struct GNU_PACKED _TXWI_NMAC {
	/* Word	0 */
	/* ex: 00 03 00 40 means txop = 3, PHYMODE = 1 */
	UINT32 	FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	UINT32	MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32	CFACK:1;
	UINT32	TS:1;
	UINT32	AMPDU:1;
	UINT32	MpduDensity:3;
	UINT32	txop:2;
	UINT32	CWMIN:3;
	UINT32	Autofallback:1;	/* TX rate auto fallback disable */
	UINT32	TXRPT:1;
	UINT32	TXLUT:1;
	UINT32	MCS:7;
	UINT32	BW:1;
	UINT32	ShortGI:1;
	UINT32	STBC:2;
	UINT32	Rsv1:3;	
	UINT32	PHYMODE:2;  

	/* Word1 */
	/* ex:  1c ff 38 00 means ACK=0, BAWinSize=7, MPDUtotalByteCnt = 0x38 */
	UINT32	ACK:1;
	UINT32	NSEQ:1;
	UINT32	BAWinSize:6;
	UINT32	wcid:8;
	UINT32	MPDUtotalByteCnt:12;
	UINT32	TxPktId:4;
	
	/*Word2 */
	UINT32	IV;
	
	/*Word3 */
	UINT32	EIV;

	/* Word 4 */
	UINT32	TxEAPId:8;
	UINT32	TxStreamMode:8;
	UINT32	TxPwrAdj:4;
	UINT32	CCP:1;		/* Channel Check Packet */
	UINT32	Rsv2:1;
	UINT32	PIFS_REV:1;
	UINT32	Rsv3:9;
}	TXWI_NMAC, *PTXWI_NMAC;
#endif /* RT_BIG_ENDIAN */


typedef	union GNU_PACKED _TXWI_STRUC {
#ifdef RLT_MAC
	struct _TXWI_NMAC 	TXWI_N;
#endif /* RLT_MAC */
	UINT32 	word;
}TXWI_STRUC, *PTXWI_STRUC;

#define TxWIMPDUByteCnt		TXWI_N.MPDUtotalByteCnt
#define TxWIWirelessCliID	TXWI_N.wcid
#define TxWIFRAG			TXWI_N.FRAG
#define TxWICFACK			TXWI_N.CFACK
#define TxWITS				TXWI_N.TS
#define TxWIAMPDU			TXWI_N.AMPDU
#define TxWIACK				TXWI_N.ACK
#define TxWITXOP			TXWI_N.txop
#define TxWINSEQ			TXWI_N.NSEQ
#define TxWIBAWinSize		TXWI_N.BAWinSize
#define TxWIShortGI			TXWI_N.ShortGI
#define TxWISTBC			TXWI_N.STBC
#define TxWIPacketId		TXWI_N.TxPktId
#define TxWIBW				TXWI_N.BW
#define TxWIMCS				TXWI_N.MCS
#define TxWIPHYMODE			TXWI_N.PHYMODE
#define TxWIMIMOps			TXWI_N.MIMOps
#define TxWIMpduDensity		TXWI_N.MpduDensity
#define TxWITXRPT			TXWI_N.TXRPT
#define TxWITXLUT			TXWI_N.TXLUT

typedef struct GNU_PACKED _TXINFO_NMAC_PKT {
	UINT32 	pkt_len:16;
	UINT32 	next_vld:1;
	UINT32 	tx_burst:1;
	UINT32 	rsv0:1;
	UINT32 	pkt_80211:1;
	UINT32 	tso:1;
	UINT32 	cso:1;
	UINT32 	rsv1:2;
	UINT32 	wiv:1;
	UINT32 	QSEL:2;
	UINT32 	d_port:3;
	UINT32 	info_type:2;
} TXINFO_NMAC_PKT;

typedef struct GNU_PACKED _TXINFO_NMAC_CMD {
	UINT32 	pkt_len:16;
	UINT32 	cmd_seq:4;
	UINT32 	cmd_type:7;
	UINT32 	d_port:3;
	UINT32 	info_type:2;
} TXINFO_NMAC_CMD;


#define TXINFO_SIZE			4

typedef union GNU_PACKED _TXINFO_STRUC{
#ifdef RLT_MAC
	struct _TXINFO_NMAC_PKT txinfo_nmac_pkt;
	//struct _TXINFO_NMAC_CMD txinfo_nmac_cmd;
#endif /* RLT_MAC */
	UINT32 word;
} TXINFO_STRUC, *PTXINFO_STRUC;

typedef struct _TX_DESC_RING {
	USHORT 	BulkOUTLen;
	PUCHAR 	pBulkOUTBuff;
	PTXINFO_STRUC 	pTXINF;
	PTXWI_STRUC 	pTXWI;
	USHORT 	WifiHeaderLen;
	PHEADER_802_11	pWifiHeader;
	USHORT	DataLen;
	PUCHAR	pDataBuff;	
} TX_CONTEXT, *PTX_CONTEXT;

typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		reserved;
	UINT32		BA:1;				/* The received frame is part of BA session, so re-ordering is required. */
	UINT32		DATA:1;				/* 1: the received frame is DATA type. */
	UINT32		NULLDATA:1;			/* 1: the received frame has sub-type NULL/QOS-NULL. */
	UINT32		FRAG:1;				/* 1: the received frame is a fragment. */
	UINT32		U2M:1;				/* 1: this RX frame is unicast to me . ADDR1 = my MAC address */
	UINT32		Mcast:1;			/* 1: this is a multicast frame. ADDR1 = multicast */
	UINT32		Bcast:1;			/* 1: this is a broadcast frame. ADDR1 = ff:ff:ff:ff:ff:ff  */
	UINT32		MyBss:1;			/* 1: the received frame BSSID is one of my BSS. */
	UINT32		Crc:1;				/* 1: CRC error */
	UINT32		CipherErr:2;	  	/* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	UINT32		AMSDU:1;			/* rx with 802.3 header, not 802.11 header. obsolete. */
	UINT32		HTC:1;
	UINT32		RSSI:1;
	UINT32		L2PAD:1;			/* 1: the L2 header is recognizable and is 2-byte-padded to ensure payloads aligns at 4-byte boundary. 0:L2 header not extra padded */
	UINT32		AMPDU:1;
	UINT32		Decrypted:1;		/* 1: the is a decrypted frame */
	UINT32		BssIdx3:1;
	UINT32		wapi_kidx:1;
	UINT32		pn_len:3;
#ifdef HDR_TRANS_SUPPORT
	UINT32		pkt_80211:1;
	UINT32		rsv:5;
#else
	UINT32		rsv:6;
#endif /* HDR_TRANS_SUPPORT */
	UINT32		tcp_sum_bypass:1;	/* TCP/UDP checksum bypass(hw does not do checksum) */
	UINT32		ip_sum_bypass:1;	/* IP checksum bypass(hw does not do checksum) */
	UINT32		tcp_sum_err:1;		/* TCP checksum error */
	UINT32		ip_sum_err:1;		/* IP checksum error */
}RXINFO_STRUC, *PRXINFO_STRUC;



typedef	struct GNU_PACKED _RXWI_STRUC{
	/* Word	0 */
	UINT32		wcid:8;
	UINT32		key_idx:2;
	UINT32		bss_idx:3;
	UINT32		UDF:3;
	UINT32		MPDUtotalByteCnt:12;
	UINT32		tid:4;

	/* Word	1 */
	UINT32		FRAG:4;
	UINT32		SEQUENCE:12;
	UINT32		mcs:7;
	UINT32		bw:1;
	UINT32		sgi:1;
	UINT32		stbc:2;
	UINT32		eTxBF:1; 			/* eTxBF enable */
	UINT32		Sounding:1; 		/* Sounding enable */
	UINT32		iTxBF:1;			/* iTxBF enable */
	UINT32		phy_mode:2;    		/* 1: this RX frame is unicast to me */

	/*Word2 */
	UINT32		RSSI0:8;
	UINT32		RSSI1:8;
	UINT32		RSSI2:8;
	UINT32		rsv1:8;

	/*Word3 */
	UINT32		SNR0:8;
	UINT32		SNR1:8;
	UINT32		SNR2:8;
	UINT32		FOFFSET:8;

	/*Word4 */
	UINT32		rsv3;

//#if defined(RT5592) || defined(MT7601)
	/* Word 5 */
	/* For Exper Antenna */
	UINT32      EANT_ID:8;
	UINT32      rsv4:24;
//#endif /* RT5592 */
}	RXWI_STRUC, *PRXWI_STRUC;




#define RxWIMPDUByteCnt		RXWI_N.MPDUtotalByteCnt
#define RxWIWirelessCliID	RXWI_N.wcid
#define RxWIKeyIndex		RXWI_N.key_idx
#define RxWIMCS				RXWI_N.mcs
#define RxWIBW				RXWI_N.bw
#define RxWIBSSID			RXWI_N.bss_idx
#define RxWISGI				RXWI_N.sgi
#define RxWIPhyMode			RXWI_N.phy_mode
#define RxWISTBC			RXWI_N.stbc
#define RxWITID				RXWI_N.tid
#define RxWIRSSI0			RXWI_N.rssi[0]
#define RxWIRSSI1			RXWI_N.rssi[1]
#define RxWIRSSI2			RXWI_N.rssi[2]
#define RxWISNR0			RXWI_N.bbp_rxinfo[0]
#define RxWISNR1			RXWI_N.bbp_rxinfo[1]
#define RxWISNR2			RXWI_N.bbp_rxinfo[2]
#define RxWIFOFFSET			RXWI_N.bbp_rxinfo[3]

#if 0
typedef struct _APScanResultElement 
{	
	UCHAR 	channel;
	UCHAR 	SsIdLen;
	UCHAR 	SsId[MAX_LEN_OF_SSID];
	UCHAR	BssId[MAC_ADDR_LEN];
	CHAR 	auth[16]; // for APP toread
	UCHAR 	auth_mode;
	UCHAR 	enc_mode;
	UCHAR 	signal;
} APScanResultElement ;


typedef struct _APScanResultArray
{
	UCHAR nCollectNbr;
	APScanResultElement ScanElem[MAX_AP_SCAN_NUM];
} APScanResultArray;
#endif

struct _RTMP_ADAPTER {
	RTMP_CHIP_CAP 	chipCap;
	COMMON_CONFIG 	CommonCfg;
	UCHAR 	OpMode;				/* OPMODE_STA, OPMODE_AP */
	CHAR 	Ssid[MAX_LEN_OF_SSID];
	INT32 	SsidLen;	
	INT32 	channel;
	UCHAR 	essid[6];
	ULONG 	beaconPeriod;
	USHORT 	DataSeq;
	UINT32 	MACVersion;
	USHORT 	EEPROMAddressNum;
	UCHAR 	PermanentAddress[6];
	UCHAR 	CurrentAddress[6];
	UCHAR 	DataAddr[6];
	UCHAR 	MgmAddr[6];
	ULONG 	EepromVersion;		/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG 	FirmwareVersion;	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	USHORT 	EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR 	EepromAccess;
	UCHAR 	EFuseTag;
	USHORT 	BeaconOffset[HW_BEACON_MAX_NUM];
	TXWI_STRUC BeaconTxWI;
	PUCHAR 	BeaconBuf;
	CHANNEL_TX_POWER 	TxPower[MAX_NUM_OF_CHANNELS];		/* Store Tx power value for all channels. */
	CHANNEL_TX_POWER 	ChannelList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */
	ULONG 	Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG 	Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG 	Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG 	Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	UCHAR	bAutoTxAgcA;					/* Enable driver auto Tx Agc control */
	UCHAR 	TssiRefA;						/* Store Tssi reference value as 25 temperature. */
	UCHAR 	TssiPlusBoundaryA[5];			/* Tssi boundary for increase Tx power to compensate. */
	UCHAR 	TssiMinusBoundaryA[5];			/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR 	TxAgcStepA;						/* Store Tx TSSI delta increment / decrement value */
	CHAR 	TxAgcCompensateA;				/* Store the compensation (TxAgcStep * (idx-1)) */
	UCHAR	bAutoTxAgcG;					/* Enable driver auto Tx Agc control */
	UCHAR 	TssiRefG;						/* Store Tssi reference value as 25 temperature. */
	UCHAR 	TssiPlusBoundaryG[5];			/* Tssi boundary for increase Tx power to compensate. */
	UCHAR 	TssiMinusBoundaryG[5];			/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR 	TxAgcStepG;						/* Store Tx TSSI delta increment / decrement value */
	CHAR 	TxAgcCompensateG;				/* Store the compensation (TxAgcStep * (idx-1)) */
	EEPROM_ANTENNA_STRUC 	Antenna;		/* Since ANtenna definition is different for a & g. We need to save it for future reference. */
	EEPROM_NIC_CONFIG2_STRUC 	NicConfig2;
	SOFT_RX_ANT_DIVERSITY 	RxAnt;
	RTMP_INF_TYPE 	infType;
	UINT8 	RfIcType;				/* RFIC_xxx */			
	ULONG 	RfFreqOffset;			/* Frequency offset for channel switching */
	CHAR 	BbpRssiToDbmDelta;		/* change from UCHAR to CHAR for high power */
	CHAR 	BGRssiOffset0;			/* Store B/G RSSI#0 Offset value on EEPROM 0x46h */
	CHAR	BGRssiOffset1;			/* Store B/G RSSI#1 Offset value */
	CHAR 	BGRssiOffset2;			/* Store B/G RSSI#2 Offset value */
	CHAR 	ARssiOffset0;			/* Store A RSSI#0 Offset value on EEPROM 0x4Ah */
	CHAR 	ARssiOffset1;			/* Store A RSSI#1 Offset value */
	CHAR 	ARssiOffset2;			/* Store A RSSI#2 Offset value */
	TX_POWER_CONTROL TxPowerCtrl;	/* The Tx power control using the internal ALC */
	UCHAR ChannelListNum;			/* number of channel in ChannelList[] */
//	LED_CONTROL LedCntl;
	MLME_STRUCT 	Mlme;
//	BBP_R66_TUNING 	BbpTuning;
	UCHAR	LastMCUCmd;
	UCHAR 	MmpsMode;
	AP_ADMIN_CONFIG 	ApCfg;		/* Ap configuration */
	STA_ADMIN_CONFIG 	StaCfg;		/* user desired settings */
	STA_ACTIVE_CONFIG 	StaActive;	/* valid only when ADHOC_ON(pAd) || INFRA_ON(pAd) */
	MLME_AUX 	MlmeAux;			/* temporary settings used during MLME state machine */
	FREQUENCY_CALIBRATION_CONTROL 	FreqCalibrationCtrl;	/* The frequency calibration control */
	RTMP_RF_REGS 	LatchRfRegs;							/* latch th latest RF programming value since RF IC doesn't support READ */
	/* TRX Memory */
	TX_CONTEXT 	TxContext;
	/* TRX Statistic */	
	ULONG	rxCorrectSeq;
//	ULONG 	rxDataFrameCount;
//	ULONG 	rxMyDataFrameCount;
	ULONG	rxBeaconCount;
	ULONG	txFailCount;
	ULONG	txRetryCount;
	ULONG	txOKCount;
	ULONG	txTotalCount;
	PUCHAR	RxAVDataPtr;
	PUCHAR	RxUSBDataPtr;
	/*  Cipher */
	CIPHER_KEY 	SharedKey[2][4];
	UCHAR 	DefaultKeyId;
	UCHAR   AuthMode;
	UCHAR	WepStatus;
	UCHAR 	WpaState;
	USHORT 	PortSecured;
	USHORT 	PrivacyFilter;
	UCHAR  	apidx;
	UCHAR  	GTKState;
	UCHAR 	Aid;
	UCHAR 	WEPEnabled;
	/* Tx Rate */
	UCHAR	TxBW;
	UCHAR	TxMCS;
	UCHAR	TxGI;
	UCHAR	SetTxPHYRate;
	UCHAR	SetTxPHYMode;
	UCHAR   SetTxFallback;

	/* MLME */
	UCHAR	LinkState;
	UCHAR 	smartSetup;
	UCHAR	smartSetupCh;
	PUCHAR 	TxMgmBase;
	PUCHAR 	TxMgmBase_org;
	PUCHAR 	tx_dma_pool[MAX_TX_RING];
	PUCHAR 	tx_dma_org[MAX_TX_RING];
	UCHAR 	scanChannel;
	UCHAR 	scanIndex;
	UCHAR 	scanFlag;
//	BOOLEAN WiFiInitialDone;
//	Counters	SonixCounters;
	/* MT7601 */
	UCHAR 	CmdSeq;
	CHAR 	BGRssiOffset[3]; 		/* Store B/G RSSI #0/1/2 Offset value on EEPROM 0x46h */
	CHAR 	ARssiOffset[3]; 		/* Store A RSSI 0/1/2 Offset value on EEPROM 0x4Ah */
	CHAR 	BLNAGain;				/* Store B/G external LNA#0 value on EEPROM 0x44h */
	CHAR 	ALNAGain0;				/* Store A external LNA#0 value for ch36~64 */
	CHAR 	ALNAGain1;				/* Store A external LNA#1 value for ch100~128 */
	CHAR 	ALNAGain2;				/* Store A external LNA#2 value for ch132~165 */
	UINT32 	TxCCKPwrCfg;
	BSS_TABLE bss_table;
//	WiFiSetupInfo 	wifi_info;
	PMGMTENTRY 	pFirstLinkNode;
	PMGMTENTRY 	pLastLinkNode;
	UCHAR 	node_num;

	/* Power save */
	UCHAR 	APPowerSave;
	ULONG 	PSPeriod;
	UCHAR 	mac_tx_stop;

	/* Unified code */
	WIFI_RUN_MODE 	run_mode;

	/* For Private Tkip calculate */
#if 0 // TKIP unused code.	
	PRIVATE_STRUC 	PrivateInfo;	/* Private information & counters */
#endif

	/* EDCCA */
	ULONG 	ed_threshold;
	ULONG 	ed_false_cca_threshold;
	ULONG 	ed_block_tx_threshold;
	ULONG 	ed_chk_period;  // in unit of ms
	UCHAR 	ed_tx_stoped;
	UINT 	ed_trigger_cnt;
	UINT 	ed_silent_cnt;
	UINT 	ed_false_cca_cnt;
	UINT 	ed_stat;
	UINT 	false_cca_stat;

	/* AP Scan List */
	unsigned char	nCollectNbr;
	struct _APScanResultElement *FirstElem;

	wifi_event_rsp_hndler hndler;



//=========Type protect

	UCHAR 	numTypeProtect;
	USHORT	valTypeProtect[MAX_TYPE_PROTECT];
	
//	USBH_Device_Structure *DEV;
	UCHAR	USBPlugOut;				/* Detect USB dongle if plug out */

	/* Periodic jobs */
	RTMP_SONIX_TIMER PeriodicTimer;
};


#endif /* __RTMP_H__ */
