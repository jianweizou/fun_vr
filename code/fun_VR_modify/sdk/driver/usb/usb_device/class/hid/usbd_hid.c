/**
* @file
* this is usb device driver file
* usbd_hid.c
* @author IP2/Luka
*/
//#include <stdio.h>
#include "sonix_config.h"
#if defined( CONFIG_MODULE_USBD_HID_CLASS )

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "usb_device.h"
#include "usbd_hid.h"
#include "composite.h"
#include "snx_udc.h"
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif




#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
SemaphoreHandle_t usbd_hid_xfr_start=NULL;
SemaphoreHandle_t usbd_hid_key_xfr_finish=NULL;

extern void usbd_hid_task_start(void);
extern void usbd_hid_task_stop(void);
#endif

/*==========================================*/
/*				user define 				*/
/*==========================================*/
#define HID_VENDOR_ID			0x0C45	/* Sonix Technology Co. */
#define HID_PRODUCT_ID			0x8240	/* SN98660 HID */
#define HID_DEVICE_BCD			0x0100	/* 1.00 */


static struct usb_qualifier_descriptor	hid_qualifier_desc	=	{
	sizeof(struct usb_qualifier_descriptor),    // bLength
	USB_DT_DEVICE_QUALIFIER,                    // bDescriptorType
	0x0200,                                     // bcdUSB
	USB_CLASS_MISC,                             // bDeviceClass
	USB_CLASS_COMM,                             // bDeviceSubClass
	0x01,                                       // bDeviceProtocol
	0,                                          // bMaxPacketSize0 		/* dynamic */
	0,                                          // bNumConfigurations 	/* dynamic */
	0                                           // bRESERVED;
};

static struct usb_device_descriptor hid_device_desc = {
	sizeof(struct usb_device_descriptor),       // bLength
	USB_DT_DEVICE,                              // bDescriptorType
	0x0200,                                     // bcdUSB
	0,                                          // bDeviceClass
	0,                                          // bDeviceSubClass
	0,                                          // bDeviceProtocol
	0,                                          // bMaxPacketSize0 		/* dynamic */
	HID_VENDOR_ID,                              // idVendor
	HID_PRODUCT_ID,                             // idProduct
	HID_DEVICE_BCD,                             // bcdDevice
	0,                                          // iManufacturer		/* dynamic */
	0,                                          // iProduct  			/* dynamic */
	0,                                          // iSerialNumber 		/* dynamic */
	0                                           // bNumConfigurations 	/* dynamic */
};

enum hid_event
{
	HID_EVENT_IDLE = 0,		/*0*/
	HID_EVENT_READ,			/*1*/
	HID_EVENT_WRITE,		/*2*/
};

enum hid_state
{
	HID_STATE_IDLE = 0,		/*0*/
	HID_STATE_READ,			/*1*/
	HID_STATE_WRITE,		/*2*/
	HID_STATE_ERR_TIMEOUT,	/*3*/
};

enum hid_type
{
	HID_TYPE_NONE = 0,
	HID_TYPE_STANDARD_WRITE,
	HID_TYPE_STANDARD_READ,
	HID_TYPE_VENDOR_WRITE,
	HID_TYPE_VENDOR_READ,
};


static uint32_t hid_task_run;

SemaphoreHandle_t usbd_hid_sendok_sema;
SemaphoreHandle_t usbd_hid_stop;
#if defined(CONFIG_PLATFORM_SN98660 )
static usbd_drv_hid_cmd_wdata_cb_t hid_cmd_wdata_cb = NULL;
static usbd_drv_hid_cmd_wdata_cb_t hid_cmd_wdata_ext_cb = NULL;
#endif
//===================================================================

#define HID_STRING_INTERFACE		0


/* hid descriptor for a keyboard */
static struct hid_func_descriptor snx_hid_data ;


struct hid_device
{
	struct usb_function func;
	enum hid_state      state;
	enum hid_event      event;

	/* configuration */
	uint8_t             bInterfaceSubClass;
	uint8_t             bInterfaceProtocol;
	uint16_t            report_desc_length;
	char                *report_desc;
	uint16_t            report_length;
	unsigned char       *report_data;
	uint8_t             set_report_buff[100];


	struct usb_ep       *int_in_ep;
	struct usb_request  *int_in_req;
	struct usb_ep       *int_out_ep;
	struct usb_request  *int_out_req;

	QueueHandle_t       hid_event_Queue;

	/* Add for HID error handle */
	unsigned int        sema_timeout;
	int                 err_state;
} __attribute__ ((packed));

