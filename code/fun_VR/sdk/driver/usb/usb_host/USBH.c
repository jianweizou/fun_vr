// $Source: $				    
// *******************************************************************			  
//	   (C) Copyright 2015 by SONiX Technology Corp.
//
//		       All Rights Reserved
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
 * USBH.c
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#include "sonix_config.h"

#if defined (CONFIG_USBH_FREE_RTOS)
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

#if defined( CONFIG_SN_KEIL_SDK )
#include <stdlib.h>

#if defined( CONFIG_PLATFORM_ST53510 )	
#include "INTC.h"
#endif
#endif 

#if defined( CONFIG_XILINX_SDK )
#include <xil_cache.h>
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xscugic.h"
#include "xparameters.h"
#include "xusbps.h"
#include "xil_exception.h"

extern XScuGic xInterruptController;
#endif

#if defined (CONFIG_PLATFORM_SN7300)
#include "SNC7312.h"
#endif


#include "stdbool.h"
#include <string.h>

#include "USBH.h"

#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined (CONFIG_PLATFORM_ST53510)
uint32_t RF_USBH_Buf_PTR = 0;
#endif
#endif

#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
uint32_t dev_enum_time = 0;
#endif

#if defined (CONFIG_WIFI_SDK)
/* Driver list */
static struct usbh_driver usbh_driver_list = {
	.name = "driver_list_head",
	.idVendor = 0xFFFF,
	.idProduct = 0xFFFF,
	.list = LIST_HEAD_INIT(usbh_driver_list.list)
};
#endif // end of if defined (CONFIG_WIFI_SDK)

// Call Back 
	USBH_PLUG_Struct	plug[USBH_MAX_PORT*2];

#if defined (CONFIG_MODULE_USB_HID_CLASS)
// Call Back for receiving HID INT Data
	USBH_HID_INT_Struct	hid_int[USBH_MAX_PORT*2] = {0};
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
	SemaphoreHandle_t	USBH_SEM_FRAME_LIST_ROLLOVER	= NULL;
	SemaphoreHandle_t	USBH_SEM_ASYNC_ADV						= NULL;
#if !defined (CONFIG_WIFI_SDK)	
	SemaphoreHandle_t	USBH_SEM_WAKEUP_WIFI					= NULL;
#endif	
	SemaphoreHandle_t	USBH_SEM_WAKEUP_AUTO_BKIN			= NULL;
	SemaphoreHandle_t	USBH_SEM_WAKEUP_ERR_HDL				= NULL;
	SemaphoreHandle_t	USBH_SEM_AUTO_BKIN_CNT				= NULL;
	SemaphoreHandle_t	USBH_SEM_PACKET_CNT_MUTEX			= NULL;
	SemaphoreHandle_t	USBH_MUTEX_MSC_TRANSFER				= NULL;

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	QueueHandle_t		USBH_QUEUE_ERR_HDL							= NULL;	
	QueueHandle_t		USBH_QUEUE_ERR_RLT							= NULL;
#endif
	TaskHandle_t		xTASK_HDL_USBH_ENUM							= NULL;
	TaskHandle_t		xTASK_HDL_USB_XFR								= NULL;
	TaskHandle_t		xTASK_HDL_USBH_ERRHDL						= NULL;
	TaskHandle_t		xTASK_HDL_USBH_AUTO_BK_IN				= NULL;
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreId	USBH_SEM_FRAME_LIST_ROLLOVER	= NULL;
	osSemaphoreId	USBH_SEM_ASYNC_ADV						= NULL;
#if !defined (CONFIG_WIFI_SDK)	
	osSemaphoreId	USBH_SEM_WAKEUP_WIFI					= NULL;
#endif	
	osSemaphoreId	USBH_SEM_WAKEUP_AUTO_BKIN			= NULL;
	osSemaphoreId	USBH_SEM_WAKEUP_ERR_HDL				= NULL;
	osSemaphoreId	USBH_SEM_AUTO_BKIN_CNT				= NULL;
	osMutexId	USBH_SEM_PACKET_CNT_MUTEX			= NULL;
	osMutexId			USBH_MUTEX_MSC_TRANSFER						= NULL;

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osMessageQId		USBH_QUEUE_ERR_HDL							= NULL;	
	osMessageQId		USBH_QUEUE_ERR_RLT							= NULL;
#endif
	osThreadId		xTASK_HDL_USBH_ENUM							= NULL;
	osThreadId		xTASK_HDL_USB_XFR								= NULL;
	osThreadId		xTASK_HDL_USBH_ERRHDL						= NULL;
	osThreadId		xTASK_HDL_USBH_AUTO_BK_IN				= NULL;
#endif

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	automount_info_usbh	auto_mount_info_usbh[2];
#endif

#if defined( CONFIG_DUAL_HOST )
#if defined (CONFIG_USBH_FREE_RTOS)
	SemaphoreHandle_t	USBH_SEM_FRAME_LIST_ROLLOVER_2	= NULL;
	SemaphoreHandle_t	USBH_SEM_ASYNC_ADV_2						= NULL;
#if !defined (CONFIG_WIFI_SDK)		
	SemaphoreHandle_t	USBH_SEM_WAKEUP_WIFI_2					= NULL;
#endif
	SemaphoreHandle_t	USBH_SEM_WAKEUP_AUTO_BKIN_2			= NULL;
	SemaphoreHandle_t	USBH_SEM_WAKEUP_ERR_HDL_2				= NULL;
	SemaphoreHandle_t	USBH_SEM_AUTO_BKIN_CNT_2				= NULL;
	SemaphoreHandle_t	USBH_SEM_PACKET_CNT_MUTEX_2			= NULL;
	SemaphoreHandle_t	USBH_MUTEX_MSC_TRANSFER_2				= NULL;
	
	TaskHandle_t		xTASK_HDL_USBH_ENUM_2							= NULL;
	TaskHandle_t		xTASK_HDL_USB_XFR_2								= NULL;
	TaskHandle_t		xTASK_HDL_USBH_ERRHDL_2						= NULL;
	TaskHandle_t		xTASK_HDL_USBH_AUTO_BK_IN_2				= NULL;
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreId	USBH_SEM_FRAME_LIST_ROLLOVER_2	= NULL;
	osSemaphoreId	USBH_SEM_ASYNC_ADV_2						= NULL;
#if !defined (CONFIG_WIFI_SDK)		
	osSemaphoreId	USBH_SEM_WAKEUP_WIFI_2					= NULL;
#endif
	osSemaphoreId	USBH_SEM_WAKEUP_AUTO_BKIN_2			= NULL;
	osSemaphoreId	USBH_SEM_WAKEUP_ERR_HDL_2				= NULL;
	osSemaphoreId	USBH_SEM_AUTO_BKIN_CNT_2				= NULL;
	osMutexId	USBH_SEM_PACKET_CNT_MUTEX_2			= NULL;
	osMutexId			USBH_MUTEX_MSC_TRANSFER_2						= NULL;
	
	osThreadId		xTASK_HDL_USBH_ENUM_2							= NULL;
	osThreadId		xTASK_HDL_USB_XFR_2								= NULL;
	osThreadId		xTASK_HDL_USBH_ERRHDL_2						= NULL;
	osThreadId		xTASK_HDL_USBH_AUTO_BK_IN_2				= NULL;
#endif
#endif // end of #if defined( CONFIG_DUAL_HOST )

void usbh_init(void)
{
	usbh_core_init();	// USB Host Core init
	ehci_hcd_init();	// HCD init
	usbh_intr_enable();
}

void usbh_uninit(void)
{
	ehci_hcd_uninit();	// HCD uninit
	ehci_struct_uninit();
}


void usbh_intr_enable(void)
{
#if defined( CONFIG_SN_GCC_SDK ) && defined (CONFIG_PLATFORM_SN9866X)
	pic_enableInterrupt(USB_HOST_IRQ_NUM);
	pic_registerIrq(USB_HOST_IRQ_NUM, &ehci_isr, PRIORITY_IRQ_USB_HOST);
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined (CONFIG_PLATFORM_SN9868X)
	pic_enableInterrupt(USB_HOST_IRQ_NUM);
	pic_registerIrq(USB_HOST_IRQ_NUM, &ehci_isr, PRIORITY_IRQ_USB_HOST);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	INTC_IrqSetup(INTC_USB2HOST1_IRQ, INTC_LEVEL_TRIG, ehci_isr);
	INTC_IrqEnable(INTC_USB2HOST1_IRQ);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	NVIC_EnableIRQ(USBHOST_IRQn);
#endif
	
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	NVIC_EnableIRQ(USB_HOST0_IRQn);
#endif // End of defined( CONFIG_SN_KEIL_SDK )

#if defined( CONFIG_XILINX_SDK ) && defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
	int Status;
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&xInterruptController);
	
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&xInterruptController, XPAR_XUSBPS_0_INTR,(Xil_ExceptionHandler)ehci_isr,(void *)&USBH_DEV);
	if (Status != XST_SUCCESS) {
		goto out;
	}
	
	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(&xInterruptController, XPAR_XUSBPS_0_INTR);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	return ;
out:
	xil_printf( "FAILURE\r\n" );
#endif	// End of defined( CONFIG_XILINX_SDK )
}

static inline void usbh_reg_write(uint32_t addr, uint8_t start_idx, uint8_t end_idx, uint32_t value) {
	uint32_t *reg_ptr = NULL;
	uint32_t temp_1 = 0, temp_2 = 0;
	uint8_t i = 0;

	for(i = start_idx; i <= end_idx; i++)
		temp_1 |= (1<<i);

	temp_2 = (~temp_1);
	temp_2 |= (value << start_idx);

	reg_ptr = (uint32_t *)addr;

	*reg_ptr |= temp_1;
	*reg_ptr &= temp_2;
}

#if defined (CONFIG_PLATFORM_ST53510) || defined( CONFIG_PLATFORM_SN9868X )
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
void PHY_510setting_initial() {
	uint32_t *reg_ptr = NULL;
	
	// reg_ctrl from e-fuse to register
	*((uint32_t*)0x900000B8) = 0x0FC77FF8;
	
	// PHY1
	// rx squelch level
	reg_ptr = (uint32_t*)0x90050014;
	*reg_ptr |= 0x000C;
	*reg_ptr &= 0xFFF7;
	
	// tx swing control
	reg_ptr = (uint32_t*)0x90050010;
	*reg_ptr |= 0x000F;
	*reg_ptr &= 0xFFFB;
	
	// DISTLVL
	reg_ptr = (uint32_t*)0x90050014;
	*reg_ptr |= 0x0030;
	*reg_ptr &= 0xFFCF;
	
	// CDR_SQU_EN
	reg_ptr = (uint32_t*)0x90050014;
	*reg_ptr &= 0xFEFF;
	
	// TX Gating
	reg_ptr = (uint32_t*)0x90050004;
	*reg_ptr |= 0x02;
}

