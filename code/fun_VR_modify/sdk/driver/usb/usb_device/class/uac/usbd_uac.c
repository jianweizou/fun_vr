/**
* @file
* this is usb device driver file
* usbd_uac.c
* @author IP2/Luka
*/
#include "sonix_config.h"

#if defined( CONFIG_MODULE_USBD_UAC_CLASS )
#include <stdio.h>
#include "usb_device.h"
#include "usbd_uac.h"
#include "usbd_uac_desc.h"
#include "composite.h"
#include "snx_udc.h"
#include <string.h>
#include <task.h>
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif



typedef enum uac_event
{
    UAC_EVENT_ON,
    UAC_EVENT_OFF,
    UAC_EVENT_PLAY,
    UAC_EVENT_RECORD,
    UAC_EVENT_STOP,
    UAC_EVENT_SUSPEND,
    UAC_EVENT_RESUME,
    UAC_EVENT_IDLE,
} uac_event_t;

typedef enum uac_state
{
    UAC_STATE_INIT,
    UAC_STATE_PLAYING,
    UAC_STATE_RECORDING,
    UAC_STATE_STANDBY,
    UAC_STATE_SUSPEND,
} uac_state_t;

typedef struct _event_info{
    unsigned char   intf;
    uac_event_t     event;
}event_info_t; 

struct uac_microphone
{
    uac_state_t         state;
    uac_event_t         event;
	uint32_t 			SamplesPerSec;
    struct usb_ep       *ep;
    struct usb_request  *req[1];
};

struct uac_speaker
{
    uac_state_t         state;
    uac_event_t         event;
	uint32_t 			SamplesPerSec;
    struct usb_ep       *ep;
    struct usb_request  *req[1];
};


struct uac_device
{
    struct usb_function     func;
    struct {
    const struct uac_descriptor_header * const *control;
    struct uac_descriptor_header *  *fs_streaming;
    struct uac_descriptor_header *  *hs_streaming;
    } desc;

    struct uac_microphone   microphone;
    struct uac_speaker      speaker;
    //unsigned int          streaming_intf;
    unsigned int            microphone_streaming_intf;
    unsigned int            speaker_streaming_intf;


    unsigned char           mute_value;
    unsigned short          volume_value;
    unsigned short          volume_value_min;
    unsigned short          volume_value_max;
    unsigned short          volume_value_res;
    unsigned char           bass_boost_value;

    

    //struct uac_streaming_control    commit;
    //unsigned char           *audio_buf;   //remove it
    //unsigned int            audio_len;    //remove it
    xQueueHandle event_queue;
};

#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
static usbd_drv_uac_start_record_cb_t uac_start_record_cb = NULL ;
static usbd_drv_uac_stop_record_cb_t uac_stop_record_cb = NULL;
static usbd_drv_uac_start_play_cb_t uac_start_play_cb = NULL ;
static usbd_drv_uac_stop_play_cb_t uac_stop_play_cb = NULL;
#endif
static usbd_drv_uac_ctrl_volume_cb_t uac_volume_cb = NULL;
static usbd_drv_uac_ctrl_mute_cb_t uac_mute_cb = NULL;

static SemaphoreHandle_t usbd_uac_in_slice_finish = NULL;
static SemaphoreHandle_t usbd_uac_out_slice_finish = NULL;
//static SemaphoreHandle_t usbd_uac_out_slice_start;

static uint8_t *uac_out_ping_buf = NULL;
static uint8_t *uac_out_pong_buf = NULL;

static uint8_t 	g_odd = 0;
static uint8_t *g_data_buf = NULL;
static uint32_t g_actual_xfr[UAC_OUT_XFR_CNT];
static uint32_t g_actual_xfrcnt = 0;

static struct uac_device *uac = NULL;
static unsigned int uac_task_run = 0;

static struct usb_string uac_strings[4];
static struct usb_gadget_strings uac_device_strings;
static struct usb_string uac_en_us_strings[7];	



static const struct uac_descriptor_header * const uac_control_cls[] = {
    (const struct uac_descriptor_header *) &uac_control_header,    
    (const struct uac_descriptor_header *) &uac_in_input_terminal_desc,
    (const struct uac_descriptor_header *) &uac_in_feature_unit_desc,
    (const struct uac_descriptor_header *) &uac_in_output_terminal_desc,
#if USBD_UAC_ISOC_OUT_SUPPORT    
    (const struct uac_descriptor_header *) &uac_out_input_terminal_desc,
    (const struct uac_descriptor_header *) &uac_out_feature_unit_desc,
    (const struct uac_descriptor_header *) &uac_out_output_terminal_desc, 
#endif    
    NULL,
};

static struct uac_descriptor_header * uac_hs_streaming[] = {
    (struct uac_descriptor_header *) &uac_in_streaming_intf_alt0,
    (struct uac_descriptor_header *) &uac_in_streaming_intf_alt1,
    (struct uac_descriptor_header *) &uac_in_streaming_cls_intf,
    (struct uac_descriptor_header *) &uac_in_format_desc,
    (struct uac_descriptor_header *) &uac_in_streaming_ep,
    (struct uac_descriptor_header *) &uac_in_streaming_cls_ep,
#if USBD_UAC_ISOC_OUT_SUPPORT       
    (struct uac_descriptor_header *) &uac_out_streaming_intf_alt0,
    (struct uac_descriptor_header *) &uac_out_streaming_intf_alt1,
    (struct uac_descriptor_header *) &uac_out_streaming_cls_intf,
    (struct uac_descriptor_header *) &uac_out_format_desc,
    (struct uac_descriptor_header *) &uac_out_streaming_ep,
    (struct uac_descriptor_header *) &uac_out_streaming_cls_ep,
#endif    
    NULL,
};

static struct uac_descriptor_header * uac_fs_streaming[] = {
    (struct uac_descriptor_header *) &uac_in_streaming_intf_alt0,
    (struct uac_descriptor_header *) &uac_in_streaming_intf_alt1,
    (struct uac_descriptor_header *) &uac_in_streaming_cls_intf,
    (struct uac_descriptor_header *) &uac_in_format_desc,
    (struct uac_descriptor_header *) &uac_in_streaming_ep,
    (struct uac_descriptor_header *) &uac_in_streaming_cls_ep,
    NULL,
};

static struct usb_gadget_strings uac_function_strings = {
    0x0409,                     // language
    uac_en_us_strings           // strings
};

