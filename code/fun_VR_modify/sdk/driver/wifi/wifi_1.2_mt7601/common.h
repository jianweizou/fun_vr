#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>

#define SNX_AP_TITLE						"SNC7312"

#define DEFAULT_APSSID						"SN98600AP"
#define DEFAULT_APPASSWORD 					"12345"

#define DEFAULT_CHANNEL 					1
#define	DEFAULT_BCNPERIOD					100

#define TEMP_RA_WIFI_MARKER					"RT5370DN"
#define TEMP_RA_WIFI_MARKER_LEN				8
#define TEMP_MT_WIFI_MARKER					"MT7601"
#define TEMP_MT_WIFI_MARKER_LEN				4

#define SHARE_KEY_NUM 						4
#define MAX_LEN_OF_SHARE_KEY      			16		/* byte count */
#define MAX_LEN_OF_BSS_TABLE 				1
#define MAX_VIE_LEN                     	1024	/* New for WPA cipher suite variable IE sizes. */
#define MAX_CUSTOM_LEN 						128 
#define MAX_LEN_OF_MLME_QUEUE       		1		//10// 40 /*10 */
#define MGMT_DMA_BUFFER_SIZE    			600		/*2048 */
#define HW_BEACON_MAX_NUM					1
#define MAX_NUM_OF_CHANNELS					20
#define NUM_EEPROM_BBP_PARMS				19		/* Include NIC Config 0, 1, CR, TX ALC step, BBPs */
#define MAX_TXPOWER_ARRAY_SIZE				5
#define MAX_TX_RING 						1 		/* 2 */
#define MAX_CLIENT_NUM 						5
#define MAX_LEN_OF_SUPPORTED_RATES    		12		/* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
#define MAX_LEN_OF_MAC_TABLE				32
#define MAX_LEN_OF_SSID						32
#define TIME_STAMP_LEN						8
#define INTERVAL_LEN						2
#define CAPABILITY_INFO_LEN					2
#define CIPHER_TEXT_LEN 					128
#define MAX_LEN_OF_PEER_KEY             	16	/* byte count */
#define MAX_CIPHER_ERR_CNT					5
#define LOCAL_TXBUF_SIZE        			1600	/* 2048 */
#define BULKAGGRE_SIZE          			60
#define RXBULKAGGRE_SIZE					12
#define MAX_TXBULK_LIMIT					(LOCAL_TXBUF_SIZE * (BULKAGGRE_SIZE - 1))
#define MAX_TXBULK_SIZE						(LOCAL_TXBUF_SIZE * BULKAGGRE_SIZE)
#define MAX_RXBULK_SIZE						(LOCAL_TXBUF_SIZE * RXBULKAGGRE_SIZE)

#define MAX_TYPE_PROTECT 					5


#define LEN_WEP128_KEY						13 
#define LEN_WEP64_KEY						5
#define	MAXSEQ								(0xFFF)
#define DEFAULT_SCAN_PERIOD					500
#define END_OF_ARGS                			(-1)


/* Ethernet type */
#define ETHTYPE_EAPOL 						0x888E

/*----- System parameters -----*/
/* Wifi operation mode */
#define OPMODE_STA							0
#define OPMODE_AP							1
#define OPMODE_APSTA						2       /* as AP and STA at the same time */

/* Debug level definition	*/
#define RT_OFF        						0
#define RT_ERROR      						1
#define RT_INFO 							2
#define RT_WARN       						3
#define RT_TRACE      						4
#define RT_LOUD       						5
#define RTDebugLevel						RT_WARN

#define MCAST_WCID							0x1
#define BSS0								0

/* The security mode definition in MAC register */
#define CIPHER_NONE                 		0
#define CIPHER_WEP64                		1
#define CIPHER_WEP128               		2
#define CIPHER_TKIP                 		3
#define CIPHER_AES                  		4
#define CIPHER_CKIP64               		5
#define CIPHER_CKIP128              		6
#define CIPHER_CKIP152          			7
#define CIPHER_SMS4							8


