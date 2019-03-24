/**
 * @file
 * this	is USBH	header file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USBH__H
#define	USBH__H	  

#include "sonix_config.h"

#if defined( __cplusplus )
extern "C"{
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
#include "_510PF.h"

#define THREAD_PRIO_USBH_PROCESS				(osPriorityNormal)

#endif

#if defined( CONFIG_SN_GCC_SDK )
#include <generated/snx_sdk_conf.h> 
#include <nonstdlib.h>
#endif

#include "stdbool.h"

#include "USBH_CORE.h"
#include "EHCI_HCD.h"

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
#include "USBH_ERR_HDL.h"
#endif

#define USBH1	1	
#define USBH2	2

/** \defgroup USBH_DEBUG Debug definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 

#if defined( CONFIG_SN_GCC_SDK )
#if defined( CONFIG_MODULE_USBH_DEBUG )
	#define USBH_DBG(format, args...) print_msg("\nUSBH_DBG:"format, ##args);
#else
	#define USBH_DBG(args...)
#endif
	#define USBH_INFO(format, args...) print_msg_queue(format, ##args)
#endif 
	
	
#if defined( CONFIG_SN_KEIL_SDK )
#include "stdio.h"
#if defined( CONFIG_MODULE_USBH_DEBUG )
	#define USBH_DBG(format, args...) printf("\r\nUSBH_DBG:"format, ##args);	
#else
	#define USBH_DBG(args...)
#endif
	#define USBH_INFO(format, args...) printf(format, ##args)
#endif


#if defined( CONFIG_XILINX_SDK )
#include "xil_printf.h"
#if defined( CONFIG_MODULE_USBH_DEBUG )
	#define USBH_DBG(format, args...) xil_printf("\r\nUSBH_DBG:"format, ##args);
#else
	#define USBH_DBG(args...)
#endif
	#define USBH_INFO(format, args...) xil_printf(format, ##args)
#endif

// Call Back 
extern	USBH_PLUG_Struct	plug[];

#if defined (CONFIG_MODULE_USB_HID_CLASS)
// Call Back for HID INT Data
extern USBH_HID_INT_Struct	hid_int[];
#endif


/** @} */

/** \defgroup USBH_MODULE USBH Driver modules
 * 
 * @{
 */

/** @} */

/** \defgroup MSC_MODULE Mass Storage Class modules
 * 
 * @{
 */

/** @} */


/** \defgroup WIFI_MODULE WIFI Class modules
 * 
 * @{
 */

/** @} */

#define USBH_UVC_BK_STREAM_BUF_SIZE		(MAX_QTD_PER_QH-1)*20*1024 //(MAX_QTD-1)*20k

typedef	enum
{
	USBH_UNKNOW_CLASS		=	0,
	USBH_WIFI_CLASS			= 1,
	USBH_MSC_CLASS			= 2,
	USBH_UVC_BULK_CLASS	= 4,
	USBH_UVC_ISO_CLASS	= 8,
	USBH_HID_CLASS			= 16,
	USBH_HUB_CLASS			=	32

}USBH_CLASS_DRV_INDEX;

typedef	enum
{
	USBH_UVC_NONE		= 0,
	USBH_UVC_YUV,
	USBH_UVC_MJPEG,
	USBH_UVC_H264

}USBH_UVC_FRAME_FMT;

typedef struct _USBH_UVC_FRAME_INFO{
    USBH_UVC_FRAME_FMT	fmt[8];
    uint16_t	ulWidth[8];
		uint16_t	ulHeight[8];
 }USBH_UVC_FRAME_INFO;

/** \defgroup STATUS_DEF Status	definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#define	SUCCESS		0	/**< success */
#define	FAIL		1	/**< fail */
#define	NYET		2	/**< not yet */
#define	TIME_OUT	3	/**< timeout */
#define DEV_NOT_EXIST	4		/**< device not exist */
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
#define RECOVERY_SUCCESS	5	/**< reset recovery success */
#define RECOVERY_FAIL		6	/**< reset recovery fail */
#endif
/** @} */

/** \defgroup CAPACITY_DEF Max capacity	definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
#define	USBH_QUEUE_SIZE		10	/**< Max ENDP support */
#endif

//#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN9866X )
#if defined( CONFIG_PLATFORM_SN9866X )
#define	USBH_MAX_PORT		5	/**< Max Port support */
#endif

