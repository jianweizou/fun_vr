/**
 * @file
 * this is middleware avi file avi.c
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */

#include "sonix_config.h"

#if defined (CONFIG_RECORD)	

#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <task.h>
#include <string.h>
#include <stdio.h>
#include "avi_rec.h"
#include "rec_common.h"
#include "ff.h"

#define Padding_SKIP_FRAME	1
#define AVI_DBG(fmt, args...) printf("\n[AVI] %s : "fmt, __func__,##args)

unsigned char hd_skip_frame[HD_SKIP_FRAME_SIZE] = {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x9A, 0x04, 0x0B, 
	0xAF, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03,	0x00, 0x00, 
	0x03, 0x00, 0x01, 0x03
};

unsigned char fhd_skip_frame[FHD_SKIP_FRAME_SIZE] = {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x9A, 0x06, 0x0B, 
	0xAF, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,  
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,  
	0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x03, 0x00, 0x8D, 0x80
};


int 			avi_write_index(AVI_Info_t *pAviInfo ,uint8_t ucMode,uint8_t ucKeyFrame, int iFrameSize , char *flag);
int 			avi_write_gsensor(AVI_Info_t *pAviInfo, int count, int video_frame);
int 			avi_write_gps(AVI_Info_t *pAviInfo, int count, int video_frame);
int 			avi_read_header(AVI_read_Info_t *pAviReadInfo);
int 			avi_read_index(AVI_read_Info_t *pAviReadInfo);
void 			task_write_cmd_finish(void *pvParameters);
void 			write_back_to_file(AVI_Info_t *pAviInfo, void* addr, uint32_t data);
uint16_t 	Audio_format_tag_transfer(unsigned int ado_fmt);

/**
* @brief interface function - avi initialization
* @param pAviInfo avi structure
* @return return pdPASS if success
*/
int avi_init(AVI_Info_t *pAviInfo,AviWirteBufferInitInfo_t* WriteBufferInitParam,AviVdoInfo_t* AviVdoInfo,unsigned int cap)
{
	AVIFileHeader_t *pAvi;
	int i;
//	unsigned int uibps;
	memset(pAviInfo, 0, sizeof(AVI_Info_t));
#if defined( CONFIG_SN_GCC_SDK )	
	pAviInfo->pAviHeader = pvPortMalloc(sizeof(AVIFileHeader_t), GFP_KERNEL, MODULE_MID_RECORD);
#else
	pAviInfo->pAviHeader = pvPortMalloc(sizeof(AVIFileHeader_t));
#endif	
	if(pAviInfo->pAviHeader == NULL)
	{
		AVI_DBG("pAviHeader allocate fail\n");
		goto fail1;
	}
	memset(pAviInfo->pAviHeader, 0, sizeof(AVIFileHeader_t));
	pAvi = pAviInfo->pAviHeader;
	pAviInfo->has_audio = (cap & AUDIO_CAP)? 1:0;
	pAviInfo->has_gps = (cap & GPS_CAP)? 1:0;
	pAviInfo->has_gsensor = (cap & GSENSOR_CAP)? 1:0;
	AVI_DBG("audio=%d, gps=%d, gsensor=%d\n",pAviInfo->has_audio,pAviInfo->has_gps,pAviInfo->has_gsensor);
	pAviInfo->WBInitInfo.write_buf_size=WriteBufferInitParam->write_buf_size;
	pAviInfo->WBInitInfo.write_unit_to_file=WriteBufferInitParam->write_unit_to_file;

	if(writebuf_init((&pAviInfo->WBInfo),(&pAviInfo->WBInitInfo)) == pdFAIL)
	{
		AVI_DBG("WBInfo init fail\n");
		goto fail2;
	}
	
	WR_FOURCC(&pAvi->RIFF.ulList, "RIFF");
	//&pAvi->RIFF.ulSize
	WR_FOURCC(&pAvi->RIFF.ulFourCC, "AVI ");
	
	WR_FOURCC(&pAvi->head.hdrl.ulList, "LIST");
	WR_DWORD(&pAvi->head.hdrl.ulSize, sizeof(pAvi->head)-ATOM_HEAD_SIZE);
	WR_FOURCC(&pAvi->head.hdrl.ulFourCC, "hdrl");

	WR_FOURCC(&pAvi->head.avih.ulFourCC, "avih");
	WR_DWORD(&pAvi->head.avih.ulSize, sizeof(pAvi->head.avih)-ATOM_HEAD_SIZE);
	//WR_DWORD(&pAvi->head.avih.info.ulMicroSecPerFrame, 1000000/pVdoInfo->ucFps);
	//WR_DWORD(&pAvi->head.avih.info.ulMaxBytesPerSec, 16000);	//*****
	WR_DWORD(&pAvi->head.avih.info.ulMaxBytesPerSec, AviVdoInfo->uiBps);
	WR_DWORD(&pAvi->head.avih.info.ulPaddingGranularity, 0);
	WR_DWORD(&pAvi->head.avih.info.ulFlags, AVI_TRUSTCKTYPE | AVI_HASINDEX);	
	//WR_DWORD(&pAvi->head.avih.info.ulTotalFrames, );
	WR_DWORD(&pAvi->head.avih.info.ulInitialFrames, 0); 
	if(pAviInfo->has_audio){
		WR_DWORD(&pAvi->head.avih.info.ulStreams, 2);	//vdo+ado
	}
	else{
		WR_DWORD(&pAvi->head.avih.info.ulStreams, 1);	//vdo
	}
	WR_DWORD(&pAvi->head.avih.info.ulSuggestedBufferSize, 0x80000);//0x1000000);
	//WR_DWORD(&pAvi->head.avih.info.ulWidth, pVdoInfo->width);
	//WR_DWORD(&pAvi->head.avih.info.ulHeight, pVdoInfo->height);

	//video 
	WR_FOURCC(&pAvi->head.s1.strl.ulList, "LIST");
	WR_DWORD(&pAvi->head.s1.strl.ulSize, sizeof(pAvi->head.s1)-ATOM_HEAD_SIZE);
	WR_FOURCC(&pAvi->head.s1.strl.ulFourCC, "strl");
	WR_FOURCC(&pAvi->head.s1.strh.ulFourCC, "strh");
	WR_DWORD(&pAvi->head.s1.strh.ulSize, sizeof(pAvi->head.s1.strh)-ATOM_HEAD_SIZE);
	WR_FOURCC(&pAvi->head.s1.strh.info.fccType, "vids");
	//WR_FOURCC(&pAvi->head.s1.strh.info.fccHandler, pVdoInfo->ucStreamMode&2?"H264":"	");
	WR_DWORD(&pAvi->head.s1.strh.info.ulFlags, 0);
	WR_WORD(&pAvi->head.s1.strh.info.usPriority, 0);
	WR_WORD(&pAvi->head.s1.strh.info.usanguage, 0);
	WR_DWORD(&pAvi->head.s1.strh.info.ulInitialFrames, 0);
	WR_DWORD(&pAvi->head.s1.strh.info.ulScale, 1);
	//WR_DWORD(&pAvi->head.s1.strh.info.ulRate, pVdoInfo->ucFps);
	WR_DWORD(&pAvi->head.s1.strh.info.ulStart, 0);
	//WR_DWORD(pAvi->head.s1.strh.info.ulLength, );
	WR_DWORD(&pAvi->head.s1.strh.info.ulSuggestedBufferSize, 0x80000);//0x1000000);
	WR_DWORD(&pAvi->head.s1.strh.info.ulQuality, -1);
	WR_DWORD(&pAvi->head.s1.strh.info.ulSampleSize, 0);
	WR_WORD(&pAvi->head.s1.strh.info.rcFrame.left, 0);
	WR_WORD(&pAvi->head.s1.strh.info.rcFrame.top, 0);
	//WR_WORD(&pAvi->head.s1.strh.info.rcFrame.right, pVdoInfo->width);
	//WR_WORD(&pAvi->head.s1.strh.info.rcFrame.bottom, pVdoInfo->height);

	WR_FOURCC(&pAvi->head.s1.strf.ulFourCC, "strf");
	WR_DWORD(&pAvi->head.s1.strf.ulSize, sizeof(pAvi->head.s1.strf)-ATOM_HEAD_SIZE);
	WR_DWORD(&pAvi->head.s1.strf.info.ulSize, sizeof(pAvi->head.s1.strf.info));
	//WR_DWORD(&pAvi->head.s1.strf.info.ulWidth, pVdoInfo->width);
	//WR_DWORD(&pAvi->head.s1.strf.info.ulHeight, pVdoInfo->height);
	WR_WORD(&pAvi->head.s1.strf.info.usPlanes, 1 );
	WR_WORD(&pAvi->head.s1.strf.info.usBitCount, 24);
	//WR_FOURCC(&pAvi->head.s1.strf.info.ulCompression, pVdoInfo->ucStreamMode&2?"H264":"   ");
	//WR_DWORD(&pAvi->head.s1.strf.info.ulSizeImage, pVdoInfo->width*pVdoInfo->height*3);

	//audio ...
	//if(pAviInfo->has_audio){	
	WR_FOURCC(&pAvi->head.s2.strl.ulList, "LIST");
	WR_DWORD(&pAvi->head.s2.strl.ulSize, sizeof(pAvi->head.s2)-ATOM_HEAD_SIZE);
	WR_FOURCC(&pAvi->head.s2.strl.ulFourCC, "strl");


	WR_FOURCC(&pAvi->head.s2.strh.ulFourCC, "strh");
	WR_DWORD(&pAvi->head.s2.strh.ulSize, sizeof(pAvi->head.s2.strh)-ATOM_HEAD_SIZE);

	if(pAviInfo->has_audio){
		WR_FOURCC(&pAvi->head.s2.strh.info.fccType, "auds");
	}else{
		WR_FOURCC(&pAvi->head.s2.strh.info.fccType, "dats");
	}
	WR_DWORD(&pAvi->head.s2.strh.info.fccHandler, 1);
	WR_DWORD(&pAvi->head.s2.strh.info.ulFlags, 0);
	WR_WORD(&pAvi->head.s2.strh.info.usPriority, 0);
	WR_WORD(&pAvi->head.s2.strh.info.usanguage, 0);
	WR_DWORD(&pAvi->head.s2.strh.info.ulInitialFrames, 0);
		//WR_DWORD(&pAvi->head.s2.strh.info.ulScale, 1);
		//WR_DWORD(&pAvi->head.s2.strh.info.ulRate, pAdoInfo->uiSampleRate);
	WR_DWORD(&pAvi->head.s2.strh.info.ulStart, 0);
		//WR_DWORD(pAvi->head.s2.strh.info.ulLength, );
	WR_DWORD(&pAvi->head.s2.strh.info.ulSuggestedBufferSize, 0x3000);
	WR_DWORD(&pAvi->head.s2.strh.info.ulQuality, -1);
		//WR_DWORD(&pAvi->head.s2.strh.info.ulSampleSize, ado_block_align);

	WR_FOURCC(&pAvi->head.s2.strf.ulFourCC, "strf");
	WR_DWORD(&pAvi->head.s2.strf.ulSize, sizeof(pAvi->head.s2.strf)-ATOM_HEAD_SIZE);
		//WR_WORD(&pAvi->head.s2.strf.info.usFormatTag, 0x0006);	//AVI_WAVE_FORMAT_ALAW
		//WR_WORD(&pAvi->head.s2.strf.info.usChannels, pAdoInfo->ucChannel);
		//WR_DWORD(&pAvi->head.s2.strf.info.ulSamplesPerSec, pAdoInfo->uiSampleRate);
		//WR_DWORD(&pAvi->head.s2.strf.info.ulAvgBytesPerSec, pAdoInfo->uiSampleRate * ado_block_align );
		//WR_WORD(&pAvi->head.s2.strf.info.usBlockAlign, ado_block_align);
		//WR_WORD(&pAvi->head.s2.strf.info.usBitsPerSample, pAdoInfo->ucBitsPerSample);
		//WR_WORD(&pAvi->head.s2.strf.info.usSamplesPerBlock, );
	//}


	//data
	WR_FOURCC(&pAvi->movi.ulList, "LIST");
	//pAvi->movi.ulSize
	WR_FOURCC(&pAvi->movi.ulFourCC, "movi");

	//index
	for(i=0;i<AVI_INDEX_TABLE_NUM;i++)
		WR_FOURCC(&pAviInfo->AviIndex[i].idx1.ulFourCC, "idx1");
	//pAviInfo->AviIndex.idx1.ulSize

	WR_FOURCC(&pAviInfo->GPS.chunk_header.ulFourCC, "SLLT");
	WR_FOURCC(&pAviInfo->GSENSOR.chunk_header.ulFourCC, "SGLT");

	if (pdPASS != xTaskCreate(task_write_cmd_finish, "task_wb_finish", 512, pAviInfo,
		50, &pAviInfo->FinishCmdtask))
	{
		AVI_DBG("Could not create task wb finish\n");
		goto fail3; 
	}

	AVI_DBG("PASS !!\n");
	return pdPASS;
fail3:	
	writebuf_uninit(&pAviInfo->WBInfo);
fail2:		
	vPortFree(pAviInfo->pAviHeader);
	pAviInfo->pAviHeader = NULL;
fail1:	
	return pdFAIL;
}