/* Control status */
#define NDIS_SUCCESS						0x00
#define NDIS_FAILURE						0x01
#define NDIS_PLUG_OUT						0x04

/* Auth and Assoc mode related definitions */
#define AUTH_MODE_OPEN                  	0x00
#define AUTH_MODE_KEY                   	0x01

/* TXWI control parameters */
#define FIFO_MGMT                 			0
#define FIFO_HCCA                 			1
#define FIFO_EDCA                 			2

/* TXWI control parameters */
#define PID_MGMT							0x05
#define PID_BEACON							0x0c
#define PID_DATA_NORMALUCAST				0x02
#define PID_DATA_AMPDU	 					0x04
#define PID_DATA_NO_ACK    					0x08
#define PID_DATA_NOT_NORM_ACK	 			0x03

/* pTxWI->txop */
#define IFS_HTTXOP                 			0	/* The txop will be handles by ASIC. */
#define IFS_PIFS                    		1
#define IFS_SIFS                    		2
#define IFS_BACKOFF                 		3

/* value domain of pTxD->HostQId (4-bit: 0~15) */
#define QID_AC_BK               			1   /* meet ACI definition in 802.11e */
#define QID_AC_BE               			0   /* meet ACI definition in 802.11e */
#define QID_AC_VI               			2
#define QID_AC_VO               			3
#define QID_HCCA                			4
#define NUM_OF_TX_RING         	 			5
#define QID_MGMT                			13
#define QID_RX                  			14
#define QID_OTHER               			15


/* IE code */
#define IE_SSID                         	0
#define IE_SUPP_RATES                   	1
#define IE_FH_PARM                      	2
#define IE_DS_PARM                      	3
#define IE_CF_PARM                      	4
#define IE_TIM                          	5
#define IE_IBSS_PARM                    	6
#define IE_COUNTRY                      	7	/* 802.11d */
#define IE_802_11D_REQUEST              	10	/* 802.11d */
#define IE_QBSS_LOAD                    	11	/* 802.11e d9 */
#define IE_EDCA_PARAMETER               	12	/* 802.11e d9 */
#define IE_TSPEC                        	13	/* 802.11e d9 */
#define IE_TCLAS                        	14	/* 802.11e d9 */
#define IE_SCHEDULE                     	15	/* 802.11e d9 */
#define IE_CHALLENGE_TEXT               	16
#define IE_POWER_CONSTRAINT             	32	/* 802.11h d3.3 */
#define IE_POWER_CAPABILITY             	33	/* 802.11h d3.3 */
#define IE_TPC_REQUEST                  	34	/* 802.11h d3.3 */
#define IE_TPC_REPORT                  	 	35	/* 802.11h d3.3 */
#define IE_SUPP_CHANNELS                	36	/* 802.11h d3.3 */
#define IE_CHANNEL_SWITCH_ANNOUNCEMENT  	37	/* 802.11h d3.3 */
#define IE_MEASUREMENT_REQUEST          	38	/* 802.11h d3.3 */
#define IE_MEASUREMENT_REPORT           	39	/* 802.11h d3.3 */
#define IE_QUIET                        	40	/* 802.11h d3.3 */
#define IE_IBSS_DFS                     	41	/* 802.11h d3.3 */
#define IE_ERP                          	42	/* 802.11g */
#define IE_TS_DELAY                     	43	/* 802.11e d9 */
#define IE_TCLAS_PROCESSING             	44	/* 802.11e d9 */
#define IE_QOS_CAPABILITY               	46	/* 802.11e d6 */
#define IE_HT_CAP                       	45	/* 802.11n d1. HT CAPABILITY. ELEMENT ID TBD */
#define IE_AP_CHANNEL_REPORT				51	/* 802.11k d6 */
#define IE_HT_CAP2                      	52	/* 802.11n d1. HT CAPABILITY. ELEMENT ID TBD */
#define IE_RSN                          	48	/* 802.11i d3.0 */
#define IE_WPA2                         	48	/* WPA2 */
#define IE_EXT_SUPP_RATES               	50	/* 802.11g */
#define IE_SUPP_REG_CLASS               	59	/* 802.11y. Supported regulatory classes. */
#define IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT	60	/* 802.11n */
#define IE_ADD_HT                       	61	/* 802.11n d1. ADDITIONAL HT CAPABILITY. ELEMENT ID TBD */
#define IE_ADD_HT2                      	53	/* 802.11n d1. ADDITIONAL HT CAPABILITY. ELEMENT ID TBD */
#define IE_EXT_CAPABILITY               	127	/* 802.11n D3.03 */