static struct hid_device *hid = NULL;

static int hid_function_set_alt(struct usb_function *f, unsigned interface, unsigned alt);
static int hid_function_get_alt(struct usb_function *f, unsigned interface, unsigned alt);

static char hid_vendor_label[]          = "Sonix Technology Co., Ltd.";
static char hid_product_label[]         = "Sonix HID Device";
static char hid_config_label[]          = "Sonix HID";

static struct usb_string                hid_strings[5];
static struct usb_gadget_strings        hid_device_strings ;
static struct usb_string                hid_en_us_strings[2];
static struct usb_gadget_strings        hid_function_strings ;

static int hid_bind(struct usb_composite_dev *cdev);
static int hid_unbind(struct usb_composite_dev *cdev);
static int hid_config_bind(struct usb_configuration *c);

static struct usb_composite_driver      hid_driver;
static struct usb_configuration         hid_config_driver;
static struct usb_interface_descriptor  hid_intf_desc ;
static struct hid_descriptor            hid_desc;
static struct usb_endpoint_descriptor   hid_hs_in_ep_desc ;
static struct usb_endpoint_descriptor   hid_fs_in_ep_desc ;


static struct usb_descriptor_header *hid_hs_descriptors[] = {
	(struct usb_descriptor_header *)&hid_intf_desc,
	(struct usb_descriptor_header *)&hid_desc,
	(struct usb_descriptor_header *)&hid_hs_in_ep_desc,
	NULL,
};

/* Full-Speed Support */
static struct usb_descriptor_header *hid_fs_descriptors[] = {
	(struct usb_descriptor_header *)&hid_intf_desc,
	(struct usb_descriptor_header *)&hid_desc,
	(struct usb_descriptor_header *)&hid_fs_in_ep_desc,
	NULL,
};

/**
* @brief interface function - unbind  hid function
* @param c the pointer of usb configuration
* @param f the pointer of usb function
*/
static void hid_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
	usb_free_descriptors(f->descriptors);
	usb_free_descriptors(f->hs_descriptors);

	if(hid) {
		if(hid->int_in_ep && hid->int_in_req) {
			if(hid->int_in_req->buf) {
				vPortFree(hid->int_in_req->buf);
				hid->int_in_req->buf= NULL;
			}
			usb_ep_free_request(hid->int_in_ep,hid->int_in_req);
		}
		if(hid->report_data) {
			vPortFree(hid->report_data);
			hid->report_data = NULL;
		}
	}
}

/**
* @brief interface function - bind  hid function
* @param c the pointer of usb configuration
* @param f the pointer of usb function
* @return value of error code.
*/
static int hid_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_ep *ep;
	int ret = -EINVAL;

	/* Allocate endpoints. */
	ep = usb_ep_autoconfig(cdev->gadget, &hid_fs_in_ep_desc, "HID-INT");
	if (!ep) {
		USBD_HID_PRINT("Unable to allocate int in desc EP\n");
		goto error;
	}
	hid->int_in_ep = ep;
	//ep->driver_data = hid;

	if ((ret = usb_interface_id(c, f)) < 0)
		goto error;
	hid_intf_desc.bInterfaceNumber = ret;

	hid_hs_in_ep_desc.bEndpointAddress = hid_fs_in_ep_desc.bEndpointAddress;
	USBD_HID_PRINT("bEndpointAddress=%08x\n",hid_hs_in_ep_desc.bEndpointAddress);
	/* set descriptor dynamic values */
	hid_intf_desc.bInterfaceSubClass = hid->bInterfaceSubClass;
	hid_intf_desc.bInterfaceProtocol = hid->bInterfaceProtocol;
	hid_hs_in_ep_desc.wMaxPacketSize = hid->report_length;
	hid_fs_in_ep_desc.wMaxPacketSize = hid->report_length;
	hid_desc.desc[0].bDescriptorType = HID_DT_REPORT;
	hid_desc.desc[0].wDescriptorLength = hid->report_desc_length;

	f->hs_descriptors = usb_copy_descriptors(hid_hs_descriptors);
	if (!f->hs_descriptors) {
		usb_free_descriptors(f->descriptors);
		return -ENOMEM;
	}

	f->descriptors = usb_copy_descriptors(hid_fs_descriptors);
	if (!f->descriptors)
		return -ENOMEM;

	hid->int_in_req = usb_ep_alloc_request(hid->int_in_ep);
	hid->int_in_req->buf = (uint8_t*)usbd_malloc(hid->report_length);
	hid->int_in_req->length = hid->report_length;
	if (hid->int_in_req == NULL || hid->int_in_req->buf == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	hid->report_data = (unsigned char*)usbd_malloc(hid->report_length);

	return 0;

error:
	hid_function_unbind(c, f);
	return ret;
}


