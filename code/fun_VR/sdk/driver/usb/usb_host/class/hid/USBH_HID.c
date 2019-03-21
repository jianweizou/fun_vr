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
extern void flush_dcache_area(unsigned int start_addr, unsigned	int size);
#endif 

#if defined (CONFIG_MODULE_USB_HID_CLASS)
#if defined( CONFIG_SN_KEIL_SDK )
#include <stdlib.h>
#if defined( CONFIG_PLATFORM_ST53510 )
#include "INTC.h"
#endif 
#endif

#if defined( CONFIG_XILINX_SDK )
#include <stdlib.h>
#include <stdio.h>
#include <xil_cache.h>
#endif

#include <string.h>
#include "USBH.h"
#include "USBH_CORE.h"
#include "USBH_HID.h"

#if defined (CONFIG_USBH_FREE_RTOS)
//SemaphoreHandle_t USBH_SEM_WAKEUP_HID_DRV = NULL;
QueueHandle_t usbh_queue_hid_drv = NULL;
TaskHandle_t xTASK_HDL_HID_DRV[USBH_MAX_PORT*2] = {NULL};
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
osMessageQId	usbh_queue_hid_drv = NULL;
osThreadId		xTASK_HDL_HID_DRV[USBH_MAX_PORT*2] = {NULL};
#endif

uint8_t hid_transform_JOYSTICK_data(USBH_Device_Structure *HID_DEV, uint32_t dev_id, uint8_t *pData, char str[]) {

	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	uint8_t i = 0;
	char tempSTR[20] = {'\0'};

	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;
	
	sprintf(tempSTR, "dev_%d: ", dev_id);

	strcpy(str, tempSTR);

	//hid_dev->HID_JOYSTICK_MAP.joystickH = pData[0];
	//hid_dev->HID_JOYSTICK_MAP.joystickV = pData[1];
	HID_CLASS->HID_JOYSTICK_MAP.Button[0] = pData[0];
	HID_CLASS->HID_JOYSTICK_MAP.Button[1] = pData[1];

	HID_CLASS->HID_JOYSTICK_MAP.Button[2] = ((pData[2] & 0x04) != 0) ? 1 : 0;
	HID_CLASS->HID_JOYSTICK_MAP.Button[3] = ((pData[2] & 0x01) != 0) ? 1 : 0;
	HID_CLASS->HID_JOYSTICK_MAP.Button[4] = ((pData[2] & 0x02) != 0) ? 1 : 0;
	HID_CLASS->HID_JOYSTICK_MAP.Button[5] = ((pData[2] & 0x08) != 0) ? 1 : 0;

	HID_CLASS->HID_JOYSTICK_MAP.Button[6] = ((pData[2] & 0x40) != 0) ? 1 : 0;
	HID_CLASS->HID_JOYSTICK_MAP.Button[7] = ((pData[2] & 0x80) != 0) ? 1 : 0;

	sprintf(tempSTR, " 0x%x", HID_CLASS->HID_JOYSTICK_MAP.Button[0]);
	strcat(str, tempSTR);
	sprintf(tempSTR, " 0x%x", HID_CLASS->HID_JOYSTICK_MAP.Button[1]);
	strcat(str, tempSTR);

	for(i = 2; i < 8; i++) {
		sprintf(tempSTR, " %d", HID_CLASS->HID_JOYSTICK_MAP.Button[i]);
		strcat(str, tempSTR);
	}

	return SUCCESS;
}

