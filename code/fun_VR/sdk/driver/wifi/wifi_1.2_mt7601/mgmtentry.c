#include "cipher.h"
#include "asic_ctrl.h"
#include "nl80211.h"
#include "mgmtentry.h"

/**************************************************************
In current project, we only allow maxima 5 clients to connect.
Using simple desgin to record client information.

WCID tbl :  [1][2][3][4][5]
Array idx:   0  1  2  3  4

***************************************************************/

typedef struct __WCIDTBL
{
	BOOLEAN	used;

} WCIDTBL, *PWCIDTBL;

static WCIDTBL entry_wcid_tbl[MAX_CLIENT_NUM];


static SHORT WCIDTBL_DispatchWcid(void)
{
	INT i = 0;
	PWCIDTBL pWcidelement = NULL;

	for (i = 0; i <  MAX_CLIENT_NUM; i++)
	{
		pWcidelement = &entry_wcid_tbl[i];
	
		if (pWcidelement->used == FALSE) 
		{
			pWcidelement->used = TRUE;
			return (i+1);
		}
	}

	return -1;
}

static VOID WCIDTBL_ReleaseWcid(IN USHORT wcid)
{
	PWCIDTBL pWcidelement = NULL;
	
	if (wcid <= 0 || wcid > MAX_CLIENT_NUM)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Invalid wcid %d\r\n",  __FUNCTION__, wcid));
		return;
	}

	DBGPRINT(RT_INFO,("[INFO]%s: Release WCID (%d)\r\n", __FUNCTION__,  wcid));
	
	pWcidelement = &entry_wcid_tbl[wcid-1];
	pWcidelement->used = FALSE;
}


PMGMTENTRY MGMTENTRY_GetNode(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr)
{

	PMGMTENTRY	pEntry = NULL;
	
	if (pAd->node_num == 0) return NULL;

	pEntry = pAd->pFirstLinkNode;

	while (pEntry)
	{		
		if (NdisEqualMemory(pEntry->Addr, pAddr, 6))
			return pEntry;
		else 
			pEntry = pEntry->next_node;
	}
	return NULL;
}



PMGMTENTRY MGMTENTRY_SearchNodeByWCID(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			wcid)
{
	PMGMTENTRY	pEntry = NULL;
	
	if (pAd->node_num == 0) return NULL;

	pEntry = pAd->pFirstLinkNode;

	while (pEntry)
	{		
		if (pEntry->Aid == wcid)
			return pEntry;
		else 
			pEntry = pEntry->next_node;
	}
	return NULL;
}

VOID MGMTENTRY_Init(
	IN PRTMP_ADAPTER 	pAd)
{
	pAd->pFirstLinkNode = NULL;
	pAd->pLastLinkNode = NULL;
	pAd->node_num = 0;
	
	/* clear WCID table */
	NdisZeroMemory(&entry_wcid_tbl[0], sizeof(WCIDTBL) * MAX_CLIENT_NUM);
}