/**
* @brief interface function - avi uninitialization
* @param pAviInfo avi structure
* @return return pdPASS if success
*/
int avi_uninit(AVI_Info_t *pAviInfo)
{
	int i;
	writebuf_uninit(&pAviInfo->WBInfo);
	vTaskDelete(pAviInfo->FinishCmdtask);
	for(i=0;i<AVI_INDEX_TABLE_NUM;i++)
	{
		if(pAviInfo->AviIndex[i].idxentry)
		{
			vPortFree(pAviInfo->AviIndex[i].idxentry);
			pAviInfo->AviIndex[i].idxentry = NULL;
		}
	}
	if(pAviInfo->has_gps == 1){
		if(pAviInfo->GPS.entry){
			vPortFree(pAviInfo->GPS.entry);	
			pAviInfo->GPS.entry = NULL;
		}
	}

	if(pAviInfo->has_gsensor == 1){
		if(pAviInfo->GSENSOR.entry){
			vPortFree(pAviInfo->GSENSOR.entry);	
			pAviInfo->GSENSOR.entry = NULL;
		}
	}

	vPortFree(pAviInfo->pAviHeader);
	pAviInfo->pAviHeader = NULL;
	return pdPASS;
}

/**
* @brief interface function - set infomation for video and audio stream
* @param pAviInfo avi structure
* @param pVdoInfo pointer for video structure
* @param pAdoInfo pointer for audio structure
* @return return pdPASS if success
* @warning modify the parameter before avi_write_header()
*/
int avi_set_stream_info(AVI_Info_t *pAviInfo, AviVdoInfo_t *pVdoInfo, AviAdoInfo_t *pAdoInfo, int max_stream_len)
{
	int ret = pdPASS;
	int max_index_size, i, tmp;
	if(pVdoInfo)
		memcpy(&pAviInfo->AviVideoInfo, pVdoInfo, sizeof(AviVdoInfo_t));
	else
	{
		AVI_DBG("set video info fail\n");
		ret = pdFAIL;
	}
	if(pAviInfo->has_audio){
		if(pAdoInfo)
			memcpy(&pAviInfo->AviAudioInfo, pAdoInfo, sizeof(AviAdoInfo_t));
		else
		{
			AVI_DBG("set audio info fail\n");
			ret = pdFAIL;
		}	
	}
	pAviInfo->MaxStreamLength = max_stream_len + 20;	//increase 20 sec buffer to avid index overflow

#if 0
	//debug
	AVI_DBG("mode = %d, %d x %d, fps = %d scale = %d\n",pVdoInfo->ucStreamMode, pVdoInfo->width, pVdoInfo->height,  
		pVdoInfo->ucFps, pVdoInfo->ucScale);
	AVI_DBG("fmt = %d, sample rate = %d, bps = %d, fps = %d, br = %d, ch = %d\n", 
		pAdoInfo->uiFormat ,pAdoInfo->uiSampleRate, pAdoInfo->ucBitsPerSample, pAdoInfo->uiFps, 
				pAdoInfo->uiBitRate, pAdoInfo->ucChannel);
#endif	
	AVI_DBG("max stream lenght = %d\n", pAviInfo->MaxStreamLength);
	
	//pointer to current skipframe
	if(pVdoInfo->height == 1080)
	{
		pAviInfo->SkipFrameLen = FHD_SKIP_FRAME_SIZE;
		pAviInfo->SkipFrame = fhd_skip_frame;
	}	
	else
	{
		pAviInfo->SkipFrameLen = HD_SKIP_FRAME_SIZE;
		pAviInfo->SkipFrame = hd_skip_frame;
	}

	//allocate index space
	max_index_size = MAX_INDEX_SIZE(pVdoInfo->ucFps, pAdoInfo->uiFps);
	AVI_DBG("max_index_size = %d\n", max_index_size);

	if( pAviInfo->has_gps == 1 ) {
#if defined( CONFIG_SN_GCC_SDK )
		pAviInfo->GPS.entry = pvPortMalloc(max_index_size, GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviInfo->GPS.entry = pvPortMalloc(max_index_size);
#endif		
		if(pAviInfo->GPS.entry == NULL){
			AVI_DBG("GPS allocate fail\n");
			return pdFAIL;
		}
	}

	if(pAviInfo->has_gsensor == 1) {
#if defined( CONFIG_SN_GCC_SDK )
		pAviInfo->GSENSOR.entry = pvPortMalloc(max_index_size, GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviInfo->GSENSOR.entry = pvPortMalloc(max_index_size);
#endif
		if(pAviInfo->GSENSOR.entry == NULL){
			AVI_DBG("GSENSOR allocate fail\n");
			if(pAviInfo->GPS.entry ) {
				vPortFree(pAviInfo->GPS.entry );
				pAviInfo->GPS.entry = NULL;
			}
			return pdFAIL;
		}
	}

	for(i=0; i< AVI_INDEX_TABLE_NUM; i++)
	{
#if defined( CONFIG_SN_GCC_SDK )
		pAviInfo->AviIndex[i].idxentry = pvPortMalloc(max_index_size, GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviInfo->AviIndex[i].idxentry = pvPortMalloc(max_index_size);
#endif		
		if(pAviInfo->AviIndex[i].idxentry == NULL)
		{
			AVI_DBG("pAviIndex[%d] allocate fail\n", i);
			tmp = i;
			goto fail;
		}
	}
	return ret;

fail:
	for(i=0;i<tmp;i++)
		vPortFree(pAviInfo->AviIndex[i].idxentry);
	return pdFAIL;

}

/**
* @brief interface function - set avi file name
* @param pAviInfo avi structure
* @param filename string for avi file name
* @return return pdPASS if success
* @warning modify the parameter before avi_write_header()
*/
int avi_set_file_name(AVI_Info_t *pAviInfo, char *filename)
{
	AVI_DBG("avi_set_file_name : filename = %s)\n", filename);
	
	if(filename && strlen(filename)<sizeof(pAviInfo->Filename))
		strcpy(pAviInfo->Filename, filename);
	else
	{
		AVI_DBG("set file name fail(%x, len = %d)\n", filename && strlen(filename));
		return pdFAIL;
	}	
	return pdPASS;
}

/**
* @brief interface function - get avi file name
* @param pAviInfo avi structure
* @return return pointer of file name
*/
char* avi_get_file_name(AVI_Info_t *pAviInfo)
{
	return pAviInfo->Filename;
}

void avi_clear_inteval_file_name(AVI_Info_t *pAviInfo)
{
	memset(pAviInfo->IntevalFilename, 0, AVI_FILENAME_LENGTH);
}

char* avi_get_inteval_file_name(AVI_Info_t *pAviInfo)
{
	return pAviInfo->IntevalFilename;
}

/**
* @brief interface function - open avi file and write avi header
* @param pAviInfo avi structure
* @return return pdPASS if success
*/
int avi_write_header(AVI_Info_t *pAviInfo)
{
	int i;
	AVIFileHeader_t *pAvi = pAviInfo->pAviHeader;
	AviVdoInfo_t *pVdoInfo = &pAviInfo->AviVideoInfo;
		
	AviAdoInfo_t *pAdoInfo = &pAviInfo->AviAudioInfo;
	uint16_t ado_block_align = 0, fmt_tag = 0;
	float ado_samples = 0;

	frame_info finfo;

	for(i=0;i<MAX_STREAM_NUM;i++)
	{
		pAviInfo->StartTick[i] = 0;//xTaskGetTickCount();
		pAviInfo->NumFrame[i] = 0;
	}	

	for(i=0;i<MAX_DATA_NUM;i++)
		pAviInfo->NumData[i] = 0;
	pAviInfo->rec_start_ret=WB_CMD_INIT;
	pAviInfo->pre_vdo_reach_time = 0;
	pAviInfo->vdo_accumlation_duration = 0;
	pAviInfo->cur_ubAVI_H264_PIC = 2;
	pAviInfo->cur_ubAVI_H264_POC = 3;
	pAviInfo->pre_ado_reach_time = 0;
	pAviInfo->ado_accumlation_duration = 0;

	
	pAviInfo->IndexOffset = 4;

	//memset(pAvi, 0, sizeof(AVIFileHeader_t));

	//pAdoInfo->ulChunkSize = pAdoInfo->uiSampleRate*pAdoInfo->ucBitsPerSample*5/1000;	//5ms length

	WR_DWORD(&pAvi->head.avih.info.ulMicroSecPerFrame, 1000000/pVdoInfo->ucFps);	
	WR_DWORD(&pAvi->head.avih.info.ulMaxBytesPerSec, pVdoInfo->uiBps);	
	WR_DWORD(&pAvi->head.avih.info.ulWidth, pVdoInfo->width);
	WR_DWORD(&pAvi->head.avih.info.ulHeight, pVdoInfo->height);

	//video
//	WR_FOURCC(&pAvi->head.s1.strh.info.fccHandler, pVdoInfo->ucStreamMode&2?"H264":"   ");
	if(pVdoInfo->ucStreamMode == REC_FMT_H264)
	{
		WR_FOURCC(&pAvi->head.s1.strh.info.fccHandler, "H264");
		AVI_DBG("H264\n");
	}
	else if(pVdoInfo->ucStreamMode == REC_FMT_MJPEG)
	{
		WR_FOURCC(&pAvi->head.s1.strh.info.fccHandler, "MJPG");
		AVI_DBG("MJPG\n");
	}
	else
	{
		WR_FOURCC(&pAvi->head.s1.strh.info.fccHandler, "    ");
		AVI_DBG("NONE\n");
	}	
		
	WR_DWORD(&pAvi->head.s1.strh.info.ulRate, pVdoInfo->ucFps);
	WR_WORD(&pAvi->head.s1.strh.info.rcFrame.right, pVdoInfo->width);
	WR_WORD(&pAvi->head.s1.strh.info.rcFrame.bottom, pVdoInfo->height);

	WR_DWORD(&pAvi->head.s1.strf.info.ulWidth, pVdoInfo->width);
	WR_DWORD(&pAvi->head.s1.strf.info.ulHeight, pVdoInfo->height);
	
//	WR_FOURCC(&pAvi->head.s1.strf.info.ulCompression, pVdoInfo->ucStreamMode&2?"H264":"   ");
	if(pVdoInfo->ucStreamMode == REC_FMT_H264)
	{
		WR_FOURCC(&pAvi->head.s1.strf.info.ulCompression, "H264\n");
	}
	else if(pVdoInfo->ucStreamMode == REC_FMT_MJPEG)
	{
		WR_FOURCC(&pAvi->head.s1.strf.info.ulCompression, "MJPG\n");
	}
	else
	{
		WR_FOURCC(&pAvi->head.s1.strf.info.ulCompression, "    \n");
	}
	
	WR_DWORD(&pAvi->head.s1.strf.info.ulSizeImage, pVdoInfo->width*pVdoInfo->height*3);

	//audio ...
	if(pAviInfo->has_audio){
		ado_block_align = (pAdoInfo->ucBitsPerSample>>3)*pAdoInfo->ucChannel;
		fmt_tag = Audio_format_tag_transfer(pAdoInfo->uiFormat);
		//AVI_PRINT(SYS_DBG, "fmt_tag = %d\n", fmt_tag);
		
		if (pAdoInfo->uiFormat == AUD_FORMAT_AAC) {
			ado_samples = (pAdoInfo->uiSampleRate*pAdoInfo->ucBitsPerSample*pAdoInfo->ucChannel)>>3;
			/* AAC Audio sample data is 2048 Bytes,  2048Bytes -> 128Bytes(around) */
			ado_samples = ado_samples / 2048;
			WR_DWORD(&pAvi->head.s2.strh.info.ulScale, (uint32_t)(pAdoInfo->uiSampleRate/ado_samples));
			WR_WORD(&pAvi->head.s2.strf.info.usBlockAlign, 768);	// Fixed nBlockAlign value is 768 for AAC
		} else {
			WR_DWORD(&pAvi->head.s2.strh.info.ulScale, 1);//(pAdoInfo->ucBitsPerSample>>3)*pAdoInfo->ucChannel);
			WR_WORD(&pAvi->head.s2.strf.info.usBlockAlign, ado_block_align);
		}
		WR_DWORD(&pAvi->head.s2.strh.info.ulRate, pAdoInfo->uiSampleRate);
		WR_DWORD(&pAvi->head.s2.strh.info.ulSampleSize, (pAdoInfo->uiFormat==AUD_FORMAT_AAC)?0:ado_block_align);

		WR_WORD(&pAvi->head.s2.strf.info.usFormatTag, fmt_tag);
		WR_WORD(&pAvi->head.s2.strf.info.usChannels, pAdoInfo->ucChannel);
		WR_DWORD(&pAvi->head.s2.strf.info.ulSamplesPerSec, pAdoInfo->uiSampleRate);
		WR_DWORD(&pAvi->head.s2.strf.info.ulAvgBytesPerSec, (pAdoInfo->uiBitRate*pAdoInfo->ucChannel)>>3);//pAdoInfo->uiSampleRate * ado_block_align );
		WR_WORD(&pAvi->head.s2.strf.info.usBitsPerSample, pAdoInfo->ucBitsPerSample);
	}

	writebuf_open_file(&pAviInfo->WBInfo, &pAviInfo->AviFile, pAviInfo->Filename);
	
	// the flow will not break if file is created(i.e. pAviInfo->rec_start_ret is WB_CMD_SUCCESS) but SD is removed immediately
	/*while(exists(pAviInfo->Filename) != 1)
	{
		if(pAviInfo->rec_start_ret==WB_CMD_FAIL)
		{		
			return pdFAIL;
		}
		vTaskDelay( 10/portTICK_PERIOD_MS );
	}*/

	// wait for wb to call the callback func. "avi_start_finish" in task_write_cmd_finish
	//AVI_PRINT(SYS_DBG, "11 rec_start_ret = %d \n",pAviInfo->rec_start_ret);
	while(pAviInfo->rec_start_ret==WB_CMD_INIT)
	{
		//AVI_PRINT(SYS_DBG, "rec_start_ret = %d \n",pAviInfo->rec_start_ret);
		vTaskDelay( 10/portTICK_PERIOD_MS );
	}

	if(pAviInfo->rec_start_ret==WB_CMD_FAIL) {	
		AVI_DBG("open file error\n");
		return pdFAIL;
	}

	if(exists(pAviInfo->Filename) != 1)
	{
		AVI_DBG("open %s successully but storage may be removed immediately\n", pAviInfo->Filename);
	}
	
	finfo.pAddr = (unsigned char*)pAvi;
	finfo.uiSize = (unsigned int)sizeof(AVIFileHeader_t);
	writebuf_write_frame(&pAviInfo->WBInfo, &finfo, 1);
	return pdPASS;

}

void write_avi_form(AVI_Info_t *pAviInfo ,uint8_t ucMode, char* flag, uint8_t ucKeyFrame, unsigned char* pFrame, int iFrameSize)
{
	unsigned int size;
	CHUNK_t Chunk;
	frame_info finfo[2];

	avi_write_index(pAviInfo,ucMode, ucKeyFrame, iFrameSize, flag);
	WR_FOURCC(&Chunk.ulFourCC, flag);
	WR_DWORD(&Chunk.ulSize, iFrameSize);
	finfo[0].pAddr = (unsigned char*)&Chunk;
	finfo[0].uiSize = (unsigned int)sizeof(CHUNK_t);
	size = iFrameSize+(iFrameSize&1);
	finfo[1].pAddr = pFrame;
	finfo[1].uiSize = (unsigned int)size;
	writebuf_write_frame(&pAviInfo->WBInfo, finfo, 2);
	pAviInfo->NumFrame[ucMode]++;

	if(ucMode == AVI_VIDEO) {
		if((pAviInfo->GPS_info != NULL) && (pAviInfo->GPS_info->ulChunkID == 0x1)) {
			avi_write_gps(pAviInfo, pAviInfo->NumData[AVI_GPS], (pAviInfo->NumFrame[ucMode]-1));
			pAviInfo->NumData[AVI_GPS]++;
		}

		if((pAviInfo->GSENSOR_info != NULL) && (pAviInfo->GSENSOR_info->ulChunkID == 0x1)) {
			avi_write_gsensor(pAviInfo, pAviInfo->NumData[AVI_GSENSOR], (pAviInfo->NumFrame[ucMode]-1));
			pAviInfo->NumData[AVI_GSENSOR]++;
		}
	}

}



void video_add_frame(AVI_Info_t *pAviInfo, char *flag ,uint8_t ucKeyFrame, unsigned char* pFrame, int iFrameSize, uint64_t uiTimeStamp)
{
	int process_count = 0, arrival_time = 0, i,fps_time_interval;
	uint64_t cur_time = 0;
	unsigned char *frameptr = NULL;
	unsigned char ucVdoFps = pAviInfo->AviVideoInfo.ucFps;


	fps_time_interval = (1000000+ucVdoFps-1)/ucVdoFps;

	if((pAviInfo->pre_vdo_reach_time == 0) || 
		(uiTimeStamp < pAviInfo->pre_vdo_reach_time) ||
		(uiTimeStamp - pAviInfo->pre_vdo_reach_time)>1000000) 
	{	//check overflow
		pAviInfo->pre_vdo_reach_time = uiTimeStamp;
		process_count = 0;
		//AVI_PRINT(SYS_DBG, "fps_time_interval = %d\n", fps_time_interval);
	}
	else 
	{
#if Padding_SKIP_FRAME
		//calculate how much frames we lost during SD encode to reach FPS
		cur_time =  uiTimeStamp;
		arrival_time = cur_time - pAviInfo->pre_vdo_reach_time;
		pAviInfo->vdo_accumlation_duration += arrival_time;
			
		if(pAviInfo->vdo_accumlation_duration > fps_time_interval)
			pAviInfo->vdo_accumlation_duration -= fps_time_interval;
		else
			pAviInfo->vdo_accumlation_duration = 0;

		
		if(pAviInfo->vdo_accumlation_duration > fps_time_interval) {
			process_count = pAviInfo->vdo_accumlation_duration/fps_time_interval;
			pAviInfo->vdo_accumlation_duration -= process_count * fps_time_interval;
		}
		else
			process_count = 0;

		pAviInfo->pre_vdo_reach_time = cur_time;

		//AVI_PRINT(SYS_DBG, "time = %d, acc = %d, count %d", arrival_time, pAviInfo->vdo_accumlation_duration,  process_count);

#else
		process_count = 0;
#endif
	}

	if(ucKeyFrame) {
		// reset PIC & POC
		pAviInfo->cur_ubAVI_H264_PIC = 2;
		pAviInfo->cur_ubAVI_H264_POC = 3;


		//add for AV sync
		//if((pAviInfo->vdo_accumlation_duration)<= (fps_time_interval*2/3))
		//	pAviInfo->vdo_accumlation_duration = 0;

	}
	else {

		frameptr = pFrame;
		frameptr += 6;
		*frameptr = pAviInfo->cur_ubAVI_H264_PIC;
		*(frameptr+1) = pAviInfo->cur_ubAVI_H264_POC;

		if(*(pFrame+4)==0x41){  // ref P frame Add PIC
			pAviInfo->cur_ubAVI_H264_PIC += 2;
		}
		pAviInfo->cur_ubAVI_H264_POC += 2;

	}

	if (pAviInfo->cur_ubAVI_H264_POC == 0x1) {			//POC is overflow.
		if ((pAviInfo->cur_ubAVI_H264_PIC % 2) == 0) {	//PIC is even value	normally.					   
			pAviInfo->cur_ubAVI_H264_PIC += 1;			//if POC overflow was odd times(first,	third...), PIC change to odd value.
		}
		else {
			pAviInfo->cur_ubAVI_H264_PIC -= 1;			//if POC overflow	was	even times(second, fourth...), PIC back	to even	value.
		}
	}
	write_avi_form(pAviInfo ,AVI_VIDEO, flag, ucKeyFrame, pFrame, iFrameSize);

	// Add skip frame 
//	if(process_count > 0) {
	if((process_count > 0) && !(*(pFrame+4) == 0x01)) {
#if defined( CONFIG_SN_GCC_SDK )
		unsigned char *skipframe = pvPortMalloc(pAviInfo->SkipFrameLen, GFP_KERNEL, MODULE_MID_RECORD);
#else		
		unsigned char *skipframe = pvPortMalloc(pAviInfo->SkipFrameLen);
#endif		
		//AVI_PRINT(SYS_DBG, "++++++add %d skip frame\n", process_count);
		memcpy(skipframe, pAviInfo->SkipFrame, pAviInfo->SkipFrameLen);
		for(i=0 ; i<process_count; i++) {
			frameptr = skipframe;
			frameptr += 6;

			*frameptr = pAviInfo->cur_ubAVI_H264_PIC;
			*(frameptr+1) = pAviInfo->cur_ubAVI_H264_POC;
			//pAviInfo->cur_ubAVI_H264_PIC += 2;
			pAviInfo->cur_ubAVI_H264_POC += 2;
			// insert skip frame 
			write_avi_form(pAviInfo ,AVI_VIDEO, flag, 0 , skipframe, pAviInfo->SkipFrameLen);
		}
		vPortFree(skipframe);
	}

}



void audio_add_frame(AVI_Info_t *pAviInfo, char *flag , unsigned char* pFrame, int iFrameSize, uint64_t uiTimeStamp)
{
	uint64_t audio_cur_time = 0;
	uint64_t av_interval_time = 0;
	uint64_t  video_time;
	int64_t arrival_time = 0;
	uint64_t tmp_accu_duration = 0;
	unsigned int uiAdoFps = pAviInfo->AviAudioInfo.uiFps;
	unsigned char ucVdoFps = pAviInfo->AviVideoInfo.ucFps;
	uint64_t fps_time_interval = 1000000/uiAdoFps, vdo_interval = (1000000+ucVdoFps-1)/ucVdoFps;
	unsigned int audio_insert_thr = 0, audio_skip_thr = 0;
	
	audio_insert_thr = 2*fps_time_interval;
	audio_skip_thr = fps_time_interval;

	//check threshold is large enough to tolerate larger video frame interval 
	if(audio_insert_thr<vdo_interval)
		audio_insert_thr = vdo_interval;
	if(audio_skip_thr<vdo_interval)
			audio_skip_thr = vdo_interval;
	
	if(pAviInfo->pre_ado_reach_time == 0 || 
		uiTimeStamp < pAviInfo->pre_ado_reach_time ||	//check overflow
		(uiTimeStamp-pAviInfo->pre_ado_reach_time)>1000000)
	{
		pAviInfo->pre_ado_reach_time  = uiTimeStamp;
		//AVI_PRINT(SYS_DBG, "fps_time_interval = %d\n", fps_time_interval);
	}
	else { //calculate how much frames we lost during SD encode to reach FPS
		audio_cur_time = uiTimeStamp;
		arrival_time = audio_cur_time - pAviInfo->pre_ado_reach_time;
		pAviInfo->ado_accumlation_duration += arrival_time;
		pAviInfo->pre_ado_reach_time = audio_cur_time;
	}
	
	video_time = ((int64_t)pAviInfo->NumFrame[AVI_VIDEO] * 1000000) / ucVdoFps;	//ms
	tmp_accu_duration = pAviInfo->ado_accumlation_duration;
	//AVI_PRINT(SYS_DBG, "arr = %d, vdo = %d, acc = %d\n", arrival_time, video_time, pAviInfo->ado_accumlation_duration);

	if (video_time >= tmp_accu_duration) {

		write_avi_form(pAviInfo ,AVI_AUDIO, flag, 0, pFrame, iFrameSize);		
		av_interval_time = video_time - tmp_accu_duration;		
		if (av_interval_time > audio_insert_thr) {

			tmp_accu_duration += fps_time_interval;			
			write_avi_form(pAviInfo ,AVI_AUDIO, flag, 0, pFrame, iFrameSize);
			//AVI_PRINT(SYS_DBG, "insert one frame\n");
		}
	} else {
		av_interval_time = tmp_accu_duration - video_time;
		if (av_interval_time >  audio_skip_thr ) {

			tmp_accu_duration -= fps_time_interval;
			//AVI_PRINT(SYS_DBG, "skip one frame\n");
		} else {

			write_avi_form(pAviInfo ,AVI_AUDIO, flag, 0, pFrame, iFrameSize); 
			
		}
	}
	pAviInfo->ado_accumlation_duration = tmp_accu_duration;

}

/**
* @brief interface function - write avi packet
* @param pAviInfo avi structure
* @param ucMode AVI_VIDEO: video data, AVI_AUDIO: audio data
* @param ucKeyFrame is i frame or note in video frame
* @param pFrame pointer for data
* @param iFrameSize data size
* @return return pdPASS if success
*/
int avi_write_packet(AVI_Info_t *pAviInfo ,uint8_t ucMode,uint8_t ucKeyFrame, unsigned char* pFrame, int iFrameSize, uint64_t uiTimeStamp)
{

	if(!iFrameSize)
	{
		AVI_DBG("iFrameSize is zero\n");
		return pdPASS;
	}
	if(ucMode == AVI_VIDEO)
	{
		video_add_frame(pAviInfo, "00dc",ucKeyFrame, pFrame, iFrameSize, uiTimeStamp);
	}
	
	else if(ucMode == AVI_AUDIO)
	{
		if(pAviInfo->has_audio){
			if(pAviInfo->AviAudioInfo.uiFormat==AUD_FORMAT_AAC)
			{
				// work around for UVC dashcam
				audio_add_frame(pAviInfo, "01wb", pFrame, iFrameSize, uiTimeStamp);
//				audio_add_frame(pAviInfo, "01wc", pFrame, iFrameSize, uiTimeStamp);
			}	
			else
			{
				audio_add_frame(pAviInfo, "01wb", pFrame, iFrameSize, uiTimeStamp);
			}	
		}
	}

	else
	{
		AVI_DBG("ucMode = %d undefined\n", ucMode);
		return pdPASS;
	}
	return pdPASS;
}

/**
* @brief interface function - fill remain infomation and close avi file
* @param pAviInfo avi structure
* @return return pdPASS if success
*/
int avi_write_trailer(AVI_Info_t *pAviInfo)
{
	AVIFileHeader_t *pAviHdr = pAviInfo->pAviHeader;;
	AVIIndex_t *pAviIdx = &pAviInfo->AviIndex[pAviInfo->CurIndex];

	AVIGPS_t *pGPS = &pAviInfo->GPS;
	AVIGSENSOR_t *pGSENSOR= &pAviInfo->GSENSOR;
	uint32_t data_size = 0;

	int mode, TotalFrame = 0,i;
	uint32_t riff_size = 0;
	//FIL *fp = &pAviInfo->AviFile;
	frame_info finfo[2];

	for(mode=0; mode<MAX_STREAM_NUM; mode++)
	{
		TotalFrame += pAviInfo->NumFrame[mode];
	}

	//write index block to file
	WR_DWORD(&pAviIdx->idx1.ulSize,INDEX_BLOCK_SIZE(TotalFrame)-ATOM_HEAD_SIZE);
	finfo[0].pAddr = (unsigned char*)&pAviIdx->idx1;
	finfo[0].uiSize = sizeof(pAviIdx->idx1);
	finfo[1].pAddr = (unsigned char*)pAviIdx->idxentry;
	finfo[1].uiSize = INDEX_SIZE(TotalFrame);
	writebuf_write_frame(&pAviInfo->WBInfo, finfo, 2);
	pAviInfo->CurIndex = (pAviInfo->CurIndex+1)%AVI_INDEX_TABLE_NUM;
	
	AVI_DBG("TotalFrame = %d\n", TotalFrame);
		
	//riff_size = f_size(fp) - ATOM_HEAD_SIZE;
	riff_size = sizeof(AVIFileHeader_t) + pAviInfo->IndexOffset - 4 + INDEX_SIZE(TotalFrame);	//hdr + dada block + index
	AVI_DBG("riff_size = %d", riff_size);
	//write movi size, write riff size, write total frame , write lendth
//	AVI_PRINT(SYS_DBG, "GPS size = %d\n", sizeof(AVIGPSINFO_t));

	if(pAviInfo->NumData[AVI_GPS] != 0) {
		data_size = sizeof(AVIGPSINFO_t)*pAviInfo->NumData[AVI_GPS];
		riff_size += data_size;
		WR_DWORD(&pGPS->chunk_header.ulSize, data_size);
		finfo[0].pAddr = (unsigned char*)&pGPS->chunk_header;
		finfo[0].uiSize = sizeof(pGPS->chunk_header);
		finfo[1].pAddr = (unsigned char*)pGPS->entry;
		finfo[1].uiSize = data_size;
		writebuf_write_frame(&pAviInfo->WBInfo, finfo, 2);
	}

	if(pAviInfo->NumData[AVI_GSENSOR] != 0) {
		data_size = sizeof(AVIGSENSORINFO_t)*pAviInfo->NumData[AVI_GSENSOR];
		riff_size += data_size;
		WR_DWORD(&pGSENSOR->chunk_header.ulSize, data_size);
		finfo[0].pAddr = (unsigned char*)&pGSENSOR->chunk_header;
		finfo[0].uiSize = sizeof(pGSENSOR->chunk_header);
		finfo[1].pAddr = (unsigned char*)pGSENSOR->entry;
		finfo[1].uiSize = data_size;
		writebuf_write_frame(&pAviInfo->WBInfo, finfo, 2);
	}


	write_back_to_file(pAviInfo, &pAviHdr->RIFF.ulSize, riff_size);
	write_back_to_file(pAviInfo, &pAviHdr->movi.ulSize, pAviInfo->IndexOffset);
	write_back_to_file(pAviInfo, &pAviHdr->head.avih.info.ulTotalFrames, pAviInfo->NumFrame[AVI_VIDEO]);
	write_back_to_file(pAviInfo, &pAviHdr->head.s1.strh.info.ulLength, pAviInfo->NumFrame[AVI_VIDEO]); 
	if(pAviInfo->has_audio)
		write_back_to_file(pAviInfo, &pAviHdr->head.s2.strh.info.ulLength, pAviInfo->NumFrame[AVI_AUDIO]); 

	writebuf_close_file(&pAviInfo->WBInfo);

	//clear frame number to sync with current filename
	for(i=0;i<MAX_STREAM_NUM;i++)
		pAviInfo->NumFrame[i] = 0;

	for(i=0;i<MAX_DATA_NUM;i++)
		pAviInfo->NumData[i] = 0;


	
	return pdPASS;

}

/**
* @brief write avi gps block 
*/
int avi_write_gps(AVI_Info_t *pAviInfo, int count, int video_frame)
{
	AVIGPS_t *pGPS = &pAviInfo->GPS;

	pAviInfo->GPS_info->ulChunkID = 0;
//	WR_DWORD(&pGPS->entry[count].ulChunkID, video_frame+1)
	pGPS->entry[count].ulChunkID = ((video_frame>>24)&0x000000ff)
								+ ((video_frame>>8)&0x0000ff00)
								+ ((video_frame<<8)&0x00ff0000)
								+ ((video_frame<<24)&0xff000000);

//	pGPS->entry[count].ucGPSStatus =  pAviInfo->GPS_info->ucGPSStatus;
	pGPS->entry[count].ucGPSStatus =  1;

	///////////// 
//	pGPS->entry[count].ssLonDegInt = pAviInfo->GPS_info->ssLonDegInt;
	pGPS->entry[count].ssLonDegInt = (((pAviInfo->GPS_info->ssLonDegInt)>>8)&0xff)
								+ (((pAviInfo->GPS_info->ssLonDegInt)<<8)&0xff00);

//	pGPS->entry[count].ulLonDegDec = pAviInfo->GPS_info->ulLonDegDec;
	pGPS->entry[count].ulLonDegDec = ((pAviInfo->GPS_info->ulLonDegDec>>24)&0x000000ff)
								+ ((pAviInfo->GPS_info->ulLonDegDec>>8)&0x0000ff00)
								+ ((pAviInfo->GPS_info->ulLonDegDec<<8)&0x00ff0000)
								+ ((pAviInfo->GPS_info->ulLonDegDec<<24)&0xff000000);

	///////////// 
//	pGPS->entry[count].ssLatDegInt = pAviInfo->GPS_info->ssLatDegInt;
	pGPS->entry[count].ssLatDegInt = (((pAviInfo->GPS_info->ssLatDegInt)>>8)&0xff)
								+ (((pAviInfo->GPS_info->ssLatDegInt)<<8)&0xff00);

//	pGPS->entry[count].ulLatDegDec = pAviInfo->GPS_info->ulLatDegDec;
	pGPS->entry[count].ulLatDegDec = ((pAviInfo->GPS_info->ulLatDegDec>>24)&0x000000ff)
								+ ((pAviInfo->GPS_info->ulLatDegDec>>8)&0x0000ff00)
								+ ((pAviInfo->GPS_info->ulLatDegDec<<8)&0x00ff0000)
								+ ((pAviInfo->GPS_info->ulLatDegDec<<24)&0xff000000);
	///////////// Altitude
//	pGPS->entry[count].usAltitude = pAviInfo->GPS_info->usAltitude;
	pGPS->entry[count].usAltitude = (((pAviInfo->GPS_info->usAltitude)>>8)&0xff)
								+ (((pAviInfo->GPS_info->usAltitude)<<8)&0xff00);

	/////////////  speed
//	pGPS->entry[count].usSpeed = pAviInfo->GPS_info->usSpeed;
	pGPS->entry[count].usSpeed = (((pAviInfo->GPS_info->usSpeed)>>8)&0xff)
								+ (((pAviInfo->GPS_info->usSpeed)<<8)&0xff00);

	pGPS->entry[count].ucGpsHour = pAviInfo->GPS_info->ucGpsHour;
	pGPS->entry[count].ucGpsMinute = pAviInfo->GPS_info->ucGpsMinute;
	pGPS->entry[count].ucGpsSecond = pAviInfo->GPS_info->ucGpsSecond;

//	WR_WORD(&pGPS->entry[count].usGpsYear, pAviInfo->GPS_info->usGpsYear)
//	pGPS->entry[count].usGpsYear = pAviInfo->GPS_info->usGpsYear;
	pGPS->entry[count].usGpsYear = (((pAviInfo->GPS_info->usGpsYear)>>8)&0xff)
								+ (((pAviInfo->GPS_info->usGpsYear)<<8)&0xff00);

	pGPS->entry[count].ucGpsMonth = pAviInfo->GPS_info->ucGpsMonth;
	pGPS->entry[count].ucGpsDay = pAviInfo->GPS_info->ucGpsDay;

	return pdPASS;
}

/**
* @brief write avi G-sensor block 
*/
int avi_write_gsensor(AVI_Info_t *pAviInfo, int count, int video_frame)
{
	AVIGSENSOR_t *pGSENSOR = &pAviInfo->GSENSOR;

	pAviInfo->GSENSOR_info->ulChunkID = 0;
//	WR_DWORD(&pGSENSOR->entry[count].ulChunkID, video_frame)
	pGSENSOR->entry[count].ulChunkID = ((video_frame>>24)&0x000000ff)
								+ ((video_frame>>8)&0x0000ff00)
								+ ((video_frame<<8)&0x00ff0000)
								+ ((video_frame<<24)&0xff000000);


	pGSENSOR->entry[count].ucAcceRange = pAviInfo->GSENSOR_info->ucAcceRange;
	pGSENSOR->entry[count].ucGVInt_X = pAviInfo->GSENSOR_info->ucGVInt_X;
//	pGSENSOR->entry[count].ulGVDec_X = pAviInfo->GSENSOR_info->ulGVDec_X;
	pGSENSOR->entry[count].ulGVDec_X = ((pAviInfo->GSENSOR_info->ulGVDec_X>>24)&0x000000ff)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_X>>8)&0x0000ff00)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_X<<8)&0x00ff0000)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_X<<24)&0xff000000);
								
	pGSENSOR->entry[count].ucGVInt_Y = pAviInfo->GSENSOR_info->ucGVInt_Y;