uint8_t hid_transform_MOUSE_data(USBH_Device_Structure *HID_DEV, uint32_t dev_id, uint8_t* pData, char str[]) {
	uint16_t i = 0;
	uint8_t temp = 0;
	char tempSTR[20] = {'\0'};
	
	// Button
	for(i = 0; i < 3; i++) {
		if((*pData>>i) & 0x01) {
			switch(i) {
				case 0:
					strcpy(str, "L-BTN pressed\n");
				break;
				case 1:
					strcpy(str, "R-BTN pressed\n");
				break;
				case 2:
					strcpy(str, "Sroll pressed\n");
				break;
			}
		}
	}
	
	// X
	if(*(pData+1) != 0) {
		if(*(pData+1) & 0x80) { // negative
			temp = *(pData+1);
			temp ^= 0xFF;
			temp += 1;
			sprintf(tempSTR, "mouse move left %d steps\n", temp);
			strcat(str, tempSTR);
		}	else {
			temp = *(pData+1);
			sprintf(tempSTR, "mouse move right %d steps\n", temp);
			strcat(str, tempSTR);
		}
	}
	
	// Y
	if(*(pData+2) != 0) {
		if(*(pData+2) & 0x80) { // negative
			temp = *(pData+2);
			temp ^= 0xFF;
			temp += 1;
			sprintf(tempSTR, "mouse move down %d steps\n", temp);
			strcat(str, tempSTR);
		}	else {
			temp = *(pData+2);
			sprintf(tempSTR, "mouse move up %d steps\n", temp);
			strcat(str, tempSTR);
		}
	}
	
	// Scroll
	if(*(pData+3) != 0) {
		if(*(pData+3) & 0x80) { // negative
			temp = *(pData+3);
			temp ^= 0xFF;
			temp += 1;
			sprintf(tempSTR, "mouse scroll up %d steps\n", temp);
			strcat(str, tempSTR);
		} else {
			temp = *(pData+3);
			sprintf(tempSTR, "mouse scroll up %d steps\n", temp);
			strcat(str, tempSTR);
		}
	}

	return SUCCESS;
}


uint8_t hid_transform_INT_data(USBH_Device_Structure *HID_DEV, uint32_t dev_id, uint8_t* pData, char str[]) {
	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;
	
	switch(HID_CLASS->dev_type) {
//	case 1:
//		return hid_transform_MOUSE_data(hid_dev, dev_id, pData, str);
//		break;

	case 3:
		return hid_transform_JOYSTICK_data(HID_DEV, dev_id, pData, str);

	default:
		return FAIL;
	}
}

uint8_t hid_start_action(USBH_Device_Structure *HID_DEV) {

	USBH_HID_CLASS_Structure	*HID_CLASS	= NULL;
	USBH_INT_XFR_REQ_Struct	INT_REQ;
	uint8_t *INT_DATA_BUFF								= NULL;
	uint8_t status												= FAIL;

	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;;
	
	memset(&INT_REQ, 0, sizeof(USBH_INT_XFR_REQ_Struct));
	
	INT_DATA_BUFF			=	(uint8_t*)&HID_CLASS->INT_DATA_BUFF[0];
	INT_REQ.XfrType		=	EHCI_INT_IN_TYPE;
	INT_REQ.NUM				=	1;
	INT_REQ.SIZE			=	((HID_DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeHighByte<<8) | HID_DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeLowByte);
	INT_REQ.pBUFF			=	(uint32_t *)INT_DATA_BUFF;
	status = usbh_int_xfr((USBH_Device_Structure*)HID_DEV, (USBH_INT_XFR_REQ_Struct*)&INT_REQ);
	if(status == FAIL) {
		HID_DBG("HID INT ERR!");
	}
	else {
		if(usbh_HID_INT_IN_cb(USBH_HID_CLASS, HID_DEV->device_id, INT_DATA_BUFF) == FAIL) {
				HID_DBG("NO HID INT-IN CB FUNC");
		}
	}
	
	return status;
}

uint8_t hid_stop_action(USBH_Device_Structure *dev) {

	return SUCCESS;
}