#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
extern void usbd_uac_ac_in_task_start(void);
extern void usbd_uac_ac_in_task_stop(void);
extern void usbd_uac_ac_out_task_start(void);
extern void usbd_uac_ac_out_task_stop(void);
extern int usbd_uac_ac_init(void);
extern void usbd_uac_ac_uninit(void);
#endif

void usbd_drv_uac_start_record_reg_cb(usbd_drv_uac_start_record_cb_t cb)
{
#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	uac_start_record_cb = cb;
#endif
}
void usbd_drv_uac_stop_record_reg_cb(usbd_drv_uac_stop_record_cb_t cb)
{	
#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	uac_stop_record_cb = cb;
#endif
}

void usbd_drv_uac_start_play_reg_cb(usbd_drv_uac_start_play_cb_t cb)
{
#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	uac_start_play_cb = cb;
#endif
}
void usbd_drv_uac_stop_play_reg_cb(usbd_drv_uac_stop_play_cb_t cb)
{	
#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	uac_stop_play_cb = cb;
#endif
}


void usbd_drv_uac_ctrl_volume_reg_cb(usbd_drv_uac_ctrl_volume_cb_t cb)
{

#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	uac_volume_cb = cb;
#endif
}

void usbd_drv_uac_ctrl_mute_reg_cb(usbd_drv_uac_ctrl_mute_cb_t cb)
{
#if !defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	uac_mute_cb = cb;
#endif
}


/*================transfer control start=====================*/

int usbd_uac_drv_do_play(unsigned char *audio_buf, unsigned int audio_len)
{
    unsigned char *data_buf = 0;
    unsigned int offset = 0;	
    unsigned int i = 0;
    int remainder_audio_size = audio_len;

    //USBD_UAC_PRINT("%s\n",__func__);
    if(uac->speaker.event != UAC_EVENT_PLAY)	{
		return offset;	   
	}	

    if(usbd_uac_out_slice_finish){
        xSemaphoreTake(usbd_uac_out_slice_finish, portMAX_DELAY);
    }
    if(uac){ 	    
		data_buf = g_data_buf;
        for(i=0;i<g_actual_xfrcnt;i++){				
			if(remainder_audio_size<g_actual_xfr[i])	{
				USBD_UAC_PRINT_ERR("drop packet\n");
				break;
			}	
            memcpy((uint8_t*)(audio_buf+offset), data_buf, g_actual_xfr[i]);
            offset += g_actual_xfr[i];
            data_buf += uac->speaker.ep->s.packet_xfr_size;
			remainder_audio_size -= g_actual_xfr[i];		                
			if((i%10) ==9)
				portYIELD();	//prevent from hold CPU too long		
	
        }

        if(uac->speaker.event != UAC_EVENT_PLAY) {
			return offset;	        
        }
    }	

	return offset;
 
}

void usbd_uac_drv_do_record(unsigned char *audio_buf, unsigned int audio_len)
{
    uint32_t offset=0;  
    uint32_t total_len;    
    uint32_t data_slice_len ;
    uint32_t remainder_audio_size;

    //USBD_UAC_PRINT("%s\n",__func__);

    total_len = audio_len;	
    remainder_audio_size =  total_len;

    offset = 0;
	if(uac->microphone.event != UAC_EVENT_RECORD)
		return;

    do
    {
        data_slice_len = (UAC_IN_MAX_PAYLOAD_SIZE > remainder_audio_size) ? remainder_audio_size : UAC_IN_MAX_PAYLOAD_SIZE;
        uac->microphone.req[0]->buf = (uint8_t*)audio_buf + offset;	
        //memcpy(uac->microphone.req[0]->buf, audio_buf+offset, data_slice_len);
        remainder_audio_size -= data_slice_len;
        offset += data_slice_len;

        if(uac){
            uac->microphone.req[0]->length = data_slice_len;
            usb_ep_queue(uac->microphone.ep, uac->microphone.req[0]);

            if(usbd_uac_in_slice_finish){
                xSemaphoreTake(usbd_uac_in_slice_finish, 2000);
            }
            if(uac->microphone.event != UAC_EVENT_RECORD) {
                break;
            }
        }

    }while(remainder_audio_size > 0);	
 
}


static void uac_do_standby(void) {
    //USBD_UAC_PRINT("%s\n",__func__);
    uac->microphone.state = UAC_STATE_STANDBY;
    uac->speaker.state = UAC_STATE_STANDBY;
}

static void uac_do_off(void) {
    //USBD_UAC_PRINT("%s\n",__func__);
}

static void uac_do_play()
{
    //USBD_UAC_PRINT("%s\n",__func__);

    uac->speaker.state = UAC_STATE_PLAYING ;
    if(usbd_uac_out_slice_finish && uxSemaphoreGetCount(usbd_uac_out_slice_finish)==1 )
        xSemaphoreTake(usbd_uac_out_slice_finish, 0);
    
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
    usbd_uac_ac_out_task_start();
#else
	if(uac_start_play_cb)
		uac_start_play_cb(uac->speaker.SamplesPerSec);
#endif
}

static void uac_do_record(void)
{
    //USBD_UAC_PRINT("%s\n",__func__);

    uac->microphone.state = UAC_STATE_RECORDING;
    if(usbd_uac_in_slice_finish && uxSemaphoreGetCount(usbd_uac_in_slice_finish)==1 )
        xSemaphoreTake(usbd_uac_in_slice_finish, 0);
        
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	
    usbd_uac_ac_in_task_start();	
#else 
	if(uac_start_record_cb)
		uac_start_record_cb(uac->microphone.SamplesPerSec);
#endif   
}

static void uac_do_stop(char in) 
{
    //UUSBD_UAC_PRINT("%s\n",__func__);


    if(in)  {
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)	
        usbd_uac_ac_in_task_stop();
#else
	if(uac_stop_record_cb)
		uac_stop_record_cb();
#endif
        uac->microphone.state = UAC_STATE_STANDBY;
        if(usbd_uac_in_slice_finish)
		    xSemaphoreGive(usbd_uac_in_slice_finish);  //stop do-while loop in usbd_uac_drv_do_record()
    
    }    
    else    { 
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)		
        usbd_uac_ac_out_task_stop();
#else
	if(uac_stop_play_cb)
		uac_stop_play_cb();
#endif
        uac->speaker.state = UAC_STATE_STANDBY;
       	if(usbd_uac_out_slice_finish)
    		xSemaphoreGive(usbd_uac_out_slice_finish);  //stop do-while loop in usbd_uac_drv_do_play()
    }

}

static void uac_do_suspend(void) 
{
    //USBD_UAC_PRINT("%s\n",__func__);
}

static void uac_do_resume(void) 
{
    //USBD_UAC_PRINT("%s\n",__func__);
}


