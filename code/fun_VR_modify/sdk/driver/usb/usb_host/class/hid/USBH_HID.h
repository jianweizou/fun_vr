#ifndef	HID__H
#define	HID__H

#include "sonix_config.h"

#if defined( CONFIG_SN_GCC_SDK )
#include <generated/snx_sdk_conf.h>

#if defined( CONFIG_MODULE_USB_HID_DEBUG )
	#define HID_DBG(format, args...) print_msg("\nHID_DBG:"format, ##args);
#else
	#define HID_DBG(args...)
#endif
	#define HID_INFO(format, args...) print_msg_queue(format, ##args);
#endif 

#if defined( CONFIG_SN_KEIL_SDK )
#include "stdio.h"
#if defined( CONFIG_MODULE_USB_HID_DEBUG )
	#define HID_DBG(format, args...) printf("\nHID_DBG:"format, ##args);
#else
	#define HID_DBG(args...)
#endif
	#define HID_INFO(format, args...) printf(format, ##args)
#endif

#if defined( CONFIG_XILINX_SDK )
#include "xil_printf.h"
#if defined( CONFIG_MODULE_USB_HID_DEBUG )
	#define HID_DBG(format, args...) xil_printf("\nHID_DBG:"format, ##args);
#else
	#define HID_DBG(args...)
#endif
	#define HID_INFO(format, args...) xil_printf(format, ##args)
#endif

// HID Plug & play MACRO
#define	HID_EXIST				1
#define	HID_NOT_EXIST			0

#define	HID_CONNECT				HID_EXIST
#define	HID_DISCONNECT			HID_NOT_EXIST
#define	HID_ENUM				2
#define	HID_ACTIVE				3

#define REPORT_SIZE 128

#define	USBH_HID_JOYSTICK_DATA_SIZE		8

#define	USBH_HID_DATA_QUEUE_SIZE		10


typedef struct{
	//uint8_t		joystickH;
	//uint8_t		joystickV;
	uint8_t		Button[8];
}HID_JOYSTICK_MAP_STRUCT;

typedef struct {
	#define	MOUSE_KEYMAX 16
	uint32_t	HID_NUM;
	uint8_t		u8bHIDValue[MOUSE_KEYMAX];
	uint8_t		u8bByteNum;
	uint8_t		u8bDevicetype;
	uint8_t		X_RESOLUTION;
	uint8_t		Y_RESOLUTION;	
}sHID_INFO_Structure; //HID interface to APP

typedef struct{
	uint8_t		Left[2];		// Left[0] = BYTE location  ; Left[1] = BIT location 
	uint8_t		Right[2];
	uint8_t		Scroll_press[2];
	uint8_t		X[3];			// X[0] = BYTE location ; X[1] = Bit location ; X[2] = bit count 
	uint8_t		Y[3];
	uint8_t		Scroll[3];	
	uint8_t		RESERVE;
} HID_MOUSE_MAP_STRUCT;

typedef	struct {
	USBH_ERR_HDL_STRUCT			*err_hdl;				//!< USB Error Handle Structure
	uint8_t						status;					//!< Mass Storage Device Status
	uint32_t					u8HID_REPORT_DESC[REPORT_SIZE];
	uint8_t						dev_type;				// 1: mouse 2: keyboard 3 : joystick
	HID_MOUSE_MAP_STRUCT		HID_MOUSE_MAP;
	sHID_INFO_Structure			sHID_INFO;
	HID_JOYSTICK_MAP_STRUCT		HID_JOYSTICK_MAP;
	uint8_t						INT_DATA_BUFF[8];
}USBH_HID_CLASS_Structure;

#if defined (CONFIG_USBH_FREE_RTOS)
//extern SemaphoreHandle_t USBH_SEM_WAKEUP_HID_DRV;
extern QueueHandle_t		usbh_queue_hid_drv;
extern xTaskHandle xTASK_HDL_HID_DRV[USBH_MAX_PORT*2];

void hid_app_task(void *pvParameters);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
extern osMessageQId		usbh_queue_hid_drv;
extern osThreadId			xTASK_HDL_HID_DRV[USBH_MAX_PORT*2];

void hid_app_task(void const *pvParameters);
#endif


extern uint8_t hid_start_action(USBH_Device_Structure *dev);
extern uint8_t hid_stop_action(USBH_Device_Structure *dev);

uint8_t	hid_get_report_desc(USBH_Device_Structure *dev);
extern void hid_task_init(uint8_t device_id);
extern void hid_task_uninit(uint8_t device_id);

#endif