BOOLEAN MGMTENTRY_Add(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr)
{
	PMGMTENTRY	pEntry;

	if (pAd->node_num >= MAX_CLIENT_NUM) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Add Node failed: Node number upto limit %d\r\n", 
								__FUNCTION__, MAX_CLIENT_NUM));
		return FALSE;
	}

	pEntry = MGMTENTRY_GetNode(pAd, pAddr);
	if (pEntry) {
		DBGPRINT(RT_WARN, ("[WARN]%s: Add Node failed: Exist Node\r\n", __FUNCTION__));
		return TRUE;
	}

	pEntry = (PMGMTENTRY)pvPortMalloc(sizeof(MGMTENTRY));//malloc(sizeof(MGMTENTRY));
	if (pEntry == NULL) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Add Node failed: Allocate FAIL\r\n", __FUNCTION__));
		return FALSE;
	}
	
	{
		NdisZeroMemory(pEntry, sizeof(MGMTENTRY));

		if (pAd->node_num == 0)
		{
			pAd->pFirstLinkNode = pEntry;
			pAd->pLastLinkNode = pEntry;
			pEntry->pre_node = NULL;
		}
		else
		{
			pAd->pLastLinkNode->next_node = pEntry;
			pEntry->pre_node = pAd->pLastLinkNode;
			pAd->pLastLinkNode = pEntry;
		}

		/* Assign node value */
		pEntry->next_node = NULL;
		
		NdisMoveMemory(pEntry->Addr, pAddr, 6);

		pAd->node_num++;

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pEntry->GroupKeyWepStatus = pAd->ApCfg.GroupKeyWepStatus;
			pEntry->AuthMode = pAd->ApCfg.AuthMode;
			pEntry->WepStatus = pAd->ApCfg.WepStatus;
			pEntry->pAd = pAd;
			pAd->ApCfg.StaCount = pAd->node_num;
			pEntry->Aid = WCIDTBL_DispatchWcid();
		}
		else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pEntry->AuthMode = pAd->StaCfg.AuthMode;
			pEntry->WepStatus = pAd->StaCfg.WepStatus;
			/* It will be updated encryption information */
		}
	}

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef TIMER_DEBUG
		RTMPInitTimer(pAd, &pEntry->RetryTimer, (PCHAR)"WPARetryExec", WPARetryExec, (PVOID)pEntry, FALSE);
#else
		RTMPInitTimer(pAd, &pEntry->RetryTimer, WPARetryExec, (PVOID)pEntry, FALSE);
#endif
		/* Add this entry into ASIC RX WCID search table */
		AsicUpdateRxWCIDTable(pAd, pEntry->Aid, pEntry->Addr);

		DBGPRINT(RT_TRACE, ("[TRACE]%s: Add Node(Wcid:%d) Success: Total node number is %d\r\n", 
						__FUNCTION__, pEntry->Aid, pAd->node_num));
	}
	else
	{
		/* Add this entry into ASIC RX WCID search table */
		pEntry->Aid = MCAST_WCID;	/* Infrastracture mode */
		AsicUpdateRxWCIDTable(pAd, MCAST_WCID, pEntry->Addr);
	}
	
	return TRUE;
}



BOOLEAN MGMTENTRY_Del(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr)
{
	PMGMTENTRY	pPreNode = NULL, pEntry = NULL;
#ifdef PS_TX_QUEUE_DATA
	pBufferCell pNextCell, pTmpCell;
#endif

	if (pAd->node_num == 0)
		return FALSE;

	pEntry = MGMTENTRY_GetNode(pAd, pAddr);
	if (pEntry == NULL) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Delete Node failed: Can't find node\r\n", __FUNCTION__));
		return FALSE;
	}
	
	{
		BOOLEAN Cancelled;

#ifdef PS_TX_QUEUE_DATA
		pTmpCell = pEntry->pPSTxCell;
		while(pTmpCell != NULL)
		{
			vPortFree(pTmpCell->pPacket);
			pNextCell = pTmpCell->pNextCell;
			vPortFree(pTmpCell);
			pTmpCell = pNextCell;
		}

		pEntry->pPSTxCell = NULL;
#endif		

		if (pAd->node_num == 1)
		{
			pAd->pFirstLinkNode = NULL;
			pAd->pLastLinkNode = NULL;
		}
		else if (pEntry->pre_node == NULL)  //First node
		{
			pAd->pFirstLinkNode = pEntry->next_node;
			pEntry->next_node->pre_node = NULL;
		}
		else if (pEntry->next_node == NULL)	//Last node
		{
			pAd->pLastLinkNode = pEntry->pre_node;
			pEntry->pre_node->next_node = NULL;
		}
		else
		{
			pPreNode = pEntry->pre_node;
			pPreNode->next_node = pEntry->next_node;
			((PMGMTENTRY)(pEntry->next_node))->pre_node = pPreNode;
		}
		
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);
		}

		AsicRemovePairwiseKeyEntry(pAd, pEntry->Aid);
		/* Remove this entry into ASIC RX WCID search table */
		WCIDTBL_ReleaseWcid(pEntry->Aid);	
		pAd->node_num--;
		pAd->ApCfg.StaCount = pAd->node_num;

		vPortFree(pEntry);
	}
	DBGPRINT(RT_TRACE, ("[TRACE]%s: Delete Node Success: Total node number is %d\r\n", 
							__FUNCTION__, pAd->node_num));

	return TRUE;
}


