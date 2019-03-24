/*********************************************************************************
* sdcard_record.c
*
* Implementation of  recording  internal APIs
*
* History:
*    2015/10/26 - [Allen_Chang] created file
*
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.
*
*********************************************************************************/
#include "sonix_config.h"

#if defined( CONFIG_RECORD )

#include <FreeRTOS.h>
#include <task.h>
//#include <bsp.h>
//#include <timers.h>
//#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
#include <sys_clock.h>
//#include <sys/time.h>
//#include <libmid_nvram/snx_mid_nvram.h>
//#include <generated/snx_sdk_conf.h>
#include "rec_common.h"
//#include "debug.h"
#include "rec_schedule.h"
#include "record.h"
#include "avi_rec.h"


#if defined(CONFIG_AUTOMOUNT)
#include <automount.h>
#endif

//#include "../video/video_main.h"
//#include "../audio/audio_main.h"
//#include "rec_protect.h"
//#include "../daemon/json_cmd.h"
//#include "watch_task.h"
//#include "user_param.h"
//#include "../sensor_cap/sensor_capability.h"
//#include "upload1.h"
//#include "../playback/play_back.h"

//#ifndef CONFIG_APP_DRONE
//#include "mcu.h"
//#endif

//#include "rec_seed.h"

/**
 *  \ingroup rec_schedule
 *  @{
 */

extern char **g_Filelist;
extern int  g_Filelistsize;
recordinfo_t *record_info = NULL;
#ifdef CONFIG_MP4_RECOVER
static int gCloseRecover = 0;			/* notify recover task to terminate or not */
static int gMp4RecoverRun = 0;			/* notify recover task to start recovering or not */
#endif

#if defined(CONFIG_AUTOMOUNT)
automount_info_t *sdaumt_record_info;
#endif

static int userdisablerecord = 0;
#ifndef CONFIG_APP_DRONE
static int firststarttask = 0;
#endif
static system_date_t dev_time;
//vicky
//static chk_card_file_info_t card_info;

static void task_record_test( void *pvParameters );
static void task_record_uvc(void *pvParameters);

static void chk_userparam(recordinfo_t *sdrecord_info);
static void rm_file_task(void *pvParameters);
#ifdef CONFIG_MP4_RECOVER
static void rec_recover_task(void *parameter);
#endif

//#define BIT(x)												(1UL << (x))

#define	IFRAME_FILE_SIZE							165984
#define	IFRAME_DATA_BUF_SIZE					(256*1024)	

FIL 					IFrameFile;
unsigned char	*IFrameDataBuf = NULL;
RecVdoInfo_t 	video_info;

void task_record_video( void *pvParameters );
	

int readIFramData()
{
	int ret;	
	void *pbuf;
	char * filename;
	uint32_t datasize = IFRAME_FILE_SIZE;
	uint32_t readsize = 0;
	
	IFrameDataBuf = pvPortMalloc(IFRAME_DATA_BUF_SIZE);
	memset(IFrameDataBuf, 0x00, IFRAME_DATA_BUF_SIZE);
	
//	if( (ret = f_open(&IFrameFile, "./I_frame", FA_OPEN_EXISTING|FA_READ)) != FR_OK )
	if( (ret = f_open(&IFrameFile, "1:I_frame", FA_OPEN_EXISTING|FA_READ)) != FR_OK )
	{
		REC_DBG("file %s open fail(%d)\n", filename, ret);
		return ret;
	}	
	
	ret = f_read(&IFrameFile, IFrameDataBuf, datasize, &readsize);
	if( (ret != FR_OK) || (readsize != datasize) )
	{
		f_close(&IFrameFile);
		REC_DBG("file %s read fail(%d) \n", filename, ret);
		return ret;
	}	
	REC_DBG("I_Frame size is %d\n", readsize);
	
	f_close(&IFrameFile);
	
	return FR_OK;
}

/**
 * @brief setting middleware recording wribuffer size by resolution
 * @param writebufferparam pointer for middleware writebuffer stucture
 * @param pv_param video record information
 */
static void set_rec_writebuffer(RecWirteBufferInitInfo_t *writebufferparam, RecVdoInfo_t *pv_param)
{
	if ((pv_param->width >= FHD_WIDTH) && (pv_param->height >= FHD_HEIGHT))
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661)
		writebufferparam->write_buf_size = 128 * 40 * 1024;
#else
#if defined(CONFIG_SYSTEM_PLATFORM_SN98672)
		writebufferparam->write_buf_size = 128 * 20 * 1024;
#else
		writebufferparam->write_buf_size = 128 * 20 * 1024;
#endif
#endif
	else if ((pv_param->width >= HD_WIDTH) && (pv_param->height >= HD_HEIGHT))
		writebufferparam->write_buf_size = 128 * 12 * 1024;
	else if ((pv_param->width >= VGA_WIDTH) && (pv_param->height >= VGA_HEIGHT))
		writebufferparam->write_buf_size = 128 * 10 * 1024;
	else
		writebufferparam->write_buf_size = 128 * 10 * 1024;

#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661)
	writebufferparam->buflist_size = 1 * 1024 * 1024;
#else
	writebufferparam->buflist_size = 1 * 1024 * 1024;
#endif

	writebufferparam->write_unit_to_file = 0x20000;	//0x20000;
}

void rec_video_init(void)
{
	memset(&video_info, 0, sizeof(RecVdoInfo_t));
	video_info.width = 640;						//320; 640; 1280; 1920
	video_info.height = 480;					//240; 480;  720; 1080
	video_info.ucFps = 30;	
	video_info.uiBps = 8*1024*1024;	//10*1024*1024;	8*1024*1024;
	video_info.ucScale = 1;						//x1
	video_info.ucStreamMode = REC_FMT_MJPEG;	//REC_FMT_MJPEG;	//REC_FMT_H264;
}

/**
 * @brief schedual record init
 * @param info pointer for recordinfo_t  structure
 * @return 1 if record init success
 * @note get video & audio param and init middleware record
 */
//static int rec_record_init(recordinfo_t *info)
int rec_record_init(recordinfo_t *info)
{
	int 		record_type;
	int 		ret = 0;
	struct 	usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0)
	{
		REC_DBG("get usr config failed\n");
		goto finish;
	}
	
	info->RecParam.vdo.width = video_info.width;
	info->RecParam.vdo.height = video_info.height;
	info->RecParam.vdo.ucFps = video_info.ucFps;
	info->RecParam.vdo.uiBps = video_info.uiBps;
	info->RecParam.vdo.ucScale = video_info.ucScale;
	info->RecParam.vdo.ucStreamMode = video_info.ucStreamMode;

	info->RecParam.ado.uiFormat = AUD_FORMAT_S16_LE;
	info->RecParam.ado.uiSampleRate = 8000;
	info->RecParam.ado.ucBitsPerSample = 8;
	info->RecParam.ado.uiPacketSize = 160;
	info->RecParam.max_record_len = 180; // 3mins
	
