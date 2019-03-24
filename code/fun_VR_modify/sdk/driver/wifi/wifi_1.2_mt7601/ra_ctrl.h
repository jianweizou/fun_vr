#ifndef __RA_CTRL_H__
#define __RA_CTRL_H__

#include "types.h"
#include "rtmp_if.h"

#define RATE_TABLE_SIZE(pTable)			((pTable)[0])		/* Byte 0 is number of rate indices */
#define RATE_TABLE_INIT_INDEX(pTable)	((pTable)[1])		/* Byte 1 is initial rate index */

/* Values of LastSecTxRateChangeAction */
#define RATE_NO_CHANGE	0		/* No change in rate */
#define RATE_UP			1		/* Trying higher rate or same rate with different BF */
#define RATE_DOWN		2		/* Trying lower rate */

#define DRS_TX_QUALITY_WORST_BOUND       3/* 8  // just test by gary */

VOID MLME_DynamicTxRateSwitching(
	IN PRTMP_ADAPTER 	pAd);
#endif