//  sHID_INFO. u8bByteNum            // total HID data
//  sHID_INFO. u8bDevicetype         // 1: Keyboard 2: Mouse
//  sHID_INFO.u8bHIDValue[0]         // mouse button 1: left, 2:right, 4:scroll press
//  sHID_INFO.u8bHIDValue[1]         // X  logic maximum  = 2^X_RESOLUTION-1)-1 ,  logic minimum = 2's complement (2^X_ RESOLUTION-1)-1
//  sHID_INFO.u8bHIDValue[2]         // X
//  sHID_INFO.u8bHIDValue[3]         // Y  logic maximum  = 2^Y_RESOLUTION-1)-1 ,  logic minimum = 2's complement (2^Y_ RESOLUTION-1)-1
//  sHID_INFO.u8bHIDValue[4]         // Y
//  sHID_INFO.u8bHIDValue[5]         // Scroll 1: up 0xFF: down
//  sHID_INFO->X_RESOLUTION			 // X resolution
//  sHID_INFO->Y_RESOLUTION			 // Y resolution
void HID_INFO_REMAP(USBH_Device_Structure *HID_DEV, sHID_INFO_Structure *sHID_INFO) {

	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	sHID_INFO_Structure temp_sHID_INFO;
	HID_MOUSE_MAP_STRUCT *HID_MAP_TBL;
	uint32_t TEMP_DATA = 0;
	
	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;
	
	memset(&temp_sHID_INFO, 0, sizeof(sHID_INFO_Structure));	
	memcpy(&temp_sHID_INFO, sHID_INFO, sizeof(sHID_INFO_Structure));
	
	memset(&sHID_INFO->u8bHIDValue, 0, sizeof(sHID_INFO->u8bHIDValue));	
	
	HID_MAP_TBL = &HID_CLASS->HID_MOUSE_MAP;
	
	sHID_INFO->X_RESOLUTION		=	HID_MAP_TBL->X[2];
	sHID_INFO->Y_RESOLUTION		=	HID_MAP_TBL->Y[2];
	sHID_INFO->u8bHIDValue[0]  |= temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Left[0]] & HID_MAP_TBL->Left[1];
	sHID_INFO->u8bHIDValue[0]  |= temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Right[0]] & HID_MAP_TBL->Right[1];	
	sHID_INFO->u8bHIDValue[0]  |= temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Scroll_press[0]] & HID_MAP_TBL->Scroll_press[1];	
	
	
	TEMP_DATA = temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->X[0]+3]<<24 | temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->X[0]+2]<<16 | temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->X[0]+1]<<8 | temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->X[0]];
	TEMP_DATA = (TEMP_DATA>>HID_MAP_TBL->X[1]) & ((1<<HID_MAP_TBL->X[2])-1);
 	sHID_INFO->u8bHIDValue[1]  = (uint8_t)TEMP_DATA;
 	sHID_INFO->u8bHIDValue[2]  = (uint8_t)(TEMP_DATA>>8);	
	TEMP_DATA = temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Y[0]+3]<<24 | temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Y[0]+2]<<16 | temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Y[0]+1]<<8 | temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Y[0]];	
 	TEMP_DATA = (TEMP_DATA>>HID_MAP_TBL->Y[1]) & ((1<<HID_MAP_TBL->Y[2])-1);
 	sHID_INFO->u8bHIDValue[3]  = (uint8_t)TEMP_DATA;
 	sHID_INFO->u8bHIDValue[4]  = (uint8_t)(TEMP_DATA>>8);
	
	
	TEMP_DATA = temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Scroll[0]+3]<<24|temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Scroll[0]+2]<<16|temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Scroll[0]+1]<<8|temp_sHID_INFO.u8bHIDValue[HID_MAP_TBL->Scroll[0]];	
 	TEMP_DATA = (TEMP_DATA>>HID_MAP_TBL->Scroll[1])&((1<<HID_MAP_TBL->Scroll[2])-1);
 	sHID_INFO->u8bHIDValue[5]  = (uint8_t)TEMP_DATA;
 	sHID_INFO->u8bHIDValue[6]  = (uint8_t)(TEMP_DATA>>8);	
}

