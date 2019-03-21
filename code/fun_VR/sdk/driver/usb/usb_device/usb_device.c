/**
* @file
* this is usb device driver file
* usb_device.c
* @author IP2/Luka
*/
#include "sonix_config.h"

#include "FreeRTOS.h"
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include "usb_device.h"
#include "snx_udc.h"
/* include usb device class header */
#if defined( CONFIG_MODULE_USBD_UVC_CLASS )
#include "usbd_uvc.h"
#endif
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
#include "usbd_msc.h"
#endif
#if defined( CONFIG_MODULE_USBD_HID_CLASS )
#include "usbd_hid.h"
#endif
#if defined( CONFIG_MODULE_USBD_UAC_CLASS )
#include "usbd_uac.h"
#endif

xTaskHandle xSnxUSBDProcess;
xTaskHandle xSnxUSBDEP0Process;
//static int usbd_maintask_run = 0;
usbd_class_mode_t		snx_usbd_mode;
usbd_class_submode_t	snx_usbd_submode;


void USBDClassDrvInit(void)
{
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
	if (snx_usbd_mode == USBD_MODE_MSC) {
		usbd_msc_init();
	} else 
#endif

#if defined( CONFIG_MODULE_USBD_UVC_CLASS )
    if (snx_usbd_mode == USBD_MODE_UVC) {
		usbd_uvc_init();
	} else 
#endif

#if defined( CONFIG_MODULE_USBD_HID_CLASS )
    if (snx_usbd_mode == USBD_MODE_HID) {
		usbd_hid_init();
	} else 
#endif

#if defined( CONFIG_MODULE_USBD_UAC_CLASS )
    if (snx_usbd_mode == USBD_MODE_UAC) {
		usbd_uac_init();
	} else
#endif    
    {
        printf("%s:usbd mode %d not support\r\n",__func__, snx_usbd_mode);
    }
}

void USBDClassDrvUnInit(void)
{
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
	if (snx_usbd_mode == USBD_MODE_MSC) {
		usbd_msc_uninit();
	}else 
#endif

#if defined( CONFIG_MODULE_USBD_UVC_CLASS )
	if (snx_usbd_mode == USBD_MODE_UVC) {
		usbd_uvc_uninit();
	}else 
#endif

#if defined( CONFIG_MODULE_USBD_HID_CLASS )
	if (snx_usbd_mode == USBD_MODE_HID) {
		usbd_hid_uninit();
	}else 
#endif

#if defined( CONFIG_MODULE_USBD_UAC_CLASS )
	if (snx_usbd_mode == USBD_MODE_UAC) {
		usbd_uac_uninit();
	}
#endif	
    {
        ;
    }	
}

void USBDevice_init(void)
{
	/* Initialize udc */
	udc_init();

	USBDClassDrvInit();
	
	// enable interrupt
	NVIC_SetPriority( USBDEV_IRQn, 5 );
	NVIC_EnableIRQ(USBDEV_IRQn);
//	NVIC_SetPriority(USBDEV_IRQn, 1);

	if (pdPASS != xTaskCreate(usbd_process, "usbd_process task", 1024, "usbd_process", 250, &xSnxUSBDProcess)) {
		USBD_PRINT("Could not create usbd_process task !!!\r\n");
	}


	if (pdPASS != xTaskCreate(usbd_ep0_process, "usbd_process usbd_ep0_process", 1024, "usbd_ep0_process", 250, &xSnxUSBDEP0Process)) {
		USBD_PRINT("Could not create usbd_ep0_process task !!!\r\n");
	}			
}
/**
* @brief interface function - USB device uninit
*/
void USBDevice_uninit(void)
{
	USBDClassDrvUnInit();
	if (xSnxUSBDProcess != NULL) {
		vTaskDelete(xSnxUSBDProcess);
	}
	xSnxUSBDProcess = NULL;

	if(xSnxUSBDEP0Process != NULL){
		vTaskDelete(xSnxUSBDEP0Process);
	}
	xSnxUSBDEP0Process = NULL;
	
	udc_disable();
}

/**
* @brief interface function -  USB device function entry point
*/
void usbd_drv_task_init(void)
{
	USBDevice_init();
}


void usbd_set_class_mode(usbd_class_mode_t mode, usbd_class_submode_t sub_mode)
{
	snx_usbd_mode = mode;
	snx_usbd_submode = sub_mode;
}

/**
* @brief interface function -  USB device function exit point
*/
void usbd_drv_task_uninit(void)
{
#if defined(CONFIG_PLATFORM_SN98660)
	usbd_maintask_run = 0;
#else
    USBDevice_uninit();
#endif
}

#if defined(CONFIG_PLATFORM_SN98660)
int usbd_get_class_mode(void)
{
	return snx_usbd_mode;
}

void usbd_set_class_mode(int mode)
{
	int ClassReady = USBD_hotplug_GetClassReady();

	if ((usbd_class_mode_t)mode == USBD_MODE_UNKNOWN) {
		USBD_PRINT("Unknown Device Class mode\n");
		return;
	}

	if (ClassReady == 1) {
		USBDevice_uninit();
		snx_udc_fw_reconnection();
	}

	snx_usbd_mode = (usbd_class_mode_t)mode;

	if (ClassReady == 1) {
		USBDevice_init();
	}
}

inline void usbd_set_ext_hotplug_state(int state)
{
	USBD_hotplug_ext_set_state(state);
}
#endif