//	pGSENSOR->entry[count].ulGVDec_Y = pAviInfo->GSENSOR_info->ulGVDec_Y;
	pGSENSOR->entry[count].ulGVDec_Y = ((pAviInfo->GSENSOR_info->ulGVDec_Y>>24)&0x000000ff)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_Y>>8)&0x0000ff00)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_Y<<8)&0x00ff0000)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_Y<<24)&0xff000000);

	pGSENSOR->entry[count].ucGVInt_Z = pAviInfo->GSENSOR_info->ucGVInt_Z;
//	pGSENSOR->entry[count].ulGVDec_Z = pAviInfo->GSENSOR_info->ulGVDec_Z;
	pGSENSOR->entry[count].ulGVDec_Z = ((pAviInfo->GSENSOR_info->ulGVDec_Z>>24)&0x000000ff)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_Z>>8)&0x0000ff00)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_Z<<8)&0x00ff0000)
								+ ((pAviInfo->GSENSOR_info->ulGVDec_Z<<24)&0xff000000);
								
	return pdPASS;
}

/**
* @brief write avi index block 
*/
int avi_write_index(AVI_Info_t *pAviInfo ,uint8_t ucMode,uint8_t ucKeyFrame, int iFrameSize , char *flag)
{

	AVIIndex_t *pAviIdx = &pAviInfo->AviIndex[pAviInfo->CurIndex];
	int frame_num = 0, max_frame_num,i;

	for(i=0;i<MAX_STREAM_NUM;i++)
		frame_num += pAviInfo->NumFrame[i];

	if(pAviInfo->has_audio == 1)
	{
		max_frame_num = MAX_FRAME_NUM(pAviInfo->AviVideoInfo.ucFps, pAviInfo->AviAudioInfo.uiFps);
	}
	else
	{
		max_frame_num = MAX_FRAME_NUM(pAviInfo->AviVideoInfo.ucFps, 0);
	}

	if(frame_num>=max_frame_num)
	{
		AVI_DBG("frame num = %d over range(max = %d)\n", frame_num, max_frame_num);
		return pdFAIL;
	}

	WR_FOURCC(&pAviIdx->idxentry[frame_num].ulckid , flag);
	
	WR_DWORD(&pAviIdx->idxentry[frame_num].ulFlags, ucKeyFrame?AVI_KEYFRAME:0);	

	WR_DWORD(&pAviIdx->idxentry[frame_num].ulChunkOffset, pAviInfo->IndexOffset);
	WR_DWORD(&pAviIdx->idxentry[frame_num].ulChunkLength, iFrameSize);
	pAviInfo->IndexOffset += ATOM_HEAD_SIZE+iFrameSize+(iFrameSize&1);

	return pdPASS;
}