/**
* @brief interface function - setup function
* @param f the pointer of usb function
* @param ctrl the request for control endpoint
* @return value of error code.
*/
//#define HID_ASIC_ID_REPORT_ID             0x01
//#define HID_USBD_CLASS_CTRL_REPORT_ID     0x02

#define HID_ASIC_ID_REPORT_ID               0x02
#define HID_USBD_CLASS_CTRL_REPORT_ID       0x03

#define ASIC_ID_LEN                         7

#define SNX_DECLARE_HID_ASIC_ID_FEATURE_CMD(n)  DECLARE_HID_ASIC_ID_FEATURE_CMD(n)
#define SNX_HID_ASIC_ID_FEATURE_CMD(n)          HID_ASIC_ID_FEATURE_CMD(n)

#define HID_ASIC_ID_FEATURE_CMD(n)              hid_asic_id_feature_cmd_##n

#define DECLARE_HID_ASIC_ID_FEATURE_CMD(n)	\
struct HID_ASIC_ID_FEATURE_CMD(n){			\
    uint8_t	bReportID;				\
    uint8_t	bMajorVer;				\
    uint8_t	bMinorVer;				\
    uint8_t	bBuildVer;				\
    uint8_t	bDeviceIDLength;		\
    uint8_t	bDeviceID[n];			\
    uint8_t	bReserved[59-n];		\
}__attribute__((packed))


SNX_DECLARE_HID_ASIC_ID_FEATURE_CMD(ASIC_ID_LEN);
static const struct SNX_HID_ASIC_ID_FEATURE_CMD(ASIC_ID_LEN) asic_id = {
	HID_ASIC_ID_REPORT_ID,
	0x00,
	0x01,
	0x00,
	ASIC_ID_LEN,
	"ST58660",
	0,
};

static struct HID_USBD_CLASS_CTRL_FEATURE_CMD usbd_class_ctrl = {
	HID_USBD_CLASS_CTRL_REPORT_ID,
	{0x00},
	0,
	0,
};

static int hid_function_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct usb_ep *ep = hid->func.config->cdev->gadget->ep0;
	struct usb_request *req = hid->func.config->cdev->req;
	int ret = 0;
	uint16_t w_length = ctrl->wLength;
	uint8_t report_id = ctrl->wValue & 0x00FF;
	uint8_t report_type = (ctrl->wValue & 0xFF00) >> 8;

	//USBD_HID_PRINT("hid_function_setup:setup request %02x %02x value %04x index %04x %04x\n",ctrl->bRequestType, ctrl->bRequest, (ctrl->wValue),(ctrl->wIndex), (ctrl->wLength));

	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8  | HID_REQ_GET_REPORT):
		USBD_HID_PRINT( "get_report\n");
		switch(report_type) {
		case HID_REPORT_TYPE_INPUT:
		case HID_REPORT_TYPE_OUTPUT:
			req->length  = min(w_length, hid->report_length);
			memset(req->buf, 0x0, req->length); 	// send an empty report
			break;
		case HID_REPORT_TYPE_FEATURE:
			switch(report_id) {
			case HID_ASIC_ID_REPORT_ID:
				req->length = sizeof(asic_id);
				memset(req->buf, 0x00, req->length);
				memcpy(req->buf, &asic_id, req->length);
				break;
			case HID_USBD_CLASS_CTRL_REPORT_ID:
				req->length = sizeof(usbd_class_ctrl);
				memset(req->buf, 0x00, req->length);
				memcpy(req->buf, &usbd_class_ctrl, req->length);
				break;
			default:
				goto stall;
			}
			break;
		default:
			goto stall;
		}
		goto respond;

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8  | HID_REQ_GET_PROTOCOL):
		USBD_HID_PRINT("get_protocol\n");
		goto stall;
	//ret = usb_ep_queue(ep, req);
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8 | HID_REQ_SET_REPORT):
		USBD_HID_PRINT( "set_report | wLenght=%d\n", ctrl->wLength);
		switch(report_type) {
		case HID_REPORT_TYPE_INPUT:
			break;
		case HID_REPORT_TYPE_OUTPUT:
			USBD_HID_PRINT( "HID_REPORT_TYPE_OUTPUT\n");
			req->length = ctrl->wLength;
			usb_ep_queue(ep, req);
			memcpy(&hid->set_report_buff, req->buf, req->length);
			USBD_HID_PRINT( "exit\n");
			break;
		case HID_REPORT_TYPE_FEATURE:
			switch(report_id) {
			case HID_ASIC_ID_REPORT_ID:
				goto stall;
			//break;
			case HID_USBD_CLASS_CTRL_REPORT_ID:
				req->length =  ctrl->wLength;
				ret = usb_ep_queue(ep, req);
				memcpy(&usbd_class_ctrl, req->buf, req->length);
#if defined(CONFIG_PLATFORM_SN98660 )
				hid_cmd_wdata_cb(&usbd_class_ctrl, req->length);
#endif
				break;
			default:
				req->length =  ctrl->wLength;
				ret = usb_ep_queue(ep, req);
#if defined(CONFIG_PLATFORM_SN98660 )
				hid_cmd_wdata_ext_cb(req->buf, req->length);
#endif
				break;
			}
			break;
		default:
			break;
		}
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
		xSemaphoreGive(usbd_hid_xfr_start);		
		usbd_hid_task_start();
