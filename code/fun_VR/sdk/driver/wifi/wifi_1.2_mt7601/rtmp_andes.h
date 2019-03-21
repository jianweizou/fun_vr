#ifndef __RTMP_ANDES_H__
#define __RTMP_ANDES_H__

#include "types.h"
#include "rtmp.h"

#define InbandPacketMaxLen 		192

typedef void (*CMD_RSP_HANDLER)(PRTMP_ADAPTER pAd, UCHAR *Data);


struct CMD_UNIT {
	union {
		struct {
			UCHAR Command;
			UCHAR Token;
			UCHAR Arg0;
			UCHAR Arg1;
		} MCU51;
		struct {
			UINT8 Type;
			USHORT CmdPayloadLen;
			PUCHAR CmdPayload;
			USHORT RspPayloadLen;
			PUCHAR RspPayload;
			ULONG Timeout;
			BOOLEAN NeedRsp;
			BOOLEAN NeedWait;
			CMD_RSP_HANDLER CmdRspHdler;
		} ANDES;
	} u;
};


enum FUN_ID {
	Q_SELECT = 1,
	ATOMIC_TSSI_SETTING  = 5,
};
enum RX_RING_ID {
	RX_RING0,
	RX_RING1,
};

enum CMD_TYPE {
	CMD_FUN_SET_OP 			= 1,	
	CMD_BURST_WRITE 		= 8,
	CMD_READ_MODIFY_WRITE,			/* 9 */
	CMD_RANDOM_READ,				/* 10 */
	CMD_BURST_READ,					/* 11 */
	CMD_RANDOM_WRITE 		= 12,
	CMD_LED_MODE_OP 		= 16,
	CMD_POWER_SAVING_OP 	= 20,
	CMD_WOW_CONFIG,					/* 22 */
	CMD_WOW_QUERY,					/* 23 */
	CMD_WOW_FEATURE 		= 24,
	CMD_CARRIER_DETECT_OP 	= 28,
	CMD_RADOR_DETECT_OP,			/* 29 */
	CMD_SWITCH_CHANNEL_OP,			/* 30 */
	CMD_CALIBRATION_OP,				/* 31 */
	CMD_BEACON_OP,					/* 32 */
	CMD_ANTENNA_OP					/* 33 */
};


enum INFO_TYPE {
	NORMAL_PACKET,
	CMD_PACKET,
};

enum D_PORT {
	WLAN_PORT,
	CPU_RX_PORT,
	CPU_TX_PORT,
	HOST_PORT,
	VIRTUAL_CPU_RX_PORT,
	VIRTUAL_CPU_TX_PORT,
	DISCARD,
};


NDIS_STATUS AndesBBPRandomWrite(
	IN PRTMP_ADAPTER 		pAd, 
	IN UINT32 				Num, ...);


NDIS_STATUS AndesBBPRandomWritePair(
	IN PRTMP_ADAPTER 		pAd, 
	IN PRTMP_REG_PAIR 		pRegPair, 
	IN UINT32 				Num);


NDIS_STATUS AndesBurstWrite(
	IN PRTMP_ADAPTER 		pAd, 
	IN UINT32 				Offset, 
	IN PUINT32 				pData, 
	IN UINT32 				Cnt);


NDIS_STATUS AndesCalibrationOP(
	IN PRTMP_ADAPTER 		pAd, 
	IN UINT32 				CalibrationID, 
	IN UINT32 				Param);


NDIS_STATUS AndesFunSetOP(
	IN PRTMP_ADAPTER 		pAd, 
	IN UINT32 				FunID, 
	IN UINT32 				Param);


NDIS_STATUS AndesRandomWritePair(
	IN PRTMP_ADAPTER 		pAd, 
	IN PRTMP_REG_PAIR 		pRegPair, 
	IN UINT32 				Num);


NDIS_STATUS AndesRFRandomWrite(
	IN PRTMP_ADAPTER 		pAd, 
	IN UINT32 				Num, ...);


NDIS_STATUS AndesRFRandomWritePair(
	IN PRTMP_ADAPTER 		pAd, 
	IN PBANK_RF_REG_PAIR 	pRegPair, 
	IN UINT32 				Num);


NDIS_STATUS MT7601_BBP_write(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR 				regID,
	IN UCHAR 				value);


NDIS_STATUS MT7601_BBP_read(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR 				regID,
	IN PUCHAR 				pValue);


NDIS_STATUS RLT_RF_read(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR 				bank,
	IN UCHAR 				regID,
	INOUT PUCHAR 			pValue);


NDIS_STATUS RLT_RF_write(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR 				bank,
	IN UCHAR 				regID,
	IN UCHAR 				value);


#endif /* __RTMP_ANDES_H__ */
