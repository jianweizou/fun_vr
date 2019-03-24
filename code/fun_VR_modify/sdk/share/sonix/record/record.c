/**
 * @file
 * this is middleware record file record.c
 * @author Algorithm Dept Sonix. (yiling and chienjen porting to RTOS)
 */

#include "sonix_config.h"

#if defined (CONFIG_RECORD)	

#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
//#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
//#include "include/time.h"
//#include <usr_ctrl.h>
#include <error.h>
#include "record.h"


/** \defgroup mid_record Record middleware modules
 * \n
 * @{
 */
/** @} */

#define RECORD_CHECK(prec) {if(!prec) REC_DBG("invalid rec_info\n");}
#define REC_DEBUG 0
enum{
	RCT_CREATE = 0,
	RCT_CAPTURE,
	RCT_CLOSE
};


enum{
	//REC_ACT_NONE = 0,
	REC_ACT_START = 0,
	REC_ACT_RECVID,
	REC_ACT_RECADO,
	REC_ACT_NEXT,
	REC_ACT_STOP
};

enum{
	REC_NO_CHECK_FRAME=1,
	REC_CHECK_I_FRAME=2,
};

enum{
	REC_NO_CACHE_FRAME=0,
	REC_NEED_CACHE_FRAME=1,
};



typedef struct _record_cmd_queue{
	unsigned char cmd;
	unsigned char IFrame;
	unsigned char *pFrame; 
	unsigned int uiFrameSize;
	uint64_t uiTimeStamp;
	//unsigned char *pstatus;
	xSemaphoreHandle *pSemaRecFinish;
}record_cmd_queue_t;	

/** \defgroup func_mid_record Record middleware function
 *  \ingroup mid_record
 * @{
 */

char lost_path[] = "sonix/lostdata";
static int lost_flag = 0;

//record_info_t rec_info;
#if PRERECORD
void record_save_data(record_info_t* rec_info, void *data, unsigned int size, unsigned char iframe, unsigned char type, unsigned char cmd);
#endif

int add_rec_cmd(record_info_t* rec_info,unsigned char cmd,	unsigned char IFrame, unsigned char *pFrame, unsigned int uiFrameSize, uint64_t uiTimeStamp)
{
	record_cmd_queue_t queue_cmd;


	queue_cmd.cmd = cmd;
	queue_cmd.IFrame= IFrame;
	queue_cmd.pFrame = pFrame;
	queue_cmd.uiFrameSize = uiFrameSize; 
	queue_cmd.uiTimeStamp = uiTimeStamp; 
	
	xSemaphoreTake(rec_info->mutex, portMAX_DELAY);	//add mutex
	queue_cmd.pSemaRecFinish = &rec_info->CmdQueueSema[rec_info->ucSemaphoreIndex];
	rec_info->ucSemaphoreIndex = (rec_info->ucSemaphoreIndex+1)%MAX_SEMA_NUM;
	xSemaphoreGive(rec_info->mutex);	//add mutex

	while(xSemaphoreTake(*queue_cmd.pSemaRecFinish,0)==pdPASS)
	{	
		;//REC_DBG("set semaphore as block\n");
	}
	
	//send cmd to queue
	xQueueSendToBack(rec_info->RecordActQueue,&queue_cmd,portMAX_DELAY);
	

	//if(cmd == REC_ACT_RECVID || cmd == REC_ACT_RECADO || cmd == REC_ACT_STOP)
	{
		//wait for cmd finish
		xSemaphoreTake(*queue_cmd.pSemaRecFinish,portMAX_DELAY);
	}
	return pdPASS;
}

#if PRERECORD
#define list_no_wait_save	1

/**
* @brief task to process buffer list pointer and information
*/
void task_write_cmd( void *pvParameters )
{
	int mark_tmp, first_frame = 0;
	unsigned int video_time = 0;
	pBuflist_t	plist = NULL;
	record_info_t* rec_info = (record_info_t*)pvParameters;

	while(1)
	{
		if((rec_info->ucIsRecording) || (rec_info->ucFileOpen)){
			xSemaphoreTake(rec_info->VdoListMutex, portMAX_DELAY);

			if(!buflist_is_empty(&rec_info->VdoList)){
				if(rec_info->iVdoListReadPosi > buflist_get_length(&rec_info->VdoList)){
					rec_info->iVdoListReadPosi = buflist_get_length(&rec_info->VdoList);
				}
				
				for(; rec_info->iVdoListReadPosi > 0; rec_info->iVdoListReadPosi--){
					mark_tmp = buflist_get_mark(&rec_info->VdoList, rec_info->iVdoListReadPosi);
					if(mark_tmp == 0){
						plist = buflist_read_data(&rec_info->VdoList, rec_info->iVdoListReadPosi);
						
						if((plist != NULL) && !((plist->iframe == 0) && (rec_info->ucFileOpen == 0))){
#if list_no_wait_save						
							buflist_set_mark(&rec_info->VdoList, rec_info->iVdoListReadPosi, 0);

							//if(rec_info->iVdoListReadPosi > 700)
								//REC_DBG(" |||||||||||||||||||| video list read=%d  --------\n", rec_info->iVdoListReadPosi);

							xSemaphoreGive(rec_info->VdoListMutex);
#endif
							if(rec_info->ucFileOpen == 0){
								add_rec_cmd(rec_info, REC_ACT_START, 0, NULL, 0, plist->time);
								rec_info->ucFileOpen = 1;
								first_frame = 1;
								REC_DBG("Cmd new file. iframe=%d\n", plist->iframe);
							}
							else if(plist->action == REC_ACT_NEXT){
								add_rec_cmd(rec_info, REC_ACT_NEXT, 0, NULL, 0, plist->time);
								add_rec_cmd(rec_info, REC_ACT_START, 0, NULL, 0, plist->time);
								REC_DBG("Cmd next file. iframe=%d \n",plist->iframe);
							}
							
							if(plist->action == REC_ACT_STOP){
								add_rec_cmd(rec_info, REC_ACT_STOP, 0, NULL, 0, plist->time);
								rec_info->ucFileOpen = 0;
								REC_DBG(SYS_DBG, "Cmd stop record.\n");
							}
							else{
								add_rec_cmd(rec_info, REC_ACT_RECVID, plist->iframe, plist->data, plist->size, plist->time);							
							}

							video_time = plist->time;
#if list_no_wait_save
							xSemaphoreTake(rec_info->VdoListMutex, portMAX_DELAY);
							buflist_set_mark(&rec_info->VdoList, rec_info->iVdoListReadPosi, 1);
#endif

							//post-setting to clear flag
							if((rec_info->ucRecStart) && (plist->action == REC_ACT_START)){
								;//rec_info->ucRecStart = 0;
							}
							else if(plist->action == REC_ACT_STOP){
								buflist_reset_mark(&rec_info->VdoList);
								rec_info->iVdoListReadPosi = buflist_get_length(&rec_info->VdoList);
								//rec_info->ucRecStop = 0;
								rec_info->ucIsRecording = 0;
							}
							else if(plist->action == REC_ACT_NEXT){
								;//rec_info->ucRecNext = 0;
							}

							plist = NULL;
							break;
						}
					}
				}
			}
			else{
				//REC_DBG("video buffer list is empty.\n");
			}
			xSemaphoreGive(rec_info->VdoListMutex);

			xSemaphoreTake(rec_info->AdoListMutex, portMAX_DELAY);
			if(rec_info->ucFileOpen == 1){
				if(!buflist_is_empty(&rec_info->AdoList)){
					if(rec_info->iAdoListReadPosi > buflist_get_length(&rec_info->AdoList)){
						rec_info->iAdoListReadPosi = buflist_get_length(&rec_info->AdoList);
					}
					
					for(; rec_info->iAdoListReadPosi > 0; rec_info->iAdoListReadPosi--){
						mark_tmp = buflist_get_mark(&rec_info->AdoList, rec_info->iAdoListReadPosi);
						if(mark_tmp == 0){
							plist = buflist_read_data(&rec_info->AdoList, rec_info->iAdoListReadPosi);
							//skip audio packet which time before  video_time in record start
							if((plist != NULL) && !((video_time > plist->time) && (first_frame == 1))){
								first_frame = 0;
#if list_no_wait_save
								buflist_set_mark(&rec_info->AdoList, rec_info->iAdoListReadPosi, 0);

								xSemaphoreGive(rec_info->AdoListMutex);
#endif
								if((plist->time > video_time) && (rec_info->iVdoListReadPosi > 0)){
#if list_no_wait_save
									xSemaphoreTake(rec_info->AdoListMutex, portMAX_DELAY);
#endif									
									buflist_set_mark(&rec_info->AdoList, rec_info->iAdoListReadPosi, 0);
									plist = NULL;
									break;
								}

								//if(rec_info->iAdoListReadPosi > 1250)
									//REC_DBG(" |||||||||||||||||||| audio list read=%d  video_tiime=%d audio_tick=%d ////////\n", rec_info->iAdoListReadPosi, video_time, plist->time);

								add_rec_cmd(rec_info, REC_ACT_RECADO, 1, plist->data, plist->size, plist->time);

#if list_no_wait_save
								xSemaphoreTake(rec_info->AdoListMutex, portMAX_DELAY);
								buflist_set_mark(&rec_info->AdoList, rec_info->iAdoListReadPosi, 1);
#endif
								plist = NULL;
							}
						}
					}
				}
				else{
					//REC_DBG("audio buffer list is empty.\n");
				}
			}
			else{
				buflist_reset_mark(&rec_info->AdoList);
				rec_info->iAdoListReadPosi = buflist_get_length(&rec_info->AdoList);
			}
			xSemaphoreGive(rec_info->AdoListMutex);

			if((rec_info->iVdoListReadPosi == 0) && (rec_info->iAdoListReadPosi == 0)){
				vTaskDelay(10 / portTICK_RATE_MS);
			}

		}
		else{
			first_frame = 0;
			vTaskDelay(200 / portTICK_RATE_MS);
		}
	}

	vTaskDelete(NULL);
}
#endif