#if 0	
#ifndef CONFIG_APP_DRONE
	info->RecParam.ado.uiFormat = aparam.uiFormat;
	info->RecParam.ado.uiSampleRate = aparam.uiSampleRate;
	info->RecParam.ado.ucBitsPerSample = aparam.ucBitsPerSample;
	info->RecParam.ado.uiPacketSize = aparam.uiPacketSize;
	info->RecParam.ado.uiBitRate = aparam.uiBitRate;
#else
	info->RecParam.ado.uiFormat = 1 << 20;		// AUD_FORMAT_AAC;
	info->RecParam.ado.uiSampleRate = 11025;
	info->RecParam.ado.ucBitsPerSample = 16;
	info->RecParam.ado.uiPacketSize = 2048;
	info->RecParam.ado.uiBitRate = 15999;
#endif
#endif	//end of #if 0	

//	info->RecParam.max_record_len = info->sched_p->sd_sched_record_interval;
	info->type = T_RECORD;
	set_rec_writebuffer(&(info->RecParam.writebufferparam), &video_info);

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
	if ((info->RecParam.vdo.width == FHD_WIDTH) && (info->RecParam.vdo.height == FHD_HEIGHT))
		set_dwnlod_and_pb_max_conn_num(1);
	else if ((info->RecParam.vdo.width == HD_WIDTH) && (info->RecParam.vdo.height == HD_HEIGHT))
		set_dwnlod_and_pb_max_conn_num(2);
	else
		set_dwnlod_and_pb_max_conn_num(4);
#endif

	record_type = RECORD_AVI;
	
#ifdef CONFIG_MP4_RECOVER
	if (record_type == RECORD_MP4) {
		info->RecParam.recover.state = 1;			/* turn on recover method */
		info->RecParam.recover.presion = 3;			/* recover presion degree in 3 seconds */
		info->RecParam.recover.save_path = NULL;	/* default reover path */
	}

	gCloseRecover = 0;
	gMp4RecoverRun = 0;
	if (pdPASS != xTaskCreate(rec_recover_task, "recover_task", STACK_SIZE_8K, NULL, PRIORITY_TASK_APP_REC_FLOW, &info->recover_task)) {
		REC_PRINT(SYS_ERR, "cannot create recover_task\n");
		goto finish;
	}
#endif

#ifndef CONFIG_APP_DRONE
	ret = record_init(&(info->pRecord_info), &info->RecParam, 1, record_type, 1, NULL, NULL, 0, T_RECORD);
#else
	ret = record_init(&(info->pRecord_info), &info->RecParam, 1, record_type, 0, rec_finishedfile_addlen, update_seed, MAX_PRE_TIME, T_RECORD);
#endif
	if (ret == pdPASS) {
		//record_set_prefix_name(info->pRecord_info, pUserCfg->rec_prefix);
		record_set_prefix_name(info->pRecord_info, "UVC");
//		REC_DBG("info->fileformat = %d", info->fileformat);
		if (info->fileformat == RECORD_TIMEFORMAT) {
			record_set_filename_format(info->pRecord_info, RECORD_TIMEFORMAT);
		} else if (info->fileformat == RECORD_COUNTFORMAT) {
			record_set_filename_format(info->pRecord_info, RECORD_COUNTFORMAT);
		}
	} else
		goto finish;

finish:
	return ret;
}

/**
 *@brief Uninit SDRecordInfo structure
 *
 */
//static void rec_info_uninit(recordinfo_t **info)
void rec_info_uninit(recordinfo_t **info)
{
	safeFree(*info);
}

/**
 * @brief init recordinfo_t structure
 * @return return recordinfo_t pointer if success
 */
//static recordinfo_t *rec_info_init(void)
recordinfo_t *rec_info_init(void)
{
	int 							ret = 0;
	recordinfo_t 			*info;	
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_DBG("get usr config failed\n");
		goto finally;
	}

#if defined( CONFIG_SN_GCC_SDK )	
	if (!(info = (recordinfo_t *)pvPortMalloc(sizeof(recordinfo_t), GFP_KERNEL, MODULE_APP))) {
#else
	if (!(info = (recordinfo_t *)pvPortMalloc(sizeof(recordinfo_t)))) {
#endif		
		REC_DBG("sd recrod info init fail\n");
		goto finally;
	}
	
	memset(info, 0, sizeof(recordinfo_t));

	info->pRecord_info = NULL;
	info->sched_p = NULL;
	info->lastsecond = 0;
	info->recordstatus = 0;
	info->recordclose = 0;
	info->gpscount = 0;
	info->gsensorcount = 0;
	info->sdcardisfull = 0;
	info->sdcardseed = 0;
	info->rm_queue = xQueueCreate(RM_MAX_QUEUE_ITEM, sizeof(rm_info_t));
	if (NULL == info->rm_queue) {
		REC_DBG("queue rm_queue create fail\n");
		goto finally;
	}
		
//	if (pdPASS != xTaskCreate(rm_file_task, "rm_file_task", 1024, NULL, 35, &(info->rm_file_task))) {
//		REC_DBG("cannot create rm_file_task\n");
//		goto finally1;
//	}
	
	memset(info->schedpath, 0x00, sizeof(info->schedpath));
	snprintf(info->schedpath, sizeof(info->schedpath), "%s", pUserCfg->rec_path);
	return info;

finally1:
	if (info->rm_queue != NULL)
		vQueueDelete(info->rm_queue);
finally:
	if (info != NULL)
		rec_info_uninit(&info);
	return NULL;
}

/**
 * @brief interface function-rm oldest file
 * @note it is too long time to rm file , so move to task to handle
 */
static void rm_file_task(void *pvParameters)
{
	rm_info_t rm_info;
	
	while (1) {
		xQueueReceive(record_info->rm_queue, &rm_info, portMAX_DELAY );
		REC_DBG("remove %s\n", rm_info.rm_name_path);
		rec_filenode_del(record_info->type, rm_info.rn_name);
		rec_size_del(rm_info.rm_name_path, &record_info->recordusedsize);
		fs_cmd_rm(rm_info.rm_name_path);
		rec_thumbnail_del(record_info->type, rm_info.rm_name_path);
	}
}

/**
 * @brief set status to recording
 * @param info pointer for sdrecord info structure
 * @param flag status set
 */
//static void add_rec_status(recordinfo_t *info, int flag)
void add_rec_status(recordinfo_t *info, int flag)
{
	info->recordstatus |= flag;
}

/**
 * @brief check status
 * @param info pointer for sdrecord info structure
 * @param flag indicate the specific type
 * @return 1 if status on , else 0 if status off
 */
int chk_rec_status(const recordinfo_t *info, int flag)
{
	return (info->recordstatus & flag);
}

/**
 * @brief checking record start now ,and when to change record file
 *@ param info pointer for sdrecord info structure
 *@
 */