/**
* @brief check time stamp to add skip frame if frame loss 
*/

void write_back_to_file(AVI_Info_t *pAviInfo, void* addr, uint32_t data)
{
	uint32_t tmp;
	unsigned long pos;
	pos = (unsigned int)addr-(unsigned int)pAviInfo->pAviHeader;
	WR_DWORD(&tmp,data);
	writebuf_fill_data(&pAviInfo->WBInfo, pos, tmp);
}

uint16_t Audio_format_tag_transfer(unsigned int ado_fmt)
{
	switch(ado_fmt)
	{
		case AUD_FORMAT_A_LAW:
			return AVI_WAVE_FORMAT_ALAW;
		case AUD_FORMAT_MU_LAW:	
			return AVI_WAVE_FORMAT_MULAW;
		case AUD_FORMAT_S16_LE:	
			return AVI_WAVE_FORMAT_PCM;
		case AUD_FORMAT_AAC:
			return AVI_WAVE_FORMAT_AAC;
		default:
			AVI_DBG("ado_fmt = %d undefined!!\n", ado_fmt);
			return AVI_WAVE_FORMAT_PCM;
			
	}

}

void avi_start_finish(AVI_Info_t *pAviInfo, CMDStatus_t status)
{
	pAviInfo->rec_start_ret =status;
	//AVI_PRINT(SYS_DBG, "pAviInfo->rec_start_ret==%d!!\n",pAviInfo->rec_start_ret);
}