/* Reason code definitions */
#define REASON_RESERVED                 	0
#define REASON_UNSPECIFY                	1
#define REASON_NO_LONGER_VALID          	2
#define REASON_DEAUTH_STA_LEAVING       	3
#define REASON_DISASSOC_INACTIVE        	4
#define REASON_DISASSPC_AP_UNABLE       	5
#define REASON_CLS2ERR                  	6
#define REASON_CLS3ERR                  	7
#define REASON_DISASSOC_STA_LEAVING     	8
#define REASON_STA_REQ_ASSOC_NOT_AUTH   	9
#define REASON_INVALID_IE               	13
#define REASON_MIC_FAILURE              	14
#define REASON_4_WAY_TIMEOUT            	15
#define REASON_GROUP_KEY_HS_TIMEOUT     	16
#define REASON_IE_DIFFERENT             	17
#define REASON_MCIPHER_NOT_VALID        	18
#define REASON_UCIPHER_NOT_VALID        	19
#define REASON_AKMP_NOT_VALID           	20
#define REASON_UNSUPPORT_RSNE_VER       	21
#define REASON_INVALID_RSNE_CAP         	22
#define REASON_8021X_AUTH_FAIL          	23
#define REASON_CIPHER_SUITE_REJECTED    	24
#define REASON_DECLINED                 	37
/* Qos reason */
#define REASON_QOS_UNSPECIFY              	32
#define REASON_QOS_LACK_BANDWIDTH         	33
#define REASON_POOR_CHANNEL_CONDITION     	34
#define REASON_QOS_OUTSIDE_TXOP_LIMITION  	35
#define REASON_QOS_QSTA_LEAVING_QBSS      	36
#define REASON_QOS_UNWANTED_MECHANISM     	37
#define REASON_QOS_MECH_SETUP_REQUIRED    	38
#define REASON_QOS_REQUEST_TIMEOUT        	39
#define REASON_QOS_CIPHER_NOT_SUPPORT     	45


/* Status code definitions */
#define MLME_SUCCESS                    	0
#define MLME_UNSPECIFY_FAIL             	1
#define MLME_CANNOT_SUPPORT_CAP         	10
#define MLME_REASSOC_DENY_ASSOC_EXIST   	11
#define MLME_ASSOC_DENY_OUT_SCOPE       	12
#define MLME_ALG_NOT_SUPPORT            	13
#define MLME_SEQ_NR_OUT_OF_SEQUENCE     	14
#define MLME_REJ_CHALLENGE_FAILURE      	15
#define MLME_REJ_TIMEOUT                  	16
#define MLME_ASSOC_REJ_UNABLE_HANDLE_STA  	17
#define MLME_ASSOC_REJ_DATA_RATE          	18
#define MLME_ASSOC_REJ_NO_EXT_RATE        	22
#define MLME_ASSOC_REJ_NO_EXT_RATE_PBCC   	23
#define MLME_ASSOC_REJ_NO_CCK_OFDM        	24
#define MLME_QOS_UNSPECIFY                	32
#define MLME_REQUEST_DECLINED             	37
#define MLME_REQUEST_WITH_INVALID_PARAM   	38
#define MLME_INVALID_INFORMATION_ELEMENT  	40
#define MLME_INVALID_GROUP_CIPHER   	  	41
#define MLME_INVALID_PAIRWISE_CIPHER   	  	42
#define MLME_INVALID_AKMP   			  	43
#define MLME_NOT_SUPPORT_RSN_VERSION	  	44
#define	MLME_INVALID_RSN_CAPABILITIES	  	45
#define MLME_INVALID_SECURITY_POLICY      	46 /* Cipher suite rejected because of security policy */
#define MLME_DLS_NOT_ALLOW_IN_QBSS        	48
#define MLME_DEST_STA_NOT_IN_QBSS         	49
#define MLME_DEST_STA_IS_NOT_A_QSTA       	50
#define MLME_INVALID_FORMAT             	0x51
#define MLME_FAIL_NO_RESOURCE           	0x52
#define MLME_STATE_MACHINE_REJECT       	0x53
#define MLME_MAC_TABLE_FAIL             	0x54

