/**
* @file
* this is usb device driver file
* composite.c
* @author IP2/Luka
*/
#include "sonix_config.h"

#include "FreeRTOS.h"
#include "composite.h"
#include <string.h>

#include "usb_device.h"
#include "snx_udc.h"
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif

#define USB_BUFSIZ 1024

struct usb_composite_driver *composite;

static int composite_bind(struct usb_gadget *gadget);
static void composite_unbind(struct usb_gadget *gadget);
static int composite_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl);
static void composite_disconnect(struct usb_gadget *gadget);
static void composite_suspend(struct usb_gadget *gadget);
static void composite_resume(struct usb_gadget *gadget);


struct usb_request *usb_ep_alloc_request(struct usb_ep *ep);


void usb_ep_autoconfig_reset(struct usb_gadget *gadget);
void composite_ep0_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep1_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep2_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep3_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep4_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep5_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep6_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep7_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep8_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep9_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep10_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep11_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep12_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep13_callback(struct usb_gadget *gadget,struct usb_ep *ep);
void composite_ep14_callback(struct usb_gadget *gadget,struct usb_ep *ep);
/**
* @brief interface function - reset configuration
* @param cdev the pointer of usb composite device .
*/
static void reset_config(struct usb_composite_dev *cdev)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);
	cdev->config = NULL;
}
/**
* @brief interface function - set configuration
* @param cdev the pointer of usb composite device .
* @param ctrl the request of control endpoint.
* @param number the number of configuration.
* @return value of error code.
*/
static int set_config(struct usb_composite_dev *cdev,const struct usb_ctrlrequest *ctrl, unsigned number)
{
	struct usb_configuration    *c;
	int                         result;
	//unsigned                    power = 100;
	int                         tmp;


	USBD_PRINT("%s number=%d\r\n",__FUNCTION__,number);
	c = NULL;
	result = -EINVAL;
	//power = 100;

	if (number) {
		cdev->config = cdev->configs[number-1];
		c = cdev->config;
	} else {
		if (cdev->config)
			reset_config(cdev);
		result = 0;
	}

	if (!c)
		goto done;
	//cdev->config = c;

	/* Initialize all interfaces by setting them to altsetting zero. */
	for (tmp = 0; tmp < MAX_CONFIG_INTERFACES; tmp++) {
		struct usb_function             *f;
		struct usb_descriptor_header    **descriptors;

		f = c->interface[tmp];
		if (!f)
			break;

		descriptors = f->hs_descriptors;

		for (; *descriptors; ++descriptors) {
			//struct usb_endpoint_descriptor *ep;
			//int addr;

			if ((*descriptors)->bDescriptorType != USB_DT_ENDPOINT)
				continue;

			//ep = (struct usb_endpoint_descriptor *)*descriptors;
			//addr = ((ep->bEndpointAddress & 0x80) >> 3)|  (ep->bEndpointAddress & 0x0f);
			//set_bit(addr, f->endpoints);
		}

		result = f->set_alt(f, tmp, 0);
		if (result < 0) {
			USBD_PRINT("interface %d (%s/%p) alt 0 --> %d\r\n",tmp, f->name, f, result);
			reset_config(cdev);
			goto done;
		}
	}

	/* when we return, be sure our power usage is valid */
	//power = c->bMaxPower ? (2 * c->bMaxPower) : CONFIG_USB_GADGET_VBUS_DRAW;
done:
	return result;
}

static struct usb_gadget_driver composite_driver ;