int rec_gen_filepath(record_info_t* rec_info,char *szRecordFilePath, system_date_t *tmp_date)
{
#define RECORD_FILEPATH_LEN 300
	system_date_t date;
	char  szRecordType[16];

    if((szRecordFilePath==NULL)||(rec_info->path==NULL))
	{
		REC_DBG("(%d)Given the record file path is NULL!\n",__LINE__);
		return -1;
	}
	memset(szRecordFilePath,'\0',RECORD_FILEPATH_LEN);
	memset(szRecordType,'\0',16);

	if(rec_info->rec_type == RECORD_AVI)
		strncpy(szRecordType,"avi",strlen("avi"));
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4)
		strncpy(szRecordType,"mp4",strlen("mp4"));
#endif	
	else
     	strncpy(szRecordType,"h264",strlen("h264"));
	if(tmp_date == NULL)
//		vicky
//		get_date(&date);
		date = *tmp_date;
	else
		date = *tmp_date;

	snprintf(szRecordFilePath,RECORD_FILEPATH_LEN,"%s/%04d_%02d_%02d_%02d_%02d_%02d_0000.%s",rec_info->path,date.year,date.month,date.day,date.hour,date.minute,date.second,szRecordType);      
	 //REC_DBG("(%d)szRecordFilePath=%s\n",__LINE__,szRecordFilePath);	
	return 1;

}

/**
* @brief interface function - record_set_record_count
* @param 
* @param 
*/
int record_set_record_count(record_info_t* rec_info)
{
	FIL  file;
	FRESULT ret;
	char filelistpath[50];
	uint32_t bufsize = 0, uiWritedSize;
	memset(filelistpath, 0x00, sizeof(filelistpath));
	sprintf(filelistpath, "%s/%s",rec_info->path,RECORD_INDEX);
	if((ret=f_open(&file,filelistpath,FA_CREATE_ALWAYS|FA_WRITE))!=FR_OK)
	{
		 REC_DBG("file open fail(ret=%d)\n",ret);
		 return -1;
	}
	REC_DBG("write record_count=%d\n",rec_info->record_count);
	if(((ret=f_write(&file,&rec_info->record_count,sizeof(unsigned int),&uiWritedSize))!=FR_OK) || ((sizeof(unsigned int))!=uiWritedSize))
	{
		REC_DBG("write data fail(ret = %d), %x, %x\n",ret, bufsize,uiWritedSize);
	} 
    f_close(&file);

	return pdPASS;
}

/**
* @brief interface function - record_get_record_count
* @param 
* @param 
*/
int record_get_record_count(record_info_t* rec_info)
{

	FIL  file;
//	FRESULT ret,retopen;
	FRESULT ret;
	char filelistpath[50];
	uint32_t bufsize = 0, uiReaddSize;
	
	memset(filelistpath, 0x00, sizeof(filelistpath));
	sprintf(filelistpath, "%s/%s",rec_info->path,RECORD_INDEX);
	if(exists(filelistpath)==1)
	{
		if((ret=f_open(&file,filelistpath,FA_OPEN_EXISTING|FA_READ))!=FR_OK)
		{
			 REC_DBG("file open fail(ret=%d)\n",ret);
			 return pdFAIL;
		}
		if(((ret=f_read(&file,&rec_info->record_count,sizeof(unsigned int),&uiReaddSize))!=FR_OK) || ((sizeof(unsigned int))!=uiReaddSize))
		{
			REC_DBG("read data fail(ret = %d), %x, %x\n",ret, bufsize,uiReaddSize);
		}
		 f_close(&file);
	}else 
      REC_DBG("filelistpath is not exit\n");
	
   
	REC_DBG("read read read record_count=%d\n",rec_info->record_count);


	return pdPASS;
}

int set_filename(record_info_t* rec_info)
{
	void *arg;
	int ret;

	if(rec_info->rec_type == RECORD_AVI){
		if(strcmp(avi_get_inteval_file_name(&rec_info->AviInfo), "") == 0){
			ret = snx_fm_create_file(rec_info->record_file_type, avi_get_file_name(&rec_info->AviInfo), AVI_FILENAME_LENGTH, arg);
			if(ret != FM_OK){
				REC_DBG("Create filename error %d!\n", ret);
				return -1;
			}
		}else{
			memcpy(avi_get_file_name(&rec_info->AviInfo), avi_get_inteval_file_name(&rec_info->AviInfo), AVI_FILENAME_LENGTH);
			avi_clear_inteval_file_name(&rec_info->AviInfo);
		}
	}
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4){
		if(strcmp( mp4_get_inteval_file_name(&rec_info->Mp4Info), "") == 0){
			ret = snx_fm_create_file(rec_info->record_file_type, mp4_get_file_name(&rec_info->Mp4Info), MP4_FILENAME_LENGTH, arg);
			if(ret != FM_OK){
				REC_DBG("Create filename error %d!\n", ret);
				return -1;
			}
			else REC_DBG("Create mp4 file %s\n", mp4_get_file_name(&rec_info->Mp4Info));
		}else{
			memcpy(mp4_get_file_name(&rec_info->Mp4Info), mp4_get_inteval_file_name(&rec_info->Mp4Info), MP4_FILENAME_LENGTH);
			mp4_clear_inteval_file_name(&rec_info->Mp4Info);
		}
	}
#endif	
	else{
		if(strcmp(rec_info->IntevalFileName, "") == 0){
			ret = snx_fm_create_file(rec_info->record_file_type, rec_info->FileName, RECORD_FILENAME_LENGTH, arg);
			if(ret != FM_OK){
				REC_DBG("Create filename error %d!\n", ret);
				return -1;
			}
		}else{
			memcpy(rec_info->FileName, rec_info->IntevalFileName, RECORD_FILENAME_LENGTH);
			memset(rec_info->IntevalFileName, 0, RECORD_FILENAME_LENGTH);
		}
	}
	
	return 0;	
}

/**
* @brief a task to process record command in queue
*/
void task_write_file( void *pvParameters )
{
	unsigned char ret;
	record_cmd_queue_t rec_cmd;
	record_info_t* rec_info = (record_info_t*)pvParameters;
	AVI_Info_t *pAviInfo = &rec_info->AviInfo;
	
#ifdef CONFIG_MP4		
	Mp4_Info_t *pMp4Info = &rec_info->Mp4Info;
#endif	
//	frame_info finfo;

#if REC_DEBUG
	int count = 0;
	TickType_t CurTick=0, *tick;

#if defined( CONFIG_SN_GCC_SDK )
	tick = pvPortMalloc(1000*sizeof(TickType_t), GFP_KERNEL, MODULE_MID_RECORD);
#else
	tick = pvPortMalloc(1000*sizeof(TickType_t));
#endif
	
#endif	

	if(rec_info->rec_type == RECORD_AVI){
		pAviInfo->GPS_info = &rec_info->GPS_info;
		pAviInfo->GSENSOR_info = &rec_info->GSENSOR_info;
	}
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4){
		pMp4Info->GPS_info = &rec_info->GPS_info;
		pMp4Info->GSENSOR_info = &rec_info->GSENSOR_info;
	}
