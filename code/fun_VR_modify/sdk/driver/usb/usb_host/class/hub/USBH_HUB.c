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
 * this	is  USBH-HUB file
 * USBH-HUB.c
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
#include <bsp.h>
#include <nonstdlib.h>
#endif 

#if defined (CONFIG_MODULE_USB_HUB_CLASS)
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
#include "USBH_HUB.h"

#if defined (CONFIG_USBH_FREE_RTOS)
QueueHandle_t			usbh_queue_hub_drv = NULL;
QueueHandle_t			USBH_QUEUE_HUB_INT = NULL;
TaskHandle_t			xTASK_HDL_HUB_DRV[6] = {NULL}; // USBH_MAX_PORT+1
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
osMessageQId			usbh_queue_hub_drv = NULL;
osMessageQId			USBH_QUEUE_HUB_INT = NULL;
osThreadId				xTASK_HDL_HUB_DRV[6] = {NULL};
#endif


#if defined (CONFIG_USBH_FREE_RTOS)
void hub_task_init(uint8_t device_id)
{
	char task_name[256] = {'\0'};
	
	if(usbh_queue_hub_drv == NULL){
		usbh_queue_hub_drv	=	xQueueCreate(4, 4);
	}

	sprintf((char *)&task_name, "%s%d", "USBH_HUB_TASK", device_id);
	
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	xTaskCreate(
		hub_task,
		( const	char * )&task_name[0],
		4096,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_HUB_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
		hub_task,
		( const	char * )&task_name[0],
		4096,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_HUB_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
		hub_task,
		( const char * )&task_name[0],
		4096,
		(void*)&device_id,
		250,
		&xTASK_HDL_HUB_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
		hub_task,
		( const char * )&task_name[0],
		4096,
		(void*)&device_id,
		20,
		&xTASK_HDL_HUB_DRV[device_id]
	);	
#endif 
		
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	xTaskCreate(
		hub_task,
		( const char * )&task_name[0],
		512,
		(void*)&device_id,
		3,
		&xTASK_HDL_HUB_DRV[device_id]
	);	
#endif 

#if defined( CONFIG_XILINX_SDK )
	xTaskCreate(
		hub_task,
		( const	char * )&task_name[0],
		4096,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_HUB_DRV[device_id]
	);
#endif
}