/**
* @brief interface function - endpoint 0 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep0_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[0])
		f->ep_callback[0](f,ep);
}
/**
* @brief interface function - endpoint 1 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep1_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f = cdev->config->function;

	if (f && f->ep_callback[1]) {
		f->ep_callback[1](f,ep);
	}
}
/**
* @brief interface function - endpoint 2 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep2_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f = cdev->config->function;

	if (f && f->ep_callback[2]) {
		f->ep_callback[2](f,ep);
	}
}
/**
* @brief interface function - endpoint 3 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep3_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[3]) {
		f->ep_callback[3](f,ep);
	}
}
/**
* @brief interface function - endpoint 4 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep4_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[4]) {
		f->ep_callback[4](f,ep);
	}
}
/**
* @brief interface function - endpoint 5 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep5_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[5]) {
		f->ep_callback[5](f,ep);
	}
}
/**
* @brief interface function - endpoint 6 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep6_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[6])
		f->ep_callback[6](f,ep);
}
/**
* @brief interface function - endpoint 7 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep7_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[7])
		f->ep_callback[7](f,ep);
}
/**
* @brief interface function - endpoint 8 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep8_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[8])
		f->ep_callback[8](f,ep);
}
/**
* @brief interface function - endpoint 9 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep9_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[9])
		f->ep_callback[9](f,ep);
}
/**
* @brief interface function - endpoint 10 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep10_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[10])
		f->ep_callback[10](f,ep);
}
/**
* @brief interface function - endpoint 11 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep11_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[11])
		f->ep_callback[11](f,ep);
}
/**
* @brief interface function - endpoint 12 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep12_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[12])
		f->ep_callback[12](f,ep);
}
/**
* @brief interface function - endpoint 13 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep13_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[13])
		f->ep_callback[13](f,ep);
}
/**
* @brief interface function - endpoint 14 callback function
* @param gadget the pointer of usb gadget.
* @param ep the pointer of usb endpoint.
*/
void composite_ep14_callback(struct usb_gadget *gadget,struct usb_ep *ep)
{
	struct usb_composite_dev	*cdev;
	struct usb_function			*f;

	cdev = gadget->cdev;

	f = NULL;
	f= cdev->config->function;

	if (f && f->ep_callback[14])
		f->ep_callback[14](f,ep);
}
/**
* @brief interface function - composite bind function
* @param gadget the pointer of usb gadget.
* @return value of error code.
*/
static int composite_bind(struct usb_gadget *gadget)
{
	struct usb_composite_dev        *cdev;
	int32_t                         status;

	status = -ENOMEM;

	composite_driver.ep_callback[0] = composite_ep0_callback;
	composite_driver.ep_callback[1] = composite_ep1_callback;
	composite_driver.ep_callback[2] = composite_ep2_callback;
	composite_driver.ep_callback[3] = composite_ep3_callback;
	composite_driver.ep_callback[4] = composite_ep4_callback;
	composite_driver.ep_callback[5] = composite_ep5_callback;
	composite_driver.ep_callback[6] = composite_ep6_callback;
	composite_driver.ep_callback[7] = composite_ep7_callback;
	composite_driver.ep_callback[8] = composite_ep8_callback;
	composite_driver.ep_callback[9] = composite_ep9_callback;
	composite_driver.ep_callback[10] = composite_ep10_callback;
	composite_driver.ep_callback[11] = composite_ep11_callback;
	composite_driver.ep_callback[12] = composite_ep12_callback;
	composite_driver.ep_callback[13] = composite_ep13_callback;
	composite_driver.ep_callback[14] = composite_ep14_callback;



	cdev =(struct usb_composite_dev*)usbd_malloc(sizeof *cdev);
	if (!cdev)
		return status;

	memset(cdev,0x00,sizeof *cdev);

	cdev->gadget = gadget;
	gadget->cdev = cdev;

	cdev->req = usb_ep_alloc_request(gadget->ep0);
	if (!cdev->req)
		goto fail;

	cdev->req->buf =(unsigned char*)usbd_malloc(USB_BUFSIZ);
	if (!cdev->req->buf)
		goto fail;

	cdev->bufsiz = USB_BUFSIZ;
	cdev->driver = composite;



	status = composite->bind(cdev);                                         //ref to xxx_bind
	if (status < 0)
		goto fail;

	cdev->desc = *composite->dev;
	cdev->qualifier = composite->qualifier;

	cdev->desc.bMaxPacketSize0 = gadget->ep0->maxpacket;

	cdev->qualifier->bMaxPacketSize0 = cdev->desc.bMaxPacketSize0;


	USBD_PRINT("%s ready\r\n", composite->name);
	return 0;
fail:
	composite_unbind(gadget);
	return status;
}
/**
* @brief interface function - composite unbind function
* @param gadget the pointer of usb gadget.
* @return value of error code.
*/
static void composite_unbind(struct usb_gadget *gadget)
{
	struct usb_composite_dev *cdev;

	cdev = gadget->cdev;

	if (cdev->driver->unbind) {
		cdev->driver->unbind(cdev);
	}

	if (cdev->req) {
		if(cdev->req->buf) {
			vPortFree(cdev->req->buf);
			cdev->req->buf = NULL;
		}
		usb_ep_free_request(gadget->ep0, cdev->req);
	}
	vPortFree(cdev);
	cdev = NULL;
}
/**
* @brief interface function - composite setup function
* @param gadget the pointer of usb gadget.
* @param ctrl the request of control endpoint.
* @return value of error code.
*/
static int composite_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	struct usb_function			*f;
	struct usb_composite_dev	*cdev;
	unsigned short				w_index;
	unsigned short				w_value;
	unsigned char				intf;
	//unsigned char				endp;
	int							value;
	struct usb_request			*req;
	struct usb_ep				*ep;

	f = NULL;
	cdev = gadget->cdev;
	req = cdev->req;
	ep = cdev->gadget->ep0;

	w_index = ctrl->wIndex;
	w_value = ctrl->wValue;
	intf = w_index & 0xFF;
	value = -EOPNOTSUPP;

	USBD_PRINT("%s \r\n", __FUNCTION__);

	switch (ctrl->bRequest) {
	case USB_REQ_GET_CONFIGURATION:
		if (ctrl->bRequestType != USB_DIR_IN)
			goto unknown;

		if (cdev->config)
			*(unsigned char *)req->buf = cdev->config->bConfigurationValue;
		else
			*(unsigned char *)req->buf = 0;

		req->length = min(ctrl->wLength, (unsigned short) 1);
		value = usb_ep_queue(ep, req);
		break;
	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->bRequestType != 0)
			goto unknown;

		value = set_config(cdev, ctrl, w_value);
		break;
	case USB_REQ_GET_INTERFACE:
		if (ctrl->bRequestType != (USB_DIR_IN|USB_RECIP_INTERFACE))
			goto unknown;

		if (!cdev->config || intf >= MAX_CONFIG_INTERFACES)
			break;

		f = cdev->config->interface[intf];
		if (!f)
			break;

		/* lots of interfaces only need altsetting zero... */
		value = f->get_alt ? f->get_alt(f, w_index, w_value) : 0;
		if (value < 0)
			break;

		*((unsigned char *)req->buf) = value;
		req->length = min(ctrl->wLength, (unsigned short) 1);
		value = usb_ep_queue(ep, req);
		break;
	case USB_REQ_SET_INTERFACE:
		if (ctrl->bRequestType != USB_RECIP_INTERFACE)
			goto unknown;

		if (!cdev->config || intf >= MAX_CONFIG_INTERFACES)
			break;

		f = cdev->config->interface[intf];
		if (!f)
			break;

		if (w_value && !f->set_alt)
			break;

		value = f->set_alt(f, w_index, w_value);
		if (value < 0) {
			USBD_PRINT("interface %d (%s/%p) alt %d --> %d\n",w_index, f->name, f, w_value,value);
			reset_config(cdev);
		}

		goto done;
	case USB_REQ_GET_DESCRIPTOR:
		if (ctrl->bRequestType != 0) {
			goto unknown;
		}
		value = set_config(cdev, ctrl, w_value);
		goto done;
	default:
unknown:
		USBD_PRINT("non-core control req%02x.%02x v%04x i%04x l%d\r\n",ctrl->bRequestType, ctrl->bRequest,ctrl->wValue, ctrl->wIndex, ctrl->wLength);
		switch (ctrl->bRequestType & USB_RECIP_MASK) {
		case USB_RECIP_INTERFACE:
			if (intf >= MAX_CONFIG_INTERFACES) {
				req->length = 0;
				usb_ep_set_halt(gadget->ep0, 0, 1);
				value = -EINVAL;
				goto done;
			}

			if (cdev->config)
				f = cdev->config->interface[intf];
			break;

		case USB_RECIP_ENDPOINT:
			f = cdev->config->function;
		}
		if (f && f->setup)
			value = f->setup(f, ctrl);      //ref to xxx_function_setup
		goto done;
	}
done:
	return value;
}
/**
* @brief interface function - composite disconnect function
* @param gadget the pointer of usb gadget.
*/
static void composite_disconnect(struct usb_gadget *gadget)
{

}
/**
* @brief interface function - composite suspend function
* @param gadget the pointer of usb gadget.
*/
static void composite_suspend(struct usb_gadget *gadget)
{

}
/**
* @brief interface function - composite resume function
* @param gadget the pointer of usb gadget.
*/
static void composite_resume(struct usb_gadget *gadget)
{

}
/**
* @brief interface function - fill descriptor buffer
* @param buf the pointer of buffer.
* @param buflen the size of buffer.
* @param src the pointer of descriptor header's pointer
* @return the length of descriptor
*/
int usb_descriptor_fillbuf(void *buf, unsigned buflen,const struct usb_descriptor_header **src)
{
	unsigned char   *dest = buf;

	if (!src)
		return -EINVAL;

	/* fill buffer from src[] until null descriptor ptr */
	for (; NULL != *src; src++) {
		unsigned len = (*src)->bLength;

		if (len > buflen) return -EINVAL;
		memcpy(dest, *src, len);
		buflen -= len;
		dest += len;
	}
	return dest - (unsigned char *)buf;
}
/**
* @brief interface function - get configuration count
* @param cdev the pointer of usb composite device .
* @param type the type of descriptor.
* @return the count of configuration
*/
int count_configs(struct usb_composite_dev *cdev, unsigned type)
{
	struct usb_configuration        *c;
	unsigned                        count;

	count = 0;


	c = cdev->configs[0];
	if(c) {
		count = 1;
		return count;
	}

	return count;
}
/**
* @brief interface function - get configuration count
* @param config the pointer of usb usb_configuration .
* @param speed the speed type of usb device.
* @param buf the pointer of configuration buffer.
* @param type the type of descriptor.
* @return the lenth of configuration
*/
static int config_buf(struct usb_configuration *config,enum usb_device_speed speed, uint8_t *buf, uint8_t type)
{
	struct usb_config_descriptor    *c;
	uint8_t                         *next;
	int                             len;
	struct usb_function             *f;
	int                             status;
	struct usb_descriptor_header    **descriptors = NULL;

	USBD_PRINT( "%s\r\n",__FUNCTION__);
	next = buf + USB_DT_CONFIG_SIZE;
	len = USB_BUFSIZ - USB_DT_CONFIG_SIZE;

	/* write the config descriptor */
	c = (void*)buf;
	c->bLength = USB_DT_CONFIG_SIZE;
	c->bDescriptorType = type;
	/* wTotalLength is written later */
	c->bNumInterfaces = config->next_interface_id;
	c->bConfigurationValue = config->bConfigurationValue;
	//c->iConfiguration = 0;
	c->iConfiguration = config->iConfiguration;
	c->bmAttributes = USB_CONFIG_ATT_ONE | config->bmAttributes;
	c->bMaxPower = config->bMaxPower ? 1 : (CONFIG_USB_GADGET_VBUS_DRAW / 2);

	/* add each function's descriptors */
	f = config->function;

	if(speed == USB_SPEED_HIGH) {
		descriptors = (type != USB_DT_OTHER_SPEED_CONFIG) ? f->hs_descriptors : f->descriptors;
	}
	else if(speed == USB_SPEED_FULL) {
		descriptors = (type != USB_DT_OTHER_SPEED_CONFIG) ? f->descriptors : f->hs_descriptors;
	}

	status = usb_descriptor_fillbuf(next, len,(const struct usb_descriptor_header **) descriptors);

	if (status < 0)return status;
	len -= status;
	next += status;

	len = next - buf;
	c->wTotalLength = len;
	USBD_PRINT( "len%04x\r\n",len);
	return len;
}
/**
* @brief interface function - get configuration descriptor length
* @param cdev the pointer of usb usb_configuration .
* @param w_value the value of descriptor type.
* @return the lenth of configuration
*/
int config_desc(struct usb_composite_dev *cdev, unsigned w_value)
{
	struct usb_configuration        *c;
	uint8_t                         type;
	enum usb_device_speed           speed;

	USBD_PRINT( "%s\r\n",__FUNCTION__);
	type = w_value >> 8;

	speed = USB_SPEED_UNKNOWN;

	speed = cdev->gadget->speed;

	/* This is a lookup by config *INDEX* */
	w_value &= 0xff;

	c = cdev->configs[0];
	if(c)
		return config_buf(c, speed, cdev->req->buf, type);
	else
		return -EINVAL;
}
/**
 * bos_desc() - prepares the BOS descriptor.
 * @cdev: pointer to usb_composite device to generate the bos
 *      descriptor for
 *
 * This function generates the BOS (Binary Device Object)
 * descriptor and its device capabilities descriptors. The BOS
 * descriptor should be supported by a SuperSpeed device.
 */
