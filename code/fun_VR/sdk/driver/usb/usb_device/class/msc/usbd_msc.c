/**
* @file
* this is usb device driver file
* usbd_uvc.c
* @author IP2/Luka
*/
#include "sonix_config.h"
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )

#include <stdio.h>
#include "usb_device.h"
#include "usbd_msc.h"
#include "composite.h"
#include "snx_udc.h"
#include <string.h>
#include <task.h>
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif


#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
#define USBD_MSC_MEDIA_DDR			1
#else
#define USBD_MSC_MEDIA_DDR			0
#endif
#define USBD_MSC_DRAM_SIZE			1*1024*1024
//#define USBD_MSC_DRAM_SIZE			5*1024*1024	//in opi/qpi
//#define USBD_MSC_MEDIA_ADDR			0x00A10000
#define USBD_MSC_MEDIA_ADDR			USBD_MSC_MEDIA_BUFF_PTR
#define MSC_STRING_INTERFACE        0
#define MSC_BLOCK_LENGTH			512

enum msc_event
{
	MSC_EVENT_IDLE,			    /*0*/
	MSC_EVENT_CBW,			    /*1*/
	MSC_EVENT_DATA,			    /*2*/
	MSC_EVENT_DATA_OK,		    /*3*/
	MSC_EVENT_CSW,			    /*4*/
};

enum msc_state
{
	MSC_STATE_IDLE,			    /*0*/
	MSC_STATE_COMMAND,		    /*1*/
	MSC_STATE_DATA,			    /*2*/
	MSC_STATE_DATA_BUSY,	    /*3*/
	MSC_STATE_STATUS,		    /*4*/
};

enum msc_type
{
	MSC_TYPE_NONE,				/*0*/
	MSC_TYPE_STANDARD_WRITE,	/*1*/
	MSC_TYPE_STANDARD_READ,		/*2*/
	MSC_TYPE_VENDOR_WRITE,		/*3*/
	MSC_TYPE_VENDOR_READ,		/*4*/
};

struct msc_device
{
	struct usb_function     func;
	enum msc_state          state;
	enum msc_event          event;

	enum msc_type           type;
	enum data_direction     data_dir;

	struct usb_ep           *bulk_in;
	struct usb_request      *bulk_in_req;
	struct usb_ep           *bulk_out;
	struct usb_request      *bulk_out_req;

	uint32_t                TransferLength;
	uint32_t                LBA;
	void                    *databuf;
	int32_t                 Residue;
	uint32_t                std_media_maxblock;

	uint32_t                sense_data;

	uint8_t                 media_remove_flag;
	uint8_t                 csw_status;
	uint8_t                 ro;	/* read only or write protect */
	uint8_t                 rst_recovery;
};

struct msc_inquiry_strings
{
	char                    *vendor_name;
	char                    *product_name;
	char                    *release;
};

static struct usb_device_descriptor     msc_device_desc = {
	sizeof(struct usb_device_descriptor),       // bLength
	USB_DT_DEVICE,                              // bDescriptorType
	0x0200,                                     // bcdUSB
	USB_CLASS_PER_INTERFACE,                    // bDeviceClass
	0,                                          // bDeviceSubClass
	0,                                          // bDeviceProtocol
	0,                                          // bMaxPacketSize0 		/* dynamic */
	MSC_VENDOR_ID,                              // idVendor
	MSC_PRODUCT_ID,                             // idProduct
	MSC_DEVICE_BCD,                             // bcdDevice
	0,                                          // iManufacturer		/* dynamic */
	0,                                          // iProduct  			/* dynamic */
	0,                                          // iSerialNumber 		/* dynamic */
	0                                           // bNumConfigurations 	/* dynamic */
};

static struct usb_qualifier_descriptor	msc_qualifier_desc = {
	sizeof(struct usb_qualifier_descriptor),    // bLength
	USB_DT_DEVICE_QUALIFIER,                    // bDescriptorType
	0x0200,                                     // bcdUSB
	USB_CLASS_MISC,                             // bDeviceClass
	0x02,                                       // bDeviceSubClass
	0x01,                                       // bDeviceProtocol
	0,                                          // bMaxPacketSize0 		/* dynamic */
	0,                                          // bNumConfigurations 	/* dynamic */
	0                                           // bRESERVED;
};
//=============================================================
//	variable
//=============================================================
xSemaphoreHandle                        usbd_msc_sema;
xSemaphoreHandle                        usbd_msc_stop;
//xSemaphoreHandle 						usbd_msc_state_mutex = NULL;
xQueueHandle                            msc_event_Queue;

static struct msc_device                *msc;
static struct usb_composite_driver      msc_driver;
static struct usb_configuration         msc_config_driver;
struct fsg_bulk_cb_wrap                 cbw;
struct fsg_bulk_cs_wrap                 *csw;
static uint32_t                         msc_task_run;
#if USBD_MSC_MEDIA_DDR == 1
uint8_t *USBD_MSC_DRAM_ADDRESS;
#endif



//=============================================================
//	callback function
//=============================================================
static usbd_drv_msc_detect_cb_t msc_detect_cb = NULL;
static usbd_drv_msc_std_cmd_rdata_cb_t msc_std_cmd_rdata_cb = NULL;
static usbd_drv_msc_std_cmd_wdata_cb_t msc_std_cmd_wdata_cb = NULL;


//=============================================================
//	description
//=============================================================
static struct usb_descriptor_header     *fsg_hs_function[4];
static struct usb_descriptor_header     *fsg_fs_function[4];
static struct usb_interface_descriptor  fsg_intf_desc ;
static struct usb_endpoint_descriptor   fsg_fs_bulk_in_desc ;
static struct usb_endpoint_descriptor   fsg_fs_bulk_out_desc ;
static struct usb_endpoint_descriptor   fsg_hs_bulk_in_desc ;
static struct usb_endpoint_descriptor   fsg_hs_bulk_out_desc ;

//=============================================================
//	string
//=============================================================
static struct usb_gadget_strings        msc_device_strings ;
static struct usb_gadget_strings        msc_function_strings;
static struct usb_string                msc_en_us_strings[2];
static struct usb_string                mass_storage_strings[5] ;
static struct msc_inquiry_strings       msc_inquiry_strs;