void avi_set_cb(AVI_Info_t *pAviInfo, avi_cb_t hdr_cb, avi_cb_t tra_cb,avi_cb_t media_cb, void *param)
{
	pAviInfo->hdr_cb = hdr_cb;
	pAviInfo->tra_cb= tra_cb;
	pAviInfo->media_start_cb=media_cb;
	pAviInfo->cb_parm = param;
}

void task_write_cmd_finish(void *pvParameters)
{
	AVI_Info_t *pAviInfo = (AVI_Info_t *)pvParameters;
	ReturnCmd_t finished_cmd;
	xQueueHandle* pQueueFinish; 
	pQueueFinish = writebuf_get_finish_queue(&pAviInfo->WBInfo);
	while(pQueueFinish)
	{
		xQueueReceive(*pQueueFinish,&finished_cmd,portMAX_DELAY);
		if(finished_cmd.cmd_type == WB_CMD_FOPEN)
		{
			//AVI_PRINT(SYS_DBG, "WB_CMD_FOPEN\n");
			if(pAviInfo->hdr_cb && pAviInfo->cb_parm)	
				pAviInfo->hdr_cb(pAviInfo->cb_parm, finished_cmd.status);
			if(pAviInfo->media_start_cb && pAviInfo->cb_parm)
			{
				pAviInfo->media_start_cb(pAviInfo, finished_cmd.status);
			}
		}
		else if(finished_cmd.cmd_type == WB_CMD_FCLOSE)
		{
			//AVI_PRINT(SYS_DBG, "WB_CMD_FCLOSE\n");
			if(pAviInfo->tra_cb && pAviInfo->cb_parm)
				pAviInfo->tra_cb(pAviInfo->cb_parm, finished_cmd.status);
		}
	}
	vTaskDelete(NULL);
	

}