#endif
		goto respond;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8  | HID_REQ_SET_PROTOCOL):
		USBD_HID_PRINT( "set_protocol\n");
		goto stall;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8  | HID_REQ_SET_IDLE):
		USBD_HID_PRINT( "set_idle | wValue=%d\n", ctrl->wValue);
		ret =0x10000;
		break;

	case ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8	  | USB_REQ_GET_DESCRIPTOR):
		switch ( ctrl->wValue >> 8) {
		case HID_DT_REPORT:
			USBD_HID_PRINT("USB_REQ_GET_DESCRIPTOR: REPORT\n");
			req->length  = min(w_length, hid->report_desc_length);
			memcpy(req->buf, hid->report_desc, req->length);
			goto respond;

		case HID_DT_HID:
			USBD_HID_PRINT("USB_REQ_GET_DESCRIPTOR: HIDDescriptor\n");
			req->length  = min(w_length, hid_desc.bLength);
			memcpy(req->buf, &hid_desc, req->length);
			goto respond;

		default:
			USBD_HID_PRINT("Unknown decriptor request 0x%x\n", report_type);
			goto stall;
		}

	case ((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_ENDPOINT) << 8 | USB_REQ_CLEAR_FEATURE):
		if((ctrl->wIndex & USB_DIR_IN) == USB_DIR_IN) {
			usb_ep_set_halt(hid->int_in_ep, hid_fs_in_ep_desc.bEndpointAddress, 0);
		}
		else {
			//usb_ep_set_halt(hid->int_out_ep, hid_fs_out_ep_desc.bEndpointAddress, 0);
		}
		break;

	default:
		USBD_HID_PRINT("Unknown request 0x%x\n",	 ctrl->bRequest);
		goto stall;
	}

//done:
	if (ret < 0)
		USBD_HID_PRINT("usb_ep_queue error on ep0 %d\n", ctrl->wValue);
	return ret;
stall:
	return -EOPNOTSUPP;

respond:
	ret = usb_ep_queue(ep, req);
	return ret;
}

static void hid_function_disable(struct usb_function *f)
{

}

#if defined(CONFIG_PLATFORM_SN98660 )
/**
* @brief interface function - endpoint 3 callback function
* @param f the pointer of usb function
* @param ep the pointer of usb endpoint
*/
static void hid_functione_ep3_callback(struct usb_function *f, struct usb_ep *ep)
{
	USBD_HID_PRINT("%s\r\n",__FUNCTION__);
	if((ep->state & USBD_EP_STATE_IN) == USBD_EP_STATE_IN) {
		ep->state &= ~USBD_EP_STATE_IN;

	} else if((ep->state & USBD_EP_STATE_TFOC) ==  USBD_EP_STATE_TFOC) {
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC | USBD_EP_STATE_TFOC);
		xSemaphoreGive(usbd_hid_sendok_sema);
	}
}
#elif defined( CONFIG_PLATFORM_ST53510 ) || defined( CONFIG_PLATFORM_SN7320 )
/**
* @brief interface function - endpoint 5 callback function
* @param f the pointer of usb function
* @param ep the pointer of usb endpoint
*/
static void hid_functione_ep1_callback(struct usb_function *f, struct usb_ep *ep)
{
	//USBD_HID_PRINT("%s\r\n",__FUNCTION__);

	if((ep->state & USBD_EP_STATE_IN) == USBD_EP_STATE_IN) {
		ep->state &= ~USBD_EP_STATE_IN;
	} else if((ep->state & USBD_EP_STATE_TFOC) ==  USBD_EP_STATE_TFOC) {
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC | USBD_EP_STATE_TFOC);
		xSemaphoreGive(usbd_hid_sendok_sema);
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)		
		xSemaphoreGive(usbd_hid_key_xfr_finish);
