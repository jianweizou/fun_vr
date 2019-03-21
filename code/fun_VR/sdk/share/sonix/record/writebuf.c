/**
 * @file
 * this is middleware write buffer file writebuf.c
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */

#include "sonix_config.h"

#if defined (CONFIG_RECORD)	

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
//#include <nonstdlib.h>
#include <string.h>
#include "ff.h"
#include "writebuf.h"
#include "include/mp4.h"

#define MEASURE_F_WRITE_TIME	0

#if MEASURE_F_WRITE_TIME
#include <sys/time.h>
#endif

#define WB_PRINT(fmt, args...) printf("[WBF] %s : "fmt, __func__,##args)
#define WRITEBUF_CHECK(pWB) {if(!pWB) {WB_PRINT("invalid pWBInfo\n"); return pdFAIL;}}
#define WRITEBUF_BUFFER_CHECK(pWB) {if(pWB->WriteBuf == NULL) {WB_PRINT("invalid pWBInfo Buffer\n"); return pdFAIL;}}

static int WriteBufToFile(WriteBufInfo_t *pWBInfo, WriteCmd_t *pwcmd ,unsigned char *addr, int size, int *written_size)
{
	int ret = FR_OK;
	int cpy_size;
	unsigned int uiWritedSize;
#if MEASURE_F_WRITE_TIME
	struct timeval start_tv, end_tv;
	unsigned int total_ms = 0;
#endif
	*written_size = 0;

	while(size > 0)
	{
		ret = 0xff;

		if(pWBInfo->BufEnd + size <= pWBInfo->writebufferuserparam.write_buf_size)
			cpy_size = size;
		else
			cpy_size = pWBInfo->writebufferuserparam.write_buf_size - pWBInfo->BufEnd;

#if MEASURE_F_WRITE_TIME
		gettimeofday(&start_tv, NULL);
#endif
		ret = f_write(pwcmd->pf, addr, cpy_size, &uiWritedSize);
		if(ret == FR_OK)
		{
#if MEASURE_F_WRITE_TIME
			gettimeofday(&end_tv, NULL);
			total_ms = end_tv.tv_sec * 1000 + end_tv.tv_usec / 1000 - start_tv.tv_sec * 1000 - start_tv.tv_usec / 1000;
			print_msg_queue("%s(%d): write size=%d bytes, f_write uses %d ms, %d KB/sec\n", __func__, __LINE__, uiWritedSize, total_ms, uiWritedSize * 1000 / (total_ms * 1024));
#endif
			xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
			pWBInfo->ulFilePos += uiWritedSize;
			pWBInfo->BufEnd = (pWBInfo->BufEnd + uiWritedSize) % pWBInfo->writebufferuserparam.write_buf_size;
			pWBInfo->iRemainSize += uiWritedSize;
			xSemaphoreGive(pWBInfo->Sema_block);
			*written_size += uiWritedSize;
			addr = pWBInfo->WriteBuf + (pWBInfo->BufEnd % pWBInfo->writebufferuserparam.write_buf_size);
			size -= uiWritedSize;
		} else
		{
			// SD card is removed
			// if the error is not due to removal of SD, we still rety it 
			if(ret == FR_NOT_READY || ret == FR_INVALID_OBJECT || ret == FR_DISK_ERR)
			{
				xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
				pWBInfo->ulFilePos = 0;
				pWBInfo->BufStart = 0; 
				pWBInfo->BufEnd = 0;
				pWBInfo->iRemainSize = pWBInfo->writebufferuserparam.write_buf_size;
				pWBInfo->ucBufStatus = WB_STATUS_NORMAL;
				xSemaphoreGive(pWBInfo->Sema_block);
			}else
				pWBInfo->ucBufStatus|=WB_STATUS_WR_FAIL;

			writebuf_reset(pWBInfo);
			WB_PRINT("write to file fail(ret = %d, size = %d, %d)\n"
					, ret, size, uiWritedSize);
			break;
		}
	}
	return ret;
}