void hub_task_uninit(uint8_t device_id)
{
	if(usbh_queue_hub_drv != NULL) {
			vQueueDelete(usbh_queue_hub_drv);
			usbh_queue_hub_drv = NULL;
		}
	
	if(xTASK_HDL_HUB_DRV[device_id] != NULL) {
		vTaskDelete(xTASK_HDL_HUB_DRV[device_id]);
		xTASK_HDL_HUB_DRV[device_id] = NULL;
	}
	
	HUB_DBG("HUB TASK DESTORY !");
}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void hub_task_init(uint8_t device_id)
{
	char task_name[256] = {'\0'};
	
	if(usbh_queue_hub_drv == NULL){
		osMessageQDef(usbh_queue_hub_drv, 4, uint32_t);
		usbh_queue_hub_drv = osMessageCreate(osMessageQ(usbh_queue_hub_drv), NULL);
	}

	sprintf((char *)&task_name, "%s%d", "USBH_HUB_TASK", device_id);
	
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	osThreadDef(task_name, hub_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HUB_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HUB_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HUB_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(task_name, hub_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HUB_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HUB_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HUB_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	osThreadDef(task_name, hub_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HUB_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HUB_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HUB_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	osThreadDef(task_name, hub_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HUB_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HUB_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HUB_DRV[%d] fail\n", device_id);
#endif 

#if defined( CONFIG_XILINX_SDK )
	osThreadDef(task_name, hub_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HUB_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HUB_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HUB_DRV[%d] fail\n", device_id);
#endif
}

void hub_task_uninit(uint8_t device_id)
{
	if(usbh_queue_hub_drv != NULL) {
			osMutexDelete(usbh_queue_hub_drv);
			usbh_queue_hub_drv = NULL;
		}
	
	if(xTASK_HDL_HUB_DRV[device_id] != NULL) {
		osThreadTerminate(xTASK_HDL_HUB_DRV[device_id]);
		xTASK_HDL_HUB_DRV[device_id] = NULL;
	}
	
	HUB_DBG("HUB TASK DESTORY !");
}
#endif


#if defined (CONFIG_USBH_FREE_RTOS)
void hub_task(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void hub_task(void const *pvParameters)
#endif
{
	USBH_Device_Structure			*HUB_DEV	= NULL;
	USBH_HUB_CLASS_Structure	HUB_CLASS;
	uint8_t										device_id	= 0xFF;
	
	memset(&HUB_CLASS, 0, sizeof(USBH_HUB_CLASS_Structure));
	
#if defined (CONFIG_USBH_FREE_RTOS)
	xQueueReceive(usbh_queue_hub_drv, &device_id, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osMessageGet(usbh_queue_hub_drv, &device_id, USBH_MAX);
#endif
	
	while(1){
		HUB_DBG("HUB TASK ALIVE	!");
		HUB_DEV	= (USBH_Device_Structure*)usbh_hub_init(device_id);
		if(HUB_DEV == NULL) {
			while(1) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_100ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_100ms);
#endif
			}
		}
		HUB_DEV->CLASS_STRUCT_PTR   =	(USBH_HUB_CLASS_Structure*)&HUB_CLASS;
		usbh_hub_enum(HUB_DEV, device_id);
	}
}

void usbh_hub_enum(USBH_Device_Structure *HUB_DEV, uint8_t hub_id)
{
	USBH_CX_XFR_REQ_Struct			CX_REQ;
	USBH_INT_XFR_REQ_Struct			INT_REQ;	
	USBH_HUB_CLASS_Structure		*HUB_CLASS			= NULL;
	USBH_Device_Structure				*TARGET_DEV			= NULL;
	USBH_Device_Structure				*temp_DEV				=	NULL;
	uint8_t											*INT_DATA_BUFF	=	NULL;	
	uint8_t											*HUB_DATA_BUFF	=	NULL;	
	uint8_t											STATUS					=	FAIL;
	uint32_t										port						=	0;
	uint8_t											i = 1, j = 0;	
	
	HUB_CLASS	=	(USBH_HUB_CLASS_Structure*)HUB_DEV->CLASS_STRUCT_PTR;
	while(1){
		switch(HUB_CLASS->SM){
			case USBH_HUB_INIT_STATE:
				HUB_DATA_BUFF		=	(uint8_t*)HUB_DEV->BUFF.ptr;
				INT_DATA_BUFF		=	(uint8_t*)HUB_DEV->BUFF.ptr+384;
				memset(&CX_REQ, 0, sizeof(CX_REQ));
			
				// SET_FEATURE - DEVICE_REMOTE_WAKEUP
				CX_REQ.CMD			= USBH_CX_CMD_CLASS_NoneData;
				CX_REQ.SIZE			=	0;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0x00;
				CX_REQ.CLASS[1]	=	0x03;
				CX_REQ.CLASS[2]	=	0x01;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	0x00;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){
					HUB_DBG("HUB Critical error");
					goto halt;			
				}
						
				// GET_HUB_DESCRIPTOR HUB Type,
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
				CX_REQ.SIZE			=	25;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0xA0;
				CX_REQ.CLASS[1]	=	0x06;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	0x29;
				CX_REQ.CLASS[4]	=	0x00;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){
					HUB_DBG("HUB Critical error");
					goto halt;			
				}
				memcpy(&HUB_CLASS->DES, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);
						
				// GET_STATUS -	USB 2.0	 Standard Status
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
				CX_REQ.SIZE			=	2;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0x80;
				CX_REQ.CLASS[1]	=	0x00;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	0x00;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){
					HUB_DBG("HUB Critical error");
					goto halt;			
				}
				memcpy(&HUB_DEV->STDSTS, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);
						
				// CLEAR_FEATURE ENDPOINT_HALT - Endpoint 1 IN
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_NoneData;
				CX_REQ.SIZE			=	0;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0x02;
				CX_REQ.CLASS[1]	=	0x01;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	0x81;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){
					HUB_DBG("HUB Critical error");
					goto halt;
				}
				// CLR_FEATURE PORT_POWER - P1 ~ P4
				for(port = 1; port < 5; port++){
					CX_REQ.CMD			=	USBH_CX_CMD_CLASS_NoneData;
					CX_REQ.SIZE			=	0;
					CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
					CX_REQ.CLASS[0]	=	0x23;
					CX_REQ.CLASS[1]	=	0x01;
					CX_REQ.CLASS[2]	=	0x08;
					CX_REQ.CLASS[3]	=	0x00;
					CX_REQ.CLASS[4]	=	port;
					CX_REQ.CLASS[5]	=	0x00;
					STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
					if(STATUS == FAIL){
						HUB_DBG("HUB Critical error");
						goto halt;		
					}
				}
				// Do Power off	for 500ms
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_500ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_500ms);
#endif
				// SET_FEATURE PORT_POWER - P1 ~ P4
				for(port = 1; port < 5; port++){
					CX_REQ.CMD			=	USBH_CX_CMD_CLASS_NoneData;
					CX_REQ.SIZE			=	0;
					CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
					CX_REQ.CLASS[0]	=	0x23;
					CX_REQ.CLASS[1]	=	0x03;
					CX_REQ.CLASS[2]	=	0x08;
					CX_REQ.CLASS[3]	=	0x00;
					CX_REQ.CLASS[4]	=	port;
					CX_REQ.CLASS[5]	=	0x00;
					STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
					if(STATUS == FAIL){
						HUB_DBG("HUB Critical error");
						goto halt;		
					}
				}
				HUB_CLASS->SM =	USBH_HUB_DET_STATE;
						
				// Do TSIGATT+TATTDB ~=	200ms delay ( USB 2.0 SPEC 7.1.7.3 )
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_200ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_200ms);
#endif
				break;
			case USBH_HUB_DET_STATE:
				// DET HUB STATUS CHANGE
				INT_REQ.XfrType	=	EHCI_INT_IN_TYPE;
				INT_REQ.NUM			=	1;
				INT_REQ.SIZE		=	1;
				INT_REQ.pBUFF		=	(uint32_t *)INT_DATA_BUFF;	
				STATUS = usbh_int_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_INT_XFR_REQ_Struct*)&INT_REQ);
				if(STATUS == FAIL){
					HUB_DBG("HUB Critical error");
					goto halt;
				}else{
#if 0				
					HUB_CLASS->STS_CHANGE = *(INT_REQ.pBUFF);
					for(port=1;port<5;port++){
						if(HUB_CLASS->STS_CHANGE & (1<<port)) break;
					}
					TARGET_DEV = (USBH_Device_Structure*)&USBH_DEV[port];			
					HUB_CLASS->SM = USBH_HUB_STS_STATE;			
#endif				
					HUB_CLASS->STS_CHANGE |= *(INT_REQ.pBUFF);
					HUB_CLASS->SM = USBH_HUB_CONN_STATE; 
				}
				break;
			case USBH_HUB_CONN_STATE:
				for(port = 1; port < 5; port++){
					if(HUB_CLASS->STS_CHANGE & (1<<port)){
						// Get PORT STATUS 
						CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
						CX_REQ.SIZE			=	4;
						CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
						CX_REQ.CLASS[0]	=	0xA3;
						CX_REQ.CLASS[1]	=	0x00;
						CX_REQ.CLASS[2]	=	0x00;
						CX_REQ.CLASS[3]	=	0x00;
						CX_REQ.CLASS[4]	=	port;
						CX_REQ.CLASS[5]	=	0x00;
						STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
						if(STATUS == FAIL){	
							HUB_DBG("HUB Critical error");
							goto halt;
						}		
						memcpy(&HUB_CLASS->PORT_STS,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
						if(HUB_CLASS->PORT_STS.STS_C_PORT&STS_C_PORT_CONNECTION){
							CX_REQ.CMD			=	USBH_CX_CMD_CLASS_NoneData;
							CX_REQ.SIZE			=	0;
							CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;				
							CX_REQ.CLASS[0] =	0x23;
							CX_REQ.CLASS[1] =	0x01;
							CX_REQ.CLASS[2] =	0x10;
							CX_REQ.CLASS[3] =	0x00;
							CX_REQ.CLASS[4] =	port;
							CX_REQ.CLASS[5] =	0x00;				
							STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
							if(STATUS == FAIL){		
								HUB_DBG("HUB Critical error");
								goto halt;
							}				
						}
					}
				}
				HUB_CLASS->SM =	USBH_HUB_STS_STATE; 
				break;
			case USBH_HUB_STS_STATE:		
next:				
				for(port = 1; port < 5; port++){
					if(HUB_CLASS->STS_CHANGE & (1<<port)) break;
				}

				if(hub_id == 0) {
					TARGET_DEV = (USBH_Device_Structure*)&USBH_DEV[port];
				}
#if defined( CONFIG_DUAL_HOST )
				else {
					TARGET_DEV = (USBH_Device_Structure*)&USBH_DEV_2[port];
				}
#endif					
				// Get PORT STATUS 
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
				CX_REQ.SIZE			=	4;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0xA3;
				CX_REQ.CLASS[1]	=	0x00;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	port;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){	
					HUB_DBG("HUB Critical error");
					goto halt;
				}		
				memcpy(&HUB_CLASS->PORT_STS, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);		
			
				// IF STS Change
				if(HUB_CLASS->PORT_STS.STS_C_PORT){
					CX_REQ.CMD				= USBH_CX_CMD_CLASS_NoneData;
					CX_REQ.SIZE				= 0;
					CX_REQ.pBUFF			=	(uint32_t*)HUB_DATA_BUFF;
					if(HUB_CLASS->PORT_STS.STS_C_PORT&STS_C_PORT_CONNECTION){
						CX_REQ.CLASS[0]	=	0x23;
						CX_REQ.CLASS[1]	=	0x01;
						CX_REQ.CLASS[2]	=	0x10;
						CX_REQ.CLASS[3]	=	0x00;
						CX_REQ.CLASS[4]	=	port;
						CX_REQ.CLASS[5]	=	0x00;			
					} else if(HUB_CLASS->PORT_STS.STS_C_PORT&STS_C_PORT_RESET){
						CX_REQ.CLASS[0]	=	0x23;
						CX_REQ.CLASS[1]	=	0x01;
						CX_REQ.CLASS[2]	=	0x14;
						CX_REQ.CLASS[3]	=	0x00;
						CX_REQ.CLASS[4]	=	port;
						CX_REQ.CLASS[5]	=	0x00;			
					} else{
						HUB_DBG("Unknown case_1");   
						goto halt;
					}
					STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
					if(STATUS == FAIL){		
						HUB_DBG("HUB Critical error");
						goto halt;
					}
				}else{
					// Get PORT	STATUS & CLR STS CHG 
					CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
					CX_REQ.SIZE			=	4;
					CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
					CX_REQ.CLASS[0] =	0xA3;
					CX_REQ.CLASS[1] =	0x00;
					CX_REQ.CLASS[2] =	0x00;
					CX_REQ.CLASS[3] =	0x00;
					CX_REQ.CLASS[4] =	port;
					CX_REQ.CLASS[5] =	0x00;
					STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
					if(STATUS == FAIL){
						HUB_DBG("HUB Critical error");
						goto halt;
					}
					memcpy(&HUB_CLASS->PORT_STS, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);	
					if(HUB_CLASS->PORT_STS.STS_C_PORT){
						HUB_CLASS->SM =	USBH_HUB_STS_STATE;
					}else{
						if(HUB_CLASS->PORT_STS.STS_PORT_ENABLE == 1){
							for (i = 1; i < USBH_MAX_PORT; i++) {
								if(hub_id == 0) {
									temp_DEV = (USBH_Device_Structure*) &USBH_DEV[i];
								}
#if defined( CONFIG_DUAL_HOST )
								else {
									temp_DEV = (USBH_Device_Structure*) &USBH_DEV_2[i];
								}
#endif
								if (temp_DEV->bDevIsConnected == 0) {
#if defined (CONFIG_USBH_FREE_RTOS)
									for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
										if (temp_DEV->EP[j].SEM != NULL) {
											vSemaphoreDelete(temp_DEV->EP[j].SEM);
											temp_DEV->EP[j].SEM = NULL;
										}
										if (temp_DEV->EP[j].Mutex != NULL) {
											vSemaphoreDelete(temp_DEV->EP[j].Mutex);
											temp_DEV->EP[j].Mutex = NULL;
										}
									}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
									for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
										if (temp_DEV->EP[j].SEM != NULL) {
											osSemaphoreDelete(temp_DEV->EP[j].SEM);
											temp_DEV->EP[j].SEM = NULL;
										}
										if (temp_DEV->EP[j].Mutex != NULL) {
											osMutexDelete(temp_DEV->EP[j].Mutex);
											temp_DEV->EP[j].Mutex = NULL;
										}
									}
#endif	
									memset(temp_DEV, 0, sizeof(USBH_Device_Structure));
								}
							}

							if((temp_DEV == NULL) || (port > USBH_MAX_PORT-1)) {
								HUB_INFO("Not Support This Port!");
								HUB_CLASS->SM = USBH_HUB_DET_STATE;
							}
							else {
							TARGET_DEV->bDevIsConnected = 1;

							if(HUB_CLASS->PORT_STS.STS_PORT_HIGH_SPEED){
								TARGET_DEV->SPEED	=   EHCI_HIGH_SPEED;
							}else if(HUB_CLASS->PORT_STS.STS_PORT_LOW_SPEED){
								TARGET_DEV->SPEED	=   EHCI_LOW_SPEED;
							}else{
								TARGET_DEV->SPEED	=   EHCI_FULL_SPEED;
							}

							if(hub_id == 0) {
								usbh_parser_dev((USBH_Device_Structure*)TARGET_DEV, USBH_PARSER_CX_ENQU);
							}
#if defined( CONFIG_DUAL_HOST )							
							else {
								usbh_parser_dev_2((USBH_Device_Structure*)TARGET_DEV, USBH_PARSER_CX_ENQU);
							}
#endif
							TARGET_DEV->EP[0].pQH->bPortNumber	= port;
							TARGET_DEV->EP[0].pQH->bHubAddr			= 1;
							TARGET_DEV->device_id								= HUB_DEV->device_id + port;
							
							HUB_CLASS->SM = USBH_HUB_ENUM_STATE;			
							}
						}else if(HUB_CLASS->PORT_STS.STS_PORT_CONNECTION == 1){
							HUB_CLASS->SM = USBH_HUB_DEGLICH_STATE;
						}else if(HUB_CLASS->PORT_STS.STS_PORT_CONNECTION == 0){
							HUB_CLASS->SM = USBH_HUB_PLUG_OUT;						
						}else{
							HUB_DBG("Unknown case_2 ");
							goto halt;				
						}
					}			
				}
				break;
			case USBH_HUB_DEGLICH_STATE:
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_100ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_100ms);
#endif
				// Get PORT STATUS
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
				CX_REQ.SIZE			=	4;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0xA3;
				CX_REQ.CLASS[1]	=	0x00;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	port;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){
					HUB_DBG("HUB Critical error");
					goto halt;
				}		
				memcpy(&HUB_CLASS->PORT_STS, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);
				if(HUB_CLASS->PORT_STS.STS_C_PORT == 0){
					HUB_CLASS->SM = USBH_HUB_RESET_STATE;
				}else{
					HUB_CLASS->SM = USBH_HUB_STS_STATE;
				}
				break;
			case USBH_HUB_RESET_STATE:
				// Do Port Reset 
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_NoneData;
				CX_REQ.SIZE			=	0;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0x23;
				CX_REQ.CLASS[1]	=	0x03;
				CX_REQ.CLASS[2]	=	0x04;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	port;
				CX_REQ.CLASS[5]	=	0x00;			
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){	
					HUB_DBG("HUB Critical error");
					goto halt;
				}
				// Get PORT STATUS 
				CX_REQ.CMD			=	USBH_CX_CMD_CLASS_Read;
				CX_REQ.SIZE			=	4;
				CX_REQ.pBUFF		=	(uint32_t*)HUB_DATA_BUFF;
				CX_REQ.CLASS[0]	=	0xA3;
				CX_REQ.CLASS[1]	=	0x00;
				CX_REQ.CLASS[2]	=	0x00;
				CX_REQ.CLASS[3]	=	0x00;
				CX_REQ.CLASS[4]	=	port;
				CX_REQ.CLASS[5]	=	0x00;
				STATUS = usbh_cx_xfr((USBH_Device_Structure*)HUB_DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
				if(STATUS == FAIL){	
					HUB_DBG("HUB Critical error");
					goto halt;
				}
				memcpy(&HUB_CLASS->PORT_STS, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);			
				if((HUB_CLASS->PORT_STS.STS_PORT_RESET)	&& (HUB_CLASS->PORT_STS.STS_C_PORT == 0)){
					HUB_CLASS->SM = USBH_HUB_DET_STATE;			
				}else{
					HUB_CLASS->SM = USBH_HUB_STS_STATE;
				}
				break;
			case USBH_HUB_ENUM_STATE:
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_100ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_100ms);
#endif
				if(hub_id == 0) {
					STATUS = usbh_enum((USBH_Device_Structure*)TARGET_DEV, port+1);
				}