#endif
	}
}
#endif


/**
* @brief interface function - configuration bind  function
* @param c the pointer of usb configuration
* @return value of error code.
*/
static int hid_config_bind(struct usb_configuration *c)
{
	int ret;

	if (hid == NULL) {
		hid = (struct hid_device*)usbd_malloc(sizeof(struct hid_device));
		if (hid == NULL)
			return -ENOMEM;
	}
	memset(hid, 0x00, sizeof(struct hid_device));

	if ((ret = usb_string_id(c->cdev)) < 0)
		goto error;

	hid_en_us_strings[HID_STRING_INTERFACE].id = ret;
	hid_intf_desc.iInterface = ret;

	//hid->minor = ret;
	hid->bInterfaceSubClass     = snx_hid_data.subclass;
	hid->bInterfaceProtocol     = snx_hid_data.protocol;
	hid->report_length          = snx_hid_data.report_length;
	hid->report_desc_length     = snx_hid_data.report_desc_length;
	hid->report_desc            = snx_hid_data.report_desc;
	hid->func.name              = "HID Function";
	hid->func.strings           = &hid_function_strings;
	hid->func.bind              = hid_function_bind;
	hid->func.unbind            = hid_function_unbind;
	hid->func.setup             = hid_function_setup;
	hid->func.set_alt           = hid_function_set_alt;
	hid->func.disable           = hid_function_disable;
	hid->func.get_alt           = hid_function_get_alt;
#if defined(CONFIG_PLATFORM_SN98660 )
	hid->func.ep_callback[3]	= hid_functione_ep3_callback;
#elif defined( CONFIG_PLATFORM_ST53510 ) || defined( CONFIG_PLATFORM_SN7320 )
	hid->func.ep_callback[1]    = hid_functione_ep1_callback;
#endif
	hid->state                  = HID_STATE_IDLE;
	hid->hid_event_Queue        = NULL;
	hid->sema_timeout           = HID_SEMA_TIMEOUT;
	hid->err_state              = HID_ERR_NO_ERROR;

	ret = usb_add_function(c, &hid->func);


	usbd_hid_sendok_sema = xSemaphoreCreateCounting(1,0);
	if(!usbd_hid_sendok_sema) {
		USBD_HID_PRINT(" create usbd_hid_sendok_sema is fail~~~\r\n");
	}

	usbd_hid_stop = xSemaphoreCreateCounting(1,0);
	if(!usbd_hid_stop) {
		USBD_HID_PRINT(" create usbd_hid_stop is fail~~~\r\n");
	}
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)	
	usbd_hid_xfr_start = xSemaphoreCreateCounting(1,0);
	if(!usbd_hid_xfr_start) {
		USBD_HID_PRINT(" create usbd_hid_xfr_start is fail~~~\r\n");
	}	

	usbd_hid_key_xfr_finish = xSemaphoreCreateCounting(1,1);
	if(!usbd_hid_key_xfr_finish) {
		USBD_HID_PRINT(" create usbd_hid_key_xfr_finish is fail~~~\r\n");
	}	

#endif
	return 0;

error:
	return ret;
}
/**
* @brief interface function - hid	  bind function
* @param cdev the pointer of usb compisite device
* @return value of error code.
*/
static int  hid_bind(struct usb_composite_dev *cdev)
{
	int ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	hid_strings[STRING_MANUFACTURER_IDX].id = ret;
	hid_device_desc.iManufacturer = ret;


	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	hid_strings[STRING_PRODUCT_IDX].id = ret;
	hid_device_desc.iProduct = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	hid_strings[STRING_DESCRIPTION_IDX].id = ret;
	hid_config_driver.iConfiguration = ret;
	hid_qualifier_desc.bNumConfigurations = hid_config_driver.iConfiguration;

	if ((ret = usb_add_config(cdev, &hid_config_driver)) < 0)
		goto error;

	USBD_HID_PRINT("Sonix HID Gadget\n");
	return 0;

error:
	hid_unbind(cdev);
	return ret;
}
/**
* @brief interface function - hid  unbind function
* @param cdev the pointer of usb compisite device
* @return value of error code.
*/
static int hid_unbind(struct usb_composite_dev *cdev)
{
	/*
	struct usb_configuration	*c;

	list_for_each_entry(c, &cdev->configs, list) {
	    if(c){
	        hid_function_unbind(c, &hid->func);
	        break;
	    }
	}
	*/
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)		
	vSemaphoreDelete(usbd_hid_key_xfr_finish);
	usbd_hid_key_xfr_finish = NULL;
