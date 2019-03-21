/*
*       File: usbd_uvc_webcam.h
*       Version: 1.0
*       Author: chkuo
*       Date: 2015-12-24
*       Descriptions: SONiX USB Webcam Device header file
*/
#ifndef __USBD_UVC_WEBCAM_H__
#define __USBD_UVC_WEBCAM_H__

#include "composite.h"
#include "usbd_uvc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================*/
/*                              user define                             */
/*==========================================*/
#define UVC_VENDOR_ID                   0x0C45  /* Sonix Technology Co. */
#define UVC_PRODUCT_ID                  0x8200  /* SN98660 UVC */
#define UVC_DEVICE_BCD                  0x0100  /* 1.00 */

typedef struct _GUID {
	uint32_t        Data1;
	uint16_t        Data2;
	uint16_t        Data3;
	uint8_t         Data4[8];
} GUID;


static const GUID SNX_SYS_XU_GUID       = { 0x8DA31E37, 0xC7C1, 0x4AF2, { 0xB2, 0xA5, 0xE4, 0xAA, 0xB1, 0x86, 0x74, 0xF0 } };
static const GUID SNX_USR_XU_GUID       = { 0x8DA31E37, 0xC7C1, 0x4AF2, { 0xB2, 0xA5, 0xE4, 0xAA, 0xB1, 0x86, 0x75, 0xF0 } };
static const GUID H264_GUID             = { 0x34363248, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

#define UVC_STRING_ASSOCIATION_IDX      0
#define UVC_STRING_CONTROL_IDX          1
#define UVC_STRING_STREAMING_IDX        2

struct desc_uvc_format {
	uint8_t *frame_cnt;
	struct uvc_descriptor_header *(*format_func)(uint8_t format_idx, uint8_t num_frame);
	struct uvc_descriptor_header *(*frame_func)(uint8_t frame_idx, uint16_t w, uint16_t h, uint32_t img_size, \
	        uint8_t fps_num, uint8_t *fps, uint32_t *payload_size);
};

/* ------------------------------------------------------------------------
 * SONiX UVC(Webcam) descriptor
 */
static char webcam_vendor_label[]       = "Sonix Technology Co., Ltd.";
static char webcam_product_label[]      = "SN986xx Camera";
static char webcam_config_label[]       = "USB Video";
static char webcam_serial_num_label[]	= "100000000000";

static struct usb_string webcam_strings[5] ;
static struct usb_gadget_strings webcam_device_strings;
static struct usb_string uvc_en_us_strings[4];

static struct usb_gadget_strings webcam_function_strings = {
	0x0409,                         // language
	uvc_en_us_strings               // strings
};

static struct usb_qualifier_descriptor	webcam_qualifier_desc	=	{
	sizeof(struct usb_qualifier_descriptor),	// bLength
	USB_DT_DEVICE_QUALIFIER,        		// bDescriptorType
	0x0200,                         		// bcdUSB
	USB_CLASS_MISC,                 		// bDeviceClass
	0x02,                 				// bDeviceSubClass
	0x01,                           		// bDeviceProtocol
	0,                              		// bMaxPacketSize0 		/* dynamic */
	0,                              		// bNumConfigurations 	/* dynamic */
	0  						// bRESERVED;
};


static struct usb_device_descriptor webcam_device_desc = {
	sizeof(webcam_device_desc),     // bLength
	USB_DT_DEVICE,                  // bDescriptorType
	0x0200,                         // bcdUSB
	USB_CLASS_MISC,                 // bDeviceClass
	0x02,                           // bDeviceSubClass
	0x01,                           // bDeviceProtocol
	0,                              // bMaxPacketSize0 /* dynamic */
	UVC_VENDOR_ID,                  // idVendor
	UVC_PRODUCT_ID,                 // idProduct
	UVC_DEVICE_BCD,                 // bcdDevice
	0,                              // iManufacturer /* dynamic */
	0,                              // iProduct  /* dynamic */
	0,                              // iSerialNumber /* dynamic */
	0                               // bNumConfigurations /* dynamic */
};

static struct usb_interface_assoc_descriptor uvc_iad  = {
	sizeof(uvc_iad),                // bLength
	USB_DT_INTERFACE_ASSOCIATION,   // bDescriptorType
	0,                              // bFirstInterface
	2,                              // bInterfaceCount
	USB_CLASS_VIDEO,                // bFunctionClass
	0x03,                           // bFunctionSubClass
	0x00,                           // bFunctionProtocol
	0                               // iFunction
};

static const struct uvc_header_descriptor uvc_control_header = {
	UVC_DT_HEADER_SIZE(1),          // bLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VC_HEADER,                  // bDescriptorSubType
	(0x0100),                       // bcdUVC
	0,                              // wTotalLength /* dynamic */
	(15000000),                     // dwClockFrequency
	0,                              // bInCollection/* dynamic */
	0                               // baInterfaceNr[0] /* dynamic */
};

static const struct uvc_camera_terminal_descriptor uvc_camera_terminal = {
	UVC_DT_CAMERA_TERMINAL_SIZE(3), // bLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VC_INPUT_TERMINAL,          // bDescriptorSubType
	UVC_SNX_CT_ID,                  // bTerminalID
	UVC_ITT_CAMERA,                 // wTerminalType
	0,                              // bAssocTerminal
	0,                              // iTerminal
	0,                              // wObjectiveFocalLengthMin
	0,                              // wObjectiveFocalLengthMax
	0,                              // wOcularFocalLength
	3,                              // bControlSize
	0x00,                           // bmControls[0]
	0,                              // bmControls[1]
	0                               // bmControls[2]
};

static const struct uvc_processing_unit_descriptor uvc_process_unit = {
	UVC_DT_PROCESSING_UNIT_SIZE(2), // bLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VC_PROCESSING_UNIT,         // bDescriptorSubType
	UVC_SNX_PU_ID,                  // bUnitID
	UVC_SNX_CT_ID,                  // bSourceID
	0,                              // wMaxMultiplier
	2,                              // bControlSize
	0x00,                           // bmControls[0]
	0x00,                           // bmControls[1]
	0                               // iProcessing
};

static const struct uvc_output_terminal_descriptor uvc_output_terminal = {
	UVC_DT_OUTPUT_TERMINAL_SIZE,    // bLengthbLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VC_OUTPUT_TERMINAL,         // bDescriptorSubType
	UVC_SNX_OT_ID,                  // bTerminalID
	UVC_TT_STREAMING,               // wTerminalType
	0,                              // bAssocTerminal
	UVC_SNX_SYS_XU_ID,              // bSourceID
	0                               // iTerminal
};

DECLARE_UVC_EXTENSION_UNIT_DESCRIPTOR(1, 3);
static struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 3) uvc_sys_extension_unit = {
	UVC_DT_EXTENSION_UNIT_SIZE(1, 3), // bLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VC_EXTENSION_UNIT,          // bDescriptorSubType
	UVC_SNX_SYS_XU_ID,              // bUnitID
	{	0x37, 0x1E, 0xA3, 0x8D, 0xC1, 0xC7, 0xF2, 0x4A, // guidExtensionCode
		0xB2, 0xA5, 0xE4, 0xAA, 0xB1, 0x86, 0x74, 0xF0
	},
	3,                             	// bNumControls
	1,                              // bNrInPins
	UVC_SNX_PU_ID,                  // baSourceID[0]
	3,                              // bControlSize
	0x00,                           // bmControls[0]
	0x00,                           // bmControls[1]
	0x00,                           // bmControls[2]
	0                               // iExtension
};