//#if defined( CONFIG_SN_KEIL_SDK  ) && defined( CONFIG_PLATFORM_ST53510 )
#if defined( CONFIG_PLATFORM_ST53510 ) || defined( CONFIG_PLATFORM_SN9868X )
#define	USBH_MAX_PORT		5	/**< Max Port support */
#endif
 
//#if defined( CONFIG_SN_KEIL_SDK  ) && defined( CONFIG_PLATFORM_SN7300 )
#if defined( CONFIG_PLATFORM_SN7300 )
#define	USBH_MAX_PORT		1	/**< Max Port support */
#endif 

#if defined( CONFIG_PLATFORM_SN7320 )
#if defined (CONFIG_MODULE_USB_UVC_SRAM)
#define	USBH_MAX_PORT		1	/**< Max Port support */
#else
#define	USBH_MAX_PORT		5	/**< Max Port support */
#endif
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
#define	USBH_2ms	2/portTICK_RATE_MS
#define	USBH_5ms	5/portTICK_RATE_MS
#define	USBH_10ms	10/portTICK_RATE_MS
#define	USBH_20ms	20/portTICK_RATE_MS
#define	USBH_30ms	30/portTICK_RATE_MS
#define	USBH_40ms	40/portTICK_RATE_MS
#define	USBH_50ms	50/portTICK_RATE_MS
#define	USBH_60ms	60/portTICK_RATE_MS
#define	USBH_70ms	70/portTICK_RATE_MS
#define	USBH_80ms	80/portTICK_RATE_MS
#define	USBH_90ms	90/portTICK_RATE_MS
#define	USBH_100ms	100/portTICK_RATE_MS
#define	USBH_120ms	120/portTICK_RATE_MS
#define	USBH_200ms	200/portTICK_RATE_MS
#define	USBH_500ms	500/portTICK_RATE_MS
#define	USBH_1000ms	1000/portTICK_RATE_MS
#define	USBH_2000ms	2000/portTICK_RATE_MS
#define	USBH_3000ms	3000/portTICK_RATE_MS
#define	USBH_5000ms	5000/portTICK_RATE_MS
#define	USBH_10000ms	10000/portTICK_RATE_MS
#define	USBH_MAX	0xFFFFFFFF
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK  ) && defined( CONFIG_PLATFORM_ST53510 )
#define	USBH_2ms	2
#define	USBH_5ms	5
#define	USBH_10ms	10
#define	USBH_20ms	20
#define	USBH_30ms	30
#define	USBH_40ms	40
#define	USBH_50ms	50
#define	USBH_60ms	60
#define	USBH_70ms	70
#define	USBH_80ms	80
#define	USBH_90ms	90
#define	USBH_100ms	100
#define	USBH_120ms	120
#define	USBH_200ms	200
#define	USBH_500ms	500
#define	USBH_1000ms	1000
#define	USBH_2000ms	2000
#define	USBH_3000ms	3000
#define	USBH_5000ms	5000
#define	USBH_10000ms	10000
#define	USBH_MAX	0xFFFFFFFF
#endif
#endif

/** \defgroup TIMEOUT_DEF Timeout value	definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#define	USBH_CX_TIMEOUT		USBH_1000ms	/**< CX Transfer timeout value */
#define	USBH_BK_OUT_TIMEOUT	USBH_1000ms	/**< BK OUT	Transfer timeout value */
#define	USBH_BK_IN_TIMEOUT	USBH_MAX	/**< BK IN Transfer	timeout	value */
#define	USBH_WIFI_BK_IN_TIMEOUT	USBH_MAX	/**< BK IN Transfer	timeout	value */
#define	USBH_INT_OUT_TIMEOUT	USBH_500ms
#define	USBH_INT_IN_TIMEOUT	USBH_MAX


/** @} */

