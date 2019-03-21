#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <string.h>
#include <stdarg.h>     
#include <time.h>

#define RT30xx 
#define RT33xx 
#define RT3070 
#define RT3370 
#define RT5370
#define MT7601
#define	DOT11_N_SUPPORT
//#define	SINGLE_SKU 0
#define	RTMP_EFUSE_SUPPORT	
//#define 	RTMP_INTERNAL_TX_ALC
#define CONFIG_STA_SUPPORT
#define RTMP_FREQ_CALIBRATION_SUPPORT
//#define	ANT_DIVERSITY_SUPPORT	0
#define RTMP_RF_RW_SUPPORT
#define	RTMP_MAC_USB
#define RLT_MAC

#define	RTMP_USB_SUPPORT 
#define	RTMP_TIMER_TASK_SUPPORT 
#define	RTMP_RF_RW_SUPPORT 
#define	RTMP_EFUSE_SUPPORT 
#define DPD_CALIBRATION_SUPPORT
#define	hSPECIFIC_BCN_BUF_SUPPORT 
#define	RTMP_FREQ_CALIBRATION_SUPPORT 
#define	SPECIFIC_VCORECAL_SUPPORT


#define RALINK_2883_VERSION		((UINT32)0x28830300)
#define RALINK_2880E_VERSION	((UINT32)0x28720200)
#define RALINK_3883_VERSION		((UINT32)0x38830400)
#define RALINK_3070_VERSION		((UINT32)0x30700200)


#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define SWAP16(x) \
			((UINT16) (\
	       	(((UINT16) (x) & (UINT16) 0x00ffU) << 8) | \
	       	(((UINT16) (x) & (UINT16) 0xff00U) >> 8))) 
 
#define SWAP32(x) \
    		((UINT32) (\
	       	(((UINT32) (x) & (UINT32) 0x000000ffUL) << 24) | \
	       	(((UINT32) (x) & (UINT32) 0x0000ff00UL) << 8) | \
	       	(((UINT32) (x) & (UINT32) 0x00ff0000UL) >> 8) | \
	       	(((UINT32) (x) & (UINT32) 0xff000000UL) >> 24))) 

#define SWAP64(x) \
    		((UINT64)( \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) | \
    		(UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))


#ifdef BIG_ENDIAN
#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((UINT64)(x))
#define be2cpu64(x) ((UINT64)(x))
#define cpu2be32(x) ((UINT32)(x))
#define be2cpu32(x) ((UINT32)(x))
#define cpu2be16(x) ((UINT16)(x))
#define be2cpu16(x) ((UINT16)(x))
#else /* Little_Endian */
#define cpu2le64(x) ((UINT64)(x))
#define le2cpu64(x) ((UINT64)(x))
#define cpu2le32(x) ((UINT32)(x))
#define le2cpu32(x) ((UINT32)(x))
#define cpu2le16(x) ((UINT16)(x))
#define le2cpu16(x) ((UINT16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))
#endif


/* 3593 */
#define IS_RT3593(_pAd) 	(((_pAd)->MACVersion & 0xFFFF0000) == 0x35930000)

/* RT5392 */
#define IS_RT5392(_pAd)   	((_pAd->MACVersion & 0xFFFF0000) == 0x53920000) /* Include RT5392, RT5372 and RT5362 */

/* RT5390 */
#define IS_RT5390(_pAd)   	((((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000) ||IS_RT5392(_pAd))	/* Include RT5390,  RT5370, RT5392, RT5372, RT5360 and RT5362 */

#define IS_RT3290(_pAd)		(((_pAd)->MACVersion & 0xFFFF0000) == 0x32900000)

/* RT5390F */
#define IS_RT5390F(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0502))

/* RT5390R */

/* RT5370G */
#define IS_RT5370G(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0503)) /* support HW PPAD ( the hardware rx antenna diversity ) */

/* RT5390R */
#define IS_RT5390R(_pAd) 	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) == 0x1502)) /* support HW PPAD ( the hardware rx antenna diversity ) */

/* PCIe interface NIC */
#define IS_MINI_CARD(_pAd) 	((_pAd)->Antenna.field.BoardType == BOARD_TYPE_MINI_CARD)

/* 5390U (5370 using PCIe interface) */
#define IS_RT5390U(_pAd) 	(IS_MINI_CARD(_pAd) && ((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000)

/* RT5390BC8 (WiFi + BT) */

