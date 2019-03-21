// $Source: $
// *******************************************************************			  
//	   (C) Copyright 2015 by SONiX Technology Corp.
//
//			   All Rights Reserved
//
// This	program/document/source	code is	an unpublished copyrighted   
// work	which is proprietary to	SONiX Technology Corp.,	and contains 
// confidential	information that is not	to be reproduced or disclosed
// to any other	person or entity without prior written consent from
// SONiX Technology Corp. in each and every instance.
//
// *******************************************************************
// Author(s): Hammer Huang
// 
// Description:	
//
// $Date: 
// 
// $Log: $
// *******************************************************************
// SN98660A_Free_RTOS
// SVN $Rev: 
// SVN $Date:
// ------------------------------------------------------------------
/**
 * @file
 * this	is  UVC	file
 * USBH_UVC.c
 * @author SW Dept Sonix. (Hammer Huang #1359)
 */
#include "sonix_config.h"

#if defined(CONFIG_USBH_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#endif

#if defined( CONFIG_SN_GCC_SDK )
#include <generated/snx_sdk_conf.h>
#include <bsp.h>
#include <nonstdlib.h>
#endif 
	
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
	
#if defined( CONFIG_SN_KEIL_SDK )
#include <stdlib.h>
#if defined( CONFIG_PLATFORM_ST53510 )
#if defined (CONFIG_USBH_CLI_UVC)
#include "cmd_uvc.h"
#endif
#if !defined (CONFIG_USBH_CLI_UVC) && defined (CONFIG_NONCLI_APP)
#include "usbh_test.h"
#endif	
#endif 

#if defined( CONFIG_PLATFORM_SN7320 )
//#if defined (CONFIG_USBH_CLI_UVC)
//#include "cli_usbh_uvc.h"
//#endif
#if !defined (CONFIG_USBH_CLI_UVC) && defined (CONFIG_NONCLI_APP)
#include "usbh_test.h"
#endif	
#endif

#endif 

#if defined( CONFIG_XILINX_SDK )
#include <stdlib.h>
#include <stdio.h>
#include <xil_cache.h>
#endif

#include <string.h>
#include "USBH_UVC.h"
#include "USBH_CORE.h"
#include "USBH.h"
#include "EHCI_HCD.h"

USBH_UVC_APP_STRUCTURE usbh_uvc_app;

#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
uint32_t test_frame_count = 0;
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
//SemaphoreHandle_t		USBH_SEM_WAKEUP_UVC_DRV;

QueueHandle_t			usbh_queue_uvc_drv = NULL;
QueueHandle_t			usbh_queue_uvc_mw = NULL;
QueueHandle_t			usbh_queue_uvc_app = NULL;
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
TaskHandle_t			xTASK_HDL_UVC_BULK_DATA		= NULL;
QueueHandle_t			usbh_queue_uvc_bulk_data	= NULL;

SemaphoreHandle_t		bk_ping_mutex = NULL;
SemaphoreHandle_t		bk_pong_mutex = NULL;
uint8_t					ping_empty = 1;
uint8_t					pong_empty = 1;

SemaphoreHandle_t		bk_wait_compose_sem		= NULL;
SemaphoreHandle_t		bk_wait_compose_mutex	= NULL;
uint8_t					bk_wait_compose_count	= 0;

uint8_t	clean_bk_buf_idx = 0;

SemaphoreHandle_t		bk_packet_err_mutex	= NULL;
uint8_t							bk_packet_err = 0;
uint8_t							bk_discarded_frame_cnt = 0;
#endif
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
QueueHandle_t			usbh_queue_uvc_xfr_ctrl	= NULL;
TaskHandle_t			xTASK_HDL_UVC_XFR_CTRL		= NULL;
#endif

TaskHandle_t			xTASK_HDL_UVC_DRV[10] = {NULL}; // USBH_MAX_PORT
TaskHandle_t			xTASK_HDL_UVC_APP = NULL;  
TaskHandle_t			xTASK_HDL_UVC_STREAM_XFR = NULL;

#endif
#if defined (CONFIG_USBH_CMSIS_OS)
//osSemaphoreId		USBH_SEM_WAKEUP_UVC_DRV;

osMessageQId			usbh_queue_uvc_drv = NULL;
osMessageQId			usbh_queue_uvc_mw = NULL;
osMessageQId			usbh_queue_uvc_app = NULL;
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
osMessageQId			usbh_queue_uvc_bulk_data	= NULL;
#endif
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
osMessageQId			usbh_queue_uvc_xfr_ctrl		= NULL;
osThreadId				xTASK_HDL_UVC_XFR_CTRL		= NULL;
#endif

osThreadId				xTASK_HDL_UVC_DRV[USBH_MAX_PORT*2] = {NULL};
osThreadId				xTASK_HDL_UVC_APP = NULL;  
osThreadId				xTASK_HDL_UVC_STREAM_XFR	= NULL;
osThreadId				xTASK_HDL_UVC_BULK_DATA		= NULL;

osMutexId		bk_ping_mutex = NULL;
osMutexId		bk_pong_mutex = NULL;
uint8_t			ping_empty = 1;
uint8_t			pong_empty = 1;

osSemaphoreId		bk_wait_compose_sem		= NULL;
osMutexId				bk_wait_compose_mutex	= NULL;
uint8_t					bk_wait_compose_count	= 0;

uint8_t	clean_bk_buf_idx = 0;
#endif

//===========================Global==============================//
volatile USBH_UVC_INFO_Struct 	UVC_INFO_Struct;
USBH_ISO_XFR_REQ_Struct         UVC_ISO_REQ[UVC_STREAMID_NUM_MAX];
USBH_BK_XFR_REQ_Struct          UVC_BULK_REQ[UVC_STREAMID_NUM_MAX];
USBH_UVC_DATA_QUEUE_Structure   UVC_DATA;

//uint8_t                         gStream_ID[USBH_MAX_PORT][UVC_STREAMID_NUM_MAX] = {0};
uint8_t                         gStream_ID[UVC_STREAMID_NUM_MAX] = {0};


uint8_t uvc_get_devIDX_in_UVCINFOStruct(USBH_Device_Structure *UVC_DEV)
{
	uint8_t i = 0, dev_idx = 0;
	
	for(i = 0; i < USBH_MAX_PORT*2; i++) {
		if(UVC_INFO_Struct.DEV[i].iDevID == UVC_DEV->device_id) {
			dev_idx = i;
			return dev_idx;
		}
	}
	
	return (USBH_MAX_PORT * 2);
}

void uvc_parser_vc_intfdes(USBH_Device_Structure *UVC_DEV, uint8_t *pbData, uint8_t bDescriptorSubType)
{
	USBH_UVC_CLASS_Structure				*UVC_CLASS = NULL;
	USBH_UVC_DESCRIPTOR_Structure		*UVC_DES = NULL;

	uint8_t	i = 0, j = 0;
	uint8_t	bLength	= *(pbData);
	uint8_t	bControlSize = 0;
	uint8_t	bInCollection =	0;
	uint8_t	*baInterfaceNr = NULL;
	uint8_t	*bmControls = NULL;
	uint16_t	wTerminalType =	0;

	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES = (USBH_UVC_DESCRIPTOR_Structure*)(&UVC_CLASS->DES);
	switch(bDescriptorSubType){
	case VC_DESCRIPTOR_UNDEFINED:
		break;
	case VC_HEADER:
		bInCollection = *(pbData + 11);
		memcpy(&(UVC_DES->VC_Interface), pbData, bLength-bInCollection);
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )		
		do{
			baInterfaceNr =	(uint8_t*)pvPortMalloc((sizeof(uint8_t)*bInCollection), GFP_KERNEL, MODULE_DRI_USBH);
		}while(baInterfaceNr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK ) 
		do{
			baInterfaceNr = (uint8_t*)pvPortMalloc((sizeof(uint8_t)*bInCollection));
		}while(baInterfaceNr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) 
		baInterfaceNr = (uint8_t *)malloc((sizeof(uint8_t)*bInCollection));
#endif
#endif

		UVC_DES->VC_Interface.baInterfaceNr	= baInterfaceNr;
	
		for(i = 0; i < bInCollection; i++){
			UVC_DES->VC_Interface.baInterfaceNr[i] = *(pbData+12+i);
		}		
		break;
	case VC_INPUT_TERMINAL:
		wTerminalType = (uint16_t)(	(*(pbData + 4))	| ((*(pbData + 5)) << 8));
		switch(wTerminalType){
		case ITT_VENDOR_SPECIFIC:
			//UVC_DBG("ITT_VENDOR_SPECIFIC\r\n");
			break;
		case ITT_CAMERA:
			//UVC_DBG("ITT_CAMERA\r\n");
			memcpy(&(UVC_DES->VC_Camera_IT_Des), pbData, bLength);			
			break;
		case ITT_MEDIA_TRANSPORT_INPUT:
			//UVC_DBG("ITT_MEDIA_TRANSPORT_INPUT\r\n");
			memcpy(&(UVC_DES->VC_Media_IT_Des),	pbData,	bLength);			
			break;
		}//	end of switch(TerminalType)
		break;		
	case VC_OUTPUT_TERMINAL:	
		//UVC_DBG("VC_OUTPUT_TERMINAL\r\n");
		for(i = 0; i < MAX_DES_NUM;	i++){
			if(UVC_DES->VC_OT_Des[i].bLength == 0x00){			
				memcpy(&(UVC_DES->VC_OT_Des[i]), pbData, bLength);
				break;
			}
		}	
		break;
	case VC_SELECTOR_UNIT:
		//UVC_DBG("VC_SELECTOR_UNIT\r\n");
		memcpy(&(UVC_DES->VC_SU_Des), pbData, bLength);		
		break;
	case VC_PROCESSING_UNIT:
		//UVC_DBG("VC_PROCESSING_UNIT\r\n");
		memcpy(&(UVC_DES->VC_PU_Des), pbData, 8);

		bControlSize = *(pbData+7);
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )		
		do{
			bmControls = (uint8_t*)pvPortMalloc((sizeof(uint8_t)*bControlSize), GFP_KERNEL,	MODULE_DRI_USBH);
		}while(bmControls == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			bmControls = (uint8_t*)pvPortMalloc((sizeof(uint8_t)*bControlSize));
		}while(bmControls == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
		bmControls = (uint8_t *)malloc((sizeof(uint8_t)*bControlSize));
#endif
#endif

		UVC_DES->VC_PU_Des.bmControls = bmControls;
		for(i = 0; i < bControlSize; i++){
			UVC_DES->VC_PU_Des.bmControls[i] = *(pbData+8+i);
		}
		UVC_DES->VC_PU_Des.iProcessing = *(pbData+8+i);
		break;
	case VC_EXTENSION_UNIT:
		//UVC_DBG("VC_EXTENSION_UNIT\r\n");
		for(i = 0; i < MAX_DES_NUM;	i++){
			if(UVC_DES->VC_XU_Des[i].bLength == 0x00){
				uint8_t bControlSize = 0;
				bControlSize = *(pbData+23);
				memcpy(&(UVC_DES->VC_XU_Des[i]), pbData, 24);
				
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )				
				do{
					bmControls = (uint8_t*)pvPortMalloc((sizeof(uint8_t)*bControlSize), GFP_KERNEL,	MODULE_DRI_USBH);
				}while(bmControls == NULL);
#endif				
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do{
					bmControls = (uint8_t*)pvPortMalloc((sizeof(uint8_t)*bControlSize));
				}while(bmControls == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)			
#if defined( CONFIG_SN_KEIL_SDK )
				bmControls = (uint8_t *)malloc((sizeof(uint8_t)*bControlSize));
#endif
#endif

				UVC_DES->VC_XU_Des[i].bmControls = bmControls;
				for(j = 0; j < bControlSize; j++){
					UVC_DES->VC_XU_Des[i].bmControls[j] = *(pbData+24+j);
				}
				UVC_DES->VC_XU_Des[i].iExtension = *(pbData+24+j);
				break;
			}// end	of if
		}//	end of for(...)
		break;  
	}
}

 void uvc_parser_vs_intfdes(USBH_Device_Structure *UVC_DEV, uint8_t *pbData, uint8_t bDescriptorSubType, uint8_t *previous_pbData, uint8_t dev_idx)
 {
	USBH_UVC_CLASS_Structure			*UVC_CLASS = NULL;
	USBH_UVC_DESCRIPTOR_Structure			*UVC_DES = NULL;
	USBH_UVC_CS_Format_Descriptor_Struct		*VS_FORMAT_Des = NULL;
	USBH_UVC_CS_Frame_Descriptor_Struct		*VS_FRAME_Des = NULL;
	USBH_UVC_Frame_Based_Format_Descriptor_Struct	*FB_FORMAT_Des = NULL;
	USBH_UVC_Frame_Based_Frame_Descriptor_Struct	*FB_FRAME_Des = NULL;
	USBH_UVC_YUV_Format_Descriptor_Struct		*YUV_FORMAT_Des = NULL;
	USBH_UVC_YUV_Frame_Descriptor_Struct		*YUV_FRAME_Des = NULL;

	uint8_t i =	0, j = 0, h = 0;
	uint8_t indexFmt = 0;
	uint8_t bDevId = 0;
	uint8_t bLength = *(pbData);
	uint8_t bNumFormats	= 0;
	uint8_t *bmaControls = NULL;
	uint16_t wWidth = 0, wHeight = 0;	
	uint32_t *dwFrameInterval =	NULL;
	uint8_t 	bNumImageSizePatterns = 0;

	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES		=	(USBH_UVC_DESCRIPTOR_Structure*)(&UVC_CLASS->DES);
	//bDevId 		= 	UVC_DEV->device_id;
	bDevId = dev_idx;
	switch(bDescriptorSubType){
		case VS_UNDEFINED:
			break;
		case VS_INPUT_HEADER:
			//UVC_DBG("VS_INPUT_HEADER\r\n");		
			for(i = 0;i	< UVC_INTERFACE_NUM_MAX; i++){
				if(UVC_DES->VS_Interface[i].bLength == 0x00){
					//UVC_CLASS->CurIntfNum ++;
					memcpy(&(UVC_DES->VS_Interface[i]),	pbData,	13);
					bNumFormats	= *(pbData+3);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum].bFmtNum = bNumFormats;

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )					
					do{
						bmaControls = (uint8_t*)pvPortMalloc(sizeof(uint8_t) * bNumFormats, GFP_KERNEL, MODULE_DRI_USBH);
					}while(bmaControls == NULL);
#endif					
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
					do{
						bmaControls = (uint8_t*)pvPortMalloc(sizeof(uint8_t)*bNumFormats);
					}while(bmaControls == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)					
#if defined( CONFIG_SN_KEIL_SDK )
					bmaControls = (uint8_t *)malloc(sizeof(uint8_t)*bNumFormats);
#endif
#endif
					UVC_DES->VS_Interface[i].bmaControls = bmaControls;
					for(j = 0; j < bNumFormats;	j++){
						UVC_DES->VS_Interface[i].bmaControls[j]	= *(pbData+13+j);
					}
					UVC_CLASS->CurIntfNum ++;
					break;
				}// end	of if			
			}//	end of for(...)		
			break;		
		case VS_OUTPUT_HEADER:
			break;
		case VS_FORMAT_UNCOMPRESSED:
			memcpy(&(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].YUV_FORMAT_Des), pbData, bLength);
	
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
				if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == 0){
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType = USBH_UVC_STREAM_YUV;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFormatIndex = \
										UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].YUV_FORMAT_Des.bFormatIndex;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bDefFrameIndex = \
										UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].YUV_FORMAT_Des.bDefaultFrameIndex;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bResNum = \
										UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].YUV_FORMAT_Des.bNumFrameDescriptors;
					break;
				}		
			}
			break;
		case VS_FRAME_UNCOMPRESSED:
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
				if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == USBH_UVC_STREAM_YUV){
					indexFmt = i;
					break;
				}
			}
			YUV_FORMAT_Des = &(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].YUV_FORMAT_Des);
			for(i = 0; i < YUV_FORMAT_Des->bNumFrameDescriptors; i++){
				YUV_FRAME_Des =	&(YUV_FORMAT_Des->YUV_FRAME_Des[i]);
				if(YUV_FRAME_Des->bLength == 0x00){
					memcpy(&(YUV_FORMAT_Des->YUV_FRAME_Des[i]),	pbData,	26);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bFpsNum = YUV_FORMAT_Des->YUV_FRAME_Des[i].bFrameIntervalType;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bFrameIndex = YUV_FORMAT_Des->YUV_FRAME_Des[i].bFrameIndex;
					wWidth = (uint16_t)(YUV_FORMAT_Des->YUV_FRAME_Des[i].bWidthLowByte \
						| YUV_FORMAT_Des->YUV_FRAME_Des[i].bWidthHighByte<<8);
					wHeight = (uint16_t)(YUV_FORMAT_Des->YUV_FRAME_Des[i].bHeightLowByte \
						| YUV_FORMAT_Des->YUV_FRAME_Des[i].bHeightHighByte<<8);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].wWidth = wWidth;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].wHeight = wHeight;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bPixel = uvc_set_res_val(wWidth, wHeight);
					
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )					
					do{
						dwFrameInterval	= (uint32_t *)pvPortMalloc(sizeof(uint32_t)* YUV_FRAME_Des->bFrameIntervalType, GFP_KERNEL, MODULE_DRI_USBH);
					}while(dwFrameInterval == NULL);
#endif					
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
					do{
						dwFrameInterval = (uint32_t *)pvPortMalloc(sizeof(uint32_t)* YUV_FRAME_Des->bFrameIntervalType);
					}while(dwFrameInterval == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)			
#if defined( CONFIG_SN_KEIL_SDK )
					dwFrameInterval = (uint32_t *)malloc(sizeof(uint32_t)* YUV_FRAME_Des->bFrameIntervalType);
#endif
#endif
					YUV_FRAME_Des->dwFrameInterval = dwFrameInterval;
					for(j = 0, h = 0; j	< YUV_FRAME_Des->bFrameIntervalType; j++){			
						YUV_FRAME_Des->dwFrameInterval[j] = (uint32_t)(	*(pbData+26+h) | ((*(pbData+27+h))<<8) \
							| ((*(pbData+28+h))<<16) | ((*(pbData+29+h))<<24));
						UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].FPS[j].dwFrameInterval = YUV_FRAME_Des->dwFrameInterval[j];
						UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].FPS[j].dwStreamID = 0;			
						h += 4;
					}
					break;
				}
			}//	end of for(...)	
			break;
		case VS_FORMAT_MJPEG:
			memcpy(&(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].VS_FORMAT_Des), pbData, bLength);
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
				if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == 0){
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType = USBH_UVC_STREAM_MJPEG;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFormatIndex = \
									UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].VS_FORMAT_Des.bFormatIndex;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bDefFrameIndex = \
									UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].VS_FORMAT_Des.bDefaultFrameIndex;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bResNum = \
									UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].VS_FORMAT_Des.bNumFrameDescriptors;
					break;
				}
			}
			break;
		case VS_FRAME_MJPEG:		
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
				if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == USBH_UVC_STREAM_MJPEG){
					indexFmt = i;
					break;
				}
			}
			VS_FORMAT_Des = &(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].VS_FORMAT_Des);
			for(i = 0; i < VS_FORMAT_Des->bNumFrameDescriptors;	i++){
				VS_FRAME_Des = &(VS_FORMAT_Des->VS_FRAME_Des[i]);
				if(VS_FRAME_Des->bLength == 0x00){
					memcpy(&(VS_FORMAT_Des->VS_FRAME_Des[i]), pbData, 26);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bFpsNum = VS_FORMAT_Des->VS_FRAME_Des[i].bFrameIntervalType;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bFrameIndex = VS_FORMAT_Des->VS_FRAME_Des[i].bFrameIndex;
					wWidth = (uint16_t)(VS_FORMAT_Des->VS_FRAME_Des[i].bWidthLowByte \
						| VS_FORMAT_Des->VS_FRAME_Des[i].bWidthHighByte<<8);
					wHeight = (uint16_t)(VS_FORMAT_Des->VS_FRAME_Des[i].bHeightLowByte \
						| VS_FORMAT_Des->VS_FRAME_Des[i].bHeightHighByte<<8);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].wWidth = wWidth;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].wHeight = wHeight;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bPixel = uvc_set_res_val(wWidth, wHeight);
					
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )					
					do{
						dwFrameInterval	= (uint32_t *)pvPortMalloc(sizeof(uint32_t)* VS_FRAME_Des->bFrameIntervalType, GFP_KERNEL, MODULE_DRI_USBH);
					}while(dwFrameInterval == NULL);
#endif					
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
					do{
						dwFrameInterval = (uint32_t *)pvPortMalloc(sizeof(uint32_t)* VS_FRAME_Des->bFrameIntervalType);
					}while(dwFrameInterval == NULL);
#endif
#endif	
#if defined (CONFIG_USBH_CMSIS_OS)			
#if defined( CONFIG_SN_KEIL_SDK )
					dwFrameInterval = (uint32_t *)malloc(sizeof(uint32_t)* VS_FRAME_Des->bFrameIntervalType);
#endif
#endif
					VS_FRAME_Des->dwFrameInterval = dwFrameInterval;
					for(j = 0, h = 0; j	< VS_FRAME_Des->bFrameIntervalType; j++){			
						VS_FRAME_Des->dwFrameInterval[j] = (uint32_t)( *(pbData+26+h) |	((*(pbData+27+h))<<8)\
							| ((*(pbData+28+h))<<16) | ((*(pbData+29+h))<<24));			
						UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].FPS[j].dwFrameInterval = VS_FRAME_Des->dwFrameInterval[j];
						UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].FPS[j].dwStreamID = 0;
						h += 4;
					}
					break;
				}//end of if
			}//	end of for(...)
			break;
		case VS_FORMAT_MPEG2TS:
			//UVC_DBG("VS_FORMAT_MPEG2TS\r\n");
			break;
		case VS_FORMAT_DV:
			//UVC_DBG("VS_FORMAT_DV\r\n");
			break;
		case VS_COLORFORMAT:
			//UVC_DBG("VS_COLORFORMAT\r\n");
			memcpy(&(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].CLRMCH_FORMAT_Des),	pbData,	bLength);		
			break;		
		case VS_FORMAT_FRAME_BASED:
			//UVC_DBG("VS_FORMAT_FRAME_BASED\r\n");
			memcpy(&(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].FB_FORMAT_Des), pbData, bLength);
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
				if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == 0){
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType = USBH_UVC_STREAM_H264;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFormatIndex = \
									UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].FB_FORMAT_Des.bFormatIndex;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bDefFrameIndex = \
									UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].FB_FORMAT_Des.bDefaultFrameIndex;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bResNum = \
									UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].FB_FORMAT_Des.bNumFrameDescriptors;		
					break;
				}		
			}
			break;
		case VS_FRAME_FRAME_BASED:	
			//UVC_DBG("VS_FRAME_FRAME_BASED\r\n");
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
				if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == USBH_UVC_STREAM_H264){
					indexFmt = i;
					break;
				}
			}
			FB_FORMAT_Des = &(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].FB_FORMAT_Des);
			for(i = 0; i < FB_FORMAT_Des->bNumFrameDescriptors;	i++){
				FB_FRAME_Des = &(FB_FORMAT_Des->FB_FRAME_Des[i]);
				if(FB_FRAME_Des->bLength == 0x00){
					memcpy(&(FB_FORMAT_Des->FB_FRAME_Des[i]), pbData, 26);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bFpsNum = FB_FORMAT_Des->FB_FRAME_Des[i].bFrameIntervalType;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bFrameIndex = FB_FORMAT_Des->FB_FRAME_Des[i].bFrameIndex;
					wWidth = (uint16_t)(FB_FORMAT_Des->FB_FRAME_Des[i].bWidthLowByte \
						| FB_FORMAT_Des->FB_FRAME_Des[i].bWidthHighByte<<8);
					wHeight = (uint16_t)(FB_FORMAT_Des->FB_FRAME_Des[i].bHeightLowByte \
						| FB_FORMAT_Des->FB_FRAME_Des[i].bHeightHighByte<<8);
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].wWidth = wWidth;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].wHeight = wHeight;
					UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].bPixel = uvc_set_res_val(wWidth, wHeight);
					
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )					
					do{
						dwFrameInterval	= (uint32_t *)pvPortMalloc(sizeof(uint32_t)* FB_FRAME_Des->bFrameIntervalType, GFP_KERNEL, MODULE_DRI_USBH);
					}while(dwFrameInterval == NULL);
#endif					
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
					do{
						dwFrameInterval = (uint32_t *)pvPortMalloc(sizeof(uint32_t)* FB_FRAME_Des->bFrameIntervalType);
					}while(dwFrameInterval == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)				
#if defined( CONFIG_SN_KEIL_SDK )
					dwFrameInterval = (uint32_t *)malloc(sizeof(uint32_t)* FB_FRAME_Des->bFrameIntervalType);
#endif
#endif					
					FB_FRAME_Des->dwFrameInterval = dwFrameInterval;
					for(j = 0, h = 0; j	< FB_FRAME_Des->bFrameIntervalType; j++){
						FB_FRAME_Des->dwFrameInterval[j] = (uint32_t)( *(pbData+26+h) |	((*(pbData+27+h))<<8) \
							   | ((*(pbData+28+h))<<16) | ((*(pbData+29+h))<<24));
						UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].FPS[j].dwFrameInterval = FB_FRAME_Des->dwFrameInterval[j];
						UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[indexFmt].RES[i].FPS[j].dwStreamID = 0;
						h += 4;
					}
					break;
				}// end	of if
			}//	end of for(...)
			break;
		case VS_FORMAT_STREAM_BASED:		
			break;
		case VS_STILL_IMAGE_FRAME:
			bNumImageSizePatterns = *(pbData+4);

			if(*(previous_pbData+1) == CS_INTERFACE) {
				switch(*(previous_pbData+2)) {
				case VS_FRAME_UNCOMPRESSED:
					for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
						if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == USBH_UVC_STREAM_YUV){
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns, GFP_DMA, MODULE_DRI_USBH);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns, GFP_DMA, MODULE_DRI_USBH);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns);
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)				
#if defined( CONFIG_SN_KEIL_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)malloc(sizeof(uint16_t) * bNumImageSizePatterns);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)malloc(sizeof(uint16_t) * bNumImageSizePatterns);
#endif
#endif
							memcpy((USBH_UVC_Still_Image_Frame_Descriptor_Struct*)&(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des), pbData, 5);
							for(j = 0; j < bNumImageSizePatterns; j++) {
								UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth[j]
									= pbData[5+(j*4)] | (pbData[6+(j*4)] << 8);
								UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight[j]
									= pbData[7+(j*4)] | (pbData[8+(j*4)] << 8);
							}
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.bNumImageSizePatterns = bNumImageSizePatterns;

							break;
						}
					}
					break;
				case VS_FRAME_MJPEG:
					for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
						if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == USBH_UVC_STREAM_MJPEG){
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns, GFP_DMA, MODULE_DRI_USBH);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns, GFP_DMA, MODULE_DRI_USBH);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns);
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)malloc(sizeof(uint16_t) * bNumImageSizePatterns);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)malloc(sizeof(uint16_t) * bNumImageSizePatterns);
#endif
#endif	
							memcpy((USBH_UVC_Still_Image_Frame_Descriptor_Struct*)&(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des), pbData, 5);
							for(j = 0; j < bNumImageSizePatterns; j++) {
								UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth[j]
									= pbData[5+(j*4)] | (pbData[6+(j*4)] << 8);
								UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight[j]
									= pbData[7+(j*4)] | (pbData[8+(j*4)] << 8);
							}
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.bNumImageSizePatterns = bNumImageSizePatterns;

							break;
						}
					}
					break;
				case VS_FRAME_FRAME_BASED:
					for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
						if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].bFmtType == USBH_UVC_STREAM_H264){
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns, GFP_DMA, MODULE_DRI_USBH);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns, GFP_DMA, MODULE_DRI_USBH);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)pvPortMalloc(sizeof(uint16_t) * bNumImageSizePatterns);
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)				
#if defined( CONFIG_SN_KEIL_SDK )
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth
								= (uint16_t*)malloc(sizeof(uint16_t) * bNumImageSizePatterns);
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight
								= (uint16_t*)malloc(sizeof(uint16_t) * bNumImageSizePatterns);