static void chk_rec_start_changefile(recordinfo_t *sdrecord_info)
{
	if (chk_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE)) {
		int currentfilesize = 0;
		RecFileInfo_t pCurFileInfo;
		RecFileInfo_t pLastFileInfo;
		
		record_get_file_info(sdrecord_info->pRecord_info, &pCurFileInfo, &pLastFileInfo);
		if (sdrecord_info->sched_p->sd_sched_record_interval > SD_MAX_RECORD_LENGTH) {
			currentfilesize = rec_filesize_get(record_get_cur_file_name(sdrecord_info->pRecord_info));
		}
				
		if ((pCurFileInfo.VdoFrameNum >= ((sdrecord_info->sched_p->sd_sched_record_interval) * (sdrecord_info->RecParam.vdo.ucFps))) || 
			(currentfilesize > MAXFILESIZE)) {
			record_set_next(sdrecord_info->pRecord_info, 1);
			rec_filenode_add(sdrecord_info->type, record_get_last_file_name(sdrecord_info->pRecord_info), &sdrecord_info->recordusedsize);
			rec_snapshot_query(SCHED_RECORD, record_get_cur_file_name(sdrecord_info->pRecord_info));
		}
	} else {
		if (exists(sdrecord_info->schedpath) == 0) {
			mkdir(sdrecord_info->schedpath);
		}
		record_set_path(sdrecord_info->pRecord_info, sdrecord_info->schedpath);
		record_set_start(sdrecord_info->pRecord_info, 1);
		rec_snapshot_query(SCHED_RECORD, record_get_cur_file_name(sdrecord_info->pRecord_info));
		add_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE);
#if MSG_TONE_ENABLE
		aac_tone_play(NVRAM_START_REC);
#endif
		
		REC_DBG("Schedule record start\n");
		
#ifdef CONFIG_MP4_RECOVER
		/* wait record module update current recording file */ 
		gMp4RecoverRun = 1;
#endif	

	}
	add_rec_status(sdrecord_info, RECORD_START_RUNNING);
}

/**
 * @brief clear status from recording
 * @param info pointer for sdrecord info structure
 * @param flag status delete
 */
static void del_rec_status(recordinfo_t *info, int flag)
{
	info->recordstatus &= ~flag;
}

/**
 * @brief checking now record is running
 *@ , if runnig , stop record and  add file to list  and  add size
 *@
 */
static void rec_stop_changestate(recordinfo_t *sdrecord_info)
{
	if (chk_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE)) {
		REC_DBG("RECORD_SCHED_ACTIVE\n");
		record_set_stop(sdrecord_info->pRecord_info, 1);
		rec_filenode_add(sdrecord_info->type, record_get_last_file_name(sdrecord_info->pRecord_info), &sdrecord_info->recordusedsize);
		del_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE);
	}
	
	del_rec_status(sdrecord_info, RECORD_START_RUNNING);
}

/**
 * @brief checking app json cmd setting value for record
 * @param sdrecord_info pointer for sdrecord info structure
 */
static void chk_userparam(recordinfo_t *sdrecord_info)
{
	int cycle;
	int level;

//	if (chk_record_task_close() == 1) {
//		sdrecord_info->recordclose = 1;
//		sdrecord_info->sched_p->sd_sched_internal_en = 0; //disable record
//	}
//	if (chk_record_cycle_bit()) {
//		if (snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, &cycle) != NVRAM_SUCCESS) {
//			REC_PRINT(SYS_ERR, "get record schedule cycle from nvram failed!\n");
//			return;
//		}
//		REC_DBG("cycle=%d\n", cycle);
//		sdrecord_info->sched_p->sd_sched_record_cycle = cycle;
//		set_record_cycle_bit(0);
//	}
//	if (chk_record_audio_voice_bit()) {
//		int levelchangetoset;
//		if (snx_nvram_integer_get(NVRAM_PKG_AUDIO_ISP, NVRAM_CFG_AUDIO_VOICE, &level) != NVRAM_SUCCESS) {
//			REC_PRINT(SYS_ERR, "get record audio voice from nvram failed!\n");
//			return;
//		}
//		if ((level < 0) || (level > 100))
//			levelchangetoset = 20; //default
//		if (level == 0)
//			levelchangetoset = 0;
//		else if ((0 < level) && (level <= 3))
//			levelchangetoset = 1;
//		else if (level > 3) {
//			levelchangetoset = (int)level / 3;
//			if (levelchangetoset > 31)
//				levelchangetoset = 31;
//		}
//		REC_DBG("level change to set=%d\n", levelchangetoset);
//		mf_audio_set_record_voice(levelchangetoset);
//		set_record_audio_voice_bit(0);
//	}
}

/**
 * @brief schedual record flow
 * @param sdrecord_info pointer for recordinfo_t info structure
 */