void uac_set_event_to_queue(unsigned char intf, uac_event_t event)
{
    event_info_t ev_info;
    ev_info.intf = intf;
    ev_info.event = event;
    xQueueSendToBack(uac->event_queue, &ev_info, portMAX_DELAY);
}
void uac_device_task( void *pvParameters )
{
    event_info_t ev_info;
    int is_isoc_in=0;
    uac_state_t* pState;
    while(uac_task_run) {
        xQueueReceive(uac->event_queue, &ev_info,portMAX_DELAY);
        if(ev_info.intf == uac->microphone_streaming_intf) {
            is_isoc_in=1;
            pState = &uac->microphone.state;
        }    
        else if(ev_info.intf == uac->speaker_streaming_intf){
            is_isoc_in=0;
            pState = &uac->speaker.state;
        }
        else
            continue;
        
        USBD_UAC_PRINT("(%s)event:%d->state:%d\n",is_isoc_in?"in":"out", ev_info.event,*pState);
        //printf("uac_device_task min free stack = %x\n", uxTaskGetStackHighWaterMark(NULL));
        switch(ev_info.event) {
            case UAC_EVENT_ON:
                if(*pState == UAC_STATE_INIT) {
                uac_do_standby();
                }
                break;
            case UAC_EVENT_OFF:
                if(*pState == UAC_STATE_STANDBY) {
                    uac_do_off();
                }
                else if((*pState == UAC_STATE_PLAYING) || (*pState == UAC_STATE_RECORDING)) {
                    uac_do_off();
                }
                break;
            case UAC_EVENT_PLAY:
                if((*pState == UAC_STATE_STANDBY)  &&  !is_isoc_in){
                    uac_do_play();
                }
                break;
            case UAC_EVENT_RECORD:
                if((*pState == UAC_STATE_STANDBY) &&  is_isoc_in) {
                    uac_do_record();
                }
                break;
            case UAC_EVENT_STOP:
                if((*pState == UAC_STATE_PLAYING) || (*pState == UAC_STATE_RECORDING)) {
                    uac_do_stop(is_isoc_in);
                }
                break;
            case UAC_EVENT_SUSPEND:
                if((*pState == UAC_STATE_PLAYING) || (*pState == UAC_STATE_RECORDING)) {
                    uac_do_suspend();
                }
                else if(*pState == UAC_STATE_STANDBY) {
                    uac_do_suspend();
                }
                break;
            case UAC_EVENT_RESUME:
                if(*pState == UAC_STATE_SUSPEND) {
                    uac_do_resume();
                }
                break;
            case UAC_EVENT_IDLE:
                    default:
                break;
        }
    }
    vTaskDelete(NULL);
}

void uac_device_task_stop(void)
{
    uac_task_run = 0;
    if(uac->event_queue)
    {
        int timeout=100;
        //send idle event to queue,it cam make task skip block from queue
        uac->microphone.event = UAC_EVENT_IDLE;
        uac_set_event_to_queue(uac->microphone_streaming_intf, uac->microphone.event);
        while(uac_task_run && timeout>=0) 
            vTaskDelay(10 / portTICK_RATE_MS );
        if(timeout==0)
            USBD_UAC_PRINT_ERR("wait uac device task timeout\n");
        vQueueDelete(uac->event_queue);
        uac->event_queue = NULL;
    } 
}



/*================transfer control end=====================*/


/*================configuration control start=====================*/

#define UAC_COPY_DESCRIPTOR(mem, dst, desc) \
    memcpy(mem, desc, (desc)->bLength); \
    *(dst)++ = (void*)mem; \
    mem += (desc)->bLength;

#define UAC_COPY_DESCRIPTORS(mem, dst, src) \
    for (__src = src; *__src; ++__src) { \
        memcpy(mem, *__src, (*__src)->bLength); \
        *dst++ = (void*)mem; \
        mem += (*__src)->bLength; \
    }

static struct usb_descriptor_header ** uac_copy_descriptors(enum usb_device_speed speed)
{
    const struct usb_descriptor_header  * const *__src;
    struct uac_descriptor_header  * const *uac_streaming_cls;
    //struct uac_descriptor_header  **uac_streaming_std;
    unsigned int                        control_size;
    unsigned int                        streaming_size;
    unsigned int                        n_desc;
    unsigned int                        bytes;
    //unsigned int                        uac_streaming_intf;
    //struct uac_input_header_descriptor  *uac_streaming_header;
    struct uac_header_descriptor        *uac_control_header;

    const struct usb_descriptor_header  * const *src;
    struct usb_descriptor_header        **dst;
    struct usb_descriptor_header        **hdr;

    uint8_t                             *mem;

    uac_streaming_cls = (speed == USB_SPEED_FULL)  ? uac->desc.fs_streaming : uac->desc.hs_streaming;

    /* Count descriptors and compute their size. */
    control_size = 0;
    streaming_size = 0;

    bytes = uac_iad.bLength + uac_control_intf.bLength ;

    n_desc = 2;

    for (src = (const struct usb_descriptor_header**)uac->desc.control; *src; ++src) {
        control_size += (*src)->bLength;
        bytes += (*src)->bLength;
        n_desc++;
    }

    for (src = (const struct usb_descriptor_header**)uac_streaming_cls; *src; ++src) {
        streaming_size += (*src)->bLength;
        bytes += (*src)->bLength;
        n_desc++;
    }

    mem = (uint8_t*)usbd_malloc((n_desc + 1) * sizeof(*src)+ bytes);
    if (mem == NULL)
        return NULL;

    hdr = (void*)mem;
    dst = (void*)mem;
    mem += (n_desc + 1) * sizeof(*src);


    /* Copy the descriptors. */
    UAC_COPY_DESCRIPTOR(mem, dst, &uac_iad);
    UAC_COPY_DESCRIPTOR(mem, dst, &uac_control_intf);

    uac_control_header                      = (void*)mem;
    UAC_COPY_DESCRIPTORS(mem, dst, (const struct usb_descriptor_header**)uac->desc.control);
    uac_control_header->wTotalLength        = control_size;
    //uac_control_header->bInCollection       = 1;
    uac_control_header->bInCollection       = 1+USBD_UAC_ISOC_OUT_SUPPORT;
    //uac_control_header->baInterfaceNr[0]    = uac->streaming_intf;
    uac_control_header->baInterfaceNr[0]    = uac->microphone_streaming_intf;
#if USBD_UAC_ISOC_OUT_SUPPORT	
    uac_control_header->baInterfaceNr[1]    = uac->speaker_streaming_intf;
#endif
    //uac_streaming_header                    = (void*)mem;
    UAC_COPY_DESCRIPTORS(mem, dst,(const struct usb_descriptor_header**)uac_streaming_cls);