#endif

	if (hid != NULL) {
		vPortFree(hid);
		hid = NULL ;
	}
	return 0;
}
/**
* @brief interface function - set alternate setting
* @param f the pointer of usb function
* @param intf the number of interface
* @param alt the number of alternate setting
* @return the value of error code.
*/
static int hid_function_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	USBD_HID_PRINT("%s\r\n",__FUNCTION__);

	switch (alt) {
	case 0:
		usb_ep_enable(hid->int_in_ep, &hid_hs_in_ep_desc);
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
static int hid_function_get_alt(struct usb_function *f, unsigned interface, unsigned alt)
{
	return 0;
}

static void hid_do_read(void)
{
	//USBD_HID_PRINT("%s\r\n",__FUNCTION__);

	if(!hid)return;

	hid->state = HID_STATE_READ;

	if(hid->int_in_req) {
		memcpy(hid->int_in_req->buf,hid->report_data,hid->report_length);
		hid->int_in_req->length = hid->report_length;

		if ((usb_ep_queue(hid->int_in_ep, hid->int_in_req)) == 0) {
			if (xSemaphoreTake(usbd_hid_sendok_sema, portMAX_DELAY) == pdTRUE) {
				hid->state = HID_STATE_IDLE;
			} else {
				hid->state = HID_STATE_ERR_TIMEOUT;
			}
		}
	}

	if (hid->state == HID_STATE_READ) {
		hid->state = HID_STATE_IDLE;
	}

	if (hid->state != HID_STATE_IDLE) {
		printf("[USBD-HID WRONG] HID send data timeout!\n");
		hid->err_state = HID_ERR_DATA_TIMEOUT;
		hid->state = HID_STATE_IDLE;
	}
}

static void hid_do_write(void)
{
	USBD_HID_PRINT("%s\r\n",__FUNCTION__);
	//hid->state = HID_STATE_WRITE;
	hid->state = HID_STATE_IDLE;
}
static void hid_do_idle(void)
{
	USBD_HID_PRINT("%s\r\n",__FUNCTION__);
	hid->state = HID_STATE_IDLE;
}
/**
* @brief interface function - HID State Machine Task
* @param pvParameters the structure of input data
*/
void hid_device_task( void *pvParameters )
{
	uint32_t event = 0;

	hid_task_run = 1;
	hid->hid_event_Queue = xQueueCreate(50, sizeof(uint32_t));

	while(hid_task_run) {
		if(!hid)continue;
		if(hid->hid_event_Queue) {
			xQueueReceive(hid->hid_event_Queue,&event,portMAX_DELAY);
			hid->event = (enum hid_event)event;
		}
		//USBD_HID_PRINT("***[task]hid state :%d (event:%d) \n",hid->state,hid->event);
		switch(hid->event) {
		case HID_EVENT_IDLE:
			hid_do_idle();
			break;

		case HID_EVENT_READ:
			hid_do_read();
			break;

		case HID_EVENT_WRITE:
			hid_do_write();
			break;

		default:
			break;
		}
	}

	if (hid->hid_event_Queue != NULL) {
		vQueueDelete(hid->hid_event_Queue);
		hid->hid_event_Queue = NULL;
	}
	if(usbd_hid_stop) {
		xSemaphoreGive(usbd_hid_stop);
	}
	vTaskDelete(NULL);
}

void hid_device_task_stop(void)
{
	uint32_t event = HID_EVENT_IDLE;

	hid_task_run = 0;
	if(hid && hid->hid_event_Queue)
		xQueueSendToBack(hid->hid_event_Queue,&event,0);

	if(usbd_hid_sendok_sema) {
		vSemaphoreDelete(usbd_hid_sendok_sema);
		usbd_hid_sendok_sema = NULL;
	}

	hid->err_state = HID_ERR_NO_ERROR;	// Reset error state
}
/**
* @brief interface function - initial hid function
* @return value of error code.
*/
int usbd_hid_init(void)
{
	int ret;

	snx_hid_data.report_desc                = hid_report_desc;
	snx_hid_data.subclass                   = 0;			/* No subclass */
	snx_hid_data.protocol                   = 1;			/* Keyboard */
	snx_hid_data.report_length              = 64 ;
	snx_hid_data.report_desc_length         = sizeof(hid_report_desc);

	strcpy(hid_strings[STRING_MANUFACTURER_IDX].s, hid_vendor_label);
	strcpy(hid_strings[STRING_PRODUCT_IDX].s, hid_product_label);
	strcpy(hid_strings[STRING_DESCRIPTION_IDX].s, hid_config_label);
	strcpy(hid_strings[STRING_EMPTY_IDX].s,"");
	hid_device_strings.language             = 0x0409;
	hid_device_strings.strings              = hid_strings;

	strcpy(hid_en_us_strings[HID_STRING_INTERFACE].s, "");
	strcpy(hid_en_us_strings[1].s, "");
	hid_function_strings.language           = 0x0409;
	hid_function_strings.strings            = hid_en_us_strings;

	strcpy(hid_driver.name, "hid driver");
	hid_driver.dev                          = &hid_device_desc;
	hid_driver.qualifier                    = &hid_qualifier_desc;

	hid_driver.strings                      = &hid_device_strings;
	hid_driver.bind                         = hid_bind;
	hid_driver.unbind                       = hid_unbind;

	hid_config_driver.label			        = hid_config_label;
	hid_config_driver.bind			        = hid_config_bind;
	hid_config_driver.bConfigurationValue   = 1;
	hid_config_driver.iConfiguration        = 0; /* dynamic */
	hid_config_driver.bmAttributes          = USB_CONFIG_ATT_SELFPOWER;
	hid_config_driver.bMaxPower             = CONFIG_USB_GADGET_VBUS_DRAW / 2;

	hid_intf_desc.bLength                   = sizeof(hid_intf_desc);
	hid_intf_desc.bDescriptorType           = USB_DT_INTERFACE;
	/* .bInterfaceNumber			        = DYNAMIC; */
	hid_intf_desc.bAlternateSetting         = 0;
	hid_intf_desc.bNumEndpoints             = 1;
	hid_intf_desc.bInterfaceClass           = USB_CLASS_HID;
	/* .bInterfaceSubClass			        = DYNAMIC ;*/
	/* .bInterfaceProtocol			        = DYNAMIC ;*/
	/* .iInterface				            = DYNAMIC ;*/

	hid_desc.bLength                        = sizeof(hid_desc);
	hid_desc.bDescriptorType                = HID_DT_HID;
	hid_desc.bcdHID                         = 0x0111;
	hid_desc.bCountryCode                   = 0x00;
	hid_desc.bNumDescriptors                = 0x1;
	/*hid_desc.desc[0].bDescriptorType      = DYNAMIC */
	/*hid_desc.desc[0].wDescriptorLenght    = DYNAMIC */

	hid_hs_in_ep_desc.bLength               = USB_DT_ENDPOINT_SIZE;
	hid_hs_in_ep_desc.bDescriptorType       = USB_DT_ENDPOINT;
	hid_hs_in_ep_desc.bEndpointAddress      = USB_DIR_IN;
	hid_hs_in_ep_desc.bmAttributes          = USB_ENDPOINT_XFER_INT;
	/*.wMaxPacketSize	                    = DYNAMIC */
	hid_hs_in_ep_desc.bInterval             = HID_HS_INTERVAL;

	hid_fs_in_ep_desc.bLength               = USB_DT_ENDPOINT_SIZE;
	hid_fs_in_ep_desc.bDescriptorType       = USB_DT_ENDPOINT;
	hid_fs_in_ep_desc.bEndpointAddress      = USB_DIR_IN;
	hid_fs_in_ep_desc.bmAttributes          = USB_ENDPOINT_XFER_INT;
	/*.wMaxPacketSize			            = DYNAMIC */
	hid_fs_in_ep_desc.bInterval             = HID_FS_INTERVAL;

	ret = usb_composite_register(&hid_driver);

	if( ret != 0 ) {
		USBD_HID_PRINT("USBD : hid function initial error (%d)!!!\n",ret);
		goto fail;
	}
	USBD_HID_PRINT(" create hid_device_task\r\n");

	if (pdPASS != xTaskCreate(hid_device_task, "hid_device_task", 4096, (void*) NULL, 250, NULL)) {
		USBD_HID_PRINT("Could not hid_device_task\r\n");
	}

#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)
	if (pdPASS != xTaskCreate(usbd_hid_vc_task, "usbd_hid_xfr_task", 4096, (void*) NULL, 250, NULL)) {
		USBD_HID_PRINT("Could not usbd_hid_xfr_task\r\n");
	}
#endif

fail:
	return ret;
}
/**
* @brief interface function - un-initial hid function
* @return value of error code.
*/
int usbd_hid_uninit(void)
{
	hid_device_task_stop();
	if(usbd_hid_stop)
		xSemaphoreTake(usbd_hid_stop, 150);

	usb_composite_unregister();

	if(usbd_hid_stop) {
		vSemaphoreDelete(usbd_hid_stop);
		usbd_hid_stop = NULL;
	}
	return 0;
}
#if defined(CONFIG_PLATFORM_SN98660 )
void usbd_drv_hid_cmd_wdata_reg_cb(usbd_drv_hid_cmd_wdata_cb_t cb)
{
	hid_cmd_wdata_cb = cb;
}

void usbd_drv_hid_cmd_wdata_ext_reg_cb(usbd_drv_hid_cmd_wdata_cb_t cb)
{
	hid_cmd_wdata_ext_cb = cb;
}
#endif
void usbd_drv_hid_buf_reset(void)
{
	xQueueReset(hid->hid_event_Queue);                                      /* Reset HID Queue */
	usb_ep_set_halt(hid->int_in_ep, hid_hs_in_ep_desc.bEndpointAddress, 0); /* Flush endpoint buffer */
}

void usbd_drv_hid_cmd_rdata(uint8_t *buf,uint32_t len)
{
	uint32_t event;

	//USBD_HID_PRINT("%s-len=%d\r\n", __FUNCTION__, len);

	if(hid) {
		if (hid->err_state != HID_ERR_NO_ERROR) {
			return;
		}
#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN)			
		xSemaphoreTake(usbd_hid_key_xfr_finish, portMAX_DELAY);
#endif
		memcpy(hid->report_data, buf,len);
		hid->report_length = len;
		event = HID_EVENT_READ;
		xQueueSendToBack(hid->hid_event_Queue, &event, 0);
	}
}

void usbd_drv_hid_set_desc_info(struct hid_device_desc_info *hid_desc_info)
{
	if (hid_desc_info == NULL) {
		return;
	}
#if defined(CONFIG_PLATFORM_SN98660 )
	if (hid_desc_info->idVendor != NULL) {
		hid_device_desc.idVendor = *(hid_desc_info->idVendor);
	}
	if (hid_desc_info->idProduct != NULL) {
		hid_device_desc.idProduct = *(hid_desc_info->idProduct);
	}
	if (hid_desc_info->bcdDevice != NULL) {
		hid_device_desc.bcdDevice = *(hid_desc_info->bcdDevice);
	}

	if (hid_desc_info->strVendor != NULL) {
		hid_strings[STRING_MANUFACTURER_IDX].s = hid_desc_info->strVendor;
	}
	if (hid_desc_info->StrProduct != NULL) {
		hid_strings[STRING_PRODUCT_IDX].s = hid_desc_info->StrProduct;
	}
	if (hid_desc_info->StrConfig != NULL) {
		hid_strings[STRING_DESCRIPTION_IDX].s = hid_desc_info->StrConfig;
	}

	if (hid_desc_info->bmAttributes != NULL) {
		hid_config_driver.bmAttributes = *(hid_desc_info->bmAttributes);
	}
	if (hid_desc_info->bMaxPower != NULL) {
		hid_config_driver.bMaxPower = *(hid_desc_info->bMaxPower);
	}
#endif
	//USBD_HID_PRINT("idVendor=%x, idProduct=%x, bcdDevice=%x\n", hid_desc_info.idVendor, hid_desc_info.idProduct, hid_desc_info.bcdDevice);
	//USBD_HID_PRINT("strVendor=%s, strProduct=%s, strConfig=%s\n", hid_strings[STRING_MANUFACTURER_IDX].s, hid_strings[STRING_PRODUCT_IDX].s, hid_strings[STRING_DESCRIPTION_IDX].s);
}

int usbd_drv_hid_get_and_reset_state(void)
{
	int state = HID_ERR_HOST_NOT_FOUND;

	if (hid) {
		state = hid->err_state;
		if (hid->err_state != HID_ERR_NO_ERROR) {
			hid->err_state = HID_ERR_NO_ERROR;
			usbd_drv_hid_buf_reset();
		}
	}

	return state;
}

#endif //#if defined( CONFIG_MODULE_USBD_HID_CLASS )