static void sched_rec_flow(recordinfo_t *sdrecord_info)
{
	system_date_t date;
	int ret;
	
	for (;;) {
		chk_rec_start_changefile(sdrecord_info);
//		if ((sdrecord_info->sched_p->sd_sched_internal_en == 1) && (userdisablerecord == 0)) {
//			get_date(&date);
//			date.week = date.week - 1;
//			if ((sdrecord_info->sched_p->day[date.week].duration != 0) && (sdrecord_info->sdcardisfull == 0)) {
//				int st = sdrecord_info->sched_p->day[date.week].hour * 3600 + sdrecord_info->sched_p->day[date.week].minute * 60 + sdrecord_info->sched_p->day[date.week].second;
//				int et = st + sdrecord_info->sched_p->day[date.week].duration;
//				int now = date.hour * 3600 + date.minute * 60 + date.second;
//				if ((st <= now) && (et > now)) {
//					chk_rec_start_changefile(sdrecord_info);
//				} else {
//					rec_stop_changestate(sdrecord_info);
//				}
//			} else {
//				rec_stop_changestate(sdrecord_info);
//			}
//		} else {
//			rec_stop_changestate(sdrecord_info);
//		}
		
//		if ((sdrecord_info->sched_p->sd_sched_internal_en == 1) && (userdisablerecord == 0) && (sdrecord_info->readfilelistok == 1)) {
//			long long  size = 0;
//			long long  schedfolercanusesize = 0;
//			
//			schedfolercanusesize = get_schedfolder_canusesize();
//			size = (schedfolercanusesize - sdrecord_info->recordusedsize);
//			if (size < SD_RECORD_RESERVED) {
//				if (sdrecord_info->sched_p->sd_sched_record_cycle == 1) {
//					rec_full_deloldestfile(sdrecord_info);
//				} else {
//				#if MSG_TONE_ENABLE
//					aac_tone_play(NVRAM_END_REC);
//				#endif
//					rec_stop_changestate(sdrecord_info);
//					sdrecord_info->sdcardisfull = 1;
//					remind_sdcardisfull();
//				}
//			} else {
//				sdrecord_info->sdcardisfull = 0;
//				disable_remind_sdcardisfull();
//			}
//		}
		//chk_userparam(sdrecord_info);
#if defined( CONFIG_AUTOMOUNT )
		if (get_sd_umount_err() == 1) { //error or eject
#if MSG_TONE_ENABLE
			aac_tone_play(NVRAM_SD_EJECT);	
#endif
			REC_DBG("sdcard eject or sdcard error\n");
			rec_stop_changestate(sdrecord_info);
			snx_fm_release_filelist(sdrecord_info->type, NULL);
			break;
		}
#endif		
		
		if ((sdrecord_info->recordclose == 1) && (sdrecord_info->readfilelistok == 1)) {
			rec_stop_changestate(sdrecord_info);
			break;
		}

		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

/**
 *@brief init_record_test_task
 * @return 0 :success , -1:Taskcreate Fail
 */
int init_record_test_task(void)
{
//	int intbuf;
//	
//	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_RECORD_UPBD, &intbuf);
//	if (intbuf <= 0)
//		return 0;
	
	if (pdPASS != xTaskCreate(task_record_test, "TASK_REC_TEST", 1536, NULL, 35, NULL))
	{
		REC_DBG("could not create record test task !!\n");
		return (-1);
	}
	
	return 0;
}

int init_record_uvc_task(void)
{
	if (pdPASS != xTaskCreate(task_record_uvc, "TASK_REC_UVC", 1536, NULL, 35, NULL))
	{
		REC_DBG("could not create record uvc task !!\n");
		return (-1);
	}
	
	return 0;
}

void init_record_flow()
{
	int ret;
	//unsigned int sensor_cap;
	
	record_info = rec_info_init();
	if( record_info == NULL )
//		goto finally;
		uninit_record_flow();
	
	rec_video_init();
		
	ret = rec_record_init(record_info);
	if (ret == pdFAIL) {
		REC_DBG("record_init fail\n");
		uninit_record_flow();
//		goto finally;
	}

	rec_filenode_update(record_info->type);
	
//finally:
	

}

void uninit_record_flow()
{
#ifdef CONFIG_MP4_RECOVER
		gCloseRecover = 1;
#endif
	
	if (record_info->pRecord_info != NULL)
	{
		record_uninit(record_info->pRecord_info);
		record_info->pRecord_info = NULL;
	}
	
	snx_fm_release_filelist(record_info->type, NULL);

	if (record_info != NULL)
	{
		if (record_info->rm_file_task != NULL) {
			vTaskDelete(record_info->rm_file_task);
			record_info->rm_file_task = NULL;
		}
		if (record_info->rm_queue != NULL) {
			vQueueDelete(record_info->rm_queue);
			record_info->rm_queue = NULL;
		}
		rec_info_uninit(&record_info);
	}	
}

/**
 *@brief RecordTaskFlow
 *@note init sdinfo,init record, init nvram, and check sdcard insert
 */
static void task_record_test(void *pvParameters)
{
	init_record_flow();
	
	if( readIFramData() != FR_OK )
	{
		REC_DBG("read I Frame Fail !!\n");
		uninit_record_flow();
		goto end;	//finally;
	}
		
//	mf_video_set_record_cb(recordvideo);
	if (pdPASS != xTaskCreate(task_record_video, "TaskRecordVideo", 512, NULL, 40, NULL))
	{
		REC_DBG("Could not create task record\n");
		uninit_record_flow();
		goto end;	//finally;
	}	
	
#if CONFIG_APP_DRONE
	if (record_info->sched_p->sd_sched_record_en == 1)
		user_enable_rec();
	else
		user_diable_rec();
#endif

#if defined(CONFIG_AUTOMOUNT)	
	sdaumt_record_info = get_automount_info();
#endif	
	
//	for (;;) 
	{
		if (record_info->recordclose == 1) {
			uninit_record_flow();
			goto end;	//finally;
		}
				
		record_info->readfilelistok = 0;
		record_info->sdcardseed = 0;
		
#if defined(CONFIG_AUTOMOUNT)		
		if (xSemaphoreTake(sdaumt_record_info->SdRecordMutex, portMAX_DELAY) == pdTRUE)
#endif			
		{
			add_rec_status(record_info, RECORD_START_RUNNING);
#ifndef CONFIG_APP_DRONE
			int updateseed;
#endif
			rec_filenode_update(record_info->type);
//			if (chk_record_task_close() == 1) {
//				del_rec_status(record_info, RECORD_START_RUNNING);
//				goto finally;
//			}
			
			record_writebufreset(record_info->pRecord_info);
			record_info->sdcardisfull = 0;
//			chk_cardsize();
//			record_info->recordusedsize = get_schedrec_usedsize();
			//parsing card and import filenode
//			int w = 10; //anydata;
//			vicky
//			xQueueSendToBack(card_info.chk_card_queue, (void *)&w, 0);

//#if defined(CONFIG_MODULE_RTC_SUPPORT) && !defined(CONFIG_APP_DRONE)
//			updateseed = check_update_seed(&dev_time);
//			if (updateseed == 1)
//				update_seed();
//#else
//			start_new_seed();
//#endif

			record_set_start(record_info->pRecord_info, 1);
			vTaskDelay(10000 / portTICK_RATE_MS );
				
			record_set_stop(record_info->pRecord_info, 1);
			REC_DBG("completed !!!!!!!!!!!!!!!\n");
	
//			if (get_readcard_finish() == 1) {
//				long long space;
//				space = get_schedfolder_canusesize();
//				if ((space - record_info->recordusedsize < SD_RECORD_RESERVED) && 
//					(record_info->sched_p->sd_sched_record_cycle == 0)) {
//					REC_DBG("SD Card Record is Full & No Cycle\n");
//					record_info->sdcardisfull = 1;
//				}
//				sched_rec_flow(record_info);
//				
//#ifdef CONFIG_MP4_RECOVER
//				/* clear MP4 update flag */
//				gMp4RecoverRun = 0;
//#endif
//			}
		}
	}	//end of for (;;) 

//finally:
//	
//#ifdef CONFIG_MP4_RECOVER
//		gCloseRecover = 1;
//#endif
//	
//	if (record_info->pRecord_info != NULL) {
//		record_uninit(record_info->pRecord_info);
//		record_info->pRecord_info = NULL;
//	}
////	cardsize_uinit();
//	snx_fm_release_filelist(record_info->type, NULL);
////	if (record_info->sched_p != NULL)
////		rec_nvram_uninit(&record_info->sched_p);
//	if (record_info != NULL) {
//		if (record_info->rm_file_task != NULL) {
//			vTaskDelete(record_info->rm_file_task);
//			record_info->rm_file_task = NULL;
//		}
//		if (record_info->rm_queue != NULL) {
//			vQueueDelete(record_info->rm_queue);
//			record_info->rm_queue = NULL;
//		}
//		rec_info_uninit(&record_info);
//	}

end:
	vTaskDelete(NULL);
	
	for(;;)
		vTaskDelay(100/portTICK_PERIOD_MS);
}

static void task_record_uvc(void *pvParameters)
{
	init_record_flow();
		
//	if (pdPASS != xTaskCreate(uvc_record_video_task, "TASK_UVC_REC_VDO", 512, NULL, 40, NULL))
//	{
//		UVC_DBG("Could not create TASK_UVC_REC_VDO !! \n");
//		goto finally;
//	}	
	
	if (record_info->recordclose == 1) {
			goto finally;
		}
				
		record_info->readfilelistok = 0;
		record_info->sdcardseed = 0;
//		if (xSemaphoreTake(uvc_automount_info->SdRecordMutex, portMAX_DELAY) == pdTRUE)
		{
			add_rec_status(record_info, RECORD_START_RUNNING);
			rec_filenode_update(record_info->type);
			
			record_writebufreset(record_info->pRecord_info);
			record_info->sdcardisfull = 0;

			record_set_start(record_info->pRecord_info, 1);
		}
		
//	uvc_automount_info = get_automount_info();
	
//	for (;;) {
//		if (record_info->recordclose == 1) {
//			goto finally;
//		}
//				
//		record_info->readfilelistok = 0;
//		record_info->sdcardseed = 0;
////		if (xSemaphoreTake(uvc_automount_info->SdRecordMutex, portMAX_DELAY) == pdTRUE)
//		{
//			add_rec_status(record_info, RECORD_START_RUNNING);
//			rec_filenode_update(record_info->type);
//			
//			record_writebufreset(record_info->pRecord_info);
//			record_info->sdcardisfull = 0;

//			record_set_start(record_info->pRecord_info, 1);
//			REC_DBG("record start !!\n");
//		}
//	}

	for(;;)
		vTaskDelay(100/portTICK_RATE_MS );

finally:
		
	if (record_info->pRecord_info != NULL) {
		record_uninit(record_info->pRecord_info);
		record_info->pRecord_info = NULL;
	}
	snx_fm_release_filelist(record_info->type, NULL);
	if (record_info != NULL) {
		if (record_info->rm_file_task != NULL) {
			vTaskDelete(record_info->rm_file_task);
			record_info->rm_file_task = NULL;
		}
		if (record_info->rm_queue != NULL) {
			vQueueDelete(record_info->rm_queue);
			record_info->rm_queue = NULL;
		}
		rec_info_uninit(&record_info);
	}

end:
//	record_is_running(0);
	vTaskDelete(NULL);
}

void task_record_video( void *pvParameters )
{
	unsigned char *pFrame;
	unsigned int uiFrameSize;
	unsigned char IFrame = 1;
	int frame_num = 0;
	
	while(1)
	{
		if(video_info.ucStreamMode|REC_FMT_H264)
		{
			pFrame = IFrameDataBuf;
			uiFrameSize = IFRAME_FILE_SIZE;
			recordvideo(IFrame, pFrame, uiFrameSize);
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		else
			vTaskDelay(10 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}



/**
 * @brief send video data to middleware recording
 * @param iframe is I frame or not
 * @param pframe pointer for frame data
 * @param uiframeSize frame size
 * @param tval the time of frame data
 */
//void recordvideo(unsigned char iframe, unsigned char *pframe, unsigned int uiframeSize, struct timeval tval)
void recordvideo(unsigned char iframe, unsigned char *pframe, unsigned int uiframeSize)
{
	if (((record_info != NULL) && (record_info->pRecord_info != NULL)) && (record_info->recordclose != 1))
	{
//		if (iframe == 1) {
//#ifndef CONFIG_APP_DRONE
//			chk_gpsinfo(record_info);
//			chk_gsensorinfo(record_info);
//#endif
//		}
		//record_video(record_info->pRecord_info, iframe, pframe, uiframeSize, tval);
		record_video(record_info->pRecord_info, iframe, pframe, uiframeSize);
	}
}


void modify_dev_time(void)
{
//	vicky
//	get_date(&dev_time);
	REC_DBG("modify dev_time dev_year=%d,dev_month=%d,dev_day=%d,dev_hour=%d,dev_minute=%d,dev_second=%d\n", (dev_time).year, (dev_time).month, (dev_time).day, (dev_time).hour, (dev_time).minute, (dev_time).second);
}

/**
 * @brief enable recording
 */
void enable_rec(void)
{
	REC_DBG("start record access sdcard\n");
	if ((record_info != NULL) && (record_info->sched_p != NULL))
		record_info->sched_p->sd_sched_internal_en = 1;
}

/**
 * @brief disalbe recording but stream continue
 */
void diable_rec(void)
{
	REC_DBG("stop record access sdcard\n");
	if ((record_info != NULL) && (record_info->sched_p != NULL))
		record_info->sched_p->sd_sched_internal_en = 0;
}

/**
 * @brief check status
 * @return -1 if sdrecord_info is not created. 1 if recording is working ; 0 if recording is finished
 */
int get_schedrec_status(void)
{
	if(record_info == NULL)
		return -1;
	else
		return record_info->recordstatus;
}







#if 0
/**
* @brief  init schedule record time structure
* @return return struct SchedTime if success
*/

struct SchedTime *rec_nvraminit(void)
{
	struct SchedTime *sched_p;
	
	if (!(sched_p = (SchedTime_t *)pvPortMalloc(sizeof(SchedTime_t), GFP_KERNEL, MODULE_APP))) {
		REC_PRINT(SYS_ERR, "recrodinfo_t init fail\n");
		goto finally;
	}
	
	memset(sched_p, 0, sizeof(SchedTime_t));
	return sched_p;
	
finally:
	return NULL;
}


/**
* @brief uninit schedule time structure
* @param sched_p double pointer for schaduled time
* @return
*/

void rec_nvram_uninit(struct SchedTime **sched_p)
{
	safeFree(*sched_p);
}


/**
* @brief  get record param from nvram data
* @param  sched_p: get vlaue from nvram and set the info to sched_p structure
* @return return 0 if success
*/
int get_record_param(struct SchedTime *sched_p)
{
	int intbuf;
	unsigned int unsignintbuf;
	
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_BEG_HOUR, &intbuf);
	sched_p->day[0].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_BEG_MINUTE, &intbuf);
	sched_p->day[0].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_BEG_SECOND, &intbuf);
	sched_p->day[0].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_DURATION, &unsignintbuf);
	sched_p->day[0].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_BEG_HOUR, &intbuf);
	sched_p->day[1].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_BEG_MINUTE, &intbuf);
	sched_p->day[1].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_BEG_SECOND, &intbuf);
	sched_p->day[1].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_DURATION, &unsignintbuf);
	sched_p->day[1].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_BEG_HOUR, &intbuf);
	sched_p->day[2].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_BEG_MINUTE, &intbuf);
	sched_p->day[2].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_BEG_SECOND, &intbuf);
	sched_p->day[2].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_DURATION, &unsignintbuf);
	sched_p->day[2].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_BEG_HOUR, &intbuf);
	sched_p->day[3].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_BEG_MINUTE, &intbuf);
	sched_p->day[3].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_BEG_SECOND, &intbuf);
	sched_p->day[3].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_DURATION, &unsignintbuf);
	sched_p->day[3].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_BEG_HOUR, &intbuf);
	sched_p->day[4].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_BEG_MINUTE, &intbuf);
	sched_p->day[4].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_BEG_SECOND, &intbuf);
	sched_p->day[4].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_DURATION, &unsignintbuf);
	sched_p->day[4].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_BEG_HOUR, &intbuf);
	sched_p->day[5].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_BEG_MINUTE, &intbuf);
	sched_p->day[5].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_BEG_SECOND, &intbuf);
	sched_p->day[5].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_DURATION, &unsignintbuf);
	sched_p->day[5].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_BEG_HOUR, &intbuf);
	sched_p->day[6].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_BEG_MINUTE, &intbuf);
	sched_p->day[6].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_BEG_SECOND, &intbuf);
	sched_p->day[6].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_DURATION, &unsignintbuf);
	sched_p->day[6].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_INTERVAL, &intbuf); //20sec
	sched_p->sd_sched_record_interval = intbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_MD_INTERVAL, &intbuf); //5sec
	sched_p->sd_alarm_record_interval = intbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, &intbuf);    //0:not cycle 1:cycle
	sched_p->sd_sched_record_cycle = intbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_ENABLE, &intbuf); //enable
	sched_p->sd_sched_record_en = intbuf;
	sched_p->sd_sched_internal_en = 1;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_MD_ENABLE, &intbuf); //enable
	sched_p->sd_alarm_record_en = intbuf;

	return 0;
}