    *dst = NULL;
    return hdr;
}



#define NO_ERROR_ERR                                    0x00
#define NOT_READY_ERR                                   0x01
#define WRONG_STATE_ERR                                 0x02
#define POWER_ERR                                       0x03
#define OUT_OF_RANGE_ERR                                0x04
#define INVALID_UNIT_ERR                                0x05
#define INVALID_CONTROL_ERR                             0x06
#define INVALID_REQUEST_ERR                             0x07
#define UNKNOWN_ERR                                     0xFF
int     error_code_control;
int     err;

static int uac_events_process_control(const struct usb_ctrlrequest *ctrl)
{
    unsigned char       endpoint = ctrl->wIndex & 0xFF;
    unsigned char       cs = ctrl->wValue >> 8;

    struct usb_ep       *ep = uac->func.config->cdev->gadget->ep0;
    struct usb_request  *req = uac->func.config->cdev->req;
    int                 ret = ESUCCESS;
	uint32_t cur_samfreq = 0;

    //USBD_UAC_PRINT("%s\n",__func__);

    error_code_control = NO_ERROR_ERR;


    if(!uac) return -ECONNRESET;

    switch (cs) {
        case UAC_EP_CS_ATTR_SAMPLE_RATE:
            //USBD_UAC_PRINT("UAC_EP_CS_ATTR_SAMPLE_RATE op=%08x\n",ctrl->bRequest);
            switch (ctrl->bRequest) {
                case UAC_SET_CUR:
					req->length = 3; 
                    ret = usb_ep_queue(ep, req);					
                    if(endpoint == uac_in_streaming_ep.bEndpointAddress)    {
                        uac->microphone.event = UAC_EVENT_RECORD;
						uac->microphone.SamplesPerSec = (req->buf[2]<<16) | (req->buf[1]<<8) | req->buf[0];
						USBD_UAC_PRINT("(in)set frequency %x to start streaming\n", uac->microphone.SamplesPerSec);
                        uac_set_event_to_queue(uac->microphone_streaming_intf, uac->microphone.event);
                    }    
                    else if(endpoint == uac_out_streaming_ep.bEndpointAddress)  {
                        uac->speaker.event = UAC_EVENT_PLAY;
						uac->speaker.SamplesPerSec = (req->buf[2]<<16) | (req->buf[1]<<8) | req->buf[0];
						USBD_UAC_PRINT("(out)set frequency %x to start streaming\n", uac->speaker.SamplesPerSec);
                        uac_set_event_to_queue(uac->speaker_streaming_intf, uac->speaker.event);
                    }
                    break;
                case UAC_GET_CUR:
					
					if(endpoint == uac_in_streaming_ep.bEndpointAddress) 	{
						cur_samfreq = uac->microphone.SamplesPerSec;

					}	
					else if(endpoint == uac_out_streaming_ep.bEndpointAddress)	{
						cur_samfreq = uac->speaker.SamplesPerSec;	

					}	
                    req->buf[0] = cur_samfreq&0xff;
                    req->buf[1] = (cur_samfreq>>8)&0xff;
                    req->buf[2] = (cur_samfreq>>16)&0xff;	
					req->length = min((int)ctrl->wLength, 3);
                    ret = usb_ep_queue(ep, req);
                    break;
                default:
                    ret = INVALID_REQUEST_ERR;
                    break;
            }
            break;
        default:
            error_code_control = INVALID_REQUEST_ERR;
            break;
    }

    if(error_code_control != NO_ERROR_ERR)
    ret = -EOPNOTSUPP;
    return ret;
}

static int uac_events_process_fu_control(const struct usb_ctrlrequest *ctrl)
{
    unsigned char       entity		= ctrl->wIndex >> 8;
    unsigned char       interface 	= ctrl->wIndex & 0x00ff;
    unsigned char       cs			= ctrl->wValue >> 8;
    unsigned char       cn			= ctrl->wValue & 0x00ff;
    int                 ret			= ESUCCESS;
    struct usb_ep       *ep			= uac->func.config->cdev->gadget->ep0;;
    struct usb_request  *req		= uac->func.config->cdev->req;;

    //USBD_UAC_PRINT("%s\n",__func__);

    if(!uac) return -ECONNRESET;
        // check recipient, must be interface recipient
        if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE) {
        USBD_UAC_PRINT_ERR("Invalid recipient!\n");
        return INVALID_REQUEST_ERR;
    }
    // check interface
    if (interface != 0) {
        USBD_UAC_PRINT_ERR("Invalid interface!\n");
        return INVALID_CONTROL_ERR;
    }
    error_code_control = NO_ERROR_ERR;

    switch (cs)
    {
        case UAC_FU_MUTE:
			if(ctrl->wLength!=1)	{
				ret = -EOPNOTSUPP;
				goto end;
			}	
			req->length = 1;
            switch (ctrl->bRequest)
            {
                case UAC_SET_CUR:
                    ret = usb_ep_queue(ep, req);
					if((entity == UAC_IN_FEATURE_UNIT_ID) && uac_mute_cb)
						uac_mute_cb(cn, UAC_SET_CUR, req->buf);
					else
						uac->mute_value = req->buf[0];
                    break;
                case UAC_GET_CUR:
					if((entity == UAC_IN_FEATURE_UNIT_ID) && uac_mute_cb)
						uac_mute_cb(cn, UAC_GET_CUR, req->buf);
					else
						req->buf[0] = uac->mute_value;
                    ret = usb_ep_queue(ep, req);
                    break;
                default:
                    ret = INVALID_REQUEST_ERR;
                    break;
            }
            break;
        case UAC_FU_VOLUME:
			if(ctrl->wLength!=2)	{
				ret = -EOPNOTSUPP;
				goto end;
			}	
			req->length = 2;
            switch (ctrl->bRequest)
            {
            	unsigned short volume_value;
                case UAC_SET_CUR:
                    err = usb_ep_queue(ep, req);
					volume_value = (req->buf[1]<<8) | (req->buf[0]);
					if((entity == UAC_IN_FEATURE_UNIT_ID) && uac_volume_cb)
						uac_volume_cb(cn, UAC_SET_CUR, &volume_value);
					else
						uac->volume_value = volume_value;
                    break;
                case UAC_GET_CUR:
                    if((entity == UAC_IN_FEATURE_UNIT_ID) && uac_volume_cb)	
						uac_volume_cb(cn, UAC_GET_CUR, &volume_value);
					else
						volume_value = uac->volume_value;
					req->buf[0] = volume_value;
					req->buf[1] = volume_value>>8;
                    ret = usb_ep_queue(ep, req);
                    break;
                case UAC_GET_MIN:
                    if((entity == UAC_IN_FEATURE_UNIT_ID) && uac_volume_cb)	
						uac_volume_cb(cn, UAC_GET_MIN, &volume_value);
					else
						volume_value = uac->volume_value_min;
					req->buf[0] = volume_value;
					req->buf[1] = volume_value>>8;
                    ret = usb_ep_queue(ep, req);
                    break;
                case UAC_GET_MAX:
                    if((entity == UAC_IN_FEATURE_UNIT_ID) && uac_volume_cb)	
						uac_volume_cb(cn, UAC_GET_MAX, &volume_value);
					else
						volume_value = uac->volume_value_max;
					req->buf[0] = volume_value;
					req->buf[1] = volume_value>>8;
                    ret = usb_ep_queue(ep, req);
                    break;
                case UAC_GET_RES:
					req->buf[0] = uac->volume_value_res;
					req->buf[1] = uac->volume_value_res>>8;
                    ret = usb_ep_queue(ep, req);
                    break;
                default:
                    ret = INVALID_REQUEST_ERR;
                    break;
            }
            break;
        case UAC_FU_BASS_BOOST:
            switch (ctrl->bRequest)
            {
                case UAC_SET_CUR:
                    ret = usb_ep_queue(ep, req);
                    memcpy(&(uac->bass_boost_value),req->buf,sizeof(uac->bass_boost_value));
                    break;
                case UAC_GET_CUR:
                    req->buf[0]=uac->bass_boost_value;
                    req->length = sizeof(uac->bass_boost_value);
                    ret = usb_ep_queue(ep, req);
                    break;
                default:
                    ret = INVALID_REQUEST_ERR;
                    break;
            }
            break;
        default:
            error_code_control = INVALID_REQUEST_ERR;
            break;
    }
    if(error_code_control != NO_ERROR_ERR)
	    ret = -EOPNOTSUPP;
