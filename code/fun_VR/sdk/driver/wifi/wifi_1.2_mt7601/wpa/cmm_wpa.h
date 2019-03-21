#ifndef __CMM_WPA_H__
#define __CMM_WPA_H__


#include "rtmp_if.h"
#include "types.h"
#include "system.h"

typedef char 				* PNDIS_BUFFER;

#define MIX_CIPHER_WPA_TKIP_ON(x)       (((x) & 0x08) != 0)
#define MIX_CIPHER_WPA_AES_ON(x)        (((x) & 0x04) != 0)
#define MIX_CIPHER_WPA2_TKIP_ON(x)      (((x) & 0x02) != 0)
#define MIX_CIPHER_WPA2_AES_ON(x)       (((x) & 0x01) != 0)


#define MAX_MBSSID_NUM(__pAd)		1

#define MAIN_MBSSID                 0


/* sanity check for apidx */
#define MBSS_MR_APIDX_SANITY_CHECK(__pAd, apidx) \
    { if ((apidx >= MAX_MBSSID_NUM(__pAd)) || \
		(apidx >= HW_BEACON_MAX_NUM)) { \
          DBGPRINT(RT_ERROR, ("%s> Error! apidx = %d > MAX_MBSSID_NUM!\n", __FUNCTION__, apidx)); \
	  apidx = MAIN_MBSSID; } }


#define	SET_UINT16_TO_ARRARY(_V, _LEN)			\
{												\
	_V[0] = ((UINT16)_LEN) >> 8;				\
	_V[1] = ((UINT16)_LEN & 0xFF);				\
}

#define	INC_UINT16_TO_ARRARY(_V, _LEN)			\
{												\
	UINT16	var_len;							\
												\
	var_len = (_V[0]<<8) | (_V[1]);				\
	var_len += _LEN;							\
												\
	_V[0] = (var_len & 0xFF00) >> 8;			\
	_V[1] = (var_len & 0xFF);					\
}

#define	ADD_ONE_To_64BIT_VAR(_V)				\
{												\
	UCHAR	cnt = LEN_KEY_DESC_REPLAY;			\
	do											\
	{											\
		cnt--;									\
		_V[cnt]++;								\
		if (cnt == 0)							\
			break;								\
	}while (_V[cnt] == 0);						\
}

#define IS_WPA_CAPABILITY(a)       (((a) >= Ndis802_11AuthModeWPA) && ((a) <= Ndis802_11AuthModeWPA1PSKWPA2PSK))


#ifndef ROUND_UP
#define ROUND_UP(__x, __y) \
	(((ULONG)((__x)+((__y)-1))) & ((ULONG)~((__y)-1)))
#endif

#define	CONV_ARRARY_TO_UINT16(_V)	((_V[0]<<8) | (_V[1]))



/* The length is the EAPoL-Key frame except key data field. 
   Please refer to 802.11i-2004 ,Figure 43u in p.78 */
#define MIN_LEN_OF_EAPOL_KEY_MSG	95	

#define LEN_PTK_KCK					16
#define LEN_PTK_KEK					16
#define LEN_TK						16	/* The length Temporal key. */
#define LEN_TKIP_MIC				8	/* The length of TX/RX Mic of TKIP */
#define LEN_TK2						(2 * LEN_TKIP_MIC)
#define LEN_PTK						(LEN_PTK_KCK + LEN_PTK_KEK + LEN_TK + LEN_TK2)

#define LEN_TKIP_PTK				LEN_PTK
#define LEN_AES_PTK					(LEN_PTK_KCK + LEN_PTK_KEK + LEN_TK)
#define LEN_TKIP_GTK				(LEN_TK + LEN_TK2)
#define LEN_AES_GTK					LEN_TK
#define LEN_TKIP_TK					(LEN_TK + LEN_TK2)
#define LEN_AES_TK					LEN_TK


#define OFFSET_OF_PTK_TK			(LEN_PTK_KCK + LEN_PTK_KEK)	/* The offset of the PTK Temporal key in PTK */
#define OFFSET_OF_AP_TKIP_TX_MIC	(OFFSET_OF_PTK_TK + LEN_TK)
#define OFFSET_OF_AP_TKIP_RX_MIC	(OFFSET_OF_AP_TKIP_TX_MIC + LEN_TKIP_MIC)
#define OFFSET_OF_STA_TKIP_RX_MIC	(OFFSET_OF_PTK_TK + LEN_TK)
#define OFFSET_OF_STA_TKIP_TX_MIC	(OFFSET_OF_AP_TKIP_TX_MIC + LEN_TKIP_MIC)