#if defined( CONFIG_DUAL_HOST )				
				else {
					STATUS = usbh_enum_2((USBH_Device_Structure*)TARGET_DEV, port+1);
				}
#endif
				if(STATUS == FAIL){
					HUB_DBG("HUB Port#%d ENUM FAIL",port);
					goto err;
				}
				HUB_CLASS->SM =	USBH_HUB_PARSER_STATE;
				break;
			case USBH_HUB_PARSER_STATE:
				if(hub_id == 0) {
					STATUS = usbh_parser_dev((USBH_Device_Structure*)TARGET_DEV, USBH_PARSER_NONCX_ENQU);
				}
#if defined( CONFIG_DUAL_HOST )
				else {
					STATUS = usbh_parser_dev_2((USBH_Device_Structure*)TARGET_DEV, USBH_PARSER_NONCX_ENQU);
				}
#endif
				if(STATUS == SUCCESS) {
					HUB_CLASS->SM =	USBH_HUB_ACTIVE_STATE;
				} else {
					HUB_DBG("Port %d Device Not Support!", port);
					HUB_CLASS->SM = USBH_HUB_DET_STATE;	
					
					HUB_CLASS->STS_CHANGE	&= ~(1<<port);
					if(HUB_CLASS->STS_CHANGE){
						goto next;
					}else{
						HUB_CLASS->SM = USBH_HUB_DET_STATE;	
					}
				}
				break;
			case USBH_HUB_ACTIVE_STATE:
				if(hub_id == 0) {
					usbh_parser_dev((USBH_Device_Structure*)TARGET_DEV,USBH_PARSER_WAKE_CLASS_DRV);
				}