end:
    return ret;
}

static void uac_function_unbind(struct usb_configuration *c, struct usb_function *f)
{

    if(!uac)
        return;

    //if(uac->microphone.req[0]->buf) {
    //    vPortFree(uac->microphone.req[0]->buf);
    //    uac->microphone.req[0]->buf = NULL;
    //}

    if(uac->microphone.req[0])  {
        usb_ep_free_request(uac->microphone.ep, uac->microphone.req[0]);
        
    }

	uac->microphone.req[0] = NULL;
	
	if(uac_out_ping_buf)	{
		vPortFree(uac_out_ping_buf);
		uac_out_ping_buf = NULL;
	}	
	
	if(uac_out_pong_buf)	{
		vPortFree(uac_out_pong_buf);
		uac_out_pong_buf = NULL;
	}
	
    if(uac->speaker.req[0])  {
        usb_ep_free_request(uac->speaker.ep, uac->speaker.req[0]);
        uac->speaker.req[0] = NULL;
    }




    if(f->descriptors)  {
        vPortFree(f->descriptors);
        f->descriptors = NULL;
    }  

    if(f->hs_descriptors)  {
        vPortFree(f->hs_descriptors);
        f->hs_descriptors = NULL;
    }  

    

}
static int uac_function_bind(struct usb_configuration *c, struct usb_function *f)
{
    struct usb_composite_dev *cdev = c->cdev;
    struct usb_ep *ep;
    int ret = -EINVAL;

    /* Setting isoc in Endpoint */
    if ((ep = usb_ep_autoconfig(cdev->gadget, &uac_in_streaming_ep, "UAC-ISO")) == NULL) {
        USBD_UAC_PRINT_ERR("ep autoconfig fail\n");
        goto error;
    }
    ep->s.packet_xfr_size = uac_in_streaming_ep.wMaxPacketSize;  //set trb slice length
    uac->microphone.ep = ep;
    if((uac->microphone.req[0] = usb_ep_alloc_request(uac->microphone.ep))==NULL)	{
        USBD_UAC_PRINT_ERR("ep struct alloc fail\n");
        goto error;
    }
    //uac->microphone.req[0]->buf = (uint8_t*)usbd_malloc(UAC_IN_MAX_PAYLOAD_SIZE);
    //if (uac->microphone.req[0]->buf == NULL ) {
    //    ret = -ENOMEM;
    //    goto error;
    //}

    /* Setting isoc out Endpoint */
    if((ep = usb_ep_autoconfig(cdev->gadget, &uac_out_streaming_ep, "UAC-ISO")) == NULL){
        USBD_UAC_PRINT_ERR("ep autoconfig fail\n");
        goto error;
    }
    ep->s.packet_xfr_size = uac_out_streaming_ep.wMaxPacketSize;  //set trb slice length
    uac->speaker.ep = ep;
    if((uac->speaker.req[0] = usb_ep_alloc_request(uac->speaker.ep))==NULL)	{
	    USBD_UAC_PRINT_ERR("ep struct alloc fail\n");
        goto error;
    }	
    if((uac_out_ping_buf = (uint8_t*)usbd_malloc(UAC_OUT_MAX_PAYLOAD_SIZE)) == NULL) {
		USBD_UAC_PRINT_ERR("alloc ping buf fail\n");
        ret = -ENOMEM;
        goto error;
    }
    if((uac_out_pong_buf = (uint8_t*)usbd_malloc(UAC_OUT_MAX_PAYLOAD_SIZE)) == NULL) {
		USBD_UAC_PRINT_ERR("alloc pong buf fail\n");
        ret = -ENOMEM;
        goto error;
    }
	uac->speaker.req[0]->length = UAC_OUT_MAX_PAYLOAD_SIZE;
	uac->speaker.req[0]->m_length = (uint32_t*)usbd_malloc(4*(UAC_OUT_XFR_CNT+1));
	uac->speaker.req[0]->ping_buf = uac_out_ping_buf;
	uac->speaker.req[0]->pong_buf = uac_out_pong_buf;


    /* Setting interfaces */
    if ((ret = usb_interface_id(c, f)) != UAC_INTF_CONTROL)
        goto error;
    uac_iad.bFirstInterface = ret;
    uac_control_intf.bInterfaceNumber = ret;

    if ((ret = usb_interface_id(c, f)) != UAC_INTF_IN_STREAMING)
        goto error;
    uac_in_streaming_intf_alt0.bInterfaceNumber = ret;
    uac_in_streaming_intf_alt1.bInterfaceNumber = ret;
    uac->microphone_streaming_intf = ret;

#if USBD_UAC_ISOC_OUT_SUPPORT
    if ((ret = usb_interface_id(c, f)) != UAC_INTF_OUT_STREAMING)
        goto error;
    uac_out_streaming_intf_alt0.bInterfaceNumber = ret;
    uac_out_streaming_intf_alt1.bInterfaceNumber = ret;
    uac->speaker_streaming_intf = ret;
#endif

    /* Copy descriptors. */
    f->descriptors = uac_copy_descriptors(USB_SPEED_FULL);
    f->hs_descriptors = uac_copy_descriptors(USB_SPEED_HIGH);


    return 0;

error:
    uac_function_unbind(c, f);
    return ret;
}