/* The related length of the EAPOL Key frame */
#define LEN_KEY_DESC_NONCE			32
#define LEN_KEY_DESC_IV				16
#define LEN_KEY_DESC_RSC			8
#define LEN_KEY_DESC_ID				8
#define LEN_KEY_DESC_REPLAY			8
#define LEN_KEY_DESC_MIC			16

#define LEN_PMK						32
#define LEN_PMKID					16
#define LEN_PMK_NAME				16

/* RSN IE Length definition */
#define MAX_LEN_OF_RSNIE         	255
#define MIN_LEN_OF_RSNIE         	18
#define MAX_LEN_GTK					32
#define MIN_LEN_GTK					5

#define IE_WPA						221
#define IE_RSN						48

#define WPA_KDE_TYPE				0xdd


#define LEN_KDE_HDR					6
#define LEN_NONCE					32
#define LEN_PN						6
#define LEN_TKIP_IV_HDR				8
#define LEN_CCMP_HDR				8
#define LEN_CCMP_MIC				8
#define LEN_OUI_SUITE				4
#define LEN_WEP_TSC					3
#define LEN_WPA_TSC					6
#define LEN_WEP_IV_HDR				4
#define LENGTH_EAPOL_H              4

#define LEN_ICV						4

/* EAPOL frame Protocol Version */
#define	EAPOL_VER					1
#define	EAPOL_VER2					2


/* EAPOL-KEY Descriptor Type */
#define	WPA1_KEY_DESC				0xfe
#define WPA2_KEY_DESC               0x02


/* Key Descriptor Version of Key Information */
#define	KEY_DESC_TKIP			1
#define	KEY_DESC_AES			2
#define KEY_DESC_EXT			3



/*EAP Packet Type */
#define	EAPPacket		0
#define	EAPOLStart		1
#define	EAPOLLogoff		2
#define	EAPOLKey		3
#define	EAPOLASFAlert	4
#define	EAPTtypeMax		5


#define PAIRWISEKEY					1
#define GROUPKEY					0


#define	EAPOL_MSG_INVALID	0
#define	EAPOL_PAIR_MSG_1	1
#define	EAPOL_PAIR_MSG_2	2
#define	EAPOL_PAIR_MSG_3	3
#define	EAPOL_PAIR_MSG_4	4
#define	EAPOL_GROUP_MSG_1	5
#define	EAPOL_GROUP_MSG_2	6

/* RC4 init value, used fro WEP & TKIP */
#define PPPINITFCS32                0xffffffff	/* Initial FCS value */


/* Retry timer counter initial value */
#define PEER_MSG1_RETRY_TIMER_CTR      	0
#define PEER_MSG3_RETRY_TIMER_CTR      	10
#define GROUP_MSG1_RETRY_TIMER_CTR      20

/* WPA mechanism retry timer interval */
#define PEER_MSG1_RETRY_EXEC_INTV		1000	/* 1 sec */
#define PEER_MSG3_RETRY_EXEC_INTV		3000	/* 3 sec */
#define GROUP_KEY_UPDATE_EXEC_INTV		1000	/* 1 sec */
#define PEER_GROUP_KEY_UPDATE_INIV		2000	/* 2 sec */



#define MLME_UNSPECIFY_FAIL            	1
#define MLME_ALG_NOT_SUPPORT            13
#define MLME_INVALID_GROUP_CIPHER   	41
#define MLME_INVALID_PAIRWISE_CIPHER   	42
#define MLME_INVALID_AKMP   			43


/* group rekey interval */
#define TIME_REKEY                          0
#define PKT_REKEY                           1
#define DISABLE_REKEY                       2
#define MAX_REKEY                           2

#define MAX_REKEY_INTER                     0x3ffffff



/* for-wpa value domain of pMacEntry->WpaState  802.1i D3   p.114 */
typedef enum _GTKState {
	REKEY_NEGOTIATING,
	REKEY_ESTABLISHED,
	KEYERROR,
} GTK_STATE;