/* RT5390D */
#define IS_RT5390D(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0502))

/* RT5392C */
#define IS_RT5392C(_pAd)	((IS_RT5392(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0222)) /* Include RT5392, RT5372 and RT5362 */

/* RT3592BC8 (WiFi + BT) */

/*RT3390,RT3370 */
#define IS_RT3390(_pAd)	 	(((_pAd)->MACVersion & 0xFFFF0000) == 0x33900000)
#define IS_RT3090A(_pAd)    ((((_pAd)->MACVersion & 0xffff0000) == 0x30900000))

/* We will have a cost down version which mac version is 0x3090xxxx */
#define IS_RT3090(_pAd)     ((((_pAd)->MACVersion & 0xffff0000) == 0x30710000) || (IS_RT3090A(_pAd)))
#define IS_RT3070(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30700000)
#define IS_RT3071(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30710000)
#define IS_RT2070(_pAd)		(((_pAd)->RfIcType == RFIC_2020) || ((_pAd)->EFuseTag == 0x27))
#define IS_RT2860(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x28600000)
#define IS_RT2872(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x28720000)
#define IS_RT30xx(_pAd)		(((_pAd)->MACVersion & 0xfff00000) == 0x30700000||IS_RT3090A(_pAd)||IS_RT3390(_pAd))
/*#define IS_RT305X(_pAd)		((_pAd)->MACVersion == 0x28720200) */
#define IS_RT3052(_pAd)		(((_pAd)->MACVersion == 0x28720200) && (_pAd->Antenna.field.TxPath == 2))
#define IS_RT3050(_pAd)		(((_pAd)->MACVersion == 0x28720200) && ((_pAd)->RfIcType == RFIC_3020))
#define IS_RT3350(_pAd)		(((_pAd)->MACVersion == 0x28720200) && ((_pAd)->RfIcType == RFIC_3320))
#define IS_RT3352(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x33520000)
#define IS_RT5350(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x53500000)
#define IS_RT3572(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x35720000)
#define IS_MT7601(_pAd)		((((_pAd)->MACVersion & 0xFFFF0000) == 0x76010000))

#define IS_VERSION_AFTER_F(_pAd) 	((((_pAd)->MACVersion&0xffff) >= 0x0212) || (((_pAd)->b3090ESpecialChip == TRUE)))



#define NdisMoveMemory(_a, _b, len) 				memcpy(_a, _b, len)
#define NdisZeroMemory(_a, _b) 						memset(_a, 0x0, _b)
#define NdisFillMemory(_a, len, val)   				memset(_a, val, len)
#define RTMPZeroMemory(_a, _b) 						memset(_a, 0x0, _b)
#define NdisEqualMemory(_a, _b, len)   				(!memcmp(_a, _b, len))
#define NdisCmpMemory(_a, _b, len)  				memcmp(_a, _b, len)


/* Type of wifi device interface */ 
#define BOARD_TYPE_MINI_CARD		0		/* Mini card */
#define BOARD_TYPE_USB_PEN			1		/* USB pen */


/* value domain of pAd->RfIcType */
#define RFIC_2820                   1       /* 2.4G 2T3R */
#define RFIC_2850                   2       /* 2.4G/5G 2T3R */
#define RFIC_2720                   3       /* 2.4G 1T2R */
#define RFIC_2750                   4       /* 2.4G/5G 1T2R */
#define RFIC_3020                   5       /* 2.4G 1T1R */
#define RFIC_2020                   6       /* 2.4G B/G */
#define RFIC_3021                   7       /* 2.4G 1T2R */
#define RFIC_3022                   8       /* 2.4G 2T2R */
#define RFIC_3052                   9       /* 2.4G/5G 2T2R */
#define RFIC_2853					10		/* 2.4G.5G 3T3R */
#define RFIC_3320                   11      /* 2.4G 1T1R with PA (RT3350/RT3370/RT3390) */
#define RFIC_3322                   12      /* 2.4G 2T2R with PA (RT3352/RT3371/RT3372/RT3391/RT3392) */
#define RFIC_3053                   13      /* 2.4G/5G 3T3R (RT3883/RT3563/RT3573/RT3593/RT3662) */
#define RFIC_3853                   13      /* 2.4G/5G 3T3R (RT3883/RT3563/RT3573/RT3593/RT3662) */
#define RFIC_UNKNOWN				0xff

#endif //!<	__SYSTEM_H