static int uac_function_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
    int ret = ESUCCESS;
    //USBD_UAC_PRINT("%s setup request %02x %02x value %04x index %04x %04x\n",__FUNCTION__,ctrl->bRequestType, ctrl->bRequest, (ctrl->wValue),(ctrl->wIndex), (ctrl->wLength));

    if(!uac  ) return -ECONNRESET;


    /* Stall too big requests. */
    if (ctrl->wLength > UAC_MAX_REQUEST_SIZE)
    return -EINVAL;

    switch (ctrl->bRequestType & USB_TYPE_MASK) {
        case USB_TYPE_STANDARD:
            ret = -EOPNOTSUPP;
            break;
        case USB_TYPE_CLASS:
            if ((ctrl->bRequestType & USB_RECIP_MASK) == USB_RECIP_INTERFACE) {
                switch (ctrl->wIndex>>8 & 0xff) {
                    case UAC_IN_FEATURE_UNIT_ID:
                    case UAC_OUT_FEATURE_UNIT_ID:
                        ret = uac_events_process_fu_control(ctrl);
                        break;
                }
            }
            else if ((ctrl->bRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPOINT) {
                /*
                if (ctrl->wIndex == 0x0083) {
                uac_events_process_control(ctrl);
                break;
                }
                */
                //if (ctrl->wIndex == (uac->microphone.ep)->address || ctrl->wIndex == (uac->speaker.ep)->address){
                if (ctrl->wIndex == uac_out_streaming_ep.bEndpointAddress || ctrl->wIndex == uac_in_streaming_ep.bEndpointAddress){
                    uac_events_process_control(ctrl);
                    break;   
                }
            }
            else {
                ret = -EOPNOTSUPP;
            }
            break;
        default:
            ret = -EOPNOTSUPP;
            break;
    }
    return ret;
}

static void uac_function_disable(struct usb_function *f)
{

}

static int uac_function_get_alt(struct usb_function *f, unsigned interface, unsigned alt)
{
    //USBD_UAC_PRINT("%s, interface=%d, alt=%d\r\n",__FUNCTION__,interface,alt);
    //USBD_UAC_PRINT("uac->microphone.ep=%08X\r\n",uac->microphone.ep->state);

    if (interface == 0)
        return 0;

    //if (interface != uac->streaming_intf)
    //    return -EINVAL;

    if (interface != uac->microphone_streaming_intf)
        return -EINVAL;

    return (uac->microphone.ep->state & USBD_EP_STATE_DISABLE) ? 0 : 1;
}
static int uac_function_set_alt(struct usb_function *f, unsigned interface, unsigned alt)
{
    USBD_UAC_PRINT("%s:interface=%d, alt=%d\r\n",__func__, interface,alt);

    switch (alt) {
        case 0:
            if(interface == uac->microphone_streaming_intf) {
                usb_ep_disable(uac->microphone.ep);
                if(uac->microphone.state == UAC_STATE_RECORDING)    {
                    uac->microphone.event = UAC_EVENT_STOP;
                    uac_set_event_to_queue(uac->microphone_streaming_intf, uac->microphone.event);
                }    
            } 
            else if(interface == uac->speaker_streaming_intf)   {
                usb_ep_disable(uac->speaker.ep);
                if(uac->speaker.state == UAC_STATE_PLAYING) {
                    uac->speaker.event = UAC_EVENT_STOP;
					usb_ep_dequeue(uac->speaker.ep, uac->speaker.req[0]);
                    uac_set_event_to_queue(uac->speaker_streaming_intf, uac->speaker.event);
                }
            }
			else if(interface == 0){
				usb_ep_enable(uac->microphone.ep, &uac_in_streaming_ep);
				usb_ep_enable(uac->speaker.ep, &uac_out_streaming_ep);	 				
			}
            break;
        case 1:
            if(interface == uac->microphone_streaming_intf)
                usb_ep_enable(uac->microphone.ep, &uac_in_streaming_ep);
            else if(interface == uac->speaker_streaming_intf)
                usb_ep_enable(uac->speaker.ep, &uac_out_streaming_ep);

			//set ep enable for isoc in and out at the same time because value of USBD_REG_EP_NEW_MAXPKTSIZE
			//only can be changed in this flow now
//			usb_ep_enable(uac->microphone.ep, &uac_in_streaming_ep);
//			usb_ep_enable(uac->speaker.ep, &uac_out_streaming_ep);	    
            break;
        default:
            return -EINVAL;
    }
    //USBD_UAC_PRINT("uac->microphone.ep=%08X\r\n",uac->microphone.ep->state);

    return 0;
}



static void uac_functione_ep3_callback(struct usb_function *f,struct usb_ep *ep)
{
    //USBD_UAC_PRINT("ep->state=%08x\r\n",ep->state);
    if((ep->state & USBD_EP_STATE_IN) == USBD_EP_STATE_IN) {
        ep->state &= ~USBD_EP_STATE_IN;
    }
    else if((ep->state & USBD_EP_STATE_TOC) ==  USBD_EP_STATE_TOC) {
        ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC );
        if(usbd_uac_in_slice_finish && (uac->microphone.state == UAC_STATE_RECORDING)) {
            xSemaphoreGive(usbd_uac_in_slice_finish);
            //USBD_UAC_PRINT("ep3 slice_finish give\n");
        }
    }
}