void PHY_510setting_1_host1() {
	uint32_t *reg_ptr = NULL;
	
	// reg_ctrl from e-fuse to register
	*((uint32_t*)0x900000B8) = 0x0FC77FF8;
	
	// PHY1
	// rx squelch level
	reg_ptr = (uint32_t*)0x90050014;
	*reg_ptr |= 0x000C;
	*reg_ptr &= 0xFFF7;
}

void PHY_510setting_2_host1() {
	uint32_t *reg_ptr = NULL;
	
	// reg_ctrl from e-fuse to register
	*((uint32_t*)0x900000B8) = 0x0FC77FF8;
	
	// PHY1
	// rx squelch level
	reg_ptr = (uint32_t*)0x90050014;
	*reg_ptr |= 0x000C;
	*reg_ptr &= 0xFFFF;
}

void PHY_510setting_1_host2() {
	uint32_t *reg_ptr = NULL;
	
	// reg_ctrl from e-fuse to register
	*((uint32_t*)0x900000B8) = 0x0FC77FF8;
	
	// PHY1
	// rx squelch level
	reg_ptr = (uint32_t*)0x90060014;
	*reg_ptr |= 0x000C;
	*reg_ptr &= 0xFFF7;
}

void PHY_510setting_2_host2() {
	uint32_t *reg_ptr = NULL;
	
	// reg_ctrl from e-fuse to register
	*((uint32_t*)0x900000B8) = 0x0FC77FF8;
	
	// PHY1
	// rx squelch level
	reg_ptr = (uint32_t*)0x90060014;
	*reg_ptr |= 0x000C;
	*reg_ptr &= 0xFFFF;
}

#endif
#endif

#if defined (CONFIG_PLATFORM_SN7320)
void usbh_7320_phy_configuration() {
	
	usbh_reg_write(0x45000110, 3, 3, 1);
	
	usbh_reg_write(0x45000110, 5, 5, 1);
	
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST)
	usbh_reg_write(0x45000110, 8, 9, 1);
	return;
#endif
	
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)
	usbh_reg_write(0x45000110, 8, 9, 2);
	return;
#endif
	
#if defined (CONFIG_USBH_PHY0_DEV_PHY1_HOST)
	usbh_reg_write(0x45000110, 8, 9, 0);
	return;
#endif
}
#endif

/** \defgroup USBH_FUN USBH functions
 * \ingroup USBH_MODULE
 * 
 * @{
 */
 /** \fn void usbh_freertos_init(void)
 * \brief USB Host init	for FreeRTOS
 * 
 */
 /** @}	*/
#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_freertos_init(void)
{
#if defined (CONFIG_PLATFORM_ST53510) || defined( CONFIG_PLATFORM_SN9868X )
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
	PHY_510setting_initial();
#endif
#endif
	
#if defined (CONFIG_PLATFORM_SN7320)
	usbh_7320_phy_configuration();
	NVIC_SetPriority( USB_HOST0_IRQn, 6 );
#endif
	
	USBH_SEM_FRAME_LIST_ROLLOVER	=	xSemaphoreCreateCounting(1, 0);
	USBH_SEM_ASYNC_ADV		=	xSemaphoreCreateCounting(1, 0);
#if !defined (CONFIG_WIFI_SDK)		
	USBH_SEM_WAKEUP_WIFI		=	xSemaphoreCreateCounting(1, 0);
#endif // end of if !defined (CONFIG_WIFI_SDK)	
	USBH_SEM_WAKEUP_ERR_HDL		=	xSemaphoreCreateCounting(1, 0);
	USBH_SEM_WAKEUP_AUTO_BKIN	=	xSemaphoreCreateCounting(1, 0);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	if(USBH_QUEUE_ERR_HDL == NULL)
	USBH_QUEUE_ERR_HDL		=	xQueueCreate(USBH_QUEUE_SIZE, sizeof(USBH_ERR_HDL_STRUCT));
	if(USBH_QUEUE_ERR_RLT == NULL)
	USBH_QUEUE_ERR_RLT		=	xQueueCreate(USBH_QUEUE_SIZE, sizeof(USBH_ERR_RLT_STRUCT));
#endif

	if( USBH_MUTEX_MSC_TRANSFER == NULL )
	{
		USBH_MUTEX_MSC_TRANSFER = xSemaphoreCreateMutex();
	}
	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN	= xSemaphoreCreateBinary();
	auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT	= xSemaphoreCreateBinary();
#endif

#if defined (CONFIG_WIFI_SDK)	
	/* driver list init */
	usbh_driver_list.list_lock = xSemaphoreCreateCounting(1, 1);
#endif // end of if defined (CONFIG_WIFI_SDK)
	
	// Task Init 
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )		
	xTaskCreate(
		usbh_enum_task,
		( const	char * )"USBH_ENUM",
		STACK_SIZE_16K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ENUM
	);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	xTaskCreate(
		usbh_err_hdl_task,
		( const	char *)"USBH_ERROR_HANDLE",
		STACK_SIZE_4K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ERRHDL
	);  
#endif
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
		usbh_enum_task,
		( const	char * )"USBH_ENUM",
		STACK_SIZE_16K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ENUM
	);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	xTaskCreate(
		usbh_err_hdl_task,
		( const	char *)"USBH_ERROR_HANDLE",
		STACK_SIZE_4K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ERRHDL
	);  
#endif
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
		usbh_enum_task,
		( const char * )"USBH_ENUM",
		4096,
		NULL,
		250,
		&xTASK_HDL_USBH_ENUM
	);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	xTaskCreate(
		usbh_err_hdl_task,
		( const char *)"USBH_ERROR_HANDLE",
		1024,
		NULL,
		250,
		&xTASK_HDL_USBH_ERRHDL
	);
#endif 
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
		usbh_enum_task,
		( const char * )"USBH_ENUM",
		512,
		NULL,
		20,
		&xTASK_HDL_USBH_ENUM
	);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	xTaskCreate(
		usbh_err_hdl_task,
		( const char *)"USBH_ERROR_HANDLE",
		128,
		NULL,
		20,
		&xTASK_HDL_USBH_ERRHDL
	);
#endif 
#endif
		
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	xTaskCreate(
		usbh_enum_task,
		( const char * )"USBH_ENUM",
		512,
		NULL,
		3,
		&xTASK_HDL_USBH_ENUM
	);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	xTaskCreate(
		usbh_err_hdl_task,
		( const char *)"USBH_ERROR_HANDLE",
		128,
		NULL,
		3,
		&xTASK_HDL_USBH_ERRHDL
	);
#endif
#endif 

#if defined( CONFIG_XILINX_SDK )	
		xTaskCreate(
			usbh_enum_task,
			( const char * )"USBH_ENUM",
			4096,
			NULL,
			PRIORITY_TASK_DRV_USBH,
			&xTASK_HDL_USBH_ENUM
		);
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
		xTaskCreate(
			usbh_err_hdl_task,
			( const char *)"USBH_ERROR_HANDLE",
			1024,
			NULL,
			PRIORITY_TASK_DRV_USBH,
			&xTASK_HDL_USBH_ERRHDL
		);  
#endif
#endif
	//usbh_auto_bkin_init();	
}

void usbh_freertos_uninit(void)
{
	if(USBH_SEM_FRAME_LIST_ROLLOVER != NULL){
		vSemaphoreDelete(USBH_SEM_FRAME_LIST_ROLLOVER);
		USBH_SEM_FRAME_LIST_ROLLOVER = NULL;
	}
	if(USBH_SEM_ASYNC_ADV != NULL){
		vSemaphoreDelete(USBH_SEM_ASYNC_ADV);
		USBH_SEM_ASYNC_ADV = NULL;
	}
#if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_WIFI != NULL){
		vSemaphoreDelete(USBH_SEM_WAKEUP_WIFI);
		USBH_SEM_WAKEUP_WIFI = NULL;
	}
#endif // end of if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_ERR_HDL != NULL){
		vSemaphoreDelete(USBH_SEM_WAKEUP_ERR_HDL);
		USBH_SEM_WAKEUP_ERR_HDL = NULL;
	}
	if(USBH_SEM_WAKEUP_AUTO_BKIN != NULL){
		vSemaphoreDelete(USBH_SEM_WAKEUP_AUTO_BKIN);
		USBH_SEM_WAKEUP_AUTO_BKIN = NULL;
	}
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	if(USBH_QUEUE_ERR_HDL != NULL){
		vQueueDelete(USBH_QUEUE_ERR_HDL);
		USBH_QUEUE_ERR_HDL = NULL;
	}
	if(USBH_QUEUE_ERR_RLT != NULL){
		vQueueDelete(USBH_QUEUE_ERR_RLT);
		USBH_QUEUE_ERR_RLT = NULL;
	}
#endif

	usbh_uninit();
	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	if (auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN != NULL) {
		vSemaphoreDelete(auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN);
		auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN = NULL;
	}

	if (auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT != NULL) {
		vSemaphoreDelete(auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT);
		auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT = NULL;
	}
