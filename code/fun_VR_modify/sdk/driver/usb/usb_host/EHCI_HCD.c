// $Source: $
// *******************************************************************
//	(C) Copyright 2015 by SONiX Technology Corp.
//
//		All Rights Reserved
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
#if defined( CONFIG_PLATFORM_ST53510 )
#include "INTC.h"
#endif
#endif

#if defined( CONFIG_XILINX_SDK )
#include <stdlib.h>
#include <xil_cache.h>
#endif

#include <string.h>

#include "USBH.h"
#include "EHCI_HCD.h"
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
#include "USBH_HUB.h"
#endif

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
#include "USBH_MSC.h"
#endif

#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#include "USBH_UVC.h"
#endif

#if defined (CONFIG_MODULE_USB_HID_CLASS)
#include "USBH_HID.h"
#endif

uint8_t *QH_STRUCTURE;
uint8_t *QTD_STRUCTURE;
uint8_t *ITD_STRUCTURE;
uint8_t *SXITD_STRUCTURE;

#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
uint8_t	reset_count = 0;
#endif

EHCI_PERIODIC_FRAME_LIST_STRUCT 	*FRAME_LIST;
volatile uint32_t 		*USBH_MEM_PTR;
volatile uint8_t 			EHCI_QH_Manage[EHCI_QH_MAX];
volatile uint8_t 			EHCI_QTD_Manage[EHCI_QTD_MAX];
volatile uint8_t 			EHCI_ITD_Manage[EHCI_ITD_MAX];
volatile uint8_t 			EHCI_SXITD_Manage[EHCI_SXITD_MAX];
volatile EHCI_PERIODIC_TABLE_Struct 	PERIODIC_TABLE;
volatile SONIX_EHCI_STRUCTURE 		*EHCI;
volatile EHCI_QH_STRUCTURE 		*pHEAD_QH;

#if defined( CONFIG_DUAL_HOST )	
uint8_t	*QH_STRUCTURE_2;
uint8_t	*QTD_STRUCTURE_2;
uint8_t	*ITD_STRUCTURE_2;
uint8_t	*SXITD_STRUCTURE_2;

EHCI_PERIODIC_FRAME_LIST_STRUCT		*FRAME_LIST_2;
volatile uint32_t			*USBH_MEM_PTR_2;
volatile uint8_t			EHCI_QH_Manage_2[EHCI_QH_MAX];
volatile uint8_t			EHCI_QTD_Manage_2[EHCI_QTD_MAX];
volatile uint8_t			EHCI_ITD_Manage_2[EHCI_ITD_MAX];
volatile uint8_t			EHCI_SXITD_Manage_2[EHCI_SXITD_MAX];
volatile EHCI_PERIODIC_TABLE_Struct	PERIODIC_TABLE_2;	
volatile SONIX_EHCI_STRUCTURE		*EHCI2;
volatile EHCI_QH_STRUCTURE  		*pHEAD_QH_2;
#endif // End of if defined( CONFIG_DUAL_HOST )

#if defined (CONFIG_USBH_FREE_RTOS)
extern xTaskHandle xWiFiTask;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
extern osThreadId xWiFiTask;
#endif
//extern xQueueHandle WiFi_Task_Init(xQueueHandle xCstreamerEventQueue);
//extern void WiFi_Task_Init(void);
//extern void WiFi_Task_UnInit(void);

void ehci_hcd_init(void) {
	uint32_t *ptr = NULL;
    
	//Init EHCI	Register Address
	EHCI				=	(SONIX_EHCI_STRUCTURE*)EHCI_REG_BASE_ADDRESS;

	//Turn off PortPower
	EHCI->PORTSC[0].PP			=	0;

	// HCRESET
	EHCI->USBCMD.HCRESET			=	1;
	while(EHCI->USBCMD.HCRESET	== 1);

#if defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 ) 
	EHCI->USBMODE				=	0x03;
#endif
	
	//EHCI INT Init 
	EHCI->USBINTR.ASYNC_ADVANCE_INT_EN	=	1;
	EHCI->USBINTR.SYS_ERR_INT_EN		=	1;
	EHCI->USBINTR.ROLLOVER_INT_EN		=	0;
	EHCI->USBINTR.PO_CHG_INT_EN		=	0;
	EHCI->USBINTR.USB_ERR_INT_EN		=	1;
	EHCI->USBINTR.USB_INT_EN		=	1;
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
	EHCI->USBINTR.BUF_FULL_INT_EN		=	1;
#else
EHCI->USBINTR.BUF_FULL_INT_EN		=	0;
#endif
#endif

#if defined( CONFIG_PLATFORM_SN7320 )
	EHCI->USBINTR.BUF_FULL_INT_EN		=	0;
#endif

	// Special register	init
#if defined( CONFIG_PLATFORM_SN9866X ) || defined( CONFIG_PLATFORM_ST53510 ) || defined (CONFIG_PLATFORM_SN9868X) || defined( CONFIG_PLATFORM_SN7320 )
	EHCI->MISC.LEV_INT_EN 			= 	1;	// Keep LEVEL INT Alive , add by Hammer 20171003
#endif 	

	EHCI->USBCMD.INT_CTR			=	1;
	EHCI->USBCMD.PARK_EN			=	0;
	EHCI->USBCMD.PARK_CNT			=	3;
	EHCI->CONFIG_FLAG			=	1;

	// frame list size init
	EHCI->USBCMD.FRAME_CTR			=	2;
	
#if defined( CONFIG_PLATFORM_ST53510 ) || defined( CONFIG_PLATFORM_SN7320 )	
	EHCI->EC_CTL.DENOISE_EN			=	1;
#endif 

#if defined( CONFIG_PLATFORM_SN7320 )	
	EHCI->EC_CTL.UFC_EOF_TO_EN		=	1;
	EHCI->EC_CTL.SX_ACT_CLR_EN		=	1;
	EHCI->EC_CTL.SX_FB_ACT_CLR_EN		=	1;	
#endif 
		
	EHCI->PORTSC[0].PP			=	1;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	vTaskDelay(USBH_100ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osDelay(USBH_100ms);
#endif
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do{
		ptr = pvPortMalloc(EHCI_STRUCT_SIZE,GFP_DMA,MODULE_DRI_USBH);
	}while(ptr == NULL);	
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do{
		ptr = pvPortMalloc(EHCI_STRUCT_SIZE);
	}while(ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		if(RF_USBH_Buf_PTR == 0)
			RF_USBH_Buf_PTR = 0x900000;
		ptr = (uint32_t *)RF_USBH_Buf_PTR;
		RF_USBH_Buf_PTR += EHCI_STRUCT_SIZE;
#endif
#endif

	// Clear mem
	memset(ptr,0,EHCI_STRUCT_SIZE);
	
	// record origin allocate mem ptr 
	USBH_MEM_PTR	= ptr;
	
	// do 4K alignment
	ptr	= (uint32_t*)((((uint32_t)ptr)&0xFFFFF000)+0x1000); 
	
	ehci_struct_init(ptr);
	EHCI->USBCMD.ASYNC_EN		=	1;
}

void ehci_hcd_uninit(void) {

	// Disable USBH DMA
	EHCI->USBCMD.ASYNC_EN			= 0;
	EHCI->USBCMD.PERIODIC_EN 		= 0;

	// HCRESET
	EHCI->USBCMD.HCRESET 			= 1;
	while (EHCI->USBCMD.HCRESET == 1);

}

void ehci_struct_init(uint32_t *ptr) {
	uint32_t i;

#if defined (CONFIG_MODULE_USB_UVC_SRAM)
	QH_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 0K
	ptr += 0x400;	
	QTD_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 4K
	ptr += 0x400; 	
//	ITD_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 12K
//	ptr += 0x400; 	
	SXITD_STRUCTURE =	(uint8_t*)ptr;	// PTR + 16K
	ptr += 0x400; 		
	FRAME_LIST	=	(EHCI_PERIODIC_FRAME_LIST_STRUCT*)ptr;	// PTR + 20K
#else
	QH_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 0K
	ptr += 0x400;	
	QTD_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 4K
	ptr += 0x800; 	
	ITD_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 12K
	ptr += 0x400; 	
	SXITD_STRUCTURE	=	(uint8_t*)ptr;	// PTR + 16K
	ptr += 0x400; 		
	FRAME_LIST	=	(EHCI_PERIODIC_FRAME_LIST_STRUCT*)ptr;	// PTR + 20K
#endif
	
	memset((uint8_t*)&EHCI_QH_Manage,0,EHCI_QH_MAX);
	memset((uint8_t*)&EHCI_QTD_Manage,0,EHCI_QTD_MAX);
	memset((uint8_t*)&EHCI_ITD_Manage,0,EHCI_ITD_MAX);
	memset((uint8_t*)&EHCI_SXITD_Manage,0,EHCI_SXITD_MAX);
	memset((uint8_t*)&PERIODIC_TABLE,0,sizeof(PERIODIC_TABLE));
		
	// Init Head QH 
	EHCI_QH_Manage[0]				=	EHCI_MEM_USED;	// Reserve for Head
	pHEAD_QH					=	(EHCI_QH_STRUCTURE*)&QH_STRUCTURE[0];
	pHEAD_QH->bOverlay_Status			=	0x40;	//Set Halt bit
	pHEAD_QH->bHeadOfReclamationListFlag		=	1;	//Set Head
	pHEAD_QH->bType					=	EHCI_NEXT_TYPE_QH;
	pHEAD_QH->bNextQHDPointer			=	(((uint32_t) &QH_STRUCTURE[0]) >> 5);		
		
	// Init FrameList
	for	(i=0;i<Host20_Preiodic_Frame_List_MAX;i++){
		FRAME_LIST->sCell[i].bTerminal		=	1;
	}	

	// Init Async Schedule link	pointer
	EHCI->ASYNCLISTADDR.LPL			=	(((uint32_t) &QH_STRUCTURE[0]) >> 5);

	// Init Async Schedule link	pointer
	EHCI->PERIODICLISTBASE.BASE_ADDR		=	(((uint32_t) &FRAME_LIST[0]) >>	12);		
}

void ehci_struct_uninit(void){
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree((void *)USBH_MEM_PTR);
#endif
}

EHCI_ENDP_STRUCT ehci_cx_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq, uint8_t host) {
	EHCI_ENDP_STRUCT EP;
	EHCI_QH_STRUCTURE *pTempQH;
	EHCI_QTD_STRUCTURE *pQTD[MAX_QTD_PER_QH];

	memset(&EP, 0, sizeof(EP));

	// Allocate	QH
	AllocReq->XfrType = EHCI_CX_TYPE;
	AllocReq->NakCount = 15;
	AllocReq->StructType = EHCI_MEM_TYPE_QH;
	AllocReq->NextStructType = EHCI_NEXT_TYPE_QH;
	pTempQH				=	(EHCI_QH_STRUCTURE*)ehci_get_structure(AllocReq,host);

	// Allocate QTD for SETUP
	AllocReq->StructType = EHCI_MEM_TYPE_QTD;
	pQTD[0]				=	(EHCI_QTD_STRUCTURE*)ehci_get_structure(AllocReq,host);
	pQTD[0]->bDataToggle = 0;
	pQTD[0]->bPID = EHCI_SETUP_TOKEN;

	// Allocate	QTD for	DATA		
	pQTD[1]				=	(EHCI_QTD_STRUCTURE*)ehci_get_structure(AllocReq,host);
	pQTD[1]->bDataToggle = 1;
	if (AllocReq->DataDir == EHCI_DATA_IN) {
		pQTD[1]->bPID = EHCI_IN_TOKEN;
	} else {
		pQTD[1]->bPID = EHCI_OUT_TOKEN;
	}

	// Allocate	QTD for	STATUS	
	pQTD[2]				=	(EHCI_QTD_STRUCTURE*)ehci_get_structure(AllocReq,host);
	pQTD[2]->bDataToggle = 1;
	if (AllocReq->DataDir == EHCI_DATA_IN) {
		pQTD[2]->bPID = EHCI_OUT_TOKEN;
	} else {
		pQTD[2]->bPID = EHCI_IN_TOKEN;
	}
	pQTD[2]->bTerminate = 1;
	pQTD[2]->bAlternateTerminate = 1;

	// Link QH&QTD
	pTempQH->bOverlay_NextqTD = (uint32_t) pQTD[0] >> 5;
	pTempQH->bOverlay_AlternateqTD = (uint32_t) pQTD[0] >> 5;
	pQTD[0]->bNextQTDPointer = (uint32_t) pQTD[1] >> 5;
	pQTD[0]->bAlternateQTDPointer = (uint32_t) pQTD[1] >> 5;
	pQTD[1]->bNextQTDPointer = (uint32_t) pQTD[2] >> 5;
	pQTD[1]->bAlternateQTDPointer = (uint32_t) pQTD[2] >> 5;

	// Prepare return data
	EP.XfrType = AllocReq->XfrType;
	EP.DataSize = AllocReq->DataSize;
	EP.pQH = pTempQH;
	EP.pQTD[0] = pQTD[0];
	EP.pQTD[1] = pQTD[1];
	EP.pQTD[2] = pQTD[2];
	EP.LastQTD = 2;
	EP.EdNumber = AllocReq->Endp;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	EP.SEM		= xSemaphoreCreateCounting(1, 0);
	EP.Mutex	= xSemaphoreCreateMutex();
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreDef(ep_sem);
	EP.SEM    = osSemaphoreCreate(osSemaphore(ep_sem), 1);
	osSemaphoreWait(EP.SEM, USBH_10ms);
	
	osMutexDef(ep_mutex);
	EP.Mutex = osMutexCreate(osMutex(ep_mutex));
#endif

	return EP;
}

EHCI_ENDP_STRUCT ehci_bk_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq, uint8_t host) {
	EHCI_ENDP_STRUCT EP;
	EHCI_QH_STRUCTURE *pTempQH;
	EHCI_QTD_STRUCTURE *pQTD[MAX_QTD_PER_QH];
	uint8_t i, LastQTD;
	
	memset(&EP, 0, sizeof(EP));

	// Allocate	QH
	if (AllocReq->DataDir == EHCI_DATA_IN) {
		AllocReq->XfrType = EHCI_BK_IN_TYPE;
	} else {
		AllocReq->XfrType = EHCI_BK_OUT_TYPE;
	}
	AllocReq->NakCount = 15;
	AllocReq->StructType = EHCI_MEM_TYPE_QH;
	AllocReq->NextStructType = EHCI_NEXT_TYPE_QH;
	pTempQH				=	(EHCI_QH_STRUCTURE*)ehci_get_structure(AllocReq,host);

	// Allocate	QTD for	BK
	i = 0;
	for (;;) {
		AllocReq->StructType = EHCI_MEM_TYPE_QTD;
		pQTD[i]				=	(EHCI_QTD_STRUCTURE*)ehci_get_structure(AllocReq,host);
		pQTD[i]->bDataToggle = 0;
		if (AllocReq->DataDir == EHCI_DATA_IN) {
			pQTD[i]->bPID = EHCI_IN_TOKEN;
		} else {
			pQTD[i]->bPID = EHCI_OUT_TOKEN;
		}
		if (AllocReq->DataSize <= 20480) {
			break;
		} else {
			AllocReq->DataSize -= 20480;
			i++;
		}
	}
	LastQTD = i;
	// Link QH&QTD
	pTempQH->bOverlay_NextqTD = (uint32_t) pQTD[0] >> 5;
	pTempQH->bOverlay_AlternateqTD = (uint32_t) pQTD[0] >> 5;
	
	for (i = 0; i < LastQTD; i++) {
		pQTD[i]->bNextQTDPointer = (uint32_t) pQTD[i + 1] >> 5;
		pQTD[i]->bAlternateQTDPointer = (uint32_t) pQTD[LastQTD] >> 5;

		pQTD[i]->bTerminate = 0;
		pQTD[i]->bAlternateTerminate = 0;
	}
	
	pQTD[LastQTD]->bNextQTDPointer = (uint32_t) pQTD[LastQTD] >> 5;
	pQTD[LastQTD]->bAlternateQTDPointer = (uint32_t) pQTD[LastQTD] >> 5;
	pQTD[LastQTD]->bTerminate = 1;
	pQTD[LastQTD]->bAlternateTerminate = 0;

	pQTD[LastQTD]->bStatus &= ~BIT7;
	pQTD[LastQTD]->bTotalBytes = 0;
		
	// Prepare return data
	EP.XfrType = AllocReq->XfrType;
	EP.DataSize = AllocReq->DataSize;
	EP.pQH = pTempQH;
	for (i = 0; i < LastQTD + 1; i++) {
		EP.pQTD[i] = pQTD[i];
	};
	EP.LastQTD = LastQTD;
	EP.EdNumber = AllocReq->Endp;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	EP.SEM		= xSemaphoreCreateCounting(1, 0);
	EP.Mutex	= xSemaphoreCreateMutex();
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreDef(ep_sem);
	EP.SEM    = osSemaphoreCreate(osSemaphore(ep_sem), 1);
	osSemaphoreWait(EP.SEM, USBH_10ms);
	
	osMutexDef(ep_mutex);
	EP.Mutex = osMutexCreate(osMutex(ep_mutex));
#endif

	return EP;
}

EHCI_ENDP_STRUCT ehci_int_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq, uint8_t host) {
	EHCI_ENDP_STRUCT EP;
	EHCI_QH_STRUCTURE *pTempQH = NULL;
	EHCI_QTD_STRUCTURE *pQTD[MAX_QTD_PER_QH] = {NULL};
	uint8_t i = 0, LastQTD = 0;

	memset(&EP, 0, sizeof(EP));

	// Allocate	QH
	if (AllocReq->DataDir == EHCI_DATA_IN) {
		AllocReq->XfrType = USBH_INT_IN_TYPE;
	} else {
		AllocReq->XfrType = USBH_INT_OUT_TYPE;
	}
	AllocReq->NakCount = 0;
	AllocReq->StructType = EHCI_MEM_TYPE_QH;
	AllocReq->NextStructType = EHCI_NEXT_TYPE_QH;
	pTempQH				=	(EHCI_QH_STRUCTURE*)ehci_get_structure(AllocReq,host);

	// Allocate	QTD  for	INT
	i = 0;
	for (;;) {
		AllocReq->StructType = EHCI_MEM_TYPE_QTD;
		pQTD[i]			=	(EHCI_QTD_STRUCTURE*)ehci_get_structure(AllocReq,host);
		pQTD[i]->bDataToggle = 0;
		if (AllocReq->DataDir == EHCI_DATA_IN) {
			pQTD[i]->bPID = EHCI_IN_TOKEN;
		} else {
			pQTD[i]->bPID = EHCI_OUT_TOKEN;
		}
		if (AllocReq->DataSize <= 20480) {
			break;
		} else {
			AllocReq->DataSize -= 20480;
			i++;
		}
	}
	LastQTD = i;

	// Link QH&QTD
	pTempQH->bOverlay_NextqTD = (uint32_t) pQTD[0] >> 5;
	pTempQH->bOverlay_AlternateqTD = (uint32_t) pQTD[0] >> 5;
	for (i = 0; i < LastQTD; i++) {
		pQTD[i]->bNextQTDPointer = (uint32_t) pQTD[i + 1] >> 5;
		pQTD[i]->bAlternateQTDPointer = (uint32_t) pQTD[i + 1] >> 5;
	}

	pQTD[LastQTD]->bNextQTDPointer = (uint32_t) pQTD[LastQTD] >> 5;
	pQTD[LastQTD]->bAlternateQTDPointer = (uint32_t) pQTD[LastQTD] >> 5;
	
	pQTD[LastQTD]->bTerminate = 1;
	pQTD[LastQTD]->bAlternateTerminate = 1;

	// Prepare return data
	EP.XfrType = AllocReq->XfrType;
	EP.DataSize = AllocReq->DataSize;
	EP.pQH = pTempQH;
	for (i = 0; i < LastQTD + 1; i++) {
		EP.pQTD[i] = pQTD[i];
	};
	EP.LastQTD = LastQTD;
	EP.EdNumber = AllocReq->Endp;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	EP.SEM		= xSemaphoreCreateCounting(1, 0);
	EP.Mutex	= xSemaphoreCreateMutex();
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
osSemaphoreDef(ep_sem);
	EP.SEM    = osSemaphoreCreate(osSemaphore(ep_sem), 1);
	osSemaphoreWait(EP.SEM, USBH_10ms);
	
	osMutexDef(ep_mutex);
	EP.Mutex = osMutexCreate(osMutex(ep_mutex));
#endif
	
	return EP;
}

EHCI_ENDP_STRUCT ehci_iso_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq, uint8_t host) {

#if defined( CONFIG_SNX_ISO_ACCELERATOR )
	EHCI_ENDP_STRUCT EP;	
	memset(&EP,0,sizeof(EP));

	// Allocate	ITD
	if(AllocReq->DataDir == EHCI_DATA_IN) {
		AllocReq->XfrType = USBH_ISO_IN_TYPE;
	} else {
		AllocReq->XfrType = USBH_ISO_OUT_TYPE;
	}
	AllocReq->StructType = EHCI_MEM_TYPE_SXITD;
	AllocReq->NextStructType = EHCI_NEXT_TYPE_ITD;

	EP.pSXITD			=	(EHCI_SXITD_STRUCTURE*)ehci_get_structure(AllocReq,host);
	EP.pSXITD->bTerminate = 1;
	EP.pSXITD->bSX_EN = 1;
	EP.pSXITD->bDEV_ADDR = (AllocReq->Addr);
	EP.pSXITD->bEndPt = (AllocReq->Endp&0x7F);
	EP.pSXITD->bMAX_PK_SIZE = (AllocReq->MaxPktSize&0x7FF);
	EP.pSXITD->bMult = (AllocReq->MaxPktSize>>11);
	EP.pSXITD->bFID_Count = 0;

	// Prepare return data
	EP.XfrType = AllocReq->XfrType;
	EP.EdNumber = AllocReq->Endp;
	#if defined (CONFIG_USBH_FREE_RTOS)
	EP.SEM		= xSemaphoreCreateCounting(1, 0);
	EP.Mutex	= xSemaphoreCreateMutex();
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreDef(ep_sem);
	EP.SEM    = osSemaphoreCreate(osSemaphore(ep_sem), 1);
	osSemaphoreWait(EP.SEM, USBH_10ms);
	
	osMutexDef(ep_mutex);
	EP.Mutex = osMutexCreate(osMutex(ep_mutex));
#endif
	return EP;
#else
	EHCI_ENDP_STRUCT EP;
	uint8_t i;
	uint8_t io;
	uint16_t epaddr;
	uint8_t mult;
	uint16_t io_maxpksize;

	memset(&EP, 0, sizeof(EP));

	// Allocate	ITD
	if (AllocReq->DataDir == EHCI_DATA_IN) {
		AllocReq->XfrType = USBH_ISO_IN_TYPE;
		io = 1;
	} else {
		AllocReq->XfrType = USBH_ISO_OUT_TYPE;
		io = 0;
	}
	AllocReq->StructType = EHCI_MEM_TYPE_ITD;
	AllocReq->NextStructType = EHCI_NEXT_TYPE_ITD;

	epaddr = (((AllocReq->Endp & 0x7F) << 8) | (AllocReq->Addr));
	mult = (AllocReq->MaxPktSize >> 11)+1;
	io_maxpksize = ((io << 11) | (AllocReq->MaxPktSize & 0x7FF));
	for (i = 0; i < Standard_iTD_EP_Max_Count; i++) {
		EP.pITD[i] = (EHCI_ITD_STRUCTURE*) ehci_get_structure(AllocReq,host);
		EP.pITD[i]->ArrayBufferPointer_Word[0].bParameter = epaddr;
		EP.pITD[i]->ArrayBufferPointer_Word[1].bParameter = io_maxpksize;
		EP.pITD[i]->ArrayBufferPointer_Word[2].bParameter = mult;
	}
	// Prepare return data
	EP.XfrType = AllocReq->XfrType;
	EP.EdNumber = AllocReq->Endp;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	EP.SEM		= xSemaphoreCreateCounting(1, 0);
	EP.Mutex	= xSemaphoreCreateMutex();
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
osSemaphoreDef(ep_sem);
	EP.SEM    = osSemaphoreCreate(osSemaphore(ep_sem), 1);
	osSemaphoreWait(EP.SEM, USBH_10ms);
	
	osMutexDef(ep_mutex);
	EP.Mutex = osMutexCreate(osMutex(ep_mutex));
#endif
	
	return EP;
#endif
}

