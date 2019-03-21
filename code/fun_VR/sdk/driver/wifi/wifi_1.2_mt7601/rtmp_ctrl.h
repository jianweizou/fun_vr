#ifndef __RTMP_CTRL_H__
#define __RTMP_CTRL_H__

#include "MT7601/mt7601_ctrl.h"

#define RTMP_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)	MT7601_BBP_read(_A, _I, _pV)
#define RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)	MT7601_BBP_write(_A, _I, _V)


VOID RT28XXDMAEnable(
	IN PRTMP_ADAPTER 	pAd);


VOID RTMPDrvOpen(
	IN PRTMP_ADAPTER	pAd);


VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd);


VOID RTMPDisableRxTx(
	IN PRTMP_ADAPTER	pAd);


VOID RTMPSetLinkFliter(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN			bTurnOn);

VOID RTMPSetPowerSuspend(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			off);

VOID RTMPSetPhyMode(
	IN PRTMP_ADAPTER 	pAd,
	IN ULONG 			phymode);


INT	RTMPSetCommonHT(
	IN PRTMP_ADAPTER 	pAd);


VOID RTMPSetHT(
	IN PRTMP_ADAPTER 		pAd,
	IN POID_SET_HT_PHYMODE 	pHTPhyMode);


VOID RTMPReadTxPwrPerRate(
	IN PRTMP_ADAPTER 	pAd);


VOID RTMP_EdccaStatusRead(
	IN PRTMP_ADAPTER 	pAd);


VOID RTMP_EdccaTxCtrl(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			stop);


INT RTMP_BBPSetBW(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				bw);


INT RTMP_BBPSetCtrlCH(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				ext_ch);


UCHAR RTMP_BBPSetRxPath(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			rxpath);


INT RTMP_BBPSetTxDAC(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				tx_dac);


INT RTMP_MACSetCtrlCH(
	IN PRTMP_ADAPTER 	pAd, 
	IN INT 				extch);

#endif