/* SHORTGI */
#define GI_800								0
#define GI_400								1	/* only support in HT mode */
#define GI_BOTH								2

/* BANDWIDTH */
#define BW_20								0
#define BW_40								1
#define BW_BOTH								2
#define BW_10								3  /* 802.11j has 10MHz. This definition is for internal usage. doesn't fill in the IE or other field. */

/* STBC */
#define STBC_NONE							0
#define STBC_USE							1	/* limited use in rt2860b phy */

/* PHY rate */
#define RATE_1                      		0
#define RATE_2                      		1
#define RATE_5_5                    		2
#define RATE_11                     		3
#define RATE_6                      		4	/* OFDM */
#define RATE_9                      		5	/* OFDM */
#define RATE_12                     		6	/* OFDM */
#define RATE_18                     		7	/* OFDM */
#define RATE_24                     		8	/* OFDM */
#define RATE_36                     		9	/* OFDM */
#define RATE_48                     		10	/* OFDM */
#define RATE_54                     		11	/* OFDM */
#define RATE_6_5                    		12	/* HT mix */
#define RATE_13                     		13	/* HT mix */
#define RATE_19_5                   		14	/* HT mix */
#define RATE_26                     		15	/* HT mix */
#define RATE_39                     		16	/* HT mix */
#define RATE_52                     		17	/* HT mix */
#define RATE_58_5                   		18	/* HT mix */
#define RATE_65                     		19	/* HT mix */
#define RATE_78                     		20	/* HT mix */
#define RATE_104                    		21	/* HT mix */
#define RATE_117                    		22	/* HT mix */
#define RATE_130                    		23	/* HT mix */

/* MIMO power safe */
#define	MMPS_STATIC							0
#define	MMPS_DYNAMIC						1
#define MMPS_RSV							2
#define MMPS_ENABLE							3

/* RF section */
#define RF_BANK0							0
#define RF_BANK1							1
#define RF_BANK2							2
#define RF_BANK3							3
#define RF_BANK4							4
#define RF_BANK5							5
#define RF_BANK6							6
#define RF_BANK7							7
#define RF_BANK8							8
#define RF_BANK9							9
#define RF_BANK10							10
#define RF_BANK11							11
#define RF_BANK12							12
#define RF_BANK13							13
#define RF_BANK14							14
#define RF_BANK15							15