//=============================================================
//	function
//=============================================================
void msc_device_task( void *pvParameters );
static int  msc_bind(struct usb_composite_dev *cdev);
static int  msc_unbind(struct usb_composite_dev *cdev);
static int  msc_config_bind(struct usb_configuration *c);
static void msc_function_unbind(struct usb_configuration *c, struct usb_function *f);
static int msc_function_set_alt(struct usb_function *f, unsigned interface, unsigned alt);
static int msc_function_get_alt(struct usb_function *f, unsigned interface, unsigned alt);
/**
* @brief interface function - initial msc function
* @return value of error code.
*/
int usbd_msc_init(void)
{
	int ret;

	strcpy(mass_storage_strings[STRING_MANUFACTURER_IDX].s, mass_storage_vendor_label);
	strcpy(mass_storage_strings[STRING_PRODUCT_IDX].s, mass_storage_product_label);
	strcpy(mass_storage_strings[STRING_DESCRIPTION_IDX].s, mass_storage_config_label);
	strcpy(mass_storage_strings[STRING_SERIAL_NUM_IDX].s, mass_storage_serial_num_label);
	strcpy(mass_storage_strings[STRING_EMPTY_IDX].s, "");

	msc_inquiry_strs.vendor_name            = msc_vendor_name;
	msc_inquiry_strs.product_name           = msc_product_name;
	msc_inquiry_strs.release                = msc_release;

	msc_function_strings.language           = 0x0409;
	msc_function_strings.strings            = msc_en_us_strings;

	msc_device_strings.language	            = 0x0409;
	msc_device_strings.strings	            = mass_storage_strings;

	strcpy(msc_en_us_strings[MSC_STRING_INTERFACE].s, "Mass Storage interface");
	strcpy(msc_en_us_strings[1].s,"");

	strcpy(msc_driver.name,"msc driver");
	msc_driver.dev                          = &msc_device_desc;
	msc_driver.qualifier                    = &msc_qualifier_desc;
	msc_driver.strings                      = &msc_device_strings;
	msc_driver.bind                         = msc_bind;
	msc_driver.unbind                       = msc_unbind;

	msc_config_driver.label                 = mass_storage_config_label;
	msc_config_driver.bind                  = msc_config_bind;
	msc_config_driver.bConfigurationValue   = 1;
	msc_config_driver.iConfiguration        = 0; /* dynamic */
	msc_config_driver.bmAttributes          = USB_CONFIG_ATT_SELFPOWER;
	msc_config_driver.bMaxPower             = CONFIG_USB_GADGET_VBUS_DRAW / 2;

	//=============================================================
	//	description
	//=============================================================
	fsg_intf_desc.bLength                   = sizeof(fsg_intf_desc);
	fsg_intf_desc.bDescriptorType           = USB_DT_INTERFACE;
	fsg_intf_desc.bNumEndpoints             = 2;
	fsg_intf_desc.bInterfaceClass           = USB_CLASS_MASS_STORAGE;
	fsg_intf_desc.bInterfaceSubClass        = USB_SC_SCSI;
	fsg_intf_desc.bInterfaceProtocol        = USB_PR_BULK;

	fsg_fs_bulk_in_desc.bLength             = USB_DT_ENDPOINT_SIZE;
	fsg_fs_bulk_in_desc.bDescriptorType     = USB_DT_ENDPOINT;
	fsg_fs_bulk_in_desc.bEndpointAddress    = USB_DIR_IN;
	fsg_fs_bulk_in_desc.bmAttributes        = USB_ENDPOINT_XFER_BULK;
	fsg_fs_bulk_in_desc.wMaxPacketSize      = MSC_BLOCK_LENGTH / 8;
	fsg_fs_bulk_in_desc.bInterval           = 0x00;

	fsg_fs_bulk_out_desc.bLength            = USB_DT_ENDPOINT_SIZE,
	fsg_fs_bulk_out_desc.bDescriptorType    = USB_DT_ENDPOINT,
	fsg_fs_bulk_out_desc.bEndpointAddress   = USB_DIR_OUT,
	fsg_fs_bulk_out_desc.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	fsg_fs_bulk_out_desc.wMaxPacketSize     = MSC_BLOCK_LENGTH / 8;
	fsg_fs_bulk_out_desc.bInterval          = 0x00;

	fsg_hs_bulk_in_desc.bLength             = USB_DT_ENDPOINT_SIZE;
	fsg_hs_bulk_in_desc.bDescriptorType     = USB_DT_ENDPOINT;
	fsg_hs_bulk_in_desc.bmAttributes        = USB_ENDPOINT_XFER_BULK;
	fsg_hs_bulk_in_desc.wMaxPacketSize      = MSC_BLOCK_LENGTH;
	fsg_hs_bulk_in_desc.bInterval           = 1;	/* NAK every 1 uframe */

	fsg_hs_bulk_out_desc.bLength            = USB_DT_ENDPOINT_SIZE;
	fsg_hs_bulk_out_desc.bDescriptorType    = USB_DT_ENDPOINT;
	fsg_hs_bulk_out_desc.bmAttributes       = USB_ENDPOINT_XFER_BULK;
	fsg_hs_bulk_out_desc.wMaxPacketSize     = MSC_BLOCK_LENGTH;
	fsg_hs_bulk_out_desc.bInterval          = 1;	/* NAK every 1 uframe */

	fsg_hs_function[0]                      = (struct usb_descriptor_header *) &fsg_intf_desc;
	fsg_hs_function[1]                      = (struct usb_descriptor_header *) &fsg_hs_bulk_in_desc;
	fsg_hs_function[2]                      = (struct usb_descriptor_header *) &fsg_hs_bulk_out_desc;
	fsg_hs_function[3]                      = NULL;

	fsg_fs_function[0]                      = (struct usb_descriptor_header *) &fsg_intf_desc;
	fsg_fs_function[1]                      = (struct usb_descriptor_header *) &fsg_fs_bulk_in_desc;
	fsg_fs_function[2]                      = (struct usb_descriptor_header *) &fsg_fs_bulk_out_desc;
	fsg_fs_function[3]                      = NULL;

	ret = usb_composite_register(&msc_driver);
	if( ret != 0 ) {
		USBD_MSC_PRINT("USBD : MSC function initial error (%d)!!!\r\n",ret);
		goto fail;
	}
fail:
	return ret;
}
/**
* @brief interface function - bind  msc function
* @param c the pointer of usb configuration
* @param f the pointer of usb function
* @return value of error code.
*/
static int msc_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_ep *ep;
	int ret = -EINVAL;

	ep = usb_ep_autoconfig(cdev->gadget, &fsg_fs_bulk_in_desc, "MSC-BULK");
	if (!ep) {
		USBD_MSC_PRINT("Unable to allocate bulk_in_desc EP\r\n");
		goto error;
	}
	msc->bulk_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, &fsg_fs_bulk_out_desc, "MSC-BULK");
	if (!ep) {
		USBD_MSC_PRINT("Unable to allocate bulk_out_desc EP\r\n");
		goto error;
	}
	msc->bulk_out = ep;

	if ((ret = usb_interface_id(c, f)) < 0)
		goto error;
	fsg_intf_desc.bInterfaceNumber = ret;

	f->descriptors = usb_copy_descriptors(fsg_fs_function);
	if (!f->descriptors)
		return -ENOMEM;

	// Assume endpoint addresses are the same for both speeds
	fsg_hs_bulk_in_desc.bEndpointAddress =fsg_fs_bulk_in_desc.bEndpointAddress;
	fsg_hs_bulk_out_desc.bEndpointAddress =fsg_fs_bulk_out_desc.bEndpointAddress;
	f->hs_descriptors = usb_copy_descriptors(fsg_hs_function);
	if (!f->hs_descriptors) {
		usb_free_descriptors(f->descriptors);
		return -ENOMEM;
	}

	msc->bulk_in_req = usb_ep_alloc_request(msc->bulk_in);
	msc->bulk_in_req->buf = (uint8_t*)usbd_malloc(65536 * 1);