#endif

	if(xTASK_HDL_USBH_ERRHDL != NULL){
		vTaskDelete(xTASK_HDL_USBH_ERRHDL);
		xTASK_HDL_USBH_ERRHDL = NULL;
	}

	if(xTASK_HDL_USBH_ENUM != NULL){
		vTaskDelete(xTASK_HDL_USBH_ENUM);
		xTASK_HDL_USBH_ENUM = NULL;
	}
}
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_freertos_init(void)
{
#if defined (CONFIG_PLATFORM_ST53510) || defined( CONFIG_PLATFORM_SN9868X )
#if defined (CONFIG_MODULE_USB_UVC_510REG_PATCH)
	PHY_510setting_initial();
#endif
#endif
	
#if defined (CONFIG_PLATFORM_SN7320)
	usbh_7320_phy_configuration();
	
	NVIC_SetPriority( USB_HOST0_IRQn, 6 );
#endif
	
	osSemaphoreDef(USBH_SEM_FRAME_LIST_ROLLOVER);
	USBH_SEM_FRAME_LIST_ROLLOVER    = osSemaphoreCreate(osSemaphore(USBH_SEM_FRAME_LIST_ROLLOVER), 1);
	osSemaphoreWait(USBH_SEM_FRAME_LIST_ROLLOVER, USBH_10ms);
	
	osSemaphoreDef(USBH_SEM_ASYNC_ADV);
	USBH_SEM_ASYNC_ADV    = osSemaphoreCreate(osSemaphore(USBH_SEM_ASYNC_ADV), 1);
	osSemaphoreWait(USBH_SEM_ASYNC_ADV, USBH_10ms);
				
#if !defined (CONFIG_WIFI_SDK)		
	osSemaphoreDef(USBH_SEM_WAKEUP_WIFI);
	USBH_SEM_WAKEUP_WIFI    = osSemaphoreCreate(osSemaphore(USBH_SEM_WAKEUP_WIFI), 1);
	osSemaphoreWait(USBH_SEM_WAKEUP_WIFI, USBH_10ms);
#endif
	
	osSemaphoreDef(USBH_SEM_WAKEUP_ERR_HDL);
	USBH_SEM_WAKEUP_ERR_HDL    = osSemaphoreCreate(osSemaphore(USBH_SEM_WAKEUP_ERR_HDL), 1);
	osSemaphoreWait(USBH_SEM_WAKEUP_ERR_HDL, USBH_10ms);
	
	osSemaphoreDef(USBH_SEM_WAKEUP_AUTO_BKIN);
	USBH_SEM_WAKEUP_AUTO_BKIN    = osSemaphoreCreate(osSemaphore(USBH_SEM_WAKEUP_AUTO_BKIN), 1);
	osSemaphoreWait(USBH_SEM_WAKEUP_AUTO_BKIN, USBH_10ms);
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	if(USBH_QUEUE_ERR_HDL == NULL) {
	osMessageQDef(USBH_QUEUE_ERR_HDL, USBH_QUEUE_SIZE, USBH_ERR_HDL_STRUCT);
	USBH_QUEUE_ERR_HDL = osMessageCreate(osMessageQ(USBH_QUEUE_ERR_HDL), NULL);
	}
	
	if(USBH_QUEUE_ERR_RLT == NULL) {
	osMessageQDef(USBH_QUEUE_ERR_RLT, USBH_QUEUE_SIZE, USBH_ERR_RLT_STRUCT);
	USBH_QUEUE_ERR_RLT = osMessageCreate(osMessageQ(USBH_QUEUE_ERR_RLT), NULL);
	}
#endif
	
	osMutexDef(USBH_MUTEX_MSC_TRANSFER);
	USBH_MUTEX_MSC_TRANSFER = osMutexCreate(osMutex(USBH_MUTEX_MSC_TRANSFER));

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	osSemaphoreDef(SEM_USBH_PLUG_IN);
	auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN    = osSemaphoreCreate(osSemaphore(SEM_USBH_PLUG_IN), 1);
	osSemaphoreWait(auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN, USBH_10ms);

	osSemaphoreDef(SEM_USBH_PLUG_OUT);
	auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT    = osSemaphoreCreate(osSemaphore(SEM_USBH_PLUG_OUT), 1);
	osSemaphoreWait(auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT, USBH_10ms);
#endif

#if defined (CONFIG_WIFI_SDK)
	osSemaphoreDef(usbh_driver_list.list_lock);
	usbh_driver_list.list_lock    = osSemaphoreCreate(osSemaphore(usbh_driver_list.list_lock), 1);
#endif
	
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	osThreadDef(USBH_ENUM, usbh_enum_task, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_16K);
	xTASK_HDL_USBH_ENUM = osThreadCreate(osThread(USBH_ENUM), NULL);
	if( xTASK_HDL_USBH_ENUM == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osThreadDef(USBH_ERROR_HANDLE, usbh_err_hdl_task, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_4K);
	xTASK_HDL_USBH_ERRHDL = osThreadCreate(osThread(USBH_ERROR_HANDLE), NULL);
	if( xTASK_HDL_USBH_ERRHDL == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(USBH_ENUM, usbh_enum_task, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_16K);
	xTASK_HDL_USBH_ENUM = osThreadCreate(osThread(USBH_ENUM), NULL);
	if( xTASK_HDL_USBH_ENUM == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osThreadDef(USBH_ERROR_HANDLE, usbh_err_hdl_task, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_4K);
	xTASK_HDL_USBH_ERRHDL = osThreadCreate(osThread(USBH_ERROR_HANDLE), NULL);
	if( xTASK_HDL_USBH_ERRHDL == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	osThreadDef(USBH_ENUM, usbh_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_USBH_ENUM = osThreadCreate(osThread(USBH_ENUM), NULL);
	if( xTASK_HDL_USBH_ENUM == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osThreadDef(USBH_ERROR_HANDLE, usbh_err_hdl_task, THREAD_PRIO_USBH_PROCESS, 1, 1024);
	xTASK_HDL_USBH_ERRHDL = osThreadCreate(osThread(USBH_ERROR_HANDLE), NULL);
	if( xTASK_HDL_USBH_ERRHDL == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif 
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	osThreadDef(USBH_ENUM, usbh_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 512);
	xTASK_HDL_USBH_ENUM = osThreadCreate(osThread(USBH_ENUM), NULL);
	if( xTASK_HDL_USBH_ENUM == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osThreadDef(USBH_ERROR_HANDLE, usbh_err_hdl_task, THREAD_PRIO_USBH_PROCESS, 1, 128);
	xTASK_HDL_USBH_ERRHDL = osThreadCreate(osThread(USBH_ERROR_HANDLE), NULL);
	if( xTASK_HDL_USBH_ERRHDL == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif 
#endif
	
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 ) 
	osThreadDef(USBH_ENUM, usbh_enum_task, osPriorityHigh, 1, 512);
	xTASK_HDL_USBH_ENUM = osThreadCreate(osThread(USBH_ENUM), NULL);
	if( xTASK_HDL_USBH_ENUM == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osThreadDef(USBH_ERROR_HANDLE, usbh_err_hdl_task, osPriorityHigh, 1, 128);
	xTASK_HDL_USBH_ERRHDL = osThreadCreate(osThread(USBH_ERROR_HANDLE), NULL);
	if( xTASK_HDL_USBH_ERRHDL == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif 
#endif

#if defined( CONFIG_XILINX_SDK )
	osThreadDef(USBH_ENUM, usbh_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_USBH_ENUM = osThreadCreate(osThread(USBH_ENUM), NULL);
	if( xTASK_HDL_USBH_ENUM == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	osThreadDef(USBH_ERROR_HANDLE, usbh_err_hdl_task, THREAD_PRIO_USBH_PROCESS, 1, 1024);
	xTASK_HDL_USBH_ERRHDL = osThreadCreate(osThread(USBH_ERROR_HANDLE), NULL);
	if( xTASK_HDL_USBH_ERRHDL == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n"); 
#endif
#endif
	//usbh_auto_bkin_init();
}

void usbh_freertos_uninit(void)
{
	if(USBH_SEM_FRAME_LIST_ROLLOVER != NULL){
		osSemaphoreDelete(USBH_SEM_FRAME_LIST_ROLLOVER);
		USBH_SEM_FRAME_LIST_ROLLOVER = NULL;
	}
	if(USBH_SEM_ASYNC_ADV != NULL){
		osSemaphoreDelete(USBH_SEM_ASYNC_ADV);
		USBH_SEM_ASYNC_ADV = NULL;
	}
#if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_WIFI != NULL){
		osSemaphoreDelete(USBH_SEM_WAKEUP_WIFI);
		USBH_SEM_WAKEUP_WIFI = NULL;
	}
#endif // end of if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_ERR_HDL != NULL){
		osSemaphoreDelete(USBH_SEM_WAKEUP_ERR_HDL);
		USBH_SEM_WAKEUP_ERR_HDL = NULL;
	}
	if(USBH_SEM_WAKEUP_AUTO_BKIN != NULL){
		osSemaphoreDelete(USBH_SEM_WAKEUP_AUTO_BKIN);
		USBH_SEM_WAKEUP_AUTO_BKIN = NULL;
	}
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	if(USBH_QUEUE_ERR_HDL != NULL){
		osMutexDelete(USBH_QUEUE_ERR_HDL);
		USBH_QUEUE_ERR_HDL = NULL;
	}
	
	if(USBH_QUEUE_ERR_RLT != NULL){
		osMutexDelete(USBH_QUEUE_ERR_RLT);
		USBH_QUEUE_ERR_RLT = NULL;
	}
#endif

	usbh_uninit();
	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	if (auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN != NULL) {
		osSemaphoreDelete(auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN);
		auto_mount_info_usbh[0].xSEM_USBH_PLUG_IN = NULL;
	}

	if (auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT != NULL) {
		osSemaphoreDelete(auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT);
		auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT = NULL;
	}
#endif

	if(xTASK_HDL_USBH_ERRHDL != NULL){
		osThreadTerminate(xTASK_HDL_USBH_ERRHDL);
		xTASK_HDL_USBH_ERRHDL = NULL;
	}

	if(xTASK_HDL_USBH_ENUM != NULL){
		osThreadTerminate(xTASK_HDL_USBH_ENUM);
		xTASK_HDL_USBH_ENUM = NULL;
	}
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		RF_USBH_Buf_PTR = 0;
#endif
#endif
}

#endif

/** \defgroup WIFI_FUN WIFI functions
 * \ingroup WIFI_MODULE
 * 
 * @{
 */
/** \fn	uint32_t usbh_wifi_init(uint8_t ID)
 * \brief WIFI Class init fucntion
 * 
 * \param ID 1 = primary WIFI device , 2 = secodary WIFI device	, ETC ...
 * 
 * \return Device structure pointer (USBH_Device_Structure)
 */
  /** @} */
#if !defined (CONFIG_WIFI_SDK)
#if defined (CONFIG_MODULE_USB_WIFI_CLASS) 
uint32_t usbh_wifi_init(uint8_t ID)
{
	USBH_Device_Structure *DEV = NULL;
	uint8_t i = 0, j = 0;

	for(i=0;i<USBH_MAX_PORT;i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV[i];
		if(DEV->CLASS_DRV == USBH_WIFI_CLASS){
			j++;
		}
		if(j==ID) {
			return (uint32_t)DEV;
		}
	}
#if defined( CONFIG_DUAL_HOST )	
	for(i=0;i<USBH_MAX_PORT;i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV_2[i];
		if(DEV->CLASS_DRV == USBH_WIFI_CLASS){
			j++;
		}
		if(j==ID) {
			return (uint32_t)DEV;
		}
	}
#endif 		
	return 0;
}
#endif // end of if defined (CONFIG_MODULE_USB_WIFI_CLASS)
#endif // end of if !defined (CONFIG_WIFI_SDK)

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 * 
 * @{
 */
/** \fn	uint32_t usbh_msc_init(uint8_t ID)
 * \brief msc device init function
 * 
 * \param ID 1 = primary MSC device , 2	= secondary MSC device ,	ETC ...
 * 
 * \return device structure pointer (USBH_Device_Structure*)
 */
  /** @} */
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
uint32_t usbh_msc_init(uint8_t ID)
{
	USBH_Device_Structure *DEV = NULL;
	uint8_t i = 0;

	for(i=0; i<USBH_MAX_PORT; i++)
	{
		DEV = (USBH_Device_Structure *)&USBH_DEV[i];
		if(DEV->CLASS_DRV == USBH_MSC_CLASS)
		{
			if(DEV->device_id == ID)
				return (uint32_t)DEV;
		}
	}

#if defined( CONFIG_DUAL_HOST )	
	for(i=0;i<USBH_MAX_PORT;i++)
	{
		DEV = (USBH_Device_Structure *)&USBH_DEV_2[i];
		if(DEV->CLASS_DRV == USBH_MSC_CLASS)
		{
			if(DEV->device_id == ID)
				return (uint32_t)DEV;
		}		
	}	
#endif
	return 0;
}
#endif

/** \defgroup UVC_FUN UVC functions
 * \ingroup UVC_MODULE
 * \n
 * @{
 */
/** \fn	uint32_t usbh_uvc_init(uint8_t ID)
 * \brief UVC Class init fucntion 
 * \n
 * \param ID 1 = primary UVC device , 2	= secodary UVC device ,	ETC ...	
 * \n
 * \return Device structure pointer (USBH_Device_Structure)
 */
  /** @} */
#if defined (CONFIG_MODULE_USB_UVC_CLASS)
uint32_t usbh_uvc_init(uint8_t ID)
{
	USBH_Device_Structure *DEV = NULL;
	uint8_t i = 0;

	for(i=0;i<USBH_MAX_PORT;i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV[i];
		if((DEV->CLASS_DRV == USBH_UVC_ISO_CLASS) || (DEV->CLASS_DRV == USBH_UVC_BULK_CLASS) 
			|| ((DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)))) {
			if(DEV->device_id == ID)
				return (uint32_t)DEV;
		}
	}
#if defined( CONFIG_DUAL_HOST )	
	for(i=0;i<USBH_MAX_PORT;i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV_2[i];
		if((DEV->CLASS_DRV == USBH_UVC_ISO_CLASS) || (DEV->CLASS_DRV == USBH_UVC_BULK_CLASS) 
			|| ((DEV->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)))) {
			if(DEV->device_id == ID)
			return (uint32_t)DEV;
		}
	}	
#endif // End of if defined( CONFIG_DUAL_HOST )	
	return 0;
}

#endif  //End if defined (CONFIG_MODULE_USB_UVC_CLASS)

/** \defgroup HUB_FUN HUB functions
 * \ingroup HUB_MODULE
 * \n
 * @{
 */
/** \fn	uint32_t usbh_hub_init(uint8_t ID)
 * \brief HUB Class init fucntion 
 * \n
 * \param ID 1 = primary HUB device , 2	= secodary HUB device ,	ETC ...	
 * \n
 * \return Device structure pointer (USBH_Device_Structure)
 */
  /** @} */
#if defined (CONFIG_MODULE_USB_HUB_CLASS)
uint32_t usbh_hub_init(uint8_t ID)
{
	USBH_Device_Structure *DEV = NULL;
	uint8_t i = 0;

	for(i=0;i<USBH_MAX_PORT;i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV[i];
		if(DEV->CLASS_DRV == USBH_HUB_CLASS){
			if(DEV->device_id == ID)
			return (uint32_t)DEV;
		}
	}
#if defined( CONFIG_DUAL_HOST )	
	for(i=0;i<USBH_MAX_PORT;i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV_2[i];
		if(DEV->CLASS_DRV == USBH_HUB_CLASS){
			if(DEV->device_id == ID)
			return (uint32_t)DEV;
		}
	}	
#endif // End of if defined( CONFIG_DUAL_HOST )	
	return 0;
}
#endif

#if defined (CONFIG_MODULE_USB_HID_CLASS)
uint32_t usbh_hid_init(uint8_t ID)
{
	USBH_Device_Structure *DEV = NULL;
	uint8_t i = 0;

	for(i = 0; i < USBH_MAX_PORT; i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV[i];
		if(DEV->CLASS_DRV == USBH_HID_CLASS){
			if(DEV->device_id == ID)
				return (uint32_t)DEV;
		}
	}
#if defined( CONFIG_DUAL_HOST )
	for(i = 0; i < USBH_MAX_PORT; i++){
		DEV = (USBH_Device_Structure *)&USBH_DEV_2[i];
		if(DEV->CLASS_DRV == USBH_HID_CLASS){
			if(DEV->device_id == ID)
				return (uint32_t)DEV;
		}
	}	
#endif // End of if defined( CONFIG_DUAL_HOST )	
	return 0;
}
#endif

/** \defgroup WIFI_FUN WIFI functions
 * \ingroup WIFI_MODULE
 * 
 * @{
 */
/** \fn	AUTO_BKIN_STRUCTURE get_auto_bkin_struct()
 * \brief WIFI Class Auto BULK IN , Get	Auto BULK IN Strucutre 
 * 
 * \return AUTO_BKIN_STRUCTURE 
 */
  /** @} */ 
#if defined (CONFIG_MODULE_USB_WIFI_CLASS) && !defined (CONFIG_WIFI_SDK)
AUTO_BKIN_STRUCTURE get_auto_bkin_struct(void)
{
	return AUTO_BK_IN;
}

/** \defgroup WIFI_FUN WIFI functions
 * \ingroup WIFI_MODULE
 * 
 * @{
 */
/** \fn	void set_auto_bkin_processed_cnt(int xfr_cnt)
 * \brief Pointer to next target buffer	
 * 
 * \param xfr_cnt How many packet processed 
 */
  /** @} */ 
void set_auto_bkin_processed_cnt(int xfr_cnt)
{
#if defined (CONFIG_USBH_FREE_RTOS)
	xSemaphoreTake(USBH_SEM_PACKET_CNT_MUTEX, portMAX_DELAY);
	AUTO_BK_IN.xfr_cnt -= xfr_cnt;
	xSemaphoreGive(USBH_SEM_PACKET_CNT_MUTEX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osMutexWait(USBH_SEM_PACKET_CNT_MUTEX, osWaitForever);
	AUTO_BK_IN.xfr_cnt -= xfr_cnt;
	osMutexRelease(USBH_SEM_PACKET_CNT_MUTEX);
#endif

	if((AUTO_BK_IN.start_addr +	(PACKET_SIZE*xfr_cnt)) > (AUTO_BK_IN.base_addr + (PACKET_SIZE*(PACKET_MAX_CNT-1)))){
		AUTO_BK_IN.start_addr =	AUTO_BK_IN.base_addr + (AUTO_BK_IN.start_addr +	PACKET_SIZE*xfr_cnt) - AUTO_BK_IN.boundary_addr;
	}else{
		AUTO_BK_IN.start_addr += (PACKET_SIZE*xfr_cnt);
	}

	AUTO_BK_IN.buff_full = 0;
}
#endif


#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_enum_task(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_enum_task(void const *pvParameters)
#endif
{
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
	dev_enum_time = USBH_500ms;
#endif
	usbh_init();
	while(1) {
		usbh_root_enum();
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(dev_enum_time);
#endif	
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(dev_enum_time);
#endif
#else
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_10ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_10ms);
#endif		
#endif
	}
}

uint32_t usbh_wifi_uninit_flag = 0;
uint32_t usbh_wifi_uninit_ready = 0;
uint32_t usbh_wifi_init_ready = 0;
#if defined (CONFIG_MODULE_USB_WIFI_CLASS) && !defined (CONFIG_WIFI_SDK)
#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_auto_bkin_task(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_auto_bkin_task(void const *pvParameters)
#endif
{
	uint8_t STATUS				=	SUCCESS;
	uint8_t *PTR				=	NULL;
	uint8_t PRINT_MSG			=	0;
	uint32_t bkin_addr			= 0;
	uint32_t *BULK_DATA_BUFF		= NULL;
	USBH_Device_Structure *DEV		= NULL;
	USBH_BK_XFR_REQ_Struct BK_REQ;	

#if defined (CONFIG_USBH_FREE_RTOS)
	USBH_SEM_AUTO_BKIN_CNT			=	xSemaphoreCreateCounting(1, 0);
	USBH_SEM_PACKET_CNT_MUTEX		=	xSemaphoreCreateMutex();
	
#if defined( CONFIG_SN_GCC_SDK )
	do{
		PTR = pvPortMalloc(PACKET_MAX_CNT*PACKET_SIZE+1024, GFP_DMA,MODULE_DRI_USBH);
	}while(PTR == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined ( CONFIG_XILINX_SDK )
	do{
		PTR = pvPortMalloc(PACKET_MAX_CNT*PACKET_SIZE+1024);
	}while(PTR == NULL);
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreDef(USBH_SEM_AUTO_BKIN_CNT);
	USBH_SEM_AUTO_BKIN_CNT    = osSemaphoreCreate(osSemaphore(USBH_SEM_AUTO_BKIN_CNT), 1);
	osSemaphoreWait(USBH_SEM_AUTO_BKIN_CNT, USBH_10ms);
	
	osMutexDef(USBH_SEM_PACKET_CNT_MUTEX);
	USBH_SEM_PACKET_CNT_MUTEX = osMutexCreate(osMutex(USBH_SEM_PACKET_CNT_MUTEX));
	
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		if(RF_USBH_Buf_PTR == 0)
			RF_USBH_Buf_PTR = 0x900000+EHCI_STRUCT_SIZE;
		PTR = (uint8_t *)RF_USBH_Buf_PTR;
		RF_USBH_Buf_PTR += PACKET_MAX_CNT*PACKET_SIZE+1024;
#endif
#endif
	PTR				=	(uint8_t*)((((uint32_t)PTR)&0xFFFFFC00)+0x400);
	bkin_addr	=	(uint32_t)PTR;

	AUTO_BK_IN.base_addr			= (uint32_t)PTR;
	AUTO_BK_IN.boundary_addr	= AUTO_BK_IN.base_addr + (PACKET_SIZE*PACKET_MAX_CNT);
	AUTO_BK_IN.start_addr			= (uint32_t)PTR;
	AUTO_BK_IN.xfr_cnt				= 0;
	AUTO_BK_IN.buff_full			= 0;

	while(1){
		memset((uint8_t*)AUTO_BK_IN.base_addr, 0, PACKET_SIZE*PACKET_MAX_CNT);
		memset(&BK_REQ,	0, sizeof(BK_REQ));
		
#if defined (CONFIG_USBH_FREE_RTOS)
		xSemaphoreTake(USBH_SEM_WAKEUP_AUTO_BKIN_2, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osSemaphoreWait(USBH_SEM_WAKEUP_AUTO_BKIN_2, osWaitForever);
#endif
		
		USBH_DBG("AUTO BK IN TASK START!!");

		usbh_wifi_init_ready = 1;

		do{
			if(AUTO_BK_IN.buff_full != 1){
				BULK_DATA_BUFF	=	(uint32_t*)bkin_addr;
				BK_REQ.XfrType	=	USBH_BK_IN_TYPE;
				BK_REQ.NUM			=	1;
				BK_REQ.SIZE			=	PACKET_SIZE;
				BK_REQ.pBUFF		=	(uint32_t*)BULK_DATA_BUFF;
#if defined (CONFIG_WIFI_SDK)
				DEV = (USBH_Device_Structure*)pvParameters;
#else
				DEV = (USBH_Device_Structure*)usbh_wifi_init(1);
#endif 				
				if(DEV == NULL) break;
				STATUS = usbh_bk_xfr((USBH_Device_Structure*)DEV, (USBH_BK_XFR_REQ_Struct*)&BK_REQ);
				if(usbh_wifi_uninit_flag) break;
				if(STATUS == SUCCESS){
					if((bkin_addr+PACKET_SIZE) > (AUTO_BK_IN.base_addr+PACKET_SIZE*(PACKET_MAX_CNT-1))){
						bkin_addr = AUTO_BK_IN.base_addr;
					}else{
						bkin_addr += PACKET_SIZE;
					}
					
#if defined (CONFIG_USBH_FREE_RTOS)
					xSemaphoreTake(USBH_SEM_PACKET_CNT_MUTEX, portMAX_DELAY);
					AUTO_BK_IN.xfr_cnt++;
					xSemaphoreGive(USBH_SEM_PACKET_CNT_MUTEX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMutexWait(USBH_SEM_PACKET_CNT_MUTEX, osWaitForever);
					AUTO_BK_IN.xfr_cnt++;
					osMutexRelease(USBH_SEM_PACKET_CNT_MUTEX);
#endif
					if(AUTO_BK_IN.xfr_cnt == PACKET_MAX_CNT){
						AUTO_BK_IN.buff_full = 1;
						PRINT_MSG = 0;
					}
					
#if defined (CONFIG_USBH_FREE_RTOS)
					xSemaphoreGive(USBH_SEM_AUTO_BKIN_CNT);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osSemaphoreRelease(USBH_SEM_AUTO_BKIN_CNT);
#endif		
				}else{
					USBH_DBG("BK IN FAIL, GOING TO ERROR HANDLING");
					break;
				}
			}else{
				if(PRINT_MSG ==	0){
					USBH_DBG(" USBH>>RING BUFFER FULL");
#if defined (CONFIG_USBH_FREE_RTOS)
					xSemaphoreGive(USBH_SEM_AUTO_BKIN_CNT);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osSemaphoreRelease(USBH_SEM_AUTO_BKIN_CNT);
#endif	
					PRINT_MSG =	1;
				}
				
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_10ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_10ms);
#endif
			}
		}while(!usbh_wifi_uninit_flag);// end of while(1)
		usbh_wifi_uninit_ready = 1;
	}//end of while(1)	
}

#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_auto_bkin_init(void){

	if (xTASK_HDL_USBH_AUTO_BK_IN != NULL)
		return;

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	xTaskCreate(
		usbh_auto_bkin_task,
		( const	char *)"USBH_AUTO_BKIN",
		STACK_SIZE_4K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_AUTO_BK_IN
	);	
#endif 

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
		usbh_auto_bkin_task,
		( const	char *)"USBH_AUTO_BKIN",
		STACK_SIZE_4K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_AUTO_BK_IN
	);	
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
		usbh_auto_bkin_task,
		( const char *)"USBH_AUTO_BKIN",
		1024,
		NULL,
		250,
		&xTASK_HDL_USBH_AUTO_BK_IN
	);
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
		usbh_auto_bkin_task,
		( const char *)"USBH_AUTO_BKIN",
		512,
		NULL,
		2,
		&xTASK_HDL_USBH_AUTO_BK_IN
	);	
#endif
		
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 ) 
	xTaskCreate(
		usbh_auto_bkin_task,
		( const char *)"USBH_AUTO_BKIN",
		512,
		NULL,
		3,
		&xTASK_HDL_USBH_AUTO_BK_IN
	);	
#endif

#if defined( CONFIG_XILINX_SDK )	
	xTaskCreate(
		usbh_auto_bkin_task,
		( const char *)"USBH_AUTO_BKIN",
		1024,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_AUTO_BK_IN
	);
#endif
}

void usbh_auto_bkin_uninit(void){
	USBH_Device_Structure	*DEV = NULL;
	uint32_t i,j;
	
	usbh_wifi_uninit_flag = 1;
	DEV = (USBH_Device_Structure*)usbh_wifi_init(1);		
	
	if (DEV != NULL) {	
		for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
			if ((USBH_DEV[i].EP[j].XfrType == USBH_BK_IN_TYPE) || (USBH_DEV[i].EP[j].XfrType == USBH_BK_OUT_TYPE)) {
				if(xSemaphoreTake(USBH_DEV[i].EP[j].Mutex, 0) == pdFALSE){
					xSemaphoreGive(USBH_DEV[i].EP[j].SEM);							
					ehci_stop_xfr((void *)&USBH_DEV[i].EP[j]);	
					USBH_DEV[i].EP[j].DataTog = 0;
				}
				xSemaphoreGive(USBH_DEV[i].EP[j].Mutex);				
			}
		}
	}
	if(xTASK_HDL_USBH_AUTO_BK_IN !=	NULL){
		if(usbh_wifi_init_ready) {
			while(!usbh_wifi_uninit_ready) {
				vTaskDelay(USBH_5ms);
			}
		}
		vTaskDelete(xTASK_HDL_USBH_AUTO_BK_IN);
		xTASK_HDL_USBH_AUTO_BK_IN = NULL;
		vSemaphoreDelete(USBH_SEM_AUTO_BKIN_CNT);
		USBH_SEM_AUTO_BKIN_CNT = NULL;
		vSemaphoreDelete(USBH_SEM_PACKET_CNT_MUTEX);		
		USBH_SEM_PACKET_CNT_MUTEX = NULL;
	}
	usbh_wifi_uninit_flag  = 0;
	usbh_wifi_uninit_ready = 0;
	usbh_wifi_init_ready = 0;
}
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_auto_bkin_init(void){

if (xTASK_HDL_USBH_AUTO_BK_IN != NULL)
		return;

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	osThreadDef(USBH_AUTO_BKIN, usbh_auto_bkin_task, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_4K);
	xTASK_HDL_USBH_AUTO_BK_IN = osThreadCreate(osThread(USBH_AUTO_BKIN), NULL);
	if( xTASK_HDL_USBH_AUTO_BK_IN == NULL )
		printf("Create xTASK_HDL_USBH_AUTO_BK_IN fail\n");
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(USBH_AUTO_BKIN, usbh_auto_bkin_task, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_4K);
	xTASK_HDL_USBH_AUTO_BK_IN = osThreadCreate(osThread(USBH_AUTO_BKIN), NULL);
	if( xTASK_HDL_USBH_AUTO_BK_IN == NULL )
		printf("Create xTASK_HDL_USBH_AUTO_BK_IN fail\n");
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	osThreadDef(USBH_AUTO_BKIN, usbh_auto_bkin_task, THREAD_PRIO_USBH_PROCESS, 1, 1024);
	xTASK_HDL_USBH_AUTO_BK_IN = osThreadCreate(osThread(USBH_AUTO_BKIN), NULL);
	if( xTASK_HDL_USBH_AUTO_BK_IN == NULL )
		printf("Create xTASK_HDL_USBH_AUTO_BK_IN fail\n");
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	osThreadDef(USBH_AUTO_BKIN, usbh_auto_bkin_task, THREAD_PRIO_USBH_PROCESS, 1, 512);
	xTASK_HDL_USBH_AUTO_BK_IN = osThreadCreate(osThread(USBH_AUTO_BKIN), NULL);
	if( xTASK_HDL_USBH_AUTO_BK_IN == NULL )
		printf("Create xTASK_HDL_USBH_AUTO_BK_IN fail\n");
#endif
	
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 ) 
	osThreadDef(USBH_AUTO_BKIN, usbh_auto_bkin_task, THREAD_PRIO_USBH_PROCESS, 1, 512);
	xTASK_HDL_USBH_AUTO_BK_IN = osThreadCreate(osThread(USBH_AUTO_BKIN), NULL);
	if( xTASK_HDL_USBH_AUTO_BK_IN == NULL )
		printf("Create xTASK_HDL_USBH_AUTO_BK_IN fail\n");
#endif

#if defined( CONFIG_XILINX_SDK )
	osThreadDef(USBH_AUTO_BKIN, usbh_auto_bkin_task, THREAD_PRIO_USBH_PROCESS, 1, 1024);
	xTASK_HDL_USBH_AUTO_BK_IN = osThreadCreate(osThread(USBH_AUTO_BKIN), NULL);
	if( xTASK_HDL_USBH_AUTO_BK_IN == NULL )
		printf("Create xTASK_HDL_USBH_AUTO_BK_IN fail\n");
#endif
}

void usbh_auto_bkin_uninit(void){
	USBH_Device_Structure	*DEV = NULL;
	uint32_t i,j;
	
	usbh_wifi_uninit_flag = 1;
	DEV = (USBH_Device_Structure*)usbh_wifi_init(1);		
	
	if (DEV != NULL) {	
		for (j = 0; j < MAX_QH_PER_DEVICE; j++) {
			if ((USBH_DEV[i].EP[j].XfrType == USBH_BK_IN_TYPE) || (USBH_DEV[i].EP[j].XfrType == USBH_BK_OUT_TYPE)) {
				if(osMutexWait(USBH_DEV[i].EP[j].Mutex, 0) != osOK){
					osMutexRelease(USBH_DEV[i].EP[j].SEM);							
					ehci_stop_xfr((void *)&USBH_DEV[i].EP[j]);	
					USBH_DEV[i].EP[j].DataTog = 0;
				}
				osMutexRelease(USBH_DEV[i].EP[j].Mutex);				
			}
		}
	}
	if(xTASK_HDL_USBH_AUTO_BK_IN !=	NULL){
		if(usbh_wifi_init_ready) {
			while(!usbh_wifi_uninit_ready) {
				osDelay(USBH_5ms);
			}
		}

		osThreadTerminate(xTASK_HDL_USBH_AUTO_BK_IN);
		xTASK_HDL_USBH_AUTO_BK_IN = NULL;
		osSemaphoreDelete(USBH_SEM_AUTO_BKIN_CNT);
		USBH_SEM_AUTO_BKIN_CNT = NULL;
		osMutexDelete(USBH_SEM_PACKET_CNT_MUTEX);		
		USBH_SEM_PACKET_CNT_MUTEX = NULL;
	}
	usbh_wifi_uninit_flag  = 0;
	usbh_wifi_uninit_ready = 0;
	usbh_wifi_init_ready = 0;
}
#endif
#endif

void usbh_plug_cb_reg(uint32_t CLASS_TYPE, void (*in_callback)(void), void (*out_callback)(void))
{
	uint32_t i = 0;

	// find exist cb  
	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if(plug[i].class_type == CLASS_TYPE){
			plug[i].CB.in		= in_callback;
			plug[i].CB.out	= out_callback;				
			return;
		}
	}

	// add new cb 
	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if (plug[i].class_type == 0) {
			plug[i].class_type	= CLASS_TYPE;
			plug[i].CB.in				= in_callback;
			plug[i].CB.out			= out_callback;			
			return;
		}
	}
}

uint8_t usbh_plug_in_cb(uint32_t CLASS_TYPE )
{
	uint32_t i = 0;
	void (*func)(void) = NULL;
	
	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if (plug[i].class_type == CLASS_TYPE) {	
			//func = (void(*))plug[i].CB.in;
			func = (void(*)(void))plug[i].CB.in;	// Fix Warning 		
			if(func != NULL) {
			func();
			return SUCCESS;
			} else {
				return FAIL;
			}
		}
	}
	return FAIL;	
}

uint8_t usbh_plug_out_cb(uint32_t CLASS_TYPE)
{
	uint32_t i = 0;
	void (*func)(void) = NULL;
	
	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if (plug[i].class_type == CLASS_TYPE) {
			//func = (void(*))plug[i].CB.out;			
			func = (void(*)(void))plug[i].CB.out;	// Fix Warning 
			if(func != NULL) {
			func();	
			return SUCCESS;
			} else {
				return FAIL;
			}
		}
	}
	return FAIL;
}

#if defined (CONFIG_MODULE_USB_HID_CLASS)
uint8_t usbh_HID_INT_IN_cb(uint32_t CLASS_TYPE, uint8_t dev_id, uint8_t *INT_DATA_BUFF)
{
	uint32_t i = 0;
	void (*func)(uint32_t, uint8_t*);

	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if ((hid_int[i].class_type == CLASS_TYPE) && (CLASS_TYPE == USBH_HID_CLASS)) {	
			//func = (void(*))hid_int[i].CB.in;
			func = (void(*)(uint32_t,uint8_t *))hid_int[i].CB.in;	// Fix Warning 		
			func(dev_id, INT_DATA_BUFF);
			return SUCCESS;
		}
	}
	return FAIL;	
}