#ifndef CONFIG_APP_DRONE
/**
 * @brief read mcu GPS data and send to avi file
 * @param info pointer for sdrecord info structure
 */
static void chk_gpsinfo(recordinfo_t *info)
{
	unsigned short sn;
	
	sn = get_gps_sn();
	if ((sn >= 1) && (info->gpscount != sn)) {
		info->gpscount = sn;
		AVIGPSINFO_t *gpsinfo;
		gpsinfo = get_avi_gps_info();
		info->pRecord_info->GPS_info.ulChunkID = 1;
		info->pRecord_info->GPS_info.ucGPSStatus = 1;
		info->pRecord_info->GPS_info.ssLonDegInt = gpsinfo->ssLonDegInt;
		info->pRecord_info->GPS_info.ulLonDegDec = gpsinfo->ulLonDegDec;
		info->pRecord_info->GPS_info.ssLatDegInt = gpsinfo->ssLatDegInt;
		info->pRecord_info->GPS_info.ulLatDegDec = gpsinfo->ulLatDegDec;
		info->pRecord_info->GPS_info.usAltitude = gpsinfo->usAltitude;
		info->pRecord_info->GPS_info.usSpeed = gpsinfo->usSpeed;
		info->pRecord_info->GPS_info.ucGpsHour = gpsinfo->ucGpsHour;
		info->pRecord_info->GPS_info.ucGpsMinute = gpsinfo->ucGpsMinute;
		info->pRecord_info->GPS_info.ucGpsSecond = gpsinfo->ucGpsSecond;
		info->pRecord_info->GPS_info.usGpsYear = gpsinfo->usGpsYear;
		info->pRecord_info->GPS_info.ucGpsMonth = gpsinfo->ucGpsMonth;
		info->pRecord_info->GPS_info.ucGpsDay = gpsinfo->ucGpsDay;
	} else {
		info->pRecord_info->GPS_info.ulChunkID = 0;
		info->pRecord_info->GPS_info.ucGPSStatus = 0;
	}
}

