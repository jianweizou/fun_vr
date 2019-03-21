/*
*   File: usbd_uvc_vc.c
*   Version: 1.0
*   Author: chkuo
*   Date: 2015-10-19
*   Descriptions: SONiX USB UVC Device Video Capture functions
*/
#include "sonix_config.h"
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN) && defined( CONFIG_MODULE_USBD_UAC_CLASS )

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "usbd_uac.h"
#include "usbd_uac_desc.h"
#include "usbd_uac_vc.h"
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif

typedef enum {
    AC_STATE_START=0,
    AC_STATE_STOP,
    AC_STATE_FINISH
}uac_ac_state_t;

typedef struct _usbd_uac_ac_info{
    xTaskHandle in_task;
    xTaskHandle out_task;
    xSemaphoreHandle in_start_sema;
    xSemaphoreHandle out_start_sema;
    unsigned char in_state;
    unsigned char out_state;
}usbd_uac_ac_info_t;

usbd_uac_ac_info_t g_ac_info={NULL, NULL, NULL, NULL, AC_STATE_FINISH, AC_STATE_FINISH};

void usbd_uac_ac_in_task( void *pvParaeters )
{
    unsigned char *data_buf = NULL;
    unsigned int data_len = 0, index = 0;

    //data_len = sizeof(Audio_dummy_data_16res);
    //data_buf = (uint8_t*)&Audio_dummy_data_16res;
    data_len = sizeof(pattern_6ch_24bit);
    data_buf = (uint8_t*)&pattern_6ch_24bit;

    while(1){
        xSemaphoreTake(g_ac_info.in_start_sema, portMAX_DELAY);
        USBD_UAC_PRINT("audio record start\n");
		index = 0;
        while(g_ac_info.in_state==AC_STATE_START){
			index ++;
			data_buf[0] = index&0xff;
			data_buf[1] = (index>>8)&0xff;
			data_buf[2] = (index>>16)&0xff;
            usbd_uac_drv_do_record(data_buf, data_len);
        }
        USBD_UAC_PRINT("audio record stop\n");
        g_ac_info.in_state=AC_STATE_FINISH;
    }

    //vTaskDelete(NULL);
}

#if USBD_UAC_ISOC_OUT_SUPPORT

#if 0
/*
typedef struct _err_info{
	uint16_t count;
	uint16_t ofx;
	unsigned char pt[6];
	unsigned char bf[6];
}err_info_t;
err_info_t err_list[20];

*/
int g_ac_out_cmp_start = 0;
static int data_cmp(unsigned char *buf1, unsigned char *buf2, int size)
{
#if 1
	return memcmp(buf1, buf2, size);
#else
	int i = 0;
	for(i=0;i<size;i++)	{
		if(buf1[i] != buf2[i])
			break;
	}
	return i;
#endif

}
static int pattern_cmp(unsigned char *data_buf, int data_size)
{
	static unsigned char *ptr = (unsigned char*)Audio_dummy_data_16res;
	static int count = 0, start = 1, discard_count = 0;
	unsigned char *end = (unsigned char*)Audio_dummy_data_16res + sizeof(Audio_dummy_data_16res), *skip_start;
	//static int err=0;
	int /*cmp_size, i, */skip_packet = 0, packet_size = 0xc0;
	unsigned char *buf = data_buf, miss=0;
	int size = data_size;
	if(size%packet_size)
		printf("buf size incorrect 0x%x\r\n", size);
	
	if(start)	{

		int i, shift;
		for(i=12;i<30;i++)
		{
			shift = i*packet_size;
			if((*((int*)(buf+shift)))==0 && (*((int*)(buf+shift+4)))==0 && (*((int*)(buf+shift+8)))==0 && (*((int*)(buf+shift+12)))==0 )
				continue;
			else	{
				printf("ofx = %x\r\n", shift);
				break;
			}
		}		
		buf += shift;
		size -= shift;
		start = 0;
	}

	while(size)	{
		if(ptr>=end)	{
			count++ ;
			Audio_dummy_data_16res[0] = count&0xffff;
			ptr = (unsigned char*)Audio_dummy_data_16res;
		}	
		if(data_cmp(ptr, buf, 8)==0)	{
			if(skip_packet)	{
				printf("loss %d packets(%x, %x)\r\n", skip_packet, count, (int)skip_start-(int)Audio_dummy_data_16res);
				skip_packet = 0;
			}
			buf += packet_size;
			size -= packet_size;	
			ptr += packet_size;	
		}
		else	{
			if(skip_packet==0)	{
				skip_start = ptr;
				printf("cmp %x %x fail\r\n", buf, ptr);
				
				if(data_cmp(ptr+packet_size, buf+packet_size, 8)==0)	{
					discard_count++;
					printf("discard packets %d (%x, %x)\r\n", discard_count, count, (int)skip_start-(int)Audio_dummy_data_16res);
					buf += packet_size;
					size -= packet_size;
					ptr += packet_size;					
					continue;
				}	
			}
			skip_packet ++;
			if(skip_packet>=10)	{
				printf("data miss(%x, %x)\r\n", count, (int)skip_start-(int)Audio_dummy_data_16res);
				miss = 1;
				//printf("1111111\r\n");
				break;
			}	
			ptr += packet_size;	
		}	
		

	}	

	return 1;
}
#endif