#define RF_R00								0
#define RF_R01								1
#define RF_R02								2
#define RF_R03								3
#define RF_R04								4
#define RF_R05								5
#define RF_R06								6
#define RF_R07								7
#define RF_R08								8
#define RF_R09								9
#define RF_R10								10
#define RF_R11								11
#define RF_R12								12
#define RF_R13								13
#define RF_R14								14
#define RF_R15								15
#define RF_R16								16
#define RF_R17								17
#define RF_R18								18
#define RF_R19								19
#define RF_R20								20
#define RF_R21								21
#define RF_R22								22
#define RF_R23								23
#define RF_R24								24
#define RF_R25								25
#define RF_R26								26
#define RF_R27								27
#define RF_R28								28
#define RF_R29								29
#define RF_R30								30
#define RF_R31								31
#define	RF_R32					    		32
#define	RF_R33					    		33
#define	RF_R34					    		34
#define	RF_R35					    		35
#define	RF_R36					    		36
#define	RF_R37					    		37
#define	RF_R38					    		38
#define	RF_R39					    		39
#define	RF_R40					    		40
#define	RF_R41					    		41
#define	RF_R42					    		42
#define	RF_R43					    		43
#define	RF_R44					    		44
#define	RF_R45					    		45
#define	RF_R46					    		46
#define	RF_R47					    		47
#define	RF_R48					    		48
#define	RF_R49					    		49
#define	RF_R50					    		50
#define	RF_R51					    		51
#define	RF_R52					    		52
#define	RF_R53					    		53
#define	RF_R54					    		54
#define	RF_R55					    		55
#define	RF_R56					    		56
#define	RF_R57					    		57
#define	RF_R58					    		58
#define	RF_R59					    		59
#define	RF_R60					    		60
#define	RF_R61					    		61
#define	RF_R62					    		62
#define	RF_R63					    		63


/* EXTCHA */
#define EXTCHA_NONE							0
#define EXTCHA_ABOVE						0x1
#define EXTCHA_BELOW						0x3


/* HT mode */
#define HTMODE_MM							0
#define HTMODE_GF							1


/* HT */
#define MCS_0								0	/* 1S */
#define MCS_1								1
#define MCS_2								2
#define MCS_3								3
#define MCS_4								4
#define MCS_5								5
#define MCS_6								6
#define MCS_7								7
#define MCS_8								8	/* 2S */
#define MCS_9								9
#define MCS_10								10
#define MCS_11								11
#define MCS_12								12
#define MCS_13								13
#define MCS_14								14
#define MCS_15								15
#define MCS_16								16	/* 3*3 */
#define MCS_17								17
#define MCS_18								18
#define MCS_19								19
#define MCS_20								20
#define MCS_21								21
#define MCS_22								22
#define MCS_23								23
#define MCS_32								32
#define MCS_AUTO							33


#define NO_LINK_FILTER	 					0x1FF97
#define LINK_FILTER	 						0x1FF9F
#define SCAN_FILTER	 						0x1FF93
#define STANORMAL							0x17F97
#define APNORMAL	 						0x15F97


#define SNX_MULTICASE_PKT_SIGN1				0x01
#define SNX_MULTICASE_PKT_SIGN2				0x00
#define SNX_MULTICASE_PKT_SIGN3				0x5E


/* ------------------------------------------------------ */
/* BBP & RF	definition */
/* ------------------------------------------------------ */
#define	BUSY		                		1
#define	IDLE		                		0


enum WIFI_MODE{
	WMODE_INVALID 	= 0,
	WMODE_A 		= 1 << 0,
	WMODE_B 		= 1 << 1,
	WMODE_G 		= 1 << 2,
	WMODE_GN 		= 1 << 3,
	WMODE_AN 		= 1 << 4,
	WMODE_AC 		= 1 << 5,
	WMODE_COMP 		= 6,	/* total types of supported wireless mode, add this value once yow add new type */
};


//; smart config
typedef enum{
	SmartConfigOff = 0x66,
	SmartConfigStart,
	SmartConfigCheckBeacon,
	SmartConfigRestart,
	SmartConfigFail,
} SM_CFIG;


//; station mode state flow
typedef enum {
	LS_NO_LINK,
	LS_SCAN,
	LS_SCAN_LINK,
	LS_SCAN_BSSID,
	LS_START_LINK,
	LS_BEACON_AUTH,
	LS_AUTH_ASSOCIATE,
	LS_LINKED
} LINK_STATE;