int bos_desc(struct usb_composite_dev *cdev)
{
	struct usb_ext_cap_descriptor *usb_ext;
#ifdef USB3_SUPPORT
	struct usb_ss_cap_descriptor *ss_cap;
#endif
	struct usb_bos_descriptor *bos;

	bos = (struct usb_bos_descriptor*)cdev->req->buf;
	bos->bLength = sizeof(struct usb_bos_descriptor);
	bos->bDescriptorType = USB_DT_BOS;

	bos->wTotalLength = bos->bLength;
	bos->bNumDeviceCaps = 0;

	/*
	 * A SuperSpeed device shall include the USB2.0 extension descriptor
	 * and shall support LPM when operating in USB2.0 HS mode.
	 */
	usb_ext = (struct usb_ext_cap_descriptor *)(cdev->req->buf + bos->wTotalLength);
	usb_ext->bLength = sizeof(struct usb_ext_cap_descriptor);
	bos->bNumDeviceCaps++;
	bos->wTotalLength += usb_ext->bLength;
	usb_ext->bDescriptorType = USB_DT_DEVICE_CAPABILITY;
	usb_ext->bDevCapabilityType = USB_CAP_TYPE_EXT;
#ifdef DEV_POWER_SAVING_EN
	usb_ext->bmAttributes = (USB_LPM_SUPPORT | USB_BESL_SUPPORT | USB_BESL_BASELINE_VALID);
#else
	usb_ext->bmAttributes = 0;
#endif

	/*
	 * The Superspeed USB Capability descriptor shall be implemented by all
	 * SuperSpeed devices.
	 */
#ifdef USB3_SUPPORT
	ss_cap = cdev->req->buf + bos->wTotalLength;
	ss_cap->bLength = sizeof(struct usb_ss_cap_descriptor);
	bos->bNumDeviceCaps++;
	bos->wTotalLength += ss_cap->bLength;
	ss_cap->bDescriptorType = USB_DT_DEVICE_CAPABILITY;
	ss_cap->bDevCapabilityType = USB_SS_CAP_TYPE;
#ifdef DEV_POWER_SAVING_EN
	ss_cap->bmAttributes = USB_LPM_SUPPORT | USB_BESL_SUPPORT,              // LTM is supported , BESL also support
#else
	ss_cap->bmAttributes = 0; /* LTM is not supported yet */
#endif
	        ss_cap->wSpeedSupported = (USB_FULL_SPEED_OPERATION | USB_HIGH_SPEED_OPERATION | USB_5GBPS_OPERATION);
	ss_cap->bFunctionalitySupport = USB_LOW_SPEED_OPERATION;

	ss_cap->bU1devExitLat = 0x04;
	ss_cap->bU2DevExitLat = 0x0002;
#endif
	return bos->wTotalLength;
}
/**
* @brief interface function - ascii to unicode
* @param s the pointer of source ascii.
* @param buf the pointer of unicode.
* @param len the lenth of ascii' char.
* @return the lenth of ascii' char.
*/
int ansitouni(const char *s,uint8_t *buf, unsigned len)
{
	int i;

	for(i=0; i<len; i++) {
		buf[i*2+0]=s[i];
		buf[i*2+1]=0x00;
	}

	return len;
}
/**
* @brief interface function - get string
* @param table the pointer of string array.
* @param id the id of string.
* @param buf the pointer of buffer.
* @return the lenth of string.
*/
int usb_gadget_get_string (struct usb_gadget_strings* table, int id, uint8_t *buf)
{
	struct usb_string       *temp_s;
	int                     len;

	for (temp_s = table->strings; temp_s && temp_s->s; temp_s++) {
		if (temp_s->id == id)
			break;
		if (strcmp(temp_s->s,"") == 0)
			break;
	}

	if (!temp_s || !temp_s->s || (strcmp(temp_s->s,"") == 0))
		return -EINVAL;

	len = min ((size_t) 126, strlen (temp_s->s));
	memset (buf + 2, 0, 2 * len);   /* zero all the bytes */
	len = ansitouni(temp_s->s, &buf[2], len);
	if (len < 0)
		return -EINVAL;

	buf [0] = (len + 1) * 2;
	buf [1] = USB_DT_STRING;
	return buf [0];
}
/**
* @brief interface function - lookup string
* @param sp the pointer of string array.
* @param buf the pointer of buffer.
* @param language the value of language type.
* @param id the id of string.
* @return the lenth of string or error code.
*/
static int lookup_string(struct usb_gadget_strings *sp, void *buf,uint16_t language,int id)
{
	struct usb_gadget_strings       *s;
	int                             value;

	if (sp) {
		s = sp++;
		if (s->language != language)
			return -EINVAL;
		value = usb_gadget_get_string(s, id, buf);
		if (value > 0)
			return value;
	}
	return -EINVAL;
}
/**
* @brief interface function - get string
* @param cdev the pointer of usb composite dev .
* @param buf the pointer of buffer.
* @param language the value of language type.
* @param id the id of string.
* @return the lenth of string or error code.
*/
int get_string(struct usb_composite_dev *cdev,uint8_t *buf, uint16_t language, int id)
{
	struct usb_configuration        *c;
	struct usb_function             *f;
	int                             len;


	if(cdev->config ==  NULL) {
		c = cdev->configs[0];
		f = c->function;
	}
	else {
		c = cdev->config;
		f = cdev->config->function;
	}

	if (id == 0 && composite->strings) {
		buf [0] = 4;
		buf [1] = USB_DT_STRING;
		buf [2] = (uint8_t) composite->strings->language;
		buf [3] = (uint8_t) (composite->strings->language >> 8);
		return 4;
	}

	if (composite->strings) {
		len = lookup_string(composite->strings, buf, language, id);
		if (len > 0)
			return len;
	}

	if (c->strings) {
		len = lookup_string(c->strings, buf, language, id);
		if (len > 0)
			return len;
	}

	if (f->strings) {
		len = lookup_string(f->strings, buf, language, id);
		if (len > 0)
			return len;
	}

	if (len <= 0) {
		buf [0] = 4;
		buf [1] = USB_DT_STRING;
		buf [2] = (uint8_t) composite->strings->language;
		buf [3] = (uint8_t) (composite->strings->language >> 8);
		return 4;
	}

	return -EINVAL;
}
/**
* @brief interface function - register composite
* @param driver the structure pointer for usb composite driver.
* @return value of error code.
*/
int usb_composite_register(struct usb_composite_driver *driver)
{
	composite_driver.speed          = USB_SPEED_HIGH;
	composite_driver.bind           = composite_bind;
	composite_driver.unbind         = composite_unbind;
	composite_driver.setup          = composite_setup;
	composite_driver.disconnect     = composite_disconnect;
	composite_driver.suspend        = composite_suspend;
	composite_driver.resume         = composite_resume;

	composite_driver.function       = (char *) driver->name;
	composite                       = driver;

	return usb_gadget_register_driver(&composite_driver);
}
int usb_composite_unregister()
{
	return usb_gadget_unregister_driver(&composite_driver);
}
/**
* @brief interface function - reset endpoint configuration
* @param gadget the pointer of usb gadget.
*/
void usb_ep_autoconfig_reset (struct usb_gadget *gadget)
{

}
/**
* @brief interface function - enable usb ep
* @param ep the pointer of usb ep.
* @param desc the pointer of dependent descriptor.
* @return value of error code.
*/
int usb_ep_enable(struct usb_ep *ep,const struct usb_endpoint_descriptor *desc)
{
	USBD_PRINT("%s\r\n",__FUNCTION__);
	return ep->ops->enable(ep, desc);
}
int usb_ep_disable(struct usb_ep *ep)
{
	USBD_PRINT("%s\r\n",__FUNCTION__);
	return ep->ops->disable(ep);
}
/**
* @brief interface function - add configuration
* @param cdev the point of usb composite device.
* @param config the point of usb configuration.
* @return value of error code.
*/
int usb_add_config(struct usb_composite_dev *cdev,struct usb_configuration *config)
{
	int status;

	USBD_PRINT("adding config #%u '%s'/%p\r\n",config->bConfigurationValue,config->label, config);

	status = -EINVAL;

	if (!config->bConfigurationValue || !config->bind)
		goto done;

	config->cdev = cdev;

	cdev->configs[0] = config;
	cdev->config = config;

	config->next_interface_id = 0;

	status = config->bind(config);          //ref to msc_config_bind or uvc_config_bind
	if (status < 0) {
		config->cdev = NULL;
	} else {
		unsigned        i;
		for (i = 0; i < MAX_CONFIG_INTERFACES; i++) {
			struct usb_function     *f;

			f = config->interface[i];
			if (!f)
				continue;
			//DBG(cdev, "  interface %d = %s/%p\n",i, f->name, f);
		}
	}

	usb_ep_autoconfig_reset(cdev->gadget);

done:
	if (status)
		USBD_PRINT("added config '%s'/%u --> %d\r\n", config->label,config->bConfigurationValue, status);
	return status;
}