#if defined( CONFIG_DUAL_HOST )
				else {
					usbh_parser_dev_2((USBH_Device_Structure*)TARGET_DEV,USBH_PARSER_WAKE_CLASS_DRV);
				}
#endif
				for(i = 1; i < MAX_QH_PER_DEVICE; i++) {
					if(TARGET_DEV->EP[i].pQH != NULL) {
						TARGET_DEV->EP[i].pQH->bPortNumber	= port;
						TARGET_DEV->EP[i].pQH->bHubAddr			= 1;
					}
				}
				
				HUB_CLASS->STS_CHANGE	&= ~(1<<port);
				if(HUB_CLASS->STS_CHANGE){
					goto next;
				}else{
					HUB_CLASS->SM = USBH_HUB_DET_STATE;	
				}
				break;		
			case USBH_HUB_PLUG_OUT:
				HUB_DBG("Detect Port %d Plug Out!!!", port);
				TARGET_DEV->bDevIsConnected = 0;
				if(hub_id == 0) {
					usbh_parser_dev(TARGET_DEV,USBH_PARSER_PORT_HUB_DEQ);
				}
#if defined( CONFIG_DUAL_HOST )
				else {
					usbh_parser_dev_2(TARGET_DEV,USBH_PARSER_PORT_HUB_DEQ);
				}
#endif
				HUB_CLASS->STS_CHANGE	&= ~(1<<port);
				HUB_CLASS->SM =	USBH_HUB_DET_STATE;
				break;			
			case USBH_HUB_ERR_HDL_STATE:
err:
				HUB_CLASS->STS_CHANGE	&= ~(1<<port);
				if(HUB_CLASS->STS_CHANGE){
					goto next;
				}else{
					HUB_CLASS->SM = USBH_HUB_DET_STATE;
				}
				break;
			case USBH_HUB_HALT_STATE:
halt:
			HUB_DBG("Please Re-PLUG HUB! \r\n");
			for(;;){
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_1000ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_1000ms);
#endif
			}
		}
	}
}
#endif