static void uac_functione_ep4_callback(struct usb_function *f,struct usb_ep *ep)
{    
    //USBD_UAC_PRINT("ep->state=%08x\r\n",ep->state);
	
	if((ep->state & USBD_EP_STATE_OUT) == USBD_EP_STATE_OUT) {
		ep->state &= ~USBD_EP_STATE_OUT;
		g_odd = 0;
	}
	else if((ep->state & USBD_EP_STATE_TOC) ==  USBD_EP_STATE_TOC){
		ep->state &= ~USBD_EP_STATE_TOC; 
		
		g_actual_xfrcnt = uac->speaker.req[0]->m_cnt;
		
		memcpy( g_actual_xfr, uac->speaker.req[0]->m_length,  g_actual_xfrcnt*sizeof(uint32_t));
		//while(i < g_actual_xfrcnt){
		//	g_actual_xfr[i] = *((uint32_t*)uac->speaker.req[0]->m_length + i);
		//	i++;
		//}
		
		
		//erk uac->speaker.req[0]->length = UAC_OUT_MAX_PAYLOAD_SIZE;
		if(!g_odd)
			g_data_buf = uac->speaker.req[0]->ping_buf;        
		else
			g_data_buf = uac->speaker.req[0]->pong_buf; 

		g_odd = ((~g_odd) & 0x01);

        if(usbd_uac_out_slice_finish && (uac->speaker.state == UAC_STATE_PLAYING)){
            xSemaphoreGive(usbd_uac_out_slice_finish);
        }
    }	      
}



static int uac_config_bind(struct usb_configuration *c)
{
    int ret = 0;
	xTaskHandle uac_dev_task = NULL;
	const struct uac_descriptor_header * const *control = uac_control_cls;
    struct uac_descriptor_header **fs_streaming         = uac_fs_streaming;
    struct uac_descriptor_header **hs_streaming         = uac_hs_streaming;

    //USBD_UAC_PRINT("%s\n",__func__);

    uac = (struct uac_device*)usbd_malloc(sizeof(struct uac_device));
    if (uac == NULL)	{
		USBD_UAC_PRINT_ERR("alloc struct uac_device fail\n");
        return -ENOMEM;
    }
    memset(uac, 0, sizeof(*uac));
    
    uac->volume_value_min =	UAC_FU_VOLUME_MIN;
    uac->volume_value_max =	UAC_FU_VOLUME_MAX;
    uac->volume_value_res =	UAC_FU_VOLUME_RES;

	ret = -EINVAL;
    if (control == NULL || control[0] == NULL ||  control[0]->bDescriptorSubType != UAC_HEADER)
        goto error;

    if (fs_streaming == NULL || fs_streaming[0] == NULL)
        goto error;

    if (hs_streaming == NULL || hs_streaming[0] == NULL)
        goto error;

    uac->desc.control = control;
    uac->desc.fs_streaming = fs_streaming;
    uac->desc.hs_streaming = hs_streaming;

    if ((ret = usb_string_id(c->cdev)) < 0)
        goto error;
    uac_en_us_strings[UAC_STRING_ASSOCIATION_IDX].id = ret;
    uac_iad.iFunction = ret;

    if ((ret = usb_string_id(c->cdev)) < 0)
        goto error;
    uac_en_us_strings[UAC_STRING_CONTROL_IDX].id = ret;
    uac_control_intf.iInterface = ret;

    if ((ret = usb_string_id(c->cdev)) < 0)
        goto error;
    uac_en_us_strings[UAC_STRING_IN_STREAMING_ALT0_IDX].id = ret;
    uac_in_streaming_intf_alt0.iInterface = ret;

    if ((ret = usb_string_id(c->cdev)) < 0)
        goto error;
    uac_en_us_strings[UAC_STRING_IN_STREAMING_ALT1_IDX].id = ret;
    uac_in_streaming_intf_alt1.iInterface = ret;


    if ((ret = usb_string_id(c->cdev)) < 0)
        goto error;
    uac_en_us_strings[UAC_STRING_OUT_STREAMING_ALT0_IDX].id = ret;
    uac_out_streaming_intf_alt0.iInterface = ret;

    if ((ret = usb_string_id(c->cdev)) < 0)
        goto error;
    uac_en_us_strings[UAC_STRING_OUT_STREAMING_ALT1_IDX].id = ret;
    uac_out_streaming_intf_alt1.iInterface = ret;

    uac->func.name              = "USB audio Function";
    uac->func.strings           = &uac_function_strings;
    uac->func.bind              = uac_function_bind;
    uac->func.unbind            = uac_function_unbind;
    uac->func.get_alt           = uac_function_get_alt;
    uac->func.set_alt           = uac_function_set_alt;
    uac->func.disable           = uac_function_disable;
    uac->func.setup             = uac_function_setup;
    uac->func.ep_callback[3]    = uac_functione_ep3_callback;
    uac->func.ep_callback[4]    = uac_functione_ep4_callback;

    ret = usb_add_function(c, &uac->func);
	if(ret<0)
		goto error;

    if((uac->event_queue = xQueueCreate(5, sizeof(event_info_t)))!=NULL)
    {
        uac->microphone.event = UAC_EVENT_ON;

		//change state of isoc in/out from UAC_STATE_INIT to UAC_STATE_STANDBY
        uac_set_event_to_queue(uac->microphone_streaming_intf, uac->microphone.event);	
    }   
    if((usbd_uac_in_slice_finish = xSemaphoreCreateCounting(1,0)) == NULL)	{
		ret = -ENOMEM;
		goto error;
    }
   if((usbd_uac_out_slice_finish = xSemaphoreCreateCounting(1,0)) == NULL)	{
	   ret = -ENOMEM;
	   goto error;
   }

/*
    usbd_uac_stop = xSemaphoreCreateCounting(1,0);
    if(!usbd_uac_stop) {
        SBD_UAC_PRINT(" create usbd_uac_stop is fail~~~\r\n");
    }
*/
/*
    //remove because isn't used
    if(uac) {
        uac->audio_buf = (unsigned char*)usbd_malloc(UAC_AUDIO_DATA_BUFFER_SIZE);
        if (uac->audio_buf == NULL) {
            USBD_UAC_PRINT("USB device warning !!! Unable to allocate space for audio buffer !!!\r\n");
        }
    }
*/
    USBD_UAC_PRINT("create uvc_device_task\r\n");

    uac_task_run = 1;
    if (pdPASS != xTaskCreate(uac_device_task, "uac_device_task", 256, (void*) NULL, 250, &uac_dev_task)) {
        USBD_UAC_PRINT_ERR("Could not uac_device_task\r\n");
		ret = -ENOMEM;
		goto error;		
    }
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
    if(pdPASS != usbd_uac_ac_init())	{
        USBD_UAC_PRINT_ERR("usbd uac audio init fail\r\n");
		ret = -ENOMEM;
		goto error;
    }
#endif
    return 0;
error:

	USBD_UAC_PRINT_ERR("return fail\n");

	if(uac_dev_task)
		vTaskDelete(uac_dev_task);
	
    if(usbd_uac_out_slice_finish)   {
        vSemaphoreDelete(usbd_uac_out_slice_finish);
        usbd_uac_out_slice_finish = NULL;
    }  

    if(usbd_uac_in_slice_finish)   {
        vSemaphoreDelete(usbd_uac_in_slice_finish);
        usbd_uac_in_slice_finish = NULL;
    }    

	if(uac->event_queue) {
		vQueueDelete(uac->event_queue);
		uac->event_queue = NULL;
	}

	if(uac->func.unbind)	{
		uac->func.unbind(c, &uac->func);
	}

	if(uac)	{
		vPortFree(uac);
		uac = NULL;
	}	
	return ret;
}