void usb_free_descriptors(struct usb_descriptor_header **v)
{
	vPortFree(v);
	v = NULL;
}
/**
* @brief interface function - add usb function
* @param config the point of usb configuration.
* @param function the point of usb function.
* @return value of error code.
*/
int usb_add_function(struct usb_configuration *config,struct usb_function *function)
{
	int	value;

	USBD_PRINT("adding '%s'/%p to config '%s'/%p\r\n",        function->name, function,config->label, config);

	value = -EINVAL;
	if (!function->set_alt || !function->disable)
		goto done;

	function->config = config;
	config->function = function;

	/* REVISIT *require* function->bind? */
	if (function->bind) {
		value = function->bind(config, function);                       //ref to XXX_function_bind
		if (value < 0) {
			function->config = NULL;
		}
	} else
		value = 0;

	/*
	if (!config->fullspeed && function->descriptors)
		config->fullspeed = true;
	if (!config->highspeed && function->hs_descriptors)
		config->highspeed = true;
	*/
done:
	//if (value)    USBD_PRINT("adding '%s'/%p --> %d\n",function->name, function, value);
	return value;
}
/**
* @brief interface function - get interface id
* @param config the point of usb configuration.
* @param function the point of usb function.
* @return interface id or the value of error code.
*/
int usb_interface_id(struct usb_configuration *config,  struct usb_function *function)
{
	unsigned id = config->next_interface_id;

	if (id < MAX_CONFIG_INTERFACES) {
		config->interface[id] = function;
		config->next_interface_id = id + 1;
		return id;
	}
	return -ENODEV;
}
/**
* @brief interface function - find match endpoint
* @param gadget the point of usb gadget.
* @param ep the point of usb endpoint.
* @param desc the point of usb endpoint descriptor.
* @return the value of error code.
*/
static int ep_matches (struct usb_gadget *gadget,struct usb_ep *ep, struct usb_endpoint_descriptor  *desc, char *class_type)
{
	unsigned char	type = 0;
	unsigned char	num = 0;
	unsigned char	dir = 0;
	unsigned char	find_ep = 0;
	
	/* only support ep0 for portable CONTROL traffic */
	type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	if (USB_ENDPOINT_XFER_CONTROL == type)
		return 0;

	dir = (desc->bEndpointAddress & USB_DIR_MASK);
	/* some other naming convention */
	if ('e' != ep->name[0])
		return 0;

	if( strcmp(class_type, "MSC-BULK") == 0){
		if((strcmp(ep->name,"epc-bulk") == 0) && (type == USB_ENDPOINT_XFER_BULK) && (desc->bEndpointAddress == USB_DIR_IN)){
			num = 12;
			find_ep = 1;
		}
		else if((strcmp(ep->name,"epd-bulk")==0) && (type == USB_ENDPOINT_XFER_BULK) && (desc->bEndpointAddress == USB_DIR_OUT)){
			num = 13;
			find_ep = 1;
		}
	}
	else if(strcmp(class_type, "UVC-BULK") == 0){
		if((strcmp(ep->name,"epe-bulk")==0 )&& (type == USB_ENDPOINT_XFER_BULK) && (dir == USB_DIR_IN )){
			num = 14;
			find_ep = 1;			
		}
	}
	else if(strcmp(class_type, "UVC-ISO") == 0){
		if((strcmp(ep->name,"ep7-iso")==0 )&& (type == USB_ENDPOINT_XFER_ISOC) && (dir == USB_DIR_IN )){
			num = 7;
			find_ep = 1;			
		}
	}
	else if(strcmp(class_type, "UVC-INT") == 0){
		if((strcmp(ep->name,"ep5-int")==0 )&& (type == USB_ENDPOINT_XFER_INT) && (dir == USB_DIR_IN )){
			num = 5;
			find_ep = 1;
		}			
	}	
	else if(strcmp(class_type, "UAC-ISO") == 0){
		if((strcmp(ep->name,"ep3-iso")==0 )&& (type == USB_ENDPOINT_XFER_ISOC) && (dir == USB_DIR_IN )){
			num = 3;
			find_ep = 1;
		}
		else if((strcmp(ep->name,"ep4-iso")==0 )&& (type == USB_ENDPOINT_XFER_ISOC) && (dir == USB_DIR_OUT )){
			num = 4;
			find_ep = 1;			
		}
	}
	else if(strcmp(class_type, "HID-INT") == 0){
		if((strcmp(ep->name,"ep1-int")==0 )&& (type == USB_ENDPOINT_XFER_INT) && (dir == USB_DIR_IN )){
			num = 1;	
			find_ep = 1;
		}
	}	
	else
		return 0;
	
	if(find_ep == 1){
		desc->bEndpointAddress |= num;
		return 1;
	}
	else
		return 0;
//	if((strcmp(ep->name,"epc-bulk")==0 )&& (type == USB_ENDPOINT_XFER_BULK) && (desc->bEndpointAddress ==USB_DIR_IN ))
//		num = 12;
//	else if((strcmp(ep->name,"epd-bulk")==0 )&& (type == USB_ENDPOINT_XFER_BULK) && (desc->bEndpointAddress ==USB_DIR_OUT ))
//		num = 13;
//	else if((strcmp(ep->name,"ep5-int")==0 )&& (type == USB_ENDPOINT_XFER_INT))
//		num = 5;

//	else if((strcmp(ep->name,"ep7-iso")==0 )&& (type == USB_ENDPOINT_XFER_ISOC))
//		num = 7;
//	else if((strcmp(ep->name,"ep3-iso")==0 )&& (type == USB_ENDPOINT_XFER_ISOC)&& (desc->wMaxPacketSize < 0x100 ) && (dir == USB_DIR_IN))
//		num = 3;
//	else if((strcmp(ep->name,"ep4-iso")==0 )&& (type == USB_ENDPOINT_XFER_ISOC)&& (desc->wMaxPacketSize < 0x100 ) && (dir == USB_DIR_OUT))
//		num = 4;	
//	else
//		return 0;

//	desc->bEndpointAddress |= num;
//	return 1;
}
/**
* @brief interface function - auto configuration endpoint
* @param gadget the point of usb gadget.
* @param desc the point of usb endpoint descriptor.
* @return the point of endpoint.
*/
struct usb_ep *usb_ep_autoconfig (struct usb_gadget *gadget,    struct usb_endpoint_descriptor  *desc, char *class_type)
{
	struct usb_ep   *ep;
	uint32_t        i;