//    msc->bulk_in_req->buf = (uint8_t*)USBD_MSC_BKIN_BUFF_PTR;
	USBD_MSC_PRINT("msc->bulk_in_req->buf %x\n", msc->bulk_in_req->buf);
	if (msc->bulk_in_req == NULL || msc->bulk_in_req->buf == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	msc->bulk_out_req = usb_ep_alloc_request(msc->bulk_out);
	msc->bulk_out_req->length = 65536;
	//msc->bulk_out_req->buf = (uint8_t*)usbd_malloc(65536 * 1);
	msc->bulk_out_req->buf = msc->bulk_in_req->buf;
//    msc->bulk_out_req->buf = (uint8_t*)USBD_MSC_BKOUT_BUFF_PTR;
	USBD_MSC_PRINT("msc->bulk_out_req->buf %x\n", msc->bulk_out_req->buf);
	if (msc->bulk_out_req == NULL || msc->bulk_out_req->buf == NULL) {
		ret = -ENOMEM;
		goto error;
	}
	return 0;
error:
	msc_function_unbind(c, f);
	return ret;
}
/**
* @brief interface function - unbind  msc function
* @param c the pointer of usb configuration
* @param f the pointer of usb function
*/
static void msc_function_unbind(struct usb_configuration *c, struct usb_function *f)
{

}
/**
* @brief interface function - setup function
* @param f the pointer of usb function
* @param ctrl the request for control endpoint
* @return value of error code.
*/
static int msc_function_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct usb_ep *ep           = msc->func.config->cdev->gadget->ep0;
	struct usb_request *req     = msc->func.config->cdev->req;
	int ret                     = 0;

	uint16_t w_length           = ctrl->wLength;
	uint16_t w_index            = ctrl->wIndex;
	//uint16_t w_value            = ctrl->wValue;
	uint32_t event;

	USBD_MSC_PRINT("msc_function_setup:setup request %02x %02x value %04x index %04x %04x\r\n",ctrl->bRequestType, ctrl->bRequest, (ctrl->wValue),(ctrl->wIndex), (ctrl->wLength));
	switch (ctrl->bRequest) {
	case USB_REQ_CLEAR_FEATURE:
		event = MSC_EVENT_CSW;

		if(msc->rst_recovery == 0) {
			if((w_index & USB_DIR_IN) == USB_DIR_IN) {
				usb_ep_set_halt(msc->bulk_in, fsg_hs_bulk_in_desc.bEndpointAddress, 0);
			}
			else {
				usb_ep_set_halt(msc->bulk_out, fsg_hs_bulk_out_desc.bEndpointAddress, 0);
			}
		}

		xQueueSendToBack(msc_event_Queue,&event,0);
		return ret;

	case USB_BULK_RESET_REQUEST:
		USBD_MSC_PRINT("MSC-bulk reset request\r\n");

		msc->rst_recovery = 0;

		if (ctrl->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE)) {
			break;
		}

		if((ctrl->wValue != 0) || (ctrl->wLength != 0)) {
			break;//ret = -EOPNOTSUPP;
		}

		ret = 0x10000;
		return ret;

	case USB_BULK_GET_MAX_LUN_REQUEST:
		USBD_MSC_PRINT("MSC-get max LUN\n");

		if (ctrl->wIndex < MAX_CONFIG_INTERFACES) {
			if((ctrl->bRequestType & 0x1f) != 1) { //Invalid recipient!
				break;//ret = -EOPNOTSUPP;
			}

			if(ctrl->wValue != 0) { //Invalid wValue!
				break;//ret = -EOPNOTSUPP;
			}

			if(ctrl->wLength != 1) { //Invalid length!
				break;//ret = -EOPNOTSUPP;
			}

			if((ctrl->wIndex & 0xff) != 0) { //Invalid interface!
				break;//ret = -EOPNOTSUPP;
			}

			req->buf[0] = 0;
			/* Respond with data/status */
			req->length = min((uint16_t)1, w_length);
			ret = usb_ep_queue(ep, req);

			if (ret != 0 && ret != -ESHUTDOWN) {
				USBD_MSC_PRINT("MSC-error in submission: --> %d\n", ret);
			}
		}
		else { //Invalid interface!
			break;//ret = -EOPNOTSUPP;
		}
//			if(ret != -EOPNOTSUPP){
//				req->buf[0] = 0;

//				/* Respond with data/status */
//				req->length = min((uint16_t)1, w_length);

//				ret = usb_ep_queue(ep, req);

//				if (ret != 0 && ret != -ESHUTDOWN) {
//					USBD_MSC_PRINT("MSC-error in submission: --> %d\n", ret);
//				}
//			}

		return ret;
	}

	USBD_MSC_PRINT("MSC-unknown class-specific control req ""%02x.%02x v%04x i%04x l%u\r\n", ctrl->bRequestType, ctrl->bRequest,ctrl->wValue, ctrl->wIndex, ctrl->wLength);
	return -EOPNOTSUPP;
}

static void msc_function_disable(struct usb_function *f)
{

}
/**
* @brief interface function - endpoint 1 callback function
* @param f the pointer of usb function
* @param ep the pointer of usb endpoint
*/
static void msc_functione_ep12_callback(struct usb_function *f, struct usb_ep *ep)
{
	uint32_t event;

	USBD_MSC_PRINT("%s-%d\r\n",__FUNCTION__,__LINE__);

	if(!msc_task_run || !msc) {
		return;
	}

	if((ep->state & USBD_EP_STATE_IN) == USBD_EP_STATE_IN) {
		ep->state &= ~USBD_EP_STATE_IN;
	}
	else if((ep->state & USBD_EP_STATE_TOC) ==  USBD_EP_STATE_TOC) {
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC );
		if(msc) {
			//printf("ent:%x\n", msc->event);
			//printf("sta:%x\n", msc->state);
//            if(msc->state == MSC_STATE_DATA){
//                event = MSC_EVENT_CSW;
//            }
//            else {
//                event = MSC_EVENT_IDLE;
//            }

//            if(msc_event_Queue){
//                printf("v\n");
//                xQueueSendToBack(msc_event_Queue, &event, 0);
//            }
			//xSemaphoreTake(usbd_msc_state_mutex, portMAX_DELAY);
			if(msc->state == MSC_STATE_DATA) {  //complete to send data
				vTaskDelay(1);	
				event = MSC_EVENT_CSW;
			}
			else {                              //complete to send csw
				event = MSC_EVENT_IDLE;
			}
			//xSemaphoreGive(usbd_msc_state_mutex);
			if(msc_event_Queue) {
				xQueueSendToBack(msc_event_Queue, &event, 0);
			}
		}
	}
}
/**
* @brief interface function - endpoint 2 callback function
* @param f the pointer of usb function
* @param ep the pointer of usb endpoint
*/
static void msc_functione_ep13_callback(struct usb_function *f,struct usb_ep *ep)
{
	struct usb_request *req = msc->bulk_out_req;
	uint32_t event = 0;

	USBD_MSC_PRINT("%s-%d\r\n",__FUNCTION__,__LINE__);

	if(!msc_task_run || !msc) {
		return;
	}
	if((ep->state & USBD_EP_STATE_OUT) == USBD_EP_STATE_OUT) {
		ep->state &= ~USBD_EP_STATE_OUT;
		ep->state |= (USBD_EP_STATE_BUSY);
	}
	else if((ep->state & USBD_EP_STATE_TOC) ==  USBD_EP_STATE_TOC) {
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC );
		switch(msc->state){
			case MSC_STATE_IDLE:
			case MSC_STATE_STATUS:
                //recieve CBW
    			event = MSC_EVENT_CBW;
    				if(req && req->buf){
    				memcpy(&cbw,req->buf, sizeof(struct fsg_bulk_cb_wrap));
    			}

    			if(msc && msc_event_Queue)
    				xQueueSendToBack(msc_event_Queue,&event,0);
    				
    			if(usbd_msc_sema) {
    				xSemaphoreTake(usbd_msc_sema, portMAX_DELAY);
    				USBD_MSC_PRINT("%s-%d\r\n",__FUNCTION__,__LINE__);
    			}
				break;
			case MSC_STATE_DATA:
			case MSC_STATE_DATA_BUSY:
                //recieve Data
    			event = MSC_EVENT_DATA_OK;
    			if(msc_event_Queue)
    				xQueueSendToBack(msc_event_Queue,&event,0);
    				break;
			default:
				USBD_MSC_PRINT("\r ERROR\n");
				
		}
	}
}
/**
* @brief interface function - configuration bind  function
* @param c the pointer of usb configuration
* @return value of error code.
*/
static int  msc_config_bind(struct usb_configuration *c)
{
	int ret;

	if (msc == NULL) {
		msc = (struct msc_device*)usbd_malloc(sizeof(struct msc_device));
		if (msc == NULL)
			return -ENOMEM;
	}

	memset(msc, 0x00, sizeof(struct msc_device));
	msc->state = MSC_STATE_IDLE;
	msc->event = MSC_EVENT_IDLE;
	msc->media_remove_flag = 0;
	//msc->sense_data = SS_NO_SENSE;
	msc->sense_data = SS_MEDIUM_REMOVAL_PREVENTED;
	msc->ro = 0;
	if ((ret = usb_string_id(c->cdev)) < 0)
		goto error;

	msc_en_us_strings[MSC_STRING_INTERFACE].id = ret;

	fsg_intf_desc.iInterface = 0;

	msc->func.name              = "Mass Storage Function";
	msc->func.strings           = &msc_function_strings;
	msc->func.bind              = msc_function_bind;
	msc->func.unbind            = msc_function_unbind;
	msc->func.setup             = msc_function_setup;
	msc->func.set_alt           = msc_function_set_alt;
	msc->func.disable           = msc_function_disable;
	msc->func.get_alt           = msc_function_get_alt;
	msc->func.ep_callback[12]   = msc_functione_ep12_callback;
	msc->func.ep_callback[13]   = msc_functione_ep13_callback;

	ret = usb_add_function(c, &msc->func);
	if(ret < 0)
		goto error;
	msc->std_media_maxblock = 1*1024*1024 / 512;
#if(USBD_MSC_MEDIA_DDR == 1)
	msc->std_media_maxblock = USBD_MSC_DRAM_SIZE / 512;

//    USBD_MSC_DRAM_ADDRESS = (uint8_t*)USBD_MSC_MEDIA_ADDR;
	USBD_MSC_DRAM_ADDRESS = (uint8_t*)usbd_malloc(USBD_MSC_DRAM_SIZE);
#endif
	//usbd_msc_sema = 0;
	usbd_msc_sema = xSemaphoreCreateCounting(1,0);
	if(!usbd_msc_sema) {
		USBD_MSC_PRINT(" create usbd_msc_sema is fail~~~\r\n");
	}

	usbd_msc_stop = xSemaphoreCreateCounting(1,0);
	if(!usbd_msc_stop) {
		USBD_MSC_PRINT(" create usbd_msc_stop is fail~~~\r\n");
	}

	//usbd_msc_state_mutex =  xSemaphoreCreateMutex();
	//if(!usbd_msc_state_mutex){
	//	USBD_MSC_PRINT("create usbd_msc_state_mutex is fail~~~\r\n");
	//}
	
	msc_task_run = 1;
	msc_event_Queue = xQueueCreate(20, sizeof(uint32_t));
	USBD_MSC_PRINT(" create msc_device_task\r\n");

	if (pdPASS != xTaskCreate(msc_device_task, "msc_device_task", 4096, (void*) NULL, 250, NULL)) {
		USBD_MSC_PRINT("Could not msc_device_task\r\n");
	}

	return 0;
error:
	if (msc != NULL) {
		//vPortFree(msc);
		msc = NULL ;
	}
	return ret;
}
/**
* @brief interface function - msc  bind function
* @param cdev the pointer of usb compisite device
* @return value of error code.
*/
static int msc_bind(struct usb_composite_dev *cdev)
{
	int ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	mass_storage_strings[STRING_MANUFACTURER_IDX].id = ret;
	msc_device_desc.iManufacturer = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	mass_storage_strings[STRING_PRODUCT_IDX].id = ret;
	msc_device_desc.iProduct = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	mass_storage_strings[STRING_SERIAL_NUM_IDX].id = ret;
	msc_device_desc.iSerialNumber = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	mass_storage_strings[STRING_DESCRIPTION_IDX].id = ret;

	msc_config_driver.iConfiguration = 0;

	if ((ret = usb_add_config(cdev, &msc_config_driver)) < 0)
		goto error;

	USBD_MSC_PRINT("Sonix Mass Storage Gadget\r\n");
	return 0;

error:
	msc_unbind(cdev);
	return ret;
}

