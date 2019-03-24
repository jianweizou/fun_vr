#ifndef __RTMP_IF_H__
#define __RTMP_IF_H__

#include "types.h"

/* Pre-declartion */
typedef struct _RTMP_ADAPTER 	RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER *	PRTMP_ADAPTER;

typedef struct _MGMTENTRY		MGMTENTRY;
typedef struct _MGMTENTRY	*	PMGMTENTRY;


/* WLAN_802_11_NETWORK_INFRASTRUCTURE */
typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
    Ndis802_11IBSS,
    Ndis802_11Infrastructure,
    Ndis802_11AutoUnknown,
    /*defined as upper bound */
    Ndis802_11InfrastructureMax
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;


/* WLAN_802_11_ENCRYPTION_MODE */
typedef enum _NDIS_802_11_ENCRYPTION_MODE
{
    Ndis802_11Cipher_NONE,
    Ndis802_11Cipher_WEP40,
    Ndis802_11Cipher_TKIP,
    Ndis802_11Cipher_CCMP,
    Ndis802_11Cipher_WEP104,
    Ndis802_11Cipher_UNKNOWN
} NDIS_802_11_ENCRYPTION_MODE;

/* NVRAM  Data Mapping to Driver Auth Mode */
typedef enum _NVRAM_AUTH_MODE {
	OPEN,
	WEP,
	WPA,
	WPA2,
} NVRAM_AUTH_MODE;

/* WLAN_802_11_AUTHENTICATION_MODE */
typedef enum _NDIS_802_11_AUTHENTICATION_MODE {
	Ndis802_11AuthModeOpen,
	Ndis802_11AuthModeShared,
	Ndis802_11AuthModeAutoSwitch,
	Ndis802_11AuthModeWPA,
	Ndis802_11AuthModeWPAPSK,
	Ndis802_11AuthModeWPANone,
	Ndis802_11AuthModeWPA2,
	Ndis802_11AuthModeWPA2PSK,
	Ndis802_11AuthModeWPA1WPA2,
	Ndis802_11AuthModeWPA1PSKWPA2PSK,
	Ndis802_11AuthModeMax	/* Not a real mode, defined as upper bound */
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;


typedef enum _NDIS_802_11_WEP_STATUS {
	Ndis802_11WEPEnabled,
	Ndis802_11Encryption1Enabled 		= Ndis802_11WEPEnabled,
	Ndis802_11WEPDisabled,
	Ndis802_11EncryptionDisabled 		= Ndis802_11WEPDisabled,
	Ndis802_11WEPKeyAbsent,
	Ndis802_11Encryption1KeyAbsent 		= Ndis802_11WEPKeyAbsent,
	Ndis802_11WEPNotSupported,
	Ndis802_11EncryptionNotSupported 	= Ndis802_11WEPNotSupported,
	Ndis802_11Encryption2Enabled,
	Ndis802_11Encryption2KeyAbsent,
	Ndis802_11Encryption3Enabled,
	Ndis802_11Encryption3KeyAbsent,
	Ndis802_11Encryption4Enabled,	/* TKIP or AES mix */
	Ndis802_11Encryption4KeyAbsent,
	Ndis802_11GroupWEP40Enabled,
	Ndis802_11GroupWEP104Enabled
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS, NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

typedef struct _CIPHER_KEY {
	UCHAR Key[16];		/* 128 bits max */
	UCHAR TxMic[8];
	UCHAR RxMic[8];
	UCHAR TxTsc[16];	/* TSC value. Change it from 48bit to 128bit */
	UCHAR RxTsc[16];	/* TSC value. Change it from 48bit to 128bit */
	UCHAR CipherAlg;	/* 0:none, 1:WEP64, 2:WEP128, 3:TKIP, 4:AES, 5:CKIP64, 6:CKIP128 */
	UCHAR KeyLen;		/* Key length for each key, 0: entry is invalid */
	UCHAR BssId[6];		/* CONFIG_STA_SUPPORT */
	UCHAR Type;			/* Indicate Pairwise/Group when reporting MIC error */
} CIPHER_KEY, *PCIPHER_KEY;


#endif  /* __RTMP_IF_H__ */
