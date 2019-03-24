#include "rtmp_timer.h"
#include "system.h"
#include "common.h"

/* Using RTOS-Timer to control RTMP timer event */

#define DEFAULT_TIMER			10000 // (10000 ms = 10 sec)


NDIS_STATUS	RTMPInitTimer(
	IN	PRTMP_ADAPTER			pAd,
	IN	PRTMP_SONIX_TIMER		pTimer,
#ifdef TIMER_DEBUG
	IN	PCHAR					pDescribe,
#endif
	IN	PVOID					pTimerFunc,
	IN	PVOID					pData,
	IN	BOOLEAN					Repeat)
{
	pTimer->Valid 			= TRUE;
	pTimer->PeriodicType 	= Repeat;
	pTimer->State      		= FALSE;
	pTimer->cookie 			= (ULONG) pData;
	pTimer->pAd 			= pAd;
	pTimer->TimerValue		= DEFAULT_TIMER;
	
#ifdef TIMER_DEBUG
	NdisZeroMemory(pTimer->Describe, sizeof(pTimer->Describe));
	NdisMoveMemory(pTimer->Describe, pDescribe, strlen(pDescribe));
#endif

	/* create timer mutex to make sure timer control right */
	pTimer->TimerSemLock	= xSemaphoreCreateMutex();

	RTMP_SEM_UNLOCK(pTimer->TimerSemLock);

	/* create rtos sw timer */
	pTimer->xTimer = xTimerCreate(NULL, pTimer->TimerValue/portTICK_RATE_MS, pTimer->PeriodicType, pData, (TimerCallbackFunction_t)pTimerFunc);

#ifdef TIMER_DEBUG
	DBGPRINT(RT_TRACE,("[TRACE]>>>>>>>%s: (%s) timer obj(%p), with mutex(%p) and xTimer(%p)\n", 
						__FUNCTION__, pTimer->Describe, pTimer, pTimer->TimerSemLock, pTimer->xTimer));
#endif
	return NDIS_SUCCESS;
}


VOID RTMPReleaseTimer(
	IN	PRTMP_SONIX_TIMER		pTimer,
	OUT	BOOLEAN					*pCancelled)

{	
	if (pTimer->Valid)
	{
		vSemaphoreDelete(pTimer->TimerSemLock);

		if (xTimerDelete(pTimer->xTimer, 0) == pdFAIL) 
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Destory Timer Failed!!\n", __FUNCTION__));
			*pCancelled = FALSE;
			return;
		}

		*pCancelled = TRUE;
		pTimer->State = TRUE;
		pTimer->Valid = FALSE;
	}
	else
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: failed, Timer hasn't been initialize!\n", __FUNCTION__));
		return;
	}
	
#ifdef TIMER_DEBUG
	DBGPRINT(RT_TRACE,("[TRACE]>>>>>>>%s: (%s) timer obj(%p), with mutex(%p) and xTimer(%p)\n", 
						__FUNCTION__, pTimer->Describe, pTimer, pTimer->TimerSemLock, pTimer->xTimer));
#endif

}


NDIS_STATUS	RTMPSetTimer(
	IN	PRTMP_SONIX_TIMER	pTimer,
	IN	ULONG				Value)
{
	RTMP_SEM_LOCK(pTimer->TimerSemLock);

	if (pTimer->Valid) 
	{	
		if (xTimerStop(pTimer->xTimer, 0) == pdFAIL)
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Stop Timer failed!\n", __FUNCTION__));
			RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
			return NDIS_FAILURE;
		}

		if (xTimerChangePeriod(pTimer->xTimer, Value/portTICK_RATE_MS, 0) != pdPASS) {
			RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
			DBGPRINT(RT_WARN, ("[WARN]%s: Cannot change timer period!!\n", __FUNCTION__));
			return NDIS_FAILURE;
		}

		pTimer->TimerValue = Value;

		/* startup timer */
		if (xTimerStart(pTimer->xTimer, 0) == pdFAIL)
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Start Timer failed!\n", __FUNCTION__));
			RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
			return NDIS_FAILURE;
		}
	}
	else
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: failed, Timer hasn't been initialize!\n", __FUNCTION__));
		RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
		return NDIS_FAILURE;
	}
	
	RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
#ifdef TIMER_DEBUG
	DBGPRINT(RT_TRACE,("[TRACE]>>>>>>>%s: (%s) timer obj(%p), with mutex(%p) and xTimer(%p)\n", 
						__FUNCTION__, pTimer->Describe, pTimer, pTimer->TimerSemLock, pTimer->xTimer));
#endif

	return NDIS_SUCCESS;
}


VOID RTMPCancelTimer(
	IN	PRTMP_SONIX_TIMER		pTimer,
	OUT	BOOLEAN					*pCancelled)
{
	RTMP_SEM_LOCK(pTimer->TimerSemLock);

	if (pTimer->Valid) 
	{
		if (xTimerStop(pTimer->xTimer, 0) == pdFAIL)
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Stop Timer failed!\n", __FUNCTION__));
			RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
			return;
		}
	}
	else
	{
		DBGPRINT(RT_INFO,("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
		RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
		return;
	}
	
	RTMP_SEM_UNLOCK(pTimer->TimerSemLock);
	
#ifdef TIMER_DEBUG
	DBGPRINT(RT_TRACE,("[TRACE]>>>>>>>%s: (%s) timer obj(%p), with mutex(%p) and xTimer(%p)\n", 
						__FUNCTION__, pTimer->Describe, pTimer, pTimer->TimerSemLock, pTimer->xTimer));
#endif
}