#endif
#endif
							memcpy((USBH_UVC_Still_Image_Frame_Descriptor_Struct*)&(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des), pbData, 5);
							for(j = 0; j < bNumImageSizePatterns; j++) {
								UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth[j]
									= pbData[5+(j*4)] | (pbData[6+(j*4)] << 8);
								UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight[j]
									= pbData[7+(j*4)] | (pbData[8+(j*4)] << 8);
							}
							UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.bNumImageSizePatterns = bNumImageSizePatterns;

							break;
						}
					}
					break;
				} // end of switch
			} // end of if
			break;
	}
 }

uint8_t	uac_parser_ac_intfdes(USBH_Device_Structure *UVC_DEV, uint8_t *pbData, uint8_t bDescriptorSubType)
{
	USBH_UVC_CLASS_Structure				*UVC_CLASS = NULL;
	USBH_UVC_DESCRIPTOR_Structure		*UVC_DES = NULL;

	uint8_t	i = 0;
	uint8_t	bLength	= *(pbData);
	uint8_t	bControlSize = 0;
	//uint8_t		*bmaControls = NULL;

	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES		=	(USBH_UVC_DESCRIPTOR_Structure*)(&UVC_CLASS->DES);

	switch(bDescriptorSubType){
		case AC_DESCRIPTOR_UNDEFINED:
			break;
		case AC_HEADER:
			//UVC_DBG("AC_HEADER\r\n");
			memcpy(&(UVC_DES->AC_Interface), pbData, bLength);
			break;
		case AC_INPUT_TERMINAL:
			//UVC_DBG("AC_INPUT_TERMINAL\r\n");
			for(i=0;i<MAX_DES_NUM;i++){
			if(UVC_DES->AC_ITD[i].bLength == 0 || UVC_DES->AC_ITD[i].bLength != 0x0c)
				break;
			}
			memcpy(&(UVC_DES->AC_ITD[i]), pbData, bLength);
			break;
		case AC_OUTPUT_TERMINAL:	
			//UVC_DBG("AC_OUTPUT_TERMINAL\r\n");
			for(i=0;i<MAX_DES_NUM;i++){
			if(UVC_DES->AC_OTD[i].bLength == 0)
				break;
			}
			memcpy(&(UVC_DES->AC_OTD[i]), pbData, bLength);
			break;
		case AC_MIXER_UNIT:
			break;
		case AC_SELECTOR_UNIT:
			break;
		case AC_FEATURE_UNIT:
			memcpy(&(UVC_DES->AC_FUD), pbData, 5);
			bControlSize = *(pbData+5);
			for(i = 0; i < bControlSize; i++){
				UVC_DES->AC_FUD.bmaControls[i][0] = *(pbData+5+i*2);
				UVC_DES->AC_FUD.bmaControls[i][1] = *(pbData+6+i*2);
			}
			UVC_DES->AC_FUD.iFeature = *(pbData+5+(i-1)*2);
			break;
		case AC_PROCESSING_UNIT:
			break;
		case AC_EXTENSION_UNIT:
			break;
	}
	return SUCCESS;
}

uint8_t	uac_parser_as_intfdes(USBH_Device_Structure *UVC_DEV, uint8_t *pbData, uint8_t bDescriptorSubType)
{
	USBH_UVC_CLASS_Structure			*UVC_CLASS = NULL;
	USBH_UVC_DESCRIPTOR_Structure	*UVC_DES = NULL;

	uint8_t i	=	0;
	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES		=	(USBH_UVC_DESCRIPTOR_Structure*)(&UVC_CLASS->DES);

	switch(bDescriptorSubType){
		case AS_DESCRIPTOR_UNDEFINED:		
			break;
		case AS_GENERAL:
			for(i = 0;i	< UVC_INTERFACE_NUM_MAX; i++){
				if(UVC_DES->AS_Interface[i].bLength == 0){
					memcpy(&(UVC_DES->AS_Interface[i]),	pbData,	5);
					UVC_CLASS->UAC_IntfIndex++;
					break;
				}
			}
			break;
		case AS_FORMAT_TYPE:		
			memcpy(&(UVC_DES->AS_Interface[UVC_CLASS->UAC_IntfIndex-1].AS_TFMD), pbData, 8);			
			for(i = 0; i < UVC_DES->AS_Interface[UVC_CLASS->UAC_IntfIndex-1].AS_TFMD.bSamFreqType; i++){
				UVC_DES->AS_Interface[UVC_CLASS->UAC_IntfIndex-1].AS_TFMD.tSamFreq[i][0] = *(pbData+8+i*3);
				UVC_DES->AS_Interface[UVC_CLASS->UAC_IntfIndex-1].AS_TFMD.tSamFreq[i][1] = *(pbData+9+i*3);
				UVC_DES->AS_Interface[UVC_CLASS->UAC_IntfIndex-1].AS_TFMD.tSamFreq[i][2] = *(pbData+10+i*3);
			}	
			break;		
		case AS_FORMAT_SPECIFIC:
			break;
	}
	return SUCCESS; 
}

void uvc_parser_intfdes(USBH_Device_Structure *UVC_DEV,	uint8_t	*pbData, uint8_t bFunctionClass, uint8_t bInterfaceSubClass, uint8_t bDescriptorSubType, uint8_t *previous_pbData, uint8_t dev_idx)
{
	switch(bFunctionClass){	
		case USBH_AUDIO_CLASS_CODE:
			//UAC
			switch(bInterfaceSubClass){
				case SC_AUDIOCONTROL:
					//UVC_DBG("SC_AUDIOCONTROL\r\n");
					uac_parser_ac_intfdes(UVC_DEV, pbData, bDescriptorSubType);
					break;
				case SC_AUDIOSTREAMING:
					//UVC_DBG("SC_AUDIOSTREAMING\r\n");
					uac_parser_as_intfdes(UVC_DEV, pbData, bDescriptorSubType);
					break;
				case SC_MIDISTREAMING:
					break;
			}
			break;
		case USBH_VIDEO_CLASS_CODE:
			//UVC
			switch(bInterfaceSubClass){
				case SC_VIDEOCONTROL:
					//UVC_DBG("SC_VIDEOCONTROL\r\n");
					uvc_parser_vc_intfdes(UVC_DEV, pbData, bDescriptorSubType);
					break;
				case SC_VIDEOSTREAMING:
					//UVC_DBG("SC_VIDEOSTREAMING\r\n");
					uvc_parser_vs_intfdes(UVC_DEV, pbData, bDescriptorSubType, previous_pbData, dev_idx);
					break;
			}
		break;
	}
}

void uvc_parser_endpdes(USBH_Device_Structure *UVC_DEV,	uint8_t	*pbData, uint8_t bDescriptorSubType)
{
	USBH_UVC_CLASS_Structure			*UVC_CLASS = NULL;
	USBH_UVC_DESCRIPTOR_Structure	*UVC_DES = NULL;

	uint8_t i =	0;
	uint8_t bLength = *(pbData);

	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES = (USBH_UVC_DESCRIPTOR_Structure*)(&UVC_CLASS->DES);
	
	switch(bDescriptorSubType){
	case EP_UNDEFINED:
		break;
	case EP_GENERAL:
		break;
	case EP_ENDPOINT:
		break;
	case EP_INTERRUPT:
		for(i = 0; i < UVC_ENDPOINT_NUM_MAX; i++){
			if(UVC_DES->VC_Interface.CS_ENDPDes[i].bLength == 0x00){
				memcpy(&(UVC_DES->VC_Interface.CS_ENDPDes[i]), pbData, bLength);
				break;
			}
		}
		break;	
	}
}

void uvc_parser_des(USBH_Device_Structure *UVC_DEV, uint8_t dev_idx)
{
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	uint8_t *pbData = &(UVC_DEV->bData[0]);
	uint8_t *previos_pbData = &(UVC_DEV->bData[0]);
	uint8_t i =	0;

	//uint8_t bLength =	0;
	uint8_t bDevId = 0;
	uint8_t bFunctionClass = 0;
	uint8_t bDescriptorType = 0;
	uint8_t bDescriptorSubType = 0;
	uint8_t bAlternateSetting = 0;
	uint8_t bInterfaceSubClass = 0;
	uint8_t IntfOffset=0;

	UVC_CLASS = (USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	//bDevId = UVC_DEV->device_id;
	bDevId = dev_idx;
	UVC_INFO_Struct.DEV[bDevId].iDevID = UVC_DEV->device_id;
	IntfOffset = UVC_INFO_Struct.DEV[bDevId].bIntfNum;
	do{
		bDescriptorType	= *(pbData+1);
		switch(bDescriptorType){
			case USBH_CONFIGURATION:
				UVC_INFO_Struct.DEV[bDevId].bIntfNum = *(pbData+4) - 1; //except interface 0
				pbData = pbData	+ *(pbData);
				break;
			case USBH_INTERFACE:
				bAlternateSetting = *(pbData+3);
				bInterfaceSubClass = *(pbData+6);
				if(bAlternateSetting == 0 && bInterfaceSubClass == SC_VIDEOSTREAMING){
					UVC_CLASS->MaxStreamCnt++;
				}
				pbData = pbData	+ *(pbData);
				break;
			case USBH_INTERFACE_ASSOCIATION:
				bFunctionClass = *(pbData+4);
				pbData = pbData	+ *(pbData);
				break;
			case USBH_ENDPOINT:
				pbData = pbData	+ *(pbData);
				break;
			case CS_INTERFACE:
				bDescriptorSubType = *(pbData+2);
				uvc_parser_intfdes(UVC_DEV, pbData, bFunctionClass, bInterfaceSubClass,	bDescriptorSubType, previos_pbData, dev_idx);			
				previos_pbData = pbData;
				pbData = pbData	+ *(pbData);
				break;
			case CS_ENDPOINT:
				bDescriptorSubType = *(pbData+2);
				uvc_parser_endpdes(UVC_DEV, pbData, bDescriptorSubType);
				pbData = pbData	+ *(pbData);
				break;
			default:
				pbData = pbData	+ *(pbData);
				break;
		}
	}while(*(pbData) !=	0x00);

	// Add EP INFO to UVC_INFO_Struct
	for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum-IntfOffset;i++){
		UVC_INFO_Struct.DEV[bDevId].INTF[i+IntfOffset].EP = &UVC_DEV->EP[i+1];
	}
}

uint8_t	uvc_get_intfnum(USBH_Device_Structure *UVC_DEV, uint8_t bTgtFmt, uint8_t intf_start_index)
{
	uint8_t i = 0, j = 0;
	uint8_t bDevId = 0;
	
	//bDevId = UVC_DEV->device_id;
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	for(i = intf_start_index; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
		for(j =	0; j < UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum; j++){
			if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bFmtType == bTgtFmt)
				return i+1;
		}
	}

	return 0;
}

void uvc_set_parameter(USBH_Device_Structure *UVC_DEV, USBH_UVC_STREAMING_CONTROL *UVC_PROBE, uint8_t intf_start_index) {
	
	uint8_t	i = 0;
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	
	// set compression window size
	UVC_PROBE->bCompWindowSize[0] = UVC_CLASS->PROBE.bCompWindowSize[0];
	UVC_PROBE->bCompWindowSize[1] = UVC_CLASS->PROBE.bCompWindowSize[1];

	// set maximum video frame size
	for(i = 0; i < sizeof(UVC_CLASS->PROBE.bMaxVideoFrameSize);	i++){
		UVC_PROBE->bMaxVideoFrameSize[i] = UVC_CLASS->PROBE.bMaxVideoFrameSize[i];
	}

	// set payload xfr size
	for(i = 0; i < sizeof(UVC_CLASS->PROBE.bMaxPayloadTransferSize); i++){
		UVC_PROBE->bMaxPayloadTransferSize[i] =	UVC_CLASS->PROBE.bMaxPayloadTransferSize[i];
	}
}

uint8_t	uvc_set_still_probe_param(USBH_Device_Structure *UVC_DEV, USBH_UVC_STILL_CONTROL *UVC_STILL_PROBE, uint8_t intf_start_index) {
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	uint8_t	i = 0, j = 0, k	= 0;
	uint8_t bDevId = 0;
	uint8_t	STATUS = FAIL;
	
	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	//bDevId = UVC_DEV->device_id;
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);

	if((UVC_CLASS->STILL_PROBE.bStillTgtWidth == 0) && (UVC_CLASS->STILL_PROBE.bStillTgtHeight)) {
		return STATUS;
	}

	for(i = intf_start_index; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
		for(j =	0; j < UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;	j++){
			if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bFmtType == UVC_CLASS->STILL_PROBE.bStillTgtFmt){
				UVC_STILL_PROBE->bFormatIndex = UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bFormatIndex;

				for(k =	0; k < UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].STILL_FRAME_Des.bNumImageSizePatterns; k++){
					if((UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].STILL_FRAME_Des.wWidth[k] == UVC_CLASS->STILL_PROBE.bStillTgtWidth)
							&& (UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].STILL_FRAME_Des.wHeight[k] == UVC_CLASS->STILL_PROBE.bStillTgtHeight)){
						UVC_STILL_PROBE->bFrameIndex = (k+1);

						UVC_CLASS->STILL_COMMIT.bStillInftIndex	= i;
						UVC_CLASS->STILL_COMMIT.bStillFmtIndex	= j;
						UVC_CLASS->STILL_COMMIT.bStillResIndex	= (k+1);
						STATUS = SUCCESS;
						break;
					}
				}
			}
		}
	}

	return STATUS;
}

uint8_t	uvc_set_probe(USBH_Device_Structure *UVC_DEV, USBH_UVC_STREAMING_CONTROL *UVC_PROBE, uint8_t intf_start_index) {
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	uint8_t	i = 0, j = 0, k	= 0, g = 0, h =	0;
	uint8_t bDevId = 0;
	uint8_t	STATUS = FAIL;

	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	//bDevId = UVC_DEV->device_id;
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	if(UVC_CLASS->PROBE.bTgtFmt	== 0 &&	UVC_CLASS->PROBE.bTgtRes == 0 && UVC_CLASS->PROBE.dwTgtFps == 0){ 
		// if uvc app doesn't set any value to preview,	use default probe setting
		UVC_CLASS->PROBE.bTgtFmt = USBH_UVC_STREAM_MJPEG;
		UVC_CLASS->PROBE.bTgtRes = USBH_UVC_STREAM_1280X720;
		UVC_CLASS->PROBE.dwTgtFps = USBH_UVC_STREAM_30_FPS;
	}

	for(i = intf_start_index; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
	//for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
		for(j =	0; j < UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;	j++){
			if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bFmtType == UVC_CLASS->PROBE.bTgtFmt){
				// set format index
				UVC_PROBE->bFormatIndex	= UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bFormatIndex;
				for(k =	0; k < UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bResNum; k++){
					if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bPixel == UVC_CLASS->PROBE.bTgtRes){
						// set frame index
						UVC_PROBE->bFrameIndex = UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFrameIndex;
						for(g =	0; g < UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFpsNum; g++){
							if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwFrameInterval == UVC_CLASS->PROBE.dwTgtFps){
								// set frame interval
								for(h =	0; h < 4; h++){
									UVC_PROBE->bFrameInterval[h] = (uint8_t)(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwFrameInterval>>h*8);
								}
								UVC_CLASS->COMMIT.bInftIndex =	i;
								UVC_CLASS->COMMIT.bFmtIndex = j;
								UVC_CLASS->COMMIT.bResIndex = k;
								UVC_CLASS->COMMIT.bFpsIndex = g;
								STATUS = SUCCESS;
								g = UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFpsNum;
								k = UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bResNum;
								j = UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;
								i = UVC_INFO_Struct.DEV[bDevId].bIntfNum;
								break;
							}
						}// end	of for(bFpsNum)
					}
				}// end	of for(bResNum)
			}		
		}// end	of for(bFmtNum)
	}

	return STATUS;
}

uint8_t	uvc_start_preview(USBH_Device_Structure	*UVC_DEV, USBH_ISO_XFR_REQ_Struct *ISO_REQ)
{
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	USBH_CX_XFR_REQ_Struct		CX_REQ;

	uint8_t	STATUS = FAIL;
	uint8_t	i = 0, j = 0;
	uint8_t	bAltSetFound = 0;

	uint8_t	bTransactionFrame = 0;
	uint16_t	wSize =	0;
	uint16_t	wMaxPayloadTransferSize	= 0;
	
	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	memset(&(CX_REQ), 0, sizeof(USBH_CX_XFR_REQ_Struct));
	
	wMaxPayloadTransferSize = (uint16_t)((UVC_CLASS->PROBE.bMaxPayloadTransferSize[0])\
						|(UVC_CLASS->PROBE.bMaxPayloadTransferSize[1]) << 8);

	for(i = 0; i < UVC_DEV->DEVDes.bCONFIGURATION_NUMBER; i++){
		for(j =	0; j < UVC_DEV->CFGDes[i].Interface[UVC_CLASS->PROBE.bIntfNum].bEP_NUMBER; j++){	
			bTransactionFrame =	(uint8_t)((UVC_DEV->CFGDes[i].Interface[UVC_CLASS->PROBE.bIntfNum].ENDPDes[j].bED_wMaxPacketSizeHighByte & 0x18)>>3) + 1;
			wSize = UVC_DEV->CFGDes[i].Interface[UVC_CLASS->PROBE.bIntfNum].ENDPDes[j].bED_wMaxPacketSizeHighByte;
			wSize = ((wSize << 8) | UVC_DEV->CFGDes[i].Interface[UVC_CLASS->PROBE.bIntfNum].ENDPDes[j].bED_wMaxPacketSizeLowByte);
			//*RtnMaxPktSize = wSize;		
			ISO_REQ->MaxPktSize	= wSize;
			wSize &= 0x07FF;
			if(wSize*bTransactionFrame == wMaxPayloadTransferSize){
				i = UVC_DEV->DEVDes.bCONFIGURATION_NUMBER;
				bAltSetFound = 1;		
				break;
			}
		}
	}

	if(bAltSetFound){
		ISO_REQ->IntfNum = UVC_CLASS->PROBE.bIntfNum;

		CX_REQ.CMD					=	USBH_CX_CMD_SET_INTERFACE;
		CX_REQ.TimeoutMsec	=	1000;
		CX_REQ.wValue				=	j+1;
		CX_REQ.wIndex				=	ISO_REQ->IntfNum;

		STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS == FAIL){
			UVC_INFO("ERROR");
		}
	}

	return STATUS;
}

uint8_t	uvc_stop_preview(USBH_Device_Structure *UVC_DEV, uint32_t dwStreamID)
{
	USBH_CX_XFR_REQ_Struct		CX_REQ;
	EHCI_ENDP_STRUCT					*EP = NULL;	
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	uint8_t STATUS = FAIL;
	uint8_t EPNUM = 0, j = 0;
	uint8_t XfrType = USBH_NONE_TYPE, NUM = 0;

	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;

	memset(&(CX_REQ), 0, sizeof(USBH_CX_XFR_REQ_Struct));

	if(UVC_DEV->CLASS_DRV == USBH_UVC_ISO_CLASS){
		CX_REQ.CMD		=	USBH_CX_CMD_SET_INTERFACE;
		CX_REQ.wValue	=	0;
		CX_REQ.wIndex	=	uvc_streamid_to_intfnum(UVC_DEV, dwStreamID);
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS == FAIL){
			UVC_DBG("SET_INTERFACE ERROR");
		}
	}
	else if(UVC_DEV->CLASS_DRV == USBH_UVC_BULK_CLASS){
		EP = uvc_streamid_to_ep(dwStreamID);
		CX_REQ.CMD		=	USBH_CX_CMD_CLEAR_FEATURE;
		CX_REQ.wValue	=	0;
		CX_REQ.wIndex	=	EP->EdNumber;
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS == FAIL){
			UVC_DBG("CLEAR_FEATURE ERROR");
		} else {
			XfrType	= USBH_BK_IN_TYPE;
			NUM		= UVC_CLASS->COMMIT.bInftIndex + 1;

			for (EPNUM = 1; EPNUM < MAX_QH_PER_DEVICE; EPNUM++) {
				if (UVC_DEV->EP[EPNUM].XfrType == XfrType) {
					j++;
					if (j == NUM) {
						break;
		}
	}
			}
			UVC_DEV->EP[EPNUM].DataTog = 0;
		}
	}

	return STATUS;
}

uint8_t uvc_still_probe(USBH_Device_Structure *UVC_DEV, uint8_t intf_start_index) {
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	USBH_UVC_STILL_CONTROL		UVC_STILL_PROBE;
	USBH_CX_XFR_REQ_Struct		CX_REQ;
	uint8_t *UVC_DATA_BUFF = NULL;
	uint8_t STATUS = FAIL, i = 0;

	UVC_CLASS		=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DATA_BUFF	=	(uint8_t*)UVC_DEV->BUFF.ptr;

	if(UVC_CLASS->STILL_PROBE.bStillIntfNum == 0)
		return STATUS;

	memset((uint8_t*)&UVC_STILL_PROBE, 0x00, sizeof(USBH_UVC_STILL_CONTROL));

	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Read;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)UVC_DATA_BUFF;
	CX_REQ.CLASS[0]	=	0xA1;
	CX_REQ.CLASS[1]	=	GET_CUR;
	CX_REQ.CLASS[2]	=	0x00;
	CX_REQ.CLASS[3]	=	VS_STILL_PROBE_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL PROBE ERROR\n");
		return STATUS;
	}
	memcpy(&UVC_STILL_PROBE, CX_REQ.pBUFF,  CX_REQ.ACT_SIZE);

	STATUS = uvc_set_still_probe_param(UVC_DEV, &UVC_STILL_PROBE, intf_start_index);
	if(STATUS == FAIL){
		UVC_DBG("STILL PROBE ERROR\n");
		return STATUS;
	}
	for(i = 0; i < 4; i++) {
		UVC_STILL_PROBE.bMaxPayloadTransferSize[i] = 0;
	}
	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Write;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)&UVC_STILL_PROBE;
	CX_REQ.CLASS[0]	=	0x21;
	CX_REQ.CLASS[1]	=	SET_CUR;
	CX_REQ.CLASS[2] =	0x00;
	CX_REQ.CLASS[3] =	VS_STILL_PROBE_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL PROBE ERROR\n");
		return STATUS;
	}

	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Read;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)UVC_DATA_BUFF;
	CX_REQ.CLASS[0]	=	0xA1;
	CX_REQ.CLASS[1]	=	GET_CUR;
	CX_REQ.CLASS[2]	=	0x00;
	CX_REQ.CLASS[3]	=	VS_STILL_PROBE_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL PROBE ERROR\n");
		return STATUS;
	}
	memcpy(&UVC_STILL_PROBE, CX_REQ.pBUFF,  CX_REQ.ACT_SIZE);

	for(i = 0; i < 4; i++) {
		UVC_STILL_PROBE.bMaxPayloadTransferSize[i] = 0;
	}
	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Write;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)&UVC_STILL_PROBE;
	CX_REQ.CLASS[0]	=	0x21;
	CX_REQ.CLASS[1]	=	SET_CUR;
	CX_REQ.CLASS[2] =	0x00;
	CX_REQ.CLASS[3] =	VS_STILL_PROBE_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL PROBE ERROR\n");
		return STATUS;
	}

	memcpy(&(UVC_CLASS->STILL_PROBE), &UVC_STILL_PROBE, CX_REQ.SIZE);

	return STATUS;
}