//; NVRAM wifi related information
typedef enum
{
/* AP mode configuration */
	WIFI_AP_CHANNEL_INFO,
	WIFI_AP_SSID_INFO,
	WIFI_AP_SSID_LEN,
	WIFI_AP_AUTH_MODE,						
	WIFI_AP_ENC_MODE,
	WIFI_AP_KEY_INFO,
	WIFI_AP_KEY_LEN,
	WIFI_AP_SSID_PREFIX,				
	WIFI_AP_SSID_PREFIX_LEN,			
	WIFI_AP_MAC_NUM,					
/* Device configuration */
	WIFI_DEV_CHANNEL_INFO,
	WIFI_DEV_SSID_INFO,
	WIFI_DEV_SSID_LEN,
	WIFI_DEV_AUTH_MODE,						
	WIFI_DEV_ENC_MODE,
	WIFI_DEV_KEY_INFO,
	WIFI_DEV_KEY_LEN,
	WIFI_DEV_BSSID_INFO,
	WIFI_DEV_WPA_PSK,	
}SNX_DEV_INFO_ID;

#define HW_BEACON_BASE0(__pAd) 		((__pAd)->BeaconOffset[0])

#define HW_RESERVED_WCID(__pAd)		((__pAd)->chipCap.WcidHwRsvNum)

/* Then dedicate wcid of DFS and Carrier-Sense. */
#define DFS_CTS_WCID(__pAd) 		(HW_RESERVED_WCID(__pAd) - 1)
#define CS_CTS_WCID(__pAd) 			(HW_RESERVED_WCID(__pAd) - 2)
#define LAST_SPECIFIC_WCID(__pAd)	(HW_RESERVED_WCID(__pAd) - 2)

/* If MAX_MBSSID_NUM is 8, the maximum available wcid for the associated STA is 211. */
/* If MAX_MBSSID_NUM is 7, the maximum available wcid for the associated STA is 228. */
#define MAX_AVAILABLE_CLIENT_WCID(__pAd)	(LAST_SPECIFIC_WCID(__pAd) - MAX_MBSSID_NUM(__pAd) - 1)

/* TX need WCID to find Cipher Key */
/* these wcid 212 ~ 219 are reserved for bc/mc packets if MAX_MBSSID_NUM is 8. */
#define GET_GroupKey_WCID(__pAd, __wcid, __bssidx) 									\
	{																				\
		__wcid = LAST_SPECIFIC_WCID(__pAd) - (MAX_MBSSID_NUM(__pAd)) + __bssidx;	\
	}

#define COPY_MAC_ADDR(_d, _a)				memcpy(_d, _a, 6)
#define MAC_ADDR_EQUAL(a, b)				((memcmp(a, b, 6)==0)? 1:0)
#define MATCH_MULTI(_a) 					((_a[0] == SNX_MULTICASE_PKT_SIGN1) && (_a[1] == SNX_MULTICASE_PKT_SIGN2) && (_a[2] == SNX_MULTICASE_PKT_SIGN3))

#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define RTMPusecDelay(L)	(L >= 1000)? (vTaskDelay((L/1000)/portTICK_RATE_MS)) : (vTaskDelay(10/portTICK_RATE_MS))

#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define CAP_GENERATE(ess,ibss,priv,s_pre,s_slot,spectrum)  (((ess) ? 0x0001 : 0x0000) | ((ibss) ? 0x0002 : 0x0000) | ((priv) ? 0x0010 : 0x0000) | ((s_pre) ? 0x0020 : 0x0000) | ((s_slot) ? 0x0400 : 0x0000) | ((spectrum) ? 0x0100 : 0x0000))

#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)		if (_pAd->OpMode == OPMODE_AP)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)		if (_pAd->OpMode == OPMODE_STA)

#define LLCH_SET(dst)	if(dst) { dst[0]  = dst[1]= 0xAA; \
						dst[2] = 0x03; \
						dst[3] = dst[4] = dst[5] = 0x0;}



#if 0
#define DBGPRINT(Level, Fmt)
#else                        
#define DBGPRINT(Level, Fmt) 			\
	do {                               	\
    	if (Level <= RTDebugLevel)      \
    	{                               \
        	printf Fmt;          	\
    	}                               \
	 } while (0)
#endif

#endif /* __COMMON_H__ */