DECLARE_UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4);
static struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_usr_extension_unit = {
	UVC_DT_EXTENSION_UNIT_SIZE(1, 4), // bLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VC_EXTENSION_UNIT,          // bDescriptorSubType
	UVC_SNX_SYS_XU_ID,              // bUnitID
	{	0x37, 0x1E, 0xA3, 0x8D, 0xC1, 0xC7, 0xF2, 0x4A, // guidExtensionCode
		0xB2, 0xA5, 0xE4, 0xAA, 0xB1, 0x86, 0x75, 0xF0
	},
	32,                             // bNumControls
	1,                              // bNrInPins
	UVC_SNX_SYS_XU_ID,              // baSourceID[0]
	4,                              // bControlSize
	0x00,                           // bmControls[0]
	0x00,                           // bmControls[1]
	0x00,                           // bmControls[2]
	0x00,                           // bmControls[3]
	0,                              // iExtension
};

static struct usb_interface_descriptor uvc_control_intf  = {
	USB_DT_INTERFACE_SIZE,          // bLength
	USB_DT_INTERFACE,               // bDescriptorType
	UVC_INTF_VIDEO_CONTROL,         // bInterfaceNumber
	0,                              // bAlternateSetting
	1,                              // bNumEndpoints
	USB_CLASS_VIDEO,                // bInterfaceClass
	UVC_SC_VIDEOCONTROL,            // bInterfaceSubClass
	0x00,                           // bInterfaceProtocol
	0                               // iInterface
};