/*for-wpa value domain of pMacEntry->WpaState  802.1i D3   p.114 */
typedef enum _ApWpaState {
	AS_NOTUSE,					/* 0 */
	AS_DISCONNECT,				/* 1 */
	AS_DISCONNECTED,			/* 2 */
	AS_INITIALIZE,				/* 3 */
	AS_AUTHENTICATION,			/* 4 */
	AS_AUTHENTICATION2,			/* 5 */
	AS_INITPMK,					/* 6 */
	AS_INITPSK,					/* 7 */
	AS_PTKSTART,				/* 8 */
	AS_PTKINIT_NEGOTIATING,		/* 9 */
	AS_PTKINITDONE,				/* 10 */
	AS_UPDATEKEYS,				/* 11 */
	AS_INTEGRITY_FAILURE,		/* 12 */
	AS_KEYUPDATE,				/* 13 */
} AP_WPA_STATE;

/* */
/*	The definition of the cipher combination */
/* */
/* 	 bit3	bit2  bit1   bit0 */
/*	+------------+------------+ */
/* 	|	  WPA	 |	   WPA2   | */
/*	+------+-----+------+-----+ */
/*	| TKIP | AES | TKIP | AES | */
/*	|	0  |  1  |   1  |  0  | -> 0x06 */
/*	|	0  |  1  |   1  |  1  | -> 0x07 */
/*	|	1  |  0  |   0  |  1  | -> 0x09 */
/*	|	1  |  0  |   1  |  1  | -> 0x0B */
/*	|	1  |  1  |   0  |  1  | -> 0x0D */
/*	|	1  |  1  |   1  |  0  | -> 0x0E */
/*	|	1  |  1  |   1  |  1  |	-> 0x0F */
/*	+------+-----+------+-----+ */
/* */
typedef enum _WpaMixPairCipher {
	MIX_CIPHER_NOTUSE 			= 0x00,
	WPA_NONE_WPA2_TKIPAES 		= 0x03,	/* WPA2-TKIPAES */
	WPA_AES_WPA2_TKIP 			= 0x06,
	WPA_AES_WPA2_TKIPAES 		= 0x07,
	WPA_TKIP_WPA2_AES 			= 0x09,
	WPA_TKIP_WPA2_TKIPAES 		= 0x0B,
	WPA_TKIPAES_WPA2_NONE 		= 0x0C,	/* WPA-TKIPAES */
	WPA_TKIPAES_WPA2_AES 		= 0x0D,
	WPA_TKIPAES_WPA2_TKIP 		= 0x0E,
	WPA_TKIPAES_WPA2_TKIPAES 	= 0x0F,
} WPA_MIX_PAIR_CIPHER;

typedef struct GNU_PACKED _EID_STRUCT{
    UCHAR   Eid;
    UCHAR   Len;
    UCHAR   Octet[1];
} EID_STRUCT,*PEID_STRUCT, BEACON_EID_STRUCT, *PBEACON_EID_STRUCT;




/* It's defined in IEEE Std 802.11-2007 Table 8-4 */
typedef enum _WPA_KDE_ID
{		
   	KDE_RESV0,
   	KDE_GTK,
   	KDE_RESV2,
   	KDE_MAC_ADDR,
   	KDE_PMKID,
   	KDE_SMK,
   	KDE_NONCE,
   	KDE_LIFETIME,
   	KDE_ERROR,
   	KDE_RESV_OTHER
} WPA_KDE_ID;

/* EAPOL Key Information definition within Key descriptor format */
typedef	struct GNU_PACKED _KEY_INFO
{
#ifdef RT_BIG_ENDIAN
	UCHAR	KeyAck:1;
    UCHAR	Install:1;
    UCHAR	KeyIndex:2;
    UCHAR	KeyType:1;
    UCHAR	KeyDescVer:3;
    UCHAR	Rsvd:3;
    UCHAR	EKD_DL:1;		/* EKD for AP; DL for STA */
    UCHAR	Request:1;
    UCHAR	Error:1;
    UCHAR	Secure:1;
    UCHAR	KeyMic:1;
#else
	UCHAR	KeyMic:1;
	UCHAR	Secure:1;
	UCHAR	Error:1;
	UCHAR	Request:1;
	UCHAR	EKD_DL:1;       /* EKD for AP; DL for STA */
	UCHAR	Rsvd:3;
	UCHAR	KeyDescVer:3;
	UCHAR	KeyType:1;
	UCHAR	KeyIndex:2;
	UCHAR	Install:1;
	UCHAR	KeyAck:1;
#endif	
}	KEY_INFO, *PKEY_INFO;