uint8_t	uvc_probe_commit(USBH_Device_Structure *UVC_DEV, uint8_t intf_start_index)
{
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	USBH_UVC_STREAMING_CONTROL	UVC_PROBE;
	USBH_CX_XFR_REQ_Struct	CX_REQ;

	uint8_t i =	0;
	uint8_t STATUS = SUCCESS;
	uint8_t uvc_cmd = USBH_UVC_PROBE_NONE;
	uint8_t *UVC_DATA_BUFF = NULL;
	uint32_t dwFrameInterval = 0;

	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DATA_BUFF	=	(uint8_t*)UVC_DEV->BUFF.ptr;
	
	do{
		switch(uvc_cmd){	
			case USBH_UVC_PROBE_NONE:	
				memset((uint8_t*)&UVC_PROBE, 0x00, sizeof(USBH_UVC_STREAMING_CONTROL));
				uvc_cmd	= USBH_UVC_GET_CUR;			
				break;
			case USBH_UVC_GET_CUR:
				memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
				CX_REQ.CMD	=	USBH_CX_CMD_CLASS_Read;
				CX_REQ.SIZE	=	26;
				CX_REQ.pBUFF	=	(uint32_t*)UVC_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0xA1;
				CX_REQ.CLASS[1]	=	GET_CUR;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	VS_PROBE_CONTROL;
				CX_REQ.CLASS[4]	=	UVC_CLASS->PROBE.bIntfNum;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){
					UVC_INFO("ERROR\n");
					return STATUS;
				}
				memcpy(&(UVC_CLASS->PROBE), CX_REQ.pBUFF,  CX_REQ.ACT_SIZE);
				uvc_cmd	= USBH_UVC_SET_CUR;
				break;
			case USBH_UVC_SET_CUR:
				memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
				for(i=0;i<4;i++){
					dwFrameInterval |= (uint32_t)(UVC_CLASS->PROBE.bFrameInterval[i] <<	i*8);
				}
				if((memcmp(&UVC_PROBE.bFrameInterval[0], &(UVC_CLASS->PROBE.bFrameInterval[0]),	4) == 0) && (dwFrameInterval !=	0)){
					uvc_set_parameter(UVC_DEV, &UVC_PROBE, intf_start_index);
					STATUS = uvc_set_streamid(UVC_DEV);
					memcpy(UVC_DATA_BUFF, &(UVC_CLASS->PROBE), 26);
					CX_REQ.pBUFF	=	(uint32_t*)UVC_DATA_BUFF;
					//CX_REQ.pBUFF	=	(uint32_t*)&(UVC_CLASS->PROBE);
					CX_REQ.CLASS[2]	=	0x00;
					CX_REQ.CLASS[3]	=	VS_COMMIT_CONTROL;
					uvc_cmd = USBH_UVC_PROBE_DONE;
				}else{
					STATUS = uvc_set_probe(UVC_DEV, &UVC_PROBE, intf_start_index);
					if(STATUS == FAIL){
						UVC_INFO("ERROR\n");
						return STATUS;
					}
					memcpy(UVC_DATA_BUFF, &UVC_PROBE, 26);
					CX_REQ.pBUFF	=	(uint32_t*)UVC_DATA_BUFF;
					//CX_REQ.pBUFF	=	(uint32_t*)&UVC_PROBE;
					CX_REQ.CLASS[2] =	0x00;
					CX_REQ.CLASS[3] =	VS_PROBE_CONTROL;
					uvc_cmd = USBH_UVC_GET_CUR;
				}
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Write;
				CX_REQ.SIZE			=	26;
				CX_REQ.CLASS[0]	=	0x21;
				CX_REQ.CLASS[1]	=	SET_CUR;
				CX_REQ.CLASS[4]	=	UVC_CLASS->PROBE.bIntfNum;
				CX_REQ.CLASS[5]	=	0x00;		
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){					
					UVC_INFO("ERROR\n");
					return STATUS;
				}
				break;
			case USBH_UVC_GET_MAX:
				break;
			case USBH_UVC_GET_MIN:
				break;
		}
	}while(uvc_cmd != USBH_UVC_PROBE_DONE);

#if defined(CONFIG_MODULE_USB_UVC_SNAPSHOT)
	uvc_still_setting(UVC_DEV, USBH_UVC_STREAM_MJPEG, 1280, 720, 0);
	uvc_still_probe(UVC_DEV, intf_start_index);
#endif

	return STATUS;
}
uint8_t	uvc_set_res_val(uint16_t wWidth, uint16_t wHeight)
{
	uint8_t rel	= 0;

	switch(wWidth){ 
		case 1920:
			switch(wHeight){
				case 1080:
					rel	= USBH_UVC_STREAM_1920X1080;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
		case 1280:
			switch(wHeight){
				case 800:
					rel	= USBH_UVC_STREAM_1280X800;
					break;
				case 720:
					rel	= USBH_UVC_STREAM_1280X720;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
		case 960:
			switch(wHeight){
				case 540:
					rel	= USBH_UVC_STREAM_960X540;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
		case 848:
			switch(wHeight){
				case 480:
					rel	= USBH_UVC_STREAM_848X480;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
		case 800:
			switch(wHeight){
				case 600:
					rel	= USBH_UVC_STREAM_800X600;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
		case 640:
			switch(wHeight){
				case 480:
					rel	= USBH_UVC_STREAM_640X480;
					break;
				case 360:
					rel	= USBH_UVC_STREAM_640X360;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
				break;
			}
			break;
		case 424:
			switch(wHeight){
				case 240:
					rel	= USBH_UVC_STREAM_424X240;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
		case 320:
			switch(wHeight){
				case 240:
					rel	= USBH_UVC_STREAM_320X240;
					break;
				case 180:
					rel	= USBH_UVC_STREAM_320X180;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
				break;
			}
			break;
		case 160:
			switch(wHeight){
				case 120:
					rel	= USBH_UVC_STREAM_160X120;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
			
		case 176:
			switch(wHeight){
				case 176:
					rel	= USBH_UVC_STREAM_176X176;
					break;
				default:
					rel	= USBH_UVC_STREAM_UNKNOW;
					break;
			}
			break;
			
		default:
			rel	= USBH_UVC_STREAM_UNKNOW;
			break;
	}
	return rel;
}

// UVC PRINT INFORMATION
void uvc_print_descriptor_info(USBH_Device_Structure *UVC_DEV)
{
	USBH_UVC_DESCRIPTOR_Structure	*UVC_DES		= NULL;
	USBH_UVC_CLASS_Structure			*UVC_CLASS	= NULL;

	//uint8_t *pbData =	NULL;
	//uint8_t i	= 0;
	//uint8_t j	= 0;
	//uint8_t h	= 0;

	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES = &(UVC_CLASS->DES);

	uvc_print_cs_intf_info(UVC_DES);
	
	uvc_print_iit_camera_info(UVC_DES);
	uvc_print_ot_info(UVC_DES);
	uvc_print_xu_info(UVC_DES);
	uvc_print_pu_info(UVC_DES);
	
	uvc_print_vs_intf_info(UVC_DES);

	uac_print_ac_intf_info(UVC_DES);
	uac_print_fu_info(UVC_DES);
	uac_print_su_info(UVC_DES);
	uac_print_mu_info(UVC_DES);
	uac_print_ot_info(UVC_DES);
	uac_print_it_info(UVC_DES);
	uac_print_as_intf_info(UVC_DES);	
}

void uvc_print_cs_intf_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0;
	uint16_t bcdVDC = 0;
	uint16_t wTotalLength = 0;
	uint32_t dwClockFrequency =	0;

	bcdVDC = (uint16_t)((UVC_DES->VC_Interface.bVerLowByte)|((UVC_DES->VC_Interface.bVerHighByte)<<8));
	wTotalLength = (uint16_t)((UVC_DES->VC_Interface.bTotalLengthLowByte)|((UVC_DES->VC_Interface.bTotalLengthHighByte)<<8));
	dwClockFrequency = (uint32_t)((UVC_DES->VC_Interface.bClockFrequency[0])|((UVC_DES->VC_Interface.bClockFrequency[1])<<8)\
				|((UVC_DES->VC_Interface.bClockFrequency[2])<<16)|((UVC_DES->VC_Interface.bClockFrequency[3])<<24));

	UVC_INFO("*** CS_INTERFACE Descriptor (%d bytes)***", UVC_DES->VC_Interface.bLength);
	UVC_INFO("bLength:\t\t\t0x%02x", UVC_DES->VC_Interface.bLength);
	UVC_INFO("bDescriptorType:\t\t0x%02x", UVC_DES->VC_Interface.bDescriptorType);
	UVC_INFO("bDescriptorSubType:\t\t0x%02x", UVC_DES->VC_Interface.bDescriptorSubType);
	UVC_INFO("bcdUSB:\t\t\t0x%04x", bcdVDC);
	UVC_INFO("wTotalLength:\t\t\t0x%04x", wTotalLength);
	UVC_INFO("dwClockFrequency:\t\t0x%08x", dwClockFrequency);
	UVC_INFO("bInCollection:\t\t\t0x%02x", UVC_DES->VC_Interface.bInCollection);
	
	for(i = 0; i < UVC_DES->VC_Interface.bInCollection;i++){
		UVC_INFO("baInterfaceNr(%d):\t\t0x%02x", i+1 ,UVC_DES->VC_Interface.baInterfaceNr[i]);
	}
	UVC_INFO("");
}

void uvc_print_ot_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0;
	uint16_t wTerminalType = 0;

	for(i = 0; i < MAX_DES_NUM;	i++){
		if(UVC_DES->VC_OT_Des[i].bLength != 0x00){
			wTerminalType = (uint16_t)((UVC_DES->VC_OT_Des[i].bTerminalTypeLowByte)\
				|((UVC_DES->VC_OT_Des[i].bTerminalTypeHighByte)<<8));
			UVC_INFO("*** OUTPUT_TERMINAL Descriptor #(%d) (%d bytes)***", i, UVC_DES->VC_OT_Des[i].bLength);
			UVC_INFO("bLength:\t\t\t0x%02x", UVC_DES->VC_OT_Des[i].bLength);
			UVC_INFO("bDescriptorType:\t\t0x%02x", UVC_DES->VC_OT_Des[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:\t\t0x%02x", UVC_DES->VC_OT_Des[i].bDescriptorSubType);
			UVC_INFO("bTerminalID:\t\t\t0x%02x", UVC_DES->VC_OT_Des[i].bTerminalID);
			UVC_INFO("wTerminalType:\t\t\t0x%04x", wTerminalType);
			UVC_INFO("bAssocTerminal:\t\t0x%02x", UVC_DES->VC_OT_Des[i].bAssocTerminal);
			UVC_INFO("bSourceID:\t\t\t0x%02x", UVC_DES->VC_OT_Des[i].bSourceID);
			UVC_INFO("iTerminal:\t\t\t0x%02x", UVC_DES->VC_OT_Des[i].iTerminal);
			UVC_INFO("");
		}
	}
}

void uvc_print_xu_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0, j =	0;

	for(i = 0; i < MAX_DES_NUM;	i++){
		if(UVC_DES->VC_XU_Des[i].bLength != 0x00){
			UVC_INFO("*** EXTENSION_UNIT Descriptor #(%d) (%d bytes)***", i, UVC_DES->VC_XU_Des[i].bLength);
			UVC_INFO("bLength:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].bLength);
			UVC_INFO("bDescriptorType:\t\t0x%02x", UVC_DES->VC_XU_Des[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:\t\t0x%02x", UVC_DES->VC_XU_Des[i].bDescriptorSubType);
			UVC_INFO("bUnitID:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].bUnitID);
			UVC_INFO("guidExtensionCode:\t\t%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[0],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[1],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[2],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[3],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[4],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[5],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[6],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[7],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[8],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[9],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[10],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[11],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[12],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[13],\
				  UVC_DES->VC_XU_Des[i].bGuidExtensionCode[14],UVC_DES->VC_XU_Des[i].bGuidExtensionCode[15]);
			UVC_INFO("bNumControls:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].bNumControls);
			UVC_INFO("bNrInPins:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].bNrInPins);
			UVC_INFO("baSourceID:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].baSourceID);
			UVC_INFO("bControlSize:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].bControlSize);
			for(j = 0; j < UVC_DES->VC_XU_Des[i].bControlSize; j++){
				UVC_INFO("bmControls(%d):\t\t\t0x%02x", j+1, UVC_DES->VC_XU_Des[i].bmControls[j]);
			}
			UVC_INFO("iExtension:\t\t\t0x%02x", UVC_DES->VC_XU_Des[i].iExtension);
			UVC_INFO("");
		}
	}
}

void uvc_print_iit_camera_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0;
	uint16_t wTerminalType = 0;
	uint16_t wObjectiveFocalLengthMin =	0;
	uint16_t wObjectiveFocalLengthMax =	0;
	uint16_t wOcularFocalLength	= 0;

	wTerminalType = (uint16_t)((UVC_DES->VC_Camera_IT_Des.bTerminalTypeLowByte)|((UVC_DES->VC_Camera_IT_Des.bTerminalTypeHighByte)<<8));
	wObjectiveFocalLengthMin = (uint16_t)((UVC_DES->VC_Camera_IT_Des.bObjectiveFocalLengthMinLowByte)\
					|((UVC_DES->VC_Camera_IT_Des.bObjectiveFocalLengthMinHighByte)<<8));
					
	wObjectiveFocalLengthMax = (uint16_t)((UVC_DES->VC_Camera_IT_Des.bObjectiveFocalLengthMaxLowByte)\
					|((UVC_DES->VC_Camera_IT_Des.bObjectiveFocalLengthMaxHighByte)<<8));
					
	wOcularFocalLength = (uint16_t)((UVC_DES->VC_Camera_IT_Des.bOcularFocalLengthLowByte)\
					|((UVC_DES->VC_Camera_IT_Des.bOcularFocalLengthHighByte)<<8));
	
	UVC_INFO("*** ITT_CAMERA Descriptor	(%d bytes)***",	UVC_DES->VC_Camera_IT_Des.bLength);
	UVC_INFO("bLength:\t\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.bLength);
	UVC_INFO("bDescriptorType:\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.bDescriptorType);
	UVC_INFO("bDescriptorSubType:\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.bDescriptorSubType);
	UVC_INFO("bTerminalID:\t\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.bTerminalID);
	UVC_INFO("wTerminalType:\t\t\t0x%04x", wTerminalType);
	UVC_INFO("bAssocTerminal:\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.bAssocTerminal);
	UVC_INFO("iTerminal:\t\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.iTerminal);
	UVC_INFO("wObjectiveFocalLengthMin:\t0x%04x", wObjectiveFocalLengthMin);
	UVC_INFO("wObjectiveFocalLengthMax:\t0x%04x", wObjectiveFocalLengthMax);
	UVC_INFO("wOcularFocalLength:\t\t0x%04x", wOcularFocalLength);
	UVC_INFO("bControlSize:\t\t\t0x%02x", UVC_DES->VC_Camera_IT_Des.bControlSize);
	for(i = 0; i < 3; i++){
		UVC_INFO("bmControls(%d):\t\t\t0x%02x", i, UVC_DES->VC_Camera_IT_Des.bmControls[i]);
	}
	UVC_INFO("");
}

void uvc_print_pu_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0;
	uint16_t wMaxMultiplier = 0;
	
	wMaxMultiplier = (uint16_t)((UVC_DES->VC_PU_Des.bMaxMultiplierLowByte)|((UVC_DES->VC_PU_Des.bMaxMultiplierLowByte)<<8));
	
	UVC_INFO("*** PROCESSING_UNIT Descriptor (%d bytes)***", UVC_DES->VC_PU_Des.bLength);
	UVC_INFO("bLength:\t\t\t0x%02x", UVC_DES->VC_PU_Des.bLength);
	UVC_INFO("bDescriptorType:\t\t0x%02x", UVC_DES->VC_PU_Des.bDescriptorType);
	UVC_INFO("bDescriptorSubType:\t\t0x%02x", UVC_DES->VC_PU_Des.bDescriptorSubType);
	UVC_INFO("bUnitID:\t\t\t0x%02x", UVC_DES->VC_PU_Des.bUnitID);
	UVC_INFO("bSourceID:\t\t\t0x%02x", UVC_DES->VC_PU_Des.bSourceID);
	UVC_INFO("wMaxMultiplier:\t\t0x%04x", wMaxMultiplier);
	UVC_INFO("bControlSize:\t\t\t0x%02x", UVC_DES->VC_PU_Des.bControlSize);
	for(i = 0; i < UVC_DES->VC_PU_Des.bControlSize; i++){
		UVC_INFO("bmControls(%d):\t\t\t0x%02x",i+1, UVC_DES->VC_PU_Des.bmControls[i]);
	}
	UVC_INFO("iProcessing:\t\t\t0x%02x", UVC_DES->VC_PU_Des.iProcessing);
	UVC_INFO("");
}

void uvc_print_yuv_info(USBH_UVC_YUV_Format_Descriptor_Struct	*YUV_FORMAT_Des)
{
	uint8_t  i = 0, j =	0;
	uint16_t wWidth = 0, wHeight = 0;
	uint32_t dwMinBitRate = 0, dwMaxBitRate = 0;
	uint32_t dwMaxVideoFrameBufferSize = 0, dwDefaultFrameInterval = 0,	dwFrameInterval	= 0;
	
	UVC_INFO("*** Uncompressed Video Format Descriptor (%d bytes)***", YUV_FORMAT_Des->bLength);
	UVC_INFO("bLength:\t\t\t0x%02x", YUV_FORMAT_Des->bLength);
	UVC_INFO("bDescriptorType:\t\t0x%02x", YUV_FORMAT_Des->bDescriptorType);
	UVC_INFO("bDescriptorSubType:\t\t0x%02x", YUV_FORMAT_Des->bDescriptorSubType);
	UVC_INFO("bFormatIndex:\t\t0x%02x", YUV_FORMAT_Des->bFormatIndex);
	UVC_INFO("bNumFrameDescriptors:\t0x%02x", YUV_FORMAT_Des->bNumFrameDescriptors);
	UVC_INFO("guidFormat:\t\t%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",\
		YUV_FORMAT_Des->guidFormat[0],YUV_FORMAT_Des->guidFormat[1],\
		YUV_FORMAT_Des->guidFormat[2],YUV_FORMAT_Des->guidFormat[3],\
		YUV_FORMAT_Des->guidFormat[4],YUV_FORMAT_Des->guidFormat[5],\
		YUV_FORMAT_Des->guidFormat[6],YUV_FORMAT_Des->guidFormat[7],\
		YUV_FORMAT_Des->guidFormat[8],YUV_FORMAT_Des->guidFormat[9],\
		YUV_FORMAT_Des->guidFormat[10],YUV_FORMAT_Des->guidFormat[11],\
		YUV_FORMAT_Des->guidFormat[12],YUV_FORMAT_Des->guidFormat[13],\
		YUV_FORMAT_Des->guidFormat[14],YUV_FORMAT_Des->guidFormat[15]);
	
	UVC_INFO("bBitsPerPixel:\t\t0x%02x", YUV_FORMAT_Des->bBitsPerPixel);
	UVC_INFO("bDefaultFrameIndex:\t\t0x%02x", YUV_FORMAT_Des->bDefaultFrameIndex);
	UVC_INFO("bAspectRatioX:\t\t0x%02x", YUV_FORMAT_Des->bAspectRatioX);
	UVC_INFO("bAspectRatioY:\t\t0x%02x", YUV_FORMAT_Des->bAspectRatioY);
	UVC_INFO("bmInterlaceFlags:\t\t0x%02x", YUV_FORMAT_Des->bmInterlaceFlags);
	UVC_INFO("bCopyProtect:\t\t0x%02x", YUV_FORMAT_Des->bCopyProtect);
	UVC_INFO("");
	
	for(i = 0; i < YUV_FORMAT_Des->bNumFrameDescriptors; i++){
		wWidth = (uint16_t)((YUV_FORMAT_Des->YUV_FRAME_Des[i].bWidthLowByte)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bWidthHighByte)<<8));

		wHeight	= (uint16_t)((YUV_FORMAT_Des->YUV_FRAME_Des[i].bHeightLowByte)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bHeightHighByte)<<8));
	
		dwMinBitRate = (uint32_t)((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMinBitRate[0])\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMinBitRate[1])<<8)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMinBitRate[2])<<16)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMinBitRate[3])<<24));
				
		dwMaxBitRate = (uint32_t)((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxBitRate[0])\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxBitRate[1])<<8)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxBitRate[2])<<16)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxBitRate[3])<<24));
				
		dwMaxVideoFrameBufferSize = (uint32_t)((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxVideoFrameBufferSize[0])\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxVideoFrameBufferSize[1])<<8)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxVideoFrameBufferSize[2])<<16)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bMaxVideoFrameBufferSize[3])<<24));
				
		dwDefaultFrameInterval = (uint32_t)((YUV_FORMAT_Des->YUV_FRAME_Des[i].bDefaultFrameInterval[0])\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bDefaultFrameInterval[1])<<8)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bDefaultFrameInterval[2])<<16)\
			|((YUV_FORMAT_Des->YUV_FRAME_Des[i].bDefaultFrameInterval[3])<<24));
				
		UVC_INFO("*** Uncompressed Video Frame Descriptor #(%d)(%d bytes)***", i+1, YUV_FORMAT_Des->YUV_FRAME_Des[i].bLength);
		UVC_INFO("bLength:\t\t\t0x%02x", YUV_FORMAT_Des->YUV_FRAME_Des[i].bLength);
		UVC_INFO("bDescriptorType:\t\t0x%02x", YUV_FORMAT_Des->YUV_FRAME_Des[i].bDescriptorType);
		UVC_INFO("bDescriptorSubType:\t\t0x%02x", YUV_FORMAT_Des->YUV_FRAME_Des[i].bDescriptorSubType);
		UVC_INFO("bFrameIndex:\t\t0x%02x", YUV_FORMAT_Des->YUV_FRAME_Des[i].bFrameIndex);
		UVC_INFO("bmCapabilities:\t\t0x%02x", YUV_FORMAT_Des->YUV_FRAME_Des[i].bmCapabilities);
		UVC_INFO("wWidth:\t\t\t0x%04x", wWidth);
		UVC_INFO("wHeight:\t\t\t0x%04x", wHeight);
		UVC_INFO("dwMinBitRate:\t\t0x%08x", dwMinBitRate);
		UVC_INFO("dwMaxBitRate:\t\t0x%08x", dwMaxBitRate);
		UVC_INFO("dwMaxVideoFrameBufferSize:\t0x%08x", dwMaxVideoFrameBufferSize);
		UVC_INFO("dwDefaultFrameInterval:\t0x%08x", dwDefaultFrameInterval);
		UVC_INFO("bFrameIntervalType:\t0x%02x", YUV_FORMAT_Des->YUV_FRAME_Des[i].bFrameIntervalType);
		for(j =	0; j < YUV_FORMAT_Des->YUV_FRAME_Des[i].bFrameIntervalType; j++){
			dwFrameInterval = YUV_FORMAT_Des->YUV_FRAME_Des[i].dwFrameInterval[j];
			UVC_INFO("dwFrameInterval(%d):\t0x%08x", j+1,	dwFrameInterval);
		}
		UVC_INFO("");
	}	
}

void uvc_print_fb_info(USBH_UVC_Frame_Based_Format_Descriptor_Struct *FB_FORMAT_Des)
{
	uint8_t  i = 0, j =	0;
	uint16_t wWidth = 0, wHeight = 0;
	uint32_t dwMinBitRate = 0, dwMaxBitRate = 0;
	uint32_t dwDefaultFrameInterval = 0, dwBytesPerLine	= 0, dwFrameInterval = 0;
	
	UVC_INFO("*** Frame	Based Payload Video Format Descriptor (%d bytes)***", FB_FORMAT_Des->bLength);
	UVC_INFO("bLength:\t\t\t0x%02x", FB_FORMAT_Des->bLength);
	UVC_INFO("bDescriptorType:\t\t0x%02x", FB_FORMAT_Des->bDescriptorType);
	UVC_INFO("bDescriptorSubType:\t\t0x%02x", FB_FORMAT_Des->bDescriptorSubType);
	UVC_INFO("bFormatIndex:\t\t\t0x%02x", FB_FORMAT_Des->bFormatIndex);
	UVC_INFO("bNumFrameDescriptors:\t\t0x%02x", FB_FORMAT_Des->bNumFrameDescriptors);
	UVC_INFO("guidFormat:\t\t\t%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",\
		FB_FORMAT_Des->guidFormat[0],FB_FORMAT_Des->guidFormat[1],\
		FB_FORMAT_Des->guidFormat[2],FB_FORMAT_Des->guidFormat[3],\
		FB_FORMAT_Des->guidFormat[4],FB_FORMAT_Des->guidFormat[5],\
		FB_FORMAT_Des->guidFormat[6],FB_FORMAT_Des->guidFormat[7],\
		FB_FORMAT_Des->guidFormat[8],FB_FORMAT_Des->guidFormat[9],\
		FB_FORMAT_Des->guidFormat[10],FB_FORMAT_Des->guidFormat[11],\
		FB_FORMAT_Des->guidFormat[12],FB_FORMAT_Des->guidFormat[13],\
		FB_FORMAT_Des->guidFormat[14],FB_FORMAT_Des->guidFormat[15]);
	
	UVC_INFO("bBitsPerPixel:\t\t\t0x%02x", FB_FORMAT_Des->bBitsPerPixel);
	UVC_INFO("bDefaultFrameIndex:\t\t0x%02x", FB_FORMAT_Des->bDefaultFrameIndex);
	UVC_INFO("bAspectRatioX:\t\t\t0x%02x", FB_FORMAT_Des->bAspectRatioX);
	UVC_INFO("bAspectRatioY:\t\t\t0x%02x", FB_FORMAT_Des->bAspectRatioY);
	UVC_INFO("bmInterlaceFlags:\t\t0x%02x", FB_FORMAT_Des->bmInterlaceFlags);
	UVC_INFO("bCopyProtect:\t\t\t0x%02x", FB_FORMAT_Des->bCopyProtect);
	UVC_INFO("bVariableSize:\t\t\t0x%02x", FB_FORMAT_Des->bVariableSize);
	UVC_INFO("");
	
	for(i = 0; i < FB_FORMAT_Des->bNumFrameDescriptors;i++){
		wWidth = (uint16_t)((FB_FORMAT_Des->FB_FRAME_Des[i].bWidthLowByte)\
			   |((FB_FORMAT_Des->FB_FRAME_Des[i].bWidthHighByte)<<8));
			   
		wHeight	= (uint16_t)((FB_FORMAT_Des->FB_FRAME_Des[i].bHeightLowByte)\
			   |((FB_FORMAT_Des->FB_FRAME_Des[i].bHeightHighByte)<<8));
	
		dwMinBitRate = (uint32_t)((FB_FORMAT_Des->FB_FRAME_Des[i].bMinBitRate[0])\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bMinBitRate[1])<<8)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bMinBitRate[2])<<16)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bMinBitRate[3])<<24));
				
		dwMaxBitRate = (uint32_t)((FB_FORMAT_Des->FB_FRAME_Des[i].bMaxBitRate[0])\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bMaxBitRate[1])<<8)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bMaxBitRate[2])<<16)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bMaxBitRate[3])<<24));
				
		dwDefaultFrameInterval = (uint32_t)((FB_FORMAT_Des->FB_FRAME_Des[i].bDefaultFrameInterval[0])\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bDefaultFrameInterval[1])<<8)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bDefaultFrameInterval[2])<<16)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bDefaultFrameInterval[3])<<24));
				
		dwBytesPerLine = (uint32_t)((FB_FORMAT_Des->FB_FRAME_Des[i].bBytesPerLine[0])\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bBytesPerLine[1])<<8)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bBytesPerLine[2])<<16)\
			|((FB_FORMAT_Des->FB_FRAME_Des[i].bBytesPerLine[3])<<24));
				
		UVC_INFO("*** Frame Based Payload Video Frame Descriptor #(%d)(%d bytes)***",	i+1, FB_FORMAT_Des->FB_FRAME_Des[i].bLength);
		UVC_INFO("bLength:\t\t\t0x%02x", FB_FORMAT_Des->FB_FRAME_Des[i].bLength);
		UVC_INFO("bDescriptorType:\t\t0x%02x", FB_FORMAT_Des->FB_FRAME_Des[i].bDescriptorType);
		UVC_INFO("bDescriptorSubType:\t\t0x%02x", FB_FORMAT_Des->FB_FRAME_Des[i].bDescriptorSubType);
		UVC_INFO("bFrameIndex:\t\t\t0x%02x", FB_FORMAT_Des->FB_FRAME_Des[i].bFrameIndex);
		UVC_INFO("bmCapabilities:\t\t0x%02x", FB_FORMAT_Des->FB_FRAME_Des[i].bmCapabilities);
		UVC_INFO("wWidth:\t\t\t0x%04x", wWidth);
		UVC_INFO("wHeight:\t\t\t0x%04x", wHeight);
		UVC_INFO("dwMinBitRate:\t\t\t0x%08x", dwMinBitRate);
		UVC_INFO("dwMaxBitRate:\t\t\t0x%08x", dwMaxBitRate);
		UVC_INFO("dwDefaultFrameInterval:\t0x%08x", dwDefaultFrameInterval);
		UVC_INFO("bFrameIntervalType:\t\t0x%02x", FB_FORMAT_Des->FB_FRAME_Des[i].bFrameIntervalType);
		UVC_INFO("dwBytesPerLine:\t\t0x%08x", dwBytesPerLine);
		for(j =	0; j < FB_FORMAT_Des->FB_FRAME_Des[i].bFrameIntervalType; j++){
			dwFrameInterval = FB_FORMAT_Des->FB_FRAME_Des[i].dwFrameInterval[j];
			UVC_INFO("dwFrameInterval(%d):\t\t0x%08x", j+1,	dwFrameInterval);
		}
		UVC_INFO("");
	}		
}