#endif	

	while(1)
	{	
		xQueueReceive(rec_info->RecordActQueue,&rec_cmd,portMAX_DELAY);
		//REC_DBG("cmd = %d, status = %d, tick = %d(%c)\n", rec_cmd.cmd, rec_info->ucRecStatus, xTaskGetTickCount()
		//									,rec_cmd.IFrame?'i':'p');
//		REC_DBG("cmd = %d, status = %d, frame = %c\n", rec_cmd.cmd, rec_info->ucRecStatus, rec_cmd.IFrame?'i':'p');
#if REC_DEBUG
		CurTick = xTaskGetTickCount();
#endif
		switch(rec_cmd.cmd)
		{
			case REC_ACT_START:
				if(rec_info->ucRecStatus == RCT_CLOSE)
				{	
					set_filename(rec_info);
					if(rec_info->rec_type == RECORD_AVI)
					{
						REC_DBG("REC_ACT_START (open file %s)\n", avi_get_file_name(pAviInfo));
						if((ret = avi_write_header(pAviInfo)) != pdPASS)
						{
							REC_DBG("%s:avi write header fail(%d!=%d)\n",  __func__,ret, pdPASS);
							buflist_clear(&rec_info->AVBufList);
							break;
						}
					}
#ifdef CONFIG_MP4						
					else if(rec_info->rec_type == RECORD_MP4){
						REC_DBG("action start: (open file %s)\n", mp4_get_file_name(pMp4Info));
						if((ret = mp4_write_header(pMp4Info)) != pdPASS)
						{
							REC_DBG("%s:mp4 write header fail(%d!=%d)\n",  __func__,ret, pdPASS);
							buflist_clear(&rec_info->AVBufList);
							break;
						}
					}
#endif					
					//else{
						//REC_DBG("action start: (open file %s)\n", rec_info->FileName);
						//writebuf_open_file(&rec_info->WBInfo,&rec_info->fstream,  rec_info->FileName);
						//set_start_finish(rec_info, pdPASS);
					//}
					xSemaphoreTake(rec_info->AVListMutex, portMAX_DELAY);
					REC_DBG("%p buflist_is_empty %d  len %d\n",&rec_info->AVBufList, buflist_is_empty(&rec_info->AVBufList), buflist_get_length(&rec_info->AVBufList));
					if(!buflist_is_empty(&rec_info->AVBufList)){
						pBuflist_t	plist = NULL;
						int len;
						unsigned char	*tmp = rec_info->AVBufList.data;
						
						while(!buflist_is_empty(&rec_info->AVBufList)){
							
						#if USE_FIXED_MEM
							len = buflist_get_length(&rec_info->AVBufList);
							plist = buflist_read_data(&rec_info->AVBufList, len);
							
							if(plist != NULL){
								plist->data = tmp ;
								tmp = tmp + plist->size;
//								REC_DBG("del size %d  iframe %d type %d time %d---%d\n",plist->size,plist->iframe, plist->type,plist->time,len);
								if(rec_info->rec_type == RECORD_AVI){
									ret = avi_write_packet(pAviInfo, plist->type,  plist->iframe, plist->data,plist->size, plist->time);

									if(ret != pdPASS)
									{
										REC_DBG("%s:avi write video packet fail\n", __func__);
										//break;
									}
								}
#ifdef CONFIG_MP4									
								else if(rec_info->rec_type == RECORD_MP4){

									ret = mp4_write_packet(pMp4Info, plist->type,  plist->iframe, plist->data,plist->size, plist->time);
									if(ret != pdPASS)
									{
										REC_DBG("%s:mp4 write video packet fail\n", __func__);
										//break;
									}
								}
#endif								
								else{
									REC_DBG("action start: (open file %s)\n", rec_info->FileName);
								}
							}
						#else
							len = buflist_get_length(&rec_info->AVBufList);
							plist = buflist_read_data(&rec_info->AVBufList, len);

					
							if(plist != NULL){
//								REC_DBG("del size %d  iframe %d type %d time %d---%d\n",plist->size,plist->iframe, plist->type,plist->time,len);
								if(rec_info->rec_type == RECORD_AVI){
									ret = avi_write_packet(pAviInfo, plist->type,  plist->iframe, plist->data,plist->size, plist->time);

									if(ret != pdPASS)
									{
										REC_DBG("%s:avi write video packet fail\n", __func__);
										//break;
									}
								}
#ifdef CONFIG_MP4									
								else if(rec_info->rec_type == RECORD_MP4){

									ret = mp4_write_packet(pMp4Info, plist->type,  plist->iframe, plist->data,plist->size, plist->time);
									if(ret != pdPASS)
									{
										REC_DBG("%s:mp4 write video packet fail\n", __func__);
										//break;
									}
								}
#endif								
								else{
									REC_DBG("action start: (open file %s)\n", rec_info->FileName);
								}
							}
							#endif
							
							if(!buflist_delete(&rec_info->AVBufList, len)){
								REC_DBG("buffer list delete fail. list position=%d\n", len);
							}

						}	
					}
					rec_info->ucRecCache = REC_NO_CACHE_FRAME;
					xSemaphoreGive(rec_info->AVListMutex);
					//REC_DBG( "REC_ACT_START done \n");
					rec_info->ucRecStatus = RCT_CAPTURE;
				}
				else
					REC_DBG("REC_ACT_START : cmd ignore(cmd = %d, status = %d)\n", rec_cmd.cmd, rec_info->ucRecStatus);
				break;
			case REC_ACT_RECVID:
				if(rec_info->ucRecStatus == RCT_CAPTURE)
				{
					if(rec_info->rec_type == RECORD_AVI){
						ret = avi_write_packet(pAviInfo, AVI_VIDEO,  rec_cmd.IFrame, rec_cmd.pFrame,rec_cmd.uiFrameSize,rec_cmd.uiTimeStamp);

						if(ret != pdPASS)
						{
							REC_DBG("%s:avi write video packet fail\n", __func__);
							break;
						}
					}	
#ifdef CONFIG_MP4						
					else if(rec_info->rec_type == RECORD_MP4){

						ret = mp4_write_packet(pMp4Info, MP4_VIDEO, rec_cmd.IFrame, rec_cmd.pFrame,rec_cmd.uiFrameSize,rec_cmd.uiTimeStamp);
						if(ret != pdPASS)
						{
							REC_DBG("%s:mp4 write video packet fail\n", __func__);
							break;
						}
					}
#endif					
					//else{
					//	finfo.pAddr =  rec_cmd.pFrame;
					//	finfo.uiSize = rec_cmd.uiFrameSize;
					//	writebuf_write_frame(&rec_info->WBInfo, &finfo, 1);
					//}

				}
				else
					REC_DBG("REC_ACT_RECVID : cmd ignore(cmd = %d, status = %d)\n", rec_cmd.cmd, rec_info->ucRecStatus);
				break;
			case REC_ACT_RECADO:
				if(rec_info->ucRecStatus == RCT_CAPTURE)
				{

					if(rec_info->rec_type == RECORD_AVI){
						if(pAviInfo->has_audio){
							ret = avi_write_packet(pAviInfo, AVI_AUDIO,  1, rec_cmd.pFrame,rec_cmd.uiFrameSize,rec_cmd.uiTimeStamp);

							if(ret != pdPASS)
							{
								REC_DBG("%s:avi write audio packet fail\n", __func__);
								break;
							}	
						}
					}
#ifdef CONFIG_MP4						
					else if(rec_info->rec_type == RECORD_MP4){
						if(pMp4Info->has_audio){
							ret = mp4_write_packet(pMp4Info, MP4_AUDIO, 1, rec_cmd.pFrame,rec_cmd.uiFrameSize,rec_cmd.uiTimeStamp);
							if(ret != pdPASS)
							{
								REC_DBG("%s:mp4 write video packet fail\n", __func__);
								break;
							}
						}
					}
#endif					
					//else{
						//rec_cmd.pFrame must be allocated use GFP_DMA parameter
					//	finfo.pAddr =  rec_cmd.pFrame;
					//	finfo.uiSize = rec_cmd.uiFrameSize;
					//	REC_DBG("ado\n");
					//	writebuf_write_frame(&rec_info->WBInfo, &finfo, 1);
					//}
				}
				else
					REC_DBG("REC_ACT_RECADO : cmd ignore(cmd = %d, status = %d)\n", rec_cmd.cmd, rec_info->ucRecStatus);
				break;
			case REC_ACT_NEXT:
				if(rec_info->ucRecStatus == RCT_CAPTURE)
				{
					REC_DBG("action savd and create new file\n");
					xSemaphoreTake(rec_info->FileSyncMutex, portMAX_DELAY);		//add mutex to sync filename and avi index mapping
							
					if(rec_info->rec_type == RECORD_AVI){
					ret = avi_write_trailer(pAviInfo);
						if(ret != pdPASS)
						{
							REC_DBG("avi write trailer fail\n");
							break;
						}
						strcpy(rec_info->LastFileNameClosing, avi_get_file_name(&rec_info->AviInfo));
					}
#ifdef CONFIG_MP4						
					else if(rec_info->rec_type == RECORD_MP4){
						ret = mp4_write_trailer(pMp4Info);
						if(ret != pdPASS)
						{
							REC_DBG("mp4 write trailer fail\n");
							break;
						}
						strcpy(rec_info->LastFileNameClosing, mp4_get_file_name(&rec_info->Mp4Info));
					}
#endif					
					//else{
					//	writebuf_close_file(&rec_info->WBInfo);
					//	if(ret != FR_OK)
					//	{
					//		REC_DBG("write trailer fail\n");	
					//		break;
					//	}	
					//	strcpy(rec_info->LastFileName, rec_info->FileName);
					//	set_end_finish(rec_info, pdPASS);
					//}
					xSemaphoreGive(rec_info->FileSyncMutex);
					rec_info->ucRecStatus = RCT_CLOSE;
					REC_DBG("REC_ACT_NEXT : ucRecStatus = RCT_CLOSE\n");
					//REC_DBG("count = %d\n", count);
				}
				else
					REC_DBG("REC_ACT_NEXT : cmd ignore(cmd = %d, status = %d)\n", rec_cmd.cmd, rec_info->ucRecStatus);
				break;
			case REC_ACT_STOP:
				//if(rec_info->ucRecStatus == RCT_CREATE || rec_info->ucRecStatus == RCT_CAPTURE)
        if(rec_info->ucRecStatus == RCT_CREATE || rec_info->ucRecStatus == RCT_CAPTURE ||rec_info->ucRecStatus == RCT_CLOSE)
				{
          //for sd insert  and open file fail because of eject  
					REC_DBG("action close\n");
					REC_DBG("status = %d\n",rec_info->ucRecStatus);
					xSemaphoreTake(rec_info->FileSyncMutex, portMAX_DELAY);		//add mutex to sync filename and avi index mapping
					if(rec_info->rec_type == RECORD_AVI){	
						ret = avi_write_trailer(pAviInfo);
						if(ret != pdPASS)
						{
							REC_DBG("avi write trailer fail\n");
						}
						strcpy(rec_info->LastFileNameClosing, avi_get_file_name(&rec_info->AviInfo));
					}
#ifdef CONFIG_MP4						
					else if(rec_info->rec_type == RECORD_MP4){
						ret = mp4_write_trailer(pMp4Info);
						if(ret != pdPASS)
						{
							REC_DBG("mp4 write trailer fail\n");
							break;
						}
						strcpy(rec_info->LastFileNameClosing, mp4_get_file_name(&rec_info->Mp4Info));
					}
#endif					
					//else{
					//	writebuf_close_file(&rec_info->WBInfo);
					//	strcpy(rec_info->LastFileName, rec_info->FileName);
					//}
					xSemaphoreGive(rec_info->FileSyncMutex);
					rec_info->ucRecStatus = RCT_CLOSE;
					REC_DBG("REC_ACT_STOP : ucRecStatus = RCT_CLOSE\n");
					#if REC_DEBUG
					{
						int i;
						
						for(i=0;i<count;i++)
						{
							if((i%10) == 0)
								print_msg_queue("\n");
							print_msg_queue("%d  ", tick[i]);
						}
						print_msg_queue("\n");
						vPortFree(tick);
					}
					#endif
				}
				else
					REC_DBG("REC_ACT_STOP : cmd ignore(cmd = %d, status = %d)\n", rec_cmd.cmd, rec_info->ucRecStatus);
				break;

		}
		#if REC_DEBUG
		if(count<1000)
			tick[count++] = xTaskGetTickCount()-CurTick;
		#endif
		xSemaphoreGive(*rec_cmd.pSemaRecFinish);

	}