//----- for playback middleware -----
/**
* @brief interface function - get video or audio total frame from avi hearder.
* @param AviHeader avi file header structure.
* @param ubStreamType type of video or audio.
* @return return frames number.
*/
uint32_t avi_get_movi_frmcnt(AVIFileHeader_t AviHeader, uint8_t ubStreamType)
{
	if(ubStreamType == AV_TYPE_VIDEO){
		return AviHeader.head.s1.strh.info.ulLength;
	}
	else if(ubStreamType == AV_TYPE_AUDIO){
		return AviHeader.head.s2.strh.info.ulLength;
	}
	else{
		return 0;
	}
}

/**
* @brief interface function - get video or audio rate from avi hearder.
* @param AviHeader avi file header structure.
* @param ubStreamType type of video or audio.
* @return return value of rate.
*/
uint32_t avi_get_rate(AVIFileHeader_t AviHeader, uint8_t ubStreamType)
{
	if(ubStreamType == AV_TYPE_VIDEO){
		return AviHeader.head.s1.strh.info.ulRate;
	}
	else if(ubStreamType == AV_TYPE_AUDIO){
		return AviHeader.head.s2.strh.info.ulRate;
	}
	else{
		return 0;
	}
}

#if 0
uint32_t avi_get_scale(AVIFileHeader_t AviHeader, uint8_t ubStreamType)
{
	if(ubStreamType == AV_TYPE_VIDEO){
		return AviHeader.head.s1.strh.info.ulScale;
	}
	#if AVI_HAS_AUDIO
	else if(ubStreamType == AV_TYPE_AUDIO){
		return AviHeader.head.s2.strh.info.ulScale;
	}
	#endif
	else{
		return 0;
	}
}
#endif

/**
* @brief interface function - get audio sample rate.
* @param AviHeader avi file header structure.
* @return return sample rate.
*/
uint32_t avi_get_ado_sample_rate(AVIFileHeader_t AviHeader)
{
	return AviHeader.head.s2.strf.info.ulSamplesPerSec;
}

/**
* @brief interface function - get audio bit rate.
* @param AviHeader avi file header structure.
* @return return bit rate.
*/
uint32_t avi_get_ado_bits_per_sample(AVIFileHeader_t AviHeader)
{
	return AviHeader.head.s2.strf.info.usBitsPerSample;
}

/**
* @brief interface function - get video resolution from avi hearder.
* @param AviHeader avi file header structure.
* @param width return width.
* @param height return height.
* @return return pdPASS if success.
*/
int avi_get_vdo_resolution(AVIFileHeader_t AviHeader, uint32_t *width, uint32_t *height)
{
	*width = AviHeader.head.avih.info.ulWidth;
	*height = AviHeader.head.avih.info.ulHeight;
	
	return pdPASS;
}

/**
* @brief interface function - set index pointer when file recording.
* @param pAviReadInfo avi file header structure.
* @param pAviIndex avi index structure pointer.
* @return return pdPASS if success.
*/
int avi_set_recording_index_ptr(AVI_read_Info_t *pAviReadInfo, AVIIndex_t *pAviIndex)
{
	if(pAviReadInfo != NULL){
		if(avi_check_fourcc(pAviIndex->idx1.ulFourCC, "idx1") == pdPASS){
			pAviReadInfo->FrmIdxNum = 0xFFFFFFFF;
			pAviReadInfo->pAviIndex = pAviIndex->idxentry;
			pAviReadInfo->IdxRdy = 1;
			pAviReadInfo->ReadRecordingFlag = 1;
		
			AVI_DBG("set recording avi index ok.......\n");
			return pdPASS;
		}

		//AVI_PRINT(SYS_DBG, "recording avi index FOURCC error.\n");
	}
	
	pAviReadInfo->ReadRecordingFlag = 0;
	return pdFAIL;
}

/**
* @brief interface function - read avi file initialization.
* @param pAviReadInfo read avi file structure pointer.
* @return return pdPASS if success.
*/
int avi_read_init(AVI_read_Info_t *pAviReadInfo)
{
	pAviReadInfo->FrmIdx = 0;
	pAviReadInfo->pFrmbuf = NULL;
	pAviReadInfo->dev=f_get_drv();
	
	if(f_open(&pAviReadInfo->AviFile, pAviReadInfo->Filename, FA_READ|FA_OPEN_EXISTING) != FR_OK){
		AVI_DBG("open %s file fail.\n", pAviReadInfo->Filename);
		goto fail1;
	}

	if(avi_read_header(pAviReadInfo) != pdPASS){
		goto fail2;
	}

	if(pAviReadInfo->ReadRecordingFlag == 0){
		if(avi_read_index(pAviReadInfo) != pdPASS){
			//AVI_PRINT(SYS_DBG, "%s:load avi index fail.\n",pAviReadInfo->Filename);
		}
	}
	
	if(avi_reset_frmidx(pAviReadInfo) != pdPASS){
		//AVI_PRINT(SYS_DBG, "reset frame index fail!!!!!\n");
		//goto fail4;
	}

	return pdPASS;

//fail4:
	//vPortFree(pAviReadInfo->pAviHeader);

fail2:
	f_close(&pAviReadInfo->AviFile);
	
fail1:
	return pdFAIL;
}

/**
* @brief interface function - read avi file uninitialization.
* @param pAviReadInfo read avi file structure pointer.
* @return return pdPASS if success.
*/
int avi_read_uninit(AVI_read_Info_t *pAviReadInfo)
{
#if fast_seek
	if(pAviReadInfo->AviFile.cltbl != NULL){
		pAviReadInfo->AviFile.cltbl = NULL;
		vPortFree(pAviReadInfo->clmt);
	}
#endif

	if((pAviReadInfo->IdxRdy)&&(pAviReadInfo->ReadRecordingFlag == 0)){    //if read recording ramindex not free aviindex, aviindex free by middleware record
		if(pAviReadInfo->dev==REC_DEV_USBH){
			if(pAviReadInfo->OriginAviIndexAddr)
				vPortFree(pAviReadInfo->OriginAviIndexAddr);
			pAviReadInfo->OriginAviIndexAddr = NULL;
			
			pAviReadInfo->pAviIndex = NULL;
		}else{
			if(pAviReadInfo->pAviIndex)
				vPortFree(pAviReadInfo->pAviIndex);
			pAviReadInfo->pAviIndex = NULL;
		}
	}
	
    if(pAviReadInfo->pAviHeader!=NULL){
		if(pAviReadInfo->dev==REC_DEV_USBH){
			if(pAviReadInfo->OriginAviHeaderAddr)
				vPortFree(pAviReadInfo->OriginAviHeaderAddr);
			pAviReadInfo->OriginAviHeaderAddr = NULL;
			
			pAviReadInfo->pAviHeader = NULL;
		}else{
			if(pAviReadInfo->pAviHeader)
				vPortFree(pAviReadInfo->pAviHeader);
			pAviReadInfo->pAviHeader = NULL;
		}
    }
	if(pAviReadInfo->pFrmbuf!=NULL)
	{
		vPortFree(pAviReadInfo->pFrmbuf);
		pAviReadInfo->pFrmbuf = NULL;
	}
	f_close(&pAviReadInfo->AviFile);
	
	return pdPASS;
}