#define	BIT0	0x01
#define	BIT1	0x02
#define	BIT2	0x04
#define	BIT3	0x08
#define	BIT4	0x10
#define	BIT5	0x20
#define	BIT6	0x40
#define	BIT7	0x80
#define	BIT8	0x0100
#define	BIT9	0x0200
#define	BIT10	0x0400
#define	BIT11	0x0800
#define	BIT12	0x1000
#define	BIT13	0x2000
#define	BIT14	0x4000
#define	BIT15	0x8000
#define	BIT16	0x010000
#define	BIT17	0x020000
#define	BIT18	0x040000
#define	BIT19	0x080000
#define	BIT20	0x100000
#define	BIT21	0x200000
#define	BIT22	0x400000
#define	BIT23	0x800000
#define	BIT24	0x01000000
#define	BIT25	0x02000000
#define	BIT26	0x04000000
#define	BIT27	0x08000000
#define	BIT28	0x10000000
#define	BIT29	0x20000000
#define	BIT30	0x40000000
#define	BIT31	0x80000000

#if defined( CONFIG_RECORD ) || defined( CONFIG_AUTOMOUNT )	
#if defined (CONFIG_USBH_FREE_RTOS)	
typedef struct _automount_info_usb
{
	uint8_t						usb_device_id;
  SemaphoreHandle_t xSEM_USBH_PLUG_IN;
  SemaphoreHandle_t xSEM_USBH_PLUG_OUT;
 }automount_info_usbh;
#endif
 
#if defined (CONFIG_USBH_CMSIS_OS)
typedef struct _automount_info_usb
{
	uint8_t						usb_device_id;
	osSemaphoreId			xSEM_USBH_PLUG_IN;
	osSemaphoreId			xSEM_USBH_PLUG_OUT;
}automount_info_usbh;
#endif
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
extern	SemaphoreHandle_t	USBH_SEM_FRAME_LIST_ROLLOVER;
extern	SemaphoreHandle_t	USBH_SEM_ASYNC_ADV;
#if !defined (CONFIG_WIFI_SDK)
extern	SemaphoreHandle_t	USBH_SEM_WAKEUP_WIFI;
#endif // end of if !defined (CONFIG_WIFI_SDK)
extern	SemaphoreHandle_t	USBH_SEM_WAKEUP_AUTO_BKIN;
extern	SemaphoreHandle_t	USBH_SEM_WAKEUP_ERR_HDL;
extern	SemaphoreHandle_t	USBH_SEM_AUTO_BKIN_CNT;
extern	SemaphoreHandle_t	USBH_SEM_PACKET_CNT_MUTEX;

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
extern	QueueHandle_t	USBH_QUEUE_ERR_HDL;	
extern	QueueHandle_t	USBH_QUEUE_ERR_RLT;
#endif
extern	TaskHandle_t	xTASK_HDL_USBH_ENUM;
extern	TaskHandle_t	xTASK_HDL_USB_XFR;
extern	TaskHandle_t	xTASK_HDL_USBH_ERRHDL;

#if defined( CONFIG_DUAL_HOST )
extern SemaphoreHandle_t	USBH_SEM_FRAME_LIST_ROLLOVER_2;

#if !defined (CONFIG_WIFI_SDK)	
extern SemaphoreHandle_t	USBH_SEM_WAKEUP_WIFI_2;
#endif
#endif

#endif
 
#if defined (CONFIG_USBH_CMSIS_OS)
extern	osSemaphoreId	USBH_SEM_FRAME_LIST_ROLLOVER;
extern	osSemaphoreId	USBH_SEM_ASYNC_ADV;
#if !defined (CONFIG_WIFI_SDK)
extern	osSemaphoreId	USBH_SEM_WAKEUP_WIFI;
#endif
extern	osSemaphoreId	USBH_SEM_WAKEUP_AUTO_BKIN;
extern	osSemaphoreId	USBH_SEM_WAKEUP_ERR_HDL;
extern	osSemaphoreId	USBH_SEM_AUTO_BKIN_CNT;
extern	osMutexId	USBH_SEM_PACKET_CNT_MUTEX;

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
extern	osMessageQId	USBH_QUEUE_ERR_HDL;
extern	osMessageQId	USBH_QUEUE_ERR_RLT;
#endif
extern	osThreadId	xTASK_HDL_USBH_ENUM;
extern	osThreadId	xTASK_HDL_USB_XFR;
extern	osThreadId	xTASK_HDL_USBH_ERRHDL;

#if defined( CONFIG_DUAL_HOST )
extern osSemaphoreId	USBH_SEM_FRAME_LIST_ROLLOVER_2;

#if !defined (CONFIG_WIFI_SDK)		
extern osSemaphoreId	USBH_SEM_WAKEUP_WIFI_2;
#endif
#endif