//fail:	
	vTaskDelete(NULL);


}

void set_start_finish(record_info_t* rec_info, CMDStatus_t status)
{
	rec_info->ucRecStart = 0;
	rec_info->rec_start_ret = status;
	//if record set start fail, change record status as RCT_CLOSE for retry   
	if(status == pdFAIL)
		rec_info->ucRecStatus = RCT_CLOSE;
}

void set_end_finish(record_info_t* rec_info, CMDStatus_t status)
{	

	char renameoffile[100]={0};
	//rec_info->ucRecWaitFileClose=0;
	//strcpy(rec_info->LastFileName, avi_get_file_name(&rec_info->AviInfo));	
	if(rec_info->ucRecNext)
	{
		if(status == WB_CMD_FAIL)
			rec_info->ucRecStatus = RCT_CLOSE;
		if(status== WB_CMD_SUCCESS)
		{
			if(rec_info->finishedfile_cb!=NULL)
			{
				rec_info->finishedfile_cb(rec_info->record_file_type, rec_info->LastFileNameClosing, renameoffile);
				strcpy(rec_info->LastFileName,renameoffile);			
			}else
			    REC_DBG("Must Set Finished File CB\n");	
		}
		else
		{
			  REC_DBG("wb_cmd status=%d""\n",status);
			  strcpy(rec_info->LastFileName, "");	
		}
		rec_info->ucRecNext = 0;
		rec_info->rec_next_ret = status;
		rec_info->ucRecWaitFileClose=0;
		//if record set next fail, change record status as RCT_CLOSE for retry   

	}
	else
	{
		if(status== WB_CMD_SUCCESS)
		{
			if(rec_info->finishedfile_cb!=NULL)
			{
				rec_info->finishedfile_cb(rec_info->record_file_type, rec_info->LastFileNameClosing, renameoffile);
				strcpy(rec_info->LastFileName,renameoffile);
			}else
			    REC_DBG("Must Set Finished File CB\n");			
		}
		else
		{
			REC_DBG("wb_cmd status=%d""\n",status);
			strcpy(rec_info->LastFileName, "");   
		}
		rec_info->ucRecStop = 0;
		rec_info->rec_stop_ret = status;
		rec_info->ucRecWaitFileClose=0;
	}		
}

void record_writebufreset(record_info_t* rec_info)
{
	if(rec_info->rec_type==RECORD_AVI)
		writebuf_reset(&rec_info->AviInfo.WBInfo);
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type==RECORD_MP4)
		writebuf_reset(&rec_info->Mp4Info.WBInfo);	
#endif	
}
/**
* @brief interface function - record initialization
* @param prec_info pointer for record structure
* @param pRecParam pointer for param of the recording(include audio attr,video attr and the write buffer attr)
* @param record_type pointer for recording type(avi or mp4) 
* @param has_audio  for 1:has audio 0:no audio 
* @param cb pointer for modify the name of the file when recording over 
* @return return pdPASS if success
*/
int record_init(record_info_t** prec_info, RecParam_t *pRecParam, unsigned char index,
		enum  RECORD_TYPE record_type, unsigned int cap, finishedfile_cb_t cb,
		update_seed_cb_t update_seed, int max_pre_time, enum FILE_TYPE file_type)
{
	int i, tmp;
	char task_name[50];
	RecVdoInfo_t *pvdo = &pRecParam->vdo;
	RecAdoInfo_t *pado = &pRecParam->ado;
	RecWirteBufferInitInfo_t *pbufferdo = &pRecParam->writebufferparam;
//	RecRecoverInfo_t *pRecRecoverInfo = &pRecParam->recover;
	record_info_t* rec_info;

	*prec_info = NULL;
#if defined( CONFIG_SN_GCC_SDK )
	rec_info = pvPortMalloc(sizeof(record_info_t), GFP_KERNEL, MODULE_MID_RECORD);
#else
	rec_info = pvPortMalloc(sizeof(record_info_t));
#endif	
	
	if(rec_info == NULL)
	{
		REC_DBG("record worksapce allocate fail;\n");		
		goto fail12;
	}	
	memset(rec_info, 0, sizeof(record_info_t));
	
	rec_info->index = index;
	rec_info->rec_type = record_type;
	rec_info->ucRecWaitFileClose=0;
	rec_info->ucRecCache = REC_NO_CACHE_FRAME;
	rec_info->AVListMutex = xSemaphoreCreateMutex();
	rec_info->finishedfile_cb = cb;
	rec_info->update_seed_cb=update_seed;
	rec_info->record_file_type = file_type;
	
	
	
	if(NULL == rec_info->AVListMutex)
	{
		REC_DBG("sempphore rec_info.AVListMutex create fail\n"); 
		goto fail11;
	}	
	if(!buflist_init(&rec_info->AVBufList, pRecParam->writebufferparam.buflist_size))
	{
		goto fail10;
	}

#if PRERECORD
	if(!buflist_init(&rec_info->VdoList,pRecParam->writebufferparam.buflist_size))
	{
		goto fail9;
	}
	
	if(!buflist_init(&rec_info->AdoList,pRecParam->writebufferparam.buflist_size))
	{
		goto fail8;
	}
#endif	

	if(rec_info->rec_type == RECORD_AVI){
		/*****init AVI*******/
		AviVdoInfo_t VdoInfo;
		AviAdoInfo_t AdoInfo;
		AviWirteBufferInitInfo_t WriteBufferInitParam;
		VdoInfo.width = pvdo->width;
		VdoInfo.height= pvdo->height;
		VdoInfo.ucFps = pvdo->ucFps; 
		VdoInfo.uiBps = pvdo->uiBps; 
		VdoInfo.ucScale= pvdo->ucScale;
		VdoInfo.ucStreamMode = pvdo->ucStreamMode;
		AdoInfo.uiFormat = pado->uiFormat;
		AdoInfo.uiSampleRate = pado->uiSampleRate;
		AdoInfo.ucChannel = 1;
		AdoInfo.ucBitsPerSample = pado->ucBitsPerSample;
		AdoInfo.uiFps = pado->uiSampleRate*(pado->ucBitsPerSample>>3)/pado->uiPacketSize;
		AdoInfo.uiBitRate = pado->uiBitRate;
		WriteBufferInitParam.write_buf_size=pbufferdo->write_buf_size;
		WriteBufferInitParam.write_unit_to_file=pbufferdo->write_unit_to_file;

		if(avi_init(&rec_info->AviInfo,&WriteBufferInitParam,&VdoInfo, cap) != pdPASS)
			goto fail7;


		if(avi_set_stream_info(&rec_info->AviInfo, &VdoInfo, &AdoInfo, pRecParam->max_record_len)!=pdPASS)
			goto fail7;

		avi_set_cb(&rec_info->AviInfo, (avi_cb_t)set_start_finish, (avi_cb_t)set_end_finish,(avi_cb_t)avi_start_finish, (void*)rec_info);
	}
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4){
		Mp4VdoInfo_t VdoInfo;
		Mp4AdoInfo_t AdoInfo;
		Mp4WirteBufferInitInfo_t WriteBufferInitParam;
		Mp4RecoverInfo_t Mp4RecoverInfo;
		memset(&VdoInfo, 0, sizeof(Mp4VdoInfo_t));
		memset(&AdoInfo, 0,  sizeof(Mp4AdoInfo_t));
		memset(&Mp4RecoverInfo, 0,  sizeof(Mp4RecoverInfo_t));
		memset(&WriteBufferInitParam, 0,  sizeof(Mp4WirteBufferInitInfo_t));
		
		VdoInfo.width = pvdo->width;
		VdoInfo.height= pvdo->height;
		VdoInfo.ucFps = pvdo->ucFps; 
		VdoInfo.ucScale= pvdo->ucScale;
		VdoInfo.ucStreamMode = pvdo->ucStreamMode;
		AdoInfo.uiFormat = pado->uiFormat;
		AdoInfo.uiSampleRate = pado->uiSampleRate;
		AdoInfo.ucChannel = 1;
		AdoInfo.ucBitsPerSample = pado->ucBitsPerSample;
		AdoInfo.uiFps = pado->uiSampleRate*(pado->ucBitsPerSample>>3)/pado->uiPacketSize;
		AdoInfo.uiBitRate = pado->uiBitRate;
		WriteBufferInitParam.write_buf_size=pbufferdo->write_buf_size;
		WriteBufferInitParam.write_unit_to_file=pbufferdo->write_unit_to_file;
		Mp4RecoverInfo.state = pRecRecoverInfo->state;
		Mp4RecoverInfo.save_path = pRecRecoverInfo->save_path;
		Mp4RecoverInfo.presion = pRecRecoverInfo->presion;

		if(mp4_init(&rec_info->Mp4Info,&WriteBufferInitParam, cap) != pdPASS)
			goto fail7;


		if(mp4_set_stream_info(&rec_info->Mp4Info, &VdoInfo, &AdoInfo)!=pdPASS)
			goto fail7;

		mp4_set_cb(&rec_info->Mp4Info, (mp4_cb_t)set_start_finish, (mp4_cb_t)set_end_finish,(mp4_cb_t)mp4_start_finish ,(void*)rec_info);
		
		mp4_save_trailer_init(&rec_info->Mp4Info,  &Mp4RecoverInfo);
	}