/* EAPOL Key descriptor format */
typedef	struct GNU_PACKED _KEY_DESCRIPTER
{
	UCHAR		Type;
	KEY_INFO	KeyInfo;
	UCHAR		KeyLength[2];
	UCHAR		ReplayCounter[LEN_KEY_DESC_REPLAY];
	UCHAR		KeyNonce[LEN_KEY_DESC_NONCE];
	UCHAR		KeyIv[LEN_KEY_DESC_IV];
	UCHAR		KeyRsc[LEN_KEY_DESC_RSC];
	UCHAR		KeyId[LEN_KEY_DESC_ID];
	UCHAR		KeyMic[LEN_KEY_DESC_MIC];
	UCHAR		KeyDataLen[2];	   
	UCHAR		KeyData[];
}	KEY_DESCRIPTER, *PKEY_DESCRIPTER;

typedef	struct GNU_PACKED _EAPOL_PACKET
{
	UCHAR	 			ProVer;
	UCHAR	 			ProType;
	UCHAR	 			Body_Len[2];
	KEY_DESCRIPTER		KeyDesc;
}	EAPOL_PACKET, *PEAPOL_PACKET;

typedef struct GNU_PACKED _KDE_HDR
{
    UCHAR               Type;
    UCHAR               Len;
    UCHAR               OUI[3];
    UCHAR               DataType;
	UCHAR				octet[];
}   KDE_HDR, *PKDE_HDR;

/*802.11i D10 page 83 */
typedef struct GNU_PACKED _GTK_KDE
{
#ifndef RT_BIG_ENDIAN
    UCHAR               Kid:2;
    UCHAR               tx:1;
    UCHAR               rsv:5;
    UCHAR               rsv1;
#else
    UCHAR               rsv:5;
    UCHAR               tx:1;
    UCHAR               Kid:2;
    UCHAR               rsv1;    	
#endif
    UCHAR               GTK[];
}   GTK_KDE, *PGTK_KDE;

/* For WPA1 */
typedef struct GNU_PACKED _RSNIE {
    UCHAR   oui[4];
    USHORT  version;
    UCHAR   mcast[4];
    USHORT  ucount;
    struct GNU_PACKED {
        UCHAR oui[4];
    } ucast[1];
} RSNIE, *PRSNIE;

/* For WPA2 */
typedef struct GNU_PACKED _RSNIE2 {
    USHORT  version;
    UCHAR   mcast[4];
    USHORT  ucount;
    struct GNU_PACKED {
        UCHAR oui[4];
    } ucast[1];
} RSNIE2, *PRSNIE2;

/* AKM Suite */
typedef struct GNU_PACKED _RSNIE_AUTH {
    USHORT acount;
    struct GNU_PACKED {
        UCHAR oui[4];
    }auth[1];
} RSNIE_AUTH,*PRSNIE_AUTH;

/* PMKID List */
typedef struct GNU_PACKED _RSNIE_PMKID {
    USHORT pcount;
    struct GNU_PACKED {
        UCHAR list[16];
    }pmkid[1];
} RSNIE_PMKID,*PRSNIE_PMKID;

typedef	union GNU_PACKED _RSN_CAPABILITIES	{
	struct	GNU_PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT	Rsvd:8;		
	USHORT	MFPC:1;
	USHORT	MFPR:1;
    USHORT	GTKSA_R_Counter:2;
    USHORT	PTKSA_R_Counter:2;
    USHORT	No_Pairwise:1;
	USHORT	PreAuth:1;
#else
    USHORT	PreAuth:1;
	USHORT	No_Pairwise:1;
	USHORT	PTKSA_R_Counter:2;
	USHORT	GTKSA_R_Counter:2;
	USHORT	MFPR:1;
	USHORT	MFPC:1;
	USHORT	Rsvd:8;
#endif
	} field;
	USHORT			word;
}	RSN_CAPABILITIES, *PRSN_CAPABILITIES;

typedef struct GNU_PACKED _EAP_HDR {
    UCHAR   ProVer;
    UCHAR   ProType;
    UCHAR   Body_Len[2];
    UCHAR   code;
    UCHAR   identifier;
    UCHAR   length[2]; /* including code and identifier, followed by length-2 octets of data */
} EAP_HDR, *PEAP_HDR;

