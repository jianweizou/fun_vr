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
// Author(s): IP2
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
 * this	is  USBH file
 * USBH-ERR-HDL.c
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
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
#include <nonstdlib.h>
#endif

#if defined( CONFIG_SN_KEIL_SDK )
#include <stdlib.h>
#endif 

#if defined( CONFIG_XILINX_SDK )
#include <stdlib.h>
#include <stdio.h>
#include <xil_cache.h>
#endif

#include <string.h>
#include "USBH.h"

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
#include "USBH_MSC.h"

void usbh_err_hdl_msc(USBH_ERR_HDL_STRUCT *err_hdl, USBH_ERR_RLT_STRUCT	*err_rlt)
{
	uint8_t									status					= SUCCESS;
	USBH_Device_Structure		*dev						= NULL;
	MSC_REQ_Struct					msc_req;
	uint8_t									*msc_data_buff	= NULL;
	uint8_t									err_type = 0;
	USBH_CX_XFR_REQ_Struct cx_req;
	
	memset(&cx_req, 0, sizeof(cx_req));

	dev	=	(USBH_Device_Structure*)usbh_msc_init(err_hdl->device_id);
	memset(&msc_req, 0,	sizeof(msc_req));

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )	
	do{
		msc_data_buff =	pvPortMalloc(32, GFP_DMA,MODULE_DRI_USBH);
	}while(msc_data_buff == NULL);
#endif	
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do{
		msc_data_buff = pvPortMalloc(32);
	}while(msc_data_buff == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
	msc_data_buff = (uint8_t *)malloc(32);
#endif
#endif

	msc_req.pbuff = msc_data_buff;
	err_rlt->pbuff[0] = 0;
	err_type = err_hdl->pbuff[0];
	
	switch(err_type){
		case CMD_FAIL:
			status = msc_request_sense((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req, (uint8_t) err_hdl->pbuff[1]);
			if(status == SUCCESS) {
				memcpy(err_rlt->pbuff, msc_req.pbuff,18);
			}
			break;

		case EP_STALL:
			usbh_cx_clear_feature((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)&cx_req, BULK_IN_EP);
			break;

		case PHASE_ERR:
		case CSW_NOT_VALID:
			err_rlt->pbuff[0] = MSC_RESET;
			msc_reset_recovery((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)&cx_req);
			break;

		case CONTINUOUS_ERR:
			err_rlt->pbuff[0] = MSC_RESET;
			msc_cx_bkonly_reset((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)&cx_req);
			break;

		case TIMEOUT:
		case NO_RESPONSE:
			usbh_force_root_plug_out();
			break;

		default:
			break;
	}
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(msc_data_buff);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(msc_data_buff);
#endif
}

void usbh_err_hdl_process(USBH_ERR_HDL_STRUCT *err_hdl,	USBH_ERR_RLT_STRUCT *err_rlt)
{
//  USBH_DBG("ERROR HANDLE PROCESS!!");
	
	switch(err_hdl->DriverClass){
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
		case MSC:
			usbh_err_hdl_msc(err_hdl, err_rlt);
			break;
#endif
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
		case UVC:
			break;
		#endif
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
		case WIFI:
			break;
#endif
#if defined (CONFIG_MODULE_USB_HID_CLASS)
		case HID:
			break;
#endif
		default:
			break;
	}
}

#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_err_hdl_task(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_err_hdl_task(void const *pvParameters)
#endif
{
	USBH_ERR_HDL_STRUCT	ERR_HDL_ST;
	USBH_ERR_RLT_STRUCT	ERR_RLT_ST;
	uint8_t err_data[18] = {0};

	memset((uint8_t*)&ERR_HDL_ST, 0, sizeof(ERR_HDL_ST));
	memset((uint8_t*)&ERR_RLT_ST, 0, sizeof(ERR_RLT_ST));

	ERR_RLT_ST.pbuff = &err_data[0];
	
#if defined (CONFIG_USBH_FREE_RTOS)
	while (1) {
		if (xQueueReceive(USBH_QUEUE_ERR_HDL, &ERR_HDL_ST, USBH_MAX)) {
			usbh_err_hdl_process(&ERR_HDL_ST, &ERR_RLT_ST);
			if ((ERR_HDL_ST.DriverClass == MSC) && ((ERR_HDL_ST.pbuff[0] == TIME_OUT)||(ERR_HDL_ST.pbuff[0] == NO_RESPONSE))) {
			} else {
					xQueueSend(USBH_QUEUE_ERR_RLT, &ERR_RLT_ST, 0);
			}
		}
		vTaskDelay(USBH_100ms);
	}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	while (1) {
		if (osMessageGet(USBH_QUEUE_ERR_HDL, &ERR_HDL_ST, USBH_MAX)) {
			usbh_err_hdl_process(&ERR_HDL_ST, &ERR_RLT_ST);
			if ((ERR_HDL_ST.DriverClass == MSC) && ((ERR_HDL_ST.pbuff[0] == TIME_OUT)||(ERR_HDL_ST.pbuff[0] == NO_RESPONSE))) {
			} else {
					osMessagePut(USBH_QUEUE_ERR_RLT, &ERR_RLT_ST, 0);
			}
		}
		osDelay(USBH_100ms);
	}
#endif
}

#if defined( CONFIG_DUAL_HOST )
#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_err_hdl_task_2(void *pvParameters)
{
	while(1){
		vTaskDelay(USBH_100ms);
	}
}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_err_hdl_task_2(void const *pvParameters)
{
	while(1){
		osDelay(USBH_100ms);
	}
}
#endif
#endif
#endif // end of #if defined (CONFIG_MODULE_USB_MSC_CLASS)