#endif	
	rec_info->max_av_length = max_pre_time * (pvdo->ucFps +  (pado->uiSampleRate*(pado->ucBitsPerSample>>3)/pado->uiPacketSize));
	
#if PRERECORD
	rec_info->ucVdoFps = pvdo->ucFps;
	rec_info->uiAdoFps = pado->uiSampleRate*(pado->ucBitsPerSample>>3)/pado->uiPacketSize;
	
	if (pdPASS != xTaskCreate(task_write_cmd, "task_write_cmd", 512, rec_info,
			100, &rec_info->WriteCmdtask))
	{
		REC_DBG("Could not create task write cmd\n");
		goto fail6;
	}

	rec_info->VdoListMutex = xSemaphoreCreateMutex();
	if(NULL == rec_info->VdoListMutex)
	{
		REC_DBG("sempphore rec_info.VdoListMutex create fail\n"); 
		goto fail5;
	}

	rec_info->AdoListMutex = xSemaphoreCreateMutex();
	if(NULL == rec_info->AdoListMutex)
	{
		REC_DBG("sempphore rec_info.AdoListMutex create fail\n"); 
		goto fail4;
	}		
#endif

	for(i=0;i<MAX_SEMA_NUM;i++)
	{
		rec_info->CmdQueueSema[i] = xSemaphoreCreateBinary();
		if(NULL == rec_info->CmdQueueSema[i])
		{
			REC_DBG("sempphore rec_info.CmdQueueSema create fail\n"); 
			tmp = i;
			goto fail3;
		}		
		//xSemaphoreTake(rec_info->CmdQueueSema[i],0);
	}
	tmp = MAX_SEMA_NUM;


	rec_info->mutex = xSemaphoreCreateMutex();

	if(NULL == rec_info->mutex)
	{
		REC_DBG("mutex create fail\n");
		goto fail3;

	}

	rec_info->RecordActQueue = xQueueCreate(CMD_QUEUE_BUF, sizeof(record_cmd_queue_t));
	if(NULL == rec_info->RecordActQueue)
	{
		REC_DBG("RecvActQueue create fail\n");
		goto fail2;
	}

	rec_info->FileSyncMutex = xSemaphoreCreateMutex();

	if(NULL == rec_info->FileSyncMutex)
	{
		REC_DBG("FileSyncMutex create fail\n");
		goto fail1;
	}
	
	rec_info->rec_start_ret = WB_CMD_INIT;
	rec_info->rec_next_ret = WB_CMD_INIT;
	rec_info->rec_stop_ret = WB_CMD_INIT;

	//start record task
	sprintf(task_name,"rec_%x", (int)rec_info);
	if (pdPASS != xTaskCreate(task_write_file, task_name, 512, rec_info,
			100, &rec_info->WriteFiletask))
	{
		REC_DBG("Could not create task record\n");
		goto fail0;
	}

	*prec_info = rec_info;

	rec_info->ucRecStatus = RCT_CLOSE;
	REC_DBG("RCT_CLOSE !!\n");
	
	return pdPASS;

	vTaskDelete(rec_info->WriteFiletask);
	
fail0:
	vSemaphoreDelete(rec_info->FileSyncMutex);	
fail1:	
	vQueueDelete(rec_info->RecordActQueue);
fail2:		
	vSemaphoreDelete(rec_info->mutex);
fail3:
	for(i=0;i<tmp;i++)
		vSemaphoreDelete(rec_info->CmdQueueSema[i]);	

#if PRERECORD
	vSemaphoreDelete(rec_info->AdoListMutex);
fail4:
	vSemaphoreDelete(rec_info->VdoListMutex);
fail5:
	vTaskDelete(rec_info->WriteCmdtask);
fail6:
#endif
	if(rec_info->rec_type == RECORD_AVI)
		avi_uninit(&rec_info->AviInfo);
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4)
		mp4_uninit(&rec_info->Mp4Info);
#endif	
	//else
	//	writebuf_uninit(&rec_info->WBInfo);
fail7:
#if PRERECORD
	buflist_uninit(&rec_info->AdoList);
fail8:
	buflist_uninit(&rec_info->VdoList);
fail9:
#endif
	buflist_uninit(&rec_info->AVBufList);
fail10:
	vSemaphoreDelete(rec_info->AVListMutex);
fail11:
	vPortFree(rec_info);
fail12:
	return pdFAIL;
}

/**
* @brief interface function - record uninitialization
* @param prec_info pointer for record structure
*/
int record_uninit(record_info_t* rec_info)
{	
	int i, ret = 0;
	
	if(!rec_info)
		return pdFAIL;
	while(rec_info->ucRecStatus != RCT_CLOSE){
		vTaskDelay( 100/portTICK_PERIOD_MS );
		ret ++;
		if(ret >= 100){
			REC_DBG("Could not uinit record task\n");
			return pdFAIL;
		}
	}
	vTaskDelete(rec_info->WriteFiletask);
	vSemaphoreDelete(rec_info->FileSyncMutex);
	vQueueDelete(rec_info->RecordActQueue);
	vSemaphoreDelete(rec_info->mutex);
	for(i=0;i<MAX_SEMA_NUM;i++)
		vSemaphoreDelete(rec_info->CmdQueueSema[i]);
	if(rec_info->rec_type == RECORD_AVI)
		avi_uninit(&rec_info->AviInfo);
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4)
	{
		mp4_save_trailer_uninit(&rec_info->Mp4Info);	
		mp4_uninit(&rec_info->Mp4Info);	
	}
#endif	

	xSemaphoreTake(rec_info->AVListMutex, portMAX_DELAY);
	buflist_uninit(&rec_info->AVBufList);
	vSemaphoreDelete(rec_info->AVListMutex);
#if PRERECORD
	vTaskDelete(rec_info->WriteCmdtask);
	xSemaphoreTake(rec_info->AdoListMutex, portMAX_DELAY);
	buflist_uninit(&rec_info->AdoList);
	xSemaphoreTake(rec_info->VdoListMutex, portMAX_DELAY);
	buflist_uninit(&rec_info->VdoList);
	vSemaphoreDelete(rec_info->AdoListMutex);
	vSemaphoreDelete(rec_info->VdoListMutex);
#endif
	vPortFree(rec_info);
	return pdPASS;
}

/**
* @brief interface function - release write buffer
* @param rec_info pointer for record structure
*/
int record_writebuf_release(record_info_t* rec_info)
{
	if(rec_info->rec_type == RECORD_AVI)
		return writebuf_wb_release(&(rec_info->AviInfo.WBInfo));
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4)
		return writebuf_wb_release(&(rec_info->Mp4Info.WBInfo));
#endif

	return -1;
}

/**
* @brief interface function - create write buffer
* @param rec_info pointer for record structure
*/
int record_writebuf_create(record_info_t* rec_info)
{
	if(rec_info->rec_type == RECORD_AVI)
		return writebuf_wb_create(&(rec_info->AviInfo.WBInfo), &(rec_info->AviInfo.WBInitInfo));
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4)
		return writebuf_wb_create(&(rec_info->Mp4Info.WBInfo), &(rec_info->Mp4Info.WBInitInfo));
#endif
	
	return -1;	
}


void task_send_next_cmd( void *pvParameters ){
	record_info_t* rec_info = (record_info_t*)pvParameters;

	add_rec_cmd(rec_info, REC_ACT_NEXT, 0,NULL,0, 0);
	add_rec_cmd(rec_info, REC_ACT_START, 0,NULL,0, 0);

	vTaskDelete(NULL);
}

const char *get_lost_data_path(){
	return lost_path;
}

void set_lost_data_flag(int val){
	lost_flag = val;
}

static int gen_file_name(record_info_t* rec_info){
	int ret;
	void *arg;
	
	if(rec_info->rec_type == RECORD_AVI){
		ret = snx_fm_create_file(rec_info->record_file_type, avi_get_inteval_file_name(&rec_info->AviInfo), AVI_FILENAME_LENGTH, arg);
		if(ret != FM_OK){
			REC_DBG("Create filename error %d, %s!\n", ret, __FUNCTION__);
			return -1;
		}
	}
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4){
		ret = snx_fm_create_file(rec_info->record_file_type, mp4_get_inteval_file_name(&rec_info->Mp4Info), MP4_FILENAME_LENGTH, arg);
		if(ret != FM_OK){
			REC_DBG("Create filename error %d, %s!\n", ret, __FUNCTION__);
			return -1;
		}
	}
#endif	
	else{
		ret = snx_fm_create_file(rec_info->record_file_type, rec_info->IntevalFileName, RECORD_FILENAME_LENGTH, arg);
		if(ret != FM_OK){
			REC_DBG("Create filename error %d, %s!\n", ret, __FUNCTION__);
			return -1;
		}
	}
	
	return -1;
}