HID_MOUSE_MAP_STRUCT HID_RPT_TO_MOUSE_MAP(USBH_Device_Structure *HID_DEV){

	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	uint16_t *ptr = NULL;
	
	HID_MOUSE_MAP_STRUCT TEMP_TBL;
	uint16_t	DATA				= 0;
	uint8_t		SIZE				= 0;
	uint8_t		COUNT				= 0;
	uint16_t	BIT_COUNT		= 0;
	uint8_t		BIT_OFFSET	= 0;
	uint8_t		BYTE_INDEX	= 0;
	
	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;
	
	ptr = (uint16_t*)&HID_CLASS->u8HID_REPORT_DESC[0];

	memset(&TEMP_TBL, 0, sizeof(HID_MOUSE_MAP_STRUCT));
	
	ptr++;									// to avoid first Usage Page (0x0105)
	DATA = ptr[0];
	while(DATA){
		if((DATA&0x00FF) == 0x0085){		// Report ID
			BYTE_INDEX ++;
		}else if(DATA == 0x0905){			// BUTTON
			SIZE				= 0;
			COUNT				= 0;
			BIT_COUNT		= 0;
			TEMP_TBL.Left[0]					= BYTE_INDEX;
			TEMP_TBL.Left[1]					= 0x01;
			TEMP_TBL.Right[0]					= BYTE_INDEX;
			TEMP_TBL.Right[1]					= 0x02;
			TEMP_TBL.Scroll_press[0]	= BYTE_INDEX;
			TEMP_TBL.Scroll_press[1]	= 0x04;
			while(1){
				ptr++;
				DATA = ptr[0];		
				if((DATA&0x00FF) == 0x0075){		// find size & count
					SIZE = ((DATA&0xFF00)>>8);
					ptr++;
					DATA = ptr[0];						
					COUNT = ((DATA&0xFF00)>>8);
					BIT_COUNT += SIZE*COUNT;
				}else if((DATA&0x00FF) == 0x0095){
					COUNT = ((DATA&0xFF00)>>8);
					ptr++;
					DATA = ptr[0];
					SIZE = ((DATA&0xFF00)>>8);
					BIT_COUNT += SIZE*COUNT;
				}				
				if(DATA == 0x0105){					// find next use page 
					if(BIT_COUNT%8){
						HID_DBG("/r/n HID PARSER ERROR !!! /r/n!!");
					}else{
						BYTE_INDEX+= BIT_COUNT/8;
					}
					ptr--;							// back to previous ptr
					break;
				}
			}
		}else if(DATA == 0x0105){					// XY
			while(1){
				ptr ++;
				DATA = ptr[0];
				if((DATA&0x00FF) == 0x0075){
					BIT_COUNT = ((DATA&0xFF00)>>8);								
					break;
				}
			}
			BIT_OFFSET 						= 0;
			TEMP_TBL.X[0]					= BYTE_INDEX;
			TEMP_TBL.X[1]					= BIT_OFFSET;
			TEMP_TBL.X[2]					= BIT_COUNT;
			
			BYTE_INDEX 					 += ((BIT_COUNT+BIT_OFFSET)/8);
			TEMP_TBL.Y[0]					= BYTE_INDEX;			
			BIT_OFFSET  					= ((BIT_COUNT+BIT_OFFSET)%8);
			TEMP_TBL.Y[1]					= BIT_OFFSET;	
			TEMP_TBL.Y[2]					= BIT_COUNT;				
			
			BYTE_INDEX 					 += ((BIT_COUNT+BIT_OFFSET)/8);
			TEMP_TBL.Scroll[0]		= BYTE_INDEX;			
			BIT_OFFSET  					= ((BIT_COUNT+BIT_OFFSET)%8);			
			TEMP_TBL.Scroll[1]		= BIT_OFFSET;
			TEMP_TBL.Scroll[2]		= BIT_COUNT;
			
			if(TEMP_TBL.X[0])			return TEMP_TBL;
		}
		ptr ++;
		DATA = ptr[0];		
	}
	
	return TEMP_TBL;
}

uint8_t hid_get_dev_type(USBH_Device_Structure *HID_DEV) {
	
	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	uint16_t *ptr		=	NULL;
	uint16_t DATA		=	0;
	
	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;
	
	ptr		=	(uint16_t*)&HID_CLASS->u8HID_REPORT_DESC[0];

	ptr++;
	DATA = ptr[0];
	while(DATA){
		if(DATA == 0x0209){				// Mouse
			return 1;
		}
		else if(DATA == 0x0409) {		// JoyStick
			return 3;
		}
		
		ptr ++;
		DATA = ptr[0];
	}
	return 0;
}