void uvc_print_vs_info(USBH_UVC_CS_Format_Descriptor_Struct *VS_FORMAT_Des)
{
	uint8_t  i = 0, j =	0;
	uint16_t wWidth = 0, wHeight = 0;
	uint32_t dwMinBitRate = 0, dwMaxBitRate = 0;
	uint32_t dwMaxVideoFrameBufSize = 0, dwDefaultFrameInterval	= 0, dwFrameInterval = 0;
	
	UVC_INFO("*** Class	- specific VS Format Descriptor	(%d bytes)***",	VS_FORMAT_Des->bLength);
	UVC_INFO("bLength:\t\t\t0x%02x", VS_FORMAT_Des->bLength);
	UVC_INFO("bDescriptorType:\t\t0x%02x", VS_FORMAT_Des->bDescriptorType);
	UVC_INFO("bDescriptorSubType:\t\t0x%02x", VS_FORMAT_Des->bDescriptorSubType);
	UVC_INFO("bFormatIndex:\t\t\t0x%02x", VS_FORMAT_Des->bFormatIndex);
	UVC_INFO("bNumFrameDescriptors:\t\t0x%02x", VS_FORMAT_Des->bNumFrameDescriptors);
	UVC_INFO("bmFlags:\t\t\t0x%02x", VS_FORMAT_Des->bmFlags);
	UVC_INFO("bDefaultFrameIndex:\t\t0x%02x", VS_FORMAT_Des->bDefaultFrameIndex);
	UVC_INFO("bAspectRatioX:\t\t\t0x%02x", VS_FORMAT_Des->bAspectRatioX);
	UVC_INFO("bAspectRatioY:\t\t\t0x%02x", VS_FORMAT_Des->bAspectRatioY);
	UVC_INFO("bmInterlaceFlags:\t\t0x%02x", VS_FORMAT_Des->bmInterlaceFlags);
	UVC_INFO("bCopyProtect:\t\t\t0x%02x", VS_FORMAT_Des->bCopyProtect);
	UVC_INFO("");
	
	for(i = 0; i < VS_FORMAT_Des->bNumFrameDescriptors;	i++){
		wWidth = (uint16_t)((VS_FORMAT_Des->VS_FRAME_Des[i].bWidthLowByte)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bWidthHighByte)<<8));
			
		wHeight	= (uint16_t)((VS_FORMAT_Des->VS_FRAME_Des[i].bHeightLowByte)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bHeightHighByte)<<8));
	
		dwMinBitRate = (uint32_t)((VS_FORMAT_Des->VS_FRAME_Des[i].bMinBitRate[0])\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMinBitRate[1])<<8)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMinBitRate[2])<<16)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMinBitRate[3])<<24));
				
		dwMaxBitRate = (uint32_t)((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxBitRate[0])\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxBitRate[1])<<8)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxBitRate[2])<<16)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxBitRate[3])<<24));
				
		dwMaxVideoFrameBufSize = (uint32_t)((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxVideoFrameBufSize[0])\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxVideoFrameBufSize[1])<<8)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxVideoFrameBufSize[2])<<16)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bMaxVideoFrameBufSize[3])<<24));
				
		dwDefaultFrameInterval = (uint32_t)((VS_FORMAT_Des->VS_FRAME_Des[i].bDefaultFrameInterval[0])\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bDefaultFrameInterval[1])<<8)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bDefaultFrameInterval[2])<<16)\
			|((VS_FORMAT_Des->VS_FRAME_Des[i].bDefaultFrameInterval[3])<<24));
				
		UVC_INFO("*** Class - specific VS Frame	Descriptor #(%d)(%d bytes)***",	i+1, VS_FORMAT_Des->VS_FRAME_Des[i].bLength);
		UVC_INFO("bLength:\t\t\t0x%02x", VS_FORMAT_Des->VS_FRAME_Des[i].bLength);
		UVC_INFO("bDescriptorType:\t\t0x%02x", VS_FORMAT_Des->VS_FRAME_Des[i].bDescriptorType);
		UVC_INFO("bDescriptorSubType:\t\t0x%02x", VS_FORMAT_Des->VS_FRAME_Des[i].bDescriptorSubType);
		UVC_INFO("bFrameIndex:\t\t\t0x%02x", VS_FORMAT_Des->VS_FRAME_Des[i].bFrameIndex);
		UVC_INFO("bmCapabilities:\t\t0x%02x", VS_FORMAT_Des->VS_FRAME_Des[i].bmCapabilities);
		UVC_INFO("wWidth:\t\t\t0x%04x", wWidth);
		UVC_INFO("wHeight:\t\t\t0x%04x", wHeight);
		UVC_INFO("dwMinBitRate:\t\t\t0x%08x", dwMinBitRate);
		UVC_INFO("dwMaxBitRate:\t\t\t0x%08x", dwMaxBitRate);
		UVC_INFO("dwMaxVideoFrameBufSize:\t0x%08x", dwMaxVideoFrameBufSize);
		UVC_INFO("dwDefaultFrameInterval:\t0x%08x", dwDefaultFrameInterval);
		UVC_INFO("bFrameIntervalType:\t\t0x%02x", VS_FORMAT_Des->VS_FRAME_Des[i].bFrameIntervalType);
		for(j =	0; j < VS_FORMAT_Des->VS_FRAME_Des[i].bFrameIntervalType; j++){
			dwFrameInterval = VS_FORMAT_Des->VS_FRAME_Des[i].dwFrameInterval[j];
			UVC_INFO("dwFrameInterval(%d):\t\t0x%08x", j+1,	dwFrameInterval);
		}
		UVC_INFO("");
	}	
}

void uvc_print_vs_intf_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0, j =	0;
	uint16_t wTotalLength = 0;
	
	for(i = 0; i < UVC_INTERFACE_NUM_MAX; i++){
		if(UVC_DES->VS_Interface[i].bLength != 0x00){
			wTotalLength = (uint16_t)((UVC_DES->VS_Interface[i].bTotalLengthLowByte)|((UVC_DES->VS_Interface[i].bTotalLengthHighByte)<<8));
			UVC_INFO("*** Class	- specific VS Header Descriptor	#(%d)(%d bytes)***", i+1, UVC_DES->VS_Interface[i].bLength);
			UVC_INFO("bLength:\t\t\t0x%02x", UVC_DES->VS_Interface[i].bLength);
			UVC_INFO("bDescriptorType:\t\t0x%02x", UVC_DES->VS_Interface[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:\t\t0x%02x", UVC_DES->VS_Interface[i].bDescriptorSubType);
			UVC_INFO("bNumFormats:\t\t\t0x%02x", UVC_DES->VS_Interface[i].bNumFormats);
			UVC_INFO("wTotalLength:\t\t\t0x%02x", wTotalLength);
			UVC_INFO("bEndpointAddress:\t\t0x%04x", UVC_DES->VS_Interface[i].bEndpointAddress);
			UVC_INFO("bmInfo:\t\t\t0x%02x", UVC_DES->VS_Interface[i].bmInfo);
			UVC_INFO("bTerminalLink:\t\t\t0x%02x", UVC_DES->VS_Interface[i].bTerminalLink);
			UVC_INFO("bStillCaptureMethod:\t\t0x%02x", UVC_DES->VS_Interface[i].bStillCaptureMethod);
			UVC_INFO("bTriggerSupport:\t\t0x%02x", UVC_DES->VS_Interface[i].bTriggerSupport);
			UVC_INFO("bTriggerUsage:\t\t\t0x%02x", UVC_DES->VS_Interface[i].bTriggerUsage);
			UVC_INFO("bControlSize:\t\t\t0x%02x", UVC_DES->VS_Interface[i].bControlSize);
			for(j = 0; j < UVC_DES->VS_Interface[i].bControlSize; j++){
				UVC_INFO("bmaControls(%d):\t\t0x%02x",j+1, UVC_DES->VS_Interface[i].bmaControls[j]);
			}
			UVC_INFO("");
		
			if(UVC_DES->VS_Interface[i].VS_FORMAT_Des.bLength != 0){
				uvc_print_vs_info(&(UVC_DES->VS_Interface[i].VS_FORMAT_Des));
			}
		
			if(UVC_DES->VS_Interface[i].FB_FORMAT_Des.bLength != 0){
				uvc_print_fb_info(&(UVC_DES->VS_Interface[i].FB_FORMAT_Des));
			}
		
			if(UVC_DES->VS_Interface[i].YUV_FORMAT_Des.bLength != 0){
				uvc_print_yuv_info(&(UVC_DES->VS_Interface[i].YUV_FORMAT_Des));
			}				
		}
	}
}

void uac_print_ac_intf_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint16_t bcdADC = 0;
	uint16_t wTotalLength = 0;

	if(UVC_DES->AC_Interface.bLength !=	0){
		bcdADC = (uint16_t)((UVC_DES->AC_Interface.bcdADC[0])|((UVC_DES->AC_Interface.bcdADC[1])<<8));
		wTotalLength = (uint16_t)((UVC_DES->AC_Interface.bTotalLengthLowByte)|((UVC_DES->AC_Interface.bTotalLengthHighByte)<<8));

		UVC_INFO("*** AC_INTERFACE Descriptor (%d bytes)***\n",	UVC_DES->AC_Interface.bLength);
		UVC_INFO("bLength:			 0x%02x\n",	UVC_DES->AC_Interface.bLength);
		UVC_INFO("bDescriptorType:		 0x%02x\n",	UVC_DES->AC_Interface.bDescriptorType);
		UVC_INFO("bDescriptorSubType:		 0x%02x\n",	UVC_DES->AC_Interface.bDescriptorSubType);
		UVC_INFO("bcdUSB:			 0x%04x\n",	bcdADC);
		UVC_INFO("wTotalLength:			 0x%04x\n",	wTotalLength);
		UVC_INFO("bInCollection:		 0x%02x\n",	UVC_DES->AC_Interface.bInCollection);
		UVC_INFO("\r\n");
	}
}


void uac_print_fu_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t	i = 0;

	if(UVC_DES->AC_FUD.bLength != 0){
		UVC_INFO("*** Feature Unit Descriptor (%d bytes)***\n",	UVC_DES->AC_FUD.bLength);
		UVC_INFO("bLength:			 0x%02x\n",	UVC_DES->AC_FUD.bLength);
		UVC_INFO("bDescriptorType:		 0x%02x\n",	UVC_DES->AC_FUD.bDescriptorType);
		UVC_INFO("bDescriptorSubType:		 0x%02x\n",	UVC_DES->AC_FUD.bDescriptorSubType);
		UVC_INFO("bUnitID:			 0x%02x\n",	UVC_DES->AC_FUD.bUnitID);
		UVC_INFO("bSourceID:			 0x%02x\n",	UVC_DES->AC_FUD.bSourceID);
		UVC_INFO("bControlSize:			 0x%02x\n",	UVC_DES->AC_FUD.bControlSize);

		for(i=0;i<UVC_DES->AC_FUD.bControlSize;i++){		
			UVC_INFO("bmaControls(%d):		  0x%02x\n", i,	UVC_DES->AC_FUD.bmaControls[i][0]);
		}
		UVC_INFO("iFeature:			 0x%02x\n",	UVC_DES->AC_FUD.iFeature);
		UVC_INFO("\r\n");
	}
}

void uac_print_su_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t	i = 0, j = 0;

	for(i = 0; i < MAX_DES_NUM;	i++){
		if(UVC_DES->AC_SUD[i].bLength != 0){
			UVC_INFO("*** Selector Unit	Descriptor #(%d) (%d bytes)***\n", i+1,	UVC_DES->AC_SUD[i].bLength);
			UVC_INFO("bLength:		 0x%02x\n", UVC_DES->AC_SUD[i].bLength);
			UVC_INFO("bDescriptorType:	 0x%02x\n", UVC_DES->AC_SUD[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:	 0x%02x\n", UVC_DES->AC_SUD[i].bDescriptorSubType);
			UVC_INFO("bUnitID:		 0x%02x\n", UVC_DES->AC_SUD[i].bUnitID);
			UVC_INFO("bNrInPins:		 0x%02x\n", UVC_DES->AC_SUD[i].bNrInPins);
			for(j=0;j<UVC_DES->AC_SUD[i].bNrInPins;j++){
				UVC_INFO("baSourceID(%d):	 0x%02x\n",	j+1, UVC_DES->AC_SUD[i].baSourceID[j]);
			}
			UVC_INFO("iSelector:	 0x%02x\n", UVC_DES->AC_SUD[i].iSelector);
			UVC_INFO("\r\n");
		}
	}
}

void uac_print_mu_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t	i = 0;
	uint16_t	wChannelConfig = 0;

	if(UVC_DES->AC_MUD.bLength != 0){
		wChannelConfig = (uint16_t)((UVC_DES->AC_MUD.bChannelConfigLowByte)|((UVC_DES->AC_MUD.bChannelConfigHighByte)<<8));

		UVC_INFO("*** Mixer Unit Descriptor (%d	bytes)***\n", UVC_DES->AC_MUD.bLength);
		UVC_INFO("bLength:			 0x%02x\n",	UVC_DES->AC_MUD.bLength);
		UVC_INFO("bDescriptorType:		 0x%02x\n",	UVC_DES->AC_MUD.bDescriptorType);
		UVC_INFO("bDescriptorSubType:		 0x%02x\n",	UVC_DES->AC_MUD.bDescriptorSubType);
		UVC_INFO("bUnitID:			 0x%02x\n",	UVC_DES->AC_MUD.bUnitID);
		UVC_INFO("bNrInPins:			 0x%02x\n",	UVC_DES->AC_MUD.bNrInPins);
		for(i=0;i<UVC_DES->AC_MUD.bNrInPins;i++){
			UVC_INFO("baSourceID(%d):		 0x%02x\n", i+1, UVC_DES->AC_MUD.baSourceID[i]);
		}
		UVC_INFO("bNrChannels:			 0x%02x\n",	UVC_DES->AC_MUD.bNrChannels);
		UVC_INFO("wChannelConfig:		 0x%04x\n",	wChannelConfig);
		UVC_INFO("iChannelNames:		 0x%02x\n",	UVC_DES->AC_MUD.iChannelNames);
		for(i=0;i<UVC_DES->AC_MUD.iChannelNames;i++){	
			UVC_INFO("bmControls(%d):		 0x%02x\n", i+1, UVC_DES->AC_MUD.bmControls[i]);
		}
		UVC_INFO("iMixer:			 0x%02x\n",	UVC_DES->AC_MUD.iMixer);
		UVC_INFO("\r\n");
	}
}

void uac_print_ot_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t	i = 0;
	uint16_t	wTerminalType =	0;

	for(i = 0; i < MAX_DES_NUM;	i++){
		if(UVC_DES->AC_OTD[i].bLength != 0){
			wTerminalType = (uint16_t)((UVC_DES->AC_OTD[i].bTerminalTypeLowByte)|((UVC_DES->AC_OTD[i].bTerminalTypeHighByte)<<8));

			UVC_INFO("*** Output Terminal Descriptor #(%d) (%d bytes)***\n", i+1, UVC_DES->AC_OTD[i].bLength);
			UVC_INFO("bLength:		 0x%02x\n", UVC_DES->AC_OTD[i].bLength);
			UVC_INFO("bDescriptorType:	 0x%02x\n", UVC_DES->AC_OTD[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:	 0x%02x\n", UVC_DES->AC_OTD[i].bDescriptorSubType);
			UVC_INFO("bTerminalID:		 0x%02x\n", UVC_DES->AC_OTD[i].bTerminalID);
			UVC_INFO("wTerminalType:	 0x%04x\n", wTerminalType);
			UVC_INFO("bAssocTerminal:	 0x%02x\n", UVC_DES->AC_OTD[i].bAssocTerminal);
			UVC_INFO("bSourceID:		 0x%02x\n", UVC_DES->AC_OTD[i].bSourceID);
			UVC_INFO("iTerminal:		 0x%02x\n", UVC_DES->AC_OTD[i].iTerminal);
			UVC_INFO("\r\n");
		}
	}
}

void uac_print_it_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t	i = 0;
	uint16_t	wTerminalType =	0;
	uint16_t	wbmChannelConfig = 0;

	for(i = 0; i < MAX_DES_NUM;	i++){
		if(UVC_DES->AC_ITD[i].bLength != 0){
			wTerminalType = (uint16_t)((UVC_DES->AC_ITD[i].bTerminalTypeLowByte)|((UVC_DES->AC_ITD[i].bTerminalTypeHighByte)<<8));
			wbmChannelConfig = (uint16_t)((UVC_DES->AC_ITD[i].bChannelConfigLowByte)|((UVC_DES->AC_ITD[i].bChannelConfigHighByte)<<8));

			UVC_INFO("*** Input	Terminal Descriptor #(%d) (%d bytes)***\n", i+1, UVC_DES->AC_ITD[i].bLength);
			UVC_INFO("bLength:		 0x%02x\n", UVC_DES->AC_ITD[i].bLength);
			UVC_INFO("bDescriptorType:	 0x%02x\n", UVC_DES->AC_ITD[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:	 0x%02x\n", UVC_DES->AC_ITD[i].bDescriptorSubType);
			UVC_INFO("bTerminalID:		 0x%02x\n", UVC_DES->AC_ITD[i].bTerminalID);
			UVC_INFO("wTerminalType:	 0x%04x\n", wTerminalType);
			UVC_INFO("bAssocTerminal:	 0x%02x\n", UVC_DES->AC_ITD[i].bAssocTerminal);
			UVC_INFO("bNrChannels:		 0x%02x\n", UVC_DES->AC_ITD[i].bNrChannels);
			UVC_INFO("wbmChannelConfig:	 0x%04x\n", wbmChannelConfig);
			UVC_INFO("iChannelNames:	 0x%02x\n", UVC_DES->AC_ITD[i].iChannelNames);
			UVC_INFO("iTerminal:		 0x%02x\n", UVC_DES->AC_ITD[i].iTerminal);
			UVC_INFO("\r\n");
		}
	}
}

void uac_print_as_intf_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES)
{
	uint8_t  i = 0;
	uint16_t wmFormats = 0;

	for(i = 0; i < UVC_INTERFACE_NUM_MAX; i++){
		if(UVC_DES->AS_Interface[i].bLength != 0){
			wmFormats =	(uint16_t)((UVC_DES->AS_Interface[i].bmFormats[0])|((UVC_DES->AS_Interface[i].bmFormats[1])<<8));
			UVC_INFO("*** AS_INTERFACE Descriptor #(%d)	 (%d bytes)***\n", i+1,	UVC_DES->AS_Interface[i].bLength);
			UVC_INFO("bLength:		 0x%02x\n", UVC_DES->AS_Interface[i].bLength);
			UVC_INFO("bDescriptorType:	 0x%02x\n", UVC_DES->AS_Interface[i].bDescriptorType);
			UVC_INFO("bDescriptorSubType:	 0x%02x\n", UVC_DES->AS_Interface[i].bDescriptorSubType);
			UVC_INFO("bTerminalLink:	 0x%02x\n", UVC_DES->AS_Interface[i].bTerminalLink);
			UVC_INFO("bDelay:		 0x%02x\n", UVC_DES->AS_Interface[i].bDelay);
			UVC_INFO("wmFormats:		 0x%04x\n", wmFormats);
			UVC_INFO("\r\n");
			uac_print_as_fmt_type_info(&UVC_DES->AS_Interface[i].AS_TFMD);
		}
	}
}

void uac_print_as_fmt_type_info(USBH_UAC_AS_FMT_TYPE_Descriptor_Struct *AS_TFMD)
{
	uint8_t  i = 0;
	uint32_t tSamFreq =	0;

	if(AS_TFMD->bLength	!= 0){
		UVC_INFO("*** AS_INTERFACE Format Type (%d bytes)***\n", AS_TFMD->bLength);
		UVC_INFO("bLength:			 0x%02x\n",	AS_TFMD->bLength);
		UVC_INFO("bDescriptorType:		 0x%02x\n",	AS_TFMD->bDescriptorType);
		UVC_INFO("bDescriptorSubType:		 0x%02x\n",	AS_TFMD->bDescriptorSubType);
		UVC_INFO("bFormatType:			 0x%02x\n",	AS_TFMD->bFormatType);
		UVC_INFO("bNrChannels:			 0x%02x\n",	AS_TFMD->bNrChannels);
		UVC_INFO("bSubFrameSize:		 0x%02x\n",	AS_TFMD->bSubFrameSize);
		UVC_INFO("bBitResolution:		 0x%02x\n",	AS_TFMD->bBitResolution);
		UVC_INFO("bSamFreqType:			 0x%02x\n",	AS_TFMD->bSamFreqType);
		for(i=0;i<AS_TFMD->bSamFreqType;i++){
			tSamFreq = (uint32_t)((AS_TFMD->tSamFreq[i][0]) | (AS_TFMD->tSamFreq[i][1] << 8) | (AS_TFMD->tSamFreq[i][2]	<< 16));
			UVC_INFO("tSamFreq(%d):		  0x%08x\n", i+1, tSamFreq);
		}
		UVC_INFO("\r\n");
	}	
}

void uvc_print_info_data(USBH_UVC_INFO_Struct *UVC_Info)
{
	uint8_t i = 0, j = 0, k = 0, g = 0, h = 0;
	uint8_t status = FAIL;
	
	for(i=0;i<USBH_MAX_PORT*2;i++){
		//UVC_INFO("UVC Device ID: %d", i+1);
		//if(UVC_Info->DEV[i].iDevID != 0){
//			UVC_INFO("UVC Device ID: %d", UVC_Info->DEV[i].iDevID);
		
		if(UVC_Info->DEV[i].bIntfNum == 0)
			continue;
		
		status = SUCCESS;
		
			for(j=0;j<UVC_Info->DEV[i].bIntfNum;j++){
				UVC_INFO("  Interface: %d", j+1);
				for(k=0;k<UVC_Info->DEV[i].INTF[j].bFmtNum;k++){
					if(UVC_Info->DEV[i].INTF[j].FMT[k].bFmtType == USBH_UVC_STREAM_H264){ 
						UVC_INFO("    Format: H264");
					}else if(UVC_Info->DEV[i].INTF[j].FMT[k].bFmtType == USBH_UVC_STREAM_MJPEG){
						UVC_INFO("    Format: MJPEG");
					}else if(UVC_Info->DEV[i].INTF[j].FMT[k].bFmtType == USBH_UVC_STREAM_YUV){
						UVC_INFO("    Format: YUV");
					}
					for(g=0;g<UVC_Info->DEV[i].INTF[j].FMT[k].bResNum;g++){
						if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_1280X800){
							UVC_INFO("      Resolution: 1280X800");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_1280X720){
							UVC_INFO("      Resolution: 1280X720");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_960X540){
							UVC_INFO("      Resolution: 960X540");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_800X600){
							UVC_INFO("      Resolution: 800X600");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_848X480){
							UVC_INFO("      Resolution: 848X480");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_640X480){
							UVC_INFO("      Resolution: 640X480");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_640X360){
							UVC_INFO("      Resolution: 640X360");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_424X240){
							UVC_INFO("      Resolution: 424X240");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_320X240){
							UVC_INFO("      Resolution: 320X240");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_320X180){
							UVC_INFO("      Resolution: 320X180");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_160X120){
							UVC_INFO("      Resolution: 160X120");
						}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bPixel == USBH_UVC_STREAM_1920X1080){
							UVC_INFO("      Resolution: 1920X1080");
						}
						for(h=0;h<UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].bFpsNum;h++){
							if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].FPS[h].dwFrameInterval == USBH_UVC_STREAM_5_FPS){
								UVC_INFO("        Frame Rate: 5  fps");	
							}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].FPS[h].dwFrameInterval == USBH_UVC_STREAM_10_FPS){
								UVC_INFO("        Frame Rate: 10 fps");
							}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].FPS[h].dwFrameInterval == USBH_UVC_STREAM_15_FPS){
								UVC_INFO("        Frame Rate: 15 fps");
							}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].FPS[h].dwFrameInterval == USBH_UVC_STREAM_20_FPS){
								UVC_INFO("        Frame Rate: 20 fps");
							}else if(UVC_Info->DEV[i].INTF[j].FMT[k].RES[g].FPS[h].dwFrameInterval == USBH_UVC_STREAM_30_FPS){
								UVC_INFO("        Frame Rate: 30 fps");
							}
						}// end of for(FRAME_RATE_NUM)
					}// end of for(RESOLUTION_NUM)
				}// end	of for(FORMAT_NUM)			
			}// end of for(INTERFACE_NUM)
		//}		
	}// end of for(USBH_MAX_PORT)
	
	if(status == FAIL)
		UVC_INFO("No Device Exist!");
}