void usbh_HID_INT_cb_reg(uint32_t CLASS_TYPE, void (*callback)(uint32_t, uint8_t*))
{
	uint32_t i = 0;

	// find exist cb  
	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if((hid_int[i].class_type == CLASS_TYPE) && (CLASS_TYPE == USBH_HID_CLASS)){
			hid_int[i].CB.in			= callback;		
			break;
		}
	}

	// add new cb 
	for (i = 0; i < USBH_MAX_PORT*2; i++) {
		if (hid_int[i].class_type == 0) {
			hid_int[i].class_type = CLASS_TYPE;
			hid_int[i].CB.in			= callback;		
			break;
		}
	}
}

void usbh_HID_INT_cb_UNreg(uint32_t CLASS_TYPE, uint8_t dev_id)
{
		if((hid_int[dev_id-1].class_type == CLASS_TYPE) && (CLASS_TYPE == USBH_HID_CLASS)){
			hid_int[dev_id-1].class_type	= 0;
			hid_int[dev_id-1].CB.in				= 0;		
		}
}
#endif

uint8_t pre_periodic_en_status = 0;
uint8_t usbh_root_suspend() {

	uint8_t status = FAIL;

	if (EHCI->PORTSC[0].PORTENABLED == 1) {
		if (EHCI->PORTSC[0].SUSPEND == 1) {
			status = SUCCESS;
		} else {
			if (EHCI->USBCMD.ASYNC_EN == 1) {
				EHCI->USBCMD.ASYNC_EN = 0;
			}

			pre_periodic_en_status = EHCI->USBCMD.PERIODIC_EN;
			if (EHCI->USBCMD.PERIODIC_EN == 1) {
				EHCI->USBCMD.PERIODIC_EN = 0;
			}

			if (EHCI->USBCMD.RUN_NSTOP == 1) {
				EHCI->USBCMD.RUN_NSTOP = 0;
			}
//			while (EHCI->USBSTS.HC_HALTED == 0)
//				;
			while (EHCI->USBCMD.RUN_NSTOP == 1)
				;

			EHCI->PORTSC[0].SUSPEND = 1;
#if defined (CONFIG_USBH_FREE_RTOS)
			vTaskDelay(USBH_20ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osDelay(USBH_20ms);
#endif
			
			status = SUCCESS;
		}
	}

	return status;
}

uint8_t usbh_root_resume() {

	uint8_t status = FAIL;

	if ((EHCI->PORTSC[0].PORTENABLED == 1)
			&& (EHCI->PORTSC[0].SUSPEND == 1)) {
		if (EHCI->PORTSC[0].FORC_PO_RESU == 1) {
			status = FAIL;
		} else {
			if (EHCI->USBCMD.RUN_NSTOP == 0) {
				EHCI->USBCMD.RUN_NSTOP = 1;
			}
//			while (EHCI->USBSTS.HC_HALTED == 1)
//				;
			while (EHCI->USBCMD.RUN_NSTOP == 0)
				;

			EHCI->PORTSC[0].FORC_PO_RESU = 1;
#if defined (CONFIG_USBH_FREE_RTOS)
			vTaskDelay(USBH_30ms);
			EHCI->PORTSC[0].FORC_PO_RESU = 0;
			vTaskDelay(USBH_20ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osDelay(USBH_30ms);
			EHCI->PORTSC[0].FORC_PO_RESU = 0;
			osDelay(USBH_20ms);
#endif
			
			while(EHCI->PORTSC[0].SUSPEND == 1) {
				EHCI->PORTSC[0].FORC_PO_RESU = 1;
#if defined (CONFIG_USBH_FREE_RTOS)
				vTaskDelay(USBH_30ms);
				EHCI->PORTSC[0].FORC_PO_RESU = 0;
				vTaskDelay(USBH_20ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				osDelay(USBH_30ms);
				EHCI->PORTSC[0].FORC_PO_RESU = 0;
				osDelay(USBH_20ms);
#endif

				if (EHCI->PORTSC[0].SUSPEND == 1) {
					EHCI->USBCMD.RUN_NSTOP = 0;
#if defined (CONFIG_USBH_FREE_RTOS)
					vTaskDelay(USBH_30ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osDelay(USBH_30ms);
#endif
					return status;
				}
			}

			EHCI->USBCMD.ASYNC_EN = 1;
			EHCI->USBCMD.PERIODIC_EN = pre_periodic_en_status;

			status = SUCCESS;
		}
	}

	return status;
}

#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
uint32_t ulUSBH_GetUVCFrameBufferSize(USBH_UVC_FRAME_FMT fmt, uint32_t ulWidth, uint32_t ulHeight) {
	
	uint32_t size = 0;
	uint32_t iso_size = 0, bk_size = 0;
	bk_size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;
	
	switch(fmt) {
		case USBH_UVC_NONE:
			return 0;
		case USBH_UVC_YUV:
			iso_size = ulWidth*ulHeight*2*3;
			size = ((iso_size > bk_size) ? iso_size : bk_size);
			break;
		case USBH_UVC_MJPEG:
			iso_size = MJPEGFrameSize;
			size = ((iso_size > bk_size) ? iso_size : bk_size);
			break;
		case USBH_UVC_H264:
			iso_size = H264FrameSize;
			size = ((iso_size > bk_size) ? iso_size : bk_size);
			break;
	}

	return size;
}

uint32_t ulUSBH_GetBufSize(bool bIsEnableWIFI, bool bIsEnableDualHost, uint8_t stream_num, USBH_UVC_FRAME_INFO uvc_frame_info) {
	
	uint8_t i = 0;
	uint32_t size = EHCI_STRUCT_SIZE;
	
	if(bIsEnableWIFI) {
		size += PACKET_MAX_CNT*PACKET_SIZE+1024;
	}
	
	if(bIsEnableDualHost) {
		size += EHCI_STRUCT_SIZE;
	}
	
	for(i = 0; i < stream_num; i++) {
		size += ulUSBH_GetUVCFrameBufferSize(uvc_frame_info.fmt[i], uvc_frame_info.ulWidth[i], uvc_frame_info.ulHeight[i]);
	}
	
	return size;
}

void USBH_BufInit(uint32_t ulBufferStrAddr) {
	RF_USBH_Buf_PTR = ulBufferStrAddr;
}
#endif
#endif

#if defined( CONFIG_DUAL_HOST )	
void usbh_init_2(void)
{
	usbh_core_init_2();	// USB Host Core init
	ehci_hcd_init_2();	// HCD init
	usbh_intr_enable_2();
}

void usbh_uninit_2(void)
{
	ehci_hcd_uninit_2();	// HCD uninit
	ehci_struct_uninit_2();
}
	
void usbh_intr_enable_2(void)
{
#if defined( CONFIG_SN_GCC_SDK ) && defined (CONFIG_PLATFORM_SN9866X)
	pic_enableInterrupt(USB_HOST_IRQ_NUM);
	pic_registerIrq(USB_HOST_IRQ_NUM, &ehci_isr, PRIORITY_IRQ_USB_HOST);
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined (CONFIG_PLATFORM_SN9868X)
	pic_enableInterrupt(USB_HOST_IRQ_NUM);
	pic_registerIrq(USB_HOST_IRQ_NUM, &ehci_isr, PRIORITY_IRQ_USB_HOST);
#endif  //End of defined( CONFIG_SN_GCC_SDK )

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	uint32_t *REG_GLB_USB_PHY2 = NULL;	
	
	INTC_IrqSetup(INTC_USB2HOST2_IRQ, INTC_LEVEL_TRIG, ehci_isr_2);
	INTC_IrqEnable(INTC_USB2HOST2_IRQ);	
	
	REG_GLB_USB_PHY2 = (uint32_t *)0x9006001C;
	*REG_GLB_USB_PHY2 = 0x7A;
	
	REG_GLB_USB_PHY2 = (uint32_t *)0x90060000;
	*REG_GLB_USB_PHY2 |= 0x80000000;	
	//REG_GLB_USB_PHY2 = (uint32_t *)0x9006001C;
	//*REG_GLB_USB_PHY2 = 0x7A;	
	
	// Write clean phy2 connect change  
	EHCI2->PORTSC[0].CNNT_ST_CHG = 1;
	//REG_GLB_USB_PHY2 = (uint32_t *)EHCI_REG_BASE_ADDRESS_2+0x54;
	//*REG_GLB_USB_PHY2 = 0x00000001;
	
#endif // End of defined( CONFIG_SN_KEIL_SDK )
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	NVIC_EnableIRQ(USB_HOST1_IRQn);
#endif // End of defined( CONFIG_SN_KEIL_SDK )
#if defined( CONFIG_XILINX_SDK ) && defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
	int Status;
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&xInterruptController);
	
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&xInterruptController, XPAR_XUSBPS_0_INTR,(Xil_ExceptionHandler)ehci_isr,(void *)&USBH_DEV);
	if (Status != XST_SUCCESS) {
		goto out;
	}
	
	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(&xInterruptController, XPAR_XUSBPS_0_INTR);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	return ;
out:
	xil_printf( "FAILURE\r\n" );
#endif	// End of defined( CONFIG_XILINX_SDK )
}

#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_freertos_init_2(void)
{
#if defined (CONFIG_PLATFORM_SN7320)
	usbh_7320_phy_configuration();
	
	NVIC_SetPriority( USB_HOST1_IRQn, 6 );
#endif
	
	// Semaphore Init
	USBH_SEM_FRAME_LIST_ROLLOVER_2	=	xSemaphoreCreateCounting(1, 0);	
	USBH_SEM_ASYNC_ADV_2						=	xSemaphoreCreateCounting(1, 0);
#if !defined (CONFIG_WIFI_SDK)	
	USBH_SEM_WAKEUP_WIFI_2					=	xSemaphoreCreateCounting(1, 0);
#endif // end of if !defined (CONFIG_WIFI_SDK)	
	USBH_SEM_WAKEUP_ERR_HDL_2				=	xSemaphoreCreateCounting(1, 0);
	USBH_SEM_WAKEUP_AUTO_BKIN_2			=	xSemaphoreCreateCounting(1, 0);
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	if(USBH_QUEUE_ERR_HDL == NULL)
		USBH_QUEUE_ERR_HDL		=	xQueueCreate(USBH_QUEUE_SIZE, sizeof(USBH_ERR_HDL_STRUCT));
	if(USBH_QUEUE_ERR_RLT == NULL)
		USBH_QUEUE_ERR_RLT		=	xQueueCreate(USBH_QUEUE_SIZE, sizeof(USBH_ERR_RLT_STRUCT));
#endif

	if( USBH_MUTEX_MSC_TRANSFER_2 == NULL )
		USBH_MUTEX_MSC_TRANSFER_2 = xSemaphoreCreateMutex();
	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN	= xSemaphoreCreateBinary();
	auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT	= xSemaphoreCreateBinary();
#endif
	
#if defined (CONFIG_WIFI_SDK)	
	/* driver list init */
	usbh_driver_list.list_lock = xSemaphoreCreateCounting(1, 1);
#endif // end of if defined (CONFIG_WIFI_SDK)
		
	// Task Init 

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	xTaskCreate(
		usbh_enum_task_2,
		( const	char * )"USBH_ENUM_2",
		STACK_SIZE_16K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ENUM_2
	);

	xTaskCreate(
		usbh_err_hdl_task_2,
		( const	char *)"USBH_ERROR_HANDLE_2",
		STACK_SIZE_4K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ERRHDL_2
	);  
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
		usbh_enum_task_2,
		( const	char * )"USBH_ENUM_2",
		STACK_SIZE_16K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ENUM_2
	);

	xTaskCreate(
		usbh_err_hdl_task_2,
		( const	char *)"USBH_ERROR_HANDLE_2",
		STACK_SIZE_4K,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ERRHDL_2
	);  
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
		usbh_enum_task_2,
		( const char * )"USBH_ENUM_2",
		4096,
		NULL,
		250,
		&xTASK_HDL_USBH_ENUM_2
	);

	xTaskCreate(
		usbh_err_hdl_task_2,
		( const char *)"USBH_ERROR_HANDLE_2",
		1024,
		NULL,
		250,
		&xTASK_HDL_USBH_ERRHDL_2
	);
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
		usbh_enum_task_2,
		( const char * )"USBH_ENUM_2",
		512,
		NULL,
		20,
		&xTASK_HDL_USBH_ENUM_2
	);

	xTaskCreate(
		usbh_err_hdl_task_2,
		( const char *)"USBH_ERROR_HANDLE_2",
		128,
		NULL,
		20,
		&xTASK_HDL_USBH_ERRHDL_2
	);
