/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WiFiMain.h
	\brief		Wi-Fi Header file
	\author		Hann Chiu          
	\version	2
	\date		2015/01/27
	\copyright	Copyright (C) 2014 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef __WIFIMAIN_H
#define __WIFIMAIN_H

#include "FreeRTOS.h"
#include <stdint.h>
#include "wifi_api.h"
#include "semphr.h"

void wifi_init(void);
void wifi_Set_AP(void);
#endif

