/**
 * @file
 * this is buffer list file
 * buflist.c
 * @author CJ
 */

#include "sonix_config.h"

#if defined (CONFIG_RECORD)	

#include <FreeRTOS.h>
//#include <nonstdlib.h>
#include "buflist.h"

#define MID_BUFLIST_PRINT_QUEUE(fmt, args...) printf("[BUFLIST]%s: "fmt, __func__,##args)

static uint32_t buflist_size = 0;

/**
* @brief Initial buflist parameter.
* @param pList buflist_t structure pointer.
* @return Success or fail.
*/
int buflist_init(pBuflist_t pList , uint32_t size)
{
/*	pList = (pBuflist_t)pvPortMalloc(sizeof(buflist_t), GFP_KERNEL, MODULE_MID_RECORD);
	if(pList == NULL)
	{
		MID_BUFLIST_PRINT_QUEUE("list allocate fail;\n");
		return FALSE;
	}*/
#if USE_FIXED_MEM
	if(size <= 0 ){
		size = (1 << 20);
	}
	
#if defined( CONFIG_SN_GCC_SDK )	
	pList->data = (unsigned char *)pvPortMalloc(size, GFP_KERNEL|GFP_PREALLOCATE, MODULE_MID_RECORD);
#else
	pList->data = (unsigned char *)pvPortMalloc(size);
#endif
	
	if(pList->data == NULL) {
		MID_BUFLIST_PRINT_QUEUE("avbuf allocate fail;\n");
		return FALSE;
	}
	memset(pList->data, 0, size);
	buflist_size = size;
	pList->read_mark = 0;
	pList->size = 0;
	pList->iframe = 0;
	pList->next = NULL;
#else
	pList->read_mark = 0;
	pList->size = 0;
	pList->iframe = 0;
	pList->data = NULL;
	pList->next = NULL;
#endif
	
	return TRUE;
}

/**
* @brief Uninitial buflist and memory.
* @param pList buflist_t structure pointer.
* @return Success or fail.
*/
int buflist_uninit(pBuflist_t pList)
{
	if(!buflist_clear(pList)){
		MID_BUFLIST_PRINT_QUEUE("list clear fail;\n");
		return FALSE;
	}
	
	//vPortFree(pList);
#if USE_FIXED_MEM
	if(pList->data)
		vPortFree(pList->data);

	pList->data = NULL;
#endif
	
	return TRUE;
}

/**
* @brief Check buflist is empty.
* @param pList buflist_t structure pointer.
* @return Success or fail.
*/
int buflist_is_empty(pBuflist_t pList)
{
	if(pList->next == NULL){
		return TRUE;
	}
	
	return FALSE;
}

/**
* @brief Get buflist node length.
* @param pList buflist_t structure pointer.
* @return List length.
*/
int buflist_get_length(pBuflist_t pList)
{
	int listLength = 0;
	buflist_t *pTemp = pList;
	
	while(pTemp->next != NULL){
		pTemp = pTemp->next;
		listLength++;
	}
	
	return listLength;
}

/**
* @brief Clear list node and release memory.
* @param pList buflist_t structure pointer.
* @return Success or fail.
*/
int buflist_clear(pBuflist_t pList)
{
	buflist_t *pTemp = NULL;
	
	if(pList == NULL){
		return FALSE;
	}

	while(pList->next != NULL){
		pTemp = pList->next;
		pList->next = pTemp->next;
		#if USE_FIXED_MEM

		#else
		vPortFree(pTemp->data);
		#endif
		vPortFree(pTemp);
	}

#if USE_FIXED_MEM
	if(pList->data){
		MID_BUFLIST_PRINT_QUEUE(" ##  buflist_clear\n");
		memset(pList->data, 0, buflist_size);
		pList->size = 0;
	}
#endif
	
	return TRUE;
}

#if 0
int SetNodePkt(pBuflist_t pList,int position, unsigned char *newpkt)
{
	buflist_t *pTemp = NULL;
	
	if(!(buflist_get_node(pList, position, &pTemp))){
		return FALSE;
	}
	
	pTemp->data = newpkt;
	
	return TRUE;
}

unsigned char *buflist_get_data(pBuflist_t pList, int position)
{
	buflist_t *pTemp = NULL;
	
	if(!(buflist_get_node(pList, position, &pTemp))){
		MID_BUFLIST_PRINT_QUEUE("cannot find node position\n");
		return FALSE;
	}
	
	return pTemp->data;
}
#endif

/**
* @brief Get node read mark.
* @param pList buflist_t structure pointer.
* @param position node position.
* @return The read mark of node.
* @warning position limit from 1 to listlength.
*/
int buflist_get_mark(pBuflist_t pList, int position)
{
	buflist_t *pTemp = NULL;
	
	if(!(buflist_get_node(pList, position, &pTemp))){
		MID_BUFLIST_PRINT_QUEUE("cannot find node position\n");
		return FALSE;
	}
	
	return pTemp->read_mark;
}