static struct usb_configuration uac_config_driver = {
    uac_config_label,               // label
    0,                              // usb_function
    uac_config_bind,                // bind
    1,                              // bConfigurationValue
    0,                              // iConfiguration /* dynamic */
    USB_CONFIG_ATT_SELFPOWER,       // bmAttributes
    CONFIG_USB_GADGET_VBUS_DRAW / 2,// bMaxPower
};


/*================configuration control end=====================*/


/*================composite driver start=====================*/

void uac_description_init(void)
{
    int i;
    if(UAC_IN_LOGICAL_CHANNEL<=1)
    {
        uac_in_input_terminal_desc.wTerminalType = UAC_INPUT_TERMINAL_MICROPHONE;
        uac_in_input_terminal_desc.wChannelConfig = 0x03;
    }
    else
    {
        uac_in_input_terminal_desc.wTerminalType = UAC_INPUT_TERMINAL_PROC_MICROPHONE_ARRAY;
#if UAC_IN_LOGICAL_CHANNEL==3
        uac_in_input_terminal_desc.wChannelConfig = 0x00f3;
#elif UAC_IN_LOGICAL_CHANNEL==4
        uac_in_input_terminal_desc.wChannelConfig = 0x00ff;
#else
        uac_in_input_terminal_desc.wChannelConfig = 0x0003;
#endif
    }

    for(i=0;i<UAC_IN_LOGICAL_CHANNEL;i++)
        uac_in_feature_unit_desc.bmaControls[i+1] = UAC_FU_MUTE|UAC_FU_VOLUME;
    uac_in_feature_unit_desc.iFeature = 0;

}

static int uac_unbind(struct usb_composite_dev *cdev)
{

	USBD_UAC_PRINT("%s\n",__func__);
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
    usbd_uac_ac_uninit();
#endif
    //clear all resource alloced in uac_config_bind()
    uac_device_task_stop();

    if(usbd_uac_in_slice_finish)   {
        vSemaphoreDelete(usbd_uac_in_slice_finish);
        usbd_uac_in_slice_finish = NULL;
    }    

    if(usbd_uac_out_slice_finish)   {
        vSemaphoreDelete(usbd_uac_out_slice_finish);
        usbd_uac_out_slice_finish = NULL;
    }  

	if(cdev->config)
		uac_function_unbind(cdev->config, &uac->func); 

    if(uac) {
        vPortFree(uac);
        uac = NULL;
    }
    return 0;
}


static int uac_bind(struct usb_composite_dev *cdev)
{
    int ret;
    if ((ret = usb_string_id(cdev)) < 0)
        goto error;
    uac_strings[STRING_MANUFACTURER_IDX].id = ret;
    uac_device_desc.iManufacturer = ret;

    if ((ret = usb_string_id(cdev)) < 0)
        goto error;
    uac_strings[STRING_PRODUCT_IDX].id = ret;
    uac_device_desc.iProduct = ret;

    if ((ret = usb_string_id(cdev)) < 0)
        goto error;
    uac_strings[STRING_DESCRIPTION_IDX].id = ret;
    uac_config_driver.iConfiguration = ret;

    uac_qualifier_desc.bNumConfigurations = uac_config_driver.iConfiguration;

    uac_description_init();
    //uac_config_driver.bind = uac_config_bind;
    if ((ret = usb_add_config(cdev, &uac_config_driver)) < 0)
        goto error;

    USBD_UAC_PRINT("Sonix UAC Driver\n");
    return 0;
error:
    uac_unbind(cdev);
    return ret;
}

static void uac_suspend(struct usb_composite_dev *cdev)
{

}

static void uac_resume(struct usb_composite_dev *cdev)
{

}


static struct usb_composite_driver uac_driver = {
    "uac driver",                   // name
    &uac_device_desc,               // dev
    &uac_qualifier_desc,            // qualifier
    &uac_device_strings,            // strings
    uac_bind,                       // bind
    uac_unbind,                     // unbind
    uac_suspend,                    // suspend
    uac_resume,                     // resume
};

/*================composite driver end=====================*/



/*================device driver start=====================*/



int usbd_uac_init(void)
{
    int ret = 0;

    strcpy(uac_strings[STRING_MANUFACTURER_IDX].s, uac_vendor_label);
    strcpy(uac_strings[STRING_PRODUCT_IDX].s, uac_product_label);
    strcpy(uac_strings[STRING_DESCRIPTION_IDX].s, uac_config_label);

    //strcpy(uac_strings[STRING_EMPTY_IDX].s,"");

    uac_device_strings.language 	= 0x0409;
    uac_device_strings.strings	= uac_strings;

    strcpy(uac_en_us_strings[UAC_STRING_ASSOCIATION_IDX].s,         "SN7320 Audio");
    strcpy(uac_en_us_strings[UAC_STRING_CONTROL_IDX].s,             "Audio Control Interface");
    strcpy(uac_en_us_strings[UAC_STRING_IN_STREAMING_ALT0_IDX].s,   "Audio Capture Inactive");
    strcpy(uac_en_us_strings[UAC_STRING_IN_STREAMING_ALT1_IDX].s,   "Audio Capture active");

    strcpy(uac_en_us_strings[UAC_STRING_OUT_STREAMING_ALT0_IDX].s,  "Audio Playback Inactive");
    strcpy(uac_en_us_strings[UAC_STRING_OUT_STREAMING_ALT1_IDX].s,  "Audio Playback active");


    ret = usb_composite_register(&uac_driver);


    return ret;
}

int usbd_uac_uninit(void)
{
	usb_composite_unregister();
    return 0;
}
/*================device driver end=====================*/

void usbd_uac_dqueue(void)
{	
	usb_ep_dequeue(uac->speaker.ep, uac->speaker.req[0]); //erk+
}
#endif //#if defined( CONFIG_MODULE_USBD_UAC_CLASS )