static void task_write_to_file(void *pvParameters)
{
	int ret=0, count = 0;
	unsigned int uiWritedSize;
	WriteBufInfo_t *pWBInfo = (WriteBufInfo_t *)pvParameters;
	WriteCmd_t wcmd;
	ReturnCmd_t rt_cmd;
	ReturnCmd_t rt_recover ;
	int recover_flag = 0;
	int TotalWriteSize = 0;
	int written_size = 0;
//	int quenum = 0;
	int wfstatus = 0;
	WriteCmd_t wcmd_t;
	CurrentMp4Info *curMp4Info = NULL;
	
	while(1)
	{
#if 0
		quenum = uxQueueSpacesAvailable(pWBInfo->queue_write);

		if ((quenum == WRITE_CMD_WRITE_NUM) && (wfstatus == 2) && (TotalWriteSize >= (pWBInfo->iWriteToFileUnit))) {
			unsigned char *addr = pWBInfo->WriteBuf + pWBInfo->BufEnd;
print_msg_queue("====>TotalWriteSize=%d\n", TotalWriteSize);
		//	WB_PRINT(SYS_DBG", quenum=%x,%x,%x,%x,%s\n",quenum,pWBInfo->WriteBuf,pWBInfo->BufEnd,wfstatus,wcmd_t.arg1);
			WriteBufToFile(pWBInfo,  &wcmd_t, addr, pWBInfo->iWriteToFileUnit);
			TotalWriteSize -= pWBInfo->iWriteToFileUnit;
		}
		if (xQueueReceive(pWBInfo->queue_write, &wcmd,
				/*(1000 / portTICK_RATE_MS)*/portMAX_DELAY) == pdFALSE) {
//			WB_PRINT(SYS_DBG, "queue receive timeout, retry..., TotalWriteSize=%d\n", TotalWriteSize);
			continue;
		}
#else
		memset(&rt_cmd, 0, sizeof(ReturnCmd_t));
		memset(&rt_recover, 0, sizeof(ReturnCmd_t));
		curMp4Info = NULL;
		
		xQueueReceive(pWBInfo->queue_write, &wcmd, portMAX_DELAY);
#endif
		rt_cmd.cmd_type = wcmd.cmd_type;
		rt_cmd.pf = wcmd.pf;
		rt_cmd.arg1= wcmd.arg1;
		rt_cmd.arg2= wcmd.arg2;
		rt_cmd.status = WB_CMD_FAIL;
	
		rt_recover.cmd_type =  wcmd.cmd_type;
		rt_recover.user_info = wcmd.user_info;
		if (rt_recover.user_info != NULL)
			curMp4Info = (CurrentMp4Info *)rt_recover.user_info;
	
		rt_recover.status = WB_CMD_FAIL;
		
		ret = -1;
		if(!wcmd.pf)
		{
			WB_PRINT("pf is invalid\n");
			continue;
		}

		switch(wcmd.cmd_type)
		{
			case WB_CMD_FOPEN:
				{			
					char *fname = (char *)wcmd.arg1;

					xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
					pWBInfo->ulFilePos = 0;
					xSemaphoreGive(pWBInfo->Sema_block);

					count = 0;
					TotalWriteSize = 0;
					wcmd_t = wcmd;

#ifdef CONFIG_FATFS_CMD_QUEUE					
					if((ret = f_open_to_front(wcmd.pf, fname, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK)
#else
					if((ret = f_open(wcmd.pf, fname, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK)					
#endif						
					{
						WB_PRINT("open %s file fail(%d)\n", fname, ret);
						rt_cmd.status = WB_CMD_FAIL;
						rt_recover.status = WB_CMD_FAIL;
					}
					else
					{
						WB_PRINT("open %s file success\n", fname);	//dbg
						rt_cmd.status = WB_CMD_SUCCESS;
						rt_recover.status = WB_CMD_SUCCESS;
						
						if (curMp4Info != NULL)
						{
							if (curMp4Info->state == 1)
								recover_flag = 1;
						}

						wfstatus = 1;
					}	
				}	
				break;
			case WB_CMD_WR_FRAME:
				{
					int FrameSize = (int)wcmd.arg2;

					count ++;
					if(count > FILE_ATTR_UPDATE_COUNT)
					{
						f_updateattr(wcmd.pf);
						count = 0;
					}

					TotalWriteSize += FrameSize;

					while(TotalWriteSize >= pWBInfo->iWriteToFileUnit)
					{
						ret = WriteBufToFile(pWBInfo, &wcmd, pWBInfo->WriteBuf + pWBInfo->BufEnd, pWBInfo->iWriteToFileUnit, &written_size);
						if (ret == FR_OK)
							TotalWriteSize -= written_size;
						else
							break;
//						vTaskDelay(30 / portTICK_RATE_MS);
					}
					if (ret == FR_OK)
					{
						rt_cmd.status = WB_CMD_SUCCESS;		
						rt_recover.status = WB_CMD_SUCCESS;
					}	
					else 
					{
						rt_cmd.status = WB_CMD_FAIL;
						//WB_PRINT(SYS_DBG, "rt_cmd.status = WB_CMD_FAIL\n"); 
					}
					
					wfstatus = 2;
				}

				break;
			case WB_CMD_FILL_DATA:
				{
					unsigned int* pDataPtr;
					unsigned int* pTmpDataPtr;
					unsigned long pos = (unsigned long)wcmd.arg1;
					unsigned long data = (unsigned long)wcmd.arg2;
					//WB_PRINT(SYS_DBG, "seek = %d data = 0x%x\n", pos, data);
					if(pWBInfo->dev==REC_DEV_USBH){
#if defined( CONFIG_SN_GCC_SDK )
						pDataPtr = (unsigned int*)pvPortMalloc((unsigned long)+0x1ff, GFP_KERNEL, MODULE_MID_RECORD);
#else
						pDataPtr = (unsigned int*)pvPortMalloc((unsigned long)+0x1ff);
#endif						
						if(pDataPtr == NULL){
							WB_PRINT("allocating mem fails \n");
							break;	
						}
							
						pTmpDataPtr = (unsigned int*)(((unsigned int)pDataPtr+0x1ff) & ~0x1ff);
						
						memcpy(pTmpDataPtr,&wcmd.arg2,sizeof(uint32_t));
						//WB_PRINT(SYS_DBG, "WB_CMD_FILL_DATA,pDataPtr=%x , pTmpDataPtr=%x, data=%d, pf.fptr=%x, pos=%x\n",pDataPtr,pTmpDataPtr, (unsigned int)*pTmpDataPtr,wcmd.pf->fptr, wcmd.arg1);
					}
					
					if((ret = f_lseek(wcmd.pf, pos)) != FR_OK)
					{
						WB_PRINT("seek %d fail(ret = %d)\n", pos, ret);
						break;				
					}	

					if(pWBInfo->dev==REC_DEV_USBH)
						f_write(wcmd.pf, pTmpDataPtr, 4, &uiWritedSize);
					else
						f_write(wcmd.pf, &data, 4, &uiWritedSize);

					
					if( (ret = f_lseek(wcmd.pf, f_size(wcmd.pf))) )//back to end
					{
						WB_PRINT("seek %d fail(ret = %d)\n", f_size(wcmd.pf), ret);
						break;				
					}

					if(pWBInfo->dev==REC_DEV_USBH){
						if(pDataPtr)
							vPortFree(pDataPtr);
					}
					
					rt_cmd.status = WB_CMD_SUCCESS;
				}
				break;
			case WB_CMD_CLEAN_ALL:
				{
					xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
					pWBInfo->ulFilePos = 0;
					pWBInfo->BufStart = 0; 
					pWBInfo->BufEnd = 0;
					pWBInfo->iRemainSize = pWBInfo->writebufferuserparam.write_buf_size;
					pWBInfo->ucBufStatus = WB_STATUS_NORMAL;					
					xSemaphoreGive(pWBInfo->Sema_block);
				}
				break;
			case WB_CMD_FCLOSE:
					ret = FR_OK;
					if(TotalWriteSize > 0)
					{
						ret = WriteBufToFile(pWBInfo, &wcmd, pWBInfo->WriteBuf + pWBInfo->BufEnd, TotalWriteSize, &written_size);
						if (ret == FR_OK)
						{
							TotalWriteSize -= written_size;
							if(pWBInfo->dev==REC_DEV_USBH)
								writebuf_reset(pWBInfo);
						}
						else
						{
							rt_recover.status = WB_CMD_FAIL;
							break;
						}
					}

					if(f_close(wcmd.pf) == FR_OK && ret == FR_OK)
					{
						//WB_PRINT(SYS_DBG, "total size = %d\n", f_size(wcmd.pf));	//dbg
						rt_cmd.status = WB_CMD_SUCCESS;
						rt_recover.status = WB_CMD_SUCCESS;
						wfstatus = 0;			
					}
					else
					{
						WB_PRINT("close file fail\n");				//dbg
						rt_cmd.status = WB_CMD_FAIL;
						rt_recover.status = WB_CMD_FAIL;
					}
				break;
		}
		xQueueSendToBack( pWBInfo->queue_finish, &rt_cmd, portMAX_DELAY );
		
		if(recover_flag == 1)
		{
			xQueueSendToBack( pWBInfo->queue_recover, &rt_recover, portMAX_DELAY);
			if (rt_recover.cmd_type == WB_CMD_FCLOSE)
				recover_flag = 0;
		}
		
	}
	vTaskDelete(NULL);
}

/**
* @brief interface function - write buffer initialization
* @param pWBInfo write buffer structure
* @return return pdPASS if success
*/
int writebuf_init(WriteBufInfo_t *pWBInfo,BufInitInfo_t *pWBInitInfoParam)
{
	char task_name[configMAX_TASK_NAME_LEN * 2];

	memset(task_name, 0, configMAX_TASK_NAME_LEN * 2);

	WRITEBUF_CHECK(pWBInfo);
	memset(pWBInfo, 0, sizeof(WriteBufInfo_t));
	pWBInfo->dev=f_get_drv();

	if(pWBInfo->dev==REC_DEV_USBH){
#if defined( CONFIG_SN_GCC_SDK )		
		pWBInfo->OriginAddr= (uint8_t*)pvPortMalloc(pWBInitInfoParam->write_buf_size + 0x1FF, GFP_KERNEL|GFP_PREALLOCATE, MODULE_MID_RECORD);
#else		
		pWBInfo->OriginAddr= (uint8_t*)pvPortMalloc(pWBInitInfoParam->write_buf_size + 0x1FF);
#endif		
		pWBInfo->WriteBuf = (uint8_t*)(((unsigned int)pWBInfo->OriginAddr + 0x1FF) & ~0x1FF);
	}else{
#if defined( CONFIG_SN_GCC_SDK )		
		pWBInfo->WriteBuf = pvPortMalloc(pWBInitInfoParam->write_buf_size, GFP_KERNEL|GFP_PREALLOCATE, MODULE_MID_RECORD);
#else
		pWBInfo->WriteBuf = pvPortMalloc(pWBInitInfoParam->write_buf_size);
#endif		
	}
	
	if(!pWBInfo->WriteBuf)
	{
		WB_PRINT("\n\n write buffer alloc fail(size = %d)\n\n",pWBInitInfoParam->write_buf_size);
		goto fail1;
	}
	pWBInfo->BufStart = 0; 
	pWBInfo->BufEnd =  0;
	pWBInfo->iRemainSize = pWBInitInfoParam->write_buf_size;
	pWBInfo->ucBufStatus = WB_STATUS_NORMAL;
	WB_PRINT("WRITE_BUF_SIZE = %d\n", pWBInitInfoParam->write_buf_size);
	pWBInfo->iWriteToFileUnit=pWBInitInfoParam->write_unit_to_file;
	WB_PRINT("WRITE_TOFILEUNIT = %d\n", pWBInfo->iWriteToFileUnit);
	pWBInfo->writebufferuserparam.write_buf_size=pWBInitInfoParam->write_buf_size;
	//pWBInfo->mutex = xSemaphoreCreateMutex();
	//if(!pWBInfo->mutex)
	//{
	//	WB_PRINT(SYS_DBG, "mutex alloc fail\n");
	//	goto fail2;
	//}

	pWBInfo->queue_finish = xQueueCreate(WRITE_CMD_FINISH_NUM, sizeof(ReturnCmd_t));
	if(pWBInfo->queue_finish == NULL)
	{
		WB_PRINT("queue queue_finish create fail\n");
		goto fail3;
	}

	pWBInfo->queue_recover= xQueueCreate(WRITE_CMD_RECOVER_NUM, sizeof(ReturnCmd_t));
	if(pWBInfo->queue_finish == NULL)
	{
		WB_PRINT("queue queue_recover create fail\n");
		goto fail3;
	}
	
	pWBInfo->queue_write = xQueueCreate(WRITE_CMD_WRITE_NUM, sizeof(WriteCmd_t));
	if(pWBInfo->queue_write == NULL)
	{
		WB_PRINT("queue queue_write create fail\n");
		goto fail4;
	}

//	pWBInfo->Sema_block = xSemaphoreCreateBinary();
	pWBInfo->Sema_block = xSemaphoreCreateMutex();
	if(!pWBInfo->Sema_block)
	{
		WB_PRINT("sema_block alloc fail\n");
		goto fail5;
	}
	//set sema as block
//	while(xSemaphoreTake(pWBInfo->Sema_block, 0) == pdPASS) ;

	//start task to write buffer
	sprintf(task_name,"%s-wb2f", pcTaskGetTaskName(NULL));
	if (pdPASS != xTaskCreate(task_write_to_file, task_name, 512, pWBInfo,
			50, &pWBInfo->task_write))	
	{
		WB_PRINT("Could not create task_wb\n");
		goto fail6;
	}
	return pdPASS;

	//vTaskDelete(pWBInfo->task_write);	
 fail6:	
	vSemaphoreDelete(pWBInfo->Sema_block);
 fail5:	
	vQueueDelete(pWBInfo->queue_write);
 fail4:	
 	vQueueDelete(pWBInfo->queue_finish);
 fail3:
	//vSemaphoreDelete(pWBInfo->mutex);
 //fail2:	
	if(pWBInfo->dev==REC_DEV_USBH){
		if(pWBInfo->OriginAddr){
			vPortFree(pWBInfo->OriginAddr); 
			pWBInfo->OriginAddr = NULL;
		}
	}else{
		if(pWBInfo->WriteBuf){
		 	vPortFree(pWBInfo->WriteBuf);		 
			pWBInfo->WriteBuf = NULL;
		}
	}
 fail1:		
	return pdFAIL;
}


/**
* @brief interface function - write buffer create mem
* @param pWBInfo write buffer structure
* @return return pdPASS if success
*/
int writebuf_wb_create(WriteBufInfo_t *pWBInfo,BufInitInfo_t *pWBInitInfoParam)
{
	if(pWBInfo->WriteBuf != NULL){
		WB_PRINT("\n\n WB ALREADY exists  \n\n");
		return pdPASS;
	}
#if defined( CONFIG_SN_GCC_SDK )
	pWBInfo->WriteBuf = pvPortMalloc(pWBInitInfoParam->write_buf_size, GFP_KERNEL|GFP_PREALLOCATE, MODULE_MID_RECORD);
#else	
	pWBInfo->WriteBuf = pvPortMalloc(pWBInitInfoParam->write_buf_size);
#endif	
	if(!pWBInfo->WriteBuf)
	{
		WB_PRINT("\n\n write buffer alloc fail(size = %d)  \n\n", pWBInitInfoParam->write_buf_size);
		return pdFAIL;
	}

	return pdPASS;
}

/**
* @brief interface function - write buffer release mem
* @param pWBInfo write buffer structure
* @return return pdPASS if success
*/
int writebuf_wb_release(WriteBufInfo_t *pWBInfo)
{
	if(pWBInfo->WriteBuf != NULL){
		vPortFree(pWBInfo->WriteBuf);	
		pWBInfo->WriteBuf = NULL;
	}
	return pdPASS;
}

/**
* @brief interface function - write buffer uninitialization
* @param pWBInfo write buffer structure
* @return return pdPASS if success
*/
int writebuf_uninit(WriteBufInfo_t *pWBInfo)
{
	WRITEBUF_CHECK(pWBInfo);
	//wait for write buffer write to file finish
	vTaskDelete(pWBInfo->task_write); 
	vSemaphoreDelete(pWBInfo->Sema_block);
	vQueueDelete(pWBInfo->queue_write);
	vQueueDelete(pWBInfo->queue_finish);
	//vSemaphoreDelete(pWBInfo->mutex);
	if(pWBInfo->dev==REC_DEV_USBH){
		if(pWBInfo->OriginAddr){
			vPortFree(pWBInfo->OriginAddr); 
			pWBInfo->OriginAddr = NULL;
		}
	}else{
		if(pWBInfo->WriteBuf){
		 	vPortFree(pWBInfo->WriteBuf);		 
			pWBInfo->WriteBuf = NULL;
		}		 
	}
	
	return pdPASS;
}

xQueueHandle* writebuf_get_finish_queue(WriteBufInfo_t *pWBInfo)
{
	return &pWBInfo->queue_finish;
}

xQueueHandle* writebuf_get_recover_queue(WriteBufInfo_t *pWBInfo)
{
	return &pWBInfo->queue_recover;
}

void set_writebuf_cmd(WriteBufInfo_t *pWBInfo, CMDType_t cmd_type,FIL *pf, uint32_t arg1, uint32_t arg2)
{
	WriteCmd_t wcmd;

	memset(&wcmd, 0, sizeof(WriteCmd_t));
	wcmd.cmd_type = cmd_type;
	wcmd.pf = pf;
	wcmd.arg1 = arg1;
	wcmd.arg2 = arg2;	
	
	xQueueSendToBack( pWBInfo->queue_write, &wcmd, portMAX_DELAY);
}

void set_writebuf_cmd2(WriteBufInfo_t *pWBInfo, CMDType_t cmd_type,FIL *pf, uint32_t arg1, uint32_t arg2, void *user_info)
{
	WriteCmd_t wcmd;

	memset(&wcmd, 0, sizeof(WriteCmd_t));
	wcmd.cmd_type = cmd_type;
	wcmd.pf = pf;
	wcmd.arg1 = arg1;
	wcmd.arg2 = arg2;
	wcmd.user_info = user_info;
	
	xQueueSendToBack( pWBInfo->queue_write, &wcmd, portMAX_DELAY);
}

/**
* @brief interface function - write data to write buffer
* @param pWBInfo write buffer structure
* @param pfinfo array of data address and data size
* @param len item number of pfinfo
* @return return pdPASS if success
*/
int writebuf_write_frame(WriteBufInfo_t *pWBInfo, frame_info *pframe_info, int len)
{
	int i, total_size=0;
	uint8_t *pdata;
	
	WRITEBUF_CHECK(pWBInfo);
	WRITEBUF_BUFFER_CHECK(pWBInfo);

	for(i=0; i<len; i++)
	{
		total_size += pframe_info[i].uiSize;
	}

	if(total_size > (pWBInfo->writebufferuserparam.write_buf_size))  
	{
		WB_PRINT("(error)data size larger than write buffer size (%d > %d)\n", total_size, (pWBInfo->writebufferuserparam.write_buf_size));
	}
	else 	
	{
		while(total_size > pWBInfo->iRemainSize)
		{
			WB_PRINT("error!! write buffer is full(%d > %d)\n", total_size, pWBInfo->iRemainSize);
			pWBInfo->ucBufStatus|=WB_STATUS_FULL;
			vTaskDelay(100 / portTICK_RATE_MS );
		}

		//copy data to write buffer
		pdata = pWBInfo->WriteBuf + pWBInfo->BufStart;
		for(i=0; i<len; i++)
		{
			
			if(((pWBInfo->writebufferuserparam.write_buf_size) - pWBInfo->BufStart) >= pframe_info[i].uiSize)
			{
				memcpy(pWBInfo->WriteBuf + pWBInfo->BufStart, pframe_info[i].pAddr, pframe_info[i].uiSize);
				xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
				pWBInfo->BufStart = (pWBInfo->BufStart + pframe_info[i].uiSize) % (pWBInfo->writebufferuserparam.write_buf_size);
				xSemaphoreGive(pWBInfo->Sema_block);
			}
			else
			{
				int cpy_size1, cpy_size2;
				cpy_size1 = (pWBInfo->writebufferuserparam.write_buf_size) - pWBInfo->BufStart;
				memcpy(pWBInfo->WriteBuf + pWBInfo->BufStart, pframe_info[i].pAddr, cpy_size1);
				cpy_size2 = pframe_info[i].uiSize - cpy_size1;
				memcpy(pWBInfo->WriteBuf, pframe_info[i].pAddr + cpy_size1, cpy_size2);
				xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
				pWBInfo->BufStart = cpy_size2;
				xSemaphoreGive(pWBInfo->Sema_block);
			}

			//WB_PRINT(SYS_DBG, "cpy 0x%x to 0x%x(size = 0x%x, total = 0x%x)\n", pframe_info[i].pAddr, pWBInfo->pBufStart, pframe_info[i].uiSize, pWBInfo->iRemainSize);	//dbg

			xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
			pWBInfo->iRemainSize -= pframe_info[i].uiSize;
			xSemaphoreGive(pWBInfo->Sema_block);
		}
		//	WB_PRINT(SYS_DBG, "ptr = %x, size = %x, start = %x, end = %x, remain = %x\n",pdata,  total_size, pWBInfo->BufStart, pWBInfo->BufEnd, pWBInfo->iRemainSize);
		set_writebuf_cmd(pWBInfo, WB_CMD_WR_FRAME, pWBInfo->pfile, (uint32_t)pdata, (uint32_t)total_size);
	}

	return pdPASS;
}

/**
* @brief interface function - write data to write buffer and notify write buffer user information
* @param pWBInfo write buffer structure
* @param pfinfo array of data address and data size
* @param len item number of pfinfo
* @param user_info : user defined info
* @return return pdPASS if success
*/
int writebuf_write_frame2(WriteBufInfo_t *pWBInfo, frame_info *pframe_info, int len, void *user_info)
{
	int i, total_size=0;
	uint8_t *pdata;
	
	WRITEBUF_CHECK(pWBInfo);

	for(i=0; i<len; i++)
	{
		total_size += pframe_info[i].uiSize;
	}

	if(total_size > (pWBInfo->writebufferuserparam.write_buf_size))  
	{
		WB_PRINT("(error)data size larger than write buffer size (%d > %d)\n", total_size, (pWBInfo->writebufferuserparam.write_buf_size));
	}
	else 	
	{
		while(total_size > pWBInfo->iRemainSize)
		{
			WB_PRINT("error:write buffer is full(%d > %d)\n", total_size, pWBInfo->iRemainSize);
			vTaskDelay(100 / portTICK_RATE_MS );
		}

		//copy data to write buffer
		pdata = pWBInfo->WriteBuf + pWBInfo->BufStart;
		for(i=0; i<len; i++)
		{
			
			if(((pWBInfo->writebufferuserparam.write_buf_size) - pWBInfo->BufStart) >= pframe_info[i].uiSize)
			{
				memcpy(pWBInfo->WriteBuf + pWBInfo->BufStart, pframe_info[i].pAddr, pframe_info[i].uiSize);
				xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
				pWBInfo->BufStart = (pWBInfo->BufStart + pframe_info[i].uiSize) % (pWBInfo->writebufferuserparam.write_buf_size);
				xSemaphoreGive(pWBInfo->Sema_block);
			}
			else
			{
				int cpy_size1, cpy_size2;
				cpy_size1 = (pWBInfo->writebufferuserparam.write_buf_size) - pWBInfo->BufStart;
				memcpy(pWBInfo->WriteBuf + pWBInfo->BufStart, pframe_info[i].pAddr, cpy_size1);
				cpy_size2 = pframe_info[i].uiSize - cpy_size1;
				memcpy(pWBInfo->WriteBuf, pframe_info[i].pAddr + cpy_size1, cpy_size2);
				xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
				pWBInfo->BufStart = cpy_size2;
				xSemaphoreGive(pWBInfo->Sema_block);
			}

			//WB_PRINT(SYS_DBG, "cpy 0x%x to 0x%x(size = 0x%x, total = 0x%x)\n", pframe_info[i].pAddr, pWBInfo->pBufStart, pframe_info[i].uiSize, pWBInfo->iRemainSize);	//dbg

			xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
			pWBInfo->iRemainSize -= pframe_info[i].uiSize;
			xSemaphoreGive(pWBInfo->Sema_block);
		}
		//	WB_PRINT(SYS_DBG, "ptr = %x, size = %x, start = %x, end = %x, remain = %x\n",pdata,  total_size, pWBInfo->BufStart, pWBInfo->BufEnd, pWBInfo->iRemainSize);
		set_writebuf_cmd2(pWBInfo, WB_CMD_WR_FRAME, pWBInfo->pfile, (uint32_t)pdata, (uint32_t)total_size, user_info);
	}

	return pdPASS;
}

int writebuf_fill_data(WriteBufInfo_t *pWBInfo, unsigned long pos, unsigned long data)
{
	WRITEBUF_CHECK(pWBInfo);

	set_writebuf_cmd(pWBInfo, WB_CMD_FILL_DATA, pWBInfo->pfile, pos, data);
	return pdPASS;
}

/**
* @brief interface function - open file for saving write buffer
* @param pWBInfo write buffer structure
* @param pf file pointer
* @param fname file name
* @return return pdPASS if success
*/
int writebuf_open_file(WriteBufInfo_t *pWBInfo, FIL *pf, char* fname)
{
	WRITEBUF_CHECK(pWBInfo);

	pWBInfo->pfile = pf;
	set_writebuf_cmd(pWBInfo, WB_CMD_FOPEN, pf, (uint32_t)fname, 0);
	return pdPASS;
}

int writebuf_open_file2(WriteBufInfo_t *pWBInfo, FIL *pf, char* fname, void *user_info)
{
	WRITEBUF_CHECK(pWBInfo);
	
	pWBInfo->pfile = pf;
	set_writebuf_cmd2(pWBInfo, WB_CMD_FOPEN, pf, (uint32_t)fname, 0, user_info);
	return pdPASS;
}


/**
* @brief interface function - close file
* @param pWBInfo write buffer structure
* @param pf file pointer
* @return return pdPASS if success
*/
int writebuf_close_file(WriteBufInfo_t *pWBInfo)
{
	WRITEBUF_CHECK(pWBInfo);

	set_writebuf_cmd(pWBInfo, WB_CMD_FCLOSE, pWBInfo->pfile, 0, 0);
	pWBInfo->pfile = 0;
	return pdPASS;
}

/**
* @brief interface function - resetbuf if sdeject 
* @param pWBInfo write buffer structure
* @param pf file pointer
* @return return pdPASS if success
*/
int writebuf_reset(WriteBufInfo_t *pWBInfo)
{
	xSemaphoreTake(pWBInfo->Sema_block, portMAX_DELAY);
	pWBInfo->ulFilePos = 0;
	pWBInfo->BufStart = 0; 
	pWBInfo->BufEnd = 0;
	pWBInfo->iRemainSize = pWBInfo->writebufferuserparam.write_buf_size;
	pWBInfo->ucBufStatus = WB_STATUS_NORMAL;
	
	xSemaphoreGive(pWBInfo->Sema_block);
	return pdPASS;
}

/**
* @brief interface function - write all data in write buffer to file(can't work and wait remove)
* @param pWBInfo write buffer structure
* @return return pdPASS if success
*/
int writebuf_sync(WriteBufInfo_t *pWBInfo)
{
	WRITEBUF_CHECK(pWBInfo);
	//*****//
	return pdPASS;
}

char writebuf_get_wb_status(WriteBufInfo_t *pWBInfo)
{
	//WBUF_PRINT_QUEUE("pWBInfo->ucBufStatus=%d\n",pWBInfo->ucBufStatus);
	return pWBInfo->ucBufStatus;
}
#endif