void ehci_release_structure(uint32_t StructType, uint32_t PTR, uint8_t host) {
	
	uint32_t i = 0, j = 0;
	uint8_t	*manage_ptr = NULL;	

	switch (StructType) {
	case EHCI_MEM_TYPE_QH:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_QH_Manage[0];				
				i =	((PTR -	(uint32_t)QH_STRUCTURE)/EHCI_QH_SIZE);				
			}
#if defined( CONFIG_DUAL_HOST )				
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_QH_Manage_2[0];				
				i =	((PTR -	(uint32_t)QH_STRUCTURE_2)/EHCI_QH_SIZE);				
			}			
#endif // End of if defined( CONFIG_DUAL_HOST )				
			
			if(	(i <= EHCI_QH_MAX) && (manage_ptr[i] == EHCI_MEM_USED) ){
				manage_ptr[i] = EHCI_MEM_FREE;
			}else{
				if(i > EHCI_QH_MAX) {
				USBH_DBG("QH Release Structure Fail!");
				} else {
					USBH_DBG("QH is original FREE!");
				}
			}
			break;
	case EHCI_MEM_TYPE_QTD:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_QTD_Manage[0];				
				i =	((PTR -	(uint32_t)QTD_STRUCTURE)/EHCI_QTD_SIZE);					
			}
#if defined( CONFIG_DUAL_HOST )				
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_QTD_Manage_2[0];				
				i =	((PTR -	(uint32_t)QTD_STRUCTURE_2)/EHCI_QTD_SIZE);					
			}		
#endif // End of if defined( CONFIG_DUAL_HOST )	
			
			if(	(i <= EHCI_QTD_MAX) && (manage_ptr[i]==EHCI_MEM_USED) ){
				manage_ptr[i]=EHCI_MEM_FREE;
				break;
			}else{
				if(i > EHCI_QTD_MAX) {
				USBH_DBG("QTD Release Structure	Fail!");
				} else {
					USBH_DBG("QTD is original FREE");
				}
			}
			break;
	case EHCI_MEM_TYPE_ITD:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_ITD_Manage[0];				
				i =	((PTR -	(uint32_t)ITD_STRUCTURE)/EHCI_ITD_SIZE);				
			}
#if defined( CONFIG_DUAL_HOST )				
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_ITD_Manage_2[0];				
				i =	((PTR -	(uint32_t)ITD_STRUCTURE_2)/EHCI_ITD_SIZE);				
			}			
#endif // End of if defined( CONFIG_DUAL_HOST )	
			
			for(j=i;j<Standard_iTD_EP_Max_Count;j++){		
				if( (j <= EHCI_ITD_MAX)	&& (manage_ptr[j]==EHCI_MEM_USED) ){
					manage_ptr[j]=EHCI_MEM_FREE;
				}else{
					if(j > EHCI_ITD_MAX) {
					USBH_DBG("ITD Release Structure Fail!");
				} else {
					USBH_DBG("ITD is original FREE!");
					}
				}		
			}
		break;
	case EHCI_MEM_TYPE_SXITD:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_SXITD_Manage[0];				
				i =	((PTR -	(uint32_t)SXITD_STRUCTURE)/EHCI_SXITD_SIZE);				
			}
#if defined( CONFIG_DUAL_HOST )				
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_SXITD_Manage_2[0];				
				i =	((PTR -	(uint32_t)SXITD_STRUCTURE_2)/EHCI_SXITD_SIZE);				
			}
#endif // End of if defined( CONFIG_DUAL_HOST )	
			
			if(	(i <= EHCI_SXITD_MAX) && (manage_ptr[i]==EHCI_MEM_USED) ){
				manage_ptr[i]=EHCI_MEM_FREE;
			}else{
				if(i > EHCI_SXITD_MAX) {
					USBH_DBG("SXITD	Release	Structure Fail!");
				} else {
					USBH_DBG("SXITD is original FREE!");
				}
		}
	}
}

uint32_t ehci_get_structure(EHCI_ALLOC_REQUEST_Struct *AllocReq, uint8_t host) {
	uint32_t i = 0;
	uint8_t bFound = 0;
	EHCI_QTD_STRUCTURE *pTempQTD = NULL;
	EHCI_QH_STRUCTURE *pTempQH = NULL;
	EHCI_ITD_STRUCTURE *pTempITD = NULL;
	EHCI_SXITD_STRUCTURE *pTempSXITD = NULL;
	uint8_t			*manage_ptr = NULL;

	switch (AllocReq->StructType) {
	case EHCI_MEM_TYPE_QH:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_QH_Manage[0];
				pTempQH = (EHCI_QH_STRUCTURE*)QH_STRUCTURE;
			}
#if defined( CONFIG_DUAL_HOST )				
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_QH_Manage_2[0];				
				pTempQH = (EHCI_QH_STRUCTURE*)QH_STRUCTURE_2;
			}
#endif // End of if defined( CONFIG_DUAL_HOST )				
		for (i = 0; i < EHCI_QH_MAX; i++) {
				//if (EHCI_QH_Manage[i]==EHCI_MEM_FREE) {
				if(manage_ptr[i] == EHCI_MEM_FREE){
				bFound = 1;
					//EHCI_QH_Manage[i]=EHCI_MEM_USED;
					manage_ptr[i] = EHCI_MEM_USED;
				break;
			}
		}
		if (bFound == 1) {
			if(host == USBH1){
				pTempQH = (EHCI_QH_STRUCTURE*) (QH_STRUCTURE + i * EHCI_QH_SIZE);
				}
#if defined( CONFIG_DUAL_HOST )	
				else if(host == USBH2){
				pTempQH = (EHCI_QH_STRUCTURE*) (QH_STRUCTURE_2 + i * EHCI_QH_SIZE);				
			}
#endif
			memset(pTempQH, 0, EHCI_QH_SIZE);
			pTempQH->bTerminate = 0;
			pTempQH->bType = AllocReq->NextStructType;
			pTempQH->bDeviceAddress = AllocReq->Addr;
			pTempQH->bEdNumber = AllocReq->Endp;
			pTempQH->bEdSpeed = AllocReq->Speed;
			if (AllocReq->XfrType == EHCI_CX_TYPE) {
				pTempQH->bDataToggleControl = 1;
			} else {
				pTempQH->bDataToggleControl = 0;
			}
			pTempQH->bMaxPacketSize = AllocReq->MaxPktSize;
			if ((AllocReq->Speed != EHCI_HIGH_SPEED) && (AllocReq->XfrType == EHCI_CX_TYPE)) {
				pTempQH->bControlEdFlag = 1;
			} else {
				pTempQH->bControlEdFlag = 0;
			}
			pTempQH->bNakCounter = AllocReq->NakCount;
			if (AllocReq->Speed != EHCI_HIGH_SPEED) {
				pTempQH->bInterruptScheduleMask = AllocReq->SMask;
				pTempQH->bSplitTransactionMask = AllocReq->CMask;
				pTempQH->bHubAddr = AllocReq->HubAddr;
				pTempQH->bPortNumber = AllocReq->PortNumber;
			}
			pTempQH->bOverlay_Status = 0x40;
			return ((uint32_t) pTempQH);
		}
		break;
	case EHCI_MEM_TYPE_QTD:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_QTD_Manage[0];
				pTempQTD = (EHCI_QTD_STRUCTURE*)QTD_STRUCTURE;
			}
#if defined( CONFIG_DUAL_HOST )			
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_QTD_Manage_2[0];				
				pTempQTD = (EHCI_QTD_STRUCTURE*)QTD_STRUCTURE_2;
			}			
#endif // End of if defined( CONFIG_DUAL_HOST )				
		for (i = 0; i < EHCI_QTD_MAX; i++) {
				//if (EHCI_QTD_Manage[i]==EHCI_MEM_FREE) {
				if(manage_ptr[i] == EHCI_MEM_FREE){				
				bFound = 1;
					//EHCI_QTD_Manage[i]=EHCI_MEM_USED;
					manage_ptr[i] = EHCI_MEM_USED;
				break;
			}
		}
		if (bFound == 1) {
			//pTempQTD=(EHCI_QTD_STRUCTURE*)(QTD_STRUCTURE+i*EHCI_QTD_SIZE);				
			pTempQTD=(EHCI_QTD_STRUCTURE*)(pTempQTD+i);
			memset(pTempQTD, 0, EHCI_QTD_SIZE);
			pTempQTD->bStatus = 0x80;
			pTempQTD->bErrorCounter = 3;
			return ((uint32_t) pTempQTD);
		}
		break;
	case EHCI_MEM_TYPE_ITD:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_ITD_Manage[0];
				pTempITD = (EHCI_ITD_STRUCTURE*)ITD_STRUCTURE;
			}
#if defined( CONFIG_DUAL_HOST )			
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_ITD_Manage_2[0];
				pTempITD = (EHCI_ITD_STRUCTURE*)ITD_STRUCTURE_2;
			}	
#endif // End of if defined( CONFIG_DUAL_HOST )				
		for (i = 0; i < EHCI_ITD_MAX; i++) {
				//if (EHCI_ITD_Manage[i]==EHCI_MEM_FREE) {
				if(manage_ptr[i] == EHCI_MEM_FREE){				
				bFound = 1;
					//EHCI_ITD_Manage[i]=EHCI_MEM_USED;
					manage_ptr[i] = EHCI_MEM_USED;					
				break;
			}
		}
		if (bFound == 1) {
			//pTempITD=(EHCI_ITD_STRUCTURE*)(ITD_STRUCTURE+i*EHCI_ITD_SIZE);
			pTempITD=(EHCI_ITD_STRUCTURE*)(pTempITD+i);				
			memset(pTempITD, 0, EHCI_ITD_SIZE);
			return ((uint32_t) pTempITD);
		}
		break;
	case EHCI_MEM_TYPE_SXITD:
			if(host == USBH1){
				manage_ptr = (uint8_t *)&EHCI_SXITD_Manage[0];
				pTempSXITD = (EHCI_SXITD_STRUCTURE*)SXITD_STRUCTURE;
			}
#if defined( CONFIG_DUAL_HOST )				
			else if(host == USBH2){
				manage_ptr = (uint8_t *)&EHCI_SXITD_Manage_2[0];
				pTempSXITD = (EHCI_SXITD_STRUCTURE*)SXITD_STRUCTURE_2;
			}			
#endif // End of if defined( CONFIG_DUAL_HOST )				
		for (i = 0; i < EHCI_SXITD_MAX; i++) {
				//if (EHCI_SXITD_Manage[i]==EHCI_MEM_FREE) {
				if(manage_ptr[i] == EHCI_MEM_FREE){				
				bFound = 1;
					//EHCI_SXITD_Manage[i]=EHCI_MEM_USED;
					manage_ptr[i] = EHCI_MEM_USED;	
				break;
			}
		}
		if (bFound == 1) {
			//pTempSXITD=(EHCI_SXITD_STRUCTURE*)(SXITD_STRUCTURE+i*EHCI_SXITD_SIZE);
			pTempSXITD=(EHCI_SXITD_STRUCTURE*)(pTempSXITD+i);				
			memset(pTempSXITD, 0, EHCI_SXITD_SIZE);
			return ((uint32_t) pTempSXITD);
		}
		break;
	default:
		return 0;
	}
	return 0;
}

#if defined (CONFIG_USBH_FREE_RTOS)
void ehci_xfr_chk_isr(void) {

	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint32_t i = 0, j = 0;

	for (i = 0; i < USBH_MAX_PORT; i++) {
		for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
			if (USBH_DEV[i].EP[j].XfrType != 0) {// filter Null type	& iso type
				if (USBH_DEV[i].EP[j].XfrType <= 5) {
					if ((USBH_DEV[i].EP[j].pQH->bOverlay_InterruptOnComplete) && ((USBH_DEV[i].EP[j].pQH->bOverlay_Status & 0xF0) == 0x00)) {
						USBH_DEV[i].EP[j].pQH->bOverlay_InterruptOnComplete = 0;
						USBH_DEV[i].EP[j].pQH->bOverlay_Status = 0x04;
                       
						xSemaphoreGiveFromISR(USBH_DEV[i].EP[j].SEM, &xHigherPriorityTaskWoken);
					}else if((USBH_DEV[i].EP[j].pQH->bOverlay_InterruptOnComplete == 0x00) && ((USBH_DEV[i].EP[j].pQH->bOverlay_Status & 0xF4) == 0x00) && (USBH_DEV[i].EP[j].pQH->bOverlay_TotalBytes != 0x00)){
						USBH_DEV[i].EP[j].pQH->bOverlay_Status = 0x04;
						// modify status to 0x04 to avoid bug that
						// bk short packet result in cx accidentally enter here

                        if(USBH_DEV[i].EP[j].XfrType != EHCI_CX_TYPE)
                            xSemaphoreGiveFromISR(USBH_DEV[i].EP[j].SEM, &xHigherPriorityTaskWoken);
					}
				} else if ((USBH_DEV[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE) || (USBH_DEV[i].EP[j].XfrType == EHCI_ISO_IN_TYPE)) {

#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined (CONFIG_SNX_ISO_ACCELERATOR)
					if(USBH_DEV[i].EP[j].LastFID != USBH_DEV[i].EP[j].pSXITD->bFID_Count) {
						uvc_process_data((USBH_Device_Structure *)&USBH_DEV[i],(EHCI_ENDP_STRUCT *)&USBH_DEV[i].EP[j]);
					}
#else 
					if (USBH_DEV[i].EP[j].LastITD != 0) {
						if ((USBH_DEV[i].EP[j].LastITD->ArrayStatus_Word[7].bInterruptOnComplete == 1) && (USBH_DEV[i].EP[j].LastITD->ArrayStatus_Word[7].bLength != 0xC00)) {
							xSemaphoreGiveFromISR(USBH_DEV[i].EP[j].SEM, &xHigherPriorityTaskWoken);
						}
					}
#endif
#endif
				}
#if !defined ( CONFIG_PLATFORM_SN7300 )	&& !defined ( CONFIG_PLATFORM_SN7320 )			
				vTaskSwitchContext();
#else
				taskYIELD();
//				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);				
#endif 				
			}
		}
	}
}

