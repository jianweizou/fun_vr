/*
*	File: usbd_hid_vc.h
*	Version: 1.0
*	Author: erick chang
*	Date: 2018-4-10
*	Descriptions: SONiX USB HID Device header file.
*/
#ifndef __USBD_HID_VC_H__
#define __USBD_HID_VC_H__

#include "FreeRTOS.h"


#ifdef __cplusplus
extern "C" {
#endif

// Variable


// Functions
extern void usbd_hid_task_start(void);
extern void usbd_hid_task_stop(void);
	
#ifdef __cplusplus
}
#endif

//static uint8_t HID_DUMMY_DATA[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
static uint8_t HID_DUMMY_DATA[] = "String For Test... ";

#endif /*__USBD_HID_VC_H__*/