#if 0 // TKIP unused code.
/* */
/* Tkip Key structure which RC4 key & MIC calculation */
/* */
typedef struct _TKIP_KEY_INFO {
	UINT nBytesInM;		/* # bytes in M for MICKEY */
	ULONG IV16;
	ULONG IV32;
	ULONG K0;		/* for MICKEY Low */
	ULONG K1;		/* for MICKEY Hig */
	ULONG L;		/* Current state for MICKEY */
	ULONG R;		/* Current state for MICKEY */
	ULONG M;		/* Message accumulator for MICKEY */
	UCHAR RC4KEY[16];
	UCHAR MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;

/* */
/* Private / Misc data, counters for driver internal use */
/* */
typedef struct __PRIVATE_STRUC {
	UINT SystemResetCnt;	/* System reset counter */
	UINT TxRingFullCnt;	/* Tx ring full occurrance number */
	UINT PhyRxErrCnt;	/* PHY Rx error count, for debug purpose, might move to global counter */
	/* Variables for WEP encryption / decryption in rtmp_wep.c */
	/* Tkip stuff */
	TKIP_KEY_INFO Tx;
	TKIP_KEY_INFO Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;
#endif

/*
	Packet information for NdisQueryPacket
*/
typedef struct  _PACKET_INFO    {
	UINT PhysicalBufferCount;    /* Physical breaks of buffer descripor chained */
	UINT BufferCount;           /* Number of Buffer descriptor chained */
	UINT TotalPacketLength ;     /* Self explained */
	PNDIS_BUFFER pFirstBuffer;   /* Pointer to first buffer descriptor */
#ifdef TX_PKT_SG
	PKT_SG_T sg_list[MAX_SKB_FRAGS];
#endif /* TX_PKT_SG */
} PACKET_INFO, *PPACKET_INFO;

/* structure to define WPA Group Key Rekey Interval */
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;	/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG ReKeyInterval;	/* time-based: seconds, packet-based: kilo-packets */
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;


VOID Wpa_SupplicantTask(void *param);


VOID WpaApStartup(
	IN PRTMP_ADAPTER    pAd);


VOID WpaApStop(
	IN PRTMP_ADAPTER    pAd);


VOID WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd,
    IN PMGMTENTRY 		pEntry);


VOID HandleCounterMeasure(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY 		pEntry);

INT RT_CfgSetWPAPSKKey(
	IN PRTMP_ADAPTER	pAd, 
	IN PSTRING			keyString,
	IN INT				keyStringLen,
	IN PUCHAR			pHashStr,
	IN INT				hashStrLen,
	OUT PUCHAR			pPMKBuf);

VOID RTMPMakeRSNIE(
    IN  PRTMP_ADAPTER   pAd,
    IN  UINT            AuthMode,
    IN  UINT            WepStatus,
	IN	UCHAR			apidx);

VOID GenRandom(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR			*macAddr,
	OUT	UCHAR			*random);

VOID WPARetryExec(
    IN PVOID FunctionContext);

VOID CMTimerExec(
    IN PVOID FunctionContext);

VOID WpaGREKEYPeriodicExec(
    IN PVOID FunctionContext);

VOID hex_dump(
	IN PUCHAR 	title, 
	IN PUCHAR 	value, 
	IN int 		size);

#if 0 // TKIP unused code..
BOOLEAN	RTMPTkipCompareMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pSrc,
	IN	PUCHAR			pDA,
	IN	PUCHAR			pSA,
	IN	PUCHAR			pMICKey,
	IN	UCHAR			UserPriority,
	IN	UINT			Len);
#endif

VOID TKIP_GTK_KEY_WRAP( 
    IN UCHAR    *key,
    IN UCHAR	*iv,
    IN UCHAR    *input_text,
    IN UINT32    input_len,
    OUT UCHAR   *output_text);

VOID TKIP_GTK_KEY_UNWRAP( 
    IN UCHAR    *key,
    IN UCHAR	*iv,
    IN UCHAR    *input_text,
    IN UINT32    input_len,
    OUT UCHAR   *output_text);

VOID WpaDeriveGTK(
    IN  UCHAR   *GMK,
    IN  UCHAR   *GNonce,
    IN  UCHAR   *AA,
    OUT UCHAR   *output,
    IN  UINT    len);

VOID WPAInstallSharedKey(
	PRTMP_ADAPTER		pAd,
	UINT8				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	BOOLEAN				bAE,
	PUINT8				pGtk,
	UINT8				GtkLen);
#endif