void ehci_err_chk_isr(void) {
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint32_t i = 0, j = 0;
	
	for (i = 0; i < USBH_MAX_PORT; i++) {
		for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
			if (USBH_DEV[i].EP[j].XfrType != 0) {// filter Null type	& iso type
				if (USBH_DEV[i].EP[j].XfrType <= 5) {
					if (USBH_DEV[i].EP[j].pQH->bOverlay_Status & 0x78) {
						xSemaphoreGiveFromISR(USBH_DEV[i].EP[j].SEM, &xHigherPriorityTaskWoken);
					}
				} else if ((USBH_DEV[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE)
						|| (USBH_DEV[i].EP[j].XfrType == EHCI_ISO_IN_TYPE)) {

#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined (CONFIG_SNX_ISO_ACCELERATOR)
							//if((USBH_DEV[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV[i].EP[j].pSXITD->bBabble_Det) || (USBH_DEV[i].EP[j].pSXITD->bUNDERFLOW)) {
							if((USBH_DEV[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV[i].EP[j].pSXITD->bBabble_Det)) {
								uvc_process_err_data((USBH_Device_Structure *)&USBH_DEV[i],(EHCI_ENDP_STRUCT *)&USBH_DEV[i].EP[j]);
              }

#if defined (CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
					if(USBH_DEV[i].EP[j].pSXITD->bUNDERFLOW) {
						xQueueSendFromISR(usbh_queue_uvc_xfr_ctrl, (EHCI_ENDP_STRUCT*)&USBH_DEV[i].EP[j], &xHigherPriorityTaskWoken);
						//EHCI->USBINTR.BUF_FULL_INT_EN = 0;
					}
#endif

#endif
#endif
				}
			}
		}
	}
}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void ehci_xfr_chk_isr(void) {

	uint32_t i = 0, j = 0;

	for (i = 0; i < USBH_MAX_PORT; i++) {
		for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
			if (USBH_DEV[i].EP[j].XfrType != 0) {// filter Null type	& iso type
				if (USBH_DEV[i].EP[j].XfrType <= 5) {
					if ((USBH_DEV[i].EP[j].pQH->bOverlay_InterruptOnComplete) && ((USBH_DEV[i].EP[j].pQH->bOverlay_Status & 0xF0) == 0x00)) {
						USBH_DEV[i].EP[j].pQH->bOverlay_InterruptOnComplete = 0;
						USBH_DEV[i].EP[j].pQH->bOverlay_Status = 0x04;

						osSemaphoreRelease(USBH_DEV[i].EP[j].SEM);
					}else if((USBH_DEV[i].EP[j].pQH->bOverlay_InterruptOnComplete == 0x00) && ((USBH_DEV[i].EP[j].pQH->bOverlay_Status & 0xF4) == 0x00) && (USBH_DEV[i].EP[j].pQH->bOverlay_TotalBytes != 0x00)){
						USBH_DEV[i].EP[j].pQH->bOverlay_Status = 0x04;
						// modify status to 0x04 to avoid bug that
						// bk short packet result in cx accidentally enter here

						osSemaphoreRelease(USBH_DEV[i].EP[j].SEM);
					}
				} else if ((USBH_DEV[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE) || (USBH_DEV[i].EP[j].XfrType == EHCI_ISO_IN_TYPE)) {

#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )					
					if(USBH_DEV[i].EP[j].LastFID != USBH_DEV[i].EP[j].pSXITD->bFID_Count) {
						uvc_process_data((USBH_Device_Structure *)&USBH_DEV[i],(EHCI_ENDP_STRUCT *)&USBH_DEV[i].EP[j]);
					} 
#else
					if (USBH_DEV[i].EP[j].LastITD != 0) {
						if ((USBH_DEV[i].EP[j].LastITD->ArrayStatus_Word[7].bInterruptOnComplete == 1) && (USBH_DEV[i].EP[j].LastITD->ArrayStatus_Word[7].bLength != 0xC00)) {
							osSemaphoreRelease(USBH_DEV[i].EP[j].SEM);
						}
					}
#endif
#endif
				}
			}
		}
	}
}

void ehci_err_chk_isr(void) {
	uint32_t i = 0, j = 0;
	
	for (i = 0; i < USBH_MAX_PORT; i++) {
		for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
			if (USBH_DEV[i].EP[j].XfrType != 0) {// filter Null type	& iso type
				if (USBH_DEV[i].EP[j].XfrType <= 5) {
					if (USBH_DEV[i].EP[j].pQH->bOverlay_Status & 0x78) {

						osSemaphoreRelease(USBH_DEV[i].EP[j].SEM);
					}
				} else if ((USBH_DEV[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE)
						|| (USBH_DEV[i].EP[j].XfrType == EHCI_ISO_IN_TYPE)) {

#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined (CONFIG_SNX_ISO_ACCELERATOR)					
					//if((USBH_DEV[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV[i].EP[j].pSXITD->bBabble_Det) || (USBH_DEV[i].EP[j].pSXITD->bUNDERFLOW) ) {
					if((USBH_DEV[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV[i].EP[j].pSXITD->bBabble_Det)) {
						uvc_process_err_data((USBH_Device_Structure *)&USBH_DEV[i],(EHCI_ENDP_STRUCT *)&USBH_DEV[i].EP[j]);						
					}

#if defined (CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
					if(USBH_DEV[i].EP[j].pSXITD->bUNDERFLOW) {
						osMessagePut(usbh_queue_uvc_xfr_ctrl, (EHCI_ENDP_STRUCT*)&USBH_DEV[i].EP[j], 0);
						//EHCI->USBINTR.BUF_FULL_INT_EN = 0;
					}
#endif
					
#endif
#endif
				}
			}
		}
	}
}
#endif

#if defined( CONFIG_SN_GCC_SDK )
void ehci_isr(int irq)
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
void ehci_isr(void)
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
void USBHOST_IRQHandler(void)
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
void USB_HOST0_IRQHandler(void)
#endif 

#if defined( CONFIG_XILINX_SDK ) && defined ( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
void ehci_isr(void *HandlerRef)
#endif
{
#if defined (CONFIG_USBH_FREE_RTOS)
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif
	EHCI_USBSTS cur_USBSTS;
	cur_USBSTS = EHCI->USBSTS;
	EHCI->USBSTS = EHCI->USBSTS;
	
	if (cur_USBSTS.USB_INT == 1) {
		ehci_xfr_chk_isr();
	}
    
	if (cur_USBSTS.USB_ERR_INT == 1) {
		ehci_err_chk_isr();
	}
	
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
if((EHCI->USBINTR.BUF_FULL_INT_EN == 1) && (cur_USBSTS.BUF_FULL == 1)) {
		//EHCI->USBSTS.BUF_FULL = 1;
		//EHCI->USBINTR.BUF_FULL_INT_EN = 0;
		
		ehci_err_chk_isr();
	}
#endif
#endif

#if defined (CONFIG_PLATFORM_SN7320)
if((EHCI->USBINTR.BUF_FULL_INT_EN == 1) && (cur_USBSTS.BUF_FULL == 1)) {
		EHCI->USBINTR.BUF_FULL_INT_EN = 0;
		ehci_err_chk_isr();
	}
#endif

	if((cur_USBSTS.SYS_ERR_INT == 1) || (cur_USBSTS.HC_HALTED)){
		ehci_err_chk_isr();
	}
	if (cur_USBSTS.ROLLOVER_INT == 1) {
#if defined (CONFIG_USBH_FREE_RTOS)
		xSemaphoreGiveFromISR(USBH_SEM_FRAME_LIST_ROLLOVER,	&xHigherPriorityTaskWoken);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osSemaphoreRelease(USBH_SEM_FRAME_LIST_ROLLOVER);
#endif
	}
	if (cur_USBSTS.ASYNC_ADVANCE_INT == 1) {
#if defined (CONFIG_USBH_FREE_RTOS)
		xSemaphoreGiveFromISR(USBH_SEM_ASYNC_ADV, &xHigherPriorityTaskWoken);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osSemaphoreRelease(USBH_SEM_ASYNC_ADV);
#endif
	}
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if !defined ( CONFIG_PLATFORM_SN7300 )	&& !defined ( CONFIG_PLATFORM_SN7320 )		
		vTaskSwitchContext();
#else
		taskYIELD();
//		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
#endif 		
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	INTC_IrqClear(INTC_USB2HOST1_IRQ);
#if defined (CONFIG_USBH_FREE_RTOS)
	vTaskSwitchContext();
#endif
#endif 	
}

uint8_t ehci_chk_xfr_result(EHCI_ENDP_STRUCT *EP) {
	uint8_t Temp_Overlay_Status = 0;

	Temp_Overlay_Status = EP->pQH->bOverlay_Status;
	if ((Temp_Overlay_Status & 0x78) == 0) {
		EP->status = EHCI_QTD_STATUS_SUCCESS;
		return SUCCESS;
	}

	USBH_DBG("??? USB Error Interrupt Event...\n");

	if (Temp_Overlay_Status & EHCI_QTD_STATUS_Halted) {
		USBH_DBG("??? qHD Status => Halted ...");
		EP->status = EHCI_QTD_STATUS_Halted;
	}

	if (Temp_Overlay_Status & EHCI_QTD_STATUS_BufferError) {
		USBH_DBG("??? qHD Status => HOST20_qTD_STATUS_BufferError...");
		EP->status = EHCI_QTD_STATUS_BufferError;
	}

	if (Temp_Overlay_Status & EHCI_QTD_STATUS_Babble) {
		USBH_DBG("??? qHD Status => HOST20_qTD_STATUS_Babble...");
		EP->status = EHCI_QTD_STATUS_Babble;
	}

	if (Temp_Overlay_Status & EHCI_QTD_STATUS_TransactionError) {
		USBH_DBG("??? qHD Status => HOST20_qTD_STATUS_TransactionError...");
		EP->status = EHCI_QTD_STATUS_TransactionError;
	}

	if (Temp_Overlay_Status & EHCI_QTD_STATUS_MissMicroFrame) {
		USBH_DBG("??? qHD Status => HOST20_qTD_STATUS_MissMicroFrame...");
		EP->status = EHCI_QTD_STATUS_MissMicroFrame;
	}

	return FAIL;
}

void ehci_qh_list_for_each(EHCI_QH_LINK_LIST *ll, uint8_t host) {
	ll->link_tail = 0;
	ll->Num[0] = 0;
	if(host == USBH1){
		ll->pQH[0] = (EHCI_QH_STRUCTURE*) &QH_STRUCTURE[0];
	}
#if defined( CONFIG_DUAL_HOST )		
	else if(host == USBH2){
		ll->pQH[0]		=	(EHCI_QH_STRUCTURE*)&QH_STRUCTURE_2[0];			
	}
#endif // End of if defined( CONFIG_DUAL_HOST )	
	do {
		if ((uint32_t) ll->pQH[ll->link_tail]->bNextQHDPointer << 5 == (uint32_t) ll->pQH[0])
			break;
		ll->link_tail++;
		ll->Num[ll->link_tail] = ll->link_tail;
		ll->pQH[ll->link_tail] = (EHCI_QH_STRUCTURE*) ((uint32_t) ll->pQH[ll->link_tail - 1]->bNextQHDPointer << 5);
	} while (ll->link_tail < 10);
}

void ehci_adding_qh_tail(EHCI_QH_STRUCTURE *ADD_QH, uint8_t host) {
	EHCI_QH_STRUCTURE *Prev_QH;
	EHCI_QH_LINK_LIST ll;
	memset(&ll, 0, sizeof(ll));

	ehci_qh_list_for_each((EHCI_QH_LINK_LIST*)&ll,host);
	if (ll.link_tail > 0) {
		Prev_QH = (EHCI_QH_STRUCTURE*) (ll.pQH[ll.link_tail]);
	} else {
		Prev_QH = (EHCI_QH_STRUCTURE*) (ll.pQH[0]);
	}
	ADD_QH->bNextQHDPointer = (uint32_t) ll.pQH[0] >> 5;
	Prev_QH->bNextQHDPointer = (uint32_t) ADD_QH >> 5;
	ehci_qh_list_for_each((EHCI_QH_LINK_LIST*)&ll,host);
}

void ehci_removing_qh(EHCI_QH_STRUCTURE *RMV_QH, uint8_t CHK_ADV, uint8_t host) {
	uint32_t i;
	EHCI_QH_STRUCTURE *Prev_QH;
	EHCI_QH_LINK_LIST ll;
	memset(&ll, 0, sizeof(ll));
	ehci_qh_list_for_each((EHCI_QH_LINK_LIST*)&ll,host);
	for (i = 0; i < ll.link_tail; i++) {
		if (RMV_QH == ll.pQH[i])
			break;
	}
	Prev_QH = ll.pQH[i - 1];
	Prev_QH->bNextQHDPointer = RMV_QH->bNextQHDPointer;
	if (CHK_ADV) {
		if(host == USBH1){
			EHCI->USBCMD.DOORBELL_EN	=	1;
		}
#if defined( CONFIG_DUAL_HOST )			
		else if(host == USBH2){
			EHCI2->USBCMD.DOORBELL_EN = 1;
		}
#endif // End of if defined( CONFIG_DUAL_HOST )		
		ehci_qh_list_for_each((EHCI_QH_LINK_LIST*)&ll,host);
		if(host == USBH1){
#if defined (CONFIG_USBH_FREE_RTOS)
			if (xSemaphoreTake(USBH_SEM_ASYNC_ADV, USBH_100ms ) == pdFAIL) {
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			if (osSemaphoreWait(USBH_SEM_ASYNC_ADV, USBH_100ms ) != osOK) {
#endif
				USBH_DBG("ASYNC ADVANCE FAIL");
			}
		}
#if defined( CONFIG_DUAL_HOST )			
		else if(host == USBH2){
#if defined (CONFIG_USBH_FREE_RTOS)
			if (xSemaphoreTake(USBH_SEM_ASYNC_ADV_2, USBH_100ms ) == pdFAIL) {
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			if (osSemaphoreWait(USBH_SEM_ASYNC_ADV_2, USBH_100ms ) != osOK) {
#endif
				USBH_DBG("ASYNC ADVANCE_2 FAIL");
			}
		}
#endif
		if(host == USBH1){
			EHCI->USBCMD.DOORBELL_EN	=	0;
		}
#if defined( CONFIG_DUAL_HOST )			
		else if(host == USBH2){
			EHCI2->USBCMD.DOORBELL_EN	=	0;			
		}		
#endif // End of if defined( CONFIG_DUAL_HOST )			
	} else {
		ehci_qh_list_for_each((EHCI_QH_LINK_LIST*)&ll,host);
	}
}

uint32_t order2power(uint32_t order) {
	uint32_t i;
	uint32_t temp = 1;

	for (i = 0; i < 11; i++) {
		if ((temp << i) >= order)
			return (temp << i);
	};
	return 0;
}

void ehci_link_framelist(void) {
	uint32_t i, j, k;
	uint32_t curr_order, next_order;
	uint32_t curr_type, next_type;
	uint32_t curr_index;
	EHCI_QH_STRUCTURE *QH = NULL;
	EHCI_ITD_STRUCTURE *ITD = NULL;
	EHCI_SXITD_STRUCTURE *SXITD = NULL;

	//link by interval 
	if (PERIODIC_TABLE.link_tail <= 1) {			// no next interval
		curr_type = PERIODIC_TABLE.element[0].type;
		if (curr_type == EHCI_INT) {
			QH = (EHCI_QH_STRUCTURE*) PERIODIC_TABLE.element[0].ptr;
			QH->bTerminate = 1;
		} else if (curr_type == EHCI_ISO) {
			for (k = 0; k < Standard_iTD_EP_Max_Count; k++) {
				ITD = (EHCI_ITD_STRUCTURE*) (((uint32_t) PERIODIC_TABLE.element[0].ptr) + (EHCI_ITD_SIZE * k));
				ITD->bTerminate = 1;
			}
		} else if (curr_type == EHCI_SXISO) {
			SXITD = (EHCI_SXITD_STRUCTURE*) PERIODIC_TABLE.element[0].ptr;
			SXITD->bTerminate = 1;
		}
	} else {
		for (i = 0; i < Host20_Preiodic_Frame_List_MAX; i++) {// do next interval
			curr_index = 0;
			curr_order = PERIODIC_TABLE.element[curr_index].order;
			if (!(i % curr_order)) {
				for (j = 0; j < PERIODIC_TABLE.link_tail; j++) {
					curr_type = PERIODIC_TABLE.element[curr_index].type;
					if (curr_type == EHCI_INT) {
						QH = (EHCI_QH_STRUCTURE*) PERIODIC_TABLE.element[curr_index].ptr;
					} else if (curr_type == EHCI_ISO) {
						ITD = (EHCI_ITD_STRUCTURE*) ((uint32_t) (PERIODIC_TABLE.element[curr_index].ptr) + (EHCI_ITD_SIZE * (i % Standard_iTD_EP_Max_Count)));
					} else if (curr_type == EHCI_SXISO) {
						SXITD = (EHCI_SXITD_STRUCTURE*) PERIODIC_TABLE.element[curr_index].ptr;
					}
					next_order = PERIODIC_TABLE.element[j + 1].order;
					if (next_order == 0) {
						if (curr_type == EHCI_INT) {
							QH->bTerminate = 1;
						} else if (curr_type == EHCI_ISO) {
							ITD->bTerminate = 1;
						} else if (curr_type == EHCI_SXISO) {
							SXITD->bTerminate = 1;
						}
						break;
					}
					if (!(i % next_order)) {
						switch (curr_type) {
						case EHCI_INT:
							next_type = PERIODIC_TABLE.element[j + 1].type;
							if (next_type == EHCI_INT) {
								QH->bType = EHCI_QH_TYPE;
								QH->bNextQHDPointer = (((uint32_t) PERIODIC_TABLE.element[j + 1].ptr) >> 5);
								QH->bTerminate = 0;
							} else if (next_type == EHCI_SXISO) {
								QH->bType = EHCI_ITD_TYPE;
								QH->bNextQHDPointer = (((uint32_t) PERIODIC_TABLE.element[j + 1].ptr) >> 5);
								QH->bTerminate = 0;
							} else {
								QH->bTerminate = 1;
							}
							break;
						case EHCI_ISO:
							next_type = PERIODIC_TABLE.element[j + 1].type;
							if (next_type == EHCI_INT) {
								ITD->bType = EHCI_QH_TYPE;
								ITD->bNextLinkPointer = (((uint32_t) PERIODIC_TABLE.element[j + 1].ptr) >> 5);
								ITD->bTerminate = 0;
							} else if (next_type == EHCI_ISO) {
								ITD->bType = EHCI_ITD_TYPE;
								ITD->bNextLinkPointer = (((uint32_t) PERIODIC_TABLE.element[j + 1].ptr) + (EHCI_ITD_SIZE * (i % Standard_iTD_EP_Max_Count)) >> 5);
								ITD->bTerminate = 0;
							} else if (next_type == EHCI_SXISO) {
								ITD->bType = EHCI_ITD_TYPE;
								ITD->bNextLinkPointer = (((uint32_t) PERIODIC_TABLE.element[j + 1].ptr) >> 5);
								ITD->bTerminate = 0;
							} else {
								ITD->bTerminate = 1;
							}
							break;
						case EHCI_SXISO:
							next_type = PERIODIC_TABLE.element[j + 1].type;
							if (next_type == EHCI_SXISO) {
								SXITD->bType = EHCI_ITD_TYPE;
								SXITD->bNextLinkPointer = (((uint32_t) PERIODIC_TABLE.element[j + 1].ptr) >> 5);
								SXITD->bTerminate = 0;
							} else {
								SXITD->bTerminate = 1;
							}
							break;
						default:
							USBH_DBG("Relink Err ! ");
							break;
						}
						curr_index = j + 1;
					}
				}
			}
		}
	}

	// link to framelist 
	if (PERIODIC_TABLE.link_tail == 0) {	// Table empty
		for (i = 0; i < Host20_Preiodic_Frame_List_MAX; i++) {
			FRAME_LIST->sCell[i].bTerminal = 1;
		}
	} else {
		for (i = 0; i < Host20_Preiodic_Frame_List_MAX; i++) {
			for (j = 0; j < PERIODIC_TABLE.link_tail; j++) {
				curr_order = PERIODIC_TABLE.element[j].order;
				if (!(i % curr_order)) {
					if (PERIODIC_TABLE.element[j].type == EHCI_INT) {
						FRAME_LIST->sCell[i].bLinkPointer = (((uint32_t) PERIODIC_TABLE.element[j].ptr) >> 5);
						FRAME_LIST->sCell[i].bType = EHCI_QH_TYPE;
					} else if (PERIODIC_TABLE.element[j].type == EHCI_ISO) {
						FRAME_LIST->sCell[i].bLinkPointer = ((((uint32_t) PERIODIC_TABLE.element[j].ptr) + (EHCI_ITD_SIZE * (i % Standard_iTD_EP_Max_Count))) >> 5);
						FRAME_LIST->sCell[i].bType = EHCI_ITD_TYPE;
					} else if (PERIODIC_TABLE.element[j].type == EHCI_SXISO) {
						FRAME_LIST->sCell[i].bLinkPointer = (((uint32_t) PERIODIC_TABLE.element[j].ptr) >> 5);
						FRAME_LIST->sCell[i].bType = EHCI_ITD_TYPE;
					}
					FRAME_LIST->sCell[i].bTerminal = 0;
					break;
				} else {
					FRAME_LIST->sCell[i].bTerminal = 1;
				}
			}
		}
	}
}

void ehci_add_periodic_table(uint32_t type, uint32_t order, uint32_t *ptr) {
	uint32_t i, j;
	uint32_t curr_order = 0;
	uint32_t curr_type = 0;
	uint32_t curr_index = 0;
	EHCI_PERIODIC_ELEMENT_Struct temp_element;

	// add  
	curr_index = PERIODIC_TABLE.link_tail;
	PERIODIC_TABLE.element[curr_index].type = type;
	if (type == EHCI_INT) {
		curr_order = order2power(order);
	} else if (type == EHCI_ISO) {
		curr_order = 0xFFF;
	} else if (type == EHCI_SXISO) {
		curr_order = 0x0;
	}
	PERIODIC_TABLE.element[curr_index].order = curr_order;
	PERIODIC_TABLE.element[curr_index].ptr = ptr;
	PERIODIC_TABLE.link_tail++;

	// sorting 
	for (i = 0; i < PERIODIC_TABLE.link_tail; i++) {
		for (j = i + 1; j < PERIODIC_TABLE.link_tail; j++) {
			if (PERIODIC_TABLE.element[i].order < PERIODIC_TABLE.element[j].order) {
				temp_element = PERIODIC_TABLE.element[j];
				PERIODIC_TABLE.element[j] = PERIODIC_TABLE.element[i];
				PERIODIC_TABLE.element[i] = temp_element;
			}
		}
	}

	// fix iso order 
	for (i = 0; i < PERIODIC_TABLE.link_tail; i++) {
		curr_type = PERIODIC_TABLE.element[i].type;
		if ((curr_type == EHCI_ISO) || (curr_type == EHCI_SXISO)) {
			PERIODIC_TABLE.element[i].order = 1;
		}
	}

	//enable periodic schedule
	if (EHCI->USBCMD.PERIODIC_EN == 0) {
		EHCI->USBCMD.PERIODIC_EN = 1;

		vTaskDelay(USBH_2ms);
	}
    
	//Periodic table link to framelist
	ehci_link_framelist();
}

void ehci_remove_periodic_table(void *ptr) {
	uint32_t i = 0, j = 0;
	uint32_t curr_type = 0;
	EHCI_PERIODIC_ELEMENT_Struct temp_element;

	memset(&temp_element, 0, sizeof(EHCI_PERIODIC_ELEMENT_Struct));

	// find & remove
	for (i = 0; i < PERIODIC_TABLE.link_tail; i++) {
		if (PERIODIC_TABLE.element[i].type == EHCI_ISO) {
			PERIODIC_TABLE.element[i].order = 0xFFF;
		} else if (PERIODIC_TABLE.element[i].type == EHCI_SXISO) {
			PERIODIC_TABLE.element[i].order = 0;
		}
		if (PERIODIC_TABLE.element[i].ptr == ptr) {
			PERIODIC_TABLE.element[i].type = 0;
			PERIODIC_TABLE.element[i].order = 0;
			PERIODIC_TABLE.element[i].ptr = 0;
			break;
		}
	}

	// sorting 
	for (i = 0; i < PERIODIC_TABLE.link_tail; i++) {
		for (j = i + 1; j < PERIODIC_TABLE.link_tail; j++) {
			if (PERIODIC_TABLE.element[i].order < PERIODIC_TABLE.element[j].order) {
				temp_element = PERIODIC_TABLE.element[j];
				PERIODIC_TABLE.element[j] = PERIODIC_TABLE.element[i];
				PERIODIC_TABLE.element[i] = temp_element;
			}
		}
	}
	PERIODIC_TABLE.link_tail--;

	// fix iso order 
	for (i = 0; i < PERIODIC_TABLE.link_tail; i++) {
		curr_type = PERIODIC_TABLE.element[i].type;
		if ((curr_type == EHCI_ISO) || (curr_type == EHCI_SXISO)) {
			PERIODIC_TABLE.element[i].order = 1;
		}
	}

	//Periodic table link to framelist
	ehci_link_framelist();
	
	if(PERIODIC_TABLE.link_tail	== 0){
		if(EHCI->USBCMD.PERIODIC_EN == 1){
			EHCI->USBCMD.PERIODIC_EN	=	0;
			
			EHCI->FRINDEX.FRINDEX &= 0x3FF8;
			vTaskDelay(USBH_2ms);
			
			EHCI->FRINDEX.FRINDEX |= 0x0007;
			vTaskDelay(USBH_2ms);
		}
	}
}

void ehci_remove_dev_resource(USBH_Device_Structure *DEV, uint8_t CHK_ADV, uint8_t host) {

	uint8_t i = 0, j = 0;
	for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
		if (DEV->EP[i].XfrType != EHCI_NONE_TYPE) {
			if ((DEV->EP[i].XfrType == EHCI_INT_IN_TYPE) || (DEV->EP[i].XfrType == EHCI_INT_OUT_TYPE)) {
				if(host == USBH1){
					ehci_remove_periodic_table(DEV->EP[i].pQH);
					DEV->EP[i].XfrType = EHCI_NONE_TYPE;
				}
#if defined( CONFIG_DUAL_HOST )				
				else if(host == USBH2){
					ehci_remove_periodic_table_2(DEV->EP[i].pQH);
					DEV->EP[i].XfrType = EHCI_NONE_TYPE;
				}
#endif // End of if defined( CONFIG_DUAL_HOST )					
				ehci_release_structure(EHCI_MEM_TYPE_QH,(uint32_t) DEV->EP[i].pQH,host);
				for (j = 0; j <= DEV->EP[i].LastQTD; j++) {
					ehci_release_structure(EHCI_MEM_TYPE_QTD,(uint32_t) DEV->EP[i].pQTD[j],host);
				}
				DEV->EP[i].XfrType = EHCI_NONE_TYPE;
			} else if ((DEV->EP[i].XfrType == EHCI_ISO_IN_TYPE) || (DEV->EP[i].XfrType == EHCI_ISO_OUT_TYPE)) {
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )
				ehci_stop_xfr(&DEV->EP[i]);
				if(host == USBH1){
					ehci_remove_periodic_table(DEV->EP[i].pSXITD);
				}
#if defined( CONFIG_DUAL_HOST )				
				else if(host == USBH2){
					ehci_remove_periodic_table_2(DEV->EP[i].pSXITD);
				}
#endif // End of if defined( CONFIG_DUAL_HOST )					
				ehci_release_structure(EHCI_MEM_TYPE_SXITD,(uint32_t)DEV->EP[i].pSXITD,host);
				DEV->EP[i].XfrType = EHCI_NONE_TYPE;
#else
				ehci_stop_xfr(&DEV->EP[i]);
				if(host == USBH1){
					ehci_remove_periodic_table(DEV->EP[i].pITD[0]);
				}
#if defined( CONFIG_DUAL_HOST )				
				else if(host == USBH2){
					ehci_remove_periodic_table_2(DEV->EP[i].pITD[0]);					
				}
#endif // End of if defined( CONFIG_DUAL_HOST )						
					
				ehci_release_structure(EHCI_MEM_TYPE_ITD,(uint32_t) DEV->EP[i].pITD[0],host);
				DEV->EP[i].XfrType = EHCI_NONE_TYPE;
#endif			
			} else {
				ehci_removing_qh(DEV->EP[i].pQH, CHK_ADV,host);
				ehci_release_structure(EHCI_MEM_TYPE_QH,(uint32_t) DEV->EP[i].pQH,host);
				for (j = 0; j <= DEV->EP[i].LastQTD; j++) {
					ehci_release_structure(EHCI_MEM_TYPE_QTD,(uint32_t) DEV->EP[i].pQTD[j],host);
				}
				DEV->EP[i].XfrType = EHCI_NONE_TYPE;
			}

//			xSemaphoreGive(DEV->EP[i].SEM);
//			xSemaphoreGive(DEV->EP[i].Mutex);

			//vSemaphoreDelete(DEV->EP[i].SEM);
			//vSemaphoreDelete(DEV->EP[i].Mutex);
		}
	}
	if (DEV->BUFF.size > 0) {
		if (DEV->BUFF.ptr != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
			vPortFree((uint32_t *) DEV->BUFF.ptr);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			free(DEV->BUFF.ptr);
#endif
			DEV->BUFF.ptr = NULL;
		}
		DEV->BUFF.size = 0;
	}

	switch(DEV->CLASS_DRV){
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
		case USBH_HUB_CLASS:
			//DEV->CLASS_DRV = 0;
			if(usbh_plug_out_cb(USBH_HUB_CLASS) == FAIL){
				hub_task_uninit(DEV->device_id);
			}
			DEV->CLASS_DRV = 0;
			break;
#endif
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
		case USBH_WIFI_CLASS:
			//DEV->CLASS_DRV = 0;
			for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
				if(DEV->EP[i].SEM != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
					xSemaphoreGive(DEV->EP[i].SEM);
				//xSemaphoreGive(DEV->EP[i].Mutex);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osSemaphoreRelease(DEV->EP[i].SEM);
#endif
				}
			}
#if defined (CONFIG_WIFI_SDK)			
			/* Only Wi-Fi subsystem use the driver register function
			 * use usbh_device_remove to device disconnect notification for now.
			 */
			usbh_device_remove(DEV);
#else
			if(usbh_plug_out_cb(USBH_WIFI_CLASS) == FAIL){	
				//WiFi_Task_UnInit();
			}
#endif
			DEV->CLASS_DRV = 0;
			break;
#endif
			
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
		case USBH_MSC_CLASS:
			//DEV->CLASS_DRV = 0;			
			for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
				if(DEV->EP[i].SEM != NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
					xSemaphoreGive(DEV->EP[i].SEM);
//				xSemaphoreGive(DEV->EP[i].Mutex);
#endif
					
#if defined (CONFIG_USBH_CMSIS_OS)
					osSemaphoreRelease(DEV->EP[i].SEM);
#endif
				}
			}
			if(usbh_plug_out_cb(USBH_MSC_CLASS) == FAIL){		
				msc_task_uninit(host, DEV->device_id);
			}
			DEV->CLASS_DRV = 0;	
			break;
#endif
			
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
		case USBH_UVC_ISO_CLASS:
			//DEV->CLASS_DRV = 0;			
			usbh_plug_out_cb(USBH_UVC_ISO_CLASS);
			uvc_task_uninit(DEV->device_id);
			DEV->CLASS_DRV = 0;	
			break;

		case USBH_UVC_BULK_CLASS:
			//DEV->CLASS_DRV = 0;			
			usbh_plug_out_cb(USBH_UVC_BULK_CLASS);
			uvc_task_uninit(DEV->device_id);
			DEV->CLASS_DRV = 0;	
			break;

		case (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS):
			//DEV->CLASS_DRV = 0;			
			usbh_plug_out_cb((USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS));
			uvc_task_uninit(DEV->device_id);
			DEV->CLASS_DRV = 0;	
			break;
#endif			
	
#if defined (CONFIG_MODULE_USB_HID_CLASS)
		case USBH_HID_CLASS:
			//DEV->CLASS_DRV = 0;			
			if(usbh_plug_out_cb(USBH_HID_CLASS) == FAIL){	
				hid_task_uninit(DEV->device_id);
			}
			DEV->CLASS_DRV = 0;	
			break;
#endif
	}
}