int avi_read_header(AVI_read_Info_t *pAviReadInfo)
{
	int ret,fptr_offset;
	uint32_t readsize;

	if(pAviReadInfo->dev==REC_DEV_USBH){
		fptr_offset = 512 - (pAviReadInfo->AviFile.fptr%512);
#if defined( CONFIG_SN_GCC_SDK )
		pAviReadInfo->OriginAviHeaderAddr= (AVIFileHeader_t*)pvPortMalloc((sizeof(AVIFileHeader_t) + 512 + 512), GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviReadInfo->OriginAviHeaderAddr = (uint8_t *)((AVIFileHeader_t*)pvPortMalloc((sizeof(AVIFileHeader_t) + 512 + 512)));
#endif		
		pAviReadInfo->pAviHeader = (AVIFileHeader_t*) ((((unsigned int) pAviReadInfo->OriginAviHeaderAddr + 512) &(0xFFFFFE00)) + (512 - fptr_offset));
		//print_msg ("xxxxxxxxxxxxxxxxxxxx pAviHeader=%x\n",pAviReadInfo->pAviHeader);
		//print_msg ("xxxxxxxxxxxxxxxxxxxx pAviHeader ptr=%x , %x\n",pAviReadInfo->AviFile.fptr, fptr_offset);
	}
	else
	{
#if defined( CONFIG_SN_GCC_SDK )
		pAviReadInfo->pAviHeader = (AVIFileHeader_t*)pvPortMalloc(sizeof(AVIFileHeader_t), GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviReadInfo->pAviHeader = (AVIFileHeader_t*)pvPortMalloc(sizeof(AVIFileHeader_t));
#endif		
	}

	if(pAviReadInfo->dev==REC_DEV_USBH) {
		if(pAviReadInfo->OriginAviHeaderAddr == NULL){
			AVI_DBG("allocate memory fail (header).\n");
			goto fail1;
		}
	}else {
		if(pAviReadInfo->pAviHeader == NULL){
			AVI_DBG("allocate memory fail (header).\n");
			goto fail1;
		}
	}
	
	if(f_lseek(&pAviReadInfo->AviFile, 0) != FR_OK){
		AVI_DBG("seek to header fail.\n");
		goto fail2;
	}

	ret = f_read(&pAviReadInfo->AviFile, pAviReadInfo->pAviHeader, sizeof(AVIFileHeader_t), &readsize);
	if((ret != FR_OK) || (sizeof(AVIFileHeader_t) != readsize)){
		AVI_DBG("read file header fail(ret = %d, size = %d, %d).\n", ret, sizeof(AVIFileHeader_t), readsize);
		goto fail2;
	}

	if(pAviReadInfo->pAviHeader->head.avih.info.ulWidth == 1920 && pAviReadInfo->pAviHeader->head.avih.info.ulHeight == 1080)
		pAviReadInfo->avi_fb_size = FHD_DEMUX_FRAME_SIZE;
	else 
		pAviReadInfo->avi_fb_size = HD_DEMUX_FRAME_SIZE;
	
	return pdPASS;
	
fail2:
	if(pAviReadInfo->dev==REC_DEV_USBH){
		if(pAviReadInfo->OriginAviHeaderAddr)
			vPortFree(pAviReadInfo->OriginAviHeaderAddr);
		pAviReadInfo->OriginAviHeaderAddr=NULL;
		pAviReadInfo->pAviHeader = NULL;
	}else{
		if(pAviReadInfo->pAviHeader)
			vPortFree(pAviReadInfo->pAviHeader);
		pAviReadInfo->pAviHeader = NULL;
	}
fail1:
	return pdFAIL;
}

int avi_check_index_exist(AVI_read_Info_t *pAviReadInfo)
{
	if(pAviReadInfo->pAviHeader->head.avih.info.ulFlags & AVI_HASINDEX){
		return pdPASS;
	}
	else{
		return pdFAIL;
	}
}

int avi_read_index(AVI_read_Info_t *pAviReadInfo)
{
	int ret,fptr_offset;
	uint32_t size, readsize, offset;
	uint8_t idx_info[8];
	
	if(avi_check_index_exist(pAviReadInfo) == pdFAIL){
		AVI_DBG("file none index.\n");
		goto fail1;
	}
	
	offset = pAviReadInfo->pAviHeader->movi.ulSize + sizeof(AVIFileHeader_t) - sizeof(FOURCC);
	//AVI_PRINT("avi index offset= %d.\n", offset);

	if(f_lseek(&pAviReadInfo->AviFile, offset) != FR_OK){
		AVI_DBG("seek to index fail.\n");
		goto fail1;
	}
	
	ret = f_read(&pAviReadInfo->AviFile, idx_info, sizeof(CHUNK_t), &readsize);
	if((ret != FR_OK) || (sizeof(CHUNK_t) != readsize)){
		AVI_DBG("read index info fail(ret = %d, size = %d, %d).\n", ret, sizeof(CHUNK_t), readsize);
		goto fail1;
	}
	
	if((idx_info[0] != 'i') || (idx_info[1] != 'd') || (idx_info[2] != 'x') || (idx_info[3] != '1')){
		//AVI_PRINT(SYS_DBG, "avi index FOURCC error.\n");
		goto fail1;
	}
	
	size = RD_DWORD(&idx_info[4]);

	if(pAviReadInfo->dev==REC_DEV_USBH){
		fptr_offset = 512 - (pAviReadInfo->AviFile.fptr%512);
#if defined( CONFIG_SN_GCC_SDK )
		pAviReadInfo->OriginAviIndexAddr = (AVIIndexEntry_t*)pvPortMalloc((size + 512 + 512), GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviReadInfo->OriginAviIndexAddr = (uint8_t *)((AVIIndexEntry_t*)pvPortMalloc((size + 512 + 512)));
#endif		
		pAviReadInfo->pAviIndex =  (AVIIndexEntry_t*) ((((unsigned int) pAviReadInfo->OriginAviIndexAddr + 512) &(0xFFFFFE00)) + (512 - fptr_offset));
		//print_msg ("xxxxxxxxxxxxxxxxxxxx pAviIndex=%x\n",pAviReadInfo->pAviIndex);
		//print_msg ("xxxxxxxxxxxxxxxxxxxx pAviIndex ptr=%x , %x\n",pAviReadInfo->AviFile.fptr, fptr_offset);
	}
	else
	{
#if defined( CONFIG_SN_GCC_SDK )		
		pAviReadInfo->pAviIndex = (AVIIndexEntry_t*)pvPortMalloc(size, GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviReadInfo->pAviIndex = (AVIIndexEntry_t*)pvPortMalloc(size);
#endif		
	}


	if(pAviReadInfo->dev==REC_DEV_USBH){
		if(pAviReadInfo->OriginAviIndexAddr == NULL){
			AVI_DBG("allocate memory fail (index).\n");
			goto fail1;
		}
	}else{
		if(pAviReadInfo->pAviIndex == NULL){
			AVI_DBG("allocate memory fail (index).\n");
			goto fail1;
		}
	}

	ret = f_read(&pAviReadInfo->AviFile, pAviReadInfo->pAviIndex, size, &readsize);
	if((ret != FR_OK) || (size != readsize)){
		AVI_DBG("read file index fail(ret = %d, size = %d, %d).\n", ret, size, readsize);
		goto fail2;
	}

	pAviReadInfo->FrmIdxNum = size / sizeof(AVIIndexEntry_t);
	pAviReadInfo->IdxRdy = 1;

	return pdPASS;
	
fail2:
	if(pAviReadInfo->dev==REC_DEV_USBH){
		if(pAviReadInfo->OriginAviIndexAddr)
			vPortFree(pAviReadInfo->OriginAviIndexAddr);
		pAviReadInfo->OriginAviIndexAddr = NULL;
		
		pAviReadInfo->pAviIndex = NULL;
	}else{
		if(pAviReadInfo->pAviIndex)
			vPortFree(pAviReadInfo->pAviIndex);
		pAviReadInfo->pAviIndex = NULL;
	}

fail1:
	//no index, so seek to first frame data
	if(f_lseek(&pAviReadInfo->AviFile, sizeof(AVIFileHeader_t)) != FR_OK){
		AVI_DBG("seek to first frame data fail.\n");
	}

	return pdFAIL;
}

/**
* @brief interface function - read avi frame data.
* @param pAviReadInfo read avi file structure pointer.
* @param data return frame data pointer.
* @param size return frame data size.
* @param frm_type return frame type video or audio.
* @return return pdPASS if success.
*/
int avi_read_frame(AVI_read_Info_t *pAviReadInfo, void **data, uint32_t *size, uint8_t *frm_type)
{
	int ret;
	uint32_t readsize;
	uint8_t *pbuf;
	AVIIndexEntry_t *pIndex;
	
#if fast_seek
	if(pAviReadInfo->AviFile.cltbl == NULL){
#if defined( CONFIG_SN_GCC_SDK )		
		pAviReadInfo->clmt = (uint32_t *)pvPortMalloc(SZ_TBL, GFP_KERNEL, MODULE_MID_RECORD);
#else
		pAviReadInfo->clmt = (uint32_t *)pvPortMalloc(SZ_TBL);
#endif		
		if(pAviReadInfo->clmt == NULL){
			AVI_DBG("allocate memory fail (clmt).\n");
		}
		else{
			pAviReadInfo->AviFile.cltbl = (DWORD*)pAviReadInfo->clmt;			/* Enable fast seek feature (cltbl != NULL) */
			*pAviReadInfo->clmt = SZ_TBL;										/* Set table size */
			if(f_lseek(&pAviReadInfo->AviFile, CREATE_LINKMAP) != FR_OK){
				AVI_DBG("seek create CLMT fail.\n");
				pAviReadInfo->AviFile.cltbl = NULL;
				vPortFree(pAviReadInfo->clmt);
				pAviReadInfo->clmt=NULL;
			}
		}
	}
#endif

	if(pAviReadInfo->pFrmbuf == NULL){
#if defined( CONFIG_SN_GCC_SDK )
		pAviReadInfo->pFrmbuf = (unsigned char*)pvPortMalloc(pAviReadInfo->avi_fb_size, GFP_KERNEL|GFP_PREALLOCATE, MODULE_MID_RECORD);
#else
		pAviReadInfo->pFrmbuf = (unsigned char*)pvPortMalloc(pAviReadInfo->avi_fb_size);
#endif		
		if(pAviReadInfo->pFrmbuf == NULL){
			AVI_DBG("allocate avi frame buffer (%d) fail\n", pAviReadInfo->avi_fb_size);
			goto fail1;
		}
	}
	pbuf = pAviReadInfo->pFrmbuf;
	
	if(pAviReadInfo->IdxRdy){
		if(pAviReadInfo->FrmIdx >= pAviReadInfo->FrmIdxNum){
			AVI_DBG("end of index.\n");
			goto fail1;
		}
		
		pIndex = pAviReadInfo->pAviIndex + pAviReadInfo->FrmIdx;
/*
		if((avi_check_fourcc(pIndex->ulckid, "00dc") == pdPASS) || (avi_check_fourcc(pIndex->ulckid, "00db") == pdPASS)){
			*frm_type = AV_TYPE_VIDEO;
		}
		else if((avi_check_fourcc(pIndex->ulckid, "01wb") == pdPASS) || (avi_check_fourcc(pIndex->ulckid, "01wc") == pdPASS)){
			*frm_type = AV_TYPE_AUDIO;
		}
		else{
			goto fail1;
		}*/

		//AVI_PRINT(SYS_DBG, "=====file info:  size=%x, ptr=%x\n", pAviReadInfo->AviFile.fsize, pAviReadInfo->AviFile.fptr);
		
		readsize = pIndex->ulChunkLength + sizeof(CHUNK_t) + (pIndex->ulChunkLength&1);
		
		if(readsize > pAviReadInfo->avi_fb_size){
			AVI_DBG("ERROR: frame size(%d) over buffer(%d)!\n", readsize, pAviReadInfo->avi_fb_size);
			goto fail1;
		}
	
		ret = f_read(&pAviReadInfo->AviFile, pbuf, pIndex->ulChunkLength + sizeof(CHUNK_t) + (pIndex->ulChunkLength&1), &readsize);
		if((ret != FR_OK) || (pIndex->ulChunkLength + sizeof(CHUNK_t) + (pIndex->ulChunkLength&1) != readsize)){
			AVI_DBG("read frame fail(ret = %d, size = %d, %d).\n", ret, pIndex->ulChunkLength, readsize);
			goto fail1;
		}
		
		if(((*(pbuf+2) == 'd') && (*(pbuf+3) == 'c')) || ((*(pbuf+2) == 'd') && (*(pbuf+3) == 'b'))){
			*frm_type = AV_TYPE_VIDEO;
		}
		else if(((*(pbuf+2) == 'w') && (*(pbuf+3) == 'b')) || ((*(pbuf+2) == 'w') && (*(pbuf+3) == 'c'))){
			*frm_type = AV_TYPE_AUDIO;
		}
		else{
			AVI_DBG("check frame type fail!!!!\n");
			goto fail1;
		}

		*size = pIndex->ulChunkLength;		
		*data = (void*)(pbuf + sizeof(CHUNK_t));
		pAviReadInfo->FrmIdx++;
	}
	else{	//file no index

		ret = f_read(&pAviReadInfo->AviFile, pbuf, ATOM_HEAD_SIZE, &readsize);
		if((ret != FR_OK) || (ATOM_HEAD_SIZE != readsize)){
			AVI_DBG("read frame header fail(ret = %d, size = %d, %d).\n", ret, ATOM_HEAD_SIZE, readsize);
			goto fail1;
		}
		
		//if((*(pbuf) == '0') && (*(pbuf+1) == '0') && (*(pbuf+2) == 'd') && (*(pbuf+3) == 'c')){
		if(((*(pbuf+2) == 'd') && (*(pbuf+3) == 'c')) || ((*(pbuf+2) == 'd') && (*(pbuf+3) == 'b'))){
			*frm_type = AV_TYPE_VIDEO;
		}
		//else if((*(pbuf) == '0') && (*(pbuf+1) == '1') && (*(pbuf+2) == 'w') && (*(pbuf+3) == 'b')){
		else if(((*(pbuf+2) == 'w') && (*(pbuf+3) == 'b')) || ((*(pbuf+2) == 'w') && (*(pbuf+3) == 'c'))){
			*frm_type = AV_TYPE_AUDIO;
		}
		else{
			goto fail1;
		}
		
		*size = RD_DWORD(pbuf+4);

		if(*size + ((*size)&1) > pAviReadInfo->avi_fb_size){
			AVI_DBG("ERROR: frame size(%d) over buffer(%d)!\n", *size + ((*size)&1), pAviReadInfo->avi_fb_size);
			goto fail1;
		}
		ret = f_read(&pAviReadInfo->AviFile, pbuf, *size + ((*size)&1), &readsize);
		if((ret != FR_OK) || (*size + ((*size)&1) != readsize)){
			AVI_DBG("read frame fail(ret = %d, size = %d, %d).\n", ret, *size, readsize);
			goto fail1;
		}

		*data = pbuf;
	}
	
	return pdPASS;

fail1:
	*data = NULL;
	*size = 0;
	*frm_type = AV_TYPE_NONE;

	return pdFAIL;
}

/**
* @brief interface function - reset avi frame index.
* @param pAviReadInfo read avi file structure pointer.
* @return return pdPASS if success.
*/
int avi_reset_frmidx(AVI_read_Info_t *pAviReadInfo)
{
	if(pAviReadInfo->IdxRdy){
		//seek to first frame data
		if(f_lseek(&pAviReadInfo->AviFile, sizeof(AVIFileHeader_t)) == FR_OK){
			pAviReadInfo->FrmIdx = 0;
		
			return pdPASS;
		}

		AVI_DBG("seek to first frame data fail.\n");
	}

	return pdFAIL;
}

/**
* @brief interface function - reset avi frame buffer flag.
* @param pAviReadInfo read avi file structure pointer.
* @param buf buffer pointer.
* @return return pdPASS if success.
*/
int avi_reset_data_buf(AVI_read_Info_t *pAviReadInfo, void *buf)
{
	return pdPASS;
}

/**
* @brief interface function - avi seek frame.
* @param pAviReadInfo read avi file structure pointer.
* @param time seek to time (unit 1s).
* @return return pdPASS if success.
*/
int avi_seek_frame(AVI_read_Info_t *pAviReadInfo, uint32_t time)
{
	uint32_t VdoIdx, VdoFrmCnt = 0, TotalFrmCnt = 0;
	AVIIndexEntry_t *pIndex = pAviReadInfo->pAviIndex;

	VdoIdx = time * pAviReadInfo->pAviHeader->head.s1.strh.info.ulRate;

	if(pAviReadInfo->IdxRdy){
		if(VdoIdx > pAviReadInfo->pAviHeader->head.s1.strh.info.ulLength - 1){
			pAviReadInfo->FrmIdx = pAviReadInfo->FrmIdxNum;
			AVI_DBG("over index.\n");
			return pdFAIL;
		}

		while(VdoIdx != VdoFrmCnt){
			if(TotalFrmCnt >= pAviReadInfo->FrmIdxNum){
				pAviReadInfo->FrmIdx = pAviReadInfo->FrmIdxNum;
				AVI_DBG("over index.\n");
				return pdFAIL;
			}
			
			pIndex = pAviReadInfo->pAviIndex + (TotalFrmCnt++);

			if((avi_check_fourcc(pIndex->ulckid, "00dc") == pdPASS) || (avi_check_fourcc(pIndex->ulckid, "00db") == pdPASS)){
				VdoFrmCnt++;
			}
			
			if((avi_check_fourcc(pIndex->ulckid, "00dc") != pdPASS) && (avi_check_fourcc(pIndex->ulckid, "00db") != pdPASS) &&
			(avi_check_fourcc(pIndex->ulckid, "01wb") != pdPASS) && (avi_check_fourcc(pIndex->ulckid, "01wc") != pdPASS)){
				pAviReadInfo->FrmIdx = pAviReadInfo->FrmIdxNum;
				AVI_DBG("check  index ulckid fail.\n");
				return pdFAIL;
			}
		}

		if(f_lseek(&pAviReadInfo->AviFile, pIndex->ulChunkOffset + sizeof(AVIFileHeader_t) - sizeof(FOURCC)) != FR_OK){
			pAviReadInfo->FrmIdx = pAviReadInfo->FrmIdxNum;
			AVI_DBG("seek to frame data fail.\n");
			return pdFAIL;
		}
		
		if(TotalFrmCnt > 0){
			pAviReadInfo->FrmIdx = TotalFrmCnt - 1;
		}
		else{
			pAviReadInfo->FrmIdx = TotalFrmCnt;
		}

		return pdPASS;
	}
	else{	//file no index
		int ret;
		uint8_t tmp_buf[sizeof(CHUNK_t)];
		uint32_t readsize;
		uint32_t Offset = 0;
		uint32_t ChunkFourCC, DataSize;
		
		if(f_lseek(&pAviReadInfo->AviFile, sizeof(AVIFileHeader_t)) != FR_OK){
			AVI_DBG("seek to frame data fail.\n");
			return pdFAIL;
		}		
	
		while(VdoIdx != VdoFrmCnt){
			ret = f_read(&pAviReadInfo->AviFile, tmp_buf, sizeof(CHUNK_t), &readsize);
			if((ret != FR_OK) || (sizeof(CHUNK_t) != readsize)){
				AVI_DBG("read frame fail(ret = %d, size = %d, %d).\n", ret, sizeof(CHUNK_t), readsize);
				return pdFAIL;
			}
			
			ChunkFourCC = RD_DWORD(tmp_buf);
			DataSize = RD_DWORD(tmp_buf + 4);

			Offset += (DataSize + sizeof(CHUNK_t) + (DataSize&1));

			if((avi_check_fourcc(ChunkFourCC, "00dc") == pdPASS) || (avi_check_fourcc(ChunkFourCC, "00db") == pdPASS)){
				VdoFrmCnt++;
			}

			if((avi_check_fourcc(ChunkFourCC, "00dc") != pdPASS) && (avi_check_fourcc(ChunkFourCC, "00db") != pdPASS) &&
			(avi_check_fourcc(ChunkFourCC, "01wb") != pdPASS) && (avi_check_fourcc(ChunkFourCC, "01wc") != pdPASS)){
				AVI_DBG("check  index ulckid fail.\n");
				return pdFAIL;
			}

			if(f_lseek(&pAviReadInfo->AviFile, Offset + sizeof(AVIFileHeader_t)) != FR_OK){
				AVI_DBG("seek to frame data fail.\n");
				return pdFAIL;
			}
		}
		
		return pdPASS;
	}
}

int avi_check_fourcc(uint32_t Val, char *pPtr)
{
	int i;

	for(i=0; i<sizeof(FOURCC); i++){
		if(*(pPtr+i) != (uint8_t)((Val >> (i*8)) & 0x00FF)){
			return pdFAIL;
		}
	}
	
	return pdPASS;
}

char avi_get_wb_status(AVI_Info_t *pAviInfo)
{
	return writebuf_get_wb_status(&pAviInfo->WBInfo);
}

#endif 	//end of #if defined (CONFIG_RECORD)	