/**
 * @brief read mcu GSensor data and send to avi file
 * @param info pointer for sdrecord info structure
 */
static void chk_gsensorinfo(recordinfo_t *info)
{
	unsigned short sn;
	
	sn = get_g_sensor_sn();
	if ((sn >= 1) && (info->gsensorcount != sn)) {
		info->gsensorcount = sn;
		AVIGSENSORINFO_t *gsensorinfo;
		gsensorinfo = get_avi_g_sensor_info();
		info->pRecord_info->GSENSOR_info.ulChunkID = 1;
		info->pRecord_info->GSENSOR_info.ucAcceRange = gsensorinfo->ucAcceRange;
		info->pRecord_info->GSENSOR_info.ucGVInt_X = gsensorinfo->ucGVInt_X;
		info->pRecord_info->GSENSOR_info.ulGVDec_X = gsensorinfo->ulGVDec_X;
		info->pRecord_info->GSENSOR_info.ucGVInt_Y = gsensorinfo->ucGVInt_Y;
		info->pRecord_info->GSENSOR_info.ulGVDec_Y = gsensorinfo->ulGVDec_Y;
		info->pRecord_info->GSENSOR_info.ucGVInt_Z = gsensorinfo->ucGVInt_Z;
		info->pRecord_info->GSENSOR_info.ulGVDec_Z = gsensorinfo->ulGVDec_Z;
	} else {
		info->pRecord_info->GSENSOR_info.ulChunkID = 0;
	}
}
#endif

/**
 * @brief send audio data to middleware recording
 * @param pframe pointer for each frame data
 * @param uiframeSize frame size
 * @param tval the time of frame data
 */
void recordaudio(unsigned char *pframe, unsigned int uiframeSize, struct timeval tval)
{
	if (((record_info != NULL) && (record_info->pRecord_info != NULL)) && (record_info->recordclose != 1))
		record_audio(record_info->pRecord_info, pframe, uiframeSize, tval);
}

/**
 * @brief get recordinfo pointer for protect function use
 * @return if have,return recordinfo_t pointer
 */
recordinfo_t *get_rec_pointer(void)
{
	if ((record_info != NULL) && (record_info->pRecord_info != NULL))
		return record_info;
	else
		return NULL;
}

/**
 * @brief read record param from nvram
 * @param sdrecord_info pointer for recordinfo_t structure
 * @return 0 if read nvram OK
 */
static int rec_nvram(recordinfo_t *sdrecord_info)
{
	sdrecord_info->sched_p = rec_nvraminit();
	if (sdrecord_info->sched_p == NULL) {
		REC_PRINT(SYS_ERR, "sd nvram init fail\n");
		goto finally;
	}
	
	get_record_param(sdrecord_info->sched_p);
	sdrecord_info->fileformat = rec_fileformat_get();
	
	return 0;
finally:
	if (sdrecord_info->sched_p != NULL)
		rec_nvram_uninit(&sdrecord_info->sched_p);
	return (-1);
}

/**
 * @brief user app diable recording
 */
void user_diable_rec(void)
{
	REC_PRINT(SYS_DBG, "user diable record\n");
#if MSG_TONE_ENABLE
	aac_tone_play(NVRAM_END_REC);
#endif
	userdisablerecord = 1;
	mf_set_record(0);	// H.264 encode disable
}

/**
 * @brief user app enable recording
 */
void user_enable_rec(void)
{
	REC_PRINT(SYS_DBG, "user enable record\n");
	userdisablerecord = 0;
	mf_set_record(1);	// H.264 encode enable
}

/**
 * @brief check recording enable or disable  
 * @return return 0:disable 1:enable
 */
int chk_rec_enable(void)
{
	if (userdisablerecord == 1)
		return 0;
	else
		return 1;
}

/**
 * @brief suspend recording to start
 * @param force_record 1:force record for mcu  0:not force record
 */