uint8_t uvc_xfr(USBH_Device_Structure *UVC_DEV, USBH_UVC_PREVIEW UVC_PREVIEW)
{
	USBH_ISO_XFR_REQ_Struct		*ISO_REQ = NULL;
	USBH_BK_XFR_REQ_Struct		*BULK_REQ = NULL;
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	USBH_BULK_QUEUE_STRUCT		BULK_REQ_QUEUE;
	USBH_BULK_QUEUE_STRUCT		BULK_REQ_QUEUE_DATA;
	
	uint8_t bDevId = 0;
	uint32_t			alignment = 0;
	uint8_t				STATUS = SUCCESS;
	
	memset(&BULK_REQ_QUEUE, 0, sizeof(USBH_BULK_QUEUE_STRUCT));
	memset(&BULK_REQ_QUEUE_DATA, 0, sizeof(USBH_BULK_QUEUE_STRUCT));
	
	//memset(&ISO_REQ,0,sizeof(ISO_REQ));
	//memset(&BULK_REQ, 0, sizeof(BULK_REQ));

	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	if(UVC_DEV->CLASS_DRV == USBH_UVC_ISO_CLASS){
		ISO_REQ = &UVC_ISO_REQ[UVC_PREVIEW.stream_id-1];
		if(UVC_PREVIEW.stream_ctrl == USBH_UVC_PREVIEW_START){ //uvc preview start			
			STATUS = uvc_start_preview(UVC_DEV, ISO_REQ);
			if(STATUS != SUCCESS){
				return STATUS;
			}
			// iso request
			ISO_REQ->XfrType		=	EHCI_ISO_IN_TYPE;
			ISO_REQ->NUM				=	ISO_REQ->IntfNum;

			// do 4k alignment
			alignment			=	(0x1000-(((uint32_t)UVC_PREVIEW.stream_ptr)&0x00000FFF));
			ISO_REQ->RING_BUFF_PTR		=	(uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr)+alignment);
			ISO_REQ->RING_BUFF_SIZE		=	UVC_PREVIEW.stream_size - 0x1000;
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )
#if defined (CONFIG_SNXITD_DISCARD_EN)
			ISO_REQ->SXITD_Discard_En	=	1;
#else
			ISO_REQ->SXITD_Discard_En	=	0;
#endif
#if defined (CONFIG_SNXITD_ErrDISCARD_EN)
			ISO_REQ->SXITD_ErrDiscard_En	=	1;
#else
			ISO_REQ->SXITD_ErrDiscard_En	=	0;
#endif			
			ISO_REQ->SXITD_Header_Filter	=	1;
#else
			ISO_REQ->STD_ISO_PING_PTR	=	(uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr)+alignment+(Standard_iTD_interval*4*3072*8));
			ISO_REQ->STD_ISO_PONG_PTR	=	(uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr)+alignment+(Standard_iTD_interval*4*3072*8)+Max_STD_ISO_FrameSize);
#endif
			ISO_REQ->START				=	1;
			ISO_REQ->STD_ISO_SM		=	1;
			usbh_iso_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_ISO_XFR_REQ_Struct*)ISO_REQ);
		} else if(UVC_PREVIEW.stream_ctrl == USBH_UVC_PREVIEW_STOP){ //uvc preview stop
			ISO_REQ->XfrType	=	EHCI_ISO_IN_TYPE;
			ISO_REQ->NUM			=	uvc_streamid_to_intfnum(UVC_DEV, UVC_PREVIEW.stream_id);
			ISO_REQ->START		=	0;
			STATUS = usbh_iso_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_ISO_XFR_REQ_Struct*)ISO_REQ);
			if(STATUS == SUCCESS){
				STATUS = uvc_stop_preview(UVC_DEV,UVC_PREVIEW.stream_id);
			}
		}
	}// UVC_ISO_CLASS
	else if(UVC_DEV->CLASS_DRV == USBH_UVC_BULK_CLASS){
		if(UVC_PREVIEW.stream_ctrl == USBH_UVC_PREVIEW_START){ //uvc preview start
			UVC_CLASS = (USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
			BULK_REQ = &UVC_BULK_REQ[UVC_PREVIEW.stream_id-1];

			// bulk request
			BULK_REQ->XfrType	=	USBH_BK_IN_TYPE;
			//BULK_REQ->NUM		=	1;
			BULK_REQ->NUM		=	UVC_CLASS->COMMIT.bInftIndex + 1;
			// do 4k allignment
			alignment = (0x1000-(((uint32_t)UVC_PREVIEW.stream_ptr)&0x00000FFF));

#if defined (CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
			BULK_REQ->RING_BUFF_SIZE = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;	
			
			BULK_REQ->RING_BUFF_PTR_W = (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment));
			BULK_REQ->RING_BUFF_PTR_X	= (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment) + 40*1024);
			BULK_REQ->RING_BUFF_PTR_Y	= (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment) + 40*1024*2);
			
			BULK_REQ->BULK_PING_PTR = (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment) + 40*1024*3);
			BULK_REQ->BULK_PONG_PTR = (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment) + 40*1024*3 + USBH_UVC_BK_STREAM_BUF_SIZE);
#else
			BULK_REQ->BULK_PING_PTR = (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment));
			BULK_REQ->BULK_PONG_PTR = (uint32_t*)(((uint32_t)UVC_PREVIEW.stream_ptr+alignment) + USBH_UVC_BK_STREAM_BUF_SIZE);
#endif
			BULK_REQ->SIZE = (UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->COMMIT.bInftIndex].FMT[UVC_CLASS->COMMIT.bFmtIndex].RES[UVC_CLASS->COMMIT.bResIndex].wWidth 
											* UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->COMMIT.bInftIndex].FMT[UVC_CLASS->COMMIT.bFmtIndex].RES[UVC_CLASS->COMMIT.bResIndex].wHeight * 2) / 6; 
			BULK_REQ->BULK_SM = 1;
			BULK_REQ->Start	=	1;
			BULK_REQ->TimeoutMsec = USBH_10000ms;
		
			BULK_REQ_QUEUE.EP = (uint32_t*)&UVC_DEV->EP[BULK_REQ->NUM];
			BULK_REQ_QUEUE.BULK_REQ = (uint32_t*)BULK_REQ;

			memcpy(&BULK_REQ_QUEUE_DATA, &BULK_REQ_QUEUE, sizeof(USBH_BULK_QUEUE_STRUCT));
	
#if defined (CONFIG_USBH_FREE_RTOS)
			xQueueSend(USBH_QUEUE_BULK_REQ_DATA, &BULK_REQ_QUEUE_DATA, 0);

			xQueueSend(USBH_QUEUE_BULK_REQ[UVC_DEV->device_id], &BULK_REQ_QUEUE, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osMessagePut(USBH_QUEUE_BULK_REQ_DATA, &BULK_REQ_QUEUE_DATA, 0);

			osMessagePut(USBH_QUEUE_BULK_REQ[UVC_DEV->device_id], &BULK_REQ_QUEUE, 0);
#endif
		} else if(UVC_PREVIEW.stream_ctrl == USBH_UVC_PREVIEW_STOP){ //uvc preview stop
			BULK_REQ = &UVC_BULK_REQ[UVC_PREVIEW.stream_id-1];
			BULK_REQ->Start		=	0;
			BULK_REQ->BULK_SM 	=	0;
			while(BULK_REQ->XfrDone != 1){} //wait bulk xfr finish.
				STATUS = uvc_stop_preview(UVC_DEV, UVC_PREVIEW.stream_id);
		}
	}// UVC_BULK_CLASS
	return STATUS;
}

uint8_t uvc_register_write(USBH_Device_Structure *DEV, uint16_t reg_addr, uint8_t *reg_value) {

	uint8_t	STATUS = FAIL;
	uint8_t buff[4] = {0};
	USBH_CX_XFR_REQ_Struct cx_req;
	
	memset(&cx_req, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	
	buff[0] = reg_addr;
	buff[1] = reg_addr >> 8;
	buff[2] = *reg_value;
	buff[3] = 0x00;

	cx_req.pBUFF = (uint32_t*) &buff[0];
	cx_req.CMD			= USBH_CX_CMD_CLASS_Write;
	cx_req.SIZE			= 0x04;
	cx_req.CLASS[0] = 0x21;
	cx_req.CLASS[1] = 0x01;
	cx_req.CLASS[2] = 0x00;
	cx_req.CLASS[3] = 0x01;
	cx_req.CLASS[4] = 0x00;
	cx_req.CLASS[5] = 0x03;

	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV, (USBH_CX_XFR_REQ_Struct*)&cx_req);
	
	return STATUS;
	
}

uint8_t uvc_register_read(USBH_Device_Structure *DEV, uint16_t reg_addr, uint8_t *reg_value) {
	
	uint8_t	STATUS = FAIL;
	uint8_t buff[4] = {0};
	USBH_CX_XFR_REQ_Struct cx_req;
	
	memset(&cx_req, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	
	buff[0] = reg_addr;
	buff[1] = reg_addr >> 8;
	buff[2] = 0x00;
	buff[3] = 0xFF;
	
	cx_req.pBUFF = (uint32_t*) &buff[0];
	cx_req.CMD			= USBH_CX_CMD_CLASS_Write;
	cx_req.SIZE			= 0x04;
	cx_req.CLASS[0] = 0x21;
	cx_req.CLASS[1] = 0x01;
	cx_req.CLASS[2] = 0x00;
	cx_req.CLASS[3] = 0x01;
	cx_req.CLASS[4] = 0x00;
	cx_req.CLASS[5] = 0x03;

	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV, (USBH_CX_XFR_REQ_Struct*)&cx_req);
	
	if(STATUS == SUCCESS) {
		
		STATUS = FAIL;
		
		buff[0] = reg_addr;
		buff[1] = reg_addr >> 8;
		buff[2] = *reg_value;
		buff[3] = 0x00;
		
		cx_req.pBUFF		= (uint32_t*) &buff[0];
		cx_req.CMD			= USBH_CX_CMD_CLASS_Read;
		cx_req.SIZE			= 0x04;
		cx_req.CLASS[0] = 0xA1;
		cx_req.CLASS[1] = 0x81;
		cx_req.CLASS[2] = 0x00;
		cx_req.CLASS[3] = 0x01;
		cx_req.CLASS[4] = 0x00;
		cx_req.CLASS[5] = 0x03;

		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV, (USBH_CX_XFR_REQ_Struct*)&cx_req);
		
		*reg_value = buff[2];
	}
	
	return STATUS;
}

uint8_t uvc_still_setting(USBH_Device_Structure *UVC_DEV, uint8_t bFmt, uint32_t width, uint32_t height, uint8_t intf_start_index) {
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	
	UVC_CLASS->STILL_PROBE.bStillTgtFmt			=	bFmt;
	UVC_CLASS->STILL_PROBE.bStillTgtWidth		=	width;
	UVC_CLASS->STILL_PROBE.bStillTgtHeight	=	height;
	UVC_CLASS->STILL_PROBE.bStillIntfNum		=	uvc_get_intfnum(UVC_DEV, UVC_CLASS->STILL_PROBE.bStillTgtFmt, intf_start_index);

	if(UVC_CLASS->CurStreamCnt > 0){
		return FAIL;
	} else {
		return SUCCESS;
	}
}

uint8_t uvc_still_trigger(USBH_Device_Structure *UVC_DEV) {
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	USBH_UVC_STILL_CONTROL		UVC_STILL_PROBE;
	USBH_CX_XFR_REQ_Struct		CX_REQ;
	uint8_t *UVC_DATA_BUFF = NULL;
	uint8_t STATUS = FAIL;
	uint8_t still_trig_sel = 1;

	UVC_CLASS		=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DATA_BUFF	=	(uint8_t*)UVC_DEV->BUFF.ptr;

	if((UVC_DEV == NULL) || (UVC_DATA_BUFF == NULL))
		return STATUS;
	
	memcpy(&UVC_STILL_PROBE, &(UVC_CLASS->STILL_PROBE), 11);

	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Write;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)&UVC_STILL_PROBE;
	CX_REQ.CLASS[0]	=	0x21;
	CX_REQ.CLASS[1]	=	SET_CUR;
	CX_REQ.CLASS[2] =	0x00;
	CX_REQ.CLASS[3] =	VS_STILL_PROBE_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL TRIGGER FAIL\n");
		return STATUS;
	}

	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Read;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)UVC_DATA_BUFF;
	CX_REQ.CLASS[0]	=	0xA1;
	CX_REQ.CLASS[1]	=	GET_CUR;
	CX_REQ.CLASS[2]	=	0x00;
	CX_REQ.CLASS[3]	=	VS_STILL_PROBE_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL TRIGGER FAIL\n");
		return STATUS;
	}
	memcpy(&UVC_STILL_PROBE, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);

	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Write;
	CX_REQ.SIZE		=	11;
	CX_REQ.pBUFF	=	(uint32_t*)&UVC_STILL_PROBE;
	CX_REQ.CLASS[0]	=	0x21;
	CX_REQ.CLASS[1]	=	SET_CUR;
	CX_REQ.CLASS[2] =	0x00;
	CX_REQ.CLASS[3] =	VS_STILL_COMMIT_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL TRIGGER FAIL\n");
		return STATUS;
	}

	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD		=	USBH_CX_CMD_CLASS_Write;
	CX_REQ.SIZE		=	1;
	CX_REQ.pBUFF	=	(uint32_t*)&still_trig_sel;
	CX_REQ.CLASS[0]	=	0x21;
	CX_REQ.CLASS[1]	=	SET_CUR;
	CX_REQ.CLASS[2] =	0x00;
	CX_REQ.CLASS[3] =	VS_STILL_IMAGE_TRIGGER_CONTROL;
	CX_REQ.CLASS[4]	=	UVC_CLASS->STILL_PROBE.bStillIntfNum;
	CX_REQ.CLASS[5]	=	0x00;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)UVC_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		UVC_DBG("STILL TRIGGER FAIL\n");
		return STATUS;
	}

	return STATUS;
}

uint32_t uvc_start(USBH_Device_Structure *UVC_DEV, uint8_t bFmt, uint8_t bRes, uint32_t dwFps, uint32_t *ptr, uint32_t size, uint8_t intf_start_index)
{
	USBH_UVC_CLASS_Structure        *UVC_CLASS = NULL;
	USBH_UVC_PREVIEW                UVC_PREVIEW;
	uint8_t bDevId = 0;
	uint8_t                         STATUS = SUCCESS;
#if defined(CONFIG_MODULE_USB_UVC_REGCTRL)
	uint16_t												reg_addr	= 0;
	uint8_t													reg_value = 0;
#endif

	memset(&UVC_PREVIEW, 0, sizeof(USBH_UVC_PREVIEW));
	
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_CLASS->PROBE.bTgtFmt	=	bFmt;
	UVC_CLASS->PROBE.bTgtRes	=	bRes;
	UVC_CLASS->PROBE.dwTgtFps	=	dwFps;
	UVC_CLASS->PROBE.bIntfNum	=	uvc_get_intfnum(UVC_DEV, UVC_CLASS->PROBE.bTgtFmt, intf_start_index);

	if(UVC_CLASS->CurStreamCnt >= UVC_CLASS->MaxStreamCnt){
		return 0;
	}
	
#if defined(CONFIG_MODULE_USB_UVC_REGCTRL)
#if 0 // test code
	reg_addr  = 0xE7A;
	reg_value = 0x11;
	uvc_register_write(UVC_DEV, reg_addr, &reg_value);
	
	uvc_register_read(UVC_DEV, reg_addr, &reg_value);
#endif
#endif
	
	STATUS = uvc_probe_commit(UVC_DEV, intf_start_index);
	if(STATUS != SUCCESS){
		return 0;
	}
	
	//STATUS = uvc_xfr(UVC_DEV, ptr, size);
	UVC_PREVIEW.stream_size = size;
	UVC_PREVIEW.stream_ptr = ptr;
	UVC_PREVIEW.stream_ctrl = USBH_UVC_PREVIEW_START;
	UVC_PREVIEW.stream_id = UVC_CLASS->PROBE.dwStreamID;

	if(UVC_DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
		switch(UVC_DEV->CFGDes[0].Interface[UVC_CLASS->PROBE.bIntfNum].ENDPDes[0].bED_bmAttributes) {
			case EHCI_BULK:
				UVC_DEV->CLASS_DRV = USBH_UVC_BULK_CLASS;
				break;
			case EHCI_ISO:
				UVC_DEV->CLASS_DRV = USBH_UVC_ISO_CLASS;
				break;
		}
		STATUS = uvc_xfr(UVC_DEV, UVC_PREVIEW);
		UVC_DEV->CLASS_DRV = (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS);
		
	} else {
		STATUS = uvc_xfr(UVC_DEV, UVC_PREVIEW);
	}
	
	if(STATUS != SUCCESS){
		return 0;
	}
			
	UVC_CLASS->CurStreamCnt++;
	UVC_INFO_Struct.DEV[bDevId].bPrv = 1;
	return UVC_CLASS->PROBE.dwStreamID;
}

uint8_t	uvc_stop(USBH_Device_Structure *UVC_DEV, uint8_t stream_id)
{
	USBH_UVC_CLASS_Structure		*UVC_CLASS = NULL;
	USBH_UVC_PREVIEW						UVC_PREVIEW;
	uint8_t											bDevId = 0;
	uint8_t											STATUS = SUCCESS;
	uint8_t											intfnum = uvc_streamid_to_intfnum(UVC_DEV, stream_id);

	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	memset(&UVC_PREVIEW, 0, sizeof(USBH_UVC_PREVIEW));
	
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);

	if(UVC_DEV != NULL){
		//STATUS = uvc_xfr(UVC_DEV, ptr, size, UVC_PREVIEW_STOP);
		UVC_PREVIEW.stream_id = stream_id;
		UVC_PREVIEW.stream_ctrl = USBH_UVC_PREVIEW_STOP;
		
		if(UVC_DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
			switch(UVC_DEV->CFGDes[0].Interface[intfnum].ENDPDes[0].bED_bmAttributes) {
				case EHCI_BULK:
					UVC_DEV->CLASS_DRV = USBH_UVC_BULK_CLASS;
					break;
				case EHCI_ISO:
					UVC_DEV->CLASS_DRV = USBH_UVC_ISO_CLASS;
					break;
			}
			STATUS = uvc_xfr(UVC_DEV, UVC_PREVIEW);
			UVC_DEV->CLASS_DRV = (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS);
		
		} else {
			STATUS = uvc_xfr(UVC_DEV, UVC_PREVIEW);
		}
		if(STATUS != SUCCESS){
			return 0;
		}
	}
	UVC_CLASS->CurStreamCnt--;
	uvc_unregister_streamid(UVC_DEV, stream_id);
	if(UVC_CLASS->CurStreamCnt == 0){
		UVC_INFO_Struct.DEV[bDevId].bPrv = 0;
	}
	return STATUS;
}

uint8_t get_291_h264_gop(USBH_Device_Structure *uvc_dev, uint32_t *gop) {
	
	USBH_CX_XFR_REQ_Struct	CX_REQ;
	uint8_t status = FAIL;
	uint8_t xu_data[11] = {0};
	
	xu_data[0] = 0x9A;
	xu_data[1] = 0x03;
	
	memset((uint8_t*)&CX_REQ, 0, sizeof(USBH_CX_XFR_REQ_Struct));
	CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Write;
	CX_REQ.SIZE			=	11;
	CX_REQ.pBUFF		=	(uint32_t*)&xu_data[0];
	CX_REQ.CLASS[0]	=	0x21;
	CX_REQ.CLASS[1]	=	0x01;
	CX_REQ.CLASS[2]	=	0x00;
	CX_REQ.CLASS[3]	=	0x02;
	CX_REQ.CLASS[4]	=	0x00;
	CX_REQ.CLASS[5]	=	0x04;
	
	status = usbh_cx_xfr((USBH_Device_Structure*)uvc_dev,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(status == FAIL){
		USBH_INFO("h264_gop FAIL\n");
		return status;
	}
	
	memset(&xu_data[0], 0, sizeof(xu_data));
	
	CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
	CX_REQ.SIZE			=	11;
	CX_REQ.pBUFF		=	(uint32_t*)&xu_data[0];
	CX_REQ.CLASS[0]	=	0xA1;
	CX_REQ.CLASS[1]	=	0x81;
	CX_REQ.CLASS[2]	=	0x00;
	CX_REQ.CLASS[3]	=	0x02;
	CX_REQ.CLASS[4]	=	0x00;
	CX_REQ.CLASS[5]	=	0x04;
	
	status = usbh_cx_xfr((USBH_Device_Structure*)uvc_dev,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(status == FAIL){
		USBH_INFO("h264_gop FAIL\n");
		return status;
	}
	
	*gop = (xu_data[1] << 8) | xu_data[0];
	
	return status;
}

uint32_t uvc_get_info(void)
{
	uvc_print_info_data((USBH_UVC_INFO_Struct*)&UVC_INFO_Struct);
	
	return (uint32_t)&UVC_INFO_Struct;
}

uint32_t uvc_ep_to_streamid(USBH_Device_Structure *UVC_DEV, EHCI_ENDP_STRUCT *EP)
{
	uint8_t bDevId = 0;
	uint32_t i = 0, j = 0, k = 0, l = 0;

	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
		if((uint32_t)EP	== (uint32_t)UVC_INFO_Struct.DEV[bDevId].INTF[i].EP){
			for(j=0;j<UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;j++){
				for(k=0;k<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bResNum;k++){
					for(l=0;l<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFpsNum;l++){
						if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[l].dwStreamID != 0){
							return UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[l].dwStreamID;
						}
					}
				}
			}
		}
	}
	return 0;
}

EHCI_ENDP_STRUCT* uvc_streamid_to_ep(uint32_t streamID)
{
	uint32_t i = 0, j = 0, k = 0, l = 0, h = 0;

	for(i=0; i<USBH_MAX_PORT*2; i++){
		for(j=0; j<UVC_INFO_Struct.DEV[i].bIntfNum;j++){
			for(k=0;k<UVC_INFO_Struct.DEV[i].INTF[j].bFmtNum;k++){
				for(l=0;l<UVC_INFO_Struct.DEV[i].INTF[j].FMT[k].bResNum;l++){
					for(h=0;h<UVC_INFO_Struct.DEV[i].INTF[j].FMT[k].RES[l].bFpsNum;h++){
						if(UVC_INFO_Struct.DEV[i].INTF[j].FMT[k].RES[l].FPS[h].dwStreamID == streamID){
							return UVC_INFO_Struct.DEV[i].INTF[j].EP;
						}
					}
				}
			}
		}
	}
	
	return NULL;
}

uint8_t	uvc_set_streamid(USBH_Device_Structure *UVC_DEV)
{
	USBH_UVC_CLASS_Structure	*UVC_CLASS = NULL;
	uint8_t i =	0, j = 0, k = 0, g = 0, bDevId = 0;
	uint8_t STATUS = SUCCESS;

	UVC_CLASS =	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;

	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	i =	UVC_CLASS->COMMIT.bInftIndex;
	j =	UVC_CLASS->COMMIT.bFmtIndex;
	k =	UVC_CLASS->COMMIT.bResIndex;
	g =	UVC_CLASS->COMMIT.bFpsIndex;

	UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID = uvc_get_streamid(UVC_DEV);
	UVC_CLASS->PROBE.dwStreamID = UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID;  
	
	return STATUS;
}

uint32_t uvc_get_streamid(USBH_Device_Structure *UVC_DEV)
{
	uint8_t i =	0;

	for(i = 0; i < UVC_STREAMID_NUM_MAX; i++){
		if(gStream_ID[i] == 0){
			gStream_ID[i] = i+1;
			return i+1;
		}
	}

	return 0;
}

uint8_t	uvc_streamid_to_intfnum(USBH_Device_Structure *UVC_DEV, uint32_t streamID)
{
	uint8_t i =	0, j = 0, k = 0, l	= 0;
	uint8_t bDevId = 0;
	
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);

	for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
		for(j=0;j<UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;j++){
			for(k=0;k<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bResNum;k++){
				for(l=0;l<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFpsNum;l++){
					if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[l].dwStreamID == streamID){
						return i+1;
					}
				}// end	of for(FpsNum)
			}//	end of for(ResNum)
		}// end	of for(FmtNum)
	}//	end of for(IntfNum)
	return 0;
}

uint8_t uvc_streamid_to_devid(uint8_t stream_id)
{
	uint32_t i = 0, j = 0, k = 0, l = 0, h = 0;
	
	for(i=0; i<USBH_MAX_PORT*2;i++){
		for(j=0; j<UVC_INFO_Struct.DEV[i].bIntfNum;j++){
			for(k=0;k<UVC_INFO_Struct.DEV[i].INTF[j].bFmtNum;k++){
				for(l=0;l<UVC_INFO_Struct.DEV[i].INTF[j].FMT[k].bResNum;l++){
					for(h=0;h<UVC_INFO_Struct.DEV[i].INTF[j].FMT[k].RES[l].bFpsNum;h++){
						if(UVC_INFO_Struct.DEV[i].INTF[j].FMT[k].RES[l].FPS[h].dwStreamID == stream_id){
							return UVC_INFO_Struct.DEV[i].iDevID;
						}
					}// end of for(FpsNum)
				} // end of for(ResNum)
			}// end of for(FmtNum)
		}// end of for(IntfNum)
	}// end of for(DevCnt)
	return 0xFF;
}

void uvc_unregister_streamid(USBH_Device_Structure *UVC_DEV, uint32_t dwStreamID)
{
	uint8_t i = 0, j = 0, k = 0, g = 0, h = 0;
	uint8_t bDevId = 0;
	
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);

	if(dwStreamID == CLEAR_ALL_STREAM_ID){
		for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
			for(j=0;j<UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;j++){
				for(k=0;k<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bResNum;k++){
					for(g=0;g<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFpsNum;g++){
						if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID != 0){
							for(h=0;h<UVC_STREAMID_NUM_MAX;h++){
								if(gStream_ID[h] == UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID){
									gStream_ID[h] = 0;
								}                                                                                
							}
							UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID = 0;
						}
					}// end of for(bFpsNum)
				}// end of for(bResNum)
			}// end of for(bFmtNum)
		}// end of for(bIntf)
	} else{
			for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].bIntfNum; i++){
				for(j=0;j<UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;j++){
					for(k=0;k<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bResNum;k++){
						for(g=0;g<UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].bFpsNum;g++){
							if(UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID == dwStreamID){
								UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].RES[k].FPS[g].dwStreamID = 0;
								i = UVC_INFO_Struct.DEV[bDevId].bIntfNum;
								j = UVC_INFO_Struct.DEV[bDevId].INTF[i].bFmtNum;
								k = UVC_INFO_Struct.DEV[bDevId].INTF[i].FMT[j].bFormatIndex;
								gStream_ID[dwStreamID-1] = 0;
								break;
							}
						}// end of for(bFpsNum)
					}// end of for(bResNum)
				}// end of for(bFmtNum)
			}// end of for(bIntf)
		}
	// gStream_ID[dwStreamID - 1] = 0;
}