#endif 
		
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	xTaskCreate(
		usbh_enum_task_2,
		( const char * )"USBH_ENUM_2",
		512,
		NULL,
		3,
		&xTASK_HDL_USBH_ENUM_2
	);

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	xTaskCreate(
		usbh_err_hdl_task_2,
		( const char *)"USBH_ERROR_HANDLE_2",
		128,
		NULL,
		3,
		&xTASK_HDL_USBH_ERRHDL_2
	);
#endif
#endif 

#if defined( CONFIG_XILINX_SDK )	
	xTaskCreate(
		usbh_enum_task_2,
		( const char * )"USBH_ENUM",
		4096,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ENUM_2
	);
	
	xTaskCreate(
		usbh_err_hdl_task_2,
		( const char *)"USBH_ERROR_HANDLE",
		1024,
		NULL,
		PRIORITY_TASK_DRV_USBH,
		&xTASK_HDL_USBH_ERRHDL_2
	);  
#endif		
	//usbh_auto_bkin_init();	
}

void usbh_freertos_uninit_2(void)
{
	if(USBH_SEM_FRAME_LIST_ROLLOVER_2 != NULL){
		vSemaphoreDelete(USBH_SEM_FRAME_LIST_ROLLOVER_2);
		USBH_SEM_FRAME_LIST_ROLLOVER_2 = NULL;
	}
	if(USBH_SEM_ASYNC_ADV_2 != NULL){
		vSemaphoreDelete(USBH_SEM_ASYNC_ADV_2);
		USBH_SEM_ASYNC_ADV_2 = NULL;
	}
#if !defined (CONFIG_WIFI_SDK)		
	if(USBH_SEM_WAKEUP_WIFI_2 != NULL){
		vSemaphoreDelete(USBH_SEM_WAKEUP_WIFI_2);
		USBH_SEM_WAKEUP_WIFI_2 = NULL;
	}
#endif // end of if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_ERR_HDL_2 != NULL){
		vSemaphoreDelete(USBH_SEM_WAKEUP_ERR_HDL_2);
		USBH_SEM_WAKEUP_ERR_HDL_2 = NULL;
	}
	if(USBH_SEM_WAKEUP_AUTO_BKIN_2 != NULL){
		vSemaphoreDelete(USBH_SEM_WAKEUP_AUTO_BKIN_2);
		USBH_SEM_WAKEUP_AUTO_BKIN_2 = NULL;
	}

	usbh_uninit_2();

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	if (auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN != NULL) {
		vSemaphoreDelete(auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN);
		auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN = NULL;
	}

	if (auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT != NULL) {
		vSemaphoreDelete(auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT);
		auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT = NULL;
	}