/**
* @brief interface function - msc  unbind function
* @param cdev the pointer of usb compisite device
* @return value of error code.
*/
static int msc_unbind(struct usb_composite_dev *cdev)
{
	msc_function_unbind(cdev->config, &msc->func);
	if (msc != NULL) {
		//vPortFree(msc);
		msc = NULL ;
	}
	return 0;
}

/**
* @brief interface function - msc class function : do inquiry
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_inquiry(uint8_t* buf)
{
	/* Vendor (8 chars), product (16 chars), release (4 * hexadecimal digits) and NUL byte */
	char inquiry_string[8 + 16 + 4 + 1];

	USBD_MSC_PRINT( "%s\r\n",__FUNCTION__);

	if(cbw.DataTransferLength == 0) { //for usb-if msc test - command set test
		msc->data_dir = DATA_DIR_NONE;
		return 0;
	}
	else {
		buf[0] = TYPE_DISK;
		buf[1] = 0x80;
		buf[2] = 2;         /* ANSI SCSI level 2 */
		buf[3] = 2;         /* SCSI-2 INQUIRY data format */
		buf[4] = 31;        /* Additional length */
		buf[5] = 0;         /* no special options */
		buf[6] = 0;
		buf[7] = 0;

		snprintf(inquiry_string, sizeof(inquiry_string), "%-8s%-16s%-4s",msc_inquiry_strs.vendor_name, msc_inquiry_strs.product_name, msc_inquiry_strs.release);

		memcpy(buf + 8, inquiry_string, sizeof(inquiry_string));
		msc->data_dir = DATA_DIR_TO_HOST;
		return 36;
	}
}

static uint32_t __bswap_32(uint32_t __x)
{
	return ((__x>>24) | (__x>>8&0xff00) | (__x<<8&0xff0000) | (__x<<24));
}
static uint16_t __bswap_16(uint16_t __x)
{
	return ((__x<<8) | (__x>>8));
}

/**
* @brief interface function - msc class function : read format capacities
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_read_format_capacities(uint8_t* buf)
{
	uint32_t val32;
	uint16_t val16;

	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	buf[0]=0x00;
	buf[1]=0x00;
	buf[2]=0x00;
	buf[3]=0x10;
	buf += 4;

	val32=	__bswap_32(msc->std_media_maxblock);
	memcpy(buf,&val32,sizeof(uint32_t));
	buf += 4;

	/* descriptor code = 0x02 */
	buf[0] = 0x02;
	buf[1] = 0x00;
	buf += 2;

	/* Block length = 0x0200 */
	val16=	__bswap_16(MSC_BLOCK_LENGTH);
	memcpy(buf,&val16,sizeof(uint16_t));

	msc->data_dir = DATA_DIR_TO_HOST;
	return 12;
}