void uvc_enum(USBH_Device_Structure *UVC_DEV)
{
	uint8_t i = 0, dev_idx = 0;
	
	for(i = 0; i < USBH_MAX_PORT*2; i++) {
		if(UVC_INFO_Struct.DEV[i].INTF[0].bFmtNum == 0) {
			memset((uint8_t*)&UVC_INFO_Struct.DEV[i], 0, sizeof(USBH_UVC_DEV_Struct));
			dev_idx = i;
			break;
		}
	}
	if (i == (USBH_MAX_PORT*2)) {
		UVC_DBG("UVC_INFO_Struct.DEV is full");
		return;
	}
	
	//memset((uint8_t*)&UVC_INFO_Struct.DEV[UVC_DEV->device_id], 0, sizeof(USBH_UVC_DEV_Struct));	
	UVC_INFO_Struct.bCurDevCnt++;
	uvc_parser_des(UVC_DEV, dev_idx);

	UVC_DBG("WAKE UP UVC CLASS APP  !!");
#if defined (CONFIG_USBH_FREE_RTOS)
	if(usbh_queue_uvc_app != NULL) {
		xQueueSend(usbh_queue_uvc_app, (void *)&UVC_DEV->device_id, 0);	
	}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if(usbh_queue_uvc_app != NULL) {
		osMessagePut(usbh_queue_uvc_app, (void *)&UVC_DEV->device_id, 0);
	}
#endif
}

void uvc_stream_complete(USBH_UVC_MW_STRUCTURE	*usbh_uvc_mw){
	EHCI_ENDP_STRUCT *EP = NULL;
	
	EP = uvc_streamid_to_ep(usbh_uvc_mw->stream_id);
	if(EP == NULL) return;
	
	EP->pSXITD->FW_FRAME_END	=	((uint32_t)usbh_uvc_mw->ptr + usbh_uvc_mw->size);
}

#if 0
void	uvc_xact_err_handle(USBH_UVC_MW_STRUCTURE *stream) {
	
	EHCI_EC_STS		cur_EC_STS;
	cur_EC_STS		= EHCI->EC_STS;
	EHCI->EC_STS	= EHCI->EC_STS;
	
	if(cur_EC_STS.TIMEOUT == 1) {
		stream->xact_err_cat.TIMEOUT = 1;
	}
	
	if(cur_EC_STS.RXERR == 1) {
		stream->xact_err_cat.RXERR = 1;
	}
	
	if(cur_EC_STS.PID_ERR == 1) {
		stream->xact_err_cat.PID_ERR = 1;
	}
	
	if(cur_EC_STS.UVC_H_ERR == 1) {
		stream->xact_err_cat.UVC_H_ERR = 1;
	}
	
	if(cur_EC_STS.CRC16_ERR == 1) {
		stream->xact_err_cat.CRC16_ERR = 1;
	}
}
#endif

#if defined(	CONFIG_SNX_ISO_ACCELERATOR )
// ISR Call function 
void uvc_process_data(USBH_Device_Structure *UVC_DEV, EHCI_ENDP_STRUCT *EP){
#if defined (CONFIG_USBH_FREE_RTOS)
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif
	
	uint32_t offset = 0, residue = 0;
	
	EP->LastFID++;
	EP->LastFID	= EP->LastFID%4;
	if(EP->LastFID == EP->pSXITD->bFID_Count){
		residue	= EP->LastFwFrameEnd%0x10;
		if(residue <=4){
			offset = 0x04-residue;
		}else{
			offset = 0x14-residue;
		}
		
		if(EP->pSXITD->HW_FRAME_END < (EP->LastFwFrameEnd+offset)) {
			UVC_DATA.stream[UVC_DATA.index].ptr		=	(uint32_t *)EP->pSXITD->HW_FRAME_END;
			UVC_DATA.stream[UVC_DATA.index].size			=	0;
		} else {
			UVC_DATA.stream[UVC_DATA.index].ptr		=	(uint32_t *)(EP->LastFwFrameEnd+offset);
			UVC_DATA.stream[UVC_DATA.index].size	=	EP->pSXITD->HW_FRAME_END - EP->LastFwFrameEnd -	offset;
		}
		
		UVC_DATA.stream[UVC_DATA.index].dev_id				=	UVC_DEV->device_id;
		UVC_DATA.stream[UVC_DATA.index].stream_id			=	uvc_ep_to_streamid(UVC_DEV,EP);
		//UVC_DATA.stream[UVC_DATA.index].ring_buff_end			=	(EP->pSXITD->bRING_BUFFER_END<<12);
		//UVC_DATA.stream[UVC_DATA.index].ring_buff_start			=	(EP->pSXITD->bRING_BUFFER_START<<12);
	}else{
		// can't handle	FID count >= 2 , HW limit ... just do FW recovery
		UVC_DATA.stream[UVC_DATA.index].ptr							= (uint32_t *)EP->pSXITD->HW_FRAME_END;
		UVC_DATA.stream[UVC_DATA.index].size						= 0;
		UVC_DATA.stream[UVC_DATA.index].dev_id					=	UVC_DEV->device_id;
		UVC_DATA.stream[UVC_DATA.index].stream_id				=	uvc_ep_to_streamid(UVC_DEV,EP);
		//UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
		//UVC_DATA.stream[UVC_DATA.index].ring_buff_start	=	0;		               
	}
		
#if defined (CONFIG_PLATFORM_ST53510)
	if(UVC_DATA.stream[(UVC_DATA.index+USBH_UVC_DATA_QUEUE_SIZE-1)%(USBH_UVC_DATA_QUEUE_SIZE)].err) {
		UVC_DATA.stream[UVC_DATA.index].ptr							= (uint32_t *)EP->pSXITD->HW_FRAME_END;
		UVC_DATA.stream[UVC_DATA.index].size						= 0;
		UVC_DATA.stream[UVC_DATA.index].dev_id					=	UVC_DEV->device_id;
		UVC_DATA.stream[UVC_DATA.index].stream_id				=	uvc_ep_to_streamid(UVC_DEV,EP);
		UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
		UVC_DATA.stream[UVC_DATA.index].ring_buff_start	=	0;		               
	}
#endif	
	
	EP->LastFID	= EP->pSXITD->bFID_Count;
	if(EP->pSXITD->HW_FRAME_END > EP->pSXITD->bRING_THRESHOLD<<12){
		EP->LastFwFrameEnd					=	EP->pSXITD->bRING_BUFFER_START<<12;	
	}else{
		EP->LastFwFrameEnd					=	EP->pSXITD->HW_FRAME_END;
	}
	
	UVC_DATA.stream[UVC_DATA.index].stream_xfr_type = USBH_ISO_IN_TYPE;

	if(usbh_queue_uvc_mw != NULL){
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueSendToBackFromISR(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], &xHigherPriorityTaskWoken);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
		UVC_DATA.index ++;
		UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);
		
		UVC_DATA.stream[UVC_DATA.index].xact_err = 0;
		UVC_DATA.stream[UVC_DATA.index].babble = 0;
		UVC_DATA.stream[UVC_DATA.index].underflow = 0;
		UVC_DATA.stream[UVC_DATA.index].discard = 0;
#if defined (CONFIG_PLATFORM_SN7320)
		UVC_DATA.stream[UVC_DATA.index].errdiscard = 0;
#endif
	}
}

// ISR Call function 
void uvc_process_err_data(USBH_Device_Structure *UVC_DEV, EHCI_ENDP_STRUCT *EP){
//    uint32_t i = 0, j = 0;
    
#if defined (CONFIG_USBH_FREE_RTOS)
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif
	
	if(EP->pSXITD->bTransaction_Error) {
		EP->pSXITD->bTransaction_Error = 0;
		EP->pSXITD->bACTIVE = 1;

		// next eco workaround
//		for(i = 0; i<0x3000; i++);
//		while(EP->pSXITD->bTransaction_Error == 1) {
//			EP->pSXITD->bTransaction_Error = 0;
//			EP->pSXITD->bACTIVE = 1;
//			for(i = 0; i<0x3000; i++);
//		}
        
		UVC_DATA.stream[UVC_DATA.index].xact_err = 1;
		//uvc_xact_err_handle(&UVC_DATA.stream[UVC_DATA.index]);
	}
	
	if(EP->pSXITD->bBabble_Det){
		EP->pSXITD->bBabble_Det = 0;
		EP->pSXITD->bACTIVE = 1;
		
//		for(i = 0; i<0x3000; i++);
//		while(EP->pSXITD->bBabble_Det == 1) {
//			EP->pSXITD->bBabble_Det = 0;
//			EP->pSXITD->bACTIVE = 1;
//			for(i = 0; i<0x3000; i++);
//		}
		
		UVC_DATA.stream[UVC_DATA.index].babble = 1;
	}
	
	if(EP->pSXITD->bUNDERFLOW){
		//EP->pSXITD->bUNDERFLOW = 0;
		UVC_DATA.stream[UVC_DATA.index].underflow	= 1;	               
	}
    
	if(EP->pSXITD->bDATA_DISCARD){
		//EP->pSXITD->bDATA_DISCARD = 0;
		UVC_DATA.stream[UVC_DATA.index].discard 	= 1;		
	}
	
	if(EP->pSXITD->bErr_DISCARD){
		//EP->pSXITD->bErr_DISCARD = 0;
		UVC_DATA.stream[UVC_DATA.index].errdiscard 	= 1;		
	}	

	UVC_DATA.stream[UVC_DATA.index].ptr 						= (uint32_t *)EP->pSXITD->HW_FRAME_END;		
	UVC_DATA.stream[UVC_DATA.index].size						=	0;	
	UVC_DATA.stream[UVC_DATA.index].dev_id					=	UVC_DEV->device_id;
	UVC_DATA.stream[UVC_DATA.index].stream_id				=	uvc_ep_to_streamid(UVC_DEV,EP);
	//UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
	//UVC_DATA.stream[UVC_DATA.index].ring_buff_start	=	0;
	
	UVC_DATA.stream[UVC_DATA.index].stream_xfr_type = USBH_ISO_IN_TYPE;

	if(usbh_queue_uvc_mw != NULL){
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueSendToBackFromISR(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], &xHigherPriorityTaskWoken);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
		UVC_DATA.index ++;
		UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);
		
		UVC_DATA.stream[UVC_DATA.index].xact_err = 0;
		UVC_DATA.stream[UVC_DATA.index].babble = 0;
		UVC_DATA.stream[UVC_DATA.index].underflow = 0;
		UVC_DATA.stream[UVC_DATA.index].discard = 0;
		UVC_DATA.stream[UVC_DATA.index].errdiscard = 0;
	}
}

#else	
void uvc_process_data(USBH_Device_Structure *UVC_DEV, EHCI_ENDP_STRUCT *EP)
{
#if defined (CONFIG_USBH_FREE_RTOS)
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif

	if(EP->LAST_TOG){
		if(EP->PING_FRAME_DONE){
			UVC_DATA.stream[UVC_DATA.index].ptr			=	(uint32_t*)(((uint32_t)EP->PING_FRAME_PTR)-EP->PING_FRAME_SIZE);
			UVC_DATA.stream[UVC_DATA.index].size			=	EP->PING_FRAME_SIZE;
			UVC_DATA.stream[UVC_DATA.index].dev_id			=	UVC_DEV->device_id;
			UVC_DATA.stream[UVC_DATA.index].stream_id		=	uvc_ep_to_streamid(UVC_DEV,EP);
			UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
			UVC_DATA.stream[UVC_DATA.index].ring_buff_start		=	0;			
			
			UVC_DATA.stream[UVC_DATA.index].stream_xfr_type = USBH_ISO_IN_TYPE;
			
			if(usbh_queue_uvc_mw != NULL){
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSendToBackFromISR(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], &xHigherPriorityTaskWoken);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
				UVC_DATA.index ++;
				UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);				
			}
		}			
	}else{
		if(EP->PONG_FRAME_DONE){
			UVC_DATA.stream[UVC_DATA.index].ptr			=	(uint32_t*)(((uint32_t)EP->PONG_FRAME_PTR)-EP->PONG_FRAME_SIZE);
			UVC_DATA.stream[UVC_DATA.index].size			=	EP->PONG_FRAME_SIZE;
			UVC_DATA.stream[UVC_DATA.index].dev_id			=	UVC_DEV->device_id;
			UVC_DATA.stream[UVC_DATA.index].stream_id		=	uvc_ep_to_streamid(UVC_DEV,EP);
			UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
			UVC_DATA.stream[UVC_DATA.index].ring_buff_start		=	0;			

			UVC_DATA.stream[UVC_DATA.index].stream_xfr_type = USBH_ISO_IN_TYPE;

			if(usbh_queue_uvc_mw != NULL){			
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSendToBackFromISR(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], &xHigherPriorityTaskWoken);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
				UVC_DATA.index ++;
				UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);				
			}
		}	
	}
}
#endif 

void uvc_process_data_bulk(USBH_Device_Structure *UVC_DEV, EHCI_ENDP_STRUCT *EP)
{
	if(EP->LAST_TOG){
		if(EP->PING_FRAME_DONE){
			UVC_DATA.stream[UVC_DATA.index].ptr			=	(uint32_t*)(((uint32_t)EP->PING_FRAME_PTR)-EP->PING_FRAME_SIZE);
			UVC_DATA.stream[UVC_DATA.index].size			=	EP->PING_FRAME_SIZE;
			UVC_DATA.stream[UVC_DATA.index].dev_id			=	UVC_DEV->device_id;
			UVC_DATA.stream[UVC_DATA.index].stream_id		=	uvc_ep_to_streamid(UVC_DEV, EP);
			UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
			UVC_DATA.stream[UVC_DATA.index].ring_buff_start		=	0;
			
			UVC_DATA.stream[UVC_DATA.index].stream_xfr_type = USBH_BK_IN_TYPE;
			
			if(usbh_queue_uvc_mw != NULL){			
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSend(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
				UVC_DATA.index ++;
				UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);
			}
		}			
	}else{
		if(EP->PONG_FRAME_DONE){
			UVC_DATA.stream[UVC_DATA.index].ptr			=	(uint32_t*)(((uint32_t)EP->PONG_FRAME_PTR)-EP->PONG_FRAME_SIZE);
			UVC_DATA.stream[UVC_DATA.index].size			=	EP->PONG_FRAME_SIZE;
			UVC_DATA.stream[UVC_DATA.index].dev_id			=	UVC_DEV->device_id;
			UVC_DATA.stream[UVC_DATA.index].stream_id		=	uvc_ep_to_streamid(UVC_DEV, EP);
			UVC_DATA.stream[UVC_DATA.index].ring_buff_end		=	0;
			UVC_DATA.stream[UVC_DATA.index].ring_buff_start		=	0;			
			
			UVC_DATA.stream[UVC_DATA.index].stream_xfr_type = USBH_BK_IN_TYPE;
			
			if(usbh_queue_uvc_mw != NULL){			
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSend(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
				UVC_DATA.index ++;
				UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);				
			}
		}	
	}
}

void uvc_free_mem(USBH_Device_Structure *UVC_DEV) {
	
	uint8_t i = 0, bDevId = 0;
	USBH_UVC_CLASS_Structure			*UVC_CLASS	=	NULL;
	USBH_UVC_DESCRIPTOR_Structure			*UVC_DES	=	NULL;
	USBH_UVC_YUV_Format_Descriptor_Struct		*YUV_FORMAT_Des	=	NULL;	
	USBH_UVC_YUV_Frame_Descriptor_Struct		*YUV_FRAME_Des	= 	NULL;
	USBH_UVC_CS_Format_Descriptor_Struct		*VS_FORMAT_Des	=	NULL;
	USBH_UVC_CS_Frame_Descriptor_Struct		*VS_FRAME_Des	=	NULL;
	USBH_UVC_Frame_Based_Format_Descriptor_Struct	*FB_FORMAT_Des	=	NULL;
	USBH_UVC_Frame_Based_Frame_Descriptor_Struct	*FB_FRAME_Des	= 	NULL;
	
	UVC_CLASS	=	(USBH_UVC_CLASS_Structure*)UVC_DEV->CLASS_STRUCT_PTR;
	UVC_DES		=	(USBH_UVC_DESCRIPTOR_Structure*)(&UVC_CLASS->DES);
	
	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	if(UVC_DES->VC_Interface.baInterfaceNr != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
		vPortFree(UVC_DES->VC_Interface.baInterfaceNr);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		free(UVC_DES->VC_Interface.baInterfaceNr);
#endif
		UVC_DES->VC_Interface.baInterfaceNr = NULL;
	}
	if(UVC_DES->VC_PU_Des.bmControls != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
		vPortFree(UVC_DES->VC_PU_Des.bmControls);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		free(UVC_DES->VC_PU_Des.bmControls);
#endif
		UVC_DES->VC_PU_Des.bmControls = NULL;
	}
	for(i = 0; i < MAX_DES_NUM;	i++){
		if(UVC_DES->VC_XU_Des[i].bLength == 0x00){
			if(UVC_DES->VC_XU_Des[i].bmControls != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(UVC_DES->VC_XU_Des[i].bmControls);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				free(UVC_DES->VC_XU_Des[i].bmControls);
#endif
				UVC_DES->VC_XU_Des[i].bmControls = NULL;
			}
		}
	}
	for(i = 0;i	< UVC_INTERFACE_NUM_MAX; i++){
		if(UVC_DES->VS_Interface[i].bLength == 0x00){
			if(UVC_DES->VS_Interface[i].bmaControls != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(UVC_DES->VS_Interface[i].bmaControls);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				free(UVC_DES->VS_Interface[i].bmaControls);
#endif
				UVC_DES->VS_Interface[i].bmaControls = NULL;
			}
		}
	}
	YUV_FORMAT_Des = &(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].YUV_FORMAT_Des);
	for(i = 0; i < YUV_FORMAT_Des->bNumFrameDescriptors; i++){
		YUV_FRAME_Des =	&(YUV_FORMAT_Des->YUV_FRAME_Des[i]);
		if(YUV_FRAME_Des->bLength == 0x00){
			if(YUV_FRAME_Des->dwFrameInterval != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(YUV_FRAME_Des->dwFrameInterval);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				free(YUV_FRAME_Des->dwFrameInterval);
#endif
				YUV_FRAME_Des->dwFrameInterval = NULL;
			}
		}
	}
	VS_FORMAT_Des = &(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].VS_FORMAT_Des);
	for(i = 0; i < VS_FORMAT_Des->bNumFrameDescriptors;	i++){
		VS_FRAME_Des = &(VS_FORMAT_Des->VS_FRAME_Des[i]);
		if(VS_FRAME_Des->bLength == 0x00){
			if(VS_FRAME_Des->dwFrameInterval != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(VS_FRAME_Des->dwFrameInterval);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				free(VS_FRAME_Des->dwFrameInterval);
#endif
				VS_FRAME_Des->dwFrameInterval = NULL;
			}
		}
	}
	FB_FORMAT_Des = &(UVC_DES->VS_Interface[UVC_CLASS->CurIntfNum-1].FB_FORMAT_Des);
	for(i = 0; i < FB_FORMAT_Des->bNumFrameDescriptors;	i++){
		FB_FRAME_Des = &(FB_FORMAT_Des->FB_FRAME_Des[i]);
		if(FB_FRAME_Des->bLength == 0x00){
			if(FB_FRAME_Des->dwFrameInterval != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(FB_FRAME_Des->dwFrameInterval);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				free(FB_FRAME_Des->dwFrameInterval);
#endif
				FB_FRAME_Des->dwFrameInterval = NULL;
			}
		}
	}

	for(i = 0; i < UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].bFmtNum; i++){
#if defined (CONFIG_USBH_FREE_RTOS)
		if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth != NULL) {
			vPortFree(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth);
			UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth = NULL;
		}
		if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight != NULL) {
			vPortFree(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight);
			UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight = NULL;
		}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth != NULL) {
			free(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth);
			UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wWidth = NULL;
		}
		if(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight != NULL) {
			free(UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight);
			UVC_INFO_Struct.DEV[bDevId].INTF[UVC_CLASS->CurIntfNum-1].FMT[i].STILL_FRAME_Des.wHeight = NULL;
		}
#endif
	}
}

#if defined (CONFIG_USBH_FREE_RTOS)
void uvc_task_init(uint8_t device_id)
{
  memset(&usbh_uvc_app.dev[device_id], 0, sizeof(USBH_UVC_APP_DEV_STRUCTURE));
	
	char task_name[256] = {'\0'};
	
	if(usbh_queue_uvc_drv == NULL){
		usbh_queue_uvc_drv	=	xQueueCreate(1,4);
	}

	sprintf((char *)&task_name, "%s%d", "USBH_UVC_TASK", device_id);
	
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		5120,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_UVC_DRV[device_id]
	);
#endif  

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		5120,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_UVC_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		10240,
		(void*)&device_id,
		250,
		&xTASK_HDL_UVC_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		8192,
		(void*)&device_id,
		250,
		&xTASK_HDL_UVC_DRV[device_id]
	);		
#endif
		
#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_SN7320 )
#if defined (CONFIG_MODULE_USB_UVC_SRAM)
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		4096,
		(void*)&device_id,
		3,
		&xTASK_HDL_UVC_DRV[device_id]
	);	
#else
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		8192,
		(void*)&device_id,
		3,
		&xTASK_HDL_UVC_DRV[device_id]
	);	
#endif
#endif		

#if defined( CONFIG_XILINX_SDK )	
	xTaskCreate(
		uvc_task,
		( const char * )&task_name[0],
		10240,
		(void*)&device_id,
		110,
		&xTASK_HDL_UVC_DRV[device_id]
	);
#endif
}

void uvc_task_uninit(uint8_t device_id)
{
	USBH_Device_Structure		*UVC_DEV = NULL;
	uint8_t bDevId = 0;
	
#if defined (CONFIG_USBH_CLI_UVC) || defined (CONFIG_NONCLI_APP)
	uint8_t i = 0;
#endif

	UVC_DEV	= (USBH_Device_Structure*)usbh_uvc_init(device_id);
	
	uvc_free_mem(UVC_DEV);
	
  uvc_unregister_streamid(UVC_DEV, CLEAR_ALL_STREAM_ID);

	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	if(xTASK_HDL_UVC_DRV[device_id] != NULL){
		vTaskDelete(xTASK_HDL_UVC_DRV[device_id]);		
		xTASK_HDL_UVC_DRV[device_id] = NULL;
	}
        
	UVC_INFO_Struct.bCurDevCnt--;
	memset((void *)&UVC_INFO_Struct.DEV[bDevId], 0, sizeof(USBH_UVC_DEV_Struct));
	
#if defined (CONFIG_USBH_CLI_UVC) || defined (CONFIG_NONCLI_APP)
	for(i=0;i<max_stream_count;i++){
		if(usbh_uvc_app.dev[device_id].stream[i].ptr != NULL) {
			UVC_DBG(" vPortFree usbh_uvc_app.dev[%d].stream[%d].ptr = %x", device_id, i, (uint32_t)usbh_uvc_app.dev[device_id].stream[i].ptr);
			vPortFree(usbh_uvc_app.dev[device_id].stream[i].ptr);
			usbh_uvc_app.dev[device_id].stream[i].ptr = NULL;
		}
	}
	
	memset((void *)&usbh_uvc_app.dev[device_id], 0, sizeof(USBH_UVC_APP_DEV_STRUCTURE));
#endif
	
	UVC_DBG("UVC DRV TASK DESTORY !");	
	
	if(UVC_INFO_Struct.bCurDevCnt == 0){
		if(xTASK_HDL_UVC_APP != NULL) {
			vTaskDelete(xTASK_HDL_UVC_APP);
			xTASK_HDL_UVC_APP = NULL;
		}
		if(xTASK_HDL_UVC_STREAM_XFR != NULL) {
			vTaskDelete(xTASK_HDL_UVC_STREAM_XFR);
			xTASK_HDL_UVC_STREAM_XFR = NULL;
		}
		if(usbh_queue_uvc_app != NULL) {
			vQueueDelete(usbh_queue_uvc_app);
			usbh_queue_uvc_app = NULL;
		}
		if(usbh_queue_uvc_mw != NULL) {
			vQueueDelete(usbh_queue_uvc_mw);
			usbh_queue_uvc_mw = NULL;
		}
		if(usbh_queue_uvc_drv != NULL) {
			vQueueDelete(usbh_queue_uvc_drv);
			usbh_queue_uvc_drv = NULL;
		}
				
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
		if(bk_ping_mutex != NULL) {
			vSemaphoreDelete(bk_ping_mutex);
			bk_ping_mutex = NULL;
		}

		if(bk_pong_mutex != NULL) {
			vSemaphoreDelete(bk_pong_mutex);
			bk_pong_mutex = NULL;
		}
		ping_empty = 1;
		pong_empty = 1;
		
		if(bk_wait_compose_sem != NULL) {
			vSemaphoreDelete(bk_wait_compose_sem);
			bk_wait_compose_sem = NULL;
		}

		if(bk_wait_compose_mutex != NULL) {
			vSemaphoreDelete(bk_wait_compose_mutex);
			bk_wait_compose_mutex = NULL;
		}

		bk_wait_compose_count = 0;

		clean_bk_buf_idx = 0;

		if(bk_packet_err_mutex != NULL) {
			vSemaphoreDelete(bk_packet_err_mutex);
			bk_packet_err_mutex = NULL;
		}
		
		bk_discarded_frame_cnt = 0;
		bk_packet_err = 0;

		if(xTASK_HDL_UVC_BULK_DATA != NULL) {
			vTaskDelete(xTASK_HDL_UVC_BULK_DATA);
			xTASK_HDL_UVC_BULK_DATA = NULL;
		}
		
		if(usbh_queue_uvc_bulk_data != NULL) {
			vQueueDelete(usbh_queue_uvc_bulk_data);
			usbh_queue_uvc_bulk_data = NULL;
		}	
#endif

#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
		if(xTASK_HDL_UVC_XFR_CTRL != NULL) {
			vTaskDelete(xTASK_HDL_UVC_XFR_CTRL);
			xTASK_HDL_UVC_XFR_CTRL = NULL;
		}
		
		if(usbh_queue_uvc_xfr_ctrl != NULL) {
			vQueueDelete(usbh_queue_uvc_xfr_ctrl);
			usbh_queue_uvc_xfr_ctrl = NULL;
		}
#endif
		
#if defined (CONFIG_USBH_CLI_UVC) || defined (CONFIG_NONCLI_APP)		
		memset((void *)&usbh_uvc_app, 0, sizeof(USBH_UVC_APP_STRUCTURE));
#endif
	}
#if defined( CONFIG_SNX_ISO_ACCELERATOR )	

#else	
	if(USBH_QUEUE_STD_ISO_REQ != NULL){
		vQueueDelete(USBH_QUEUE_STD_ISO_REQ);	
		USBH_QUEUE_STD_ISO_REQ = NULL;		
	}	
#endif
	if(USBH_QUEUE_BULK_REQ[device_id] != NULL){
		vQueueDelete(USBH_QUEUE_BULK_REQ[device_id]);
		USBH_QUEUE_BULK_REQ[device_id] = NULL;
	}

	if(USBH_QUEUE_BULK_REQ_DATA != NULL){
		vQueueDelete(USBH_QUEUE_BULK_REQ_DATA);
		USBH_QUEUE_BULK_REQ_DATA = NULL;
	}

	UVC_DBG("UVC TASK DESTORY !");	
}

