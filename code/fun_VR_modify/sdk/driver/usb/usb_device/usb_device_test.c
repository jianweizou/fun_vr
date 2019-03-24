#include "sonix_config.h"
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb_device_test.h"
#include "usb_device.h"

#define VT100_ResetTerm         "\x1B\x63"
#define tResetTerm              printf(VT100_ResetTerm)

//------------------------------------------------------------------------------
extern usbd_class_mode_t 		snx_usbd_mode;
extern usbd_class_submode_t     snx_usbd_submode;
//------------------------------------------------------------------------------

void usbd_msc_test(void)
{
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
	snx_usbd_mode = USBD_MODE_MSC;
	snx_usbd_submode= USBD_SUBMODE_NONE;
	usbd_drv_task_init();
#endif    
}

void usbd_uvc_iso_test(void)
{
#if defined( CONFIG_MODULE_USBD_UVC_CLASS )
	snx_usbd_mode = USBD_MODE_UVC;
	snx_usbd_submode= USBD_SUBMODE_ISO;
	usbd_drv_task_init();
#endif    
}
void usbd_uvc_bulk_test(void)
{
#if defined( CONFIG_MODULE_USBD_UVC_CLASS )
	snx_usbd_mode = USBD_MODE_UVC;
	snx_usbd_submode= USBD_SUBMODE_BULK;
	usbd_drv_task_init();
#endif    
}
void usbd_uac_test(void)
{
#if defined( CONFIG_MODULE_USBD_UAC_CLASS )
	snx_usbd_mode = USBD_MODE_UAC;
	snx_usbd_submode= USBD_SUBMODE_NONE;
	usbd_drv_task_init();
#endif    
}

void usbd_hid_test(void)
{
#if defined( CONFIG_MODULE_USBD_HID_CLASS )
	snx_usbd_mode = USBD_MODE_HID;
	snx_usbd_submode= USBD_SUBMODE_NONE;
	usbd_drv_task_init();
#endif    
}
//------------------------------------------------------------------------------

/* 
//move to cli_usbd.c
void USBD_Test(void)
{
	uint8_t  uwTestId = 2;

	while(uwTestId)
	{
		tResetTerm;
		printf("\r========================================\n");
		printf("\r           USB Device Test Item List        \n");
		printf("\r----------------------------------------\n");
		printf("\r0) ESC\n");
		printf("\r1) MSC Test\n");
		printf("\r2) UVC ISO Test\n");
		printf("\r3) UVC Bulk Test\n");
		printf("\r4) UAC Test\n");
		printf("\r5) HID Test\n");
		printf("\r========================================\n");
		printf("\rCommand> ");

		//uwTestId = uart0_scanf();
		//scanf("%d\n", &uwTestId);
		
		switch (uwTestId)
		{
		case 0:
			break;
		case 1:
			usbd_msc_test();
			break;
		case 2:
			usbd_uvc_iso_test();
			break;
		case 3:
			usbd_uvc_bulk_test();
			break;
		case 4:
			usbd_uac_test();
			break;
		case 5:
			usbd_hid_test();
			break;
		default:
			printf("Bad command, try again.\n");
			break;
		}
		return;
	}
}
*/
