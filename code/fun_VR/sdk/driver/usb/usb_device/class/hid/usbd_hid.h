/**
* @file
* this is usb device driver file
* usbd_hid.h
* @author IP2/Luka
*/
#ifndef __SONIX_USBD_HID_H
#define __SONIX_USBD_HID_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "ch9.h"
#include "composite.h"

#if defined(CONFIG_MODULE_USBD_HID_DEBUG)
#define USBD_HID_PRINT(fmt, args...) printf("\r[USBD_HID]"fmt"\r", ##args)
#else
#define USBD_HID_PRINT(...)		((void) 0)
#endif



// report type define
#define HID_REPORT_TYPE_NONE			(0)
#define HID_REPORT_TYPE_INPUT			(1)
#define HID_REPORT_TYPE_OUTPUT			(2)
#define HID_REPORT_TYPE_FEATURE			(3)

/*
* USB HID interface subclass and protocol codes
 */

#define USB_INTERFACE_SUBCLASS_BOOT		1
#define USB_INTERFACE_PROTOCOL_KEYBOARD	1
#define USB_INTERFACE_PROTOCOL_MOUSE	2

/*
 * HID class requests
 */

#define HID_REQ_GET_REPORT			0x01
#define HID_REQ_GET_IDLE			0x02
#define HID_REQ_GET_PROTOCOL		0x03
#define HID_REQ_SET_REPORT			0x09
#define HID_REQ_SET_IDLE			0x0A
#define HID_REQ_SET_PROTOCOL		0x0B

/*
 * HID class descriptor types
 */

#define HID_DT_HID					(USB_TYPE_CLASS | 0x01)
#define HID_DT_REPORT				(USB_TYPE_CLASS | 0x02)
#define HID_DT_PHYSICAL				(USB_TYPE_CLASS | 0x03)

#define HID_MAX_DESCRIPTOR_SIZE		4096

#define HID_HS_INTERVAL				4
#define HID_FS_INTERVAL				1		/* HID_FS_INTERVAL = (2^(HID_HS_INTERVAL-1)) / 8 */
#define HID_SEMA_TIMEOUT			(HID_FS_INTERVAL * 3)


struct hid_class_descriptor {
	uint8_t  bDescriptorType;
	uint16_t wDescriptorLength;
} __attribute__ ((packed));

struct hid_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdHID;
	uint8_t  bCountryCode;
	uint8_t  bNumDescriptors;
	struct hid_class_descriptor desc[1];
} __attribute__ ((packed));


struct hid_device_desc_info {
	uint16_t *idVendor;
	uint16_t *idProduct;
	uint16_t *bcdDevice;
	char *strVendor;
	char *StrProduct;
	char *StrConfig;

	uint8_t *bmAttributes;
	uint8_t *bMaxPower;
};

enum HID_ERR_STATE
{
	HID_ERR_NO_ERROR = 0,	/* no error */
	HID_ERR_DATA_TIMEOUT,	/* send data timeout */
	HID_ERR_HOST_NOT_FOUND,	/* no plug-in to USB Host */
};

/* SONiX HID setting */
#define HID_KB_REPORT_ID			1
#define HID_KB_REPORT_SIZE			6	/* unit: byte */

/* SONiX HID Error Code */
#define HID_SUCCESS					0	/* Send data success */
#define HID_ERR_DEV_HANDLE			1	/* HID device handle no exist */
#define HID_ERR_PREV_DATA_NOT_YET	2	/* Previous data not yet complete */



// Variable


// Functions
int usbd_hid_init(void);
int usbd_hid_uninit(void);

struct hid_func_descriptor {
	uint8_t			subclass;
	uint8_t			protocol;
	uint16_t		report_length;
	uint16_t		report_desc_length;
	char			*report_desc;
};

struct HID_USBD_CLASS_CTRL_FEATURE_CMD {
	uint8_t	bReportID;
	uint8_t	bToolOpt[4];
	uint8_t	bUSBClassCode;
	uint8_t	bReserved[6];
} __attribute__((__packed__));



/* callback function type define */
typedef void (*usbd_drv_hid_cmd_wdata_cb_t)(uint8_t *buf,uint32_t len);

void usbd_drv_hid_cmd_wdata_ext_reg_cb(usbd_drv_hid_cmd_wdata_cb_t cb);
void usbd_drv_hid_cmd_wdata_reg_cb(usbd_drv_hid_cmd_wdata_cb_t cb);