/**
* @brief interface function - notice to record video data
* @param prec_info pointer for record structure
* @param IFrame is I frame or not
* @param pFrame pointer for frame data
* @param uiFrameSize frame size
* @param tval frame timestamp
*/
#if PRERECORD
void record_video(record_info_t* rec_info, unsigned char IFrame, unsigned char *pFrame, unsigned int uiFrameSize, struct timeval tval)
{
	unsigned char cmd = REC_ACT_START;
	RECORD_CHECK(rec_info);

	//pre-setting
	if(rec_info->ucRecStart) 
	{
		if(IFrame){ //wait for i frame
			rec_info->ucIsRecording = 1;
		}
	}
	else if(rec_info->ucRecStop)
	{
		if(rec_info->uclast_cmd != REC_ACT_STOP){
			cmd = REC_ACT_STOP;
			rec_info->uclast_cmd = cmd;
		}
	}
	else if(rec_info->ucRecNext)
	{
		if((rec_info->uclast_cmd != REC_ACT_NEXT) && (IFrame)){
			cmd = REC_ACT_NEXT;
			rec_info->uclast_cmd = cmd;
		}
	}
	else
	{
		rec_info->uclast_cmd = REC_ACT_START;
	}

	record_save_data(rec_info, pFrame, uiFrameSize, IFrame, TYPE_VIDEO, cmd, tval);
}

#else
//vicky
//void record_video(record_info_t* rec_info, unsigned char IFrame, unsigned char *pFrame, unsigned int uiFrameSize, struct timeval tval)
void record_video(record_info_t* rec_info, unsigned char IFrame, unsigned char *pFrame, unsigned int uiFrameSize)
{
	uint64_t time = 0;

	RECORD_CHECK(rec_info);
	
	//vicky
	//time = tval.tv_sec*1000000+(tval.tv_usec+500);
	
	//pre-setting
	if(rec_info->ucRecStart&REC_CHECK_I_FRAME) 
	{
		if(IFrame) //wait for i frame
		{
			rec_info->ucIsRecording = 1;
			add_rec_cmd(rec_info, REC_ACT_START, 0, NULL, 0, time);
		}
	}
	/*
	else if(rec_info->ucRecStop)
	{
		rec_info->ucIsRecording = 0;
		add_rec_cmd(rec_info, REC_ACT_STOP, 0,NULL,0, time);

	}*/
	else if((rec_info->ucRecNext & REC_CHECK_I_FRAME)&&(rec_info->ucRecWaitFileClose==0))
	{
		if(IFrame)
		{
			rec_info->ucRecWaitFileClose = 1;
			rec_info->ucRecCache = REC_NEED_CACHE_FRAME;
			//add_rec_cmd(rec_info, REC_ACT_NEXT, 0,NULL,0, time);
			//add_rec_cmd(rec_info, REC_ACT_START, 0,NULL,0, time);
			gen_file_name(rec_info);
//			vicky
//			get_date(&rec_info->date);
			
			if (pdPASS != xTaskCreate(task_send_next_cmd, "task_send_next_cmd", 512, rec_info,
					100, NULL))
			{
				REC_DBG("Could not create task send next cmd\n");
				rec_info->ucRecCache = REC_NO_CACHE_FRAME;
				add_rec_cmd(rec_info, REC_ACT_NEXT, 0,NULL,0, time);
				add_rec_cmd(rec_info, REC_ACT_START, 0,NULL,0, time);
			}
		}	
	}

	//record video
    if(rec_info->rec_start_ret==WB_CMD_SUCCESS)
    {
		if(rec_info->ucRecCache == REC_NO_CACHE_FRAME){
			if(rec_info->ucIsRecording)
				add_rec_cmd(rec_info, REC_ACT_RECVID, IFrame, pFrame, uiFrameSize, time);

		}else{
			xSemaphoreTake(rec_info->AVListMutex, portMAX_DELAY);
	
#if USE_FIXED_MEM
	
			if(uiFrameSize < ( buflist_get_size()-(rec_info->AVBufList.size) )){

				buflist_t buf_tmp;
				
				//vicky
				//struct timeval cur_tv;
				//gettimeofday(&cur_tv, NULL);
				
				buf_tmp.data = pFrame;
				buf_tmp.size = uiFrameSize;
				buf_tmp.iframe = IFrame;
				//buf_tmp.action = cmd;
				buf_tmp.type = 0;
				
				//vicky
				//buf_tmp.time = cur_tv.tv_sec*1000000+(cur_tv.tv_usec+500);
				buf_tmp.time = 0;

				if(rec_info->ucRecCache == REC_NEED_CACHE_FRAME){
					buflist_insert(&rec_info->AVBufList, 1, buf_tmp);
				}
				else{
					if(rec_info->ucIsRecording){
						add_rec_cmd(rec_info, REC_ACT_RECVID, IFrame, pFrame, uiFrameSize, time);
					}
				}

#else
			len = buflist_get_length(&rec_info->AVBufList);
			if(len < rec_info->max_av_length){
				unsigned char *mem;
				buflist_t buf_tmp;
				struct timeval cur_tv;

#if defined( CONFIG_SN_GCC_SDK )
				mem = (unsigned char *)pvPortMalloc(uiFrameSize, GFP_KERNEL, MODULE_MID_RECORD);
#else
				mem = (unsigned char *)pvPortMalloc(uiFrameSize);
#endif
				
				if(mem == NULL){
					REC_DBG("insert data allocate fail\n");
				}
				gettimeofday(&cur_tv, NULL);
				buf_tmp.data = mem;
				buf_tmp.size = uiFrameSize;
				buf_tmp.iframe = IFrame;
				//buf_tmp.action = cmd;
				buf_tmp.type = 0;
				buf_tmp.time = cur_tv.tv_sec*1000000+(cur_tv.tv_usec+500);

				if(rec_info->ucRecCache == REC_NEED_CACHE_FRAME){
					memcpy(mem, pFrame, uiFrameSize);
					buflist_insert(&rec_info->AVBufList, 1, buf_tmp);
				}else{
					vPortFree(mem);
					if(rec_info->ucIsRecording){
						add_rec_cmd(rec_info, REC_ACT_RECVID, IFrame, pFrame, uiFrameSize, time);
					}
				}
#endif
			REC_DBG("insert video size %d  iframe %d type %d time %d\n",buf_tmp.size,buf_tmp.iframe, buf_tmp.type,buf_tmp.time);
			REC_DBG("%p buflist_is_empty %d    buflist_get_length %d++++++++++\n",&rec_info->AVBufList, buflist_is_empty(&rec_info->AVBufList),buflist_get_length(&rec_info->AVBufList));
			}else{
				if(lost_flag != 0){
					FIL fd;
					FILINFO fno;
					FRESULT ret;
					char tmp_p[256];
					if(exists(lost_path)==0)
					{
						mkdir(lost_path);
					}
					snprintf(tmp_p,256,"%s/%04d_%02d_%02d_%02d_%02d_%02d.lostdata",lost_path,
			rec_info->date.year,rec_info->date.month,rec_info->date.day,rec_info->date.hour,rec_info->date.minute,rec_info->date.second);   
					if(f_stat(tmp_p,&fno) != FR_OK){
						if((ret = f_open(&fd, tmp_p, FA_WRITE|FA_CREATE_ALWAYS)) != FR_OK)
						{
							REC_DBG("open lostdata file fail: %s %d\n", tmp_p, ret);
							xSemaphoreGive(rec_info->AVListMutex);
							return;
						}
						f_close(&fd);
					}
				}
			}
			xSemaphoreGive(rec_info->AVListMutex);
			
		}
    }

/*
	//move to callback function
	//post-setting to clear flag
	if(rec_info->ucRecStart) 
	{
		if(IFrame)
		{
			rec_info->ucRecStart = 0;
		}
	}
	else if(rec_info->ucRecStop)
	{
		rec_info->ucRecStop = 0;
	}
	else if(rec_info->ucRecNext)
	{
		if(IFrame)
		{
			rec_info->ucRecNext = 0;
		}
	}
*/	

}
#endif

/**
* @brief interface function - notice to record audio data
* @param prec_info pointer for record structure
* @param pFrame pointer for each frame data
* @param uiFrameSize frame size
* @param tval frame timestamp
*/
//void record_audio(record_info_t* rec_info, unsigned char *pFrame, unsigned int uiFrameSize, struct timeval tval)
void record_audio(record_info_t* rec_info, unsigned char *pFrame, unsigned int uiFrameSize)
{
//#if	REC_HAS_AUDIO
	RECORD_CHECK(rec_info);

#if PRERECORD
	record_save_data(rec_info, pFrame, uiFrameSize, 1, TYPE_AUDIO, REC_ACT_RECADO);

#else
	//struct timeval cur_tv;
	uint64_t time = 0;

	//gettimeofday(&cur_tv, NULL);
	
	//vicky
	//time = tval.tv_sec*1000000+(tval.tv_usec+500);
	if(rec_info->rec_start_ret==WB_CMD_SUCCESS) {
		if(rec_info->ucRecCache == REC_NO_CACHE_FRAME){
			if(rec_info->ucIsRecording)
				add_rec_cmd(rec_info, REC_ACT_RECADO, 1, pFrame, uiFrameSize, time);
		}else{
			xSemaphoreTake(rec_info->AVListMutex, portMAX_DELAY);

#if USE_FIXED_MEM
			if(uiFrameSize < ( buflist_get_size()-(rec_info->AVBufList.size) )){
				buflist_t buf_tmp;
				
				//vicky
				//struct timeval cur_tv;
				//gettimeofday(&cur_tv, NULL);
				
				buf_tmp.data = pFrame;
				buf_tmp.size = uiFrameSize;
				buf_tmp.iframe = 1;
				buf_tmp.type = 1;
				
				//vicky
				//buf_tmp.time = cur_tv.tv_sec*1000000+(cur_tv.tv_usec+500);
				buf_tmp.time = 0;

				if(rec_info->ucRecCache == REC_NEED_CACHE_FRAME)
					buflist_insert(&rec_info->AVBufList, 1, buf_tmp);
				else{
					if(rec_info->ucIsRecording){
						add_rec_cmd(rec_info, REC_ACT_RECADO, 1, pFrame, uiFrameSize, time);
					}
				}
			}
#else
			len = buflist_get_length(&rec_info->AVBufList);
			if(len < rec_info->max_av_length){
				unsigned char *mem;
				buflist_t buf_tmp;
				struct timeval cur_tv;

#if defined( CONFIG_SN_GCC_SDK )
				mem = (unsigned char *)pvPortMalloc(uiFrameSize, GFP_KERNEL, MODULE_MID_RECORD);
#else
				mem = (unsigned char *)pvPortMalloc(uiFrameSize);
#endif				
				if(mem == NULL){
					MID_RECORD_PRINT_QUEUE("insert data allocate fail\n");
				}
				gettimeofday(&cur_tv, NULL);
				buf_tmp.data = mem;
				buf_tmp.size = uiFrameSize;
				buf_tmp.iframe = 1;
				buf_tmp.type = 1;
				buf_tmp.time = cur_tv.tv_sec*1000000+(cur_tv.tv_usec+500);

				if(rec_info->ucRecCache == REC_NEED_CACHE_FRAME){
					memcpy(mem, pFrame, uiFrameSize);
					buflist_insert(&rec_info->AVBufList, 1, buf_tmp);
				}else{
					vPortFree(mem);
					if(rec_info->ucIsRecording){
						add_rec_cmd(rec_info, REC_ACT_RECADO, 1, pFrame, uiFrameSize, time);
					}
				}
			}else{
			
			}//MID_RECORD_PRINT_QUEUE("insert audio size %d  iframe %d type %d time %d\n",buf_tmp.size,buf_tmp.iframe, buf_tmp.type,buf_tmp.time);
#endif // USE_FIXED_MEM end
			xSemaphoreGive(rec_info->AVListMutex);
		}
	}

#endif
}

/**
* @brief interface function - start record
* @param prec_info pointer for record structure
* @param is_block for 1:blocking 0:nonblocking
* @return return pdPASS if success
*/
int record_set_start(record_info_t* rec_info, int is_block)
{
	RECORD_CHECK(rec_info);
	REC_DBG("is_block = %d\n", is_block);

	if(rec_info->rec_type == RECORD_AVI){
		avi_clear_inteval_file_name(&rec_info->AviInfo);
	}
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4){
		mp4_clear_inteval_file_name(&rec_info->Mp4Info);
	}
#endif	
	else{
		memset(rec_info->IntevalFileName, 0, RECORD_FILENAME_LENGTH);
	}

	if(is_block)
	{
		//vicky --
		//mf_video_h264_set_iframe_for_record();
		rec_info->ucRecStart = REC_CHECK_I_FRAME;
	}
	else
	{
		rec_info->ucRecStart = REC_NO_CHECK_FRAME;
		rec_info->ucIsRecording = 1;
		add_rec_cmd(rec_info, REC_ACT_START, 0,NULL,0, 0);
	}
	
	/*while(rec_info->ucRecStart)
		vTaskDelay(10 / portTICK_RATE_MS );
	REC_DBG("finish\n");*/

	// the following two loops make sure the cmd REC_ACT_START is proceeded successfully
	while(rec_info->ucRecStart) { 
		vTaskDelay(10 / portTICK_RATE_MS );
	}

	//REC_DBG(" @@  set_start_finish is called\n");

	while(rec_info->ucRecStatus != RCT_CAPTURE) { 
		vTaskDelay(10 / portTICK_RATE_MS );
		if(rec_info->rec_start_ret == WB_CMD_FAIL)
			break;
	}
	
	REC_DBG("finish\n");
	return rec_info->rec_start_ret;	
}