void ehci_enable_xfr(EHCI_ENDP_STRUCT *EP) {
	if ((EP->XfrType == USBH_ISO_IN_TYPE) || ((EP->XfrType == USBH_ISO_OUT_TYPE))) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		EP->pSXITD->bACTIVE = 1;
#else

#endif
	} else {
		EP->pQH->bOverlay_Status &= ~BIT6;
	}
}

void ehci_disable_xfr(EHCI_ENDP_STRUCT *EP) {
	if ((EP->XfrType == USBH_ISO_IN_TYPE) || ((EP->XfrType == USBH_ISO_OUT_TYPE))) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		EP->pSXITD->bInactive_On_Next_Trans = 1;
		while(EP->pSXITD->bACTIVE);
		EP->pSXITD->bInactive_On_Next_Trans = 0;
#else
		EP->STD_ISO_SM = 0;
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueSend(USBH_QUEUE_STD_ISO_REQ, EP, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessagePut(USBH_QUEUE_STD_ISO_REQ, EP, 0);
#endif
#endif
	} else {
		EP->pQH->bOverlay_Status |= BIT6;
	}
}

void ehci_stop_xfr(EHCI_ENDP_STRUCT *EP) {
	if ((EP->XfrType == USBH_ISO_IN_TYPE) || ((EP->XfrType == USBH_ISO_OUT_TYPE))) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		EP->pSXITD->bInactive_On_Next_Trans = 1;
		EP->pSXITD->bACTIVE = 0;
		EP->pSXITD->bErr_DISCARD = 0;
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_10ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_10ms);
#endif
		EP->pSXITD->bInactive_On_Next_Trans = 0;
#else

#endif
	} else {
		EHCI->USBCMD.ASYNC_EN = 0;
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_10ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_10ms);
#endif
		EP->pQH->bOverlay_Status = 0x40;
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_10ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_10ms);
#endif
		EHCI->USBCMD.ASYNC_EN = 1;
	}
}

void ehci_cx_struct_init(EHCI_ENDP_STRUCT *EP, EHCI_CX_XFR_REQ_Struct *CX_REQ) {
	// QH Structure Init
	EP->pQH->bOverlay_CurrentqTD = (uint32_t)EP->pQH;
	EP->pQH->bOverlay_NextqTD = (uint32_t) EP->pQTD[0] >> 5;
	EP->pQH->bOverlay_NextTerminate = 0;
	EP->pQH->bOverlay_AlternateqTD = (uint32_t) EP->pQTD[0] >> 5;
	EP->pQH->bOverlay_AlternateNextTerminate = 0;
	EP->pQH->bOverlay_NanCnt = 0;
	EP->pQH->bOverlay_InterruptOnComplete = 0;
	EP->pQH->bOverlay_C_Page = 0;
	EP->pQH->bOverlay_ErrorCounter = 0;

	EP->pQH->bOverlay_TotalBytes = 0;
	EP->pQH->bOverlay_DT = 0;

	EP->pQH->bOverlay_BufferPointer_0 = 0;
	EP->pQH->bOverlay_CurrentOffset   = 0;
	EP->pQH->bOverlay_BufferPointer_1 = 0;
	EP->pQH->bOverlay_BufferPointer_2 = 0;
	EP->pQH->bOverlay_BufferPointer_3 = 0;
	EP->pQH->bOverlay_BufferPointer_4 = 0;

	switch (CX_REQ->CX_Case) {
	case CX_Read:
		EP->pQTD[0]->bNextQTDPointer = ((uint32_t) EP->pQTD[1]) >> 5;
		EP->pQTD[0]->bReserve_1 = 0;
		EP->pQTD[0]->bTerminate = 0;
		EP->pQTD[0]->bAlternateQTDPointer = ((uint32_t) EP->pQTD[1]) >> 5;
		EP->pQTD[0]->bReserve_2 = 0;
		EP->pQTD[0]->bAlternateTerminate = 0;
		EP->pQTD[0]->bDataToggle = 0;
		EP->pQTD[0]->bTotalBytes = 0x08;
		EP->pQTD[0]->bInterruptOnComplete = 0;
		EP->pQTD[0]->CurrentPage = 0;
		EP->pQTD[0]->bErrorCounter = 3;
		EP->pQTD[0]->bPID = EHCI_SETUP_TOKEN;
		EP->pQTD[0]->bStatus = 0x80;
		EP->pQTD[0]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->SETUP_CMD_BUFF;
		EP->pQTD[1]->bNextQTDPointer = ((uint32_t) EP->pQTD[2]) >> 5;
		EP->pQTD[1]->bReserve_1 = 0;
		EP->pQTD[1]->bTerminate = 0;
		EP->pQTD[1]->bAlternateQTDPointer = ((uint32_t) EP->pQTD[2]) >> 5;
		EP->pQTD[1]->bReserve_2 = 0;
		EP->pQTD[1]->bAlternateTerminate = 0;
		EP->pQTD[1]->bDataToggle = 1;
		EP->pQTD[1]->bTotalBytes = CX_REQ->SIZE;
		EP->pQTD[1]->bInterruptOnComplete = 0;
		EP->pQTD[1]->CurrentPage = 0;
		EP->pQTD[1]->bErrorCounter = 3;
		EP->pQTD[1]->bPID = EHCI_IN_TOKEN;
		EP->pQTD[1]->bStatus = 0x80;
		EP->pQTD[1]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->pBUFF;
		EP->pQTD[1]->ArrayBufferPointer_Word[1] = ((((uint32_t) CX_REQ->pBUFF) & 0xFFFFF000) + 0x1000);
		EP->pQTD[2]->bNextQTDPointer = 0;
		EP->pQTD[2]->bReserve_1 = 0;
		EP->pQTD[2]->bTerminate = 1;
		EP->pQTD[2]->bAlternateQTDPointer = 0;
		EP->pQTD[2]->bReserve_2 = 0;
		EP->pQTD[2]->bAlternateTerminate = 1;
		EP->pQTD[2]->bDataToggle = 1;
		EP->pQTD[2]->bTotalBytes = 0;
		EP->pQTD[2]->bInterruptOnComplete = 1;
		EP->pQTD[2]->CurrentPage = 0;
		EP->pQTD[2]->bErrorCounter = 3;
		EP->pQTD[2]->bPID = EHCI_OUT_TOKEN;
		EP->pQTD[2]->bStatus = 0x80;
		EP->pQTD[2]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->STS_DUMMY_BUFF;
		break;

	case CX_Write:
		EP->pQTD[0]->bNextQTDPointer = ((uint32_t) EP->pQTD[1]) >> 5;
		EP->pQTD[0]->bReserve_1 = 0;
		EP->pQTD[0]->bTerminate = 0;
		EP->pQTD[0]->bAlternateQTDPointer = ((uint32_t) EP->pQTD[1]) >> 5;
		EP->pQTD[0]->bReserve_2 = 0;
		EP->pQTD[0]->bAlternateTerminate = 0;
		EP->pQTD[0]->bDataToggle = 0;
		EP->pQTD[0]->bTotalBytes = 0x08;
		EP->pQTD[0]->bInterruptOnComplete = 0;
		EP->pQTD[0]->CurrentPage = 0;
		EP->pQTD[0]->bErrorCounter = 3;
		EP->pQTD[0]->bPID = EHCI_SETUP_TOKEN;
		EP->pQTD[0]->bStatus = 0x80;
		EP->pQTD[0]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->SETUP_CMD_BUFF;
		EP->pQTD[1]->bNextQTDPointer = ((uint32_t) EP->pQTD[2]) >> 5;
		EP->pQTD[1]->bReserve_1 = 0;
		EP->pQTD[1]->bTerminate = 0;
		EP->pQTD[1]->bAlternateQTDPointer = ((uint32_t) EP->pQTD[2]) >> 5;
		EP->pQTD[1]->bReserve_2 = 0;
		EP->pQTD[1]->bAlternateTerminate = 0;
		EP->pQTD[1]->bDataToggle = 1;
		EP->pQTD[1]->bTotalBytes = CX_REQ->SIZE;
		EP->pQTD[1]->bInterruptOnComplete = 0;
		EP->pQTD[1]->CurrentPage = 0;
		EP->pQTD[1]->bErrorCounter = 3;
		EP->pQTD[1]->bPID = EHCI_OUT_TOKEN;	
		EP->pQTD[1]->bStatus = 0x80;
		EP->pQTD[1]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->pBUFF;
		EP->pQTD[1]->ArrayBufferPointer_Word[1] = ((((uint32_t) CX_REQ->pBUFF) & 0xFFFFF000) + 0x1000);
		EP->pQTD[2]->bNextQTDPointer = 0;
		EP->pQTD[2]->bReserve_1 = 0;
		EP->pQTD[2]->bTerminate = 1;
		EP->pQTD[2]->bAlternateQTDPointer = 0;
		EP->pQTD[2]->bReserve_2 = 0;
		EP->pQTD[2]->bAlternateTerminate = 1;
		EP->pQTD[2]->bDataToggle = 1;
		EP->pQTD[2]->bTotalBytes = 0;
		EP->pQTD[2]->bInterruptOnComplete = 1;
		EP->pQTD[2]->CurrentPage = 0;
		EP->pQTD[2]->bErrorCounter = 3;
		EP->pQTD[2]->bPID = EHCI_IN_TOKEN;
		EP->pQTD[2]->bStatus = 0x80;
		EP->pQTD[2]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->STS_DUMMY_BUFF;
		break;

	case CX_NoneData:
		EP->pQTD[0]->bNextQTDPointer = ((uint32_t) EP->pQTD[2]) >> 5;
		EP->pQTD[0]->bReserve_1 = 0;
		EP->pQTD[0]->bTerminate = 0;
		EP->pQTD[0]->bAlternateQTDPointer = ((uint32_t) EP->pQTD[2]) >> 5;
		EP->pQTD[0]->bReserve_2 = 0;
		EP->pQTD[0]->bAlternateTerminate = 0;
		EP->pQTD[0]->bDataToggle = 0;
		EP->pQTD[0]->bTotalBytes = 0x08;
		EP->pQTD[0]->bInterruptOnComplete = 0;
		EP->pQTD[0]->CurrentPage = 0;
		EP->pQTD[0]->bErrorCounter = 3;
		EP->pQTD[0]->bPID = EHCI_SETUP_TOKEN;
		EP->pQTD[0]->bStatus = 0x80;
		EP->pQTD[0]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->SETUP_CMD_BUFF;		
		EP->pQTD[2]->bNextQTDPointer = 0;
		EP->pQTD[2]->bReserve_1 = 0;
		EP->pQTD[2]->bTerminate = 1;
		EP->pQTD[2]->bAlternateQTDPointer = 0;
		EP->pQTD[2]->bReserve_2 = 0;
		EP->pQTD[2]->bAlternateTerminate = 1;
		EP->pQTD[2]->bDataToggle = 1;
		EP->pQTD[2]->bTotalBytes = 0;
		EP->pQTD[2]->bInterruptOnComplete = 1;
		EP->pQTD[2]->CurrentPage = 0;
		EP->pQTD[2]->bErrorCounter = 3;
		EP->pQTD[2]->bPID = EHCI_IN_TOKEN;
		EP->pQTD[2]->bStatus = 0x80;
		EP->pQTD[2]->ArrayBufferPointer_Word[0] = (uint32_t) CX_REQ->STS_DUMMY_BUFF;	
		break;
	}
}

uint32_t ehci_get_cx_act_size(EHCI_ENDP_STRUCT *EP, EHCI_CX_XFR_REQ_Struct *CX_REQ) {
	if (CX_REQ->CX_Case == CX_NoneData) {
		return 0;
	} else {
		return (CX_REQ->SIZE - EP->pQTD[1]->bTotalBytes);
	}
}

uint32_t ehci_get_bk_act_size(EHCI_ENDP_STRUCT *EP, EHCI_BK_XFR_REQ_Struct *BK_REQ) {
	
	uint32_t i = 0;
	uint32_t tempsize = 0;

	while (1) {
		if (EP->pQTD[i]->bTerminate) {
			break;
		} else {
			tempsize += EP->pQTD[i]->bTotalBytes;
			i++;
		}
	}
	tempsize += EP->pQTD[i]->bTotalBytes;

	if(BK_REQ->SIZE > ((MAX_QTD_PER_QH -1) * 20480)) {
		tempsize += (BK_REQ->SIZE - ((MAX_QTD_PER_QH -1) * 20480));
	}

	return (BK_REQ->SIZE - tempsize);
}

void ehci_bk_struct_init(EHCI_ENDP_STRUCT *EP, EHCI_BK_XFR_REQ_Struct *BK_REQ) {

	uint32_t ALIGH_SIZE;
	uint32_t EndPtr, StartPtr;
	uint32_t ArrayIndex = 0;
	uint32_t QtdIndex = 0;
	uint32_t TempTotalSize, ArraySize;

	// QH Structure Init
	//EP->pQH->bOverlay_CurrentqTD		=	0;
	//EP->pQH->bOverlay_CurrentqTD = ((uint32_t) &EP->Dummy[0]) >> 5;
	EP->pQH->bOverlay_CurrentqTD = (uint32_t)EP->pQH;
	EP->pQH->bOverlay_NextqTD = (uint32_t) EP->pQTD[0] >> 5;
	EP->pQH->bOverlay_NextTerminate = 0;
	EP->pQH->bOverlay_AlternateqTD = (uint32_t) EP->pQTD[0] >> 5;
	EP->pQH->bOverlay_AlternateNextTerminate = 0;
	EP->pQH->bOverlay_NanCnt = 0;
	EP->pQH->bOverlay_InterruptOnComplete = 0;
	EP->pQH->bOverlay_C_Page = 0;
	EP->pQH->bOverlay_ErrorCounter = 0;
	EP->pQH->bOverlay_TotalBytes = 0;
	EP->pQH->bOverlay_DT = EP->DataTog;

	EP->pQH->bOverlay_BufferPointer_0 = 0;
	EP->pQH->bOverlay_CurrentOffset = 0;
	EP->pQH->bOverlay_BufferPointer_1 = 0;
	EP->pQH->bOverlay_BufferPointer_2 = 0;
	EP->pQH->bOverlay_BufferPointer_3 = 0;
	EP->pQH->bOverlay_BufferPointer_4 = 0;

	// Init-Boundary
	TempTotalSize = BK_REQ->SIZE;
	StartPtr = (uint32_t) BK_REQ->pBUFF;
	EndPtr = (((uint32_t) BK_REQ->pBUFF) + BK_REQ->SIZE);
	ALIGH_SIZE = (0x1000 - ((uint32_t) BK_REQ->pBUFF & 0xFFF));

	// Init-QTD
	EP->pQTD[QtdIndex]->bTotalBytes = 0;
	EP->pQTD[QtdIndex]->bDataToggle = EP->DataTog;
	EP->pQTD[QtdIndex]->bStatus = 0x80;
	EP->pQTD[QtdIndex]->CurrentPage = 0;
	EP->pQTD[QtdIndex]->bErrorCounter = 3;
	if (EP->XfrType == USBH_BK_OUT_TYPE) {
		EP->pQTD[QtdIndex]->bPID = EHCI_OUT_TOKEN;
	} else {
		EP->pQTD[QtdIndex]->bPID = EHCI_IN_TOKEN;
	}
	if (ALIGH_SIZE != 4096) {
		ArraySize = ((BK_REQ->SIZE < ALIGH_SIZE) ? BK_REQ->SIZE : ALIGH_SIZE);
		EP->pQTD[QtdIndex]->bTotalBytes += ArraySize;
		EP->pQTD[QtdIndex]->ArrayBufferPointer_Word[ArrayIndex] = StartPtr;
		ArrayIndex++;
		StartPtr += ALIGH_SIZE;
		TempTotalSize -= ArraySize;
	}
	if (TempTotalSize) {
		do {
			ArraySize = ((TempTotalSize < 4096) ? TempTotalSize : 4096);
			EP->pQTD[QtdIndex]->bTotalBytes += ArraySize;
			EP->pQTD[QtdIndex]->ArrayBufferPointer_Word[ArrayIndex] = StartPtr;
			EP->pQTD[QtdIndex]->bErrorCounter = 3;
			EP->pQTD[QtdIndex]->bInterruptOnComplete = 0;
			ArrayIndex++;
			StartPtr += ArraySize;
			TempTotalSize -= ArraySize;
			if ((ArrayIndex == 5) && (StartPtr < EndPtr)) {
				ArrayIndex = 0;
				EP->pQTD[QtdIndex]->bTerminate = 0;
				EP->pQTD[QtdIndex]->bAlternateTerminate = 0;
				if (QtdIndex < (MAX_QTD_PER_QH - 2)) {
					QtdIndex++;
					EP->pQTD[QtdIndex]->bStatus = 0x80;
					EP->pQTD[QtdIndex]->bTotalBytes = 0;
					EP->pQTD[QtdIndex]->CurrentPage = 0;
					if (EP->XfrType == USBH_BK_OUT_TYPE) {
						EP->pQTD[QtdIndex]->bPID = EHCI_OUT_TOKEN;
					} else {
						EP->pQTD[QtdIndex]->bPID = EHCI_IN_TOKEN;
					}
				} else {
					break;
				}
			}
		} while (StartPtr < EndPtr);
	}
	EP->pQTD[QtdIndex]->bTerminate = 1;
	EP->pQTD[QtdIndex]->bAlternateTerminate = 0;
	EP->pQTD[QtdIndex]->bInterruptOnComplete = 1;

}

void ehci_int_struct_init(EHCI_ENDP_STRUCT *EP, EHCI_INT_XFR_REQ_Struct *INT_REQ) {

	// QH Structure Init
	EP->pQH->bOverlay_CurrentqTD = (uint32_t)EP->pQH;
	EP->pQH->bOverlay_NextqTD = (uint32_t) EP->pQTD[0] >> 5;
	EP->pQH->bOverlay_NextTerminate = 0;
	EP->pQH->bOverlay_AlternateqTD = (uint32_t) EP->pQTD[0] >> 5;
	EP->pQH->bOverlay_AlternateNextTerminate = 0;
	EP->pQH->bOverlay_NanCnt = 0;
	EP->pQH->bOverlay_InterruptOnComplete = 0;
	EP->pQH->bOverlay_C_Page = 0;
	EP->pQH->bOverlay_ErrorCounter = 0;

	EP->pQH->bOverlay_TotalBytes = 0;
	EP->pQH->bOverlay_DT = EP->DataTog;

	EP->pQH->bOverlay_BufferPointer_0 = 0;
	EP->pQH->bOverlay_CurrentOffset   = 0;
	EP->pQH->bOverlay_BufferPointer_1 = 0;
	EP->pQH->bOverlay_BufferPointer_2 = 0;
	EP->pQH->bOverlay_BufferPointer_3 = 0;
	EP->pQH->bOverlay_BufferPointer_4 = 0;
	
	EP->pQH->bHighBandwidth = 1;
	EP->pQH->bInterruptScheduleMask = 0x01;
	EP->pQH->bSplitTransactionMask = 0x1c;

	if (EP->XfrType == USBH_INT_OUT_TYPE) {
		EP->pQTD[0]->bPID = EHCI_OUT_TOKEN;
	} else {
		EP->pQTD[0]->bPID = EHCI_IN_TOKEN;
	}
	EP->pQTD[0]->bDataToggle = EP->DataTog;
	EP->pQTD[0]->bTotalBytes = INT_REQ->SIZE;
	EP->pQTD[0]->ArrayBufferPointer_Word[0] = (uint32_t) INT_REQ->pBUFF;
	EP->pQTD[0]->bErrorCounter = 3;
	EP->pQTD[0]->bStatus = 0x80;
	EP->pQTD[0]->bTerminate = 1;
	EP->pQTD[0]->bAlternateTerminate = 1;
	EP->pQTD[0]->bInterruptOnComplete = 1;
}