#endif
	
	if(xTASK_HDL_USBH_ERRHDL_2 != NULL){
		vTaskDelete(xTASK_HDL_USBH_ERRHDL_2);
		xTASK_HDL_USBH_ERRHDL_2 = NULL;
	}
		
	if(xTASK_HDL_USBH_ENUM_2 != NULL){
		vTaskDelete(xTASK_HDL_USBH_ENUM_2);
		xTASK_HDL_USBH_ENUM_2 = NULL;
	}
}
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_freertos_init_2(void)
{
#if defined (CONFIG_PLATFORM_SN7320)
	usbh_7320_phy_configuration();
	
	NVIC_SetPriority( USB_HOST1_IRQn, 6 );
#endif
	
	osSemaphoreDef(USBH_SEM_FRAME_LIST_ROLLOVER_2);
	USBH_SEM_FRAME_LIST_ROLLOVER_2    = osSemaphoreCreate(osSemaphore(USBH_SEM_FRAME_LIST_ROLLOVER_2), 1);
	osSemaphoreWait(USBH_SEM_FRAME_LIST_ROLLOVER_2, USBH_10ms);
	
	osSemaphoreDef(USBH_SEM_ASYNC_ADV_2);
	USBH_SEM_ASYNC_ADV_2    = osSemaphoreCreate(osSemaphore(USBH_SEM_ASYNC_ADV_2), 1);
	osSemaphoreWait(USBH_SEM_ASYNC_ADV_2, USBH_10ms);
	
#if !defined (CONFIG_WIFI_SDK)
	osSemaphoreDef(USBH_SEM_WAKEUP_WIFI_2);
	USBH_SEM_WAKEUP_WIFI_2    = osSemaphoreCreate(osSemaphore(USBH_SEM_WAKEUP_WIFI_2), 1);
	osSemaphoreWait(USBH_SEM_WAKEUP_WIFI_2, USBH_10ms);
#endif
	
	osSemaphoreDef(USBH_SEM_WAKEUP_ERR_HDL_2);
	USBH_SEM_WAKEUP_ERR_HDL_2    = osSemaphoreCreate(osSemaphore(USBH_SEM_WAKEUP_ERR_HDL_2), 1);
	osSemaphoreWait(USBH_SEM_WAKEUP_ERR_HDL_2, USBH_10ms);
	
	osSemaphoreDef(USBH_SEM_WAKEUP_AUTO_BKIN_2);
	USBH_SEM_WAKEUP_AUTO_BKIN_2    = osSemaphoreCreate(osSemaphore(USBH_SEM_WAKEUP_AUTO_BKIN_2), 1);
	osSemaphoreWait(USBH_SEM_WAKEUP_AUTO_BKIN_2, USBH_10ms);
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	if(USBH_QUEUE_ERR_HDL == NULL) {
		osMessageQDef(USBH_QUEUE_ERR_HDL, USBH_QUEUE_SIZE, USBH_ERR_HDL_STRUCT);
		USBH_QUEUE_ERR_HDL = osMessageCreate(osMessageQ(USBH_QUEUE_ERR_HDL), NULL);
	}
	
	if(USBH_QUEUE_ERR_RLT == NULL) {
		osMessageQDef(USBH_QUEUE_ERR_RLT, USBH_QUEUE_SIZE, USBH_ERR_RLT_STRUCT);
		USBH_QUEUE_ERR_RLT = osMessageCreate(osMessageQ(USBH_QUEUE_ERR_RLT), NULL);
	}
#endif

	osMutexDef(USBH_MUTEX_MSC_TRANSFER_2);
	USBH_MUTEX_MSC_TRANSFER_2 = osMutexCreate(osMutex(USBH_MUTEX_MSC_TRANSFER_2));
	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	osSemaphoreDef(SEM_USBH_PLUG_IN);
	auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN    = osSemaphoreCreate(osSemaphore(SEM_USBH_PLUG_IN), 1);
	osSemaphoreWait(auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN, USBH_10ms);

	osSemaphoreDef(SEM_USBH_PLUG_OUT);
	auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT    = osSemaphoreCreate(osSemaphore(SEM_USBH_PLUG_OUT), 1);
	osSemaphoreWait(auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT, USBH_10ms);
#endif

#if defined (CONFIG_WIFI_SDK)	
	osSemaphoreDef(usbh_driver_list.list_lock);
	usbh_driver_list.list_lock    = osSemaphoreCreate(osSemaphore(usbh_driver_list.list_lock), 1);
#endif
		
#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	osThreadDef(USBH_ENUM_2, usbh_enum_task_2, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_16K);
	xTASK_HDL_USBH_ENUM_2 = osThreadCreate(osThread(USBH_ENUM_2), NULL);
	if( xTASK_HDL_USBH_ENUM_2 == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");
	
	osThreadDef(USBH_ERROR_HANDLE_2, usbh_err_hdl_task_2, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_4K);
	xTASK_HDL_USBH_ERRHDL_2 = osThreadCreate(osThread(USBH_ERROR_HANDLE_2), NULL);
	if( xTASK_HDL_USBH_ERRHDL_2 == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(USBH_ENUM_2, usbh_enum_task_2, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_16K);
	xTASK_HDL_USBH_ENUM_2 = osThreadCreate(osThread(USBH_ENUM_2), NULL);
	if( xTASK_HDL_USBH_ENUM_2 == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");
	
	osThreadDef(USBH_ERROR_HANDLE_2, usbh_err_hdl_task_2, THREAD_PRIO_USBH_PROCESS, 1, STACK_SIZE_4K);
	xTASK_HDL_USBH_ERRHDL_2 = osThreadCreate(osThread(USBH_ERROR_HANDLE_2), NULL);
	if( xTASK_HDL_USBH_ERRHDL_2 == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	osThreadDef(USBH_ENUM_2, usbh_enum_task_2, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_USBH_ENUM_2 = osThreadCreate(osThread(USBH_ENUM_2), NULL);
	if( xTASK_HDL_USBH_ENUM_2 == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

	osThreadDef(USBH_ERROR_HANDLE_2, usbh_err_hdl_task_2, THREAD_PRIO_USBH_PROCESS, 1, 1024);
	xTASK_HDL_USBH_ERRHDL_2 = osThreadCreate(osThread(USBH_ERROR_HANDLE_2), NULL);
	if( xTASK_HDL_USBH_ERRHDL_2 == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif 

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	osThreadDef(USBH_ENUM_2, usbh_enum_task_2, THREAD_PRIO_USBH_PROCESS, 1, 512);
	xTASK_HDL_USBH_ENUM_2 = osThreadCreate(osThread(USBH_ENUM_2), NULL);
	if( xTASK_HDL_USBH_ENUM_2 == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

	osThreadDef(USBH_ERROR_HANDLE_2, usbh_err_hdl_task_2, THREAD_PRIO_USBH_PROCESS, 1, 128);
	xTASK_HDL_USBH_ERRHDL_2 = osThreadCreate(osThread(USBH_ERROR_HANDLE_2), NULL);
	if( xTASK_HDL_USBH_ERRHDL_2 == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif 
	
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	osThreadDef(USBH_ENUM_2, usbh_enum_task_2, THREAD_PRIO_USBH_PROCESS, 1, 512);
	xTASK_HDL_USBH_ENUM_2 = osThreadCreate(osThread(USBH_ENUM_2), NULL);
	if( xTASK_HDL_USBH_ENUM_2 == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

	osThreadDef(USBH_ERROR_HANDLE_2, usbh_err_hdl_task_2, THREAD_PRIO_USBH_PROCESS, 1, 128);
	xTASK_HDL_USBH_ERRHDL_2 = osThreadCreate(osThread(USBH_ERROR_HANDLE_2), NULL);
	if( xTASK_HDL_USBH_ERRHDL_2 == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n");
#endif 

#if defined( CONFIG_XILINX_SDK )
	osThreadDef(USBH_ENUM_2, usbh_enum_task_2, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_USBH_ENUM_2 = osThreadCreate(osThread(USBH_ENUM_2), NULL);
	if( xTASK_HDL_USBH_ENUM_2 == NULL )
		printf("Create xTASK_HDL_USBH_ENUM fail\n");

	osThreadDef(USBH_ERROR_HANDLE_2, usbh_err_hdl_task_2, THREAD_PRIO_USBH_PROCESS, 1, 1024);
	xTASK_HDL_USBH_ERRHDL_2 = osThreadCreate(osThread(USBH_ERROR_HANDLE_2), NULL);
	if( xTASK_HDL_USBH_ERRHDL_2 == NULL )
		printf("Create xTASK_HDL_USBH_ERRHDL fail\n"); 
#endif
	//usbh_auto_bkin_init();
}

void usbh_freertos_uninit_2(void)
{
	if(USBH_SEM_FRAME_LIST_ROLLOVER_2 != NULL){
		osSemaphoreDelete(USBH_SEM_FRAME_LIST_ROLLOVER_2);
		USBH_SEM_FRAME_LIST_ROLLOVER_2 = NULL;
	}
	if(USBH_SEM_ASYNC_ADV_2 != NULL){
		osSemaphoreDelete(USBH_SEM_ASYNC_ADV_2);
		USBH_SEM_ASYNC_ADV_2 = NULL;
	}
#if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_WIFI_2 != NULL){
		osSemaphoreDelete(USBH_SEM_WAKEUP_WIFI_2);
		USBH_SEM_WAKEUP_WIFI_2 = NULL;
	}
#endif // end of if !defined (CONFIG_WIFI_SDK)	
	if(USBH_SEM_WAKEUP_ERR_HDL_2 != NULL){
		osSemaphoreDelete(USBH_SEM_WAKEUP_ERR_HDL_2);
		USBH_SEM_WAKEUP_ERR_HDL_2 = NULL;
	}
	if(USBH_SEM_WAKEUP_AUTO_BKIN_2 != NULL){
		osSemaphoreDelete(USBH_SEM_WAKEUP_AUTO_BKIN_2);
		USBH_SEM_WAKEUP_AUTO_BKIN_2 = NULL;
	}

	usbh_uninit_2();

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	if (auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN != NULL) {
		osSemaphoreDelete(auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN);
		auto_mount_info_usbh[1].xSEM_USBH_PLUG_IN = NULL;
	}

	if (auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT != NULL) {
		osSemaphoreDelete(auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT);
		auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT = NULL;
	}
#endif
	
	if(xTASK_HDL_USBH_ERRHDL_2 != NULL){
		osThreadTerminate(xTASK_HDL_USBH_ERRHDL_2);
		xTASK_HDL_USBH_ERRHDL_2 = NULL;
	}

	if(xTASK_HDL_USBH_ENUM_2 != NULL){
		osThreadTerminate(xTASK_HDL_USBH_ENUM_2);
		xTASK_HDL_USBH_ENUM_2 = NULL;
	}	
}
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
void usbh_enum_task_2(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void usbh_enum_task_2(void const *pvParameters)
#endif
{
	usbh_init_2();
	while(1) {
		usbh_root_enum_2();
		
#if defined (CONFIG_USBH_FREE_RTOS)
		vTaskDelay(USBH_10ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_10ms);
#endif
	}	
}
#endif // End of if defined( CONFIG_DUAL_HOST )

#if defined (CONFIG_WIFI_SDK)
uint32_t usbh_driver_register(struct usbh_driver *driver)
{
	struct usbh_driver *pdrv;
	struct list_head *ptr, *n;
	uint32_t err = FAIL;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	while(xSemaphoreTake(usbh_driver_list.list_lock, portMAX_DELAY) != pdPASS );
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	while(osSemaphoreWait(usbh_driver_list.list_lock, portMAX_DELAY) != osOK );
#endif

	if (!list_empty(&usbh_driver_list.list)) {
		/* lookup driver registered for same vid/pid pair */
		list_for_each_safe(ptr, n, &usbh_driver_list.list) {
			pdrv = list_entry(ptr, struct usbh_driver, list);
			USBH_INFO("%s(%d) trying add driver %s for %#04x:%#04x\n", __FUNCTION__, __LINE__,
													driver->name, pdrv->idVendor, pdrv->idProduct);
			if((pdrv->idVendor == driver->idVendor) &&
						(pdrv->idProduct == driver->idProduct)) {
				USBH_INFO("%s(%d) driver %s is already registerd for device %#04x:%#04x\n",
						__FUNCTION__, __LINE__, driver->name, driver->idVendor, driver->idProduct);
				err = FAIL;
				goto existed;
			}
		}
	}

	/* add the driver into list */
	INIT_LIST_HEAD(&driver->list);
#if defined (CONFIG_USBH_FREE_RTOS)
	driver->list_lock = xSemaphoreCreateCounting(1, 1);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreDef(driver->list_lock);
	driver->list_lock    = osSemaphoreCreate(osSemaphore(driver->list_lock), 1);
#endif
	list_add(&driver->list, &usbh_driver_list.list);

	/* to do
	 * Call probe function of the driver for plugged device
	 */
existed:
#if defined (CONFIG_USBH_FREE_RTOS)
	xSemaphoreGive(usbh_driver_list.list_lock);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreRelease(usbh_driver_list.list_lock);
#endif
	return err;
}

void usbh_driver_deregister(struct usbh_driver *driver)
{
	/* to be implemented*/
}

uint32_t usbh_device_add(USBH_Device_Structure *usbh_dev)
{
	struct usbh_driver *pdrv;
	struct list_head *ptr, *n;
	uint16_t vid = (usbh_dev->DEVDes.bVIDHighByte << 8) | (usbh_dev->DEVDes.bVIDLowByte);
	uint16_t pid = (usbh_dev->DEVDes.bPIDHighByte << 8) | (usbh_dev->DEVDes.bPIDLowByte);
	uint32_t err = FAIL;

#if defined (CONFIG_USBH_FREE_RTOS)
	while(xSemaphoreTake(usbh_driver_list.list_lock, portMAX_DELAY) != pdPASS );
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	while(osSemaphoreWait(usbh_driver_list.list_lock, portMAX_DELAY) != osOK );
#endif

	if (!list_empty(&usbh_driver_list.list)) {
		/* lookup driver registered for THIS vid/pid pair */
		list_for_each_safe(ptr, n, &usbh_driver_list.list) {
			pdrv = list_entry(ptr, struct usbh_driver, list);
			if((pdrv->idVendor == vid) &&
						(pdrv->idProduct == pid)) {
				if (pdrv->probe)
					err = pdrv->probe(usbh_dev);

				if (err == SUCCESS)
					USBH_INFO("%s(%d) driver %s for %#04x:%#04x is called\n",
									__FUNCTION__, __LINE__, pdrv->name, vid, pid);
				goto found;
			}
		}
	}

found:
#if defined (CONFIG_USBH_FREE_RTOS)
	xSemaphoreGive(usbh_driver_list.list_lock);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreRelease(usbh_driver_list.list_lock);
#endif
	return err;
}

void usbh_device_remove(USBH_Device_Structure *usbh_dev)
{
	struct usbh_driver *pdrv;
	struct list_head *ptr, *n;
	uint16_t vid = (usbh_dev->DEVDes.bVIDHighByte << 8) | (usbh_dev->DEVDes.bVIDLowByte);
	uint16_t pid = (usbh_dev->DEVDes.bPIDHighByte << 8) | (usbh_dev->DEVDes.bPIDLowByte);
	
#if defined (CONFIG_USBH_FREE_RTOS)
	while(xSemaphoreTake(usbh_driver_list.list_lock, portMAX_DELAY) != pdPASS );
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	while(osSemaphoreWait(usbh_driver_list.list_lock, portMAX_DELAY) != osOK );
#endif

	if (!list_empty(&usbh_driver_list.list)) {
		/* lookup driver registered for THIS vid/pid pair */
		list_for_each_safe(ptr, n, &usbh_driver_list.list) {
			pdrv = list_entry(ptr, struct usbh_driver, list);
			if((pdrv->idVendor == vid) &&
						(pdrv->idProduct == pid)) {
				if (pdrv->disconnect) {
					pdrv->disconnect();
					USBH_INFO("%s(%d) driver %s for %#04x:%#04x is called\n",
									__FUNCTION__, __LINE__, pdrv->name, vid, pid);
				}
				goto removed;
			}
		}
	}

removed:
#if defined (CONFIG_USBH_FREE_RTOS)
	xSemaphoreGive(usbh_driver_list.list_lock);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreRelease(usbh_driver_list.list_lock);
#endif
	
	return;
}
#endif // end of if defined (CONFIG_WIFI_SDK)