#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void uvc_task_init(uint8_t device_id)
{
	char task_name[256] = {'\0'};
	
	if(usbh_queue_uvc_drv == NULL){
		osMessageQDef(usbh_queue_uvc_drv, USBH_UVC_DATA_QUEUE_SIZE, uint32_t);
		usbh_queue_uvc_drv = osMessageCreate(osMessageQ(usbh_queue_uvc_drv), NULL);
	}

	sprintf((char *)&task_name, "%s%d", "USBH_UVC_TASK", device_id);
	
#if defined( CONFIG_SN_GCC_SDK ) && defined (CONFIG_PLATFORM_SN9866X)
	osThreadDef(task_name, uvc_task, THREAD_PRIO_USBH_PROCESS, 1, 10240);
	xTASK_HDL_UVC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_UVC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_UVC_DRV[%d] fail\n", device_id);
#endif  

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(task_name, uvc_task, THREAD_PRIO_USBH_PROCESS, 1, 10240);
	xTASK_HDL_UVC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_UVC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_UVC_DRV[%d] fail\n", device_id);
#endif  


#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_ST53510 )
	osThreadDef(task_name, uvc_task, THREAD_PRIO_USBH_PROCESS, 1, 10240);
	xTASK_HDL_UVC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_UVC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_UVC_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_SN7300 )
	osThreadDef(task_name, uvc_task, THREAD_PRIO_USBH_PROCESS, 1, 8192);
	xTASK_HDL_UVC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_UVC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_UVC_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_XILINX_SDK )	
	osThreadDef(task_name, uvc_task, THREAD_PRIO_USBH_PROCESS, 1, 10240);
	xTASK_HDL_UVC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_UVC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_UVC_DRV[%d] fail\n", device_id);
#endif  
}

void uvc_task_uninit(uint8_t device_id)
{
	USBH_Device_Structure		*UVC_DEV = NULL;
	uint8_t bDevId = 0;

#if defined (CONFIG_USBH_CLI_UVC) || defined (CONFIG_NONCLI_APP)
	uint8_t i = 0;
#endif

	UVC_DEV	= (USBH_Device_Structure*)usbh_uvc_init(device_id);

	uvc_free_mem(UVC_DEV);
	
  uvc_unregister_streamid(UVC_DEV, CLEAR_ALL_STREAM_ID);

	bDevId = uvc_get_devIDX_in_UVCINFOStruct(UVC_DEV);
	
	if(xTASK_HDL_UVC_DRV[device_id] != NULL){
		osThreadTerminate(xTASK_HDL_UVC_DRV[device_id]);		
		xTASK_HDL_UVC_DRV[device_id] = NULL;
	}
        
	UVC_INFO_Struct.bCurDevCnt--;
	memset((void *)&UVC_INFO_Struct.DEV[bDevId], 0, sizeof(USBH_UVC_DEV_Struct));
	
#if defined (CONFIG_USBH_CLI_UVC) || defined (CONFIG_NONCLI_APP)
	for(i=0;i<max_stream_count;i++){
		if(usbh_uvc_app.dev[device_id].stream[i].ptr != NULL) {
			UVC_INFO(" vPortFree usbh_uvc_app.dev[%d].stream[%d].ptr = %x", device_id, i, (uint32_t)usbh_uvc_app.dev[device_id].stream[i].ptr);
			RF_USBH_Buf_PTR -= usbh_uvc_app.dev[device_id].stream[i].size;
			usbh_uvc_app.dev[device_id].stream[i].ptr = NULL;
		}
	}
	
	memset((void *)&usbh_uvc_app.dev[device_id], 0, sizeof(USBH_UVC_APP_DEV_STRUCTURE));
#endif
	
	UVC_DBG("UVC DRV TASK DESTORY !");	
	
	if(UVC_INFO_Struct.bCurDevCnt == 0){
		if(xTASK_HDL_UVC_APP != NULL) {
			osThreadTerminate(xTASK_HDL_UVC_APP);
			xTASK_HDL_UVC_APP = NULL;
		}
		if(xTASK_HDL_UVC_STREAM_XFR != NULL) {
			osThreadTerminate(xTASK_HDL_UVC_STREAM_XFR);
			xTASK_HDL_UVC_STREAM_XFR = NULL;
		}
		if(usbh_queue_uvc_app != NULL) {
			osMutexDelete(usbh_queue_uvc_app);
			usbh_queue_uvc_app = NULL;
		}
		if(usbh_queue_uvc_mw != NULL) {
			osMutexDelete(usbh_queue_uvc_mw);
			usbh_queue_uvc_mw = NULL;
		}
		if(usbh_queue_uvc_drv != NULL) {
			osMutexDelete(usbh_queue_uvc_drv);
			usbh_queue_uvc_drv = NULL;
		}
		
		if(bk_ping_mutex != NULL) {
			osMutexDelete(bk_ping_mutex);
			bk_ping_mutex = NULL;
		}
		
		if(bk_pong_mutex != NULL) {
			osMutexDelete(bk_pong_mutex);
			bk_pong_mutex = NULL;
		}
		ping_empty = 1;
		pong_empty = 1;
		
		if(bk_wait_compose_sem != NULL) {
			osSemaphoreDelete(bk_wait_compose_sem);
			bk_wait_compose_sem = NULL;
		}

		if(bk_wait_compose_mutex != NULL) {
			osMutexDelete(bk_wait_compose_mutex);
			bk_wait_compose_mutex = NULL;
		}

		bk_wait_compose_count = 0;
		
		clean_bk_buf_idx = 0;

#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
if(xTASK_HDL_UVC_XFR_CTRL != NULL) {
			osThreadTerminate(xTASK_HDL_UVC_XFR_CTRL);
			xTASK_HDL_UVC_XFR_CTRL = NULL;
		}

		if(usbh_queue_uvc_xfr_ctrl != NULL) {
			osMutexDelete(usbh_queue_uvc_xfr_ctrl);
			usbh_queue_uvc_xfr_ctrl = NULL;
		}
#endif
memset((void *)&usbh_uvc_app, 0, sizeof(USBH_UVC_APP_STRUCTURE));
	}
#if defined( CONFIG_SNX_ISO_ACCELERATOR )	

#else	
	if(USBH_QUEUE_STD_ISO_REQ != NULL){
		osMutexDelete(USBH_QUEUE_STD_ISO_REQ);	
		USBH_QUEUE_STD_ISO_REQ = NULL;		
	}	
#endif

	if(USBH_QUEUE_BULK_REQ[device_id] != NULL){
		osMutexDelete(USBH_QUEUE_BULK_REQ[device_id]);
		USBH_QUEUE_BULK_REQ[device_id] = NULL;
	}

	if(xTASK_HDL_UVC_BULK_DATA != NULL) {
		osThreadTerminate(xTASK_HDL_UVC_BULK_DATA);
		xTASK_HDL_UVC_BULK_DATA = NULL;
	}
	
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
	if(usbh_queue_uvc_bulk_data != NULL) {
		osMutexDelete(usbh_queue_uvc_bulk_data);
		usbh_queue_uvc_bulk_data = NULL;
	}
#endif
	if(USBH_QUEUE_BULK_REQ_DATA != NULL){
		osMutexDelete(USBH_QUEUE_BULK_REQ_DATA);
		USBH_QUEUE_BULK_REQ_DATA = NULL;
	}

	UVC_DBG("UVC TASK DESTORY !");	
}
#endif

uint32_t uvc_get_time(uint32_t ptr)
{
	uint8_t *header;
	uint32_t source_time_clock;

	header  =	(uint8_t *)(ptr+10);
	source_time_clock	=	header[1] << 8 | header[0];
	return source_time_clock;
}

uint8_t	uvc_get_iframe(uint32_t	ptr){
	uint8_t *headerinfo;

	headerinfo = (uint8_t*) ptr;
	if(headerinfo[1] & EndofFrame) {
		return 1;
	}else{
		return 0;
	}
}


uint8_t uvc_scatter_gatter_dma(void *EP, USBH_ISO_XFR_REQ_Struct *ISO_REQ)
{
	if (ehci_take_ep_sem((EHCI_ENDP_STRUCT*)EP, USBH_MAX) == FAIL){
		goto dma_imeout;
	}
	if (ehci_move_itd_data_to_buff((EHCI_ENDP_STRUCT*)EP,(EHCI_ISO_XFR_REQ_Struct*)ISO_REQ) == SUCCESS){
		return SUCCESS;
	}else{
		return NYET;	
	}
dma_imeout:	
	UVC_DBG("UVC Scatter Gatter DMA FAIL;");
	return TIME_OUT;
}