void usbd_uac_ac_out_task( void *pvParaeters )
{
    unsigned char *data_buf = NULL;
    unsigned int data_len = 0, receive_data_len = 0;
	unsigned int CurTime, lastTime, data_count = 0;//, ofx = 0;

    data_len = UAC_OUT_MAX_PAYLOAD_SIZE;
    data_buf = (uint8_t*)usbd_malloc(data_len);	    

    while(1){
        xSemaphoreTake(g_ac_info.out_start_sema, portMAX_DELAY); 
        USBD_UAC_PRINT("audio play start\n");
#if 1
		lastTime = xTaskGetTickCount();	
		CurTime = lastTime;
		data_count = 0;
#endif		
        while(g_ac_info.out_state==AC_STATE_START){			
            receive_data_len = usbd_uac_drv_do_play(data_buf, data_len);	
		//if(g_ac_out_cmp_start)	
		//	pattern_cmp(data_buf, receive_data_len);
#if 1
			CurTime = xTaskGetTickCount();		
			data_count += receive_data_len;		
			if (CurTime > lastTime ) {			
				if ((CurTime - lastTime) > (60000 / portTICK_RATE_MS)) {				
					printf("byte per sec = %d (%d / %d)\r\n", data_count*portTICK_RATE_MS*10/(CurTime - lastTime)*100, data_count, CurTime - lastTime);				
					data_count = 0;				
					lastTime = CurTime;			
				}		
			} 
			else {			
				lastTime = xTaskGetTickCount();			
				CurTime = lastTime;		
			}			
#endif
        }
        USBD_UAC_PRINT("audio play stop\n");
        g_ac_info.out_state=AC_STATE_FINISH;
    }

    //vTaskDelete(NULL);
}
#endif
void usbd_uac_ac_in_task_start(void)
{
    g_ac_info.in_state = AC_STATE_START;
    if(g_ac_info.in_start_sema)
        xSemaphoreGive(g_ac_info.in_start_sema);
}

void usbd_uac_ac_in_task_stop(void)
{
    xSemaphoreTake(g_ac_info.in_start_sema, 0);
    g_ac_info.in_state = AC_STATE_STOP;
    //while(g_ac_info.in_state!=AC_STATE_FINISH)
    //   vTaskDelay((1+portTICK_RATE_MS-1)/portTICK_RATE_MS );
        
}



void usbd_uac_ac_out_task_start(void)
{
#if USBD_UAC_ISOC_OUT_SUPPORT
    g_ac_info.out_state = AC_STATE_START;
    if(g_ac_info.out_start_sema)
        xSemaphoreGive(g_ac_info.out_start_sema);  
#endif	
}

void usbd_uac_ac_out_task_stop(void)
{
#if USBD_UAC_ISOC_OUT_SUPPORT
    xSemaphoreTake(g_ac_info.out_start_sema, 0); 
    g_ac_info.out_state = AC_STATE_STOP;
#endif	
}

int usbd_uac_ac_init(void)
{
    if(g_ac_info.in_start_sema) {
        USBD_UAC_PRINT_ERR("Could not set uac ac init twice\r\n");
        return pdFAIL;
    }

    g_ac_info.in_state = AC_STATE_FINISH;
    g_ac_info.out_state = AC_STATE_FINISH;
    
    if((g_ac_info.in_start_sema=xSemaphoreCreateCounting(1,0)) == 0)    {
        USBD_UAC_PRINT_ERR("Could not create in_start_sema\r\n");
        goto fail;
    }    
     
    if (pdPASS != xTaskCreate(usbd_uac_ac_in_task, "usbd_uac_ac_in_task", 256, NULL, 248, &g_ac_info.in_task)){
        USBD_UAC_PRINT_ERR("Could not usbd_uac_ac_in_task\r\n");
        goto fail;     
    }

#if USBD_UAC_ISOC_OUT_SUPPORT
    if((g_ac_info.out_start_sema=xSemaphoreCreateCounting(1,0))==0) {
        USBD_UAC_PRINT_ERR("Could not create out_start_sema\r\n");
        goto fail;
    }

    
    if (pdPASS != xTaskCreate(usbd_uac_ac_out_task, "usbd_uac_ac_out_task", 256, NULL, 248, &g_ac_info.out_task)){
        USBD_UAC_PRINT_ERR("Could not usbd_uac_ac_out_task\r\n");
        goto fail;
    }
#endif

    return pdPASS;


fail:
	
#if USBD_UAC_ISOC_OUT_SUPPORT	
	if(g_ac_info.out_start_sema)	{
	    vSemaphoreDelete(g_ac_info.out_start_sema);
	    g_ac_info.out_start_sema = NULL;
	}
#endif	

	if(g_ac_info.in_task)	{
	    vTaskDelete(g_ac_info.in_task);
	    g_ac_info.in_task = NULL;
	}	

	if(g_ac_info.in_start_sema)	{
		vSemaphoreDelete(g_ac_info.in_start_sema);
	    g_ac_info.in_start_sema = NULL;
	}
      
    return pdFAIL;

}

void usbd_uac_ac_uninit(void)
{
    if(g_ac_info.in_state==AC_STATE_START)
        usbd_uac_ac_in_task_stop();
    vTaskDelete(g_ac_info.in_task);
    g_ac_info.in_task = NULL;
    vSemaphoreDelete(g_ac_info.in_start_sema);
    g_ac_info.in_start_sema = NULL;

#if USBD_UAC_ISOC_OUT_SUPPORT	
    if(g_ac_info.out_state==AC_STATE_START)
        usbd_uac_ac_out_task_stop();
    vTaskDelete(g_ac_info.out_task);
    g_ac_info.out_task = NULL;
    vSemaphoreDelete(g_ac_info.out_start_sema);
    g_ac_info.out_start_sema = NULL;
#endif
}


#endif  //#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN) && defined( CONFIG_MODULE_USBD_UAC_CLASS )
