#ifndef __RTMP_TIMER_H__
#define __RTMP_TIMER_H__

#include <FreeRTOS.h>
#include <timers.h>
#include <semphr.h>

#include "types.h"
#include "rtmp_if.h"

//#define TIMER_DEBUG 1

#define RTMP_SEM_LOCK(_a)		xSemaphoreTake(_a, portMAX_DELAY)
#define RTMP_SEM_UNLOCK(_a)		xSemaphoreGive(_a)	


typedef VOID (*pTimerFunc)(PVOID *arg);

typedef struct _SONIX_TIMER {
	xTimerHandle 		xTimer;
	xSemaphoreHandle 	TimerSemLock;
	BOOLEAN 			Valid;					/* Set to True when call RTMPInitTimer */
	BOOLEAN 			State;					/* True if timer cancelled */
	BOOLEAN				Active;					/* Active or not */
	BOOLEAN 			PeriodicType;			/* True if timer is periodic timer */
	BOOLEAN 			Repeat;					/* True if periodic timer */
	ULONG 				TimerValue;				/* Timer value in milliseconds */
	ULONG 				cookie;					/* os specific object */
#ifdef TIMER_DEBUG
	CHAR				Describe[64];			/* Debug only for trace timer event */
#endif
	PVOID 				pAd;
} RTMP_SONIX_TIMER, *PRTMP_SONIX_TIMER;


NDIS_STATUS	RTMPInitTimer(
	IN	PRTMP_ADAPTER			pAd,
	IN	PRTMP_SONIX_TIMER		pTimer,
#ifdef TIMER_DEBUG
	IN	PCHAR					pDescribe,
#endif
	IN	PVOID					pTimerFunc,
	IN	PVOID					pData,
	IN	BOOLEAN					Repeat);

VOID RTMPReleaseTimer(
	IN	PRTMP_SONIX_TIMER		Timer,
	OUT	BOOLEAN					*pCancelled);


NDIS_STATUS	RTMPSetTimer(
	IN	PRTMP_SONIX_TIMER		pTimer,
	IN	ULONG					Value);

VOID RTMPCancelTimer(
	IN	PRTMP_SONIX_TIMER		pTimer,
	OUT	BOOLEAN					*pCancelled);


#endif