void uvc_iso_sm(void)
{
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
#if defined (CONFIG_USBH_FREE_RTOS)
	do{
		vTaskDelay(USBH_100ms);
	}while(1);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	do{
		osDelay(USBH_100ms);
	}while(1);
#endif
#else
	USBH_STD_ISO_QUEUE_STRUCT	STD_ISO_QUEUE;
	USBH_ISO_XFR_REQ_Struct 	ISO_REQ;
	void												*DEV	= NULL;
	void												*EP		= NULL;
	uint8_t											STS		= FAIL;

	do{
		//Wait EP is Enable
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueReceive(USBH_QUEUE_STD_ISO_REQ, (USBH_STD_ISO_QUEUE_STRUCT*)&STD_ISO_QUEUE, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessageGet(USBH_QUEUE_STD_ISO_REQ, (USBH_STD_ISO_QUEUE_STRUCT*)&STD_ISO_QUEUE, USBH_MAX);
#endif
		
		memcpy(&ISO_REQ,STD_ISO_QUEUE.ISO_REQ,sizeof(USBH_ISO_XFR_REQ_Struct));
		DEV	= (void *)STD_ISO_QUEUE.DEV;
		EP	= (void *)STD_ISO_QUEUE.EP;		
		do{
			switch(ISO_REQ.STD_ISO_SM){
				case 1:
					// Wait Frame List Rollover
					ehci_rollover_enable();
#if defined (CONFIG_USBH_FREE_RTOS)
					xSemaphoreTake(USBH_SEM_FRAME_LIST_ROLLOVER, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osSemaphoreWait(USBH_SEM_FRAME_LIST_ROLLOVER, USBH_MAX);
#endif
					ehci_rollover_disable();
		
					// STD ISO X
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_X;
					usbh_iso_struct_init((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					
					// STD ISO Y
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_Y;
					usbh_iso_struct_init((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					
					ISO_REQ.STD_ISO_SM = 2;
					break;
				case 2:
					// SCATTER GATHER DMA X
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_X;
					STS = uvc_scatter_gatter_dma((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					if( STS == TIME_OUT){
						ISO_REQ.STD_ISO_SM = 0;					
						goto ERR;						
					} 	
					// STD ISO Z					
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_Z;
					usbh_iso_struct_init((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					

					// GIVE X	
					if(STS == SUCCESS){
						uvc_process_data(DEV,EP);
					}
					
					ISO_REQ.STD_ISO_SM = 3;					
					break;
				case 3:
					// SCATTER GATHER DMA Y
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_Y;
					STS = uvc_scatter_gatter_dma((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					if( STS == TIME_OUT){
						ISO_REQ.STD_ISO_SM = 0; 				
						goto ERR;						
					} 
					
					// STD ISO W
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_W;
					usbh_iso_struct_init((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);


					// GIVE Y
					if(STS == SUCCESS){
						uvc_process_data(DEV,EP);
					}				
					
					ISO_REQ.STD_ISO_SM = 4;
					break;

				case 4:
					// SCATTER GATHER DMA Z
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_Z;
					STS = uvc_scatter_gatter_dma((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					if( STS == TIME_OUT){
						ISO_REQ.STD_ISO_SM = 0; 				
						goto ERR;						
					} 
					
					// STD ISO X
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_X;
					usbh_iso_struct_init((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);


					// GIVE Z
					if(STS == SUCCESS){
						uvc_process_data(DEV,EP);
					}
					
					ISO_REQ.STD_ISO_SM = 5;
					break;			
					
				case 5:
					// SCATTER GATHER DMA W
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_W;
					STS = uvc_scatter_gatter_dma((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);
					if( STS == TIME_OUT){
						ISO_REQ.STD_ISO_SM = 0; 				
						goto ERR;						
					} 

					// STD ISO Y
					ISO_REQ.STD_ISO_REQ	=	USBH_STD_ISO_Y;
					usbh_iso_struct_init((void *)EP,(USBH_ISO_XFR_REQ_Struct*)&ISO_REQ);

					// GIVE W
					if(STS == SUCCESS){
						uvc_process_data(DEV,EP);
					}
					
					ISO_REQ.STD_ISO_SM = 2;					
					break;					
				default:
					goto ERR;
			}		
		}while(1);
ERR:		
		UVC_DBG("STD ISO TRANS STOP ! ");
	}while(1);
#endif		
}

#if defined (CONFIG_MODULE_USB_UVC_BK_FRAME_BASE)
void uvc_bulk_sm_frame_base(USBH_Device_Structure *UVC_DEV)
{
	USBH_BK_XFR_REQ_Struct	*BULK_REQ;
	USBH_BULK_QUEUE_STRUCT	BULK_REQ_QUEUE;

//	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	void						*EP;
	uint8_t					STATUS = SUCCESS;	

	do{
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueReceive(USBH_QUEUE_BULK_REQ[UVC_DEV->device_id], (USBH_BULK_QUEUE_STRUCT*)&BULK_REQ_QUEUE, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessageGet(USBH_QUEUE_BULK_REQ[UVC_DEV->device_id], (USBH_BULK_QUEUE_STRUCT*)&BULK_REQ_QUEUE, USBH_MAX);
#endif
		
		//memcpy(&BULK_REQ, BULK_REQ_QUEUE.BULK_REQ, sizeof(USBH_BK_XFR_REQ_Struct));
		BULK_REQ = (USBH_BK_XFR_REQ_Struct*)BULK_REQ_QUEUE.BULK_REQ;
		EP = (void*)BULK_REQ_QUEUE.EP;

		do{
			switch(BULK_REQ->BULK_SM){
				case 1: //ping buff
					BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;

					if(STATUS != SUCCESS){
						UVC_DBG("BK_1 XFR ERR!");
						//goto ERR;
					}
						BULK_REQ->BULK_SM = 2;
					} else{
						BULK_REQ->BULK_SM = 0;
						goto ERR;
					}
					break;
				case 2: //pong buff
					BULK_REQ->pBUFF = BULK_REQ->BULK_PONG_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
					if(STATUS != SUCCESS){
						UVC_DBG("BK_2 XFR ERR!");
						//goto ERR;
					}
						BULK_REQ->BULK_SM = 1;
					} else{
						BULK_REQ->BULK_SM = 0;
						goto ERR;
					}
					break;
				default:
					goto ERR;			
			}
			UVC_DATA.stream[UVC_DATA.index].ptr		= BULK_REQ->pBUFF;
			UVC_DATA.stream[UVC_DATA.index].size		= BULK_REQ->ACT_SIZE;					
			UVC_DATA.stream[UVC_DATA.index].stream_id	= uvc_ep_to_streamid(UVC_DEV, (EHCI_ENDP_STRUCT*)EP);
			UVC_DATA.stream[UVC_DATA.index].ring_buff_end 	= 0;
			UVC_DATA.stream[UVC_DATA.index].ring_buff_start = 0;
			if(usbh_queue_uvc_mw != NULL){
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSend(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_uvc_mw, &UVC_DATA.stream[UVC_DATA.index], 0);
#endif
				UVC_DATA.index ++;
				UVC_DATA.index = (UVC_DATA.index%USBH_UVC_DATA_QUEUE_SIZE);
			}	
		}while(1);
ERR:
		UVC_DBG("BULK_FB TRANS STOP ! ");
	}while(1);
}
#endif

#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
#if defined (CONFIG_USBH_FREE_RTOS)
void uvc_xfr_ctrl_task(void *pvParameter) {
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void uvc_xfr_ctrl_task(void const *pvParameter) {
#endif

	uint8_t timeout = 0;
	EHCI_ENDP_STRUCT EP;
	memset(&EP, 0, sizeof(EHCI_ENDP_STRUCT));
	
	while(1) {
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueReceive(usbh_queue_uvc_xfr_ctrl, &EP, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessageGet(usbh_queue_uvc_xfr_ctrl, &EP, USBH_MAX);
#endif
		
		timeout = (rand()%100)+5;
		UVC_DBG("ISO TIMEOUT = %d\n", timeout);
		
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(timeout/portTICK_RATE_MS);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(timeout);
#endif
		
//		if(EP.pSXITD->HW_FRAME_END > EP.pSXITD->bRING_THRESHOLD<<12){
//			EP.LastFwFrameEnd				=	EP.pSXITD->bRING_BUFFER_START<<12;
//			EP.pSXITD->FW_FRAME_END	=	EP.pSXITD->bRING_BUFFER_START<<12;		
//		}else{
			EP.LastFwFrameEnd				=	EP.pSXITD->HW_FRAME_END;
			EP.pSXITD->FW_FRAME_END	=	EP.pSXITD->HW_FRAME_END;
		//}		
		EP.LastFID	= EP.pSXITD->bFID_Count;
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST) 
		test_frame_count = 0;
#endif
		EHCI->USBINTR.BUF_FULL_INT_EN = 1;
	}
}
#endif

#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
#if defined (CONFIG_USBH_FREE_RTOS)
void uvc_bulk_data_move_task(void *pvParameter) {

	USBH_Device_Structure			*UVC_DEV = NULL;
	USBH_BULK_QUEUE_STRUCT		BULK_REQ_QUEUE;
	//USBH_BK_XFR_REQ_Struct		BULK_REQ;
	USBH_BK_PACKET_XFR_Struct		BULK_PACKET_Struct;
	uint32_t size = 0;
	
	uint8_t							*ptr						= NULL;
	uint8_t							*source_ptr			= NULL;
	uint8_t							*target_ptr			= NULL;
	uint8_t							start_xfr				= 0;
	//uint8_t							discard_frame_cnt = 0;

	//void										*EP = NULL;
	EHCI_ENDP_STRUCT										*EP = NULL;
	
	memset(&BULK_REQ_QUEUE, 0, sizeof(USBH_BULK_QUEUE_STRUCT));
	memset(&BULK_PACKET_Struct, 0, sizeof(USBH_BK_PACKET_XFR_Struct));
	//memset(&BULK_REQ, 0, sizeof(USBH_BK_XFR_REQ_Struct));
	UVC_DEV = (USBH_Device_Structure *)pvParameter;
	
	do{
		xQueueReceive(USBH_QUEUE_BULK_REQ_DATA, (USBH_BULK_QUEUE_STRUCT*)&BULK_REQ_QUEUE, USBH_MAX);
		
		//BULK_REQ = (USBH_BK_XFR_REQ_Struct*)BULK_REQ_QUEUE.BULK_REQ;
		//EP = (void*)BULK_REQ_QUEUE.EP;
		EP = (EHCI_ENDP_STRUCT*)BULK_REQ_QUEUE.EP;
		EP->PING_FRAME_PTR = NULL;
		EP->PONG_FRAME_PTR = NULL; 
		EP->PING_FRAME_SIZE = 0;
		EP->PONG_FRAME_SIZE = 0;
		EP->PING_FRAME_DONE = 0;
		EP->PONG_FRAME_DONE = 0;

		do{
NEXT:
			xQueueReceive(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);
			
			ptr		= (uint8_t*)BULK_PACKET_Struct.pBUFF;
			size	= BULK_PACKET_Struct.SIZE;
			
			EP->UVC_Header = *(ptr + 1);
			
			if(bk_packet_err == 1) {
				if (EP->LAST_TOG == 0) {
					if(ping_empty == 1) {
						EP->PING_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PING_PTR;
					}
				} else {
					if(pong_empty == 1) {
						EP->PONG_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PONG_PTR;
					}
				}
					
				if (EP->UVC_Header & 0x02) {
					xSemaphoreTake(bk_packet_err_mutex, USBH_10ms);
					++bk_discarded_frame_cnt;
					if(bk_discarded_frame_cnt == 2) {
						bk_packet_err = 0;
						bk_discarded_frame_cnt = 0;
					}
					xSemaphoreGive(bk_packet_err_mutex);
				}
			
				xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
				--bk_wait_compose_count;
				if(bk_wait_compose_count == 2) {
					xSemaphoreGive(bk_wait_compose_mutex);
			
					xSemaphoreGive(bk_wait_compose_sem);
				} else {
					xSemaphoreGive(bk_wait_compose_mutex);
				}
			
				goto NEXT;
			}
			
			if((EP->PING_FRAME_PTR == NULL) && (EP->PONG_FRAME_PTR == NULL)) {
				start_xfr = 1;

				if(start_xfr) {
					EP->PING_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PING_PTR;
					EP->PONG_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PONG_PTR;
				}
			}
			
			if(start_xfr == 0) {
			if (EP->LAST_TOG == 0) {
					if(ping_empty == 1) {
						if (EP->UVC_Header & 0x02) {
							start_xfr = 1;
						}
					}
				} else {
					if(pong_empty == 1) {
						if (EP->UVC_Header & 0x02) {
							start_xfr = 1;
						}
					}
				}

				xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
				--bk_wait_compose_count;
				if(bk_wait_compose_count == 2) {
					xSemaphoreGive(bk_wait_compose_mutex);

					xSemaphoreGive(bk_wait_compose_sem);
				} else {
					xSemaphoreGive(bk_wait_compose_mutex);
				}

				goto NEXT;
			}

			if (EP->LAST_TOG == 0) {
				if (ping_empty == 0) {
					UVC_DBG("BULK PING BUFFER OVERFLOW , SKIP This uSOF DATA");

					if (pong_empty == 0)
						start_xfr = 0;

					xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
					--bk_wait_compose_count;
					if(bk_wait_compose_count == 2) {
						xSemaphoreGive(bk_wait_compose_mutex);

						xSemaphoreGive(bk_wait_compose_sem);
					} else {
						xSemaphoreGive(bk_wait_compose_mutex);
				}

					goto NEXT;
				}

				target_ptr = EP->PING_FRAME_PTR;
					
				if(EP->PING_FRAME_SIZE == 0) {
				} else {
					ptr		+= 12;
					size	-= 12;
				}		
			} else {
				if (pong_empty == 0) {
					UVC_DBG("BULK PONG BUFFER OVERFLOW , SKIP This uSOF DATA");

					if (ping_empty == 0)
						start_xfr = 0;

					xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
					--bk_wait_compose_count;
					if(bk_wait_compose_count == 2) {
						xSemaphoreGive(bk_wait_compose_mutex);

						xSemaphoreGive(bk_wait_compose_sem);
					} else {
						xSemaphoreGive(bk_wait_compose_mutex);
				}

					goto NEXT;
				}

				target_ptr = EP->PONG_FRAME_PTR;
				
				if(EP->PONG_FRAME_SIZE == 0) {		
				} else {
					ptr		+= 12;
					size	-= 12;
				}	
			}
			
			source_ptr = ptr;

			memcpy(target_ptr, source_ptr, size);

			if (EP->LAST_TOG == 0) {
				EP->PING_FRAME_PTR	+= size;
				EP->PING_FRAME_SIZE	+= size;
			} else {
				EP->PONG_FRAME_PTR	+= size;
				EP->PONG_FRAME_SIZE	+= size;
			}
		
			if (EP->UVC_Header & 0x02) {
				if (EP->LAST_TOG == 0) {
					
					EP->PING_FRAME_DONE = 1;
					EP->PONG_FRAME_DONE = 0;
					
					EP->PONG_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PONG_PTR;
					EP->PONG_FRAME_SIZE = 0;

					xSemaphoreTake(bk_ping_mutex, USBH_5ms);
					ping_empty = 0;
					xSemaphoreGive(bk_ping_mutex);

				} else {
					
					EP->PONG_FRAME_DONE = 1;
					EP->PING_FRAME_DONE = 0;
					
					EP->PING_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PING_PTR;
					EP->PING_FRAME_SIZE = 0;

					xSemaphoreTake(bk_pong_mutex, USBH_5ms);
					pong_empty = 0;
					xSemaphoreGive(bk_pong_mutex);

				}
				EP->LAST_TOG++;
				
				uvc_process_data_bulk(UVC_DEV, EP);
			}

			xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
			--bk_wait_compose_count;
			if(bk_wait_compose_count == 2) {
				xSemaphoreGive(bk_wait_compose_mutex);

				xSemaphoreGive(bk_wait_compose_sem);
			} else {
				xSemaphoreGive(bk_wait_compose_mutex);
			}

			goto NEXT;
			
//FRAME_NYET:
//		UVC_DBG("Frame NYET");
		}while(1);
	}while(1);
}

static inline void uvc_bk_packet_err_occur() {
	xSemaphoreTake(bk_packet_err_mutex, USBH_10ms);
	if(bk_discarded_frame_cnt == 1)
		bk_discarded_frame_cnt = 0;
	bk_packet_err = 1;
	xSemaphoreGive(bk_packet_err_mutex);
}

void uvc_bulk_sm_packet_base(USBH_Device_Structure *UVC_DEV)
{
	USBH_BK_XFR_REQ_Struct	*BULK_REQ				= NULL;
	USBH_BULK_QUEUE_STRUCT	BULK_REQ_QUEUE;
	USBH_BK_PACKET_XFR_Struct	BULK_PACKET_Struct;
	uint8_t									STATUS					= SUCCESS;	
	uint8_t									start_send			= 0;
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
	uint8_t									timeout					= 0;
#endif
	
	memset(&BULK_REQ_QUEUE, 0, sizeof(USBH_BULK_QUEUE_STRUCT));
	memset(&BULK_PACKET_Struct, 0, sizeof(USBH_BK_PACKET_XFR_Struct));
	
	do{
		xQueueReceive(USBH_QUEUE_BULK_REQ[UVC_DEV->device_id], (USBH_BULK_QUEUE_STRUCT*)&BULK_REQ_QUEUE, USBH_MAX);
		
		//memcpy(&BULK_REQ, BULK_REQ_QUEUE.BULK_REQ, sizeof(USBH_BK_XFR_REQ_Struct));
		BULK_REQ = (USBH_BK_XFR_REQ_Struct*)BULK_REQ_QUEUE.BULK_REQ;

		BULK_PACKET_Struct.BULK_PING_PTR = BULK_REQ->BULK_PING_PTR;
		BULK_PACKET_Struct.BULK_PONG_PTR = BULK_REQ->BULK_PONG_PTR;

		do{
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
			if(test_frame_count == 50) {
				timeout = (rand()%100)+5; 
				UVC_DBG("BULK TIMEOUT = %d\n", timeout);

				vTaskDelay(timeout/portTICK_RATE_MS);

				test_frame_count = 0;
			}
#endif
			start_send = 1;

			switch(BULK_REQ->BULK_SM){
				case 1:
					BULK_REQ->pBUFF = BULK_REQ->RING_BUFF_PTR_W;
				//BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
						if((STATUS != SUCCESS) || (BULK_REQ->ACT_SIZE < 12)){
							uvc_bk_packet_err_occur();
							
							UVC_DBG("BK_PB_1 XFR ERR!");
							//goto ERR;
						}
				
						if(start_send) {
							BULK_PACKET_Struct.pBUFF = BULK_REQ->RING_BUFF_PTR_W;
							BULK_PACKET_Struct.SIZE	 = BULK_REQ->ACT_SIZE;

							xQueueSend(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);

							xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
							++bk_wait_compose_count;
							if(bk_wait_compose_count == 3) {
								xSemaphoreGive(bk_wait_compose_mutex);

								xSemaphoreTake(bk_wait_compose_sem, USBH_MAX);	
								
								uvc_bk_packet_err_occur();
							} else {
								xSemaphoreGive(bk_wait_compose_mutex);
							}
							
							BULK_REQ->BULK_SM = 2;
						}
					} else{
						BULK_REQ->BULK_SM = 0;
						goto ERR;
					}
					break;
					
				case 2:
					BULK_REQ->pBUFF = BULK_REQ->RING_BUFF_PTR_X;
				//BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
						if((STATUS != SUCCESS) || (BULK_REQ->ACT_SIZE < 12)){
							uvc_bk_packet_err_occur();
							
							UVC_DBG("BK_PB_2 XFR ERR!");
							//goto ERR;
						}

						BULK_PACKET_Struct.pBUFF = BULK_REQ->RING_BUFF_PTR_X;
						BULK_PACKET_Struct.SIZE	 = BULK_REQ->ACT_SIZE;

						xQueueSend(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);

						xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
						++bk_wait_compose_count;
						if(bk_wait_compose_count == 3) {
							xSemaphoreGive(bk_wait_compose_mutex);

							xSemaphoreTake(bk_wait_compose_sem, USBH_MAX);
							
							uvc_bk_packet_err_occur();
							
						} else {
							xSemaphoreGive(bk_wait_compose_mutex);
						}
				
						BULK_REQ->BULK_SM = 3;
					} else {
						BULK_REQ->BULK_SM = 0;
						goto ERR;
					}
					break;
					
				case 3:
					BULK_REQ->pBUFF = BULK_REQ->RING_BUFF_PTR_Y;
				//BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
						if((STATUS != SUCCESS) || (BULK_REQ->ACT_SIZE < 12)){
							uvc_bk_packet_err_occur();
							
							UVC_DBG("BK_PB_3 XFR ERR!");
							//goto ERR;
						}

						BULK_PACKET_Struct.pBUFF = BULK_REQ->RING_BUFF_PTR_Y;
						BULK_PACKET_Struct.SIZE	 = BULK_REQ->ACT_SIZE;

						xQueueSend(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);

						xSemaphoreTake(bk_wait_compose_mutex, USBH_10ms);
						++bk_wait_compose_count;
						if(bk_wait_compose_count == 3) {
							xSemaphoreGive(bk_wait_compose_mutex);

							xSemaphoreTake(bk_wait_compose_sem, USBH_MAX);
							
							uvc_bk_packet_err_occur();
						} else {
							xSemaphoreGive(bk_wait_compose_mutex);
						}

						BULK_REQ->BULK_SM = 1;
					} else {
						BULK_REQ->BULK_SM = 0;
							goto ERR;
						}
					break;
			}	
		}while(1);
ERR:
		UVC_DBG("BULK_PB TRANS STOP ! ");
	}while(1);
}
#endif // end of #if defined (CONFIG_USBH_FREE_RTOS)

#if defined (CONFIG_USBH_CMSIS_OS)
void uvc_bulk_data_move_task(void const *pvParameter) {

	USBH_Device_Structure			*UVC_DEV = NULL;
	USBH_BULK_QUEUE_STRUCT		BULK_REQ_QUEUE;
	//USBH_BK_XFR_REQ_Struct		BULK_REQ;
	USBH_BK_PACKET_XFR_Struct		BULK_PACKET_Struct;
	uint32_t size = 0;
	
	uint8_t							*ptr						= NULL;
	uint8_t							*source_ptr			= NULL;
	uint8_t							*target_ptr			= NULL;
	uint8_t							start_xfr				= 0;

	//void										*EP = NULL;
	EHCI_ENDP_STRUCT										*EP = NULL;
	
	memset(&BULK_REQ_QUEUE, 0, sizeof(USBH_BULK_QUEUE_STRUCT));
	memset(&BULK_PACKET_Struct, 0, sizeof(USBH_BK_PACKET_XFR_Struct));
	//memset(&BULK_REQ, 0, sizeof(USBH_BK_XFR_REQ_Struct));
	UVC_DEV = (USBH_Device_Structure *)pvParameter;
	
	do{
		osMessageGet(USBH_QUEUE_BULK_REQ_DATA, (USBH_BULK_QUEUE_STRUCT*)&BULK_REQ_QUEUE, USBH_MAX);
		
		//BULK_REQ = (USBH_BK_XFR_REQ_Struct*)BULK_REQ_QUEUE.BULK_REQ;
		//EP = (void*)BULK_REQ_QUEUE.EP;
		EP = (EHCI_ENDP_STRUCT*)BULK_REQ_QUEUE.EP;
		EP->PING_FRAME_PTR = NULL;
		EP->PONG_FRAME_PTR = NULL; 
		EP->PING_FRAME_SIZE = 0;
		EP->PONG_FRAME_SIZE = 0;
		EP->PING_FRAME_DONE = 0;
		EP->PONG_FRAME_DONE = 0;

		do{
NEXT:
			osMessageGet(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);

			ptr		= (uint8_t*)BULK_PACKET_Struct.pBUFF;
			size	= BULK_PACKET_Struct.SIZE;
			
			if(size <= 12) {

				osMutexWait(bk_wait_compose_mutex, USBH_10ms);
				--bk_wait_compose_count;
				if(bk_wait_compose_count == 2) {
					osMutexRelease(bk_wait_compose_mutex);

					osSemaphoreRelease(bk_wait_compose_sem);
				} else {
					osMutexRelease(bk_wait_compose_mutex);
				}

				goto NEXT;
			}
			
			EP->UVC_Header = *(ptr + 1);
			
			if((EP->PING_FRAME_PTR == NULL) && (EP->PONG_FRAME_PTR == NULL)) {
				start_xfr = 1;

				if(start_xfr) {
					EP->PING_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PING_PTR;
					EP->PONG_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PONG_PTR;
				}
			}
			
			if(start_xfr == 0) {
				if (EP->LAST_TOG == 0) {
					if(ping_empty == 1) {
						if (EP->UVC_Header & 0x02) {
							start_xfr = 1;
						}
					}
				} else {
					if(pong_empty == 1) {
						if (EP->UVC_Header & 0x02) {
							start_xfr = 1;
						}
					}
				}

				osMutexWait(bk_wait_compose_mutex, USBH_10ms);
				--bk_wait_compose_count;
				if(bk_wait_compose_count == 2) {
					osMutexRelease(bk_wait_compose_mutex);

					osSemaphoreRelease(bk_wait_compose_sem);
				} else {
					osMutexRelease(bk_wait_compose_mutex);
				}

				goto NEXT;
			}

			if (EP->LAST_TOG == 0) {
				//if ((EP->PING_FRAME_DONE) || (ping_empty == 0)) {
				if (ping_empty == 0) {
					UVC_DBG("BULK PING BUFFER OVERFLOW , SKIP This uSOF DATA");

					if (pong_empty == 0)
						start_xfr = 0;

					osMutexWait(bk_wait_compose_mutex, USBH_10ms);
					--bk_wait_compose_count;
					if(bk_wait_compose_count == 2) {
						osMutexRelease(bk_wait_compose_mutex);

						osSemaphoreRelease(bk_wait_compose_sem);
					} else {
						osMutexRelease(bk_wait_compose_mutex);
					}

					goto NEXT;
				}

				target_ptr = EP->PING_FRAME_PTR;
				
				if(EP->PING_FRAME_SIZE == 0) {
				} else {
					ptr		+= 12;
					size	-= 12;
				}		
			} else {
				//if ((EP->PONG_FRAME_DONE) || (pong_empty == 0)){
				if (pong_empty == 0) {
					UVC_DBG("BULK PONG BUFFER OVERFLOW , SKIP This uSOF DATA");

					if (ping_empty == 0)
						start_xfr = 0;

					osMutexWait(bk_wait_compose_mutex, USBH_10ms);
					--bk_wait_compose_count;
					if(bk_wait_compose_count == 2) {
						osMutexRelease(bk_wait_compose_mutex);

						osSemaphoreRelease(bk_wait_compose_sem);
					} else {
						osMutexRelease(bk_wait_compose_mutex);
					}

					goto NEXT;
				}

				target_ptr = EP->PONG_FRAME_PTR;
				
				if(EP->PONG_FRAME_SIZE == 0) {		
				} else {
					ptr		+= 12;
					size	-= 12;
				}	
			}
			
			source_ptr = ptr;

			memcpy(target_ptr, source_ptr, size);

			if (EP->LAST_TOG == 0) {
				EP->PING_FRAME_PTR	+= size;
				EP->PING_FRAME_SIZE	+= size;
			} else {
				EP->PONG_FRAME_PTR	+= size;
				EP->PONG_FRAME_SIZE	+= size;
			}
		
			if (EP->UVC_Header & 0x02) {
				if (EP->LAST_TOG == 0) {
					
					EP->PING_FRAME_DONE = 1;
					EP->PONG_FRAME_DONE = 0;
					 
					EP->PONG_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PONG_PTR;
					EP->PONG_FRAME_SIZE = 0;

					osMutexWait(bk_ping_mutex, USBH_5ms);
					ping_empty = 0;
					osMutexRelease(bk_ping_mutex);

				} else {
					
					EP->PONG_FRAME_DONE = 1;
					EP->PING_FRAME_DONE = 0;
					
					EP->PING_FRAME_PTR = (uint8_t*) BULK_PACKET_Struct.BULK_PING_PTR;
					EP->PING_FRAME_SIZE = 0;

					osMutexWait(bk_pong_mutex, USBH_5ms);
					pong_empty = 0;
					osMutexRelease(bk_pong_mutex);

				}
				EP->LAST_TOG++;
				
				uvc_process_data_bulk(UVC_DEV, EP);
			}

			osMutexWait(bk_wait_compose_mutex, USBH_10ms);
			--bk_wait_compose_count;
			if(bk_wait_compose_count == 2) {
				osMutexRelease(bk_wait_compose_mutex);

				osSemaphoreRelease(bk_wait_compose_sem);
			} else {
				osMutexRelease(bk_wait_compose_mutex);
			}

			goto NEXT;

		}while(1);
	}while(1);
}

void uvc_bulk_sm_packet_base(USBH_Device_Structure *UVC_DEV)
{
	USBH_BK_XFR_REQ_Struct	*BULK_REQ				= NULL;
	USBH_BULK_QUEUE_STRUCT	BULK_REQ_QUEUE;
	USBH_BK_PACKET_XFR_Struct	BULK_PACKET_Struct;
	uint8_t									STATUS					= SUCCESS;
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
	uint8_t									timeout					= 0;
#endif
	
	memset(&BULK_REQ_QUEUE, 0, sizeof(USBH_BULK_QUEUE_STRUCT));
	memset(&BULK_PACKET_Struct, 0, sizeof(USBH_BK_PACKET_XFR_Struct));
						
	do{
		osMessageGet(USBH_QUEUE_BULK_REQ[UVC_DEV->device_id], (USBH_BULK_QUEUE_STRUCT*)&BULK_REQ_QUEUE, USBH_MAX);
	
		//memcpy(&BULK_REQ, BULK_REQ_QUEUE.BULK_REQ, sizeof(USBH_BK_XFR_REQ_Struct));
		BULK_REQ = (USBH_BK_XFR_REQ_Struct*)BULK_REQ_QUEUE.BULK_REQ;

		BULK_PACKET_Struct.BULK_PING_PTR = BULK_REQ->BULK_PING_PTR;
		BULK_PACKET_Struct.BULK_PONG_PTR = BULK_REQ->BULK_PONG_PTR;

		do{
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
			if(test_frame_count == 50) {
				timeout = (rand()%100)+5; 
				UVC_DBG("BULK TIMEOUT = %d\n", timeout);

				osDelay(timeout);

				test_frame_count = 0;
			}
#endif
			
			switch(BULK_REQ->BULK_SM){
				case 1:
					BULK_REQ->pBUFF = BULK_REQ->RING_BUFF_PTR_W;
				//BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
						if((STATUS != SUCCESS) || (BULK_REQ->ACT_SIZE < 12)){
							UVC_DBG("BK_PB_1 XFR ERR!")
							//goto ERR;
						}
						
						BULK_PACKET_Struct.pBUFF = BULK_REQ->RING_BUFF_PTR_W;
						BULK_PACKET_Struct.SIZE	 = BULK_REQ->ACT_SIZE;
						
						osMessagePut(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);
						
						osMutexWait(bk_wait_compose_mutex, USBH_10ms);
						++bk_wait_compose_count;
						if(bk_wait_compose_count == 3) {
							osMutexRelease(bk_wait_compose_mutex);
							
							osSemaphoreWait(bk_wait_compose_sem, USBH_MAX);
						} else {
							osMutexRelease(bk_wait_compose_mutex);
						}
						
						BULK_REQ->BULK_SM = 2;
						
					} else {
						BULK_REQ->BULK_SM = 0;
						goto ERR;
					}
					break;
				
				case 2:
					BULK_REQ->pBUFF = BULK_REQ->RING_BUFF_PTR_X;
				//BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
						if((STATUS != SUCCESS) || (BULK_REQ->ACT_SIZE < 12)){
							UVC_DBG("BK_PB_2 XFR ERR!");
							//goto ERR;
						}

						BULK_PACKET_Struct.pBUFF = BULK_REQ->RING_BUFF_PTR_X;
						BULK_PACKET_Struct.SIZE	 = BULK_REQ->ACT_SIZE;

						osMessagePut(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);

						osMutexWait(bk_wait_compose_mutex, USBH_10ms);
						++bk_wait_compose_count;
						if(bk_wait_compose_count == 3) {
							osMutexRelease(bk_wait_compose_mutex);

							osSemaphoreWait(bk_wait_compose_sem, USBH_MAX);
						} else {
							osMutexRelease(bk_wait_compose_mutex);
						}

						BULK_REQ->BULK_SM = 3;
					} else {
						BULK_REQ->BULK_SM = 0;
							goto ERR;
						}
					break;
					
				case 3:
					BULK_REQ->pBUFF = BULK_REQ->RING_BUFF_PTR_Y;
				//BULK_REQ->pBUFF = BULK_REQ->BULK_PING_PTR;
					if(BULK_REQ->Start){
						BULK_REQ->XfrDone = 0;
						STATUS = usbh_bk_xfr((USBH_Device_Structure*)UVC_DEV, (USBH_BK_XFR_REQ_Struct*)BULK_REQ);
						BULK_REQ->XfrDone = 1;
						if((STATUS != SUCCESS) || (BULK_REQ->ACT_SIZE < 12)){
							UVC_DBG("BK_PB_3 XFR ERR!");
							//goto ERR;
						}
						
						//BULK_REQ->Last_pBUFF		= BULK_REQ->RING_BUFF_PTR_Y;
						//BULK_REQ->Last_ACT_SIZE	= BULK_REQ->ACT_SIZE;
					
						BULK_PACKET_Struct.pBUFF = BULK_REQ->RING_BUFF_PTR_Y;
						BULK_PACKET_Struct.SIZE	 = BULK_REQ->ACT_SIZE;
						
						osMessagePut(usbh_queue_uvc_bulk_data, &BULK_PACKET_Struct, USBH_MAX);
						
						osMutexWait(bk_wait_compose_mutex, USBH_10ms);
						++bk_wait_compose_count;
						if(bk_wait_compose_count == 3) {
							osMutexRelease(bk_wait_compose_mutex);

							osSemaphoreWait(bk_wait_compose_sem, USBH_MAX);
						} else {
							osMutexRelease(bk_wait_compose_mutex);
						}
						
						BULK_REQ->BULK_SM = 1;
					} else{
						BULK_REQ->BULK_SM = 0;
						goto ERR;
					}	
					break;
			}	
		}while(1);
ERR:
		UVC_DBG("BULK_PB TRANS STOP ! ");
	}while(1);
}
#endif // end of #if defined (CONFIG_USBH_CMSIS_OS)
#endif // end of #if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)

#if defined (CONFIG_USBH_FREE_RTOS)
void uvc_task(void *pvParameters)
{
	USBH_Device_Structure		*UVC_DEV = NULL;
	USBH_UVC_CLASS_Structure	UVC_CLASS;
	uint8_t device_id = 0;
	
	memset(&UVC_CLASS, 0, sizeof(USBH_UVC_CLASS_Structure));
	memset(&UVC_DATA, 0, sizeof(USBH_UVC_DATA_QUEUE_Structure));
	
	UVC_DATA.mutex = xSemaphoreCreateMutex();
	
	xQueueReceive(usbh_queue_uvc_drv, &device_id, USBH_MAX);
	UVC_INFO("uvc device id:%d\r\n", device_id);

	UVC_DEV	= (USBH_Device_Structure*)usbh_uvc_init(device_id);	
	UVC_DEV->CLASS_STRUCT_PTR = (USBH_UVC_CLASS_Structure*)&UVC_CLASS;
	
	uvc_enum(UVC_DEV);

	UVC_DBG("UVC TASK ALIVE	!");

	if(UVC_DEV->CLASS_DRV == USBH_UVC_ISO_CLASS){
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
		if(usbh_queue_uvc_xfr_ctrl == NULL) {
			usbh_queue_uvc_xfr_ctrl = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(EHCI_ENDP_STRUCT));	
		}
		
		if(xTASK_HDL_UVC_XFR_CTRL == NULL) {
			xTaskCreate(
				uvc_xfr_ctrl_task,
				( const char * )"USBH_UVC_XFR_CTRL",
				4096,
				NULL,
				250,
				&xTASK_HDL_UVC_XFR_CTRL
			);
		}
#endif
		
		uvc_iso_sm();
	}else if((UVC_DEV->CLASS_DRV == USBH_UVC_BULK_CLASS) || (UVC_DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS))){
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
		if (usbh_queue_uvc_bulk_data == NULL) {
			usbh_queue_uvc_bulk_data = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(USBH_BK_PACKET_XFR_Struct));
		}
		
		if(bk_wait_compose_sem == NULL)
			bk_wait_compose_sem	= xSemaphoreCreateCounting(1, 0);
		if(bk_wait_compose_mutex == NULL)
			bk_wait_compose_mutex = xSemaphoreCreateMutex();
		
		if(bk_ping_mutex == NULL)
			bk_ping_mutex = xSemaphoreCreateMutex();
		if(bk_pong_mutex == NULL)
			bk_pong_mutex = xSemaphoreCreateMutex();
		
		if(bk_packet_err_mutex == NULL)
			bk_packet_err_mutex = xSemaphoreCreateMutex();
		
#if defined( CONFIG_SN_GCC_SDK )
		if(xTASK_HDL_UVC_BULK_DATA == NULL) {
			xTaskCreate(
				uvc_bulk_data_move_task,
				( const char * )"USBH_UVC_BULK_DATA",
				5120,
				(void *)UVC_DEV,
				PRIORITY_TASK_DRV_USBH,
				&xTASK_HDL_UVC_BULK_DATA
			);
		}
#endif
#if defined( CONFIG_SN_KEIL_SDK )
		if(xTASK_HDL_UVC_BULK_DATA == NULL) {
			xTaskCreate(
				uvc_bulk_data_move_task,
				( const char * )"USBH_UVC_BULK_DATA",
				1280,
				(void *)UVC_DEV,
				3,
				&xTASK_HDL_UVC_BULK_DATA
			);
		}
#endif
		uvc_bulk_sm_packet_base(UVC_DEV);
#endif
#if defined (CONFIG_MODULE_USB_UVC_BK_FRAME_BASE)
		uvc_bulk_sm_frame_base(UVC_DEV);
#endif
	}
}
		
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
void uvc_clean_bk_buf(void) {

	if(clean_bk_buf_idx == 0) {
		if(ping_empty == 0) {
			xSemaphoreTake(bk_ping_mutex, USBH_10ms);
			ping_empty = 1;
			xSemaphoreGive(bk_ping_mutex);

			clean_bk_buf_idx = 1;
		}
	} else {
		if(pong_empty == 0) {
			xSemaphoreTake(bk_pong_mutex, USBH_10ms);
			pong_empty = 1;
			xSemaphoreGive(bk_pong_mutex);
			
			clean_bk_buf_idx = 0;
	}
}
}
#endif

#endif // end of #if defined (CONFIG_USBH_FREE_RTOS)

#if defined (CONFIG_USBH_CMSIS_OS)
void uvc_task(void const *pvParameters)
{
	USBH_Device_Structure			*UVC_DEV = NULL;
	USBH_UVC_CLASS_Structure	UVC_CLASS;
	uint32_t									device_id = 0;
	
	memset(&UVC_DATA, 0, sizeof(USBH_UVC_DATA_QUEUE_Structure));
	
	osMutexDef(uvc_data_mutex);
	UVC_DATA.mutex = osMutexCreate(osMutex(uvc_data_mutex));
	
	memset(&UVC_CLASS, 0, sizeof(USBH_UVC_CLASS_Structure));	
	
	osMessageGet(usbh_queue_uvc_drv, &device_id, USBH_MAX);

	UVC_DEV	= (USBH_Device_Structure*)usbh_uvc_init(device_id);
	UVC_DEV->CLASS_STRUCT_PTR = (USBH_UVC_CLASS_Structure*)&UVC_CLASS;
		
	uvc_enum(UVC_DEV);

	UVC_DBG("UVC TASK ALIVE	!");

	if(UVC_DEV->CLASS_DRV == USBH_UVC_ISO_CLASS){
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
		if(usbh_queue_uvc_xfr_ctrl == NULL) {
			osMessageQDef(usbh_queue_uvc_xfr_ctrl, USBH_UVC_DATA_QUEUE_SIZE, EHCI_ENDP_STRUCT);
			usbh_queue_uvc_xfr_ctrl = osMessageCreate(osMessageQ(usbh_queue_uvc_xfr_ctrl), NULL);
		}
		
		osThreadDef(USBH_UVC_XFR_CTRL, uvc_xfr_ctrl_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
		xTASK_HDL_UVC_XFR_CTRL = osThreadCreate(osThread(USBH_UVC_XFR_CTRL), NULL);
		if( xTASK_HDL_UVC_XFR_CTRL == NULL )
			printf("Create USBH_UVC_XFR_CTRL fail\n");
#endif
		uvc_iso_sm();
	}else if((UVC_DEV->CLASS_DRV == USBH_UVC_BULK_CLASS) || (UVC_DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS))){
#if defined(CONFIG_MODULE_USB_UVC_BK_PACKET_BASE)
		if (usbh_queue_uvc_bulk_data == NULL) {
			osMessageQDef(usbh_queue_uvc_bulk_data, USBH_UVC_DATA_QUEUE_SIZE, USBH_BK_PACKET_XFR_Struct);
			usbh_queue_uvc_bulk_data = osMessageCreate(osMessageQ(usbh_queue_uvc_bulk_data), NULL);
		}
		
if(bk_wait_compose_sem == NULL) {
		osSemaphoreDef(bk_wait_compose_sem);
		bk_wait_compose_sem    = osSemaphoreCreate(osSemaphore(bk_wait_compose_sem), 1);
		osSemaphoreWait(bk_wait_compose_sem, USBH_10ms);
	}
	
	if(bk_wait_compose_mutex == NULL) {
		osMutexDef(bk_wait_compose_mutex);
		bk_wait_compose_mutex = osMutexCreate(osMutex(bk_wait_compose_mutex));
	}

	if(bk_ping_mutex == NULL) {
		osMutexDef(bk_ping_mutex);
		bk_ping_mutex = osMutexCreate(osMutex(bk_ping_mutex));
	}
	
	if(bk_pong_mutex == NULL) {
		osMutexDef(bk_pong_mutex);
		bk_pong_mutex = osMutexCreate(osMutex(bk_pong_mutex));
	}
	
		osThreadDef(USBH_UVC_BULK_DATA, uvc_bulk_data_move_task, THREAD_PRIO_USBH_PROCESS, 1, 5120);
		xTASK_HDL_UVC_BULK_DATA = osThreadCreate(osThread(USBH_UVC_BULK_DATA), UVC_DEV);
		if( xTASK_HDL_UVC_BULK_DATA == NULL )
			printf("Create USBH_UVC_XFR_CTRL fail\n");
		
		uvc_bulk_sm_packet_base(UVC_DEV);
#else
		uvc_bulk_sm_frame_base(UVC_DEV);
#endif
	}
}

void uvc_clean_bk_buf(void) {
	if(clean_bk_buf_idx == 0) {
		if(ping_empty == 0) {
			osMutexWait(bk_ping_mutex, USBH_10ms);
			ping_empty = 1;
			osMutexRelease(bk_ping_mutex);

			clean_bk_buf_idx = 1;
		}
	} else {
		if(pong_empty == 0) {
			osMutexWait(bk_pong_mutex, USBH_10ms);
			pong_empty = 1;
			osMutexRelease(bk_pong_mutex);

			clean_bk_buf_idx = 0;
	}
}
}

#endif // end of #if defined (CONFIG_USBH_CMSIS_OS)

#endif