uint8_t hid_parser_report_desc(USBH_Device_Structure *HID_DEV) {
	
	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	HID_MOUSE_MAP_STRUCT temp_mouse_map;
	
	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;
	
	memset(&temp_mouse_map, 0, sizeof(HID_MOUSE_MAP_STRUCT));
	
	HID_CLASS->dev_type = hid_get_dev_type(HID_DEV);

	switch (HID_CLASS->dev_type) {
//		case 1:
//			hid_dev->HID_MOUSE_MAP = HID_RPT_TO_MOUSE_MAP(hid_dev);
//	
//			if(memcmp(&hid_dev->HID_MOUSE_MAP, &temp_mouse_map, sizeof(HID_MOUSE_MAP_STRUCT)) == 0) {
//				return FAIL;
//			} else {
//				return SUCCESS;
//			}

	case 3:
		return SUCCESS;

	default:
		return FAIL;
	}
}

uint16_t hid_get_report_desc_len(USBH_Device_Structure *DEV) {
	
	uint8_t i = 0, j = 0;
	
	for	(i = 0;	i < DEV->DEVDes.bCONFIGURATION_NUMBER; i++) {
			for(j = 0;j < DEV->CFGDes[i].bINTERFACE_NUMBER; j++) {
				if (DEV->CFGDes[i].Interface[j].bInterfaceClass	== 3) {	//Only support 1 class
					return ((DEV->CFGDes[i].Interface[j].CLASSDes[0].bLengthHighByte << 8)
						| (DEV->CFGDes[i].Interface[j].CLASSDes[0].bLengthLowByte));
				}
			}
		}
	
	return 0;
}

uint8_t	hid_get_report_desc(USBH_Device_Structure *HID_DEV) {

	USBH_HID_CLASS_Structure	*HID_CLASS = NULL;
	
	USBH_CX_XFR_REQ_Struct CX_REQ;
	uint8_t status = FAIL;
	uint8_t	*SETUP_DATA_BUFF = NULL;
	
	HID_CLASS = (USBH_HID_CLASS_Structure*)HID_DEV->CLASS_STRUCT_PTR;

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do{
		SETUP_DATA_BUFF		=	pvPortMalloc(2048, GFP_DMA, MODULE_DRI_USBH);
	}while(SETUP_DATA_BUFF == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do{
		SETUP_DATA_BUFF 	=	pvPortMalloc(2048);
	}while(SETUP_DATA_BUFF == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS) 	
#if defined( CONFIG_SN_KEIL_SDK )
	SETUP_DATA_BUFF = (uint8_t *)malloc(2048);
#endif
#endif
	
	memset(&CX_REQ, 0, sizeof(CX_REQ));		
	
	// Get Report descriptor : Addr0 ENDP0 LEN=8
	CX_REQ.CMD	=	USBH_HID_CX_CMD_GETDESCRIPTOR_REPORT;
	CX_REQ.SIZE	=	hid_get_report_desc_len(HID_DEV);
	
	if(CX_REQ.SIZE == 0) {
		USBH_DBG("GET_REPORT_DESC FAIL,GOING TO ERROR HANDLING");
		goto ERR;
	}
	
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	status = usbh_cx_xfr((USBH_Device_Structure*)HID_DEV, (USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(status == FAIL){
		USBH_DBG("GET_REPORT_DESC FAIL,GOING TO ERROR HANDLING");
		goto ERR;
	}
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF, CX_REQ.ACT_SIZE);
	}
#endif
	//memcpy(hid_dev->p_report_desc_buff, CX_REQ.pBUFF, CX_REQ.ACT_SIZE);
	if(CX_REQ.ACT_SIZE <= REPORT_SIZE)
		memcpy(&HID_CLASS->u8HID_REPORT_DESC[0], CX_REQ.pBUFF, CX_REQ.ACT_SIZE);
	
ERR:
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(SETUP_DATA_BUFF);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(SETUP_DATA_BUFF);
#endif
	return status;
}

void hid_enum(USBH_Device_Structure *HID_DEV) {

#if defined( CONFIG_MODULE_USB_HID_DEMO )
	uint8_t status = FAIL;
#endif

	if (hid_get_report_desc(HID_DEV) == SUCCESS) {
		if(hid_parser_report_desc(HID_DEV) == SUCCESS) {
		} else {
			HID_DBG("Report Desc Error!");
			for (;;) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_1000ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_1000ms);
#endif
			}
		}
	} else {
		HID_DBG("Get Report Desc Error!"); 
		for (;;) {
			
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_1000ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_1000ms);
#endif
		}
	}
	
	HID_DBG("WAKE UP HID CLASS APP  !!");	