void schedrec_suspend_restart(int force_record)
{
	if ((record_info != NULL) && (record_info->sched_p != NULL)){
		/*restore wb mem creation*/
		schedrec_set_wb_mem(1);
		record_info->sched_p->sd_sched_internal_en = 1;
	}

	if (force_record == 0) {
		if (userdisablerecord == 0) {
			mf_set_record(1);	// H.264 encode enable
		}
	} else if (force_record == 1) {
		mf_set_record(1);	// H.264 encode enable
		userdisablerecord = 0;
	}
}

/**
 * @brief suspend recording and H.264 encode disable
 */
void schedrec_suspend(void)
{
	REC_PRINT(SYS_DBG, "sched record suspend\n");
	if ((record_info != NULL) && (record_info->sched_p != NULL))
		record_info->sched_p->sd_sched_internal_en = 0;
	mf_set_record(0);	// H.264 encode disable
}

/**
 * @brief set flag to create or release wb mem
 */

void schedrec_set_wb_mem(int flag)
{
	if(record_info != NULL) {
		if(flag == 1)
			record_writebuf_create(record_info->pRecord_info);
		else
			record_writebuf_release(record_info->pRecord_info);
			
	}
}

/**
 * @brief checking now record is running
 * @return return  1:recording is running
 * @               0:recording is not running
 * @              -1:reocrding task not init
 */
int schedrec_state(void)
{
	if ((record_info == NULL) || (record_info->sched_p == NULL))
		return (-1);
#if defined( CONFIG_AUTOMOUNT )	
	else if ((record_info->sched_p->sd_sched_internal_en == 0) || (userdisablerecord == 1) || (get_sd_umount_err() == 1) || (record_info->sdcardisfull == 1))
#else		
	else if ((record_info->sched_p->sd_sched_internal_en == 0) || (userdisablerecord == 1) || (record_info->sdcardisfull == 1))
#endif		
		return 0;
	else if (record_info->sched_p->sd_sched_internal_en == 1)
		return 1;
	else
		return (-1);
}

/**
 * @brief close schedual record task
 */
void schedrec_close(void)
{
	schedrec_suspend();
	if (record_info != NULL)
		record_info->recordclose = 1;
}

/**
 * @brief report space for schedule record is full or not
 * @return 1:is full
 */
int schedrec_get_isfull(void)
{
	return record_info->sdcardisfull;
}


/**
 * @brief cheak if recording task init ok and sdcard ready.
 * @return return -1:reocrding not init
 *                 0:sdcard inset and read "/sonix/record/..." is running
 *@                1:sdcard insert ,and reading "/sonix/record/..."  total file is finished
 *@
 */
int read_card_state(void)
{
	if ((record_info == NULL) || (record_info->sched_p == NULL))
		return (-1);
	else
		return record_info->readfilelistok;
}


/**
 *@brief interface function -del record file  for app user use delete
 @ return  0: can delete file  -1: not can delete file

*/
int del_rec_recordfile(const char *delname)
{
	char target[LEN_FILENAME];
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
		return (-1);
	}

	memset(target, 0x00, sizeof(target));
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, delname);
	if ((sdrecord_info != NULL)) {
		ret = schedrec_state();
		if (ret == 0) {
			rec_filenode_del(sdrecord_info->type, delname);
			rec_size_del(target, &sdrecord_info->recordusedsize);
			rec_thumbnail_del(sdrecord_info->type, target);
			return 0;
		} else if (ret == 1) {
			if (strcmp(target, record_get_cur_file_name(sdrecord_info->pRecord_info)) != 0) {
				rec_filenode_del(sdrecord_info->type, delname);
				rec_size_del(target, &sdrecord_info->recordusedsize);
				rec_thumbnail_del(sdrecord_info->type, target);
				return 0;
			} else
				return (-1);
		} else
			return (-1);
	}
	return (-1);
}

/**
 * @brief checking record full , delete file process
 *@ param info pointer for sdrecord info structure
 *@
 */

static void rec_full_deloldestfile(recordinfo_t *sdrecord_info)
{
	char removefile[LEN_FILENAME] = {0};
	char target[LEN_FILENAME] = {0};
	int ret;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
	}
	
	memset(target, 0x00, sizeof(target));
	memset(removefile, 0x00, sizeof(removefile));

	/* get the oldest file name from file manager */
	ret = snx_fm_get_first_file(sdrecord_info->type, removefile, sizeof(removefile), NULL);
	/***
	if (ret == -1) {
		if (chk_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE)) {
			record_set_stop(sdrecord_info->pRecord_info, 1);
			rec_filenode_add(sdrecord_info->type, record_get_last_file_name(sdrecord_info->pRecord_info), &sdrecord_info->recordusedsize);
			REC_PRINT(SYS_WARN, "sdcard is full but no oldest file\n");
			del_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE);
			sdrecord_info->sdcardisfull = 1;
		}
	}***/ 
	if (ret == 0) {
		snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, removefile);
		REC_PRINT(SYS_DBG, "the oldest file is %s\n", target);
		if((0 == is_current_upload_file(target, UPLOAD_BG)) || (0 == is_current_upload_file(target, UPLOAD_FG))) {
			rm_info_t rm_info;
			memset(&rm_info, 0, sizeof(rm_info_t));
			strcpy(rm_info.rm_name_path, target);
			strcpy(rm_info.rn_name, removefile);
			xQueueSendToBack(sdrecord_info->rm_queue, &rm_info , 0);
		}
	} else if (ret == FM_LISTWAIT) {
		//do nothing..
	} else {
		REC_PRINT(SYS_ERR, "ret is not correct value=%d\n", ret);
	}
}


/**
 * @brief set chk_card task state for record
 * @param state set state
 */
void set_card_task_state(int state){
	card_info.task_state = state;
}

/**
 * @brief get chk_card task state for record 
 * @return return state
 */
int chk_card_task_state(void){
	return card_info.task_state;
}


/**
 * @brief import all record file to recording file list for record
 * @note check sdcard folder data for add to list
 */
void import_record_task(void *pvParameters)
{
	int result = 0;
	char c;
	
	while(card_info.task_state) {
		xQueueReceive(card_info.chk_card_queue, (void *)&c, portMAX_DELAY);
		if(card_info.task_state == IMPORT_CARD_INACTIVE )
			break;
		
		if(sdrecord_info) {
			result = rec_import_files(sdrecord_info->type);
			sdrecord_info->readfilelistok = 1;
		}
	}
	
	card_info.task_state = IMPORT_CARD_FINISHED;
	vTaskDelete(NULL);
}


/**
 *@brief init the task for check card info for record
 */
