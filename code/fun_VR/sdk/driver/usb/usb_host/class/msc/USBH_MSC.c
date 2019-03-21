/** \defgroup MSC_FUN MSC functions
 *  \ingroup MSC_MODULE
 *  \brief MSC Class Driver API Introduction
*/

/**
 * @file
 * this	is  USBH_MSC	file
 * MSC.c
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#include "sonix_config.h"
 
#if defined (CONFIG_MODULE_USB_MSC_CLASS)

#if defined (CONFIG_USBH_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#endif

#include <string.h>
#include "USBH_CORE.h"
#include "USBH.h"
#include "USBH_MSC.h"

#if defined( CONFIG_SN_GCC_SDK )
#include <generated/snx_sdk_conf.h> 
#include <bsp.h>
#include <nonstdlib.h>
extern void flush_dcache_area(unsigned int start_addr, unsigned int size);
#endif 

#if defined( CONFIG_SN_KEIL_SDK )
#include <stdlib.h>
#if defined( CONFIG_PLATFORM_ST53510 )
#include "INTC.h"
//#include "usbh_test.h"
#endif 
#endif

#if defined( CONFIG_XILINX_SDK )
#include <stdlib.h>
#include <stdio.h>
#include <xil_cache.h>
#endif

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
extern automount_info_usbh	auto_mount_info_usbh[2];
#endif

MSC_DEVICE_STRUCT MSC_DEV[USBH_MSC_MAX_PORT];

#if defined (CONFIG_USBH_FREE_RTOS)
QueueHandle_t	USBH_QUEUE_WAKEUP_MSC_DRV = NULL;
QueueHandle_t	USBH_QUEUE_MSC_TEST = NULL;	
QueueHandle_t	USBH_QUEUE_READER_CARD_IN[USBH_MSC_MAX_PORT] = {NULL};
TaskHandle_t 	xTASK_USBH_MSC_TEST	= NULL;	
TaskHandle_t 	xTASK_HDL_MSC_DRV[USBH_MSC_MAX_PORT] = {NULL};
TaskHandle_t 	xTASK_TEST_UNIT_READY[USBH_MSC_MAX_PORT] = {NULL};
extern 				SemaphoreHandle_t	USBH_MUTEX_MSC_TRANSFER;
#endif //end of #if defined (CONFIG_USBH_FREE_RTOS)
	
#if defined (CONFIG_USBH_CMSIS_OS)
osMessageQId 	USBH_QUEUE_WAKEUP_MSC_DRV	= NULL;
osMessageQId 	USBH_QUEUE_MSC_TEST	= NULL;	
osMessageQId	USBH_QUEUE_READER_CARD_IN[USBH_MSC_MAX_PORT] = {NULL};
osThreadId 		xTASK_USBH_MSC_TEST	= NULL;	
osThreadId 		xTASK_HDL_MSC_DRV[USBH_MSC_MAX_PORT] = {NULL};
osThreadId 		xTASK_TEST_UNIT_READY[USBH_MSC_MAX_PORT] = {NULL};
extern 				osMutexId					USBH_MUTEX_MSC_TRANSFER;
#endif	//end of #if defined (CONFIG_USBH_CMSIS_OS)

#if defined( CONFIG_DUAL_HOST )		
	#if defined (CONFIG_USBH_FREE_RTOS)	
	QueueHandle_t	USBH_QUEUE_WAKEUP_MSC_DRV_2 = NULL;	
	extern 				SemaphoreHandle_t	USBH_MUTEX_MSC_TRANSFER_2;	
	#endif

	#if defined (CONFIG_USBH_CMSIS_OS)	
	osMessageQId 	USBH_QUEUE_WAKEUP_MSC_DRV_2	= NULL;	
	extern 				osMutexId					USBH_MUTEX_MSC_TRANSFER_2;
	#endif
#endif	//end of #if defined( CONFIG_DUAL_HOST )	

//MSC_REQ_Struct msc_req_dev;

// MSC RESOURCE CHK
uint32_t MSC_RESRC = USBH1;
BK_CMD_STAGE next_stage;		// next bk_xfr stage
uint8_t MSG_PRT[2];
uint8_t continuous_err_cnt = 0;
uint32_t CBWTag = 0;
uint32_t bulk_timeout_value = USBH_10000ms;

#if defined (CONFIG_USBH_FREE_RTOS)
void test_unit_ready_task(void *pvParameters);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void test_unit_ready_task(void const *pvParameters);
#endif

// Mass Storage Class Vendor Command
uint8_t BULK_VENDOR_READ_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_VENDOR_WRITE_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_VENDOR_NONDATA_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Mass	Storage	Class Command
uint8_t BULK_INQUIRY_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00,
		0x24, 0x00, 0x00, 0x00, 0x80, 0x00, 0x05, 0x12, 0x00, 0x00, 0x00, 0x24,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_READ_CAPACITY_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00,
		0x00, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x25, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_TEST_UNIT_READY_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_REQUEST_SENSE_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00,
		0x00, 0x12, 0x00, 0x00, 0x00, 0x80, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00,
		0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_READ10_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x09, 0x28, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_WRITE10_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x2A, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_READ_FORMAT_CAPACITIES_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00,
		0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x80, 0x00, 0x09, 0x23, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00 };

uint8_t BULK_MODE_SENSE_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x09, 0x5A, 0x00, 0x3F, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t BULK_PREVENT_ALLW_MDM_RMVL_31[] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x1E, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00 };

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 *
 * @{
 */
/** \fn	uint32_t msc_dev_init(uint8_t ID)
 * \brief MSC device init function
 *
 * \param ID 1 = primary MSC device , 2	= secondary MSC device , ETC ...
 *
 * \return target USB device structure pointer (USBH_Device_Structure*)
 */
  /** @} */
uint32_t msc_dev_init(uint8_t ID) {

	return usbh_msc_init(ID);
}

void msc_take_msc_transfer_mutex()
{
//DUAL HOST
#if defined( CONFIG_DUAL_HOST )	
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)
#if defined (CONFIG_USBH_FREE_RTOS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
		xSemaphoreTake(USBH_MUTEX_MSC_TRANSFER, USBH_3000ms);	
#endif	
#if defined (CONFIG_USBH_CMSIS_OS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
		osMutexWait(USBH_MUTEX_MSC_TRANSFER, USBH_3000ms);
#endif	
#endif	//end of #if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)

#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_DEV_PHY1_HOST)
#if defined (CONFIG_USBH_FREE_RTOS)	
	if( USBH_MUTEX_MSC_TRANSFER_2 != NULL )
		xSemaphoreTake(USBH_MUTEX_MSC_TRANSFER_2, USBH_3000ms);
#endif	
#if defined (CONFIG_USBH_CMSIS_OS)	
	if( USBH_MUTEX_MSC_TRANSFER_2 != NULL )
		osMutexWait(USBH_MUTEX_MSC_TRANSFER_2, USBH_3000ms);
#endif
#endif	//end of #if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_DEV_PHY1_HOST)	
	
#else
	
//Not DUAL HOST
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)
#if defined (CONFIG_USBH_FREE_RTOS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
		xSemaphoreTake(USBH_MUTEX_MSC_TRANSFER, USBH_3000ms);
#endif	
	
#if defined (CONFIG_USBH_CMSIS_OS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
		osMutexWait(USBH_MUTEX_MSC_TRANSFER, USBH_3000ms);
#endif	
#endif	//end of #if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)

#endif	//end of #if defined( CONFIG_DUAL_HOST )	
}

void msc_give_msc_transfer_mutex()
{
//DUAL HOST
#if defined( CONFIG_DUAL_HOST )	
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)
#if defined (CONFIG_USBH_FREE_RTOS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
	{
		xSemaphoreGive(USBH_MUTEX_MSC_TRANSFER);
		USBH_MUTEX_MSC_TRANSFER = NULL;
	}
#endif	
#if defined (CONFIG_USBH_CMSIS_OS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
	{
		osMutexRelease(USBH_MUTEX_MSC_TRANSFER);
		USBH_MUTEX_MSC_TRANSFER = NULL;
	}
#endif	
#endif	//end of #if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)

#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_DEV_PHY1_HOST)
#if defined (CONFIG_USBH_FREE_RTOS)	
	if( USBH_MUTEX_MSC_TRANSFER_2 != NULL )
	{
		xSemaphoreGive(USBH_MUTEX_MSC_TRANSFER_2);
		USBH_MUTEX_MSC_TRANSFER_2 = NULL;
	}
#endif	
#if defined (CONFIG_USBH_CMSIS_OS)	
	if( USBH_MUTEX_MSC_TRANSFER_2 != NULL )
	{
		osMutexRelease(USBH_MUTEX_MSC_TRANSFER_2);
		USBH_MUTEX_MSC_TRANSFER_2 = NULL;
	}
#endif
#endif	//end of #if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_DEV_PHY1_HOST)	
	
#else
	
//Not DUAL HOST
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)
#if defined (CONFIG_USBH_FREE_RTOS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
	{
		xSemaphoreGive(USBH_MUTEX_MSC_TRANSFER);
		USBH_MUTEX_MSC_TRANSFER = NULL;
	}
#endif	
	
#if defined (CONFIG_USBH_CMSIS_OS)	
	if( USBH_MUTEX_MSC_TRANSFER != NULL )
	{
		osMutexRelease(USBH_MUTEX_MSC_TRANSFER);
		USBH_MUTEX_MSC_TRANSFER = NULL;
	}
#endif	
#endif	//end of #if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)

#endif	//end of #if defined( CONFIG_DUAL_HOST )	
}