#if !defined( CONFIG_MODULE_USB_HID_DEMO )
			for (;;) {
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_1000ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_1000ms);
#endif
			}
#endif

#if defined( CONFIG_MODULE_USB_HID_DEMO )
		do {
			status = hid_start_action(HID_DEV);
		}while(status == SUCCESS);
#endif
}

#if defined (CONFIG_USBH_FREE_RTOS)
void hid_task(void *pvParameters)  {
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void hid_task(void const *pvParameters)  {
#endif
	USBH_Device_Structure			*HID_DEV = NULL;
	USBH_HID_CLASS_Structure	HID_CLASS;
	uint8_t										device_id = 0;
	
	memset(&HID_CLASS, 0, sizeof(USBH_HID_CLASS_Structure));

#if defined (CONFIG_USBH_FREE_RTOS)
	xQueueReceive(usbh_queue_hid_drv, &device_id, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osMessageGet(usbh_queue_hid_drv, &device_id, USBH_MAX);
#endif
	
	HID_DBG("HID ENUM TASK!!");	
	HID_DEV	= (USBH_Device_Structure*)usbh_hid_init(device_id);
	HID_DEV->CLASS_STRUCT_PTR = (USBH_HID_CLASS_Structure*)&HID_CLASS;
	hid_enum(HID_DEV);
	
	do{
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_100ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_100ms);
#endif
	}while(1);
}

#if defined( CONFIG_MODULE_USB_HID_DEMO )
void hid_recv_INT_data(uint32_t dev_id, uint8_t* INT_DATA_BUFF) {
	
	uint8_t i = 0;
	uint8_t joystick_data_buff[USBH_HID_JOYSTICK_DATA_SIZE] = {0};
	
	joystick_data_buff[0] = INT_DATA_BUFF[0];
	joystick_data_buff[1] = INT_DATA_BUFF[1];
		
	joystick_data_buff[2] = ((INT_DATA_BUFF[2] & 0x04) != 0) ? 1 : 0;
	joystick_data_buff[3] = ((INT_DATA_BUFF[2] & 0x01) != 0) ? 1 : 0;
	joystick_data_buff[4] = ((INT_DATA_BUFF[2] & 0x02) != 0) ? 1 : 0;
	joystick_data_buff[5] = ((INT_DATA_BUFF[2] & 0x08) != 0) ? 1 : 0;

	joystick_data_buff[6] = ((INT_DATA_BUFF[2] & 0x40) != 0) ? 1 : 0;
	joystick_data_buff[7] = ((INT_DATA_BUFF[2] & 0x80) != 0) ? 1 : 0;
		
	printf("dev_%d:  0x%x 0x%x", dev_id, joystick_data_buff[0], joystick_data_buff[1]);
	for(i = 2; i < USBH_HID_JOYSTICK_DATA_SIZE; i++) {
		printf(" %d", joystick_data_buff[i]);
	}
	printf("\n");
}
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
void hid_task_init(uint8_t device_id)
{	
	char task_name[256] = {'\0'};
	
	if(usbh_queue_hid_drv == NULL){
		usbh_queue_hid_drv	=	xQueueCreate(USBH_HID_DATA_QUEUE_SIZE, 4);
	}
	
	sprintf((char *)&task_name, "%s%d", "USBH_HID_TASK", device_id);
	
#if defined( CONFIG_MODULE_USB_HID_DEMO )
	usbh_HID_INT_cb_reg(USBH_HID_CLASS, hid_recv_INT_data);
#endif
	
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	xTaskCreate(
		hid_task,
		( const char * )&task_name[0],
		4096,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_HID_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
		hid_task,
		(const char *)&task_name[0],
		4096,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_HID_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
		hid_task,
		( const char * )&task_name[0],
		4096,
		(void*)&device_id,
		250,
		&xTASK_HDL_HID_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
		hid_task,
		( const char * )&task_name[0],
		2048,
		(void*)&device_id,
		20,
		&xTASK_HDL_HID_DRV[device_id]
	);		
#endif
		
#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_SN7320 )
	xTaskCreate(
		hid_task,
		( const char * )&task_name[0],
		512,
		(void*)&device_id,
		3,
		&xTASK_HDL_HID_DRV[device_id]
	);		
#endif

#if defined( CONFIG_XILINX_SDK ) && defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
	xTaskCreate(
		hid_task,
		( const char * )&task_name[0],
		4096,
		(void*)&device_id,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_HID_DRV[device_id]
	);
#endif  
}

void hid_task_uninit(uint8_t device_id)
{
	if(xTASK_HDL_HID_DRV[device_id] != NULL){
		vTaskDelete(xTASK_HDL_HID_DRV[device_id]);		
		xTASK_HDL_HID_DRV[device_id] = NULL;
	}

	if(usbh_queue_hid_drv != NULL) {
		vQueueDelete(usbh_queue_hid_drv);
		usbh_queue_hid_drv = NULL;
	}
	
#if defined( CONFIG_MODULE_USB_HID_DEMO )
	usbh_HID_INT_cb_UNreg(USBH_HID_CLASS, device_id);
#endif
}
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
void hid_task_init(uint8_t device_id)
{	
	char task_name[256] = {'\0'};
	
	if(usbh_queue_hid_drv == NULL){
		osMessageQDef(usbh_queue_hid_drv, USBH_HID_DATA_QUEUE_SIZE, uint32_t);
		usbh_queue_hid_drv = osMessageCreate(osMessageQ(usbh_queue_hid_drv), NULL);
	}
	
	sprintf((char *)&task_name, "%s%d", "USBH_HID_TASK", device_id);
	
#if defined( CONFIG_MODULE_USB_HID_DEMO )
	usbh_HID_INT_cb_reg(USBH_HID_CLASS, hid_recv_INT_data);
#endif
	
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	osThreadDef(task_name, hid_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HID_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HID_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HID_DRV[%d] fail\n", device_id);
#endif  

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(task_name, hid_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HID_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HID_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HID_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_ST53510 )
	osThreadDef(task_name, hid_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HID_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HID_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HID_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined(CONFIG_PLATFORM_SN7300 )
	osThreadDef(task_name, hid_task, THREAD_PRIO_USBH_PROCESS, 1, 2048);
	xTASK_HDL_HID_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HID_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HID_DRV[%d] fail\n", device_id);
#endif

#if defined( CONFIG_XILINX_SDK ) && defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
	osThreadDef(task_name, hid_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_HID_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_HID_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_HID_DRV[%d] fail\n", device_id);
#endif  
}

void hid_task_uninit(uint8_t device_id)
{
	if(xTASK_HDL_HID_DRV[device_id] != NULL){
		osThreadTerminate(xTASK_HDL_HID_DRV[device_id]);		
		xTASK_HDL_HID_DRV[device_id] = NULL;
	}

	if(usbh_queue_hid_drv != NULL) {
		osMutexDelete(usbh_queue_hid_drv);
		usbh_queue_hid_drv = NULL;
	}
	
#if defined( CONFIG_MODULE_USB_HID_DEMO )
	usbh_HID_INT_cb_UNreg(USBH_HID_CLASS, device_id);
#endif
}
#endif


#endif