static struct usb_endpoint_descriptor uvc_control_ep  = {
	USB_DT_ENDPOINT_SIZE,           // bLength
	USB_DT_ENDPOINT,                // bDescriptorType
	USB_DIR_IN,                     // bEndpointAddress
	USB_ENDPOINT_XFER_INT,          // bmAttributes
	16,                             // wMaxPacketSize
	6                               // bInterval
};

static struct uvc_control_endpoint_descriptor uvc_control_cs_ep  = {
	UVC_DT_CONTROL_ENDPOINT_SIZE,   // bLength
	USB_DT_CS_ENDPOINT,             // bDescriptorType
	UVC_EP_INTERRUPT,               // bDescriptorSubType
	128                             // wMaxTransferSize
};

static struct usb_interface_descriptor uvc_streaming_intf_alt0  = {
	USB_DT_INTERFACE_SIZE,          // bLength
	USB_DT_INTERFACE,               // bDescriptorType
	UVC_INTF_VIDEO_STREAMING,       // bInterfaceNumber
	0,                              // bAlternateSetting
	1,                              // bNumEndpoints
	USB_CLASS_VIDEO,                // bInterfaceClass
	UVC_SC_VIDEOSTREAMING,          // bInterfaceSubClass
	0x00,                           // bInterfaceProtocol
	0                               // iInterface
};

static struct usb_endpoint_descriptor uvc_streaming_ep = {
	USB_DT_ENDPOINT_SIZE,           // bLength
	USB_DT_ENDPOINT,                // bDescriptorType
	USB_DIR_IN,                     // bEndpointAddress
	USB_ENDPOINT_XFER_ISOC,         // bmAttributes
	//USB_ENDPOINT_XFER_BULK,         // bmAttributes
	512,                            // wMaxPacketSize
	1                               // bInterval
};

static struct usb_endpoint_descriptor uvc_fs_streaming_ep = {
	USB_DT_ENDPOINT_SIZE,           // bLength
	USB_DT_ENDPOINT,                // bDescriptorType
	USB_DIR_IN,                     // bEndpointAddress
	USB_ENDPOINT_XFER_ISOC,         // bmAttributes
	64,                            // wMaxPacketSize
	1                               // bInterval
};

static const struct uvc_color_matching_descriptor uvc_color_matching = {
	UVC_DT_COLOR_MATCHING_SIZE,     // bLength
	USB_DT_CS_INTERFACE,            // bDescriptorType
	UVC_VS_COLORFORMAT,             // bDescriptorSubType
	1,                              // bColorPrimaries
	1,                              // bTransferCharacteristics
	4                               // bMatrixCoefficients
};

static struct usb_descriptor_header *  uvc_fs_streaming[] = {
	(struct usb_descriptor_header *) &uvc_streaming_intf_alt0,
	(struct usb_descriptor_header *) &uvc_fs_streaming_ep,
	NULL,
};

//static const struct usb_descriptor_header * const uvc_hs_streaming[] = {
static struct usb_descriptor_header * uvc_hs_streaming[] = {
	(struct usb_descriptor_header *) &uvc_streaming_intf_alt0,
	(struct usb_descriptor_header *) &uvc_streaming_ep,
	NULL,
};

// Variable

// Functions


#ifdef __cplusplus
}
#endif

#endif /*__USBD_UVC_WEBCAM_H__*/
