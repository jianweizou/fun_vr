/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file
	\brief
	\author
	\version
	\date
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _USB_DEVICE_TEST_H_
#define _USB_DEVICE_TEST_H_

#include "sonix_config.h"

//void USBD_Test(void);
void usbd_msc_test(void);
void usbd_uvc_iso_test(void);
void usbd_uvc_bulk_test(void);
void usbd_uac_test(void);
void usbd_hid_test(void);
//------------------------------------------------------------------------------
#endif
