#ifndef __NL80211_HDR_H__
#define __NL80211_HDR_H__

#include "system.h"
#include "types.h"

#define MAC_ADDR_LEN					6

#define MODE_CCK						0
#define MODE_OFDM   					1
#define MODE_HTMIX						2

#define MCS_RATE_1                  	0
#define MCS_RATE_2                      1
#define MCS_RATE_5_5                    2
#define MCS_RATE_11                     3
/* To send duplicate legacy OFDM. set BW=BW_40.  SGI.STBC are reserved */
#define MCS_RATE_6                      0	/* legacy OFDM */
#define MCS_RATE_9                      1	/* OFDM */
#define MCS_RATE_12                     2	/* OFDM */
#define MCS_RATE_18                     3	/* OFDM */
#define MCS_RATE_24                     4	/* OFDM */
#define MCS_RATE_36                     5	/* OFDM */
#define MCS_RATE_48                     6	/* OFDM */
#define MCS_RATE_54                     7	/* OFDM */

#define	PKT_BEACON						0x80
#define	PKT_PROBE_REQ					0x40
#define PKT_PROBE_RESP					0x50
#define PKT_AUTH						0xB0
#define PKT_AUTH_DEATH					0xC0
#define PKT_ASSOC_REQ					0x00
#define PKT_ASSOC_RSP					0x10
#define PKT_ASSOC_DIS					0xA0

#define BTYPE_MGMT                  	0
#define BTYPE_CNTL                  	1
#define BTYPE_DATA                  	2

/* BTYPE_DATA sub-member */
#define SUBTYPE_DATA                	0
#define SUBTYPE_DATA_CFACK          	1
#define SUBTYPE_DATA_CFPOLL         	2
#define SUBTYPE_DATA_CFACK_CFPOLL   	3
#define SUBTYPE_NULL_FUNC           	4
#define SUBTYPE_CFACK               	5
#define SUBTYPE_CFPOLL              	6
#define SUBTYPE_CFACK_CFPOLL        	7
#define SUBTYPE_QDATA               	8
#define SUBTYPE_QDATA_CFACK         	9
#define SUBTYPE_QDATA_CFPOLL        	10
#define SUBTYPE_QDATA_CFACK_CFPOLL  	11
#define SUBTYPE_QOS_NULL            	12
#define SUBTYPE_QOS_CFACK           	13
#define SUBTYPE_QOS_CFPOLL          	14
#define SUBTYPE_QOS_CFACK_CFPOLL    	15

/* BTYPE_MGMT sub-member */
#define SUBTYPE_ASSOC_REQ           	0
#define SUBTYPE_ASSOC_RSP           	1
#define SUBTYPE_REASSOC_REQ         	2
#define SUBTYPE_REASSOC_RSP         	3
#define SUBTYPE_PROBE_REQ           	4
#define SUBTYPE_PROBE_RSP           	5
#define SUBTYPE_BEACON              	8
#define SUBTYPE_ATIM                	9
#define SUBTYPE_DISASSOC            	10
#define SUBTYPE_AUTH                	11
#define SUBTYPE_DEAUTH              	12
#define SUBTYPE_ACTION              	13
#define SUBTYPE_ACTION_NO_ACK       	14


typedef enum _RT_802_11_PREAMBLE {
	Rt802_11PreambleLong,
	Rt802_11PreambleShort,
	Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;


typedef struct  _AUTH_IE {
  	USHORT  	auth_alg;
	USHORT		auth_seq;
	USHORT		auth_status;
} AUTH_IE, *pAUTH_IE;


typedef struct GNU_PACKED {	
#ifdef BIG_ENDIAN
	USHORT 	Order:1;	/* Strict order expected */
	USHORT 	Wep:1;		/* Wep data */
	USHORT 	MoreData:1;	/* More data bit */
	USHORT 	PwrMgmt:1;	/* Power management bit */
	USHORT 	Retry:1;	/* Retry status bit */
	USHORT 	MoreFrag:1;	/* More fragment bit */
	USHORT 	FrDs:1;		/* From DS indication */
	USHORT 	ToDs:1;		/* To DS indication */
	USHORT 	SubType:4;	/* MSDU subtype */
	USHORT 	Type:2;		/* MSDU type */
	USHORT 	Ver:2;		/* Protocol version */
#else
	USHORT 	Ver:2;		/* Protocol version */
	USHORT 	Type:2;		/* MSDU type */
	USHORT 	SubType:4;	/* MSDU subtype */
	USHORT 	ToDs:1;		/* To DS indication */
	USHORT 	FrDs:1;		/* From DS indication */
	USHORT 	MoreFrag:1;	/* More fragment bit */
	USHORT 	Retry:1;	/* Retry status bit */
	USHORT 	PwrMgmt:1;	/* Power management bit */
	USHORT 	MoreData:1;	/* More data bit */
	USHORT 	Wep:1;		/* Wep data */
	USHORT 	Order:1;	/* Strict order expected */
#endif	/* !BIG_ENDIAN */
}FRAME_CONTROL, *PFRAME_CONTROL;


typedef struct  GNU_PACKED _HEADER_802_11 {
	FRAME_CONTROL   	FC;
	USHORT          	Duration;
	UCHAR           	Addr1[MAC_ADDR_LEN];
	UCHAR           	Addr2[MAC_ADDR_LEN];
	UCHAR 				Addr3[MAC_ADDR_LEN];
#ifdef BIG_ENDIAN
	USHORT				Sequence:12;
	USHORT				Frag:4;
#else
	USHORT				Frag:4;
	USHORT				Sequence:12;
#endif /* !BIG_ENDIAN */
	//UCHAR			Octet[1];
} HEADER_802_11, *PHEADER_802_11;


#endif /* __NL80211_HDR_H__ */