/**
* @brief interface function - msc class function : do mode select
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_mode_select(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	/* No support MODE SELECT */
	msc->sense_data = SS_INVALID_COMMAND;
	msc->data_dir = DATA_DIR_NONE;
	return 0;
}
/**
* @brief interface function - msc class function : do mode sense
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_mode_sense(uint8_t* buf)
{
	int len;

	memset(buf, 0, 8);
	if(cbw.CDB[0] == SC_MODE_SENSE_6) {
		len = 4;
		buf[1] = TYPE_DISK;
		buf[2] = (msc->ro ? 0x80 : 0x00);		/* Write Protect */
		msc->Residue = cbw.CDB[4];
	} else {
		len = 8;
		buf[2] = TYPE_DISK;
		buf[3] = (msc->ro ? 0x80 : 0x00);		/* Write Protect */
		msc->Residue = (cbw.CDB[7]<<8) | cbw.CDB[8];
	}
	buf[0] = len - 1;

	msc->data_dir = DATA_DIR_TO_HOST;
	return len;

}
/**
* @brief interface function - msc class function : do prevent allow
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_prevent_allow(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);

	msc->data_dir = DATA_DIR_NONE;
	//msc->csw_status = USB_STATUS_FAIL;

	if (cbw.CDB[4] & 0x01) {
		msc->sense_data = SS_INVALID_FIELD_IN_CDB;
	}

	return 0;
}

/**
* @brief interface function - msc class function : do read
* @return the lenth of buffer.
*/
static int msc_cmd_do_read(void)
{
	uint32_t lba, len;

	// print_msg("%s\r\n",__FUNCTION__);

	msc->TransferLength= 0;
	msc->LBA = 0;

	if (cbw.CDB[0] == SC_READ_6) {
		lba = ((cbw.CDB[1] & 0x1f)<<16) | (cbw.CDB[2]<<8) | cbw.CDB[3];
		len = cbw.CDB[4];
	} else {
		lba = (cbw.CDB[2]<<24) | (cbw.CDB[3]<<16) | (cbw.CDB[4]<<8) | cbw.CDB[5];
		if (cbw.CDB[0] == SC_READ_10) {
			len = (cbw.CDB[7]<<8) + cbw.CDB[8];
		} else {
			len = (cbw.CDB[6]<<24) | (cbw.CDB[7]<<16) | (cbw.CDB[8]<<8) | cbw.CDB[9];
		}

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = don't read from the
		 * cache), but we don't implement them. */
		if ((cbw.CDB[1] & 0x18) != 0) {
			msc->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}

	if (lba >= msc->std_media_maxblock) {
		msc->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	if(cbw.Flags != USB_DIR_IN) { //case(10)Ho<>Di
		msc->TransferLength = cbw.DataTransferLength;
		msc->csw_status = USB_STATUS_FAIL;
		//msc->Residue = cbw.DataTransferLength + (msc->TransferLength<<9);
		msc->Residue = cbw.DataTransferLength + msc->TransferLength;
		msc->type = MSC_TYPE_STANDARD_WRITE;
		msc->data_dir = DATA_DIR_FROM_HOST;
		//return (msc->TransferLength<<9);
		return msc->TransferLength;
	}
	else if((cbw.DataTransferLength == 0) && (cbw.DataTransferLength != (len * MSC_BLOCK_LENGTH))) { //case(2)Hn < Di
		msc->data_dir = DATA_DIR_NONE;
		msc->Residue = 0;
		return -EINVAL;
	}
	else if(cbw.DataTransferLength > (len * MSC_BLOCK_LENGTH)) { //case(4)Hi > Dn; case(5)Hi > Di
		msc->Residue = cbw.DataTransferLength;
		return -EINVAL;
	}
	else if(cbw.DataTransferLength < (len *MSC_BLOCK_LENGTH)) { //case(7)Hi < Di
		msc->Residue = cbw.DataTransferLength;
		return -EINVAL;
	}

	msc->TransferLength = len;
	msc->LBA = lba;
	USBD_MSC_PRINT("LBA =  %08x ,TransferLength = %08x",msc->LBA,msc->TransferLength);
	msc->type = MSC_TYPE_STANDARD_READ;
	msc->data_dir = DATA_DIR_TO_HOST;

	return (msc->TransferLength<<9);

}
/**
* @brief interface function - msc class function : do read repacity
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_read_capacity(uint8_t* buf)
{

	uint32_t val32;

	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	val32=	__bswap_32(msc->std_media_maxblock-1);

	if(cbw.CDB[0] == SC_READ_CAPACITY_16) {
		memset(buf, 0x00, 32);
		buf += 4;
		memcpy(buf,&val32,sizeof(uint32_t));
		/* Block length = 0x0200 */
		buf += 4;
		buf[0]=0x00;
		buf[1]=0x00;
		buf[2]=0x02;
		buf[3]=0x00;
		msc->data_dir = DATA_DIR_TO_HOST;
		return 32;
	}
	else {
		memset(buf, 0x00, 8);
		memcpy(buf,&val32,sizeof(uint32_t));
		/* Block length = 0x0200 */
		buf += 4;
		buf[0]=0x00;
		buf[1]=0x00;
		buf[2]=0x02;
		buf[3]=0x00;
		msc->data_dir = DATA_DIR_TO_HOST;
		return 8;
	}
}
/**
* @brief interface function - msc class function : do read header
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_read_header(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	msc->data_dir = DATA_DIR_TO_HOST;
	return 8;
}
/**
* @brief interface function - msc class function : do read toc
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_read_toc(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	msc->data_dir = DATA_DIR_TO_HOST;
	return 20;
}
/**
* @brief interface function - msc class function : do request sense
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_request_sense(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);

	if(cbw.DataTransferLength == 0) { //for usb-if msc test - command set test
		msc->data_dir = DATA_DIR_NONE;
		return 0;
	}
	else {
		msc->data_dir = DATA_DIR_TO_HOST;
		memset(buf, 0, 18);
		buf[0] = 0x70;
		buf[2] = SK(msc->sense_data);
		buf[7] = 10;
		buf[12] = ASC(msc->sense_data);
		buf[13] = ASCQ(msc->sense_data);
		return cbw.DataTransferLength;
	}
}
/**
* @brief interface function - msc class function : do start stop
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_start_stop(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	msc->data_dir = DATA_DIR_NONE;
	return 0;
}
/**
* @brief interface function - msc class function : do synchronize cache
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_synchronize_cache(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	msc->data_dir = DATA_DIR_NONE;
	return 0;
}
/**
* @brief interface function - msc class function : do verify
* @param buf the pointer of buffer
* @return the lenth of buffer.
*/
static int msc_cmd_do_verify(uint8_t* buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	msc->data_dir = DATA_DIR_NONE;
	return 0;
}
/**
* @brief interface function - msc class function : do test unit ready
* @return the lenth of buffer.
*/
static int msc_cmd_do_test_unit_ready(void)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);


	if(cbw.DataTransferLength != 0) { //case(9)Ho > Dn
		msc->csw_status = USB_STATUS_FAIL;
		msc->TransferLength = cbw.DataTransferLength;
		msc->Residue = cbw.DataTransferLength + msc->TransferLength;
		msc->type = MSC_TYPE_STANDARD_WRITE;
		msc->data_dir = DATA_DIR_FROM_HOST;
		return msc->TransferLength;
	}

	if (msc->media_remove_flag) {
		msc->csw_status = USB_STATUS_FAIL;
	}

	msc->data_dir = DATA_DIR_NONE;
	return 0;
}
/**
* @brief interface function - msc class function : do write
* @return error code.
*/
static int msc_cmd_do_write(void)
{
	uint32_t lba, len;

	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);

	if (msc->ro) {
		msc->sense_data = SS_WRITE_PROTECTED;
		return -EINVAL;
	}

	if (cbw.CDB[0] == SC_WRITE_6) {
		lba = ((cbw.CDB[1] & 0x1f)<<16) | (cbw.CDB[2]<<8) | cbw.CDB[3];
		len = cbw.CDB[4];
	} else {
		lba = (cbw.CDB[2]<<24) | (cbw.CDB[3]<<16) | (cbw.CDB[4]<<8) | cbw.CDB[5];
		if (cbw.CDB[0] == SC_WRITE_10) {
			len = (cbw.CDB[7]<<8) + cbw.CDB[8];
		} else {
			len = (cbw.CDB[6]<<24) | (cbw.CDB[7]<<16) | (cbw.CDB[8]<<8) | cbw.CDB[9];
		}

		/* We allow DPO (Disable Page Out = don't save data in the
		* cache) and FUA (Force Unit Access = don't read from the
		* cache), but we don't implement them. */
		if ((cbw.CDB[1] & 0x18) != 0) {
			msc->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}

	if (lba >= msc->std_media_maxblock) {
		msc->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	if(cbw.Flags != USB_DIR_OUT) { //case(8)Hi<>Do
		msc->Residue = cbw.DataTransferLength;
		return -EINVAL;
	}
	else if((cbw.DataTransferLength == 0) && (cbw.DataTransferLength != (len * MSC_BLOCK_LENGTH))) { //case(3)Hn < Do
		msc->csw_status = USB_STATUS_FAIL;
		msc->Residue = 0;
		msc->data_dir = DATA_DIR_NONE;
		return -EINVAL;
	}
	else if(cbw.DataTransferLength > (len * MSC_BLOCK_LENGTH)) { //case(11)Ho > Do
		msc->csw_status = USB_STATUS_FAIL;
		msc->TransferLength = cbw.DataTransferLength;
		msc->Residue = cbw.DataTransferLength + msc->TransferLength;
		msc->LBA = lba;
		msc->type = MSC_TYPE_STANDARD_WRITE;
		msc->data_dir = DATA_DIR_FROM_HOST;
		return msc->TransferLength;
	}
	else if(cbw.DataTransferLength < (len * MSC_BLOCK_LENGTH)) { //case(13)Ho < Do
		msc->csw_status = USB_STATUS_FAIL;
		msc->TransferLength = cbw.DataTransferLength;
		msc->Residue = cbw.DataTransferLength + msc->TransferLength;
		msc->LBA = lba;
		msc->type = MSC_TYPE_STANDARD_WRITE;
		msc->data_dir = DATA_DIR_FROM_HOST;
		return msc->TransferLength;
	}

	msc->TransferLength = len;
	msc->LBA = lba;
	USBD_MSC_PRINT("LBA =  %08x ,TransferLength = %08x",msc->LBA,msc->TransferLength);
	msc->type = MSC_TYPE_STANDARD_WRITE;
	msc->data_dir = DATA_DIR_FROM_HOST;

	return (msc->TransferLength<<9);
}

/**
* @brief interface function - msc class function : do vendor
* @param cmd the pointer of command
* @return the lenth of buffer.
*/
static int msc_cmd_do_vendor(uint8_t* cmd, uint32_t data_len, uint8_t flags)
{
	USBD_MSC_PRINT("%s\r\n", __FUNCTION__);

	/****
	do something for vendor application
	****/


	if (data_len == 0) {
		msc->type = MSC_TYPE_NONE;
		msc->data_dir = DATA_DIR_NONE;
	} else if ((flags & 0x80) == 0x80) {
		msc->type = MSC_TYPE_VENDOR_READ;
		msc->data_dir = DATA_DIR_TO_HOST;
	} else {
		msc->type = MSC_TYPE_VENDOR_WRITE;
		msc->data_dir = DATA_DIR_FROM_HOST;
	}

	msc->TransferLength = data_len;

	return 0;
}
/**
* @brief interface function - check CBW command
* @return the value of error code.
*/
static int msc_check_cbw(void)
{
	/* Is the CBW valid? */
	if (cbw.Signature != USB_BULK_CB_SIG) {
		printf("invalid CBW:  sig 0x%x\r\n",cbw.Signature);
		return -EINVAL;
	}

	/* Is the CBW meaningful? */
	if (cbw.Lun >= FSG_MAX_LUNS || cbw.Flags & ~USB_BULK_IN_FLAG ||	cbw.Length <= 0 || cbw.Length > MAX_COMMAND_SIZE) {
		printf("non-meaningful CBW: lun = %u, flags = 0x%x, ""cmdlen %u\r\n",cbw.Lun, cbw.Flags, cbw.Length);
		return -EINVAL;
	}
	msc->Residue = cbw.DataTransferLength;
	msc->type = MSC_TYPE_NONE;
	return 0;
}

/**
* @brief interface function - do SCSI command
* @return the value of error code.
*/
static int do_scsi_command(void)
{
	int32_t reply = -EINVAL;
	uint8_t *in_data_buf = msc->bulk_in_req->buf;

	USBD_MSC_PRINT( "%s\r\n", __FUNCTION__);
	USBD_MSC_PRINT( "cbw.CDB[0] = %08x\r\n", cbw.CDB[0]);
	USBD_MSC_PRINT( "cbw.Tag = %08x\r\n", cbw.Tag);

	msc->csw_status = USB_STATUS_PASS;
	switch (cbw.CDB[0]) {
	case SC_INQUIRY:
		reply = msc_cmd_do_inquiry(in_data_buf);
		reply = (reply <= cbw.DataTransferLength) ? reply: cbw.DataTransferLength;
		break;

	case SC_READ_FORMAT_CAPACITIES:
		reply = msc_cmd_do_read_format_capacities(in_data_buf);
		break;

	case SC_MODE_SELECT_6:
	case SC_MODE_SELECT_10:
		reply = msc_cmd_do_mode_select(in_data_buf);
		break;

	case SC_MODE_SENSE_6:
	case SC_MODE_SENSE_10:
		reply = msc_cmd_do_mode_sense(in_data_buf);
		break;

	case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:
		reply = msc_cmd_do_prevent_allow(in_data_buf);
		break;

	case SC_READ_6:
	case SC_READ_10:
	case SC_READ_12:
		reply = msc_cmd_do_read();
		break;

	case SC_READ_CAPACITY:
	case SC_READ_CAPACITY_16:
		reply = msc_cmd_do_read_capacity(in_data_buf);
		break;

	case SC_READ_HEADER:
		reply = msc_cmd_do_read_header(in_data_buf);
		break;

	case SC_READ_TOC:
		reply = msc_cmd_do_read_toc(in_data_buf);
		break;

	//?
	case SC_REQUEST_SENSE:
		reply = msc_cmd_do_request_sense(in_data_buf);
		break;

	case SC_START_STOP_UNIT:
		reply = msc_cmd_do_start_stop(in_data_buf);
		break;

	case SC_SYNCHRONIZE_CACHE:
		reply = msc_cmd_do_synchronize_cache(in_data_buf);
		break;

	case SC_TEST_UNIT_READY:
		reply = msc_cmd_do_test_unit_ready();
		break;

	case SC_VERIFY:
		reply = msc_cmd_do_verify(in_data_buf);
		break;

	case SC_WRITE_6:
	case SC_WRITE_10:
	case SC_WRITE_12:
		reply = msc_cmd_do_write();
		break;

	case SC_SNX_STD_CMD:
	case SC_SNX_ICV_CMD:
	case SC_USR_STD_CMD:
		reply = msc_cmd_do_vendor(cbw.CDB, cbw.DataTransferLength, cbw.Flags);
		break;

	case SC_FORMAT_UNIT:
	case SC_RELEASE:
	case SC_RESERVE:
	case SC_SEND_DIAGNOSTIC:
	default:
		break;
	}

	if (reply == -EINTR)
		return -EINTR;

	if (reply == -EINVAL)
		return -EINVAL; /* Error reply length */

	msc->Residue -= reply;

	if(msc->data_dir == DATA_DIR_TO_HOST)
		msc->bulk_in_req->length=reply;
	else if(msc->data_dir == DATA_DIR_FROM_HOST)
		msc->bulk_out_req->length=reply;
	return 0;
}
/**
* @brief interface function - send msc status(CSW)
* @return the value of error code.
*/
static int send_status(void)
{

	uint8_t* 	in_data_buf	= msc->bulk_in_req->buf;
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	csw = (struct fsg_bulk_cs_wrap*)in_data_buf;
	msc->bulk_in_req->length = USB_BULK_CS_WRAP_LEN;
	memset(in_data_buf,0x00,USB_BULK_CS_WRAP_LEN);
	csw->Signature = USB_BULK_CS_SIG;
	csw->Tag = cbw.Tag;
	csw->Residue= msc->Residue;
	csw->Status = msc->csw_status;

	return 0;
}

/**
* @brief interface function - set alternate setting
* @param f the pointer of usb function
* @param intf the number of interface
* @param alt the number of alternate setting
* @return the value of error code.
*/
static int msc_function_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	enum usb_device_speed speed_mode;

	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);

	speed_mode = f->config->cdev->gadget->speed;

	switch (alt) {
	case 0:

		if(speed_mode == USB_SPEED_HIGH) {
			if(msc->bulk_in) {
				usb_ep_enable(msc->bulk_in, &fsg_hs_bulk_in_desc);
			}
			if(msc->bulk_out) {
				usb_ep_enable(msc->bulk_out, &fsg_hs_bulk_out_desc);
			}
		}
		else if(speed_mode == USB_SPEED_FULL) {
			if(msc->bulk_in) {
				usb_ep_enable(msc->bulk_in, &fsg_fs_bulk_in_desc);
			}
			if(msc->bulk_out) {
				usb_ep_enable(msc->bulk_out, &fsg_fs_bulk_out_desc);
			}
		}
		msc->event = MSC_EVENT_IDLE;
		msc->state = MSC_STATE_IDLE;

		break;

	case 1:
		break;

	default:
		return -EINVAL;
	}

	return 0;
}
/**
* @brief interface function - get alternate setting
* @param f the pointer of usb function
* @param interface the number of interface
* @return the value of error code.
*/
static int msc_function_get_alt(struct usb_function *f, unsigned interface, unsigned alt)
{
	return 0;
}
/**
* @brief interface function - MSC State Machine : do IDLE
*/
static void msc_do_idle(void)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	msc->state = MSC_STATE_IDLE ;
}
/**
* @brief interface function - MSC State Machine : do CBW
*/
static void msc_do_cbw(void)
{
	int32_t ret;
	uint32_t event ;

	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
	if(!msc)return;

	ret = msc_check_cbw();
	if(msc->bulk_out_req->length != USB_BULK_CB_WRAP_LEN)
		ret = -EINVAL;

	if(ret == -EINVAL) {
		usb_ep_set_halt(msc->bulk_in, fsg_hs_bulk_in_desc.bEndpointAddress, 1);
		usb_ep_set_halt(msc->bulk_out, fsg_hs_bulk_out_desc.bEndpointAddress, 1);
		msc->csw_status = USB_STATUS_FAIL;
		msc->state = MSC_STATE_DATA;
		msc->rst_recovery = 1;
		if(usbd_msc_sema)
			xSemaphoreGive(usbd_msc_sema);
		return;
	}

	ret = do_scsi_command();

	if(ret == -EINVAL) {
		usb_ep_set_halt(msc->bulk_in, fsg_hs_bulk_in_desc.bEndpointAddress, 1);
		msc->csw_status = USB_STATUS_FAIL;
		event = msc->event;
		if(usbd_msc_sema) {
			xSemaphoreGive(usbd_msc_sema);
		}
	} else if(msc->data_dir == DATA_DIR_NONE) {
		event = MSC_EVENT_CSW;
	} else {
		event = MSC_EVENT_DATA;
	}
	msc->state = MSC_STATE_COMMAND ;
	USBD_MSC_PRINT("msc->state = %d event = %d msc->data_dir = %d\r\n",msc->state,msc->event,msc->data_dir);
	if(msc_event_Queue)
		xQueueSendToBack(msc_event_Queue,&event,0);
}
/**
* @brief interface function - MSC State Machine : do DATA
*/
static void msc_do_data(void)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);

	if(msc->type == MSC_TYPE_STANDARD_READ || msc->type == MSC_TYPE_STANDARD_WRITE) {
		/* msc read */
		if(msc->data_dir == DATA_DIR_TO_HOST) {
			USBD_MSC_PRINT("msc_do_data->read \r\n");
			if(msc) {
				msc->databuf = msc->bulk_in_req->buf;
				msc->state = MSC_STATE_DATA;
				msc->bulk_in_req->length = msc->TransferLength << 9;
			}
			usbd_msc_read_data(msc->LBA, msc->TransferLength,msc->databuf);
			if(msc)
				usb_ep_queue(msc->bulk_in, msc->bulk_in_req);
		}
		/* msc write */
		else {
			USBD_MSC_PRINT("msc_do_data->write\r\n");
			msc->state = MSC_STATE_DATA_BUSY;

			//msc->bulk_out_req->length = msc->TransferLength << 9;

			//usb_ep_queue(msc->bulk_out, msc->bulk_out_req);
		}
	}
	else if(msc->type == MSC_TYPE_VENDOR_READ || msc->type == MSC_TYPE_VENDOR_WRITE) {
		/*  read */
		if(msc->data_dir == DATA_DIR_TO_HOST) {
			USBD_MSC_PRINT("msc_do_vendor_data->read \r\n");

			msc->state = MSC_STATE_DATA;
			msc->bulk_in_req->length = msc->TransferLength;

			msc->databuf = msc->bulk_in_req->buf;
			/*
						if (msc_vendor_cmd_rdata_cb != NULL) {
							msc_vendor_cmd_rdata_cb(cbw.CDB, msc->TransferLength, msc->databuf);
						}

			*/			usb_ep_queue(msc->bulk_in, msc->bulk_in_req);
		}
		/* msc write */
		else {
			USBD_MSC_PRINT("msc_do_vendor_data->write\r\n");
			msc->state = MSC_STATE_DATA_BUSY;

			msc->bulk_out_req->length = msc->TransferLength;
			//usb_ep_queue(msc->bulk_out, msc->bulk_out_req);
		}
	}
	else {
		USBD_MSC_PRINT("msc_do_data->read \r\n");
		msc->state = MSC_STATE_DATA;
		usb_ep_queue(msc->bulk_in, msc->bulk_in_req);
	}
	if(usbd_msc_sema)
		xSemaphoreGive(usbd_msc_sema);
}
/**
* @brief interface function - MSC State Machine : do DATA finish
*/
static void msc_do_data_finish(void)
{
	uint32_t event;

	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);

	if(msc->type == MSC_TYPE_STANDARD_READ || msc->type == MSC_TYPE_STANDARD_WRITE) {
		msc->databuf = msc->bulk_out_req->buf;
		usbd_msc_write_data(msc->LBA, msc->TransferLength, msc->databuf);
	}
	else { /*if(msc->type == MSC_TYPE_VENDOR_READ || msc->type == MSC_TYPE_VENDOR_WRITE)*/
		/*
		 		if (msc_vendor_cmd_wdata_cb != NULL) {
					msc->databuf = msc->bulk_out_req->buf;
		 			msc_vendor_cmd_wdata_cb(cbw.CDB, msc->TransferLength, msc->databuf);

		 		}*/
	}

	event = MSC_EVENT_CSW;
	if(msc) {
		msc->state = MSC_STATE_DATA ;
		if(msc_event_Queue)
			xQueueSendToBack(msc_event_Queue,&event,0);
	}
}