	for(i=0; i<15; i++)
	{
		ep = gadget->ep_list[i];
		if (ep_matches (gadget, ep, desc, class_type))
			return ep;
	}

	return NULL;
}
/**
* @brief interface function - copy usb descriptors
* @param src the point of usb descriptor header's pointer.
* @return the point of usb descriptor header's pointer.
*/
struct usb_descriptor_header **usb_copy_descriptors(struct usb_descriptor_header **src)
{
	struct usb_descriptor_header    **ret;
	struct usb_descriptor_header    **tmp;
	uint16_t                        bytes;
	uint16_t                        n_desc;
	uint8_t                         *mem;

	/* count descriptors and their sizes; then add vector size */
	for (bytes = 0, n_desc = 0, tmp = src; *tmp; tmp++, n_desc++)
		bytes += (*tmp)->bLength;

	bytes += (n_desc + 1) * sizeof(*tmp);
	mem = (uint8_t*)usbd_malloc(bytes);
	if (!mem)
		return NULL;

	/* fill in pointers starting at "tmp",
	 * to descriptors copied starting at "mem";
	 * and return "ret"
	 */
	tmp = (void*)mem;
	ret = (void*)mem;
	mem += (n_desc + 1) * sizeof(*tmp);
	while (*src) {
		memcpy(mem, *src, (*src)->bLength);
		*tmp = (void*)mem;
		tmp++;
		mem += (*src)->bLength;
		src++;
	}
	*tmp = NULL;
	return ret;
}
/**
* @brief interface function - allocate usb request memory in assigning endpoint
* @param ep the point of usb ep.
* @return the point of usb ep request.
*/
struct usb_request *usb_ep_alloc_request(struct usb_ep *ep)
{
	return ep->ops->alloc_request(ep);
}
/**
* @brief interface function - free usb request memory in assigning endpoint
* @param ep the pointer of usb ep.
* @param req the pointer of usb ep request.
*/
void usb_ep_free_request(struct usb_ep *ep,struct usb_request *req)
{
	ep->ops->free_request(ep, req);
}