void ehci_iso_struct_init(EHCI_ENDP_STRUCT *EP, EHCI_ISO_XFR_REQ_Struct *ISO_REQ) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )

	uint32_t start = 0;
	uint32_t end = 0;

	start = (uint32_t)ISO_REQ->RING_BUFF_PTR;
	end = start+ISO_REQ->RING_BUFF_SIZE;

	EP->pSXITD->bRING_BUFFER_START = start>>12;
	EP->pSXITD->bRING_BUFFER_END = end>>12;
	//EP->pSXITD->bRING_THRESHOLD = ((end-(ISO_REQ->RING_BUFF_SIZE/ThersholdPosition))>>12);
	EP->pSXITD->bRING_THRESHOLD = (((EP->pSXITD->bRING_BUFFER_END<<12)-(ISO_REQ->RING_BUFF_SIZE/ThersholdPosition))>>12);
	//EP->pSXITD->FW_FRAME_END = end;
	EP->pSXITD->FW_FRAME_END = EP->pSXITD->bRING_BUFFER_END<<12;
    //EP->pSXITD->HW_CURRENT_POINTER = 0;
	//EP->pSXITD->HW_FRAME_END = 0;
    EP->pSXITD->HW_CURRENT_POINTER = EP->pSXITD->bRING_BUFFER_START<<12;
	EP->pSXITD->HW_FRAME_END = EP->pSXITD->bRING_BUFFER_START<<12;

	if(ISO_REQ->SXITD_Discard_En) {
		EP->pSXITD->bDISCARD_EN = 1;
	} else {
		EP->pSXITD->bDISCARD_EN = 0;
	}
#if defined( CONFIG_PLATFORM_SN7320 )
	if(ISO_REQ->SXITD_ErrDiscard_En) {
		EP->pSXITD->bErrDiscard_EN = 1;
	} else {
		EP->pSXITD->bErrDiscard_EN = 0;
	}	
#endif
	if(ISO_REQ->SXITD_Header_Filter) {
		EP->pSXITD->bHEADER = 1;
	} else {
		EP->pSXITD->bHEADER = 0;
	}
	EP->pSXITD->bIOC = 1;
	EP->pSXITD->bRELOAD = 1;
	//EP->LastFwFrameEnd = start;
	EP->LastFwFrameEnd = EP->pSXITD->bRING_BUFFER_START<<12;
	EP->LastFID = 0;
	EP->pSXITD->bMAX_PK_SIZE = (ISO_REQ->MaxPktSize&0x7FF);
	EP->pSXITD->bMult = (ISO_REQ->MaxPktSize>>11)+1;
#else
	EHCI_ITD_STRUCTURE *pITD = NULL;
	uint32_t start = 0, temp_start = 0;
	uint32_t i = 0, j = 0;
	uint32_t MaxPktSize = 0;
	uint32_t Mult = 0;
	uint8_t io = 0;

	start = (uint32_t) ISO_REQ->RING_BUFF_PTR;
	temp_start = start + ISO_REQ->STD_ISO_REQ * Standard_iTD_interval * 8 * 3072;
	pITD = (EHCI_ITD_STRUCTURE*) EP->pITD[Standard_iTD_interval * ISO_REQ->STD_ISO_REQ];
	EP->StartITD = pITD;
	MaxPktSize = (ISO_REQ->MaxPktSize & 0x7FF);

	if (EP->XfrType == USBH_ISO_IN_TYPE) {
		io = 1;
	} else if (EP->XfrType == USBH_ISO_OUT_TYPE) {
		io = 0;
	}

	//if (MaxPktSize != 800) {
	//	UVC_DBG("MaxPktSize = %d",MaxPktSize);
	//}
	Mult = (ISO_REQ->MaxPktSize >> 11) + 1;

	for (i = 0; i < Standard_iTD_interval; i++) {
		for (j = 0; j < 7; j++) {
			pITD->ArrayBufferPointer_Word[j].bBufferPointer = (temp_start + j * 0x1000) >> 12;
		}
		//pITD->ArrayBufferPointer_Word[1].bParameter = MaxPktSize | 1 << 11;
		pITD->ArrayBufferPointer_Word[1].bParameter = MaxPktSize | io << 11;
		pITD->ArrayBufferPointer_Word[2].bParameter = Mult;
		for (j = 0; j < 8; j++) {
			pITD->ArrayStatus_Word[j].bLength = 0xbff;
			pITD->ArrayStatus_Word[j].bOffset = j * 0xC00;
			pITD->ArrayStatus_Word[j].bStatus = 8;
		}
		pITD++;
		temp_start += 24576;

	}
	pITD--;
	EP->LastITD = pITD;
	pITD->ArrayStatus_Word[7].bInterruptOnComplete = 1;

#endif 
}

uint8_t ehci_parser_dev(USBH_Device_Structure *DEV, uint8_t FUN) {
	EHCI_ALLOC_REQUEST_Struct AllocReq;
	uint16_t PID, VID;
	uint8_t ifclass, subclass;
	uint8_t i, j, k = 0;
	uint8_t status = SUCCESS;
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
	uint32_t idle_times = 0;
#endif
#endif
	
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
	uint32_t task_delay_time;
#endif

	switch (FUN) {
	case EHCI_PARSER_ROOT_DET:
		
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
		PHY_510setting_1_host1();
		for(idle_times = 0; idle_times < 100; idle_times++);
#endif
#endif
		
		// Root Detect
		if (USBH_DEV[USBH_ROOT].bDevIsConnected == 0) {
			if (EHCI->PORTSC[0].CURR_CNNT_ST == 1) {
#if defined (CONFIG_USBH_FREE_RTOS)
				for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
					if (DEV->EP[i].SEM != NULL) {
						vSemaphoreDelete(DEV->EP[i].SEM);
						DEV->EP[i].SEM = NULL;
					}
					if (DEV->EP[i].Mutex != NULL) {
						vSemaphoreDelete(DEV->EP[i].Mutex);
						DEV->EP[i].Mutex = NULL;
					}
				}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
					if (DEV->EP[i].SEM != NULL) {
						osSemaphoreDelete(DEV->EP[i].SEM);
						DEV->EP[i].SEM = NULL;
					}
					if (DEV->EP[i].Mutex != NULL) {
						osMutexDelete(DEV->EP[i].Mutex);
						DEV->EP[i].Mutex = NULL;
					}
				}
#endif
				USBH_DBG("ehci_parser_dev : Clear DEV info !!");

				memset(DEV, 0, sizeof(USBH_Device_Structure));
				DEV->device_id = 0;
				DEV->bDevIsConnected = 1;
				DEV->ENUM_SM = USBH_ROOT_PLUG_IN_STATE;
				USBH_DBG("Detect Plug In !!");
			}
		} else if (USBH_DEV[USBH_ROOT].bDevIsConnected == 1) {
			if (EHCI->PORTSC[0].CURR_CNNT_ST == 0) {
				DEV->bDevIsConnected = 0;
				DEV->ENUM_SM = USBH_ROOT_PLUG_OUT_STATE;
				USBH_DBG("Detect Plug Out !!");
			}
		}
		status = SUCCESS;
		
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
		PHY_510setting_2_host1();
#endif
#endif
		break;
	case EHCI_PARSER_ROOT_RST:
		// CHK ASYNC ENABLE
		if (EHCI->USBCMD.ASYNC_EN == 0) {
			EHCI->USBCMD.ASYNC_EN = 1;
				while (EHCI->USBCMD.ASYNC_EN == 0);
		}
		// CHK RUN STOP
		if (EHCI->USBCMD.RUN_NSTOP == 0) {
			EHCI->USBCMD.RUN_NSTOP = 1;
			while (EHCI->USBCMD.RUN_NSTOP == 0);
		}

#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
		if( reset_count == 0 ) {
			task_delay_time = USBH_5ms;
		}
		else if( reset_count == 1 ) {
			task_delay_time = USBH_50ms;
		}
		else if( reset_count == 2 ) {
			task_delay_time = USBH_200ms;
		}
		else {
			task_delay_time = USBH_5ms;
		}

		reset_count++;
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(task_delay_time);
		USBH_DBG("DO USB RESET ... \r\n");
		EHCI->PORTSC[0].PORTRESET = 1;
		vTaskDelay(USBH_20ms);
		EHCI->PORTSC[0].PORTRESET = 0;
		vTaskDelay(USBH_10ms);
		if(EHCI->PORTSC[0].PORTRESET == 1) {
			return FAIL;
		}
		USBH_DBG("USB RESET	DONE ! ");
		vTaskDelay(task_delay_time);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(task_delay_time);
		USBH_DBG("DO USB RESET ... \r\n");
		EHCI->PORTSC[0].PORTRESET = 1;
		osDelay(USBH_20ms);
		EHCI->PORTSC[0].PORTRESET = 0;
		osDelay(USBH_10ms);
		if(EHCI->PORTSC[0].PORTRESET == 1) {
			return FAIL;
		}
		USBH_DBG("USB RESET	DONE ! ");
		osDelay(task_delay_time);
#endif
#else
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_5ms);
		USBH_DBG("DO USB RESET ... \r\n");
		EHCI->PORTSC[0].PORTRESET = 1;
		vTaskDelay(USBH_20ms);
		EHCI->PORTSC[0].PORTRESET = 0;
		vTaskDelay(USBH_10ms);
		if(EHCI->PORTSC[0].PORTRESET == 1) {
			return FAIL;
		}
		USBH_DBG("USB RESET	DONE ! ");
		vTaskDelay(USBH_5ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_5ms);
		USBH_DBG("DO USB RESET ... \r\n");
		EHCI->PORTSC[0].PORTRESET = 1;
		osDelay(USBH_20ms);
		EHCI->PORTSC[0].PORTRESET = 0;
		osDelay(USBH_10ms);
		if(EHCI->PORTSC[0].PORTRESET == 1) {
			return FAIL;
		}
		USBH_DBG("USB RESET	DONE ! ");
		osDelay(USBH_5ms);
#endif
#endif	
		if (EHCI->PORTSC[0].CURR_CNNT_ST == 1) {
			if (EHCI->PORTSC[0].PORTENABLED == 1) {
				DEV->SPEED = EHCI_HIGH_SPEED;
				USBH_DBG("Detected High Speed USB Device on	root !");
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
				reset_count = 0;
#endif
				status = SUCCESS;
			} else {
				DEV->SPEED = EHCI_FULL_SPEED;
				USBH_DBG("Detected Full Speed USB Device on	root ! , Can't Support Full Speed Device !");
				status = FAIL;
			}
		} else {
			status = FAIL;
		}
		break;
	case EHCI_PARSER_CX_ENQU:
		//ENQUEUE for CX : Addr0 Endp0
		memset(&AllocReq, 0, sizeof(AllocReq));
		AllocReq.Addr = 0;
		AllocReq.Endp = 0;
		AllocReq.HubAddr = 0;
		AllocReq.Speed = DEV->SPEED;
		AllocReq.MaxPktSize = 8;
		AllocReq.DataSize = 200;
		AllocReq.DataDir = EHCI_DATA_IN;
		DEV->EP[0] = ehci_cx_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq,USBH1);
		ehci_adding_qh_tail(DEV->EP[0].pQH,USBH1);
		status = SUCCESS;
		break;
	case EHCI_PARSER_NONCX_ENQU:
		// ENQUEUE for NONCX
		memset(&AllocReq, 0, sizeof(AllocReq));
		VID = (DEV->DEVDes.bVIDHighByte << 8) | (DEV->DEVDes.bVIDLowByte);
		PID = (DEV->DEVDes.bPIDHighByte << 8) | (DEV->DEVDes.bPIDLowByte);

		// By PID VID
#if defined (CONFIG_WIFI_SDK)
		if (0) {	
#else
		if ((PID == 0x7601) && (VID == 0x148F)) {
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
			// Manufacturer:MediaTek  Product:802.11 n WLAN
			for (i = 0; i < 4; i++) {
				k++;
				AllocReq.Addr = DEV->bAdd;
				AllocReq.Endp				=	DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_EndpointAddress;
				AllocReq.Speed = DEV->SPEED;
				AllocReq.MaxPktSize			=	((DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_wMaxPacketSizeLowByte);
				AllocReq.DataSize = 100 * 1024;
				AllocReq.DataDir			=	DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_EndpointAddress>>7;
				DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
				ehci_adding_qh_tail(DEV->EP[k].pQH,USBH1);
			}
			DEV->CLASS_DRV = USBH_WIFI_CLASS;
			status = SUCCESS;
#else
			status = FAIL;
#endif // end of defined (CONFIG_MODULE_USB_WIFI_CLASS)
#endif // end of defined (CONFIG_WIFI_SDK)
		} else if (((PID == 0x6366) && (VID == 0x0c45)) || ((PID == 0x651B) && (VID == 0x0c45)) || ((PID == 0x6365) && (VID == 0x0c45))
				|| ((PID == 0x8366) && (VID == 0x0C45)) || ((PID == 0x6367) && (VID == 0x0c45)) || ((PID == 0x8200) && (VID == 0x0c45)) || ((PID == 0x6365) && (VID == 0x2084)) || ((PID == 0xb5e5) && (VID == 0x04f2))) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)			
			for(i=1;i<DEV->CFGDes[0].bINTERFACE_NUMBER;i++) {
                memset(&AllocReq, 0, sizeof(EHCI_ALLOC_REQUEST_Struct));
				AllocReq.Addr = DEV->bAdd;
				AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_EndpointAddress;
				AllocReq.HubAddr = 0;
				AllocReq.Speed = DEV->SPEED;
				AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_wMaxPacketSizeLowByte);
				AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_EndpointAddress>>7;
				switch(DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_bmAttributes){
					case EHCI_BULK:
						AllocReq.DataSize = MAX_QTD_PER_QH * 20 * 1024;
						DEV->EP[i] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
						ehci_adding_qh_tail(DEV->EP[i].pQH,USBH1);
						DEV->CLASS_DRV |= USBH_UVC_BULK_CLASS;
						status = SUCCESS;
						break;
					
					case EHCI_ISO:
						DEV->EP[i] = ehci_iso_allocate((EHCI_ALLOC_REQUEST_Struct*)&AllocReq,USBH1);
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
						ehci_add_periodic_table(EHCI_SXISO, (uint32_t)NULL, (uint32_t*)DEV->EP[i].pSXITD);
#else				
						ehci_add_periodic_table(EHCI_ISO,(uint32_t)NULL,(uint32_t *)DEV->EP[i].pITD[0]);			
#endif 
						DEV->CLASS_DRV |= USBH_UVC_ISO_CLASS;
						status = SUCCESS;
						break;
				}
			}
#else
			status = FAIL;
#endif
		} else if ((PID == 0x6362) && (VID == 0x0c45)) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
			// Manufacturer:Sonix Technology Co., Ltd. , Product 283
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
			for(i=1;i<DEV->CFGDes[0].bINTERFACE_NUMBER;i++) {
				AllocReq.Addr = DEV->bAdd;
				AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_EndpointAddress;
				AllocReq.Speed = DEV->SPEED;
				AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_wMaxPacketSizeLowByte);
				AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_EndpointAddress>>7;
				DEV->EP[i] = ehci_iso_allocate((EHCI_ALLOC_REQUEST_Struct*)&AllocReq, USBH1);
				ehci_add_periodic_table(EHCI_SXISO,(uint32_t)NULL,(uint32_t *)DEV->EP[i].pSXITD);
			}
			DEV->CLASS_DRV = USBH_UVC_ISO_CLASS;
			status = SUCCESS;
#else

#endif
#else
			status = FAIL;
#endif
		} else if ((PID == 0x8200) && (VID == 0x0c45)) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
			// Manufacturer:Sonix Technology Co., Ltd. , Product 672
			for (i = 0; i < DEV->CFGDes[0].bINTERFACE_NUMBER; i++){
				for (j = 0; j < DEV->CFGDes[0].Interface[i].bEP_NUMBER; j++){
					k++;
					AllocReq.Addr = DEV->bAdd;
					AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress;
					AllocReq.HubAddr = 0;
					AllocReq.Speed = DEV->SPEED;
					AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeHighByte << 8)	| DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeLowByte);
					AllocReq.DataSize = MAX_QTD_PER_QH * 20 * 1024;
					AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress >> 7;
					switch(DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_bmAttributes){
						case EHCI_BULK:
							DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq,USBH1);
							ehci_adding_qh_tail(DEV->EP[k].pQH,USBH1);
							break;
						case EHCI_INT:
							USBH_DBG(" INT XFR Not Support yet");
							break;
					}
				}
			}
			DEV->CLASS_DRV = USBH_UVC_BULK_CLASS;
			status = SUCCESS;
#else
			status = FAIL;
#endif
		} else if ((PID == 0x2060) && (VID == 0x0583)) {
#if defined (CONFIG_MODULE_USB_HID_CLASS)
		// HID Class
		k++;
		AllocReq.Addr 			= DEV->bAdd;
		AllocReq.Endp			= DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress;
		AllocReq.HubAddr 		= 1;
		AllocReq.Speed 			= DEV->SPEED;
		AllocReq.MaxPktSize		= ((DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeLowByte);
		AllocReq.DataSize 		= AllocReq.MaxPktSize;
		AllocReq.DataDir 		= DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress >> 7;
		DEV->EP[k] 					= ehci_int_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
		ehci_add_periodic_table(EHCI_INT, DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_Interval, (uint32_t *) DEV->EP[k].pQH);
		DEV->CLASS_DRV 			= USBH_HID_CLASS;
		status = SUCCESS;
#else
		status = FAIL;
#endif
		} else {
			// By ClASS CODE
			for (i = 0; i < DEV->CFGDes[0].bINTERFACE_NUMBER; i++) {
				ifclass = DEV->CFGDes[0].Interface[i].bInterfaceClass;
				subclass = DEV->CFGDes[0].Interface[i].bInterfaceSubClass;
					if((ifclass	== USBH_MASS_STORAGE_CLASS_CODE) && (subclass == 0x06)){
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
						for(j=0;j<DEV->CFGDes[0].Interface[i].bEP_NUMBER;j++){
						k++;
						if (k > MAX_QH_PER_DEVICE - 1) {	// Exclude ENDP0
							USBH_DBG(" Out of MAX_QH_PER_DEVICE Size ");
							return FAIL;
						}
						AllocReq.Addr = DEV->bAdd;
						AllocReq.Endp				=	DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress;
						AllocReq.HubAddr = 0;
						AllocReq.Speed = DEV->SPEED;
						AllocReq.MaxPktSize			=	((DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeLowByte);
						AllocReq.DataSize = 128 * 1024;
						AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress >> 7;
						switch (DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_bmAttributes) {
							case EHCI_BULK:
								DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
								ehci_adding_qh_tail(DEV->EP[k].pQH,USBH1);
								break;
							case EHCI_INT:
								USBH_DBG(" INT XFR Not Support yet");
								break;
						}
					}
					DEV->CLASS_DRV = USBH_MSC_CLASS;
					status = SUCCESS;
#else
					status = FAIL;
#endif
				} else if (ifclass == USBH_HUB_CLASS_CODE) {
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
					// HUB Class
					k++;
					AllocReq.Addr 		= DEV->bAdd;
					AllocReq.Endp		= DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress;
					AllocReq.HubAddr 	= 0;
					AllocReq.Speed 		= DEV->SPEED;
					AllocReq.MaxPktSize	= ((DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeLowByte);
					AllocReq.DataSize 	= AllocReq.MaxPktSize;
					AllocReq.DataDir 	= DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress >> 7;
					DEV->EP[k] 		= ehci_int_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
					ehci_add_periodic_table(EHCI_INT,DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_Interval,(uint32_t *) DEV->EP[k].pQH);
					DEV->CLASS_DRV = USBH_HUB_CLASS;
					status = SUCCESS;
#else
					status = FAIL;
#endif
				} else {
					// By General	Parser
						for(j=0;j<DEV->CFGDes[0].Interface[i].bEP_NUMBER;j++){
						k++;
						if (k > MAX_QH_PER_DEVICE - 1) {	// Exclude ENDP0
							USBH_DBG(" Out of MAX_QH_PER_DEVICE Size ");
							return FAIL;
						}
						AllocReq.Addr = DEV->bAdd;
						AllocReq.Endp			=	DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress;
						AllocReq.HubAddr = 0;
						AllocReq.Speed = DEV->SPEED;
						AllocReq.MaxPktSize		=	((DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeLowByte);
						AllocReq.DataSize = 20480;
						AllocReq.DataDir		=	DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress>>7;
						switch (DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_bmAttributes) {
							case EHCI_CONTROL:
								DEV->EP[k] = ehci_cx_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
								ehci_adding_qh_tail(DEV->EP[k].pQH,USBH1);
								break;
							case EHCI_ISO:
								USBH_DBG(" ISO XFR Not Support yet");
								break;
							case EHCI_BULK:
								DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH1);
								ehci_adding_qh_tail(DEV->EP[k].pQH,USBH1);
								break;
							case EHCI_INT:
								USBH_DBG(" INT XFR Not Support yet");
								break;
						}
					}
						status = FAIL;
				}
			}
		}
		break;
	case EHCI_PARSER_WAKE_CLASS_DRV:
		if (DEV->CLASS_DRV == USBH_HUB_CLASS) {
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
			USBH_DBG("WAKE UP HUB CLASS Driver  !!");
			DEV->BUFF.size = 512;
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )			
			do {
				DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
			} while (DEV->BUFF.ptr == NULL);
#endif		
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif		
			if(usbh_plug_in_cb(USBH_HUB_CLASS) == FAIL){	
				hub_task_init(DEV->device_id);
				
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSend(usbh_queue_hub_drv, &DEV->device_id, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_hub_drv, &DEV->device_id, 0);
#endif
			}
#endif

#if !defined (CONFIG_WIFI_SDK)
		} else if (DEV->CLASS_DRV == USBH_WIFI_CLASS) {
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
			if(usbh_plug_in_cb(USBH_WIFI_CLASS) == FAIL){		
				USBH_DBG("WAKE UP WIFI CLASS Driver !!");
		//			if (xWiFiTask == NULL)
		//				WiFi_Task_Init();
			}
#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(USBH_SEM_WAKEUP_WIFI);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osSemaphoreRelease(USBH_SEM_WAKEUP_WIFI);
#endif
#endif
#endif // end of if !defined (CONFIG_WIFI_SDK)
		} else if (DEV->CLASS_DRV == USBH_MSC_CLASS) {
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
			USBH_DBG("WAKE UP MSC CLASS Driver  !!");
			DEV->BUFF.size = (8+1)*1024;
			
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size,GFP_DMA, MODULE_DRI_USBH);
			} while (DEV->BUFF.ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
			if(usbh_plug_in_cb(USBH_MSC_CLASS) == FAIL){	
				if (xTASK_HDL_MSC_DRV[DEV->device_id] == NULL)
					msc_task_init(USBH1, DEV->device_id);
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSend(USBH_QUEUE_WAKEUP_MSC_DRV, &DEV->device_id, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(USBH_QUEUE_WAKEUP_MSC_DRV, &DEV->device_id, 0);
#endif
			}
#endif
		} else if (DEV->CLASS_DRV == USBH_UVC_ISO_CLASS) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
			USBH_DBG("WAKE UP UVC ISO CLASS	Driver	!!");
			DEV->BUFF.size = 512;
			
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )			
			do {
				DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size,GFP_DMA, MODULE_DRI_USBH);
			} while (DEV->BUFF.ptr == NULL);