#endif
 
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
extern uint32_t dev_enum_time;
#endif

extern void usbh_init(void);
extern void usbh_uninit(void); 
extern void usbh_freertos_init(void);
extern void usbh_freertos_uninit(void);
extern void usbh_intr_enable(void);

#if defined (CONFIG_USBH_FREE_RTOS)
extern void usbh_enum_task(void	*pvParameters);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
extern uint32_t RF_USBH_Buf_PTR;
#endif

extern void usbh_enum_task(void	const *pvParameters);
extern uint32_t ulUSBH_GetBufSize(bool bIsEnableWIFI, bool bIsEnableDualHost, uint8_t stream_num, USBH_UVC_FRAME_INFO uvc_frame_info);
extern void USBH_BufInit(uint32_t ulBufferStrAddr);
#endif


#if defined( CONFIG_DUAL_HOST )
#if defined (CONFIG_USBH_FREE_RTOS)
extern	SemaphoreHandle_t	USBH_SEM_ASYNC_ADV_2;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
extern	osSemaphoreId	USBH_SEM_ASYNC_ADV_2;
#endif

extern void usbh_init_2(void);
extern void usbh_uninit_2(void);
extern void usbh_freertos_init_2(void);
extern void usbh_freertos_uninit_2(void);
extern void usbh_intr_enable_2(void);
#if defined (CONFIG_USBH_FREE_RTOS)
extern void usbh_enum_task_2(void *pvParameters);
extern void usbh_auto_bkin_task(void *pvParameters);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
extern void usbh_enum_task_2(void const *pvParameters);
extern void usbh_auto_bkin_task(void const *pvParameters);
#endif
#endif // End of if defined( CONFIG_DUAL_HOST )

extern void USBH_XFR_TASK(void *pvParameters);
extern void USBH_ERR_HDL_TASK(void *pvParameters);
#if !defined (CONFIG_WIFI_SDK)
extern uint32_t	usbh_wifi_init(uint8_t ID);
#endif // end of if !defined (CONFIG_WIFI_SDK)
extern uint32_t	usbh_msc_init(uint8_t ID);
extern uint32_t	usbh_uvc_init(uint8_t ID);
extern uint32_t	usbh_hub_init(uint8_t ID);
extern uint32_t usbh_hid_init(uint8_t ID);

//extern void usbh_enum_task(void	*pvParameters);
extern void usbh_demo_task(void	*pvParameters);
extern void usbh_auto_bkin_init(void);
extern void usbh_auto_bkin_uninit(void);

extern void usbh_plug_cb_reg(uint32_t CLASS_TYPE ,void (*in_callback)(void),void (*out_callback)(void));
extern uint8_t usbh_plug_in_cb(uint32_t CLASS_TYPE );
extern uint8_t usbh_plug_out_cb(uint32_t CLASS_TYPE );

extern void usbh_HID_INT_cb_reg(uint32_t CLASS_TYPE, void (*callback)(uint32_t, uint8_t*));
extern void usbh_HID_INT_cb_UNreg(uint32_t CLASS_TYPE, uint8_t dev_id);
extern uint8_t usbh_HID_INT_IN_cb(uint32_t CLASS_TYPE, uint8_t dev_id, uint8_t *INT_DATA_BUFF);

extern uint8_t usbh_root_suspend(void);
extern uint8_t usbh_root_resume(void);

/** @} */

#if defined( __cplusplus )
}
#endif

#if defined (CONFIG_WIFI_SDK)
#define LIST_HEAD_INIT(name) { &(name), &(name) }
 
struct usbh_driver
{
	uint8_t *name;
	uint16_t idVendor;
	uint16_t idProduct;
	uint32_t (*probe)(USBH_Device_Structure *device);
	void (*disconnect)(void);
	struct list_head list;
#if defined (CONFIG_USBH_FREE_RTOS)
	SemaphoreHandle_t list_lock;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osSemaphoreId list_lock;
#endif
};

extern uint32_t usbh_driver_register(struct usbh_driver *driver);
extern void usbh_driver_deregister(struct usbh_driver *driver);
extern uint32_t usbh_device_add(USBH_Device_Structure *usbh_dev);
extern void usbh_device_remove(USBH_Device_Structure *usbh_dev);
#endif // end of if defined (CONFIG_WIFI_SDK)


#endif //USBH__H