/**
* @brief interface function - MSC State Machine : do CSW
*/
static void msc_do_csw(void)
{
	//NVIC_DisableIRQ(USBDEV_IRQn);
	//xSemaphoreTake(usbd_msc_state_mutex, portMAX_DELAY);

	send_status();
	msc->state = MSC_STATE_STATUS;
	usb_ep_queue(msc->bulk_in, msc->bulk_in_req);
	msc->bulk_out_req->length = 31;
	

	//xSemaphoreGive(usbd_msc_state_mutex);
	//NVIC_EnableIRQ(USBDEV_IRQn);
}

int msc_media_detect(void)
{
#if(USBD_MSC_MEDIA_DDR == 0)

    uint8_t media_state;
    if(msc_detect_cb==NULL) {
        msc->media_remove_flag = 1;
        msc->std_media_maxblock = 0;
        return pdFAIL;
    }    

    media_state = msc_detect_cb(&msc->ro, &msc->std_media_maxblock);
    if ((msc->media_remove_flag == 1) && (media_state == 0)) {
        /* medium insert */
        msc->sense_data = SS_NOT_READY_TO_READY_TRANSITION;
    } else if ((msc->media_remove_flag == 0) && (media_state == 1)) {
		/* medium remove */
		msc->std_media_maxblock = 0;
		msc->sense_data = SS_MEDIUM_REMOVAL_PREVENTED;
	} else if (media_state == 1) {
		msc->std_media_maxblock = 0;
		msc->sense_data = SS_MEDIUM_NOT_PRESENT;
	}
    
	if (msc->media_remove_flag != media_state) {
		msc->media_remove_flag = media_state;
	}    
#endif    
    return pdPASS;
    
#if 0
    int ret = 0;
	uint8_t media_state;

	media_state = mid_sd_card_detect();
	if(!msc)	return ret;
	if ((msc->media_remove_flag == 1) && (media_state == 0)) {


		/* medium insert */
		ret = mid_sd_identify(MID_SD_BLOCK, NULL);
		if(ret != MID_SD_QUEUE_FINISH)
		{
			USBD_MSC_PRINT("sd identify fail\r\n");
			return ret;
		}

		ret = mid_sd_get_capacity(&msc->std_media_maxblock, MID_SD_BLOCK, NULL);

		if(ret == MID_SD_QUEUE_FINISH)
		{
			USBD_MSC_PRINT("SD size = 0x%08x blocks\r\n",msc->std_media_maxblock);
		}
		if (sd_write_protect_detect() == SD_RTN_SD_CARD_WRITE_PROTECT) {
			msc->ro = 1;
		} else {
			msc->ro = 0;
		}
		msc->sense_data = SS_NOT_READY_TO_READY_TRANSITION;
	} else if ((msc->media_remove_flag == 0) && (media_state == 1)) {
		/* medium remove */
		msc->std_media_maxblock = 0;
		msc->sense_data = SS_MEDIUM_REMOVAL_PREVENTED;
	} else if (media_state == 1) {
		msc->std_media_maxblock = 0;
		msc->sense_data = SS_MEDIUM_NOT_PRESENT;
	}

	if (msc->media_remove_flag != media_state) {
		msc->media_remove_flag = media_state;
	}
    return ret;
#endif
	
}