VOID MGMTENTRY_ReleaseAll(PRTMP_ADAPTER pAd)
{
	PMGMTENTRY pTmp = NULL;
	PMGMTENTRY pEntry = pAd->pFirstLinkNode;

	while (pEntry != NULL)
	{
		pTmp = pEntry;
		pEntry = pEntry->next_node;
		SendDeAuthReq(pAd, pEntry->Addr, &pAd->TxContext, REASON_DISASSOC_STA_LEAVING);
		MGMTENTRY_Del(pAd, pTmp->Addr);
	}

	pAd->pFirstLinkNode = NULL;
	pAd->pLastLinkNode = NULL;
	pAd->node_num = 0;
	pAd->ApCfg.StaCount = pAd->node_num;
}



BOOLEAN MGMTENTRY_SetLinkStat(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr, 
	IN UCHAR 			stat)
{
	PMGMTENTRY	pEntry;

	if (pAd->node_num == 0)
		return FALSE;

	pEntry = MGMTENTRY_GetNode(pAd, pAddr);
	if (pEntry == NULL) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Set Node Status failed: Cannot find node\r\n", __FUNCTION__));
		return FALSE;
	}

	pEntry->Linkstat = stat;

	DBGPRINT(RT_INFO, ("[INFO]%s: Update Node Status = %d\r\n", __FUNCTION__, pEntry->Linkstat));	

	return TRUE;
}


BOOLEAN MGMTENTRY_UpdateInfo(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			pAddr, 
	IN ULONG 			rx_time, 
	IN UCHAR 			rx_stregth)
{
	PMGMTENTRY	pEntry;

	if (pAd->node_num == 0)
		return FALSE;

	pEntry = MGMTENTRY_GetNode(pAd, pAddr);
	if (pEntry == NULL) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Update Node Information failed: Cannot find node\r\n", __FUNCTION__));
		return FALSE;
	}

	pEntry->rx_time = rx_time;
	pEntry->rx_stregth = rx_stregth;

	DBGPRINT(RT_INFO, ("[INFO]%s: Update Node Information rx_time = %lu, rx_strength = %d\r\n", 
							__FUNCTION__, pEntry->rx_time, pEntry->rx_stregth));	

	return TRUE;
}



VOID MGMTENTRY_GetAllInfo(
	IN PRTMP_ADAPTER 	pAd)
{
	PMGMTENTRY	pEntry = pAd->pFirstLinkNode;

	DBGPRINT(RT_INFO, ("[INFO]Dump: Total number of entry:%d\r\n", pAd->node_num));
	while (pEntry != NULL)
	{
		DBGPRINT(RT_INFO, ("[INFO]Dump: (Aid:%d)%02X:%02X:%02X:%02X:%02X:%02X\r\n", pEntry->Aid, PRINT_MAC(pEntry->Addr)));
		DBGPRINT(RT_INFO, ("[INFO]Dump: Auth:%d, WepStatus:%d, GroupKeyWepStatus:%d\r\n", pEntry->AuthMode, pEntry->WepStatus, pEntry->GroupKeyWepStatus));
		DBGPRINT(RT_INFO, ("[INFO]Dump: WpaState:%d, GTKState:%d\r\n", pEntry->WpaState, pEntry->GTKState));
		DBGPRINT(RT_INFO, ("[INFO]Dump: rx_stregth:%d\r\n", pEntry->rx_stregth));
		pEntry = pEntry->next_node;
	}
}