uint8_t msc_transfer(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req) {

	USBH_BK_XFR_REQ_Struct bk_req;
	USBH_CX_XFR_REQ_Struct cx_req;
	uint32_t *cbw_data;
	uint8_t *cbw_data8;
	uint8_t status = SUCCESS;
	uint8_t bk_status = FAIL;
	uint32_t *cbw_buff, *csw_buff;
	uint32_t BulkTimeout = 0;

	msc_take_msc_transfer_mutex();
	
	memset(&bk_req, 0, sizeof(USBH_BK_XFR_REQ_Struct));
	memset(&cx_req, 0, sizeof(cx_req));

	++CBWTag;
	if(CBWTag == 0xFFFFFFFF) {
		CBWTag = 1;
	}

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do {
		cbw_buff = pvPortMalloc(32, GFP_DMA, MODULE_DRI_USBH);
	} while (cbw_buff == NULL);
	do {
		csw_buff = pvPortMalloc(32, GFP_DMA, MODULE_DRI_USBH);
	} while (csw_buff == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do {
		cbw_buff = pvPortMalloc(32);
	}while(cbw_buff == NULL);
	do {
		csw_buff = pvPortMalloc(32);
	}while(csw_buff == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
	cbw_buff = (uint32_t *)malloc(32);
	csw_buff = (uint32_t *)malloc(32);
#endif
#endif

	// CBW Stage
	if (next_stage == CBW_STAGE) {
		next_stage = DATA_STAGE;

		switch (msc_req->cmd) {
		case CBW_BULK_READ10:
			memcpy(cbw_buff, BULK_READ10_31, sizeof(BULK_READ10_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[4] = CBWTag;
			cbw_data8[5] = (CBWTag >> 8);
			cbw_data8[6] = (CBWTag >> 16);
			cbw_data8[7] = (CBWTag >> 24);
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[17] = (uint8_t) ((msc_req->lba) >> 24);
			cbw_data8[18] = (uint8_t) ((msc_req->lba) >> 16);
			cbw_data8[19] = (uint8_t) ((msc_req->lba) >> 8);
			cbw_data8[20] = (uint8_t) (msc_req->lba);
			cbw_data8[22] = (uint8_t) (cbw_data[2] / msc_req->block_len) >> 8;
			cbw_data8[23] = (uint8_t) (cbw_data[2] / msc_req->block_len);
			break;

		case CBW_BULK_WRITE10:
			memcpy(cbw_buff, BULK_WRITE10_31, sizeof(BULK_WRITE10_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[4] = CBWTag;
			cbw_data8[5] = (CBWTag >> 8);
			cbw_data8[6] = (CBWTag >> 16);
			cbw_data8[7] = (CBWTag >> 24);
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[17] = (uint8_t) ((msc_req->lba) >> 24);
			cbw_data8[18] = (uint8_t) ((msc_req->lba) >> 16);
			cbw_data8[19] = (uint8_t) ((msc_req->lba) >> 8);
			cbw_data8[20] = (uint8_t) (msc_req->lba);
			cbw_data8[22] = (uint8_t) (cbw_data[2] / msc_req->block_len) >> 8;
			cbw_data8[23] = (uint8_t) (cbw_data[2] / msc_req->block_len);

			break;
		case CBW_BULK_INQUIRY:
			memcpy(cbw_buff, BULK_INQUIRY_31, sizeof(BULK_INQUIRY_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[18] = (uint8_t) ((msc_req->size) >> 8);
			cbw_data8[19] = (uint8_t) (msc_req->size);
			break;
		case CBW_BULK_READ_CAPACITY:
			memcpy(cbw_buff, BULK_READ_CAPACITY_31, sizeof(BULK_READ_CAPACITY_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[4] = CBWTag;
			cbw_data8[5] = (CBWTag >> 8);
			cbw_data8[6] = (CBWTag >> 16);
			cbw_data8[7] = (CBWTag >> 24);
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[17] = (uint8_t) ((msc_req->lba) >> 24);
			cbw_data8[18] = (uint8_t) ((msc_req->lba) >> 16);
			cbw_data8[19] = (uint8_t) ((msc_req->lba) >> 8);
			cbw_data8[20] = (uint8_t) (msc_req->lba);
			break;
		case CBW_BULK_REQUEST_SENSE:
			memcpy(cbw_buff, BULK_REQUEST_SENSE_31, sizeof(BULK_REQUEST_SENSE_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[18] = (uint8_t) ((msc_req->size) >> 8);
			cbw_data8[19] = (uint8_t) (msc_req->size);
			break;
		case CBW_BULK_TEST_UNIT_READY:
			memcpy(cbw_buff, BULK_TEST_UNIT_READY_31, sizeof(BULK_TEST_UNIT_READY_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[4] = CBWTag;
			cbw_data8[5] = (CBWTag >> 8);
			cbw_data8[6] = (CBWTag >> 16);
			cbw_data8[7] = (CBWTag >> 24);
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			break;
		case CBW_BULK_READ_FORMAT_CAPACITIES:
			memcpy(cbw_buff, BULK_READ_FORMAT_CAPACITIES_31, sizeof(BULK_READ_FORMAT_CAPACITIES_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[22] = (uint8_t) ((msc_req->size) >> 8);
			cbw_data8[23] = (uint8_t) (msc_req->size);
			break;
		case CBW_BULK_MODE_SENSE:
			memcpy(cbw_buff, BULK_MODE_SENSE_31, sizeof(BULK_MODE_SENSE_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			cbw_data8[18] = (uint8_t) ((msc_req->size) >> 8);
			cbw_data8[19] = (uint8_t) (msc_req->size);
			break;
		case CBW_PREVENT_ALLW_MDM_RMVL:
			memcpy(cbw_buff, BULK_PREVENT_ALLW_MDM_RMVL_31, sizeof(BULK_PREVENT_ALLW_MDM_RMVL_31));
			cbw_data = (uint32_t*) cbw_buff;
			cbw_data[2] = msc_req->size;
			cbw_data8 = (uint8_t*) cbw_buff;
			cbw_data8[13] = (uint8_t) (msc_req->lun);
			break;

		case CBW_BULK_VENDOR_READ:
			memcpy(cbw_buff, BULK_VENDOR_READ_31, sizeof(BULK_VENDOR_READ_31));
			break;

		case CBW_BULK_VENDOR_WRITE:
			memcpy(cbw_buff, BULK_VENDOR_WRITE_31, sizeof(BULK_VENDOR_WRITE_31));
			break;

		case CBW_BULK_VENDOR_NONDATA:
			memcpy(cbw_buff, BULK_VENDOR_NONDATA_31, sizeof(BULK_VENDOR_NONDATA_31));
			break;
		}

		if(msc_req->size == 0) {
			next_stage = CSW_STAGE;
		}
		bk_req.XfrType = USBH_BK_OUT_TYPE;
		bk_req.NUM = 1;
		bk_req.SIZE = 31;
		bk_req.pBUFF = (uint32_t*) cbw_buff;
		//bk_req.TimeoutMsec = USBH_10000ms;
#ifdef CONFIG_MODULE_USBH_MSC_BK_OUT_TIMEOUT
#if defined (CONFIG_USBH_FREE_RTOS)
		BulkTimeout = CONFIG_MODULE_USBH_MSC_BK_OUT_TIMEOUT/portTICK_RATE_MS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		BulkTimeout = CONFIG_MODULE_USBH_MSC_BK_OUT_TIMEOUT;
#endif
#else
		BulkTimeout = USBH_BK_OUT_TIMEOUT;
#endif

		bk_req.TimeoutMsec = BulkTimeout;

		bk_status = usbh_bk_xfr(dev, (USBH_BK_XFR_REQ_Struct*) &bk_req);
		if (bk_status != SUCCESS) {
			MSC_DBG("CBW FAIL !!");
			MSC_DBG("CBWTag = 0x%x, %x, %x, %x", cbw_data8[4], cbw_data8[5], cbw_data8[6], cbw_data8[7]);
			next_stage = CBW_STAGE;
#if defined (CONFIG_USBH_FREE_RTOS)
			vPortFree(cbw_buff);
			vPortFree(csw_buff);			
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			free(cbw_buff);
			free(csw_buff);			
#endif
			msc_give_msc_transfer_mutex();
			
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
			return RECOVERY_FAIL;
#else
			return FAIL;
#endif
		}
	}

	// Data Stage
	if (next_stage == DATA_STAGE) {
		next_stage = CSW_STAGE;

		switch (msc_req->cmd) {
		case CBW_BULK_READ10:
		case CBW_BULK_INQUIRY:
		case CBW_BULK_READ_CAPACITY:
		case CBW_BULK_REQUEST_SENSE:
		case CBW_MODE_SENSE:
		case CBW_READ_FORMAT_CAPACITIES:
		case CBW_BULK_VENDOR_READ:
			bk_req.XfrType = USBH_BK_IN_TYPE;
			bk_req.NUM = 1;
			bk_req.SIZE = msc_req->size;
			bk_req.pBUFF = (uint32_t*) msc_req->pbuff;
			bk_req.TimeoutMsec = bulk_timeout_value;
			bk_status = usbh_bk_xfr(dev, (USBH_BK_XFR_REQ_Struct*) &bk_req);
			if (bk_status != SUCCESS) {
				MSC_DBG("DATAR FAIL !!");
				MSC_DBG("CBWTag = 0x%x, %x, %x, %x", cbw_data8[4], cbw_data8[5], cbw_data8[6], cbw_data8[7]);
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
				next_stage = CBW_STAGE;
#endif
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(cbw_buff);
				vPortFree(csw_buff);
#endif
				
#if defined (CONFIG_USBH_CMSIS_OS)
				free(cbw_buff);
				free(csw_buff);
#endif
				msc_give_msc_transfer_mutex();
				
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
				return RECOVERY_FAIL;
#else
				return FAIL;
#endif
			}
			break;
		case CBW_BULK_WRITE10:
		case CBW_BULK_VENDOR_WRITE:
			bk_req.XfrType = USBH_BK_OUT_TYPE;
			bk_req.NUM = 1;
			bk_req.SIZE = msc_req->size;
			bk_req.pBUFF = (uint32_t*) msc_req->pbuff;
			bk_req.TimeoutMsec = bulk_timeout_value;
			bk_status = usbh_bk_xfr(dev, (USBH_BK_XFR_REQ_Struct*) &bk_req);
			if (bk_status != SUCCESS) {
				MSC_DBG("DATAW FAIL !!");
				MSC_DBG("CBWTag = 0x%x, %x, %x, %x", cbw_data8[4], cbw_data8[5], cbw_data8[6], cbw_data8[7]);
				next_stage = CBW_STAGE;
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(cbw_buff);
				vPortFree(csw_buff);				
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
				free(cbw_buff);
				free(csw_buff);
#endif
				msc_give_msc_transfer_mutex();
				
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
				return RECOVERY_FAIL;
#else
				return FAIL;
#endif
			}
			break;
		}
	}

	// CSW Stage
	if (next_stage == CSW_STAGE) {
		next_stage = CBW_STAGE;

		bk_req.XfrType = USBH_BK_IN_TYPE;
		bk_req.NUM = 1;
		bk_req.SIZE = 13;
		bk_req.pBUFF = (uint32_t*) csw_buff;
		bk_req.TimeoutMsec = bulk_timeout_value;
		bk_status = usbh_bk_xfr(dev, (USBH_BK_XFR_REQ_Struct*) &bk_req);
if (bk_status != SUCCESS) {
			MSC_DBG("CSW FAIL !!");
			MSC_DBG("CBWTag = 0x%x, %x, %x, %x", cbw_data8[4], cbw_data8[5], cbw_data8[6], cbw_data8[7]);
#if !defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)			
			if(dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_Halted) {
				next_stage = CSW_STAGE;
			}
#endif
#if defined (CONFIG_USBH_FREE_RTOS)
			vPortFree(cbw_buff);
			vPortFree(csw_buff);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			free(cbw_buff);
			free(csw_buff);
#endif
			msc_give_msc_transfer_mutex();
			
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
			return RECOVERY_FAIL;
#else
			return FAIL;
#endif
		}

		if (csw_buff[0] == 0x53425355) {
			memcpy(&msc_req->csw, csw_buff, sizeof(msc_req->csw));
		} else {
			dev->EP[dev->iEPNUM].status = EHCI_QTD_STATUS_CSW_NOT_VALID;
			status = FAIL;
		}
	}
	
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(cbw_buff);
	vPortFree(csw_buff);
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
	free(cbw_buff);
	free(csw_buff);
#endif
	msc_give_msc_transfer_mutex();
	
	return status;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 *
 * @{
 */
/** \fn	uint8_t msc_vendor_read(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req, void *pdata_buf, uint32_t size)
 * \brief MSC Class API	function to read vendor command data from a mass storage device
 *
 * \param dev target USB device structure pointer
 *
 * \param msc_vendor_req MSC vendor request structure to give vendor read command
 *
 * \param pdata_buf pointer to store read data
 *
 * \param size data size
 *
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_vendor_read(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req, void *pdata_buf, uint32_t size) {

	MSC_REQ_Struct msc_req;
	uint8_t status = SUCCESS;

	if (dev == NULL) return FAIL;

	memset(&msc_req, 0, sizeof(MSC_REQ_Struct));

	memcpy(BULK_VENDOR_READ_31, msc_vendor_req, sizeof(BULK_VENDOR_READ_31));

	msc_req.cmd = CBW_BULK_VENDOR_READ;
	msc_req.size = size;
	msc_req.pbuff = (uint8_t*) pdata_buf;
	msc_req.lun = msc_vendor_req->lun;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) &msc_req);

	return status;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 *
 * @{
 */
/** \fn	uint8_t msc_vendor_write(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req, void *pdata_buf, uint32_t size)
 * \brief MSC Class API	function to write vendor command data to a mass storage device
 *
 * \param dev target USB device structure pointer
 *
 * \param msc_vendor_req MSC vendor request structure to give vendor write command
 *
 * \param pdata_buf pointer to store ready-to-write data
 *
 * \param size data size
 *
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_vendor_write(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req, void *pdata_buf, uint32_t size) {

	MSC_REQ_Struct msc_req;
	uint8_t status = SUCCESS;

	if (dev == NULL) return FAIL;

	memset(&msc_req, 0, sizeof(MSC_REQ_Struct));

	memcpy(BULK_VENDOR_WRITE_31, msc_vendor_req, sizeof(BULK_VENDOR_WRITE_31));

	msc_req.cmd  = CBW_BULK_VENDOR_WRITE;
	msc_req.size = size;
	msc_req.pbuff = (uint8_t*) pdata_buf;
	msc_req.lun = msc_vendor_req->lun;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) &msc_req);

	return status;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 *
 * @{
 */
/** \fn	uint8_t msc_vendor_nonedata(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req)
 * \brief MSC Class API	function to execute no data transfer vendor command of a mass storage device
 *
 * \param dev target USB device structure pointer
 *
 * \param msc_vendor_req MSC vendor request structure to give vendor no-data-transfer command
 *
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_vendor_nonedata(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req) {

	MSC_REQ_Struct msc_req;
	uint8_t status = SUCCESS;

	if (dev == NULL) return FAIL;

	memset(&msc_req, 0, sizeof(MSC_REQ_Struct));

	memcpy(BULK_VENDOR_NONDATA_31, msc_vendor_req, sizeof(BULK_VENDOR_NONDATA_31));

	msc_req.cmd 	= CBW_BULK_VENDOR_NONDATA;
	msc_req.size 	= 0;
	msc_req.lun 	= msc_vendor_req->lun;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) &msc_req);

	return status;
}


uint8_t msc_cx_get_status(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req) {

	uint8_t status = SUCCESS;

	cx_req->CMD = USBH_CX_CMD_GETSTATUS_ENDPOINT;
	cx_req->SIZE = 0x02;
	cx_req->wValue = 0x00;
	cx_req->wIndex = 0x81;

	status = usbh_cx_xfr((USBH_Device_Structure*) dev, (USBH_CX_XFR_REQ_Struct*) cx_req);
	return status;
}

#if 0
uint8_t cx_clear_feature(USBH_Device_Strcuture *dev, USBH_CX_XFR_REQ_Struct *cx_req)
{
	uint8_t status = SUCCESS;

	cx_req->CMD = USBH_CX_CMD_CLEAR_FEATURE;
	cx_req->SIZE =

	status = usbh_cx_xfr((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)&cx_req);
	return status;
}
#endif

uint8_t msc_cx_bkonly_reset(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req) {

	uint8_t status = SUCCESS;

	cx_req->CMD = USBH_CX_CMD_CLASS_Write;
	cx_req->SIZE = 0x00;
	cx_req->CLASS[0] = 0x21;
	cx_req->CLASS[1] = 0xFF;
	cx_req->CLASS[2] = 0x00;
	cx_req->CLASS[3] = 0x00;
	cx_req->CLASS[4] = 0x00;
	cx_req->CLASS[5] = 0x00;

	status = usbh_cx_xfr((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)cx_req);

	return status;
}

uint8_t msc_reset_recovery(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req) {
	
	uint8_t status = SUCCESS;

	status = msc_cx_bkonly_reset((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)cx_req);

	if(status == SUCCESS)
	{
		status = usbh_cx_clear_feature((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)cx_req, BULK_IN_EP);

		if(status == SUCCESS)
		{
			status = usbh_cx_clear_feature((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)cx_req, BULK_OUT_EP);
		}
	}
	
	if(status != SUCCESS) {
		MSC_DBG("msc_reset_recovery FAIL !!");
	}
	else {
		MSC_DBG("msc_reset_recovery SUCCESS !!");
	}

	return status;
}

uint8_t msc_cx_get_max_lun(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req) {

	uint8_t status = SUCCESS;

	cx_req->CMD = USBH_CX_CMD_CLASS_Read;
	cx_req->SIZE = 0x01;
	cx_req->CLASS[0] = 0xA1;
	cx_req->CLASS[1] = 0xFE;
	cx_req->CLASS[2] = 0x00;
	cx_req->CLASS[3] = 0x00;
	cx_req->CLASS[4] = 0x00;
	cx_req->CLASS[5] = 0x00;

	status = usbh_cx_xfr((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)cx_req);

	return status;
}

uint8_t msc_inquiry(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req) {

	uint8_t status = SUCCESS;

	msc_req->cmd = CBW_BULK_INQUIRY;
	msc_req->size = 36;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) msc_req);
	if (status == SUCCESS) {
		memcpy(msc_req->inquiry, msc_req->pbuff, msc_req->size);
	}

	return status;
}

uint8_t msc_test_unit_ready(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req) {

	uint8_t status = SUCCESS;

	msc_req->cmd = CBW_BULK_TEST_UNIT_READY;
	msc_req->size = 0;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) msc_req);
	return status;
}

uint8_t msc_get_capacity(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req) {

	uint8_t status = SUCCESS;

	msc_req->cmd = CBW_BULK_READ_CAPACITY;
	msc_req->size = 8;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) msc_req);
	if (status == SUCCESS) {
		msc_req->lba = msc_req->pbuff[0] << 24 | msc_req->pbuff[1] << 16
				| msc_req->pbuff[2] << 8 | msc_req->pbuff[3];
		msc_req->block_len = msc_req->pbuff[4] << 24 | msc_req->pbuff[5] << 16
				| msc_req->pbuff[6] << 8 | msc_req->pbuff[7];
		msc_req->capacity = ((uint64_t)(msc_req->lba)) * ((uint64_t)(msc_req->block_len));
	}
	return status;
}

uint8_t msc_request_sense(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req, uint8_t lun) {

	uint8_t status = SUCCESS;

	msc_req->cmd = CBW_BULK_REQUEST_SENSE;
	msc_req->size = 18;
	msc_req->lun = lun;

	status = msc_transfer((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)msc_req);

	if (status == SUCCESS) {
		memcpy(msc_req->request_sense, msc_req->pbuff, msc_req->size);
	}

	return status;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 *
 * @{
 */
/** \fn	uint8_t msc_read10(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req, uint32_t size, uint32_t lba)
 * \brief MSC Class API	function to execute read 10 command
 *
 * \param dev target USB device structure pointer
 *
 * \param msc_req MSC request structure to give read 10 command
 *
 * \param size data size
 *
 * \param lba logical block address to read data
 *
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_read10(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req, uint32_t size, uint32_t lba) {

	uint8_t status = SUCCESS;

	msc_req->cmd = CBW_BULK_READ10;
	msc_req->size = size;
	msc_req->lba = lba;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) msc_req);

	return status;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 *
 * @{
 */
/** \fn	uint8_t msc_write10(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req, uint32_t size, uint32_t lba)
 * \brief MSC Class API	function to execute write 10 command
 *
 * \param dev target USB device structure pointer
 *
 * \param msc_req MSC request structure to give write 10 command
 *
 * \param size data size
 *
 * \param lba logical block address to write data
 *
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_write10(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req, uint32_t size, uint32_t lba) {

	uint8_t status = SUCCESS;

	msc_req->cmd = CBW_BULK_WRITE10;
	msc_req->size = size;
	msc_req->lba = lba;

	status = msc_transfer((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) msc_req);

	return status;
}

uint8_t msc_bulk_enum(USBH_Device_Structure *dev, MSC_DEVICE_STRUCT *msc_dev) {

	USBH_CX_XFR_REQ_Struct cx_req;
	MSC_REQ_Struct msc_req;
	uint32_t size = MSC_ENUM_READ_SIZE;
	uint32_t lba = 0;
	uint32_t i = 0;
	uint8_t status = SUCCESS;
	uint8_t lun = 0;
//	uint8_t buff[64] = {0};
	uint8_t		msc_card_in_lun = 0;
	uint8_t *setup_data_buff = NULL;
	uint8_t *msc_data_buff = NULL;
	char 		task_name[256] = {'\0'};

	memset(&cx_req, 0, sizeof(cx_req));
	memset(&msc_req, 0, sizeof(msc_req));

//	setup_data_buff = &buff[0];
#if defined (CONFIG_USBH_FREE_RTOS)		
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do {
		setup_data_buff = pvPortMalloc(64);
	}while(setup_data_buff == NULL);	
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
	setup_data_buff = (uint8_t *)malloc(64);
#endif
#endif	

	msc_data_buff = (uint8_t*) dev->BUFF.ptr;

	cx_req.pBUFF = (uint32_t*) setup_data_buff;
	msc_req.pbuff = msc_data_buff;

	msc_dev->err_hdl->DriverClass = MSC;

	do {
		++continuous_err_cnt;
		if (continuous_err_cnt == 6) {
			msc_dev->err_hdl->pbuff[0] = CONTINUOUS_ERR;
			return FAIL;
		}
		if (continuous_err_cnt == 12) {
			continuous_err_cnt = 0;
			msc_dev->err_hdl->pbuff[0] = NO_RESPONSE;
			return FAIL;
		}

		//msc_req.lun = lun;
		if( (msc_dev->enum_cmd==MSC_GET_MAX_LUN) || (msc_dev->enum_cmd==MSC_GET_INQUIRY) )
			msc_req.lun = lun;
				
		switch (msc_dev->enum_cmd) {
		case MSC_GET_MAX_LUN:
			msc_cx_get_max_lun((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)&cx_req);
			msc_dev->max_lun = setup_data_buff[0];
			msc_dev->enum_cmd = MSC_GET_INQUIRY;
			MSC_DBG("MAX_LUN = %d", msc_dev->max_lun );
		
#if defined( CONFIG_SN_GCC_SDK )
			if (cx_req.ACT_SIZE) {
				flush_dcache_area((uint32_t) cx_req.pBUFF, cx_req.ACT_SIZE);
			}
#endif
			continuous_err_cnt = 0;
			break;
		case MSC_GET_INQUIRY:
			status = msc_inquiry((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) &msc_req);
			if (status == FAIL) {
				MSC_DBG("GET DEVICE	INFORMATION FAIL!!!");
				if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_Halted) {
					msc_dev->err_hdl->pbuff[0] = EP_STALL;
				} else if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_CSW_NOT_VALID) {
					msc_dev->err_hdl->pbuff[0] = CSW_NOT_VALID;
				} else {
					msc_dev->err_hdl->pbuff[0] = TIMEOUT;
				}
				return FAIL;
			} else if (msc_req.csw[12] == 1) {
				MSC_DBG("GET DEVICE	INFORMATION FAIL!!!");
				msc_dev->err_hdl->pbuff[0] = CMD_FAIL;
				msc_dev->err_hdl->pbuff[1] = msc_req.lun;
				return FAIL;
			} else if(msc_req.csw[12] == 2) {
				MSC_DBG("GET DEVICE	INFORMATION FAIL!!!");
				msc_dev->err_hdl->pbuff[0] = PHASE_ERR;
				return FAIL;
			} else {
				memcpy((uint8_t*) &msc_dev->info[lun].inquiry, (uint8_t*) &msc_req.inquiry, msc_req.size);
				if (lun < msc_dev->max_lun) {
					msc_dev->enum_cmd = MSC_GET_INQUIRY;
					lun++;
				} else {
					msc_dev->enum_cmd = MSC_TEST_UNIT_READY;
					lun = 0;
				}
			}
#if defined( CONFIG_SN_GCC_SDK )				
			if (msc_req.size) {
				flush_dcache_area((uint32_t) msc_req.pbuff, msc_req.size);
			}
#endif
			continuous_err_cnt = 0;
			break;
		case MSC_TEST_UNIT_READY:
			if( (lun == 0) && (xTASK_TEST_UNIT_READY[dev->device_id] == NULL) )
			{
				lun++;
				msc_dev->enum_cmd = MSC_TEST_UNIT_READY;
				sprintf((char *)&task_name, "%s%d", "TUREADY_TASK", dev->device_id);
				
#if defined (CONFIG_USBH_FREE_RTOS)				
				xTaskCreate(
					test_unit_ready_task,
					(const char* )&task_name[0],
					512,
					(void*)&dev->device_id,
					3,
					&xTASK_TEST_UNIT_READY[dev->device_id]
				);
#endif
					
#if defined (CONFIG_USBH_CMSIS_OS)
				//osThreadDef(task_name, test_unit_ready_task, 40, 1, 512);
                osThreadDef(task_name, test_unit_ready_task, THREAD_PRIO_USBH_PROCESS, 1, 512);	
				xTASK_TEST_UNIT_READY[dev->device_id] = osThreadCreate(osThread(task_name), &dev->device_id);
				if( xTASK_TEST_UNIT_READY[dev->device_id] == NULL )
					MSC_DBG("Create Test Unit Ready Task Fail !!\n");	
#endif				
				}
			
#if defined (CONFIG_USBH_FREE_RTOS)				
			if( xQueueReceive(USBH_QUEUE_READER_CARD_IN[dev->device_id], &msc_card_in_lun, USBH_MAX) )
#endif
			
#if defined (CONFIG_USBH_CMSIS_OS)
			osMessageGet(USBH_QUEUE_READER_CARD_IN[dev->device_id], &msc_card_in_lun, USBH_MAX);
#endif			
			{
				MSC_DBG("LUN %d READY", msc_card_in_lun );
				
				lun = 0;
				msc_req.lun = msc_card_in_lun;
					msc_dev->enum_cmd = MSC_GET_CAPACITY;
				
				if( msc_dev->max_lun == 0 )
				{
					if( xTASK_TEST_UNIT_READY[dev->device_id] != NULL )
					{
#if defined (CONFIG_USBH_FREE_RTOS)						
						vTaskDelete(xTASK_TEST_UNIT_READY[dev->device_id]);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
						osThreadTerminate(xTASK_TEST_UNIT_READY[dev->device_id]);
#endif						
						xTASK_TEST_UNIT_READY[dev->device_id] = NULL;
				}
					}
				}
#if defined( CONFIG_SN_GCC_SDK )	
			if (msc_req.size) {
				flush_dcache_area((uint32_t) msc_req.pbuff, msc_req.size);
			}
#endif
			continuous_err_cnt = 0;
			break;
		case MSC_GET_CAPACITY:
			msc_req.lba = 0;
			status = msc_get_capacity((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req);
			if (status == FAIL) {
				MSC_DBG("GET CAPACITY FAIL");
				if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_Halted) {
					msc_dev->err_hdl->pbuff[0] = EP_STALL;
				} else if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_CSW_NOT_VALID) {
					msc_dev->err_hdl->pbuff[0] = CSW_NOT_VALID;
				} else {
					msc_dev->err_hdl->pbuff[0] = TIMEOUT;
				}
				return FAIL;
			} else if (msc_req.csw[12] == 1) {
				MSC_DBG("LUN %d GET CAPACITY CMD FAIL", msc_req.lun);
				msc_dev->err_hdl->pbuff[0] = CMD_FAIL;
				msc_dev->err_hdl->pbuff[1] = msc_req.lun;
				return FAIL;
			} else if (msc_req.csw[12] == 2) {
				MSC_DBG("GET CAPACITY Phase Error");
				msc_dev->err_hdl->pbuff[0] = PHASE_ERR;
				return FAIL;
			} else {
				//if read capacity success,	record device information(ex. device capacity, block len and LBA)
				msc_dev->info[msc_req.lun].capacity = msc_req.capacity;	//lun
				msc_dev->info[msc_req.lun].block_len = msc_req.block_len;//lun
				msc_dev->info[msc_req.lun].lba = msc_req.lba;//lun
				msc_dev->msc_lun_info[msc_req.lun].lun_capacity = msc_req.capacity;
				MSC_DBG("LUN %d CAPACITY = %llu", msc_req.lun, msc_dev->msc_lun_info[msc_req.lun].lun_capacity);
//				if (lun < msc_dev->lun) {
//					msc_dev->enum_cmd = MSC_GET_CAPACITY;
//					lun++;
//				} else {
//					msc_dev->enum_cmd = MSC_READ10;
//					lun = 0;
//				}
					msc_dev->enum_cmd = MSC_READ10;
			}						// end	of else
#if defined( CONFIG_SN_GCC_SDK )				
			if (msc_req.size) {
				flush_dcache_area((uint32_t) msc_req.pbuff, msc_req.size);
			}
#endif
			continuous_err_cnt = 0;
			break;

#if 0
			case MSC_REQUEST_SENSE:
			status = msc_request_sense((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req);
			if((status == FAIL) || (msc_req.csw[12] == 1)) {
				MSC_DBG("REQUEST SENSE FAIL");
				return FAIL;
			}
			//sense_key = msc_req.request_sense[2];
			//sense_key &= 0x0F;
			//msc_cmd = pre_msc_cmd;
			break;
#endif

		case MSC_READ10:
//			for (i = 0; i <= msc_dev->lun; i++) {
//				dev_type = msc_dev->info[i].inquiry[0];
//				dev_type &= 0x1F;
//				if (dev_type == DIRECT_ACCESS_DEVICE) {
//					lun = i;
//				}
//			}
//			msc_req.lun = lun;

			msc_req.block_len = msc_dev->info[msc_req.lun].block_len;	//lun
			for (i = 0; i < MSC_ENUM_READ_TIMES; i++) {
				lba = (i * size) / msc_req.block_len;
				status = msc_read10((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req, size, lba);
				if (status == FAIL) {
					MSC_DBG("LUN %d READ10 FAIL !!", msc_req.lun);
					if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_Halted) {
						msc_dev->err_hdl->pbuff[0] = EP_STALL;
					} else if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_CSW_NOT_VALID) {
						msc_dev->err_hdl->pbuff[0] = CSW_NOT_VALID;
					} else {
						msc_dev->err_hdl->pbuff[0] = TIMEOUT;
					}
					return FAIL;
				}
				else if (msc_req.csw[12] == 1) {
					MSC_DBG("LUN %d READ10 CMD FAIL !!", msc_req.lun);
					msc_dev->err_hdl->pbuff[0] = CMD_FAIL;
					msc_dev->err_hdl->pbuff[1] = msc_req.lun;
					return FAIL;
				}
				else if(msc_req.csw[12] == 2) {
					MSC_DBG("LUN %d READ10 PHASE ERROR !!", msc_req.lun);
					msc_dev->err_hdl->pbuff[0] = PHASE_ERR;
					return FAIL;
				}
				else {
					MSC_DBG("LUN %d READ10 OK", msc_req.lun);
					msc_dev->enum_cmd = MSC_ENUM_DONE;

					msc_dev->working_lun = msc_req.lun;
					msc_dev->msc_lun_info[msc_req.lun].lun_block_len = msc_req.block_len;
//					msc_req_dev.lun = msc_req.lun;
//					msc_req_dev.block_len = msc_req.block_len;					
				}
#if defined( CONFIG_SN_GCC_SDK )					
				if (msc_req.size) {
					flush_dcache_area((uint32_t) msc_req.pbuff, msc_req.size);
				}
#endif
			}
			continuous_err_cnt = 0;
			break;
		case MSC_ENUM_DONE:
			MSC_DBG("MSC_ENUM_DONE");
			break;
		default:
			break;
		}
	}
	while (msc_dev->enum_cmd != MSC_ENUM_DONE);

	return SUCCESS;
}

#if defined (CONFIG_USBH_FREE_RTOS)
void test_unit_ready_task(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void test_unit_ready_task(void const *pvParameters)
#endif
{
	USBH_Device_Structure *dev = NULL;
	MSC_DEVICE_STRUCT 		*msc_dev = NULL;
	MSC_REQ_Struct 				msc_req;
	uint8_t								i = 0;
	uint8_t								card_in_lun = 0;
	uint8_t								device_id = 0;
	uint8_t 							status = SUCCESS;
	
	device_id = *((uint8_t*)pvParameters);	
	dev = (USBH_Device_Structure*) usbh_msc_init(device_id);
	
	while(1) {			
		msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];	
		memset(&msc_req, 0, sizeof(msc_req));
		for(i=0; i<=msc_dev->max_lun; i++)
		{		
			msc_req.lun = i;
			status = msc_test_unit_ready((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req);
			if (status == FAIL) {
				MSC_DBG("LUN %d TEST UNIT READY FAIL", msc_req.lun);
				msc_dev->msc_lun_info[i].lun_status = 0;
				if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_Halted) {
					msc_dev->err_hdl->pbuff[0] = EP_STALL;
				} else if (dev->EP[dev->iEPNUM].status == EHCI_QTD_STATUS_CSW_NOT_VALID) {
					msc_dev->err_hdl->pbuff[0] = CSW_NOT_VALID;
				} else {
					msc_dev->err_hdl->pbuff[0] = TIMEOUT;
				}					
				msc_task_uninit(MSC_RESRC, dev->device_id);
			} else if (msc_req.csw[12] == 1) {				
				if(msc_dev->msc_lun_info[i].lun_status == 1)
				{
					MSC_DBG("LUN %d DEVICE NOT READY", msc_req.lun);
					if( msc_dev->working_lun == i )
						msc_dev->working_lun = 0;
#if defined (CONFIG_USBH_FREE_RTOS)
					xQueueSend(msc_dev->QUEUE_CARD_OUT, &i, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMessagePut(msc_dev->QUEUE_CARD_OUT, &i, 0);
#endif					
				}
				
				msc_dev->msc_lun_info[i].lun_status = 0;				
				continuous_err_cnt = 0;
			} else if (msc_req.csw[12] == 2) {
				msc_dev->msc_lun_info[i].lun_status = 0;
				MSC_DBG("GET LUN %d DEVICE INFORMATION FAIL!!!", msc_req.lun);
				msc_dev->err_hdl->pbuff[0] = PHASE_ERR;				
				//return FAIL;
			} else {
				if(msc_dev->msc_lun_info[i].lun_status == 0)
				{					
					msc_dev->msc_lun_info[i].lun_status = 1;
					card_in_lun = i;
//					msc_req_dev.lun = card_in_lun;
					msc_dev->working_lun =  card_in_lun;
					msc_dev->enum_cmd = MSC_GET_CAPACITY;	
#if defined (CONFIG_USBH_FREE_RTOS)					
					xQueueSend(USBH_QUEUE_READER_CARD_IN[device_id], &card_in_lun, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
					osMessagePut(USBH_QUEUE_READER_CARD_IN[device_id], &card_in_lun, 0);
#endif
					
//					MSC_DBG("SEND LUN %d READY", card_in_lun);
				}
			}
#if defined (CONFIG_USBH_FREE_RTOS)
			vTaskDelay(USBH_50ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osDelay(USBH_50ms);
#endif			
		}
	}
}

void msc_check_dev_sts(USBH_Device_Structure *dev, MSC_DEVICE_STRUCT *msc_dev) {
	uint8_t device_id = 0;
	
	device_id = dev->device_id;
	dev = (USBH_Device_Structure*) usbh_msc_init(device_id);

	if( (dev == 0) && ((msc_dev->status == MSC_CONNECT) ||
										 (msc_dev->status == MSC_ENUM) ||
										 (msc_dev->status == MSC_ACTIVE)) )
	{
		msc_dev->status = MSC_DISCONNECT;
	}	
	else if( (dev != 0) && (msc_dev->status == MSC_DISCONNECT) )
	{
		msc_dev->status = MSC_CONNECT;
	}
}

void msc_enum(uint8_t device_id) {
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	uint8_t								host = USBH1;
#endif
	USBH_Device_Structure *dev = NULL;
	USBH_ERR_HDL_STRUCT 	ERR_HDL;
	USBH_ERR_RLT_STRUCT 	ERR_RLT;
	MSC_DEVICE_STRUCT 		*msc_dev = NULL;
	
	dev = (USBH_Device_Structure*) usbh_msc_init(device_id);
	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	if( device_id < 5 )
		host = USBH1-1;
	else
		host = USBH2-1;
#endif
	
	switch (msc_dev->enum_sm) {
	case MSC_DEVICE_DET_STATE:
		next_stage = CBW_STAGE;
		msc_dev->enum_cmd = MSC_GET_MAX_LUN;
		continuous_err_cnt = 0;
//		memset(&msc_req_dev, 0, sizeof(msc_req_dev));
		msc_check_dev_sts(dev, msc_dev);
		if (msc_dev->status == MSC_DISCONNECT) {
			MSC_DBG("MASS STORAGE DEVICE DISCONNECT!!");
			msc_dev->enum_sm = MSC_DEVICE_DET_STATE;
			msc_task_uninit(MSC_RESRC, device_id);
		} else if (msc_dev->status == MSC_CONNECT) {
			MSC_DBG("MASS STORAGE DEVICE CONNECT!!");
			msc_dev->enum_sm = MSC_DEVICE_ENUM_STATE;
		}
		break;
	case MSC_DEVICE_ENUM_STATE:
		msc_dev->status = MSC_ENUM;
		MSC_DBG("MASS STORAGE BULK ENUMERATION!!");
		if (msc_bulk_enum(dev, msc_dev) == SUCCESS) {
			MSC_DBG("MASS STORAGE ENUMERATION DONE!!");
			msc_dev->enum_sm = MSC_DEVICE_ACTIVE_STATE;
		} else {
			msc_check_dev_sts(dev, msc_dev);
			if (msc_dev->status == MSC_DISCONNECT) {
				msc_dev->enum_sm = MSC_DEVICE_DET_STATE;
			} else {
				msc_dev->enum_sm = MSC_DEVICE_ERR_STATE;
			}
		}
		break;
	case MSC_DEVICE_ERR_STATE:
		MSC_DBG("MASS STORAGE ERROR HANDLE!!");
		memcpy(&ERR_HDL, msc_dev->err_hdl, sizeof(ERR_HDL));
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueSend(USBH_QUEUE_ERR_HDL, &ERR_HDL, 0);
		//wait error handle	finish
		if (xQueueReceive(USBH_QUEUE_ERR_RLT, &ERR_RLT, USBH_MAX)) {
			if(ERR_RLT.pbuff[0] == MSC_RESET) {
				next_stage = CBW_STAGE;
			}
		}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessagePut(USBH_QUEUE_ERR_HDL, &ERR_HDL, 0);
		//wait error handle	finish
		if (osMessageGet(USBH_QUEUE_ERR_RLT, &ERR_RLT, USBH_MAX) == osOK) {
			if(ERR_RLT.pbuff[0] == MSC_RESET) {
				next_stage = CBW_STAGE;
			}
		}
#endif
		
		msc_dev->enum_sm = MSC_DEVICE_ENUM_STATE;
		break;
	case MSC_DEVICE_ACTIVE_STATE:
		msc_check_dev_sts(dev, msc_dev);
		if (msc_dev->status == MSC_DISCONNECT) {
			msc_dev->enum_sm = MSC_DEVICE_DET_STATE;
		} else if (msc_dev->status == MSC_ENUM) {
			msc_dev->status = MSC_ACTIVE;
			
#if defined (CONFIG_USBH_FREE_RTOS)			
			xQueueSend(msc_dev->QUEUE_CARD_IN, &msc_dev->working_lun, 0);
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
			osMessagePut(msc_dev->QUEUE_CARD_IN, &msc_dev->working_lun, 0);
#endif
			
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
			auto_mount_info_usbh[host].usb_device_id = device_id;			
#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(auto_mount_info_usbh[host].xSEM_USBH_PLUG_IN);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osSemaphoreRelease(auto_mount_info_usbh[host].xSEM_USBH_PLUG_IN);
#endif
			MSC_DBG("GIVE xSEM_USBH_PLUG_IN (on HOST %d)!!\n", host+1);
#endif

#if 0
			for(;;) {
				if ( msc_random_test(device_id, 100, 1000000, 1) == FAIL) {
					for(;;) {
#if defined (CONFIG_USBH_FREE_RTOS)
							vTaskDelay(USBH_1000ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
							osDelay(USBH_1000ms);
#endif
					}
				}
			};
#endif	//end of #if 0
			
			MSC_DBG("MASS STORAGE ACTIVE!!");
			MSC_DBG("WAKE UP MSC CLASS APP  !!");
			
			if( USBH_QUEUE_MSC_TEST != NULL )
			{
#if defined (CONFIG_USBH_FREE_RTOS)
				xQueueSend(USBH_QUEUE_MSC_TEST, &device_id, 0);	
#endif
#if defined (CONFIG_USBH_CMSIS_OS)				
				osMessagePut(USBH_QUEUE_MSC_TEST, &device_id, 0);
#endif				
			}
			
			if( msc_dev->max_lun > 0 )
			{
				msc_dev->enum_sm = MSC_DEVICE_ENUM_STATE;
				msc_dev->enum_cmd = MSC_TEST_UNIT_READY;
			}
			else
			{
#if defined (CONFIG_USBH_FREE_RTOS)				
			for (;;) {
				vTaskDelay(USBH_1000ms);
			}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			for (;;) {
				osDelay(USBH_1000ms);
			}
#endif
			}
		}
		break;
	}
	msc_print_enum_info(msc_dev);
}

void msc_print_enum_info(MSC_DEVICE_STRUCT *msc_dev) {
	switch (msc_dev->status) {
	case MSC_DISCONNECT:
		MSC_DBG("MASS STORAGE DEVICE Removed.. ")
		;
		break;
	case MSC_ENUM:
		if (MSG_PRT[0] == 0) {
			MSC_DBG("\r\n >>>bulk enum...... \r\n");
			MSG_PRT[0] = 1;
		}
		break;
	case MSC_ACTIVE:
		if (MSG_PRT[1] == 0) {
			MSC_DBG("MASS STORAGE DEVICE Active.. ");
			MSG_PRT[1] = 1;
		}
		break;
	}
}

#if defined (CONFIG_USBH_FREE_RTOS)
void msc_enum_task(void *pvParameters) {
	USBH_ERR_HDL_STRUCT ERR_HDL;
	uint8_t err_data[8];
	uint8_t device_id;
	uint8_t msc_dev_id = 0;
		
	device_id = *((uint8_t *)pvParameters);
	MSC_DBG("msc_enum_task : device_id = %d\r\n", device_id);	
	if( device_id < 5 )
		xQueueReceive(USBH_QUEUE_WAKEUP_MSC_DRV, &msc_dev_id, portMAX_DELAY);	
	else
	{
#if defined( CONFIG_DUAL_HOST )		
		xQueueReceive(USBH_QUEUE_WAKEUP_MSC_DRV_2, &msc_dev_id, portMAX_DELAY);	
#endif		
	}
	
	MSC_DBG("msc_enum_task : msc_dev_id = %d\r\n", msc_dev_id);
	memset(&MSC_DEV[msc_dev_id], 0, sizeof(MSC_DEVICE_STRUCT));
	memset(&ERR_HDL, 0, sizeof(ERR_HDL));
	memset(&MSG_PRT, 0, sizeof(MSG_PRT));

	MSC_DEV[msc_dev_id].err_hdl = (USBH_ERR_HDL_STRUCT*) &ERR_HDL;
	MSC_DEV[msc_dev_id].enum_sm = MSC_DEVICE_DET_STATE;
	ERR_HDL.pbuff = &err_data[0];
		
	if( MSC_DEV[msc_dev_id].QUEUE_CARD_IN == NULL )
	{
		MSC_DEV[msc_dev_id].QUEUE_CARD_IN	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, sizeof(uint8_t));
	}
	
	if( MSC_DEV[msc_dev_id].QUEUE_CARD_OUT == NULL )
	{
		MSC_DEV[msc_dev_id].QUEUE_CARD_OUT	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, sizeof(uint8_t));
	}
	
	while (1) {
		msc_enum(msc_dev_id);
		vTaskDelay(USBH_10ms);
	}
}

void msc_task_init(uint8_t host, uint8_t device_id)
{
	char 		task_name[32] = {'\0'};
	uint8_t	dev_id = 0;
	
	dev_id = device_id;
	
	sprintf((char *)&task_name, "USBH_MSC_TASK_%d", device_id);
	
//	if(MSC_RESRC != 0){
//		MSC_DBG("MSC Resource has been used");		
//		return;
//	}	
	
	MSC_DBG("MSC TASK INIT HOST %d!!!", host);
//	MSC_RESRC = host;
	
	if( USBH_QUEUE_WAKEUP_MSC_DRV == NULL )
		USBH_QUEUE_WAKEUP_MSC_DRV	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, 4);
	
#if defined( CONFIG_DUAL_HOST )	
	if( USBH_QUEUE_WAKEUP_MSC_DRV_2 == NULL )
		USBH_QUEUE_WAKEUP_MSC_DRV_2	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, 4);
#endif

	if( USBH_QUEUE_MSC_TEST == NULL )
		USBH_QUEUE_MSC_TEST	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, 4);
	
	if( USBH_QUEUE_READER_CARD_IN[device_id] == NULL )
		USBH_QUEUE_READER_CARD_IN[device_id]	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, sizeof(uint8_t));

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	xTaskCreate(
			msc_enum_task,
			(const char* )&task_name[0],
			4096,
			(void*)&dev_id,
			PRIORITY_TASK_DRV_USBH,
			&xTASK_HDL_MSC_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	xTaskCreate(
			msc_enum_task,
			(const char* )&task_name[0],
			4096,
			(void*)&dev_id,
			PRIORITY_TASK_DRV_USBH,
			&xTASK_HDL_MSC_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	xTaskCreate(
			msc_enum_task,
			(const char*)&task_name[0],
			4096,
			(void*)&dev_id,
			250,
			&xTASK_HDL_MSC_DRV[device_id]
	);
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	xTaskCreate(
			msc_enum_task,
			(const char*)&task_name[0],
			512,
			(void*)&dev_id,
			20,
			&xTASK_HDL_MSC_DRV[device_id]
	);
#endif
			
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7320 )
	xTaskCreate(
			msc_enum_task,
			(const char*)&task_name[0],
			512,
			(void*)&dev_id,
			3,
			&xTASK_HDL_MSC_DRV[device_id]
	);
#endif			

#if defined( CONFIG_XILINX_SDK ) && defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
	xTaskCreate(
			msc_enum_task,
			(const char*)&task_name[0],
			4096,
			(void*)&dev_id,
			PRIORITY_TASK_DRV_USBH,
			&xTASK_HDL_MSC_DRV[device_id]
	);
#endif	
}

void msc_task_uninit(uint8_t host, uint8_t device_id)
{
	MSC_DEVICE_STRUCT *msc_dev = NULL;
	
	MSC_RESRC = USBH1;
	
	memset(&MSC_DEV[device_id], 0, sizeof(MSC_DEVICE_STRUCT));
	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];
	msc_dev->status = MSC_DISCONNECT;
	msc_dev->enum_sm = MSC_DEVICE_DET_STATE;
	
	if(USBH_QUEUE_WAKEUP_MSC_DRV != NULL) {
		vQueueDelete(USBH_QUEUE_WAKEUP_MSC_DRV);
		USBH_QUEUE_WAKEUP_MSC_DRV = NULL;
	}

	if(USBH_QUEUE_MSC_TEST != NULL) {
		vQueueDelete(USBH_QUEUE_MSC_TEST);
		USBH_QUEUE_MSC_TEST = NULL;
	}

	if(USBH_QUEUE_READER_CARD_IN[device_id] != NULL) {
		vQueueDelete(USBH_QUEUE_READER_CARD_IN[device_id]);
		USBH_QUEUE_READER_CARD_IN[device_id] = NULL;
	}
	
	if(msc_dev->QUEUE_CARD_IN != NULL) {
		vQueueDelete(msc_dev->QUEUE_CARD_IN);
		msc_dev->QUEUE_CARD_IN = NULL;
	}
	
	if(msc_dev->QUEUE_CARD_OUT != NULL) {
		vQueueDelete(msc_dev->QUEUE_CARD_OUT);
		msc_dev->QUEUE_CARD_OUT = NULL;
	}

	if(xTASK_TEST_UNIT_READY[device_id] != NULL) {
		vTaskDelete(xTASK_TEST_UNIT_READY[device_id]);
		xTASK_TEST_UNIT_READY[device_id] = NULL;
	}

	if(xTASK_HDL_MSC_DRV[device_id] != NULL) {
		vTaskDelete(xTASK_HDL_MSC_DRV[device_id]);
		xTASK_HDL_MSC_DRV[device_id] = NULL;
	}
	
//#if defined( CONFIG_PLATFORM_SN53510 ) && defined(CONFIG_Others_APP)	
	if(xTASK_USBH_MSC_TEST != NULL) {
		vTaskDelete(xTASK_USBH_MSC_TEST);
		xTASK_USBH_MSC_TEST = NULL;
	}
//	xSemaphoreGive(USBH_SEM_TEST_STOP);
//#endif
	
#if defined(CONFIG_CLI_MSC)			
	if(xTASK_CLI_MSC_RW_TEST[device_id] != NULL) {
		vTaskDelete(xTASK_CLI_MSC_RW_TEST[device_id]);
		xTASK_CLI_MSC_RW_TEST[device_id] = NULL;
	}	
	
	if(xTASK_CLI_MSC_TEST != NULL) {
		vTaskDelete(xTASK_CLI_MSC_TEST);
		xTASK_CLI_MSC_TEST = NULL;
	}
#endif

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
	xSemaphoreGive(auto_mount_info_usbh[host-1].xSEM_USBH_PLUG_OUT);
	MSC_DBG("GIVE xSEM_USBH_PLUG_OUT (on HOST %d)!!\n", host);
#endif
		
	MSC_DBG("MSC TASK DESTORY !");
}
#endif	//end of #if defined (CONFIG_USBH_FREE_RTOS)

#if defined (CONFIG_USBH_CMSIS_OS)
void msc_enum_task(void const *pvParameters) {
	USBH_ERR_HDL_STRUCT ERR_HDL;
	uint8_t err_data[8];
	uint8_t device_id = 0;
	
	osMessageGet(USBH_QUEUE_WAKEUP_MSC_DRV, &device_id, USBH_MAX);
	MSC_DBG("MSC ENUM TASK, device_id = %d!!", device_id);
	
	memset(&MSC_DEV[device_id], 0, sizeof(MSC_DEVICE_STRUCT));
	memset(&ERR_HDL, 0, sizeof(ERR_HDL));
	memset(&MSG_PRT, 0, sizeof(MSG_PRT));

	MSC_DEV[device_id].err_hdl = (USBH_ERR_HDL_STRUCT*) &ERR_HDL;
	MSC_DEV[device_id].enum_sm = MSC_DEVICE_DET_STATE;
	ERR_HDL.pbuff = &err_data[0];
	
	if( MSC_DEV[device_id].QUEUE_CARD_IN  == NULL )
	{
		osMessageQDef(QUEUE_CARD_IN, USBH_MSC_DATA_QUEUE_SIZE, uint8_t);
		MSC_DEV[device_id].QUEUE_CARD_IN  = osMessageCreate(osMessageQ(QUEUE_CARD_IN), NULL);
	}
	
	if( MSC_DEV[device_id].QUEUE_CARD_OUT  == NULL )
	{
		osMessageQDef(QUEUE_CARD_OUT, USBH_MSC_DATA_QUEUE_SIZE, uint8_t);
		MSC_DEV[device_id].QUEUE_CARD_OUT  = osMessageCreate(osMessageQ(QUEUE_CARD_OUT), NULL);
	}
	
	while (1) {
		msc_enum(device_id);
		osDelay(USBH_10ms);
	}
}

void msc_task_init(uint8_t host, uint8_t device_id)
{
	char task_name[256] = {'\0'};
		
	sprintf((char *)&task_name, "%s%d", "USBH_MSC_TASK", device_id);
	
//	if(MSC_RESRC != 0){
//		MSC_DBG("MSC Resource has been used");		
//		return;
//	}	
	
	MSC_DBG("MSC TASK INIT HOST %d!!!", host);
//	MSC_RESRC = host;
	
	if(USBH_QUEUE_WAKEUP_MSC_DRV == NULL)
	{
		osMessageQDef(USBH_QUEUE_WAKEUP_MSC_DRV, USBH_MSC_DATA_QUEUE_SIZE, uint32_t);
		USBH_QUEUE_WAKEUP_MSC_DRV = osMessageCreate(osMessageQ(USBH_QUEUE_WAKEUP_MSC_DRV), NULL);
	}

#if defined( CONFIG_DUAL_HOST )
	if(USBH_QUEUE_WAKEUP_MSC_DRV_2 == NULL)
	{
		osMessageQDef(USBH_QUEUE_WAKEUP_MSC_DRV_2, USBH_MSC_DATA_QUEUE_SIZE, uint32_t);
		USBH_QUEUE_WAKEUP_MSC_DRV_2 = osMessageCreate(osMessageQ(USBH_QUEUE_WAKEUP_MSC_DRV_2), NULL);
	}
#endif
	
	if(USBH_QUEUE_MSC_TEST == NULL){
		osMessageQDef(USBH_QUEUE_MSC_TEST, USBH_MSC_DATA_QUEUE_SIZE, uint32_t);
		USBH_QUEUE_MSC_TEST = osMessageCreate(osMessageQ(USBH_QUEUE_MSC_TEST), NULL);
	}
	
	if( USBH_QUEUE_READER_CARD_IN[device_id] == NULL )
	{
		osMessageQDef(USBH_QUEUE_READER_CARD_IN, USBH_MSC_DATA_QUEUE_SIZE, uint8_t);
		USBH_QUEUE_READER_CARD_IN[device_id] = osMessageCreate(osMessageQ(USBH_QUEUE_READER_CARD_IN), NULL);
	}

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
	osThreadDef(task_name, msc_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_MSC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_MSC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_MSC_DRV fail\n");
#endif

#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9868X )
	osThreadDef(task_name, msc_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_MSC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_MSC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_MSC_DRV fail\n");
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	osThreadDef(task_name, msc_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_MSC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_MSC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_MSC_DRV fail\n");
#endif

#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_SN7300 )
	osThreadDef(task_name, msc_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 512);
	xTASK_HDL_MSC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_MSC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_MSC_DRV fail\n");
#endif

#if defined( CONFIG_XILINX_SDK ) && defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
	osThreadDef(task_name, msc_enum_task, THREAD_PRIO_USBH_PROCESS, 1, 4096);
	xTASK_HDL_MSC_DRV[device_id] = osThreadCreate(osThread(task_name), &device_id);
	if( xTASK_HDL_MSC_DRV[device_id] == NULL )
		printf("Create xTASK_HDL_MSC_DRV fail\n");
#endif
}

void msc_task_uninit(uint8_t host, uint8_t device_id) {

	MSC_DEVICE_STRUCT *msc_dev = NULL;
	
//	if(MSC_RESRC != host) {
//		MSC_DBG("MSC Resource has been used");			
//		return;
//	}
	MSC_RESRC = USBH1;
	
	memset(&MSC_DEV[device_id], 0, sizeof(MSC_DEVICE_STRUCT));
	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];
	msc_dev->status = MSC_DISCONNECT;
	msc_dev->enum_sm = MSC_DEVICE_DET_STATE;
	
	if(USBH_QUEUE_WAKEUP_MSC_DRV != NULL) {
		osMutexDelete(USBH_QUEUE_WAKEUP_MSC_DRV);
		USBH_QUEUE_WAKEUP_MSC_DRV = NULL;
	}
	
	if(USBH_QUEUE_MSC_TEST != NULL) {
		osMutexDelete(USBH_QUEUE_MSC_TEST);
		USBH_QUEUE_MSC_TEST = NULL;
	}

	if(USBH_QUEUE_READER_CARD_IN[device_id] != NULL) {
		osMutexDelete(USBH_QUEUE_READER_CARD_IN[device_id]);
		USBH_QUEUE_READER_CARD_IN[device_id] = NULL;
	}

	if(msc_dev->QUEUE_CARD_IN != NULL) {
		osMutexDelete(msc_dev->QUEUE_CARD_IN);
		msc_dev->QUEUE_CARD_IN = NULL;
	}
	
	if(msc_dev->QUEUE_CARD_OUT != NULL) {
		osMutexDelete(msc_dev->QUEUE_CARD_OUT);
		msc_dev->QUEUE_CARD_OUT = NULL;
	}

	if(xTASK_TEST_UNIT_READY[device_id] != NULL) {
		osThreadTerminate(xTASK_TEST_UNIT_READY[device_id]);
		xTASK_TEST_UNIT_READY[device_id] = NULL;
	}
		
	if(xTASK_HDL_MSC_DRV[device_id] != NULL) {
		osThreadTerminate(xTASK_HDL_MSC_DRV[device_id]);
		xTASK_HDL_MSC_DRV[device_id] = NULL;
	}
	
	if(xTASK_USBH_MSC_TEST != NULL) {
		osThreadTerminate(xTASK_USBH_MSC_TEST);
		xTASK_USBH_MSC_TEST = NULL;
	}
	
#if defined(CONFIG_CLI_MSC)			
	if(xTASK_CLI_MSC_RW_TEST[device_id] != NULL) {
		osThreadTerminate(xTASK_CLI_MSC_RW_TEST[device_id]);
		xTASK_CLI_MSC_RW_TEST[device_id] = NULL;
	}
	
	if(xTASK_CLI_MSC_TEST != NULL) {
		osThreadTerminate(xTASK_CLI_MSC_TEST);
		xTASK_CLI_MSC_TEST = NULL;
	}
#endif

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
  auto_mount_info_usbh[host-1].usb_device_id = device_id;
	osSemaphoreRelease(auto_mount_info_usbh[host-1].xSEM_USBH_PLUG_OUT);
#endif
	MSC_DBG("GIVE xSEM_USBH_PLUG_OUT (on HOST %d)!!\n", host);
	MSC_DBG("MSC TASK DESTORY !");
}
#endif

uint8_t msc_ready(uint8_t device_id)
{
	uint8_t								curr_lun = 0;
	USBH_Device_Structure *dev = NULL;
	MSC_DEVICE_STRUCT *msc_dev = NULL;

	dev = (USBH_Device_Structure*) usbh_msc_init(device_id);
	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];
	
	curr_lun = msc_dev->working_lun;

	if(msc_dev->max_lun == 0)
	{
	if ((dev != 0) && (msc_dev->status == MSC_ACTIVE)) {
		return SUCCESS;
	} else {
		return FAIL;
	}
}
	else
	{		
		if ((dev != 0) && (msc_dev->msc_lun_info[curr_lun].lun_status == 1)) {
			return SUCCESS;
		} else {
			return FAIL;
		}
	}
}

uint64_t msc_capacity(uint8_t device_id) {
	USBH_Device_Structure *dev = NULL;
	MSC_DEVICE_STRUCT *msc_dev = NULL;
	uint8_t lun = 0;
	uint64_t capacity = 0;

	dev = (USBH_Device_Structure*) usbh_msc_init(device_id);
	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];
	if (dev == 0)
		return 0;
	for (lun = 0; lun <= msc_dev->max_lun; lun++) {
		uint8_t dev_type = 0;
		dev_type = msc_dev->info[lun].inquiry[0];
		dev_type &= 0x1F;
		if (dev_type == DIRECT_ACCESS_DEVICE) {
			capacity = msc_dev->info[lun].capacity;
		}
	}
	return capacity;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 * 
 * @{
 */
/** \fn	uint8_t msc_read(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba)
 * \brief MSC Class API	fucntion to read data from a mass storage device
 * 
 * \param dev target USB device structure pointer
 * 
 * \param pdata_buf pointer to store read data
 * 
 * \param size data size
 * 
 * \param lba logical block address to read data
 * 
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_read(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba)
{
	uint8_t 					status = SUCCESS;
	uint8_t						curr_lun = 0;
	uint32_t 					block_len = 0;
	MSC_REQ_Struct msc_req;
	MSC_DEVICE_STRUCT *msc_dev = NULL;

	if (dev == NULL) return FAIL;

	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[dev->device_id];
	curr_lun = msc_dev->working_lun;
	block_len = msc_dev->msc_lun_info[curr_lun].lun_block_len;	
//	if (msc_req_dev.block_len == 0) return FAIL;
	if( block_len == 0 ) return FAIL;

	memset(&msc_req, 0, sizeof(msc_req));
	
	msc_req.pbuff = (uint8_t*) pdata_buf;
//	msc_req.lun = msc_req_dev.lun;
//	msc_req.block_len = msc_req_dev.block_len;
	msc_req.lun = curr_lun;
	msc_req.block_len = block_len;

	status = msc_read10((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) &msc_req, size, lba);

	return status;
}

/** \defgroup MSC_FUN MSC functions
 * \ingroup MSC_MODULE
 * 
 * @{
 */
/** \fn	uint8_t msc_write(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba)
 * \brief MSC Class API	fucntion to write data to a mass storage device
 * 
 * \param dev target USB device structure pointer
 * 
 * \param pdata_buf pointer to store ready-to-write data
 * 
 * \param size data size
 * 
 * \param lba logical block address to write data
 * 
 * \return status, SUCCESS or FAIL
 */
/** @} */
uint8_t msc_write(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba)
{
	uint8_t 					status = SUCCESS;
	uint8_t						curr_lun = 0;
	uint32_t 					block_len = 0;
	MSC_REQ_Struct msc_req;
	MSC_DEVICE_STRUCT *msc_dev = NULL;

	if (dev == NULL) return FAIL;

	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[dev->device_id];
	curr_lun = msc_dev->working_lun;
	block_len = msc_dev->msc_lun_info[curr_lun].lun_block_len;
//	if (msc_req_dev.block_len == 0) return FAIL;
	if( block_len == 0 ) return FAIL;

	memset(&msc_req, 0, sizeof(MSC_REQ_Struct));
	msc_req.pbuff = (uint8_t*) pdata_buf;
//	msc_req.lun = msc_req_dev.lun;
//	msc_req.block_len = msc_req_dev.block_len;
	msc_req.lun = curr_lun;
	msc_req.block_len = block_len;

	status = msc_write10((USBH_Device_Structure*) dev, (MSC_REQ_Struct*) &msc_req, size, lba);

	return status;
}

//#define msc_compare

uint8_t msc_random_test(uint8_t device_id, uint32_t time, uint32_t sector, uint32_t compare) {
	USBH_Device_Structure *dev = NULL;
	uint32_t addr = 0;
	uint32_t i = 1;
	uint32_t j = 0;
	uint32_t size;
	size_t pattern_size;
	uint8_t *pattern_buf;
	uint8_t *compare_buf;

	dev = (USBH_Device_Structure*) usbh_msc_init(device_id);
	if (dev == NULL) {
		MSC_DBG("device is not exist!");
		return FAIL;
	}

	size = ((dev->BUFF.size / 2) & 0xFFFFF000);
	pattern_size = size;

	pattern_buf = (uint8_t*) (dev->BUFF.ptr);
	pattern_buf = (uint8_t*) (((uint32_t) pattern_buf & 0xFFFFFC00) + 0x400); //do 1k alignment
	compare_buf = (uint8_t*) (((uint32_t) (pattern_buf)) + size);

	while (i <= time) {
		MSC_DBG("Staring %d times , Port#%d Copy to Port#%d Total %d Sector !!", i, device_id, device_id, sector);
		while ((addr + 1) <= (sector - 1)) {
			if(compare == 1) {
				memset(compare_buf, 0, pattern_size);

				for (j = 0; j < pattern_size; j++) {
					pattern_buf[j] = rand() % 256;
				}
			}		
			if (msc_write(dev, (uint8_t*) pattern_buf, pattern_size, addr) == FAIL) {
				MSC_DBG("MSC Write 10 FAIL");
				goto ERR;
			}
			if (msc_read(dev, (uint8_t*) compare_buf, pattern_size, addr) == FAIL) {
				MSC_DBG("MSC Read 10 FAIL");
				goto ERR;
			}

			if(compare == 1) {
				if (memcmp(pattern_buf, compare_buf, pattern_size) != 0) {
					MSC_DBG("MSC Data Compare FAIL");
					goto ERR;
				}
			}
			MSC_INFO(".");
			addr += (pattern_size / 512);
		}
		MSC_INFO("\n");
		i++;
		addr = 0;

		if(compare == 1) {
			memset(&pattern_buf[0], 0x00, sizeof(pattern_buf));
			memset(&compare_buf[0], 0x00, sizeof(compare_buf));
		}
	}
	return SUCCESS;
	ERR: return FAIL;
}

#if 0
uint8_t msc_rw_test(USBH_Device_Structure *dev, uint16_t sec_count,uint32_t times) {
	uint8_t *setup_data_buff = NULL;
	uint8_t *msc_data_buff = NULL;
	MSC_REQ_Struct msc_req;
	USBH_CX_XFR_REQ_Struct cx_req;
	uint32_t lba = 0,count = 0;
	uint8_t msc_sm = MSC_GET_MAX_LUN;
	uint8_t status = SUCCESS;

	memset(&cx_req, 0, sizeof(cx_req));
	memset(&msc_req, 0, sizeof(msc_req));

#if defined( CONFIG_SN_GCC_SDK )
	setup_data_buff = pvPortMalloc(64,GFP_DMA,MODULE_DRI_USBH);
	msc_data_buff = pvPortMalloc(64*1024,GFP_DMA,MODULE_DRI_USBH);
#endif 	
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	setup_data_buff = pvPortMalloc(64);
	msc_data_buff = pvPortMalloc(64*1024);
#endif 

	do {
		switch (msc_sm) {
			case MSC_GET_MAX_LUN:
			cx_req.CMD = USBH_CX_CMD_CLASS_Read;
			cx_req.SIZE = 0x01;
			cx_req.pBUFF = (uint32_t*)setup_data_buff;
			cx_req.CLASS[0] = 0xA1;
			cx_req.CLASS[1] = 0xFE;
			cx_req.CLASS[2] = 0x00;
			cx_req.CLASS[3] = 0x00;
			cx_req.CLASS[4] = 0x00;
			cx_req.CLASS[5] = 0x00;
			status = usbh_cx_xfr((USBH_Device_Structure*)dev,(USBH_CX_XFR_REQ_Struct*)&cx_req);
			if(status == FAIL) {
				MSC_DBG("ENUM FAIL,GOING TO ERROR HANDLING");
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(setup_data_buff);
				vPortFree(msc_data_buff);
#endif
				return FAIL;
			} else {
				msc_sm = MSC_GET_INQUIRY;
			}
			break;
			case MSC_GET_INQUIRY:
			msc_req.cmd = CBW_BULK_INQUIRY;
			msc_req.size = 36;
			msc_req.pbuff = msc_data_buff;
			status = msc_transfer((USBH_Device_Structure*)dev,(MSC_REQ_Struct*)&msc_req);
			if( (status == FAIL) || (msc_req.csw[12] == 1) ) {
				MSC_DBG("\r\nMSC ENUM FAIL\r\n ");
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(setup_data_buff);
				vPortFree(msc_data_buff);
#endif
				return FAIL;
			} else {
				memcpy(&msc_req.inquiry,&msc_req.pbuff,msc_req.size);
				msc_sm = MSC_TEST_UNIT_READY;
			}
			break;
			case MSC_TEST_UNIT_READY:
			msc_req.cmd = CBW_BULK_TEST_UNIT_READY;
			msc_req.size = 0;
			status = msc_transfer((USBH_Device_Structure*)dev,(MSC_REQ_Struct*)&msc_req);
			if(status == FAIL ) {
				MSC_DBG("\r\nMSC ENUM FAIL\r\n ");
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(setup_data_buff);
				vPortFree(msc_data_buff);
#endif
				return FAIL;
			} else {
				if(msc_req.csw[12] == 1) {
					vTaskDelay(USBH_10ms);
					msc_sm = MSC_TEST_UNIT_READY;
				} else {
					msc_sm = MSC_GET_CAPACITY;
				}
			}
			break;
			case MSC_GET_CAPACITY:
			msc_req.cmd = CBW_BULK_READ_CAPACITY;
			msc_req.size = 8;
			msc_req.pbuff = msc_data_buff;
			status = msc_transfer((USBH_Device_Structure*)dev,(MSC_REQ_Struct*)&msc_req);
			if( (status == FAIL) || (msc_req.csw[12] == 1) ) {
				MSC_DBG("\r\nMSC ENUM FAIL\r\n ");
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(setup_data_buff);
				vPortFree(msc_data_buff);
#endif
				return FAIL;
			} else {
				msc_req.capacity = msc_req.pbuff[0]<<24 |msc_req.pbuff[1]<<16|msc_req.pbuff[2]<<8|msc_req.pbuff[3];
				msc_req.block_len = msc_req.pbuff[4]<<24 |msc_req.pbuff[5]<<16|msc_req.pbuff[6]<<8|msc_req.pbuff[7];
				msc_sm = MSC_READ10;
			}
			break;
			case MSC_READ10:
			msc_req.cmd = CBW_BULK_READ10;
			msc_req.size = 1024*16;
			msc_req.lba = lba;
			msc_req.pbuff = msc_data_buff;
			status = msc_transfer((USBH_Device_Structure*)dev,(MSC_REQ_Struct*)&msc_req);
			if( (status == FAIL) || (msc_req.csw[12] == 1) ) {
				MSC_DBG("\r\nMSC ENUM FAIL\r\n ");
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(setup_data_buff);
				vPortFree(msc_data_buff);
#endif
				return FAIL;
			} else {
				msc_sm = MSC_WRITE10;
			}
			break;
			case MSC_WRITE10:
			msc_req.cmd = CBW_BULK_WRITE10;
			msc_req.size = 1024*16;
			msc_req.lba = lba;
			msc_req.pbuff = msc_data_buff;
			status = msc_transfer((USBH_Device_Structure*)dev,(MSC_REQ_Struct*)&msc_req);
			if( (status == FAIL) || (msc_req.csw[12] == 1) ) {
				MSC_DBG("\r\nMSC ENUM FAIL\r\n ");
#if defined (CONFIG_USBH_FREE_RTOS)
				vPortFree(setup_data_buff);
				vPortFree(msc_data_buff);
#endif
				return FAIL;
			} else {
				lba += sec_count;
				if(lba>=msc_req.capacity) {
					count ++;
				}
				if(count >= times) return SUCCESS;
				msc_sm = MSC_READ10;
			}
			break;
		}
	}while(1);

	return SUCCESS;
}
#endif	//end of #if 0

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
automount_info_usbh* get_automount_info_usbh(uint8_t host) 
{
	return &auto_mount_info_usbh[host-1];
}
#endif

void msc_set_bulk_timeout(uint32_t timeoutMsec)
{
	bulk_timeout_value = timeoutMsec;
	MSC_INFO("msc_set_bulk_timeout = %d\n", bulk_timeout_value);
}

MSC_DEVICE_STRUCT* get_msc_device_info(uint8_t device_id) 
{
	MSC_DEVICE_STRUCT *msc_dev = NULL;

	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[device_id];
	
	return msc_dev;
}

#endif	//end of #if defined (CONFIG_MODULE_USB_MSC_CLASS)