int usb_string_id(struct usb_composite_dev *cdev)
{
	if (cdev->next_string_id < 254) {
		/* string id 0 is reserved */
		cdev->next_string_id++;
		return cdev->next_string_id;
	}
	return -ENODEV;
}


/**
* @brief interface function - execute queue with assigning endpoint
* @param ep the pointer of usb enpoint
* @param req the pointer of enpoint request
* @return value of error code.
*/
int usb_ep_queue(struct usb_ep *ep,  struct usb_request *req)
{
	return ep->ops->queue(ep, req);
}
/**
* @brief interface function - stop queue with assigning endpoint
* @param ep the pointer of usb enpoint
* @param req the pointer of enpoint request
* @return value of error code.
*/
int usb_ep_dequeue(struct usb_ep *ep,  struct usb_request *req)
{
	return ep->ops->dequeue(ep, req);
}
/**
* @brief interface function - reset usb endpoint status
* @param ep the pointer of usb enpoint
* @return value of error code.
*/
int usb_ep_reset(struct usb_ep *ep)
{
	return ep->ops->reset(ep);
}
/**
* @brief interface function - set halt assigning endpoint
* @param ep the pointer of usb endpoint
* @param epnum the number of endpoint
* @param value the value of status (disable:0/enable:1)
* @return value of error code.
*/
int usb_ep_set_halt(struct usb_ep *ep,int epnum, int value)
{
	return ep->ops->set_halt(epnum, value);
}