void chk_card_task_init(void)
{
	memset(&card_info, 0, sizeof(chk_card_file_info_t));
	card_info.chk_card_queue = xQueueCreate(1, sizeof(char));
	if(NULL == card_info.chk_card_queue) {
		REC_PRINT(SYS_ERR, "Create SDcard chk Queue Failed\n");	
		goto fail1;
	}
	card_info.task_state = IMPORT_CARD_ACTIVE;
	if (pdPASS != xTaskCreate(import_record_task, "import_files_task", STACK_SIZE_6K, NULL, PRIORITY_TASK_APP_REC_FLOW, &card_info.chk_card_task)) {
		
		REC_PRINT(SYS_ERR, "SDCardCheckList task create fail\n");
		card_info.task_state = IMPORT_CARD_INACTIVE;
		goto fail2;
	}
	
	REC_PRINT(SYS_DBG, "Init chk_card_task Success\n");
	return;

fail2:
	if(card_info.chk_card_queue) {
		vQueueDelete(card_info.chk_card_queue);
		card_info.chk_card_queue = NULL;
	}	
fail1:
	return;
}


/**
 *@brief uninit the task for check card info for record
 */
void chk_card_task_uninit(void)
{
	set_card_task_state(IMPORT_CARD_INACTIVE);
	{
		int w = 10; //anydata;
		xQueueSendToBack(card_info.chk_card_queue, (void *)&w, 0);
	}
	
	while(1){
		if(chk_card_task_state() == IMPORT_CARD_FINISHED)
			break;
		vTaskDelay(20 / portTICK_RATE_MS);
	}
	
	if(card_info.chk_card_queue) {
		vQueueDelete(card_info.chk_card_queue);
		card_info.chk_card_queue = NULL;
	}
	REC_PRINT(SYS_DBG, "Uninit chk_card_task Success\n");
}


/**
* @brief create the file and write the name and file info of all schaduled record files
* @param path the absolutely path for saving the created file. 
* @return return 0 if success, otherwise if false
*/
int reclist_to_file(char *path)
{
	char target[LEN_FILENAME] = {0};
	int len = 0;
	int ret;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
		return (-1);
	}
	
	memset(target, 0x00, sizeof(target));
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, SD_RECORD_FILELIST);
	len = strlen(target);
	strncpy(path, target, len);
	if (sdrecord_info == NULL) {
		return (-1);
	} else if (is_current_upload_file(target, UPLOAD_FG) != 0) {
		REC_PRINT(SYS_ERR, "record filelist is downloading\n");
		return LIST_FILE_DOWNLOAD;
	} else {
		return rec_filelist_Createfile(T_RECORD, target, len);
	}
}

#if 0
/**
* @brief delete the uncompleted schaduled record file
* @param filename assign file 
*/
void rec_sched_incompletefile_hndl(char *filename)
{
	struct usr_config *pUserCfg;
	char target[128] = {0};
	int ret = 0;
	
	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed.\n");
		return;
	}
	/* remove incomplete file in MP4 format */
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, filename);
	REC_PRINT(SYS_DBG, "sdrecord_info->recordusedsize=%u KB\n",(int)(sdrecord_info->recordusedsize >> 10));
	rec_size_del(target, &sdrecord_info->recordusedsize);
	REC_PRINT(SYS_DBG, "after delete ,sdrecord_info->recordusedsize=%u KB\n",(int)(sdrecord_info->recordusedsize >> 10));
	fs_rm(target);
	rec_thumbnail_del(T_RECORD, target);
}
#endif

#ifdef CONFIG_MP4_RECOVER

#define INCOMPLETE_FILE_PATH 		"tmp" //> default path to stored incompleted mp4 files
/**
 * @brief recover the incompleted record mp4 file task
 * @note it will scan and recover those incompleted file in "tmp" folder.
 */
static void rec_recover_task(void *parameter)
{
	int ret, retopen, recover_result, recover_cnt = 0;
	RecoverInfo_t *pRecover = NULL;
	DIR dir;
	FILINFO fno;
	FIL  file;
	char target_dir[LEN_FILENAME] = {0};
	char target[LEN_FILENAME] = {0};
	char full_target_path[LEN_FULLPATH] = {0};
	struct usr_config *pUserCfg;
	int recover_flag = 0;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed.\n");
		goto end;
	}

	if (!(pRecover = recover_mp4_init())) {
		REC_PRINT(SYS_ERR, "Cannot alloc mp4 recover file descriptor\n");
		goto end;
	}

	snprintf(target_dir, LEN_FILENAME, "%s/%s", pUserCfg->root_path, INCOMPLETE_FILE_PATH);
	
wait_recover:
	for(;;) {
		if (gCloseRecover)	/* receive close */
			goto end;
		else if (gMp4RecoverRun) /*receive MP4 update */
			break;
		else 
			vTaskDelay(200 / portTICK_RATE_MS);
	}
	
	/* loop find the incomplete mp4 file and recover it */	
	retopen = f_opendir(&dir, target_dir);
	while ((ret = f_readdir(&dir, &fno)) == FR_OK) {
		if (ret != FR_OK || fno.fname[0] == 0) break;
#if defined( CONFIG_AUTOMOUNT )		
		if (get_sd_umount_err() == 1) break;
#endif
		
		if (gCloseRecover) {
			f_closedir(&dir);
			goto end;
		}
		
		if ((fno.fname[0] == '.') || (snx_fm_check_file_legal(GET_FN(fno)) != 1)) continue;
		if (ret == FR_OK) {
			if (fno.fattrib & AM_DIR) { //dir

			} else {
				/* get current record file and skip it. */
				char *pCur_file = record_get_cur_file_name(sdrecord_info->pRecord_info);
				int offset = rec_regularoffset_get(pCur_file);
				pCur_file += offset;
				
				if (!(strncasecmp(GET_FN(fno), pCur_file, strlen(GET_FN(fno))))) {
					REC_PRINT(SYS_DBG, "not recover file %s\n", GET_FN(fno));
					continue;
				}
			
				memset(target, 0, LEN_FILENAME);
				snprintf(target, LEN_FILENAME, "%s/%s/%s", pUserCfg->root_path, INCOMPLETE_FILE_PATH, GET_FN(fno)); 

				recover_result = recover_mp4_file(pRecover, target, NULL);
				if (recover_result != RECOVER_SUCCESS) {
					REC_PRINT(SYS_ERR, "recover mp4 file %s failed in %d\n", target, recover_result);
					/* recover failed then remove its file and thumbnail */
					fs_rm(target);
					rec_thumbnail_del(T_RECORD, target);
					continue;
				}
				else {
					char newname[LEN_FILENAME] = {0};
					char new_path[LEN_FULLPATH] = {0};
					/* rename it, move to /sonix/record/ and add it to filelist */
					rec_finishedfile_addlen(sdrecord_info->type, target, newname);

					/*Add it to filelist*/
					rec_filenode_add(sdrecord_info->type, newname, &sdrecord_info->recordusedsize);
				}
			}
		}
	}

	if (retopen == FR_OK)
		f_closedir(&dir);

	/* done, clear and wait next time update notify. */
	gMp4RecoverRun = 0;
	goto wait_recover;
	
end:
	recover_mp4_uninit(pRecover);
	pRecover = NULL;

	vTaskDelete(NULL);
}
#endif

#endif	//end of if 0

#endif	//end of #if defined( CONFIG_RECORD )