/**
* @brief interface function - close current record file and then create new record file to saving stream
* @param prec_info pointer for record structure
* @param is_block for 1:blocking 0:nonblocking
* @return return pdPASS if success
*/
int record_set_next(record_info_t* rec_info, int is_block)
{
	RECORD_CHECK(rec_info);
	REC_DBG("is_block = %d\n", is_block);
	
	if(is_block)
	{
		char tmp_p[256];
		system_date_t date;	
//			vicky
//			get_date(&date);
			snprintf(tmp_p,256,"%04d_%02d_%02d_%02d_%02d_%02d.???",
				date.year,date.month,date.day,date.hour,date.minute,date.second); 
		//vicky --
		//mf_video_h264_set_iframe_for_record();
		rec_info->ucRecNext = REC_CHECK_I_FRAME;
		REC_DBG("-------FRAMES IS %d  time is %s\n",rec_info->AviInfo.NumFrame[0],tmp_p);
	}
	else
	{
		rec_info->ucRecCache = REC_NEED_CACHE_FRAME;
		rec_info->ucRecNext = REC_NO_CHECK_FRAME;
		//add_rec_cmd(rec_info, REC_ACT_NEXT, 0,NULL,0, 0);
		//add_rec_cmd(rec_info, REC_ACT_START, 0,NULL,0, 0);
		
		gen_file_name(rec_info);
//		vicky
//		get_date(&rec_info->date);
		if (pdPASS != xTaskCreate(task_send_next_cmd, "task_send_next_cmd", 512, rec_info,
					100, NULL))
		{
			REC_DBG("Could not create task send next cmd\n");
			rec_info->ucRecCache = REC_NO_CACHE_FRAME;
			add_rec_cmd(rec_info, REC_ACT_NEXT, 0,NULL,0, 0);
			add_rec_cmd(rec_info, REC_ACT_START, 0,NULL,0, 0);
		}
	}	
	while(rec_info->ucRecNext)
		vTaskDelay(10 / portTICK_RATE_MS );
	REC_DBG("finish\n");
	return rec_info->rec_next_ret;		
}


/**
* @brief interface function - stop record
* @param prec_info pointer for record structure
* @param is_block for 1:blocking 0:nonblocking
* @return return pdPASS if success
*/
int record_set_stop(record_info_t* rec_info, int is_block)
{
	//struct timeval tval;
	RECORD_CHECK(rec_info);
	//REC_DBG("rec_info->ucRecStatus %d\n",rec_info->ucRecStatus); 

	if(rec_info->ucRecStatus == RCT_CLOSE)
      return  rec_info->rec_stop_ret;

	while(rec_info->ucRecNext)
	{
		REC_DBG("wait next cmd finish\n");
		vTaskDelay(100 / portTICK_RATE_MS );
	}

	rec_info->ucRecStop = REC_NO_CHECK_FRAME;
	rec_info->ucIsRecording = 0;
	add_rec_cmd(rec_info, REC_ACT_STOP, 0,NULL,0, 0);
	while(rec_info->ucRecStop)
		vTaskDelay(100 / portTICK_RATE_MS );
	
	if(rec_info->rec_type == RECORD_AVI){
		avi_clear_inteval_file_name(&rec_info->AviInfo);
	}
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type == RECORD_MP4){
		mp4_clear_inteval_file_name(&rec_info->Mp4Info);
	}
#endif	
	else{
		memset(rec_info->IntevalFileName, 0, RECORD_FILENAME_LENGTH);
	}
	
	REC_DBG("record_set_stop : finish\n");
	return rec_info->rec_stop_ret;
	
}

/**
* @brief interface function - get reocrd status
* @param prec_info pointer for record structure
* @return 1:current is in recording, 0:current is not in recording
*/
int record_get_state(record_info_t* rec_info)
{
	RECORD_CHECK(rec_info);
	return rec_info->ucIsRecording;
}

/**
* @brief interface function - set prereocrd time
* @param prec_info pointer for record structure
* @param time prerecord time in second unit
* @return 1 if success
*/
int prerecord_set_time(record_info_t* rec_info, unsigned char time)
{
	RECORD_CHECK(rec_info);

	rec_info->ucPrerecordTime = time;
	
	return TRUE;
}

/**
* @brief interface function - get prereocrd time
* @param prec_info pointer for record structure
* @return prerecord time in second unit
*/
unsigned char prerecord_get_time(record_info_t* rec_info)
{
	RECORD_CHECK(rec_info);

	return rec_info->ucPrerecordTime;
}

/**
* @brief interface function - get last finished record file name
* @param prec_info pointer for record structure
* @return string for file name
*/
char* record_get_last_file_name(const record_info_t* rec_info)
{
	RECORD_CHECK(rec_info);
	return (char *)(rec_info->LastFileName);	
}


/**
* @brief interface function - get current record file name
* @param prec_info pointer for record structure
* @return string for file name
*/
char* record_get_cur_file_name(const record_info_t* rec_info)
{
	RECORD_CHECK(rec_info);
	if(rec_info->rec_type == RECORD_AVI)
		return avi_get_file_name((AVI_Info_t *)&rec_info->AviInfo);
#ifdef CONFIG_MP4		
	if(rec_info->rec_type == RECORD_MP4)
		return mp4_get_file_name((AVI_Info_t *)&rec_info->Mp4Info);
#endif	
	else
		return (char *)rec_info->FileName;	
}