/**
* @brief interface function - MSC State Machine Task
* @param pvParameters the structure of input data
*/
void msc_device_task( void *pvParameters )
{
	uint32_t event = 0;
	uint32_t errorhandle = 0;
	USBD_MSC_PRINT("%s-start\r\n",__FUNCTION__);


	while(msc_task_run) {
		msc_media_detect();

		xQueueReceive(msc_event_Queue, &event, portMAX_DELAY);
		msc->event = (enum msc_event)event;
		USBD_MSC_PRINT("%s-event(%d)-state(%d)\r\n",__FUNCTION__,msc->event,msc->state);

		switch(msc->event) {
		case MSC_EVENT_IDLE:
			if(msc->state == MSC_STATE_STATUS) {
				msc_do_idle();
			}
			if(msc->state == MSC_STATE_IDLE) {
			}
			else {
				errorhandle = 2;
			}
			break;

		case MSC_EVENT_CBW:
			if(msc->state == MSC_STATE_IDLE) {
				if(msc_task_run)
					msc_do_cbw();
				else {
					break;
				}
			}
			else {
				errorhandle = 1;
			}
			break;

		case MSC_EVENT_DATA:
			if(msc->state == MSC_STATE_COMMAND) {
				if(msc_task_run)
					msc_do_data();
				else {
					break;
				}
			}
			else {
				errorhandle = 1;
			}
			break;

		case MSC_EVENT_DATA_OK:
			if(msc->state == MSC_STATE_DATA_BUSY) {
				if(msc_task_run)
					msc_do_data_finish();
				else {
					break;
				}
			}
			else {
				errorhandle = 1;
			}
			break;

		case MSC_EVENT_CSW:
			if(msc->state == MSC_STATE_DATA) {
				if(msc_task_run)
					msc_do_csw();
				else {
					break;
				}

			}
			else if(msc->state == MSC_STATE_COMMAND) {
				if(msc_task_run)
					msc_do_csw();
				else {
					break;
				}

				if(msc->csw_status != USB_STATUS_PHASE_ERROR) {
					if(usbd_msc_sema)
						xSemaphoreGive(usbd_msc_sema);
				}
			}
			else {
				errorhandle = 1;
			}
			break;

		default:
			errorhandle = 1;
			break;
		}
	}
	if(errorhandle)
		;
	vQueueDelete(msc_event_Queue);
	msc_event_Queue =NULL;
	if(usbd_msc_stop) {
		xSemaphoreGive(usbd_msc_stop);
	}
	vTaskDelete(NULL);

}