void usbd_drv_hid_cmd_rdata(uint8_t *buf,uint32_t len);
void usbd_drv_hid_set_desc_info(struct hid_device_desc_info *hid_desc_info);
int usbd_drv_hid_get_and_reset_state(void);

static char hid_report_desc[]		= {
	0x05, 0x01,		/* USAGE_PAGE (Generic Desktop)           */
	0x09, 0x06,		/* USAGE (Keyboard)                       */
	0xa1, 0x01, 		/* COLLECTION (Application)               */
	0x85, HID_KB_REPORT_ID, 		/* REPORT_ID (1)              */
	0x05, 0x07, 		/* USAGE_PAGE (Keyboard)                  */
	0x19, 0xe0, 		/* USAGE_MINIMUM (Keyboard LeftControl)   */
	0x29, 0xe7, 		/* USAGE_MAXIMUM (Keyboard Right GUI)     */
	0x15, 0x00, 		/* LOGICAL_MINIMUM (0)                    */
	0x25, 0x01, 		/* LOGICAL_MAXIMUM (1)                    */
	0x75, 0x01, 		/* REPORT_SIZE (1)                        */
	0x95, 0x08, 		/* REPORT_COUNT (8)                       */
	0x81, 0x02, 		/* INPUT (Data,Var,Abs)                   */
	0x95, 0x01, 		/* REPORT_COUNT (1)                       */
	0x75, 0x08,			/* REPORT_SIZE (8)                        */
	0x81, 0x03, 		/* INPUT (Cnst,Var,Abs)                   */
	0x95, 0x05, 		/* REPORT_COUNT (5)                       */
	0x75, 0x01, 		/* REPORT_SIZE (1)                        */
	0x05, 0x08, 		/* USAGE_PAGE (LEDs)                      */
	0x19, 0x01, 		/* USAGE_MINIMUM (Num Lock)               */
	0x29, 0x05, 		/* USAGE_MAXIMUM (Kana)                   */
	0x91, 0x02, 		/* OUTPUT (Data,Var,Abs)                  */
	0x95, 0x01, 		/* REPORT_COUNT (1)                       */
	0x75, 0x03, 		/* REPORT_SIZE (3)                        */
	0x91, 0x03, 		/* OUTPUT (Cnst,Var,Abs)                  */
	0x95, HID_KB_REPORT_SIZE, 		/* REPORT_COUNT               */
	0x75, 0x08, 		/* REPORT_SIZE (8)                        */
	0x15, 0x00, 		/* LOGICAL_MINIMUM (0)                    */
	0x25, 0x65, 		/* LOGICAL_MAXIMUM (101)                  */
	0x05, 0x07, 		/* USAGE_PAGE (Keyboard)                  */
	0x19, 0x00, 		/* USAGE_MINIMUM (Reserved)               */
	0x29, 0x65,			/* USAGE_MAXIMUM (Keyboard Application)   */
	0x81, 0x00, 		/* INPUT (Data,Ary,Abs)                   */
	0xC0,				/* END_COLLECTION                         */
	0x06, 0x00, 0xff,	/* USAGE_PAGE (Vendor Defined Page 1)     */
	0x09, 0x01,			/* USAGE (Vendor Usage 1)                 */
	0xa1, 0x01,			/* COLLECTION (Application)               */
	0x85, 0x02,			/* REPORT_ID (2)                          */
	0x75, 0x08,			/* REPORT_SIZE (8)                        */
	0x95, 0x3F,			/* REPORT_COUNT (63)                      */
	0x15, 0x00,			/* LOGICAL_MINIMUM (0)                    */
	0x26, 0xff, 0x00,	/* LOGICAL_MAXIMUM (255)                  */
	0x09, 0x01,			/* USAGE (Vendor Usage 1)                 */
	0xb1, 0x02,			/* FEATURE (Data,Var,Abs)                 */
	0x85, 0x03,			/* REPORT_ID (3)                          */
	0x75, 0x08,			/* REPORT_SIZE (8)                        */
	0x95, 0x0B,			/* REPORT_COUNT (11)                      */
	0x15, 0x00,			/* LOGICAL_MINIMUM (0)                    */
	0x26, 0xff, 0x00,	/* LOGICAL_MAXIMUM (255)                  */
	0x09, 0x01,			/* USAGE (Vendor Usage 1)                 */
	0xb1, 0x02,			/* FEATURE (Data,Var,Abs)                 */
	0xC0				/* END_COLLECTION                         */
};

#if defined(CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN) 
void usbd_hid_vc_task(void *pvParameters);
#endif


#ifdef __cplusplus
}
#endif


#endif