/**
* @brief interface function - get recording information
* @param prec_info pointer for record structure
* @param pCurFileInfo pointer for the current record file info structure
* @param pLastFileInfo pointer for the last record file info structure
*/
int record_get_file_info(const record_info_t* rec_info, RecFileInfo_t *pCurFileInfo, RecFileInfo_t *pLastFileInfo)
{

	char *AviFname;
	int lastIndex;
#ifdef CONFIG_MP4		
	char *Mp4Fname;
#endif	

	RECORD_CHECK(rec_info);	
	memset(pCurFileInfo, 0, sizeof(RecFileInfo_t));
	memset(pLastFileInfo, 0, sizeof(RecFileInfo_t));
	
	xSemaphoreTake(rec_info->FileSyncMutex, portMAX_DELAY); 
	if(rec_info->rec_type == RECORD_AVI){
		AviFname = avi_get_file_name((AVI_Info_t *)&rec_info->AviInfo);
		lastIndex = (rec_info->AviInfo.CurIndex+1)%AVI_INDEX_TABLE_NUM;
		
		memcpy(pLastFileInfo->filename, rec_info->LastFileName,300);
		memcpy(pCurFileInfo->filename, AviFname, 300);	
		pLastFileInfo->pAviIndex = (AVIIndex_t *)&rec_info->AviInfo.AviIndex[lastIndex];
		pCurFileInfo->pAviIndex = (AVIIndex_t *)&rec_info->AviInfo.AviIndex[rec_info->AviInfo.CurIndex];
		pCurFileInfo->VdoFrameNum = rec_info->AviInfo.NumFrame[AVI_VIDEO];

		pCurFileInfo->AdoFrameNum = rec_info->AviInfo.NumFrame[AVI_AUDIO];
		pCurFileInfo->stop_status = rec_info->ucRecStop?1:0;
		//REC_DBG("index = %d, %d, frame = %d, %d\n", 
		//		rec_info->AviInfo.CurIndex, lastIndex, pCurFileInfo->VdoFrameNum, pCurFileInfo->AdoFrameNum);
	}
#ifdef CONFIG_MP4	
	else if(rec_info->rec_type == RECORD_MP4){
		Mp4Fname = mp4_get_file_name(&rec_info->Mp4Info);
	//	lastIndex = (rec_info->AviInfo.CurIndex+1)%AVI_INDEX_TABLE_NUM;
		
		memcpy(pLastFileInfo->filename, rec_info->LastFileName,300);
		memcpy(pCurFileInfo->filename, Mp4Fname, 300);	
	/*	pLastFileInfo->pAviIndex = &rec_info->AviInfo.AviIndex[lastIndex];
		pCurFileInfo->pAviIndex = &rec_info->AviInfo.AviIndex[rec_info->AviInfo.CurIndex];*/
		pCurFileInfo->VdoFrameNum = rec_info->Mp4Info.NumFrame[MP4_VIDEO];

		pCurFileInfo->AdoFrameNum = rec_info->Mp4Info.NumFrame[MP4_AUDIO];

		pCurFileInfo->stop_status = rec_info->ucRecStop?1:0;
	}
#endif	
	else{
		memcpy(pLastFileInfo->filename, rec_info->LastFileName,300);
		memcpy(pCurFileInfo->filename, rec_info->FileName, 300);
	}	

	xSemaphoreGive(rec_info->FileSyncMutex);

	return pdPASS;
}


/**
* @brief interface function - set record prefix name
* @param prec_info pointer for record structure
* @param prefix_name prefix name
*/
void record_set_prefix_name(record_info_t* rec_info, char* prefix_name)
{
	RECORD_CHECK(rec_info);
	strcpy(rec_info->prefix_name, prefix_name);
}


/**
* @brief interface function - set record_set_filename_format
* @param format for calcute the file length with count or time
*/

void record_set_filename_format(record_info_t* rec_info,enum RECORDFILE_TYPE format)
{
   rec_info->fileformat = format;

}

/**
* @brief interface function - set path for record file 
* @param prec_info pointer for record structure
* @param path string for path
*/
int record_set_path(record_info_t* rec_info, char* path)
{
	char *pStrEnd;
	RECORD_CHECK(rec_info);
	
	if(strlen(path) > (sizeof(rec_info->path)-2))
	{
		REC_DBG("path name too long\n");
		return pdFAIL;
	}
	strcpy(rec_info->path,path);
	pStrEnd = rec_info->path+strlen(rec_info->path)-1;
	if(*pStrEnd == '/')
		*pStrEnd = 0;

   if(rec_info->fileformat ==RECORD_COUNTFORMAT)
   {
	   record_get_record_count(rec_info);
   }		
   return pdPASS;		
}


#if PRERECORD
#define time_debug 0
/**
* @brief insert node to buffer list
* @param rec_info pointer for record structure
* @param data source data pointer 
* @param size source data size
* @param iframe data information about iframe 
* @param type data type video or audio
* @param cmd command for record action 
*/
void record_insert_data(record_info_t* rec_info, void *data, unsigned int size, unsigned char iframe, unsigned char type, unsigned char cmd)
{
	unsigned char *mem;
	buflist_t buf_tmp;
	struct timeval cur_tv;
	

#if time_debug
	TickType_t CurTick=0, tick;
#endif	
	
#if defined( CONFIG_SN_GCC_SDK )
	mem = (unsigned char *)pvPortMalloc(size, GFP_KERNEL, MODULE_MID_RECORD);
#else	
	mem = (unsigned char *)pvPortMalloc(size);
#endif
	
	if(mem == NULL){
		REC_DBG("insert data allocate fail\n");
	}
	gettimeofday(&cur_tv, NULL);
	buf_tmp.data = mem;
	buf_tmp.size = size;
	buf_tmp.iframe = iframe;
	buf_tmp.action = cmd;
	buf_tmp.time = cur_tv.tv_sec*1000+(cur_tv.tv_usec+500)/1000;

#if time_debug
	CurTick = xTaskGetTickCount();
#endif
	memcpy(mem, data, size);

#if time_debug
	tick = xTaskGetTickCount();
	
	if((tick - CurTick) > 1){
		REC_DBG(" ******* memory copy time =%d  size=%d\n", (tick - CurTick), size);
	}
#endif

	if(type == TYPE_VIDEO){
		xSemaphoreTake(rec_info->VdoListMutex, portMAX_DELAY);
		buflist_insert(&rec_info->VdoList, 1, buf_tmp);
		rec_info->iVdoListReadPosi++;
		xSemaphoreGive(rec_info->VdoListMutex);
	}
	else if(type == TYPE_AUDIO){
		xSemaphoreTake(rec_info->AdoListMutex, portMAX_DELAY);
		buflist_insert(&rec_info->AdoList, 1, buf_tmp);
		rec_info->iAdoListReadPosi++;
		xSemaphoreGive(rec_info->AdoListMutex);
	}
	else{
		REC_DBG("insert the wrong type\n");
		return;
	}
}

/**
* @brief delete node with buffer list
* @param rec_info pointer for record structure
* @param type data type video or audio
*/
void record_del_exc_data(record_info_t* rec_info, unsigned char type)
{
	int len;
	int mark_tmp;
	buflist_t *pList;
	xSemaphoreHandle *pListSema;
	unsigned int uiFps;
	
	if(type == TYPE_VIDEO){
		pList = &rec_info->VdoList;
		pListSema = &rec_info->VdoListMutex;
		uiFps = rec_info->ucVdoFps;
	}
	else if(type == TYPE_AUDIO){
		pList = &rec_info->AdoList;
		pListSema = &rec_info->AdoListMutex;
		uiFps = rec_info->uiAdoFps;
	}
	else{
		REC_DBG("type error\n");
		return;
	}	
	
	xSemaphoreTake(*pListSema, portMAX_DELAY);
	for(len = buflist_get_length(pList); len > 0; len--){
		mark_tmp = buflist_get_mark(pList, len);
		if((((rec_info->ucIsRecording == FALSE) && !((rec_info->ucFileOpen == 1) && (mark_tmp == 0))) || ((rec_info->ucIsRecording == TRUE) && (mark_tmp == 1)))
			&& (len > ((int)rec_info->ucPrerecordTime * uiFps))){
				
			len = buflist_get_length(pList);
			if(!buflist_delete(pList, len)){
				REC_DBG("%s buffer list delete fail. list position=%d\n", type==TYPE_VIDEO?"video":"audio", len);
			}
			//REC_DBG("%s list del=%d, IsRecording=%d, FileOpen=%d\n", type==TYPE_VIDEO?"video":"audio", len, (int)rec_info->ucIsRecording, (int)rec_info->ucFileOpen);
		}
		else{
			break;
		}
	}
	xSemaphoreGive(*pListSema);
}

/**
* @brief control list node with insert and delete
* @param rec_info pointer for record structure
* @param data source data pointer 
* @param size source data size
* @param iframe data information about iframe 
* @param type data type video or audio
* @param cmd command for record action 
*/
void record_save_data(record_info_t* rec_info, void *data, unsigned int size, unsigned char iframe, unsigned char type, unsigned char cmd)
{
	record_info_t *pTmp = rec_info;
	if((data == NULL) || (size <= 0)){
		return;
	}
	
	if(pTmp){
		if(((type == TYPE_AUDIO)&&(pTmp->AdoListMutex == NULL))||((type == TYPE_VIDEO)&&(pTmp->AdoListMutex == NULL))){
			return;
		}

		if(pTmp->ucPrerecordTime > 0){
			record_insert_data(pTmp, data, size, iframe, type, cmd);
		}else if(pTmp->ucIsRecording == TRUE){
			record_insert_data(pTmp, data, size, iframe, type, cmd);
		}
		
		record_del_exc_data(pTmp, type);
	}
}
#endif
/** @} */

char record_get_wb_status(record_info_t* rec_info)
{
	if(rec_info->rec_type==RECORD_AVI)
		return avi_get_wb_status(&rec_info->AviInfo);
#ifdef CONFIG_MP4		
	else if(rec_info->rec_type==RECORD_MP4)
		return mp4_get_wb_status(&rec_info->Mp4Info);
#endif
	else
		return 0;
}

#endif