#endif			
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)			
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif

			if(usbh_plug_in_cb(USBH_UVC_ISO_CLASS) == FAIL){
				uvc_task_init(DEV->device_id);			

#if defined( CONFIG_SNX_ISO_ACCELERATOR )		
#else 
				if (USBH_QUEUE_STD_ISO_REQ == NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
					USBH_QUEUE_STD_ISO_REQ = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(USBH_STD_ISO_QUEUE_STRUCT));
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMessageQDef(USBH_QUEUE_STD_ISO_REQ, USBH_UVC_DATA_QUEUE_SIZE, USBH_STD_ISO_QUEUE_STRUCT);
					USBH_QUEUE_STD_ISO_REQ = osMessageCreate(osMessageQ(USBH_QUEUE_STD_ISO_REQ), NULL);
#endif	
				}
#endif 
#if defined (CONFIG_USBH_FREE_RTOS)
				//xSemaphoreGive(USBH_SEM_WAKEUP_UVC_DRV);
				xQueueSend(usbh_queue_uvc_drv, &DEV->device_id, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_uvc_drv, &DEV->device_id, 0);
#endif
			}
#endif
		} else if (DEV->CLASS_DRV == USBH_UVC_BULK_CLASS) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
			USBH_DBG("WAKE UP UVC BULK CLASS Driver  !!");
			DEV->BUFF.size = 512;
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size,GFP_DMA, MODULE_DRI_USBH);
			} while (DEV->BUFF.ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
			if(usbh_plug_in_cb(USBH_UVC_BULK_CLASS) == FAIL){
					uvc_task_init(DEV->device_id);

#if defined (CONFIG_USBH_FREE_RTOS)
				if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
					USBH_QUEUE_BULK_REQ[DEV->device_id] = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(EHCI_BULK_REQ_STRUCT));
				}
			
				if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
					USBH_QUEUE_BULK_REQ_DATA = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(EHCI_BULK_REQ_STRUCT));
			}
				//xSemaphoreGive(USBH_SEM_WAKEUP_UVC_DRV);
				xQueueSend(usbh_queue_uvc_drv, &DEV->device_id, 0);	
			
#if 0			
			// uvc stream data
			if (usbh_queue_uvc_mw == NULL) {
				usbh_queue_uvc_mw = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(USBH_UVC_STREAM_Structure));
			}
#endif 			
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
					char task_name[30] = {'\0'};
					sprintf((char *)&task_name, "%s%d", "USBH_QUEUE_BULK_REQ", DEV->device_id);
	
					osMessageQDef(task_name, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
					USBH_QUEUE_BULK_REQ[DEV->device_id] = osMessageCreate(osMessageQ(task_name), NULL);
				}
			
				if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
					osMessageQDef(USBH_QUEUE_BULK_REQ_DATA, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
					USBH_QUEUE_BULK_REQ_DATA = osMessageCreate(osMessageQ(USBH_QUEUE_BULK_REQ_DATA), NULL);
				}
				osMessagePut(usbh_queue_uvc_drv, &DEV->device_id, 0);	
			
#if 0			
			// uvc stream data
			if (usbh_queue_uvc_mw == NULL) {
				osMessageQDef(usbh_queue_uvc_mw, USBH_UVC_DATA_QUEUE_SIZE, USBH_UVC_STREAM_Structure);
				usbh_queue_uvc_mw = osMessageCreate(osMessageQ(usbh_queue_uvc_mw), NULL);
			}
#endif 			
#endif
			}
#endif
		} else if (DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
			USBH_DBG("WAKE UP UVC BULK_ISO CLASS Driver  !!");
			DEV->BUFF.size = 512;
			
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size,GFP_DMA, MODULE_DRI_USBH);
			} while (DEV->BUFF.ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif

			if(usbh_plug_in_cb((USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) == FAIL){
				uvc_task_init(DEV->device_id);
				
#if defined (CONFIG_USBH_FREE_RTOS)
			if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
				USBH_QUEUE_BULK_REQ[DEV->device_id] = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(EHCI_BULK_REQ_STRUCT));
			}	
				
			if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
				USBH_QUEUE_BULK_REQ_DATA = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(EHCI_BULK_REQ_STRUCT));
			}
				
#if defined( CONFIG_SNX_ISO_ACCELERATOR )		
#else 
			if (USBH_QUEUE_STD_ISO_REQ == NULL) {
				USBH_QUEUE_STD_ISO_REQ = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(USBH_STD_ISO_QUEUE_STRUCT));
			}
#endif 
			//xSemaphoreGive(USBH_SEM_WAKEUP_UVC_DRV);
			xQueueSend(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
				char task_name[30] = {'\0'};
				sprintf((char *)&task_name, "%s%d", "USBH_QUEUE_BULK_REQ", DEV->device_id);
				
				osMessageQDef(task_name, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
				USBH_QUEUE_BULK_REQ[DEV->device_id] = osMessageCreate(osMessageQ(task_name), NULL);
			}	
				
			if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
				osMessageQDef(USBH_QUEUE_BULK_REQ_DATA, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
				USBH_QUEUE_BULK_REQ_DATA = osMessageCreate(osMessageQ(USBH_QUEUE_BULK_REQ_DATA), NULL);
			}
				
#if defined( CONFIG_SNX_ISO_ACCELERATOR )		
#else 
			if (USBH_QUEUE_STD_ISO_REQ == NULL) {
				osMessageQDef(USBH_QUEUE_STD_ISO_REQ, USBH_UVC_DATA_QUEUE_SIZE, USBH_STD_ISO_QUEUE_STRUCT);
				USBH_QUEUE_STD_ISO_REQ = osMessageCreate(osMessageQ(USBH_QUEUE_STD_ISO_REQ), NULL);
			}
#endif 
			osMessagePut(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif				
			}
#endif
		} else if (DEV->CLASS_DRV == USBH_HID_CLASS) {
#if defined (CONFIG_MODULE_USB_HID_CLASS)
			USBH_DBG("WAKE UP HID CLASS Driver  !!");
			DEV->BUFF.size = 512;

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )			
			do {
				DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
			} while (DEV->BUFF.ptr == NULL);
#endif 			
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)		
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
			if(usbh_plug_in_cb(USBH_HID_CLASS) == FAIL){	
				hid_task_init(DEV->device_id);
				
#if defined (CONFIG_USBH_FREE_RTOS)
				//xSemaphoreGive(USBH_SEM_WAKEUP_HID_DRV);
				xQueueSend(usbh_queue_hid_drv, &DEV->device_id, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osMessagePut(usbh_queue_hid_drv, &DEV->device_id, 0);
#endif
			}
#endif
		}
		break;
	case EHCI_PARSER_ROOT_DEQ:
		EHCI->USBCMD.ASYNC_EN = 0;
		ehci_remove_dev_resource(DEV,EHCI_NO_CHK_ADV,USBH1);
		status = SUCCESS;
		break;
	case EHCI_PARSER_ROOT_HUB_DEQ:
		EHCI->USBCMD.ASYNC_EN = 0;
		for (i = 0; i < USBH_MAX_PORT; i++) {
			DEV = (USBH_Device_Structure *) &USBH_DEV[i];
			ehci_remove_dev_resource(DEV,EHCI_NO_CHK_ADV,USBH1);
		}
		status = SUCCESS;
		break;
	case EHCI_PARSER_PORT_HUB_DEQ:
		ehci_remove_dev_resource(DEV,EHCI_CHK_ADV,USBH1);
		status = SUCCESS;
		break;
	}
	return status;
}

void ehci_rollover_enable(void) {
	EHCI = (SONIX_EHCI_STRUCTURE*) EHCI_REG_BASE_ADDRESS;
	EHCI->USBINTR.ROLLOVER_INT_EN = 1;
}

void ehci_rollover_disable(void) {
	EHCI = (SONIX_EHCI_STRUCTURE*) EHCI_REG_BASE_ADDRESS;
	EHCI->USBINTR.ROLLOVER_INT_EN = 0;
}

uint8_t ehci_take_ep_sem(EHCI_ENDP_STRUCT *EP, uint32_t TimeoutMsec) {
#if defined (CONFIG_USBH_FREE_RTOS)
		if ( xSemaphoreTake((SemaphoreHandle_t)EP->SEM, TimeoutMsec) == pdFAIL) {
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		if ( osSemaphoreWait(EP->SEM, TimeoutMsec) != osOK) {
#endif
		goto ERR;
	}
	return SUCCESS;
ERR: 
	return FAIL;
}

#if defined (CONFIG_MODULE_USB_UVC_CLASS)
uint8_t ehci_move_itd_data_to_buff(EHCI_ENDP_STRUCT *EP, EHCI_ISO_XFR_REQ_Struct *ISO_REQ) {
	EHCI_ITD_STRUCTURE	*ITD		= NULL;
	uint8_t frame_done = NYET;
	uint8_t 						i = 0, j = 0;
	uint8_t							*ptr						= NULL;
	uint8_t							*source_ptr			= NULL;
	uint8_t							*target_ptr			= NULL;
	uint32_t						length					= 0;

	ITD = (EHCI_ITD_STRUCTURE*) EP->pITD[(ISO_REQ->STD_ISO_SM - 1) % 4 * Standard_iTD_interval];

	for (i = 0; i < Standard_iTD_interval; i++) {
		for (j = 0; j < 8; j++) {
NEXT:
			if (ITD->ArrayStatus_Word[j].bLength > 0x0C) {// lenght > 12 bytes
				ptr = (uint8_t*) ((ITD->ArrayBufferPointer_Word[0].bBufferPointer << 12) + ITD->ArrayStatus_Word[j].bOffset);
				EP->UVC_Header = *(ptr + 1);
				length = ITD->ArrayStatus_Word[j].bLength - 12;

				// check tog change 
				if ((EP->LAST_TOG) != (EP->UVC_Header & 0x01)) {
					if (EP->LAST_TOG == 0) {
						// Ping frame done
						EP->PING_FRAME_DONE = 1;
						EP->PONG_FRAME_DONE = 0;
						// Restart pong buff 
						EP->PONG_FRAME_PTR = (uint8_t*) ISO_REQ->STD_ISO_PONG_PTR;
						EP->PONG_FRAME_SIZE = 0;
					} else {
						// Pong frame done					
						EP->PONG_FRAME_DONE = 1;
						EP->PING_FRAME_DONE = 0;
						// Restart ping buff 						
						EP->PING_FRAME_PTR = (uint8_t*) ISO_REQ->STD_ISO_PING_PTR;
						EP->PING_FRAME_SIZE = 0;
					}
					EP->LAST_TOG++;
					frame_done = SUCCESS;
				}

				// pointer to target buffer 
				if (EP->LAST_TOG == 0) {
					if (EP->PING_FRAME_DONE) {
						UVC_DBG("STD ISO PING BUFFER OVERFLOW , SKIP This uSOF DATA");
						goto NEXT;
					}
					target_ptr = EP->PING_FRAME_PTR;
				} else {
					if (EP->PONG_FRAME_DONE) {
						UVC_DBG("STD ISO PONG BUFFER OVERFLOW , SKIP This uSOF DATA");
						goto NEXT;
					}
					target_ptr = EP->PONG_FRAME_PTR;
				}

				//pointer to source buffer 
				source_ptr = ptr + 12;

				// copy data to target buffer 
				memcpy(target_ptr, source_ptr, length);

				// update ptr & size3 
				if (EP->LAST_TOG == 0) {
					EP->PING_FRAME_PTR += length;
					EP->PING_FRAME_SIZE += length;
				} else {
					EP->PONG_FRAME_PTR += length;
					EP->PONG_FRAME_SIZE += length;
				}
			}
		}
		ITD++;
	}	
	return frame_done;
}
#endif

#if defined( CONFIG_DUAL_HOST )	
void ehci_hcd_init_2(void) {
	uint32_t *ptr = NULL;
	
	//Init EHCI	Register Address
	EHCI2					=	(SONIX_EHCI_STRUCTURE*)EHCI_REG_BASE_ADDRESS_2;

	//Turn off PortPower
	EHCI2->PORTSC[0].PP			=	0;

	// HCRESET
	EHCI2->USBCMD.HCRESET			=	1;
	while(EHCI2->USBCMD.HCRESET	== 1);

#if defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 ) 
	EHCI2->USBMODE				=	0x03;
#endif
	
	//EHCI INT Init 
	EHCI2->USBINTR.ASYNC_ADVANCE_INT_EN	=	1;
	EHCI2->USBINTR.SYS_ERR_INT_EN		=	1;
	EHCI2->USBINTR.ROLLOVER_INT_EN		=	0;
	EHCI2->USBINTR.PO_CHG_INT_EN		=	0;
	EHCI2->USBINTR.USB_ERR_INT_EN		=	1;
	EHCI2->USBINTR.USB_INT_EN		=	1;
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X) || defined ( CONFIG_PLATFORM_SN7320 )
	EHCI2->USBINTR.BUF_FULL_INT_EN		=	0;
#endif

	// Special register	init
#if defined( CONFIG_PLATFORM_SN9866X ) || defined( CONFIG_PLATFORM_ST53510 ) || defined( CONFIG_PLATFORM_SN9868X) || defined( CONFIG_PLATFORM_SN7320 )
	EHCI2->MISC.LEV_INT_EN 			= 	1;	// Keep LEVEL INT Alive , add by Hammer 201471003
#endif 
	EHCI2->USBCMD.INT_CTR			=	1;
	EHCI2->USBCMD.PARK_EN			=	0;
	EHCI2->USBCMD.PARK_CNT			=	3;
	EHCI2->CONFIG_FLAG			=	1;

	// frame list size init
	EHCI2->USBCMD.FRAME_CTR			=	2;
	
#if defined( CONFIG_PLATFORM_ST53510 ) || defined( CONFIG_PLATFORM_SN7320 )	
	EHCI2->EC_CTL.DENOISE_EN		=	1;
#endif 
#if defined( CONFIG_PLATFORM_SN7320 )	
	EHCI2->EC_CTL.UFC_EOF_TO_EN			=	1;
	EHCI2->EC_CTL.SX_ACT_CLR_EN			=	1;
	EHCI2->EC_CTL.SX_FB_ACT_CLR_EN	=	1;
#endif
		
	EHCI2->PORTSC[0].PP			=	1;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	vTaskDelay(USBH_100ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osDelay(USBH_100ms);
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do{
		ptr = pvPortMalloc(EHCI_STRUCT_SIZE,GFP_DMA,MODULE_DRI_USBH);
	}while(ptr == NULL);	
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do{
		ptr = pvPortMalloc(EHCI_STRUCT_SIZE);
	}while(ptr == NULL);
#endif
#endif

	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		if(RF_USBH_Buf_PTR == 0)
			RF_USBH_Buf_PTR = 0x900000+EHCI_STRUCT_SIZE+PACKET_MAX_CNT*PACKET_SIZE+1024;
		ptr = (uint32_t *)RF_USBH_Buf_PTR;
		RF_USBH_Buf_PTR += EHCI_STRUCT_SIZE;
#endif
#endif

	// Clear mem
	memset(ptr,0,EHCI_STRUCT_SIZE);	
	
	// record origin allocate mem ptr 
	USBH_MEM_PTR_2	= ptr;		
	
	// do 4K alignment
	ptr	= (uint32_t*)((((uint32_t)ptr)&0xFFFFF000)+0x1000);
	
	ehci_struct_init_2(ptr);
	EHCI2->USBCMD.ASYNC_EN			=	1;
}

void ehci_hcd_uninit_2(void) {

	// Disable USBH DMA
	EHCI2->USBCMD.ASYNC_EN 			= 0;
	EHCI2->USBCMD.PERIODIC_EN		= 0;

	// HCRESET
	EHCI2->USBCMD.HCRESET			= 1;
	while(EHCI2->USBCMD.HCRESET	== 1);
}

void ehci_struct_init_2(uint32_t *ptr)
{
	uint32_t i;
	
#if defined (CONFIG_MODULE_USB_UVC_SRAM)
	QH_STRUCTURE_2	=	(uint8_t*)ptr;	// PTR + 0K
	ptr += 0x400;
	QTD_STRUCTURE_2	=	(uint8_t*)ptr;	// PTR + 4K
	ptr += 0x400;
//	ITD_STRUCTURE_2	=	(uint8_t*)ptr;	// PTR + 12K
//	ptr += 0x400;
	SXITD_STRUCTURE_2 =	(uint8_t*)ptr;	// PTR + 16K
	ptr += 0x400;
	FRAME_LIST_2	=	(EHCI_PERIODIC_FRAME_LIST_STRUCT*)ptr;	// PTR + 20K
#else
	QH_STRUCTURE_2	=	(uint8_t*)ptr;	// PTR + 0K
	ptr += 0x400;
	QTD_STRUCTURE_2	=	(uint8_t*)ptr;	// PTR + 4K
	ptr += 0x800;
	ITD_STRUCTURE_2	=	(uint8_t*)ptr;	// PTR + 12K
	ptr += 0x400;
	SXITD_STRUCTURE_2 =	(uint8_t*)ptr;	// PTR + 16K
	ptr += 0x400;
	FRAME_LIST_2	=	(EHCI_PERIODIC_FRAME_LIST_STRUCT*)ptr;	// PTR + 20K
#endif

	memset((uint8_t*)&EHCI_QH_Manage_2,0,EHCI_QH_MAX);
	memset((uint8_t*)&EHCI_QTD_Manage_2,0,EHCI_QTD_MAX);
	memset((uint8_t*)&EHCI_ITD_Manage_2,0,EHCI_ITD_MAX);
	memset((uint8_t*)&EHCI_SXITD_Manage_2,0,EHCI_SXITD_MAX);
	memset((uint8_t*)&PERIODIC_TABLE_2,0,sizeof(PERIODIC_TABLE));		
		
	// Init Head QH 
	EHCI_QH_Manage_2[0]				=	EHCI_MEM_USED;	// Reserve for Head
	pHEAD_QH_2					=	(EHCI_QH_STRUCTURE*)&QH_STRUCTURE_2[0];
	pHEAD_QH_2->bOverlay_Status			=	0x40;	//Set Halt bit
	pHEAD_QH_2->bHeadOfReclamationListFlag		=	1;	//Set Head
	pHEAD_QH_2->bType				=	EHCI_NEXT_TYPE_QH;
	pHEAD_QH_2->bNextQHDPointer			=	(((uint32_t) &QH_STRUCTURE_2[0]) >> 5);		

	// Init FrameList
	for	(i=0;i<Host20_Preiodic_Frame_List_MAX;i++){
		FRAME_LIST_2->sCell[i].bTerminal	=	1;
	}	

	// Init Async Schedule link	pointer
	EHCI2->ASYNCLISTADDR.LPL			=	(((uint32_t) &QH_STRUCTURE_2[0]) >> 5);
	
	// Init Async Schedule link	pointer
	EHCI2->PERIODICLISTBASE.BASE_ADDR		=	(((uint32_t) &FRAME_LIST_2[0]) >> 12);
}

void ehci_struct_uninit_2(void){
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree((void *)USBH_MEM_PTR_2);
#endif
}

#if defined (CONFIG_USBH_FREE_RTOS)
void ehci_xfr_chk_isr_2(void) {
		static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		uint32_t i = 0, j = 0;

		for (i = 0; i < USBH_MAX_PORT; i++) {
			for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
				if (USBH_DEV_2[i].EP[j].XfrType != 0) {// filter Null type	& iso type
					if (USBH_DEV_2[i].EP[j].XfrType <= 5) {
						if ((USBH_DEV_2[i].EP[j].pQH->bOverlay_InterruptOnComplete) && ((USBH_DEV_2[i].EP[j].pQH->bOverlay_Status & 0x80) == 0x00)) {
							USBH_DEV_2[i].EP[j].pQH->bOverlay_InterruptOnComplete = 0;
							USBH_DEV_2[i].EP[j].pQH->bOverlay_Status = 0x04;
	
							xSemaphoreGiveFromISR(USBH_DEV_2[i].EP[j].SEM, &xHigherPriorityTaskWoken);
						}
						else if((USBH_DEV_2[i].EP[j].pQH->bOverlay_InterruptOnComplete == 0x00)	&& ((USBH_DEV_2[i].EP[j].pQH->bOverlay_Status & 0x84) == 0x00) && (USBH_DEV_2[i].EP[j].pQH->bOverlay_TotalBytes != 0x00)){
							USBH_DEV_2[i].EP[j].pQH->bOverlay_Status = 0x04;
							// modify status to 0x04 to avoid bug that
							// bk short packet result in cx accidentally enter here
	
							if(USBH_DEV_2[i].EP[j].XfrType != EHCI_CX_TYPE)
							xSemaphoreGiveFromISR(USBH_DEV_2[i].EP[j].SEM, &xHigherPriorityTaskWoken);
						}
					} else if ((USBH_DEV_2[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE) || (USBH_DEV_2[i].EP[j].XfrType == EHCI_ISO_IN_TYPE)) {
								
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )
						if(USBH_DEV_2[i].EP[j].LastFID != USBH_DEV_2[i].EP[j].pSXITD->bFID_Count) {
							uvc_process_data((USBH_Device_Structure *)&USBH_DEV_2[i],(EHCI_ENDP_STRUCT *)&USBH_DEV_2[i].EP[j]);
						}
#else 
						if (USBH_DEV_2[i].EP[j].LastITD != 0) {
							if ((USBH_DEV_2[i].EP[j].LastITD->ArrayStatus_Word[7].bInterruptOnComplete == 1) && (USBH_DEV_2[i].EP[j].LastITD->ArrayStatus_Word[7].bLength != 0xC00)) {
								xSemaphoreGiveFromISR(USBH_DEV_2[i].EP[j].SEM,&xHigherPriorityTaskWoken);
							}
						}
#endif
#endif
					}
#if !defined ( CONFIG_PLATFORM_SN7300 )	&& !defined ( CONFIG_PLATFORM_SN7320 )					
					vTaskSwitchContext();
#else
				taskYIELD();
//				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);				
#endif 					
				}
			}
		}
}