/**
* @brief Set read mark of list node.
* @param pList buflist_t structure pointer.
* @param position node position.
* @param mark set read mark value 0 or 1.
* @return Success or fail.
* @warning position limit from 1 to listlength.
*/
int buflist_set_mark(pBuflist_t pList, int position, int mark)
{
	buflist_t *pTemp = NULL;
	
	if(!(buflist_get_node(pList, position, &pTemp))){
		MID_BUFLIST_PRINT_QUEUE("cannot find node position\n");
		return FALSE;
	}

	pTemp->read_mark = mark;
	return TRUE;
}

/**
* @brief All node's read mark set to 0.
* @param pList buflist_t structure pointer.
* @return Success or fail.
*/
int buflist_reset_mark(pBuflist_t pList)
{
	buflist_t *pTemp = NULL;
	
	if(pList == NULL){
		return FALSE;
	}
	
	pTemp = pList->next;
	
	while(pTemp != NULL){
		pTemp->read_mark = 0;
		pTemp->action = 0;
		pTemp = pTemp->next;
	}
	
	return TRUE;
}

/**
* @brief Read node data info.
* @param pList buflist_t structure pointer.
* @param position node position.
* @return Buffer data pointer.
* @warning position limit from 1 to listlength.
*/
buflist_t *buflist_read_data(pBuflist_t pList, int position)
{
	buflist_t *pTemp = NULL;
	
	if(!(buflist_get_node(pList, position, &pTemp))){
		MID_BUFLIST_PRINT_QUEUE("cannot find node position\n");
		return NULL;
	}
	
	pTemp->read_mark = 1;

	return pTemp;
}

/**
* @brief Insert node to list.
* @param pList buflist_t structure pointer.
* @param beforeWhich insert node before which position.
* @param dataInfo source buffer infomation.
* @return Success or fail.
* @warning beforeWhich limit from 1 to listlength+1.
*/
int buflist_insert(pBuflist_t pList, int beforeWhich, buflist_t dataInfo)
{
	buflist_t *pTemp = NULL;
	
	if(beforeWhich < 1){
		return FALSE;
	}
	
	if(!(buflist_get_node(pList, beforeWhich - 1, &pTemp))){
		MID_BUFLIST_PRINT_QUEUE("cannot find node position\n");
		return FALSE;
	}
#if defined( CONFIG_SN_GCC_SDK )
	buflist_t *newNode = (pBuflist_t)pvPortMalloc(sizeof(buflist_t), GFP_KERNEL, MODULE_MID_RECORD);
#else
	buflist_t *newNode = (pBuflist_t)pvPortMalloc(sizeof(buflist_t));
#endif
	
	if(newNode == NULL)
	{
		MID_BUFLIST_PRINT_QUEUE("list allocate fail;\n");
		return FALSE;
	}

#if USE_FIXED_MEM
	newNode->read_mark = 0;
	newNode->data = NULL;
	memcpy(pList->data + pList->size, dataInfo.data, dataInfo.size);
	pList->size = pList->size + dataInfo.size;
	newNode->size = dataInfo.size;
	newNode->iframe = dataInfo.iframe;
	newNode->action = dataInfo.action;
	newNode->type = dataInfo.type;
	newNode->time= dataInfo.time;
	newNode->next = pTemp->next;
	pTemp->next = newNode;
#else
	newNode->read_mark = 0;
	newNode->data = dataInfo.data;
	newNode->size = dataInfo.size;
	newNode->iframe = dataInfo.iframe;
	newNode->action = dataInfo.action;
	newNode->type = dataInfo.type;
	newNode->time= dataInfo.time;
	newNode->next = pTemp->next;
	pTemp->next = newNode;
#endif

	return TRUE;
}

/**
* @brief delete one node from list.
* @param pList buflist_t structure pointer.
* @param position node position.
* @return Success or fail.
* @warning position limit from 1 to listlength.
*/
int buflist_delete(pBuflist_t pList, int position)
{
	buflist_t *pTemp = NULL;
	buflist_t *pDel = NULL;
	
	if(position < 1){
		return FALSE;
	}

	if(!(buflist_get_node(pList, position - 1, &pTemp))){
		return FALSE;
	}
	
	pDel = pTemp->next;
	pTemp->next = pDel->next;
	#if USE_FIXED_MEM
	if(position == 1) {
		memset(pList->data, 0, buflist_size);
		pList->size = 0;
	}
	#else
	vPortFree(pDel->data);
	#endif
	vPortFree(pDel);
	
	return TRUE;
}


/**
* @brief Get szie for buffer list if USE_FIXED_MEM is defined .
* @return szie for buffer list.

*/
uint32_t buflist_get_size(void)
{
	return buflist_size;
}

/**
* @brief Get node pointer from list.
* @param pList buflist_t structure pointer.
* @param position node position.
* @param node node pointer of position.
* @return Success or fail.
* @warning position limit from 1 to listlength.
*/
int buflist_get_node(pBuflist_t pList, int position, buflist_t **node)
{
	buflist_t *pTemp = NULL;
	int curPos = -1;
	
	pTemp = pList;
	while(pTemp != NULL){
		curPos++;
		if(curPos == position) 
			break;
		pTemp = pTemp->next;
	} 
	
	if(curPos != position){
		return FALSE;
	}
	
	*node = pTemp;
	
	return TRUE;
}

#endif
