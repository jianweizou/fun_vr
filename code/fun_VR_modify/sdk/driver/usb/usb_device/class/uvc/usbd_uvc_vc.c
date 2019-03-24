/*
*	File: usbd_uvc_vc.c
*	Version: 1.0
*	Author: chkuo
*	Date: 2015-10-19
*	Descriptions: SONiX USB UVC Device Video Capture functions
*/
#include "sonix_config.h"

#if defined( CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN ) && defined( CONFIG_MODULE_USBD_UVC_CLASS )

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "usbd_uvc.h"
#include "usbd_uvc_vc.h"

xSemaphoreHandle usbd_uvc_image_start;


int g_image_stop			= 0;
static const uint8_t *g_imgbuf		= NULL;
static uint32_t g_imgsize		= 0;
static uint32_t g_uvc_vc_task_state	= 0;

static const uint8_t* jpeg_data[2];
static uint32_t jpeg_data_size[2];

void uvc_cmd_stop_preview(void);
void set_dummy_pattern(uint32_t jpeg_img_width, uint32_t jpeg_img_height);

void mdelay(uint32_t value)
{
	uint32_t i;
	for (i=0; i<6000*value; i++) {
		__nop();
	}
}

void usbd_uvc_vc_task(void *pvParameters)
{
	static uint32_t jpeg_cnt = 0;	
	g_image_stop = true;
	g_uvc_vc_task_state = 1;

	USBD_UVC_PRINT("%s \r\n ",__FUNCTION__);

	while(g_uvc_vc_task_state) {
		xSemaphoreTake(usbd_uvc_image_start, portMAX_DELAY);
		jpeg_cnt = 0;
		while(!g_image_stop) {
			//vTaskDelay(5);									//for test dummy pattern ,about  30fps
			jpeg_cnt++;
			g_imgsize = jpeg_data_size[jpeg_cnt%2];
			g_imgbuf = jpeg_data[jpeg_cnt%2];
			//usbd_uvc_drv_send_image((uint32_t*)g_imgbuf,g_imgsize);
			usbd_uvc_drv_send_image((uint32_t*)g_imgbuf, g_imgsize, UVC_IMG_WITHOUT_HEADER, UVC_XFR_COPY_MEMORY);
			USBD_UVC_PRINT("g_imgsize = %08d\r\n ",g_imgsize);

		}
	}
	vSemaphoreDelete(usbd_uvc_image_start);
	vTaskDelete(NULL);
}

void usbd_uvc_vc_task_stop(void)
{
	uvc_cmd_stop_preview();
	//g_uvc_vc_task_state = 0;
}

void uvc_vc_set_image_info(struct preview_image_info *img_info)
{
	uint32_t jpeg_img_width = 0;
	uint32_t jpeg_img_height = 0;	
	USBD_UVC_PRINT("%s \r\n ",__FUNCTION__);
	jpeg_img_width = img_info->width;
	jpeg_img_height = img_info->height;
	set_dummy_pattern(jpeg_img_width, jpeg_img_height);
	g_image_stop = false;
	xSemaphoreGive(usbd_uvc_image_start);
}

void uvc_cmd_stop_preview(void)
{
	USBD_UVC_PRINT("%s \r\n ",__FUNCTION__);
	g_image_stop = true;
}

void uvc_get_image(uint8_t *buf,unsigned *len)
{
	USBD_UVC_PRINT("%s \r\n ",__FUNCTION__);
	*(len) += g_imgsize;
	memcpy(buf,g_imgbuf,g_imgsize);
}

uint32_t uvc_get_image_length(void) {
	USBD_UVC_PRINT("%s \r\n ",__FUNCTION__);
	return  g_imgsize;
}

void uvc_get_image_slice(uint8_t *buf,unsigned off,signed len) {
	USBD_UVC_PRINT("%s \r\n ",__FUNCTION__);
	memcpy(buf,&g_imgbuf[off],len);
}


void set_dummy_pattern(uint32_t jpeg_img_width, uint32_t jpeg_img_height)
{
	
	if(jpeg_img_width == UVC_VGA_WIDTH && jpeg_img_height == UVC_VGA_HEIGHT) {
		jpeg_data[0]		= JPEG_VGA_1;
		jpeg_data[1]		= JPEG_VGA_2;
		jpeg_data_size[0]	= sizeof JPEG_VGA_1;
		jpeg_data_size[1]	= sizeof JPEG_VGA_2;
	}
	else if(jpeg_img_width == UVC_HD_WIDTH && jpeg_img_height == UVC_HD_HEIGHT) {
		jpeg_data[0]		= JPEG_HD_1;
		jpeg_data[1]		= JPEG_HD_2;
		jpeg_data_size[0]	= sizeof JPEG_HD_1;
		jpeg_data_size[1]	= sizeof JPEG_HD_2;
	}
	else if(jpeg_img_width == UVC_FHD_WIDTH && jpeg_img_height == UVC_FHD_HEIGHT) {
		jpeg_data[0]		= JPEG_FHD_1;
		jpeg_data[1]		= JPEG_FHD_2;
		jpeg_data_size[0]	= sizeof JPEG_FHD_1;
		jpeg_data_size[1]	= sizeof JPEG_FHD_2;
	}	
}

int uvc_vc_init(void)
{
	usbd_uvc_image_start = xSemaphoreCreateCounting(1,0);
	if(usbd_uvc_image_start==NULL)
		return pdFAIL;
	
	USBD_UVC_PRINT("create usbd_uvc_vc_task\r\n");
	if (pdPASS != xTaskCreate(usbd_uvc_vc_task, "usbd_uvc_vc_task", 4096,(void*) NULL,250,NULL)) {
		USBD_UVC_PRINT("Could not usbd_uvc_vc_task\r\r\n");
		return pdFAIL;
	}	
	return pdPASS;
}

#endif //#if defined( CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN ) && defined( CONFIG_MODULE_USBD_UVC_CLASS )