void ehci_err_chk_isr_2(void) {
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint32_t i = 0, j = 0;
	
	for(i=0;i<USBH_MAX_PORT;i++){	
		for(j=0;j<MAX_QH_PER_DEVICE;j++){
			if(USBH_DEV_2[i].EP[j].XfrType !=0){		// filter Null type	& iso type
				if(USBH_DEV_2[i].EP[j].XfrType <=	5){
					if(USBH_DEV_2[i].EP[j].pQH->bOverlay_Status & 0x78){
						xSemaphoreGiveFromISR(USBH_DEV_2[i].EP[j].SEM, &xHigherPriorityTaskWoken);
					}
				}else if((USBH_DEV_2[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE) || (USBH_DEV_2[i].EP[j].XfrType	== EHCI_ISO_IN_TYPE)){
#if defined(CONFIG_MODULE_USB_UVC_CLASS)
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )					
					//if((USBH_DEV_2[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV_2[i].EP[j].pSXITD->bBabble_Det) || (USBH_DEV_2[i].EP[j].pSXITD->bUNDERFLOW)) {
					if((USBH_DEV_2[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV_2[i].EP[j].pSXITD->bBabble_Det)) {
						uvc_process_err_data((USBH_Device_Structure *)&USBH_DEV_2[i],(EHCI_ENDP_STRUCT *)&USBH_DEV_2[i].EP[j]);
					}															
#endif 
#endif
				}		
			}
		}
	}	
}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void ehci_xfr_chk_isr_2(void)
{
		//static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		uint32_t i, j;
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )
	
#endif 
		for (i = 0; i < USBH_MAX_PORT; i++) {
			for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
				if (USBH_DEV_2[i].EP[j].XfrType != 0) {// filter Null type	& iso type
					if (USBH_DEV_2[i].EP[j].XfrType <= 5) {
						if ((USBH_DEV_2[i].EP[j].pQH->bOverlay_InterruptOnComplete) && ((USBH_DEV_2[i].EP[j].pQH->bOverlay_Status & 0x80) == 0x00)) {
							USBH_DEV_2[i].EP[j].pQH->bOverlay_InterruptOnComplete = 0;
							USBH_DEV_2[i].EP[j].pQH->bOverlay_Status = 0x04;
	
							osSemaphoreRelease(USBH_DEV_2[i].EP[j].SEM);
						}
						else if((USBH_DEV_2[i].EP[j].pQH->bOverlay_InterruptOnComplete == 0x00)	&& ((USBH_DEV_2[i].EP[j].pQH->bOverlay_Status & 0x84) == 0x00) && (USBH_DEV_2[i].EP[j].pQH->bOverlay_TotalBytes != 0x00)){
							USBH_DEV_2[i].EP[j].pQH->bOverlay_Status = 0x04;
							// modify status to 0x04 to avoid bug that
							// bk short packet result in cx accidentally enter here
	
							osSemaphoreRelease(USBH_DEV_2[i].EP[j].SEM);
						}
					} else if ((USBH_DEV_2[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE) || (USBH_DEV_2[i].EP[j].XfrType == EHCI_ISO_IN_TYPE)) {
								
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )					
						if(USBH_DEV_2[i].EP[j].LastFID != USBH_DEV_2[i].EP[j].pSXITD->bFID_Count) {
							uvc_process_data((USBH_Device_Structure *)&USBH_DEV_2[i],(EHCI_ENDP_STRUCT *)&USBH_DEV_2[i].EP[j]);
						} 
#else 
						if (USBH_DEV_2[i].EP[j].LastITD != 0) {
							if ((USBH_DEV_2[i].EP[j].LastITD->ArrayStatus_Word[7].bInterruptOnComplete == 1) && (USBH_DEV_2[i].EP[j].LastITD->ArrayStatus_Word[7].bLength != 0xC00)) {
								osSemaphoreRelease(USBH_DEV_2[i].EP[j].SEM);
							
							}
						}
#endif
#endif
					}
				}
			}
		}

}

void ehci_err_chk_isr_2(void) {
	uint32_t i = 0, j = 0;
	
	for(i=0;i<USBH_MAX_PORT;i++){	
		for(j=0;j<MAX_QH_PER_DEVICE;j++){
			if(USBH_DEV_2[i].EP[j].XfrType !=0){		// filter Null type	& iso type
				if(USBH_DEV_2[i].EP[j].XfrType <=	5){
					if(USBH_DEV_2[i].EP[j].pQH->bOverlay_Status & 0x78){
						osSemaphoreRelease(USBH_DEV_2[i].EP[j].SEM);
					}
				}else if((USBH_DEV_2[i].EP[j].XfrType == EHCI_ISO_OUT_TYPE) || (USBH_DEV_2[i].EP[j].XfrType	== EHCI_ISO_IN_TYPE)){
#if defined(CONFIG_MODULE_USB_UVC_CLASS)
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )					
					//if((USBH_DEV_2[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV_2[i].EP[j].pSXITD->bBabble_Det) || (USBH_DEV_2[i].EP[j].pSXITD->bUNDERFLOW) ) {
					if((USBH_DEV_2[i].EP[j].pSXITD->bTransaction_Error) || (USBH_DEV_2[i].EP[j].pSXITD->bBabble_Det)) {
						uvc_process_err_data((USBH_Device_Structure *)&USBH_DEV_2[i],(EHCI_ENDP_STRUCT *)&USBH_DEV_2[i].EP[j]);						
					}															

#endif 
#endif
				}		
			}
		}
	}	
}
#endif

#if defined( CONFIG_SN_GCC_SDK )
void ehci_isr_2(int irq)
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
void ehci_isr_2(void)
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
void USBHOST_IRQHandler_2(void)
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
void USB_HOST1_IRQHandler(void)
#endif 

#if defined( CONFIG_XILINX_SDK ) && defined ( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
void ehci_isr_2(void *HandlerRef)
#endif
{
#if defined (CONFIG_USBH_FREE_RTOS)
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif
	EHCI_USBSTS cur_USBSTS;
	cur_USBSTS = EHCI2->USBSTS;
	EHCI2->USBSTS = EHCI2->USBSTS;
	
	if (cur_USBSTS.USB_INT == 1) {
		ehci_xfr_chk_isr_2();
	}
	
	if (cur_USBSTS.USB_ERR_INT == 1) {
		ehci_err_chk_isr_2();		
	}

#if defined (CONFIG_PLATFORM_SN7320)
if((EHCI2->USBINTR.BUF_FULL_INT_EN == 1) && (cur_USBSTS.BUF_FULL == 1)) {
		EHCI2->USBINTR.BUF_FULL_INT_EN = 0;
		ehci_err_chk_isr_2();
		//EHCI2->USBSTS.BUF_FULL = 1;
	}
#endif

	if((cur_USBSTS.SYS_ERR_INT == 1) || (cur_USBSTS.HC_HALTED)){
		ehci_err_chk_isr_2();
	}
	if(cur_USBSTS.ROLLOVER_INT == 1){
#if defined (CONFIG_USBH_FREE_RTOS)
		xSemaphoreGiveFromISR(USBH_SEM_FRAME_LIST_ROLLOVER_2, &xHigherPriorityTaskWoken);		
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osSemaphoreRelease(USBH_SEM_FRAME_LIST_ROLLOVER);		
#endif
	}	
	if(cur_USBSTS.ASYNC_ADVANCE_INT == 1){
#if defined (CONFIG_USBH_FREE_RTOS)
		xSemaphoreGiveFromISR(USBH_SEM_ASYNC_ADV_2, &xHigherPriorityTaskWoken);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osSemaphoreRelease(USBH_SEM_ASYNC_ADV_2);
#endif
	}
#if defined (CONFIG_USBH_FREE_RTOS)
#if !defined ( CONFIG_PLATFORM_SN7300 )	&& !defined ( CONFIG_PLATFORM_SN7320 )		
		vTaskSwitchContext();		
#else
		taskYIELD();
//		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	INTC_IrqClear(INTC_USB2HOST2_IRQ);
#if defined (CONFIG_USBH_FREE_RTOS)
	vTaskSwitchContext();
#endif
#endif 	
} 

void ehci_link_framelist_2(void){
	uint32_t i,j,k;
	uint32_t curr_order,next_order; 
	uint32_t curr_type,next_type;	
	uint32_t curr_index;
	EHCI_QH_STRUCTURE		*QH	=	NULL;
	EHCI_ITD_STRUCTURE		*ITD	=	NULL;
	EHCI_SXITD_STRUCTURE		*SXITD  =	NULL;	
	
	//link by interval 
	if(PERIODIC_TABLE_2.link_tail<= 1) {			// no next interval 
		curr_type	=	PERIODIC_TABLE_2.element[0].type;		
		if(curr_type ==	EHCI_INT){
			QH	=	(EHCI_QH_STRUCTURE*)PERIODIC_TABLE_2.element[0].ptr;			
			QH->bTerminate		=	1;
		}else if(curr_type == EHCI_ISO){
			for(k=0;k<Standard_iTD_EP_Max_Count;k++){
				ITD =	(EHCI_ITD_STRUCTURE*)(((uint32_t)PERIODIC_TABLE_2.element[0].ptr)+(EHCI_ITD_SIZE*k)); 
				ITD->bTerminate		=	1;			
			}
		}else if(curr_type == EHCI_SXISO){
			SXITD	=	(EHCI_SXITD_STRUCTURE*)PERIODIC_TABLE_2.element[0].ptr;			
			SXITD->bTerminate		=	1;
		}
	}else{
		for(i=0;i<Host20_Preiodic_Frame_List_MAX;i++){	// do next interval 
			curr_index		=	0;	
			for(j=0;j<PERIODIC_TABLE_2.link_tail;j++){
				curr_type	=	PERIODIC_TABLE_2.element[curr_index].type;		
				if(curr_type ==	EHCI_INT){
					QH		=	(EHCI_QH_STRUCTURE*)PERIODIC_TABLE_2.element[curr_index].ptr;			
				}else if(curr_type == EHCI_ISO){
					ITD		=	(EHCI_ITD_STRUCTURE*)((uint32_t)(PERIODIC_TABLE_2.element[curr_index].ptr)+(EHCI_ITD_SIZE*(i%Standard_iTD_EP_Max_Count)));				
				}else if(curr_type == EHCI_SXISO){
					SXITD	=	(EHCI_SXITD_STRUCTURE*)PERIODIC_TABLE_2.element[curr_index].ptr;			
				}
				next_order  =	PERIODIC_TABLE_2.element[j+1].order;									
				if(next_order == 0) {
					if(curr_type == EHCI_INT){
						QH->bTerminate		=	1;
					}else if(curr_type == EHCI_ISO){
						ITD->bTerminate		=	1;			
					}else if(curr_type == EHCI_SXISO){
						SXITD->bTerminate	=	1;
					}
					break;
				}
				if(!(i%next_order)){			
					switch(curr_type){
						case EHCI_INT:
							next_type	=	PERIODIC_TABLE_2.element[j+1].type;					
							if(next_type == EHCI_INT){
								QH->bType		=	EHCI_QH_TYPE;	
								QH->bNextQHDPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j+1].ptr) >> 5);		
								QH->bTerminate		=	0;
							}else if(next_type == EHCI_SXISO){
								QH->bType		=	EHCI_ITD_TYPE;
								QH->bNextQHDPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j+1].ptr) >> 5);					
								QH->bTerminate		=	0;
							}else{							
								QH->bTerminate		=	1;
							}				
							break;
						case EHCI_ISO:
							next_type	=	PERIODIC_TABLE_2.element[j+1].type;					
							if(next_type == EHCI_INT){
								ITD->bType		=	EHCI_QH_TYPE;
								ITD->bNextLinkPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j+1].ptr) >> 5);									
								ITD->bTerminate		=	0;
							}else if(next_type == EHCI_ISO){
								ITD->bType		=	EHCI_ITD_TYPE;
								ITD->bNextLinkPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j+1].ptr)+(EHCI_ITD_SIZE*(i%Standard_iTD_EP_Max_Count)) >> 5);	
								ITD->bTerminate		=	0;
							}else if(next_type == EHCI_SXISO){
								ITD->bType		=	EHCI_ITD_TYPE;
								ITD->bNextLinkPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j+1].ptr) >> 5);					
								ITD->bTerminate		=	0;
							}else{
								ITD->bTerminate		=	1;
							}				
							break;						
						case EHCI_SXISO:
							next_type	=	PERIODIC_TABLE_2.element[j+1].type;					
							if(next_type == EHCI_SXISO){
								SXITD->bType		=	EHCI_ITD_TYPE;
								SXITD->bNextLinkPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j+1].ptr) >> 5);					
								SXITD->bTerminate	=	0;					
							}else{
								SXITD->bTerminate	=	1;
							}				
							break;				
						default:
							USBH_DBG("Relink Err ! ");
							break;
					}			
					curr_index = j+1;
				}				
			}
		}	
	}

	// link to framelist 
	if(PERIODIC_TABLE_2.link_tail	== 0){	// Table empty 
		for(i=0;i<Host20_Preiodic_Frame_List_MAX;i++){
			FRAME_LIST_2->sCell[i].bTerminal	=	1;
		}
	}else{
		for(i=0;i<Host20_Preiodic_Frame_List_MAX;i++){
			for(j=0;j<PERIODIC_TABLE_2.link_tail;j++){
				curr_order  =	PERIODIC_TABLE_2.element[j].order;
				if(!(i%curr_order)){
					if(PERIODIC_TABLE_2.element[j].type == EHCI_INT){
						FRAME_LIST_2->sCell[i].bLinkPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j].ptr) >> 5);
						FRAME_LIST_2->sCell[i].bType		=	EHCI_QH_TYPE;
					}else if(PERIODIC_TABLE_2.element[j].type == EHCI_ISO){
						FRAME_LIST_2->sCell[i].bLinkPointer	=	((((uint32_t)PERIODIC_TABLE_2.element[j].ptr)+(EHCI_ITD_SIZE*(i%Standard_iTD_EP_Max_Count))) >> 5);
						FRAME_LIST_2->sCell[i].bType		=	EHCI_ITD_TYPE;
					}else if(PERIODIC_TABLE_2.element[j].type == EHCI_SXISO){
						FRAME_LIST_2->sCell[i].bLinkPointer	=	(((uint32_t)PERIODIC_TABLE_2.element[j].ptr) >> 5);
						FRAME_LIST_2->sCell[i].bType		=	EHCI_ITD_TYPE;		
					}
					FRAME_LIST_2->sCell[i].bTerminal	=	0;			
					break;
				} else {
					FRAME_LIST_2->sCell[i].bTerminal	=	1;
				}
			}
		}	
	}
}

void ehci_add_periodic_table_2(uint32_t type, uint32_t order, uint32_t *ptr) {
	uint32_t i,j;
	uint32_t curr_order	=	0;
	uint32_t curr_type	=	0;
	uint32_t curr_index	=	0;
	EHCI_PERIODIC_ELEMENT_Struct	temp_element;

	// add  
	curr_index = PERIODIC_TABLE_2.link_tail;
	PERIODIC_TABLE_2.element[curr_index].type	=	type;
	if(type == EHCI_INT){
		curr_order = order2power(order);
	}else if(type == EHCI_ISO){
		curr_order = 0xFFF;
	}else if(type == EHCI_SXISO){
		curr_order = 0x0;	
	}
	PERIODIC_TABLE_2.element[curr_index].order	=	curr_order;
	PERIODIC_TABLE_2.element[curr_index].ptr	=	ptr;
	PERIODIC_TABLE_2.link_tail++;

	// sorting 
	for(i=0;i<PERIODIC_TABLE_2.link_tail;i++){
		for(j=i+1;j<PERIODIC_TABLE_2.link_tail;j++){
			if(PERIODIC_TABLE_2.element[i].order < PERIODIC_TABLE_2.element[j].order){
				temp_element = PERIODIC_TABLE_2.element[j];
				PERIODIC_TABLE_2.element[j] =	PERIODIC_TABLE_2.element[i];
				PERIODIC_TABLE_2.element[i] =	temp_element;
			}
		}
	}

	// fix iso order 
	for(i=0;i<PERIODIC_TABLE_2.link_tail;i++){
		curr_type = PERIODIC_TABLE_2.element[i].type;
		if((curr_type == EHCI_ISO) || (curr_type == EHCI_SXISO)){
			PERIODIC_TABLE_2.element[i].order = 1;
		}
	}	
	
	//enable periodic schedule
	if(EHCI2->USBCMD.PERIODIC_EN==0){
		EHCI2->USBCMD.PERIODIC_EN	=	1;
		
		vTaskDelay(USBH_2ms);
	}

	//Periodic table link to framelist
	ehci_link_framelist_2();

}

void ehci_remove_periodic_table_2(void *ptr)
{	
	uint32_t i,j;
	uint32_t curr_type;	
	EHCI_PERIODIC_ELEMENT_Struct	temp_element;

	// find & remove
	for(i=0;i<PERIODIC_TABLE_2.link_tail;i++){
		if(PERIODIC_TABLE_2.element[i].type == EHCI_ISO){
			PERIODIC_TABLE_2.element[i].order	=	0xFFF;
		}else if(PERIODIC_TABLE_2.element[i].type	==	EHCI_SXISO){
			PERIODIC_TABLE_2.element[i].order	=	0;
		}
		if(PERIODIC_TABLE_2.element[i].ptr == ptr){
			PERIODIC_TABLE_2.element[i].type	=	0;
			PERIODIC_TABLE_2.element[i].order	=	0;
			PERIODIC_TABLE_2.element[i].ptr	=	0;
			break;
		}
	}

	// sorting 
	for(i=0;i<PERIODIC_TABLE_2.link_tail;i++){
		for(j=i+1;j<PERIODIC_TABLE_2.link_tail;j++){
			if(PERIODIC_TABLE_2.element[i].order < PERIODIC_TABLE_2.element[j].order){
				temp_element = PERIODIC_TABLE_2.element[j];
				PERIODIC_TABLE_2.element[j]	=	PERIODIC_TABLE_2.element[i];
				PERIODIC_TABLE_2.element[i]	=	temp_element;
			}
		}
	}
	PERIODIC_TABLE_2.link_tail--;		

	// fix iso order 
	for(i=0;i<PERIODIC_TABLE_2.link_tail;i++){
		curr_type = PERIODIC_TABLE_2.element[i].type;
		if((curr_type == EHCI_ISO) || (curr_type	==	EHCI_SXISO)){
			 PERIODIC_TABLE_2.element[i].order	=	1;
		}
	}

	//Periodic table link to framelist
	ehci_link_framelist_2();

	if(PERIODIC_TABLE_2.link_tail	== 0){
		if(EHCI2->USBCMD.PERIODIC_EN == 1){
			EHCI2->USBCMD.PERIODIC_EN	=	0;
			
			EHCI2->FRINDEX.FRINDEX &= 0x3FF8;
			vTaskDelay(USBH_2ms);
			
			EHCI2->FRINDEX.FRINDEX |= 0x0007;
			vTaskDelay(USBH_2ms);
		}
	}
}
	