void msc_device_task_stop(void)
{
	uint32_t event ;

	event = MSC_EVENT_IDLE;

	msc_task_run = 0;
	if(msc && msc_event_Queue)
		xQueueSendToBack(msc_event_Queue,&event,0);

	if(usbd_msc_sema) {
		vSemaphoreDelete(usbd_msc_sema);
		usbd_msc_sema = NULL;
	}

}



/**
* @brief interface function - un-initial msc function
* @return value of error code.
*/
int usbd_msc_uninit(void)
{
	msc_device_task_stop();
	if(usbd_msc_stop)
		xSemaphoreTake(usbd_msc_stop, 150);
	if(usbd_msc_stop) {
		vSemaphoreDelete(usbd_msc_stop);
		usbd_msc_stop = NULL;
	}
	//if(usbd_msc_state_mutex){
	//	vSemaphoreDelete(usbd_msc_state_mutex);
	//	usbd_msc_state_mutex = NULL;
	//}	
	return 0;
}



void usbd_msc_vendor_read_status()
{
	msc->type = 	MSC_TYPE_VENDOR_READ;
	msc->data_dir = DATA_DIR_TO_HOST;
}
void usbd_msc_vendor_write_status()
{
	msc->type = 	MSC_TYPE_VENDOR_WRITE;
	msc->data_dir = DATA_DIR_FROM_HOST;
}



void usbd_msc_read_data(uint32_t addr, uint32_t len, uint8_t *buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
#if(USBD_MSC_MEDIA_DDR==1)
	memcpy(buf,(uint8_t*)USBD_MSC_DRAM_ADDRESS+(addr<<9),len << 9);
#else
    if(msc_std_cmd_rdata_cb)
        msc_std_cmd_rdata_cb(addr, len, buf);        
#endif
}
void usbd_msc_write_data(uint32_t addr, uint32_t len, uint8_t *buf)
{
	USBD_MSC_PRINT("%s\r\n",__FUNCTION__);
#if(USBD_MSC_MEDIA_DDR==1)
	memcpy((uint8_t*)USBD_MSC_DRAM_ADDRESS+(addr<<9),buf,len << 9);
#else
    if(msc_std_cmd_wdata_cb)
        msc_std_cmd_wdata_cb(addr, len, buf);
#endif
}


void usbd_drv_msc_detect_reg_cb(usbd_drv_msc_detect_cb_t cb)
{
	msc_detect_cb = cb;
}

void usbd_drv_msc_std_cmd_rdata_reg_cb(usbd_drv_msc_std_cmd_rdata_cb_t cb)
{
	msc_std_cmd_rdata_cb = cb;
}

void usbd_drv_msc_std_cmd_wdata_reg_cb(usbd_drv_msc_std_cmd_wdata_cb_t cb)
{
	msc_std_cmd_wdata_cb = cb;
}


#endif  // #if defined( CONFIG_MODULE_USBD_MSC_CLASS )
