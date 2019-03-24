/*
*	File: usbd_hid_vc.c
*	Version: 1.0
*	Author: erick chang
*	Date: 2018-4-10
*	Descriptions: SONiX USB HID Device functions
*/
#include "sonix_config.h"

#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN) && defined( CONFIG_MODULE_USBD_HID_CLASS )
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "usbd_hid.h"
#include "usbd_hid_vc.h"
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif


extern xSemaphoreHandle usbd_hid_xfr_start;

static uint32_t g_hid_dummy_xfr_stop = 1;
static uint32_t g_hid_task_state = 0;
static uint8_t hid_key_buf[9];


void usbd_hid_set_key(uint8_t *buf, char c)
{
	buf[0] = 0x01;
	memset(buf+1, 0, 2+HID_KB_REPORT_SIZE);
	if(c==0x00)	{				//button up
		return;
	}else if(c>=0x41 && c<=0x5a)	{	//A-Z
		buf[1] = 0x02;	//set left shift
		buf[3] = c-0x41+4;
	}else if(c>=0x61 && c<=0x7a)	{  //a-z
		buf[3] = c-0x61+4;
	}else if(c==0x2e)	{  //dot 
		buf[3] = 0x37;
	}else if(c==0x20)	{  //spacebar 
		buf[3] = 0x2c;
	}else if(c==0x0a)	{  //newline 
		buf[3] = 0x28;
	}
}
void usbd_hid_vc_task( void *pvParaeters )
{
	
	int idx=0;
	g_hid_dummy_xfr_stop = true;
	g_hid_task_state = 1;
	HID_DUMMY_DATA[sizeof(HID_DUMMY_DATA)-1] = 0x0a;	//set newline
   
	while(g_hid_task_state){
		if(!usbd_hid_xfr_start)
			continue;
		xSemaphoreTake(usbd_hid_xfr_start, portMAX_DELAY);        
		while(!g_hid_dummy_xfr_stop){	
			
			for(idx=0;idx<sizeof(HID_DUMMY_DATA);idx++)	{
				usbd_hid_set_key(hid_key_buf, HID_DUMMY_DATA[idx]);
				usbd_drv_hid_cmd_rdata(hid_key_buf, sizeof(hid_key_buf));
				//vTaskDelay(3);	//fixme: remove this cause ioc/isp interrupt sometimes can't generate in DMA Multi mode 
				usbd_hid_set_key(hid_key_buf, 0);
				usbd_drv_hid_cmd_rdata(hid_key_buf, sizeof(hid_key_buf));
				vTaskDelay(1000);  //send one char each second
				//vTaskDelay(3);	//fixme: remove this cause ioc/isp interrupt sometimes can't generate in DMA Multi mode
			}
		}
	}

	vSemaphoreDelete(usbd_hid_xfr_start);
	vTaskDelete(NULL);
}

void usbd_hid_task_start(void)
{
	g_hid_dummy_xfr_stop = 0;
}

void usbd_hid_task_stop(void)
{
	g_hid_dummy_xfr_stop = 1;
}
#endif