uint8_t ehci_parser_dev_2(USBH_Device_Structure *DEV, uint8_t FUN)
{
	EHCI_ALLOC_REQUEST_Struct AllocReq;
	uint16_t PID, VID;
	uint8_t ifclass, subclass;
	uint8_t i, j, k = 0;
	uint8_t status = SUCCESS;
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
	uint32_t idle_times = 0;
#endif
#endif

	switch (FUN) {
		case EHCI_PARSER_ROOT_DET:
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
		PHY_510setting_1_host2();
for(idle_times = 0; idle_times < 100; idle_times++);
#endif
#endif		
			if (USBH_DEV_2[USBH_ROOT].bDevIsConnected == 0) {
				if (EHCI2->PORTSC[0].CURR_CNNT_ST == 1) {

#if defined (CONFIG_USBH_FREE_RTOS)
					for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
						if (DEV->EP[i].SEM != NULL) {
							vSemaphoreDelete(DEV->EP[i].SEM);
							DEV->EP[i].SEM = NULL;
						}
						if (DEV->EP[i].Mutex != NULL) {
							vSemaphoreDelete(DEV->EP[i].Mutex);
							DEV->EP[i].Mutex = NULL;
						}
					}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					for (i = 0; i < MAX_QH_PER_DEVICE; i++) {
						if (DEV->EP[i].SEM != NULL) {
							osSemaphoreDelete(DEV->EP[i].SEM);
							DEV->EP[i].SEM = NULL;
						}
						if (DEV->EP[i].Mutex != NULL) {
							osMutexDelete(DEV->EP[i].Mutex);
							DEV->EP[i].Mutex = NULL;
						}
					}
#endif
					memset(DEV, 0, sizeof(USBH_Device_Structure));
					DEV->device_id = 5;
					DEV->bDevIsConnected = 1;
					DEV->ENUM_SM = USBH_ROOT_PLUG_IN_STATE;
					USBH_DBG("Detect Plug In (host2)!!");
				}
			} else if (USBH_DEV_2[USBH_ROOT].bDevIsConnected == 1) {
				if (EHCI2->PORTSC[0].CURR_CNNT_ST == 0) {
					DEV->bDevIsConnected = 0;
					DEV->ENUM_SM = USBH_ROOT_PLUG_OUT_STATE;
					USBH_DBG("Detect Plug Out (host2)!!");
				}
			}
			status = SUCCESS;
			
#if defined (CONFIG_PLATFORM_ST53510) || defined (CONFIG_PLATFORM_SN9868X)
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
			PHY_510setting_2_host2();
#endif
#endif		
			break;
		case EHCI_PARSER_ROOT_RST:
			// CHK ASYNC ENABLE
			if (EHCI2->USBCMD.ASYNC_EN == 0) {
				EHCI2->USBCMD.ASYNC_EN = 1;
				while (EHCI2->USBCMD.ASYNC_EN == 0);
			}
			// CHK RUN STOP
			if (EHCI2->USBCMD.RUN_NSTOP == 0) {
				EHCI2->USBCMD.RUN_NSTOP = 1;
				while (EHCI2->USBCMD.RUN_NSTOP == 0);
			}
			// Do Reset
#if defined (CONFIG_USBH_FREE_RTOS)
			vTaskDelay(USBH_5ms);
			USBH_DBG("DO USB RESET (host2)... \r\n");
			EHCI2->PORTSC[0].PORTRESET = 1;
			vTaskDelay(USBH_20ms);
			EHCI2->PORTSC[0].PORTRESET = 0;
			while (EHCI2->PORTSC[0].PORTRESET == 1);
			USBH_DBG("USB RESET	DONE (host2)! ");
			vTaskDelay(USBH_5ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osDelay(USBH_5ms);
			USBH_DBG("DO USB RESET (host2)... \r\n");
			EHCI2->PORTSC[0].PORTRESET = 1;
			osDelay(USBH_20ms);
			EHCI2->PORTSC[0].PORTRESET = 0;
			while (EHCI2->PORTSC[0].PORTRESET == 1);
			USBH_DBG("USB RESET	DONE (host2)! ");
			osDelay(USBH_5ms);
#endif
			if (EHCI2->PORTSC[0].CURR_CNNT_ST == 1) {
				if (EHCI2->PORTSC[0].PORTENABLED == 1) {
					DEV->SPEED = EHCI_HIGH_SPEED;
					USBH_DBG("Detected High Speed USB Device on	root (host2)!");
					status = SUCCESS;
				} else {
					DEV->SPEED = EHCI_FULL_SPEED;
					USBH_DBG("Detected Full Speed USB Device on	root ! , Can't Support Full Speed Device (host2)!");
					status = FAIL;
				}
			} else {
				status = FAIL;
			}
			break;
		case EHCI_PARSER_CX_ENQU:
			//ENQUEUE for CX : Addr0 Endp0
			AllocReq.Addr = 0;
			AllocReq.Endp = 0;
			AllocReq.HubAddr = 0;
			AllocReq.Speed = DEV->SPEED;
			AllocReq.MaxPktSize = 8;
			AllocReq.DataSize = 200;
			AllocReq.DataDir = EHCI_DATA_IN;
			DEV->EP[0] = ehci_cx_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
			ehci_adding_qh_tail(DEV->EP[0].pQH,USBH2);
			status = SUCCESS;
			break;
		case EHCI_PARSER_NONCX_ENQU:
			// ENQUEUE for NONCX
			memset(&AllocReq, 0, sizeof(AllocReq));
			VID = (DEV->DEVDes.bVIDHighByte << 8) | (DEV->DEVDes.bVIDLowByte);
			PID = (DEV->DEVDes.bPIDHighByte << 8) | (DEV->DEVDes.bPIDLowByte);

			// By PID VID
#if defined (CONFIG_WIFI_SDK)
		if (0) {	
#else			
			if ((PID == 0x7601) && (VID == 0x148F)) {
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
				// Manufacturer:MediaTek  Product:802.11 n WLAN
				for (i = 0; i < 4; i++) {
					k++;
					AllocReq.Addr = DEV->bAdd;
					AllocReq.Endp = DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_EndpointAddress;
					AllocReq.Speed = DEV->SPEED;
					AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_wMaxPacketSizeHighByte << 8)| DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_wMaxPacketSizeLowByte);
					AllocReq.DataSize = 100 * 1024;
					AllocReq.DataDir = DEV->CFGDes[0].Interface[0].ENDPDes[i].bED_EndpointAddress >> 7;
					DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
					ehci_adding_qh_tail(DEV->EP[k].pQH,USBH2);
				}
				DEV->CLASS_DRV = USBH_WIFI_CLASS;
				status = SUCCESS;
#else
				status = FAIL;
#endif
#endif // end of defined (CONFIG_WIFI_SDK)
			} else if (((PID == 0x6366) && (VID == 0x0c45)) || ((PID == 0x651B) && (VID == 0x0c45)) || ((PID == 0x6365) && (VID == 0x0c45)) 
			|| ((PID == 0x8366) && (VID == 0x0C45)) || ((PID == 0x6367) && (VID == 0x0c45)) || ((PID == 0x8200) && (VID == 0x0c45)) || ((PID == 0x6365) && (VID == 0x2084)) || ((PID == 0xb5e5) && (VID == 0x04f2))) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
for(i=1;i<DEV->CFGDes[0].bINTERFACE_NUMBER;i++) {
					AllocReq.Addr = DEV->bAdd;
					AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_EndpointAddress;
					AllocReq.HubAddr = 0;
					AllocReq.Speed = DEV->SPEED;
					AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_wMaxPacketSizeLowByte);
					AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_EndpointAddress>>7;
					switch(DEV->CFGDes[0].Interface[i].ENDPDes[0].bED_bmAttributes) {
						case EHCI_BULK:
							AllocReq.DataSize = MAX_QTD_PER_QH * 20 * 1024;
							DEV->EP[i] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
							ehci_adding_qh_tail(DEV->EP[i].pQH,USBH2);
							DEV->CLASS_DRV |= USBH_UVC_BULK_CLASS;
							status = SUCCESS;
						break;
					
					case EHCI_ISO:
						DEV->EP[i] = ehci_iso_allocate((EHCI_ALLOC_REQUEST_Struct*)&AllocReq,USBH2);
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
						ehci_add_periodic_table_2(EHCI_SXISO, (uint32_t)NULL, (uint32_t*)DEV->EP[i].pSXITD);
#else
						ehci_add_periodic_table_2(EHCI_ISO,(uint32_t)NULL,(uint32_t *)DEV->EP[i].pITD[0]);			
#endif 
						DEV->CLASS_DRV |= USBH_UVC_ISO_CLASS;
						status = SUCCESS;
						break;
				}
			}
#else
				status = FAIL;
#endif
			} else if ((PID == 0x6362) && (VID == 0x0c45)) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
				// Manufacturer:Sonix Technology Co., Ltd. , Product 283
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
				for(i=1;i<DEV->CFGDes[0].bINTERFACE_NUMBER;i++) {
					AllocReq.Addr = DEV->bAdd;
					AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_EndpointAddress;
					AllocReq.Speed = DEV->SPEED;
					AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_wMaxPacketSizeLowByte);
					AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[1].bED_EndpointAddress>>7;
					DEV->EP[i] = ehci_iso_allocate((EHCI_ALLOC_REQUEST_Struct*)&AllocReq, USBH2);
					ehci_add_periodic_table_2(EHCI_SXISO,(uint32_t)NULL,(uint32_t *)DEV->EP[i].pSXITD);
				}
				DEV->CLASS_DRV = USBH_UVC_ISO_CLASS;
				status = SUCCESS;
#else

#endif
			} else if ((PID == 0x8200) && (VID == 0x0c45)) {
				// Manufacturer:Sonix Technology Co., Ltd. , Product 672
				for (i = 0; i < DEV->CFGDes[0].bINTERFACE_NUMBER; i++){
					for (j = 0; j < DEV->CFGDes[0].Interface[i].bEP_NUMBER; j++){
						k++;
						AllocReq.Addr = DEV->bAdd;
						AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress;
						AllocReq.HubAddr = 0;
						AllocReq.Speed = DEV->SPEED;
						AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeHighByte << 8) | DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeLowByte);
						AllocReq.DataSize = MAX_QTD_PER_QH * 20 * 1024;
						AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress >> 7;
						switch(DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_bmAttributes){
							case EHCI_BULK:
								DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
								ehci_adding_qh_tail(DEV->EP[k].pQH,USBH2);
							break;
							case EHCI_INT:
								USBH_DBG(" INT XFR Not Support yet (host2)");
								break;
						}
					}
				}
				DEV->CLASS_DRV = USBH_UVC_BULK_CLASS;
				status = SUCCESS;
#else
				status = FAIL;
#endif
			} else if ((PID == 0x2060) && (VID == 0x0583)) {
#if defined (CONFIG_MODULE_USB_HID_CLASS)
				// HID Class
				k++;
				AllocReq.Addr 			= DEV->bAdd;
				AllocReq.Endp				= DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress;
				AllocReq.HubAddr 		= 1;
				AllocReq.Speed 			= DEV->SPEED;
				AllocReq.MaxPktSize	= ((DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeHighByte<<8)|DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeLowByte);
				AllocReq.DataSize 	= AllocReq.MaxPktSize;
				AllocReq.DataDir 		= DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress >> 7;
				DEV->EP[k] 					= ehci_int_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
				ehci_add_periodic_table_2(EHCI_INT, DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_Interval, (uint32_t *) DEV->EP[k].pQH);
				DEV->CLASS_DRV 			= USBH_HID_CLASS;
				status = SUCCESS;
#else
				status = FAIL;
#endif
			} else {
				// By CLASS CODE
				for (i = 0; i < DEV->CFGDes[0].bINTERFACE_NUMBER; i++) {
					ifclass = DEV->CFGDes[0].Interface[i].bInterfaceClass;
					subclass = DEV->CFGDes[0].Interface[i].bInterfaceSubClass;
					if ((ifclass == USBH_MASS_STORAGE_CLASS_CODE) && (subclass == 0x06)) {
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
						for (j = 0; j < DEV->CFGDes[0].Interface[i].bEP_NUMBER; j++) {
							k++;
							if (k > MAX_QH_PER_DEVICE - 1) {	// Exclude ENDP0
								USBH_DBG("Out of MAX_QH_PER_DEVICE Size (host2)");
								return SUCCESS;
							}
							AllocReq.Addr = DEV->bAdd;
							AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress;
							AllocReq.HubAddr = 0;
							AllocReq.Speed = DEV->SPEED;
							AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeHighByte << 8)
										| DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeLowByte);
							AllocReq.DataSize = 128 * 1024;
							AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress >> 7;
							switch (DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_bmAttributes) {
								case EHCI_BULK:
									DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
									ehci_adding_qh_tail(DEV->EP[k].pQH,USBH2);
									break;
								case EHCI_INT:
									USBH_DBG("INT XFR Not Support yet (host2)");
									break;
							}
						}
						DEV->CLASS_DRV = USBH_MSC_CLASS;
						status = SUCCESS;
#else
						status = FAIL;
#endif
					} else if (ifclass == USBH_HUB_CLASS_CODE) {
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
						// HUB Class
						k++;
						AllocReq.Addr = DEV->bAdd;
						AllocReq.Endp = DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress;
						AllocReq.HubAddr = 0;
						AllocReq.Speed = DEV->SPEED;
						AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeHighByte << 8)
									| DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_wMaxPacketSizeLowByte);
						AllocReq.DataSize = AllocReq.MaxPktSize;
						AllocReq.DataDir = DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_EndpointAddress >> 7;
						DEV->EP[k] = ehci_int_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
						ehci_add_periodic_table_2(EHCI_INT,DEV->CFGDes[0].Interface[0].ENDPDes[0].bED_Interval,(uint32_t *) DEV->EP[k].pQH);
						DEV->CLASS_DRV = USBH_HUB_CLASS;
						status = SUCCESS;
#else
				    status = FAIL;
#endif
					} else {
						// By Generial	Parser
						for (j = 0; j < DEV->CFGDes[0].Interface[i].bEP_NUMBER; j++) {
							k++;
							if (k > MAX_QH_PER_DEVICE - 1) {	// Exclude ENDP0
								USBH_DBG("Out of MAX_QH_PER_DEVICE Size (host2)");
								return SUCCESS;
							}
							AllocReq.Addr = DEV->bAdd;
							AllocReq.Endp = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress;
							AllocReq.HubAddr = 0;
							AllocReq.Speed = DEV->SPEED;
							AllocReq.MaxPktSize = ((DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeHighByte << 8)
										| DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_wMaxPacketSizeLowByte);
							AllocReq.DataSize = 20480;
							AllocReq.DataDir = DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_EndpointAddress >> 7;
							switch (DEV->CFGDes[0].Interface[i].ENDPDes[j].bED_bmAttributes) {
								case EHCI_CONTROL:
									DEV->EP[k] = ehci_cx_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
									ehci_adding_qh_tail(DEV->EP[k].pQH,USBH2);
									break;
								case EHCI_ISO:
									USBH_DBG("ISO XFR Not Support yet (host2)");
									break;
								case EHCI_BULK:
									DEV->EP[k] = ehci_bk_allocate((EHCI_ALLOC_REQUEST_Struct*) &AllocReq, USBH2);
									ehci_adding_qh_tail(DEV->EP[k].pQH,USBH2);
									break;
								case EHCI_INT:
									USBH_DBG("INT XFR Not Support yet (host2)");
									break;
							}
						}
					}
				}
				status = SUCCESS;
			}
			break;
		case EHCI_PARSER_WAKE_CLASS_DRV:
			if (DEV->CLASS_DRV == USBH_HUB_CLASS) {
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
				USBH_DBG("WAKE UP HUB CLASS Driver (host2)!!");
				DEV->BUFF.size = 512;
			
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )			
				do {
					DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
				} while (DEV->BUFF.ptr == NULL);
#endif 			
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)			
#if defined( CONFIG_SN_KEIL_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
				if(usbh_plug_in_cb(USBH_HUB_CLASS) == FAIL){	
					hub_task_init(DEV->device_id);
					
#if defined (CONFIG_USBH_FREE_RTOS)
					xQueueSend(usbh_queue_hub_drv, &DEV->device_id, 0);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMessagePut(usbh_queue_hub_drv, &DEV->device_id, 0);	
#endif
				}
#endif
#if !defined (CONFIG_WIFI_SDK)
			} else if (DEV->CLASS_DRV == USBH_WIFI_CLASS) {
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
				if(usbh_plug_in_cb(USBH_WIFI_CLASS) == FAIL){		
					USBH_DBG("WAKE UP WIFI CLASS Driver (host2)!!");
					//if (xWiFiTask == NULL)
					//WiFi_Task_Init();
				}
				
#if defined (CONFIG_USBH_FREE_RTOS)
				xSemaphoreGive(USBH_SEM_WAKEUP_WIFI_2);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osSemaphoreRelease(USBH_SEM_WAKEUP_WIFI_2);
#endif
#endif
#endif // end of if !defined (CONFIG_WIFI_SDK)
			} else if (DEV->CLASS_DRV == USBH_MSC_CLASS) {
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
				USBH_DBG("WAKE UP MSC CLASS Driver (host2)!!");
				DEV->BUFF.size = (8+1)*1024;
				
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
				} while (DEV->BUFF.ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif
#endif
				if(usbh_plug_in_cb(USBH_MSC_CLASS) == FAIL){	
					if (xTASK_HDL_MSC_DRV[DEV->device_id] == NULL)
						msc_task_init(USBH2, DEV->device_id);
#if defined (CONFIG_USBH_FREE_RTOS)
					xQueueSend(USBH_QUEUE_WAKEUP_MSC_DRV_2, &DEV->device_id, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMessagePut(USBH_QUEUE_WAKEUP_MSC_DRV_2, &DEV->device_id, 0);
#endif	
				}
#endif
			} else if (DEV->CLASS_DRV == USBH_UVC_ISO_CLASS) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
				USBH_DBG("WAKE UP UVC ISO CLASS	Driver (host2)!!");
				DEV->BUFF.size = 512;
						
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )			
				do {
					DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
				} while (DEV->BUFF.ptr == NULL);
#endif			
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)		
#if defined( CONFIG_SN_KEIL_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
				if(usbh_plug_in_cb(USBH_UVC_ISO_CLASS) == FAIL){	
					uvc_task_init(DEV->device_id);

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
					
#else 
					if (USBH_QUEUE_STD_ISO_REQ == NULL) {
						USBH_QUEUE_STD_ISO_REQ = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(USBH_STD_ISO_QUEUE_STRUCT));
					}
#endif 
					//xSemaphoreGive(USBH_SEM_WAKEUP_UVC_DRV);
					xQueueSend(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
					
#else 	
					if (USBH_QUEUE_STD_ISO_REQ == NULL) {
						osMessageQDef(USBH_QUEUE_STD_ISO_REQ, USBH_UVC_DATA_QUEUE_SIZE, USBH_STD_ISO_QUEUE_STRUCT);
						USBH_QUEUE_STD_ISO_REQ = osMessageCreate(osMessageQ(USBH_QUEUE_STD_ISO_REQ), NULL);
					}
#endif
					osMessagePut(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif
				}
#endif
			} else if (DEV->CLASS_DRV == USBH_UVC_BULK_CLASS) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
				USBH_DBG("WAKE UP UVC BULK CLASS Driver (host2)!!");
				DEV->BUFF.size = 512;
								
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
				} while (DEV->BUFF.ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif
#endif
				if(usbh_plug_in_cb(USBH_UVC_BULK_CLASS) == FAIL) {
				uvc_task_init(DEV->device_id);

#if defined (CONFIG_USBH_FREE_RTOS)
				if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
					USBH_QUEUE_BULK_REQ[DEV->device_id] = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(EHCI_BULK_REQ_STRUCT));
					}	
				
					if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
						USBH_QUEUE_BULK_REQ_DATA = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(EHCI_BULK_REQ_STRUCT));
					}
					
					xQueueSend(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
					char task_name[30] = {'\0'};
					sprintf((char *)&task_name, "%s%d", "USBH_QUEUE_BULK_REQ", DEV->device_id);
	
					osMessageQDef(task_name, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
					USBH_QUEUE_BULK_REQ[DEV->device_id] = osMessageCreate(osMessageQ(task_name), NULL);
					}						
				
					if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
						osMessageQDef(USBH_QUEUE_BULK_REQ_DATA, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
						USBH_QUEUE_BULK_REQ_DATA = osMessageCreate(osMessageQ(USBH_QUEUE_BULK_REQ_DATA), NULL);
					}
					
					osMessagePut(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif
				}				
#endif
			} else if (DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
				USBH_DBG("WAKE UP UVC BULK_ISO CLASS Driver (host2)!!");
				DEV->BUFF.size = 512;
				
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size,GFP_DMA, MODULE_DRI_USBH);
				} while (DEV->BUFF.ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
			do {
				DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
			}while(DEV->BUFF.ptr == NULL);
#endif
#endif

			if(usbh_plug_in_cb((USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) == FAIL){
				uvc_task_init(DEV->device_id);
				
#if defined (CONFIG_USBH_FREE_RTOS)
			if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
				USBH_QUEUE_BULK_REQ[DEV->device_id] = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(EHCI_BULK_REQ_STRUCT));
			}	
				
			if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
				USBH_QUEUE_BULK_REQ_DATA = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(EHCI_BULK_REQ_STRUCT));
			}
#if defined( CONFIG_SNX_ISO_ACCELERATOR )		
#else 
			if (USBH_QUEUE_STD_ISO_REQ == NULL) {
				USBH_QUEUE_STD_ISO_REQ = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,sizeof(USBH_STD_ISO_QUEUE_STRUCT));
			}
#endif 
			//xSemaphoreGive(USBH_SEM_WAKEUP_UVC_DRV);
			xQueueSend(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			if (USBH_QUEUE_BULK_REQ[DEV->device_id] == NULL) {
				char task_name[30] = {'\0'};
				sprintf((char *)&task_name, "%s%d", "USBH_QUEUE_BULK_REQ", DEV->device_id);
				
				osMessageQDef(task_name, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
				USBH_QUEUE_BULK_REQ[DEV->device_id] = osMessageCreate(osMessageQ(task_name), NULL);
			}	
				
			if(USBH_QUEUE_BULK_REQ_DATA == NULL) {
				osMessageQDef(USBH_QUEUE_BULK_REQ_DATA, USBH_UVC_DATA_QUEUE_SIZE, EHCI_BULK_REQ_STRUCT);
				USBH_QUEUE_BULK_REQ_DATA = osMessageCreate(osMessageQ(USBH_QUEUE_BULK_REQ_DATA), NULL);
			}
				
#if defined( CONFIG_SNX_ISO_ACCELERATOR )		
#else 
			if (USBH_QUEUE_STD_ISO_REQ == NULL) {
				osMessageQDef(USBH_QUEUE_STD_ISO_REQ, USBH_UVC_DATA_QUEUE_SIZE, USBH_STD_ISO_QUEUE_STRUCT);
				USBH_QUEUE_STD_ISO_REQ = osMessageCreate(osMessageQ(USBH_QUEUE_STD_ISO_REQ), NULL);
			}
#endif 
			osMessagePut(usbh_queue_uvc_drv, &DEV->device_id, 0);	
#endif				
			}
#endif
		} else if (DEV->CLASS_DRV == USBH_HID_CLASS) {
#if defined (CONFIG_MODULE_USB_HID_CLASS)
				USBH_DBG("WAKE UP HID CLASS Driver (host2)!!");
				DEV->BUFF.size = 512;

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )			
				do {
					DEV->BUFF.ptr = (uint32_t*) pvPortMalloc(DEV->BUFF.size, GFP_DMA, MODULE_DRI_USBH);
				} while (DEV->BUFF.ptr == NULL);
#endif 			
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)pvPortMalloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif	
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)		
#if defined( CONFIG_SN_KEIL_SDK )
				do {
					DEV->BUFF.ptr = (uint32_t*)malloc(DEV->BUFF.size);
				}while(DEV->BUFF.ptr == NULL);
#endif
#endif
				if(usbh_plug_in_cb(USBH_HID_CLASS) == FAIL){	
					hid_task_init(DEV->device_id);

#if defined (CONFIG_USBH_FREE_RTOS)
					//xSemaphoreGive(USBH_SEM_WAKEUP_HID_DRV);
					xQueueSend(usbh_queue_hid_drv, &DEV->device_id, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMessagePut(usbh_queue_hid_drv, &DEV->device_id, 0);
#endif
				}
#endif
			}
			break;
		case EHCI_PARSER_ROOT_DEQ:
			EHCI2->USBCMD.ASYNC_EN = 0;
			ehci_remove_dev_resource(DEV, EHCI_NO_CHK_ADV,USBH2);
			status = SUCCESS;
			break;
		case EHCI_PARSER_ROOT_HUB_DEQ:
			EHCI2->USBCMD.ASYNC_EN = 0;
			for (i = 0; i < USBH_MAX_PORT; i++) {
				DEV = (USBH_Device_Structure *) &USBH_DEV_2[i];
				ehci_remove_dev_resource(DEV, EHCI_NO_CHK_ADV,USBH2);
			}
			status = SUCCESS;
			break;
		case EHCI_PARSER_PORT_HUB_DEQ:
			ehci_remove_dev_resource(DEV, EHCI_CHK_ADV,USBH2);
			status = SUCCESS;
			break;
	}
	return status;
}

void ehci_rollover_enable_2(void){
	EHCI2					=	(SONIX_EHCI_STRUCTURE*)EHCI_REG_BASE_ADDRESS_2;	
	EHCI2->USBINTR.ROLLOVER_INT_EN		=	1;	
}

void ehci_rollover_disable_2(void){
	EHCI2					=	(SONIX_EHCI_STRUCTURE*)EHCI_REG_BASE_ADDRESS_2;	
	EHCI2->USBINTR.ROLLOVER_INT_EN		=	0;	
}
#endif // End of if defined( CONFIG_DUAL_HOST )

