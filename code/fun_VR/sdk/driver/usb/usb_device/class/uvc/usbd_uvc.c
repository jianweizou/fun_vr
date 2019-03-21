/**
* @file
* this is usb device driver file
* usbd_uvc.c
* @author IP2/Luka
*/
#include "sonix_config.h"
#if defined( CONFIG_MODULE_USBD_UVC_CLASS)

#include <stdio.h>
#include "usb_device.h"
#include "usbd_uvc.h"
#include "composite.h"
#include "snx_udc.h"
#include <string.h>
#include <task.h>
#include "usbd_uvc_webcam.h"
#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif


//===================================================================


#if defined(CONFIG_PLATFORM_SN98660)
static uvc_desc_parsing_mode_t uvc_desc_parsing_mode    = UVC_DESC_PARSING_SENSOR_KEY_RES;
static int uvc_ext_assign_width, uvc_ext_assign_height, uvc_ext_assign_fps;
static uint8_t raw_support                              = 0;
#endif

#ifdef CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN
void uvc_vc_set_image_info(struct preview_image_info *img_info);
void usbd_uvc_vc_task_stop(void);
int uvc_vc_init(void);
#endif

extern usbd_class_submode_t                             snx_usbd_submode;

SemaphoreHandle_t usbd_uvc_sema;
SemaphoreHandle_t usbd_uvc_stop;
SemaphoreHandle_t usbd_uvc_slice_finish;
xSemaphoreHandle usbd_uvc_image_slice;

static uint8_t uvc_probe_setcur 						= false;
static uint8_t uvc_commit_setcur 						= false;

static struct uvc_device *uvc                           = NULL;
static uint32_t uvc_task_run                            = 0;

uint8_t raw_cnt, mjpg_cnt, h264_cnt;
//static usbd_drv_uvc_xu_cmd_cb_t uvc_xu_cmd_cb           = NULL;
//set volatile to fix warning about "variable was set but never used"
static volatile usbd_drv_uvc_xu_cmd_cb_t uvc_xu_cmd_cb  = NULL;
static volatile usbd_drv_uvc_start_preview_cb_t start_preview_callback  = NULL;
static volatile usbd_drv_uvc_stop_preview_cb_t stop_preview_callback    = NULL;

//===================================================================
#define SNX_DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(n, p)   DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(n, p)
#define SNX_UVC_INPUT_HEADER_DESCRIPTOR(n, p)           UVC_INPUT_HEADER_DESCRIPTOR(n, p)

#define SNX_DECLARE_UVC_FRAME_UNCOMPRESSED(n)           DECLARE_UVC_FRAME_UNCOMPRESSED(n)
#define SNX_UVC_FRAME_UNCOMPRESSED(n)                   UVC_FRAME_UNCOMPRESSED(n)

#define SNX_DECLARE_UVC_FRAME_MJPEG(n)                  DECLARE_UVC_FRAME_MJPEG(n)
#define SNX_UVC_FRAME_MJPEG(n)                          UVC_FRAME_MJPEG(n)

#define SNX_DECLARE_UVC_FRAME_FRAME_BASED(n)            DECLARE_UVC_FRAME_FRAME_BASED(n)
#define SNX_UVC_FRAME_FRAME_BASED(n)                    UVC_FRAME_FRAME_BASED(n)

#define STREAMING_CLS_COUNT                             (3 + (UVC_MAX_FMT_NUM * UVC_MAX_FRAME_NUM))
//===================================================================
static struct uvc_descriptor_header *uvc_hs_streaming_cls[STREAMING_CLS_COUNT];

static const struct uvc_descriptor_header * const uvc_control_cls[] = {
	(const struct uvc_descriptor_header *) &uvc_control_header,
	(const struct uvc_descriptor_header *) &uvc_camera_terminal,
	(const struct uvc_descriptor_header *) &uvc_process_unit,
	(const struct uvc_descriptor_header *) &uvc_sys_extension_unit,
	(const struct uvc_descriptor_header *) &uvc_output_terminal,
	NULL,
};

struct uvc_video
{
	struct usb_ep                   *ep;
	struct usb_request              *req[1];
	struct preview_image_info       img;
	uint8_t*						payload_buf;
		
	uint32_t                        SCR;
	uint32_t                        PTS;
	uint16_t                        SOF1Khz;
	uint8_t                         header[UVC_PAYLOAD_HEADER_LENGTH];
	uint8_t                         *imgbuf;
	uint32_t                        imggap;
};

struct uvc_device
{
	struct usb_function             func;
	uvc_state_t                     state;
	uvc_event_t                     event;
	struct {
		const struct uvc_descriptor_header * const *control;
		struct uvc_descriptor_header *  *fs_streaming;
		struct uvc_descriptor_header *  *hs_streaming;
	} desc;

	struct usb_ep                   *control_ep;
	struct usb_request              *control_req;
	void                            *control_buf;
	struct uvc_video                video;
	unsigned int                    control_intf;
	unsigned int                    streaming_intf;


	struct uvc_streaming_control    probe;
	struct uvc_streaming_control    commit;
	uint8_t                         xu[64];
	uint8_t                         *img_buf;
	uint8_t                         alt_set;
	uint8_t                         commit_done;
	uint32_t                        img_len;
};

typedef struct uvc_sensor_key_resolution
{
	int                             org_width;
	int                             org_height;
	int                             fps;
	int                             x;
	int                             y;
	int                             crop_width;
	int                             crop_height;
} uvc_sensor_key_resolution_t;

#define SNX_MJPG_HEADER_LEN         589
struct image_info default_mjpg_qvga = {
	UVC_VS_FORMAT_MJPEG,                // type
	320,                                // width
	240,                                // height
	(320 * 240) + SNX_MJPG_HEADER_LEN,  // imagesize
	1,                                  // fps_num
	{ 30 },                             // fps
	{ 0 }                               // payload_size
};

#define FHD_WIDTH                   1920
#define FHD_HEIGHT                  1080
#define HD_WIDTH                    1280
#define HD_HEIGHT                   720
#define VGA_WIDTH                   640
#define VGA_HEIGHT                  480
struct image_info uvc_mjpg_fhd = {
	UVC_VS_FORMAT_MJPEG,                    // type
	FHD_WIDTH,                              // width
	FHD_HEIGHT,                             // height
	(FHD_WIDTH * FHD_HEIGHT) + SNX_MJPG_HEADER_LEN, // imagesize
	5,                                      // fps_num
	{ 30, 20, 15, 10, 5 },                  // fps
};

struct image_info uvc_mjpg_hd = {
	UVC_VS_FORMAT_MJPEG,                    // type
	HD_WIDTH,                               // width
	HD_HEIGHT,                              // height
	(HD_WIDTH * HD_HEIGHT) + SNX_MJPG_HEADER_LEN, // imagesize
	5,                                      // fps_num
	{  30, 20, 15, 10, 5 },             	 // fps
};

struct image_info uvc_mjpg_vga = {
	UVC_VS_FORMAT_MJPEG,                    // type
	VGA_WIDTH,                              // width
	VGA_HEIGHT,                             // height
	(VGA_WIDTH * VGA_HEIGHT) + SNX_MJPG_HEADER_LEN, // imagesize
	5,                                      // fps_num
	{  30, 20, 15, 10, 5 },             	 // fps
};

struct image_info uvc_h264_fhd = {
	UVC_VS_FORMAT_FRAME_BASED,              // type
	FHD_WIDTH,                              // width
	FHD_HEIGHT,                             // height
	FHD_WIDTH * FHD_HEIGHT,                 // imagesize
	5,                                      // fps_num
	{ 30, 20, 15, 10, 5 },                  // fps
};

struct image_info uvc_h264_hd = {
	UVC_VS_FORMAT_FRAME_BASED,              // type
	HD_WIDTH,                               // width
	HD_HEIGHT,                              // height
	HD_WIDTH * HD_HEIGHT,                   // imagesize
	5,                                      // fps_num
	{  30, 20, 15, 10, 5 },                 // fps
};

struct image_info uvc_h264_vga = {
	UVC_VS_FORMAT_FRAME_BASED,              // type
	VGA_WIDTH,                              // width
	VGA_HEIGHT,                             // height
	VGA_WIDTH * VGA_HEIGHT,                 // imagesize
	5,                                      // fps_num
	{  30, 20, 15, 10, 5 },                 // fps
};

struct image_info uvc_yuv_fhd = {
	UVC_VS_FORMAT_UNCOMPRESSED,             // type
	FHD_WIDTH,                              // width
	FHD_HEIGHT,                             // height
	FHD_WIDTH * FHD_HEIGHT * 2,             // imagesize
	3,                                      // fps_num
	{ 20, 15, 10, 5 },                      // fps
};

struct image_info uvc_yuv_hd = {
	UVC_VS_FORMAT_UNCOMPRESSED,             // type
	HD_WIDTH,                               // width
	HD_HEIGHT,                              // height
	HD_WIDTH * HD_HEIGHT * 2,               // imagesize
	3,                                      // fps_num
	{ 20, 15, 10, 5 },                      // fps
};

struct image_info uvc_yuv_vga = {
	UVC_VS_FORMAT_UNCOMPRESSED,             // type
	VGA_WIDTH,                              // width
	VGA_HEIGHT,                             // height
	VGA_WIDTH * VGA_HEIGHT * 2,             // imagesize
	3,                                      // fps_num
	{ 20, 15, 10, 5 },                      // fps
};

struct image_info *uvc_drv_img_info_list[] = {
	// MJPG list
	&uvc_mjpg_fhd,
	&uvc_mjpg_hd,
	&uvc_mjpg_vga,

	// H264 list
	&uvc_h264_fhd,
	&uvc_h264_hd,
	&uvc_h264_vga,

	// YUV list
	&uvc_yuv_fhd,
	&uvc_yuv_hd,
	&uvc_yuv_vga,

	NULL
};

static struct image_info **img_info_list = uvc_drv_img_info_list;
static struct image_info cur_info[UVC_MAX_FRAME_NUM] = {0};


struct image_info *img_info_raw[UVC_MAX_FRAME_NUM];
struct image_info *img_info_mjpg[UVC_MAX_FRAME_NUM];
struct image_info *img_info_h264[UVC_MAX_FRAME_NUM];
struct image_info *img_info_yuv[UVC_MAX_FRAME_NUM];
struct image_info **img_info_desc[UVC_MAX_FMT_NUM];
struct desc_uvc_format desc_ops[UVC_MAX_FMT_NUM];

//===================================================================
static struct uvc_format_mjpeg uvc_format_mjpg = {0};
static struct uvc_descriptor_header *des_uvc_format_mjpg(uint8_t format_idx, uint8_t num_frame)
{
	uvc_format_mjpg.bLength                 = UVC_DT_FORMAT_MJPEG_SIZE;
	uvc_format_mjpg.bDescriptorType         = USB_DT_CS_INTERFACE;
	uvc_format_mjpg.bDescriptorSubType      = UVC_VS_FORMAT_MJPEG;
	uvc_format_mjpg.bFormatIndex            = format_idx+1;
	uvc_format_mjpg.bNumFrameDescriptors    = num_frame;
	uvc_format_mjpg.bmFlags                 = 0;
	uvc_format_mjpg.bDefaultFrameIndex      = 1;
	uvc_format_mjpg.bAspectRatioX           = 0;
	uvc_format_mjpg.bAspectRatioY           = 0;
	uvc_format_mjpg.bmInterlaceFlags        = 0;
	uvc_format_mjpg.bCopyProtect            = 0;

	return (struct uvc_descriptor_header *)&uvc_format_mjpg;
}

static struct uvc_format_frame_based uvc_format_h264 = {0};
static struct uvc_descriptor_header *des_uvc_format_h264(uint8_t format_idx, uint8_t num_frame)
{
	uint8_t guidFormat[16]                  =
	{	'H',  '2',  '6',  '4', 0x00, 0x00, 0x10, 0x00,
		0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
	};
	uvc_format_h264.bLength                 = UVC_DT_FORMAT_FRAME_BASED_SIZE;
	uvc_format_h264.bDescriptorType         = USB_DT_CS_INTERFACE;
	uvc_format_h264.bDescriptorSubType      = UVC_VS_FORMAT_FRAME_BASED;
	uvc_format_h264.bFormatIndex            = format_idx+1;
	uvc_format_h264.bNumFrameDescriptors    = num_frame;
	memcpy(uvc_format_h264.guidFormat, guidFormat, 16);
	uvc_format_h264.bBitsPerPixel           = 16;
	uvc_format_h264.bDefaultFrameIndex      = 1;
	uvc_format_h264.bAspectRatioX           = 0;
	uvc_format_h264.bAspectRatioY           = 0;
	uvc_format_h264.bmInterlaceFlags        = 0;
	uvc_format_h264.bCopyProtect            = 0;
	uvc_format_h264.bVariableSize           = 1;

	return (struct uvc_descriptor_header *)&uvc_format_h264;
}

static struct uvc_format_uncompressed uvc_format_raw = {0};
static struct uvc_descriptor_header *des_uvc_format_raw(uint8_t format_idx, uint8_t num_frame)
{
	uint8_t guidFormat[16]                  =
	{	'Y',  'U',  'Y',  '2', 0x00, 0x00, 0x10, 0x00,
		0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
	};
	uvc_format_raw.bLength                  = UVC_DT_FORMAT_UNCOMPRESSED_SIZE;
	uvc_format_raw.bDescriptorType          = USB_DT_CS_INTERFACE;
	uvc_format_raw.bDescriptorSubType       = UVC_VS_FORMAT_UNCOMPRESSED;
	uvc_format_raw.bFormatIndex             = format_idx+1;
	uvc_format_raw.bNumFrameDescriptors     = num_frame;
	memcpy(uvc_format_raw.guidFormat, guidFormat, 16);
	uvc_format_raw.bBitsPerPixel            = 16;
	uvc_format_raw.bDefaultFrameIndex       = 1;
	uvc_format_raw.bAspectRatioX            = 0;
	uvc_format_raw.bAspectRatioY            = 0;
	uvc_format_raw.bmInterlaceFlags         = 0;
	uvc_format_raw.bCopyProtect             = 0;

	return (struct uvc_descriptor_header *)&uvc_format_raw;
}

SNX_DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(1, UVC_MAX_FMT_NUM);
static struct SNX_UVC_INPUT_HEADER_DESCRIPTOR(1, UVC_MAX_FMT_NUM) uvc_input_header;
static struct uvc_descriptor_header *des_uvc_input_header(uint8_t num_fmt)
{
	uint8_t i;

	uvc_input_header.bLength               = UVC_DT_INPUT_HEADER_SIZE(1, num_fmt);
	uvc_input_header.bDescriptorType       = USB_DT_CS_INTERFACE;
	uvc_input_header.bDescriptorSubType    = UVC_VS_INPUT_HEADER;
	uvc_input_header.bNumFormats           = num_fmt;
	uvc_input_header.wTotalLength          = 0; /* dynamic */
	uvc_input_header.bEndpointAddress      = 0; /* dynamic */
	uvc_input_header.bmInfo                = 0;
	uvc_input_header.bTerminalLink         = UVC_SNX_OT_ID;
	uvc_input_header.bStillCaptureMethod   = 0;
	uvc_input_header.bTriggerSupport       = 0;
	uvc_input_header.bTriggerUsage         = 0;
	uvc_input_header.bControlSize          = 1;

	for (i=0; i<num_fmt; i++) {
		uvc_input_header.bmaControls[i][0] = 0;
	}

	return (struct uvc_descriptor_header *)&uvc_input_header;
}

SNX_DECLARE_UVC_FRAME_MJPEG(UVC_MAX_FPS_NUM);
static struct SNX_UVC_FRAME_MJPEG(UVC_MAX_FPS_NUM) uvc_frame_mjpg[UVC_MAX_FRAME_NUM];
static struct uvc_descriptor_header *des_uvc_frame_mjpg(uint8_t frame_idx, uint16_t w, uint16_t h, uint32_t img_size, \
        uint8_t fps_num, uint8_t *fps, uint32_t *payload_size)
{
	uint8_t fps_max, fps_min;
	uint8_t fps_def = 0;
	int i;

	fps_max                                             = fps[0];
	fps_min                                             = fps[fps_num-1];
	uvc_frame_mjpg[frame_idx].bLength                   = UVC_DT_FRAME_MJPEG_SIZE(fps_num);
	uvc_frame_mjpg[frame_idx].bDescriptorType           = USB_DT_CS_INTERFACE;
	uvc_frame_mjpg[frame_idx].bDescriptorSubType        = UVC_VS_FRAME_MJPEG;
	uvc_frame_mjpg[frame_idx].bFrameIndex               = frame_idx+1;
	uvc_frame_mjpg[frame_idx].bmCapabilities            = 0;
	uvc_frame_mjpg[frame_idx].wWidth                    = w;
	uvc_frame_mjpg[frame_idx].wHeight                   = h;
	uvc_frame_mjpg[frame_idx].dwMinBitRate              = img_size*8*fps_min;
	uvc_frame_mjpg[frame_idx].dwMaxBitRate              = img_size*8*fps_max;
	uvc_frame_mjpg[frame_idx].dwMaxVideoFrameBufferSize = img_size;
	uvc_frame_mjpg[frame_idx].bFrameIntervalType        = fps_num;

	for(i = 0; i < fps_num; i++) {
		if ((fps[i] <= UVC_DEF_FPS_MAX) && (fps_def < fps[i])) {
			fps_def = fps[i];
		}
		uvc_frame_mjpg[frame_idx].dwFrameInterval[i]    = 10000000/fps[i];
		payload_size[i]                             = img_size;
	}

	if (fps_def == 0) {
		fps_def = fps[0];
	}
	uvc_frame_mjpg[frame_idx].dwDefaultFrameInterval    = 10000000/fps_def;

	return (struct uvc_descriptor_header *)&uvc_frame_mjpg[frame_idx];
}

SNX_DECLARE_UVC_FRAME_FRAME_BASED(UVC_MAX_FPS_NUM);
static struct SNX_UVC_FRAME_FRAME_BASED(UVC_MAX_FPS_NUM) uvc_frame_h264[UVC_MAX_FRAME_NUM];
static struct uvc_descriptor_header *des_uvc_frame_h264(uint8_t frame_idx, uint16_t w, uint16_t h, uint32_t img_size, \
        uint8_t fps_num, uint8_t *fps, uint32_t *payload_size)
{
	uint8_t fps_max, fps_min;
	uint8_t fps_def = 0;
	int i;

	fps_max                                             = fps[0];
	fps_min                                             = fps[fps_num-1];
	uvc_frame_h264[frame_idx].bLength                   = UVC_DT_FRAME_FRAME_BASED_SIZE(fps_num);
	uvc_frame_h264[frame_idx].bDescriptorType           = USB_DT_CS_INTERFACE;
	uvc_frame_h264[frame_idx].bDescriptorSubType        = UVC_VS_FRAME_FRAME_BASED;
	uvc_frame_h264[frame_idx].bFrameIndex               = frame_idx+1;
	uvc_frame_h264[frame_idx].bmCapabilities            = 0;
	uvc_frame_h264[frame_idx].wWidth                    = w;
	uvc_frame_h264[frame_idx].wHeight                   = h;
	uvc_frame_h264[frame_idx].dwMinBitRate              = img_size*8*fps_min;
	uvc_frame_h264[frame_idx].dwMaxBitRate              = img_size*8*fps_max;
	uvc_frame_h264[frame_idx].dwBytesPerLine            = 0;
	uvc_frame_h264[frame_idx].bFrameIntervalType        = fps_num;

	for(i = 0; i < fps_num; i++) {
		if ((fps[i] <= UVC_DEF_FPS_MAX) && (fps_def < fps[i])) {
			fps_def = fps[i];
		}
		uvc_frame_h264[frame_idx].dwFrameInterval[i]    = 10000000/fps[i];
		payload_size[i]                                 = img_size;
	}

	if (fps_def == 0) {
		fps_def = fps[0];
	}

	uvc_frame_h264[frame_idx].dwDefaultFrameInterval    = 10000000/fps_def;

	return (struct uvc_descriptor_header *)&uvc_frame_h264[frame_idx];
}

SNX_DECLARE_UVC_FRAME_UNCOMPRESSED(UVC_MAX_FPS_NUM);
static struct SNX_UVC_FRAME_UNCOMPRESSED(UVC_MAX_FPS_NUM) uvc_frame_raw[UVC_MAX_FRAME_NUM];
static struct uvc_descriptor_header *des_uvc_frame_raw(uint8_t frame_idx, uint16_t w, uint16_t h, uint32_t img_size, \
        uint8_t fps_num, uint8_t *fps, uint32_t *payload_size)
{
	uint8_t fps_max, fps_min;
	uint8_t fps_def = 0;
	int i;

	fps_max                                             = fps[0];
	fps_min                                             = fps[fps_num-1];
	uvc_frame_raw[frame_idx].bLength                    = UVC_DT_FRAME_UNCOMPRESSED_SIZE(fps_num);
	uvc_frame_raw[frame_idx].bDescriptorType            = USB_DT_CS_INTERFACE;
	uvc_frame_raw[frame_idx].bDescriptorSubType         = UVC_VS_FRAME_UNCOMPRESSED;
	uvc_frame_raw[frame_idx].bFrameIndex                = frame_idx+1;
	uvc_frame_raw[frame_idx].bmCapabilities             = 0;
	uvc_frame_raw[frame_idx].wWidth                     = w;
	uvc_frame_raw[frame_idx].wHeight                    = h;
	uvc_frame_raw[frame_idx].dwMinBitRate               = img_size*8*fps_min;
	uvc_frame_raw[frame_idx].dwMaxBitRate               = img_size*8*fps_max;
	uvc_frame_raw[frame_idx].dwMaxVideoFrameBufferSize  = img_size;
	uvc_frame_raw[frame_idx].bFrameIntervalType         = fps_num;

	for(i = 0; i < fps_num; i++) {
		if ((fps[i] <= UVC_DEF_FPS_MAX) && (fps_def < fps[i])) {
			fps_def = fps[i];
		}
		uvc_frame_raw[frame_idx].dwFrameInterval[i]     = 10000000/fps[i];
		payload_size[i]                                 = img_size + UVC_PAYLOAD_HEADER_LENGTH;
	}

	if (fps_def == 0) {
		fps_def = fps[0];
	}
	uvc_frame_raw[frame_idx].dwDefaultFrameInterval     = 10000000/fps_def;

	return (struct uvc_descriptor_header *)&uvc_frame_raw[frame_idx];
}

//===================================================================
static void uvc_function_unbind(struct usb_configuration *c, struct usb_function *f);
static int uvc_bind(struct usb_composite_dev *cdev);
static int uvc_unbind(struct usb_composite_dev *cdev);

static struct usb_composite_driver uvc_driver = {
	"uvc driver",           // name
	&webcam_device_desc,    // dev
	&webcam_qualifier_desc, // qualifier
	&webcam_device_strings, // strings
	uvc_bind,               // bind
	uvc_unbind,             // unbind
};

static void uvc_ct_init(void)
{

}

static void uvc_pu_init(void)
{

}
#if defined(CONFIG_PLATFORM_SN98660)
void usbd_drv_uvc_raw_enable(uint8_t raw_en)
{
	raw_support = raw_en;
}

void usbd_drv_uvc_set_desc_parsing_mode(int mode, int width, int height, int fps)
{
	uvc_desc_parsing_mode = (uvc_desc_parsing_mode_t)mode;
	uvc_ext_assign_width = width;
	uvc_ext_assign_height = height;
	uvc_ext_assign_fps = fps;
}

static struct usb_configuration uvc_config_driver;
void usbd_drv_uvc_set_desc_info(struct uvc_device_desc_info *uvc_desc_info)
{
	if (uvc_desc_info == NULL) {
		return;
	}

	if (uvc_desc_info->idVendor != NULL) {
		webcam_device_desc.idVendor = *(uvc_desc_info->idVendor);
	}
	if (uvc_desc_info->idProduct != NULL) {
		webcam_device_desc.idProduct = *(uvc_desc_info->idProduct);
	}
	if (uvc_desc_info->bcdDevice != NULL) {
		webcam_device_desc.bcdDevice = *(uvc_desc_info->bcdDevice);
	}

	if (uvc_desc_info->strVendor != NULL) {
		webcam_strings[STRING_MANUFACTURER_IDX].s = uvc_desc_info->strVendor;
	}
	if (uvc_desc_info->StrProduct != NULL) {
		webcam_strings[STRING_PRODUCT_IDX].s = uvc_desc_info->StrProduct;
		uvc_en_us_strings[UVC_STRING_ASSOCIATION_IDX].s = uvc_desc_info->StrProduct;
	}
	if (uvc_desc_info->StrConfig != NULL) {
		webcam_strings[STRING_DESCRIPTION_IDX].s = uvc_desc_info->StrConfig;
	}

	if (uvc_desc_info->bmAttributes != NULL) {
		uvc_config_driver.bmAttributes = *(uvc_desc_info->bmAttributes);
	}
	if (uvc_desc_info->bMaxPower != NULL) {
		uvc_config_driver.bMaxPower = *(uvc_desc_info->bMaxPower);
	}

	if (uvc_desc_info->UsrXUbmControls != NULL) {
		memcpy(&uvc_usr_extension_unit.bmControls[0], &uvc_desc_info->UsrXUbmControls[0], 4);
	}

	if (uvc_desc_info->UsrXUGUID != NULL) {
		memcpy(&uvc_usr_extension_unit.guidExtensionCode[0], &uvc_desc_info->UsrXUGUID[0], 16);
	}

	if (uvc_desc_info->img_info != NULL) {
		img_info_list = uvc_desc_info->img_info;
	}
}
#endif

/**
* @brief interface function - init uvc function
* @return value of error code.
*/
int usbd_uvc_init(void)
{
	int ret = 0;

	uvc_ct_init();
	uvc_pu_init();

	strcpy(webcam_strings[STRING_MANUFACTURER_IDX].s, webcam_vendor_label);
	strcpy(webcam_strings[STRING_PRODUCT_IDX].s, webcam_product_label);
	strcpy(webcam_strings[STRING_DESCRIPTION_IDX].s, webcam_config_label);

	strcpy(webcam_strings[STRING_SERIAL_NUM_IDX].s,webcam_serial_num_label);
	strcpy(webcam_strings[4].s,"");

	webcam_device_strings.language = 0x0409;
	webcam_device_strings.strings = webcam_strings;

	strcpy(uvc_en_us_strings[UVC_STRING_ASSOCIATION_IDX].s,"SN986xx Camera");
	strcpy(uvc_en_us_strings[UVC_STRING_CONTROL_IDX].s, "Video Control");
	strcpy(uvc_en_us_strings[UVC_STRING_STREAMING_IDX].s,"Video Streaming");
	strcpy(uvc_en_us_strings[3].s,"");

	ret = usb_composite_register(&uvc_driver);

	return ret;
}
/**
* @brief interface function - un-init uvc function
* @return value of error code.
*/
int usbd_uvc_uninit(void)
{
	uvc_device_task_stop();
	if(usbd_uvc_stop)
		xSemaphoreTake(usbd_uvc_stop, 150);

	usb_composite_unregister();

	if(usbd_uvc_stop) {
		vSemaphoreDelete(usbd_uvc_stop);
		usbd_uvc_stop = NULL;
	}
	return 0;
}

void usbd_drv_uvc_xu_cmd_reg_cb(usbd_drv_uvc_xu_cmd_cb_t cb)
{
	uvc_xu_cmd_cb = cb;
}

void usbd_drv_uvc_start_preview_reg_cb(usbd_drv_uvc_start_preview_cb_t cb)
{
	start_preview_callback = cb;
}

void usbd_drv_uvc_stop_preview_reg_cb(usbd_drv_uvc_stop_preview_cb_t cb)
{
	stop_preview_callback = cb;
}

/**
* @brief interface function - endpoint 1 callback function
* @param the pointer of usb function
* @param the pointer of usb endpoint
*/
static void uvc_functione_ep7_callback(struct usb_function *f,struct usb_ep *ep)
{
	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	if((ep->state & USBD_EP_STATE_IN) == USBD_EP_STATE_IN) {
		ep->state &= ~USBD_EP_STATE_IN;
	}
	else if((ep->state & USBD_EP_STATE_TOC) ==  USBD_EP_STATE_TOC) {
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC );
		if(usbd_uvc_slice_finish && (uvc->state == UVC_STATE_PREVIEW)) {
			xSemaphoreGive(usbd_uvc_slice_finish);
		}
	}
}
/**
* @brief interface function - endpoint 1 callback function
* @param the pointer of usb function
* @param the pointer of usb endpoint
*/
static void uvc_functione_ep14_callback(struct usb_function *f,struct usb_ep *ep)
{
	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	if((ep->state & USBD_EP_STATE_IN) == USBD_EP_STATE_IN) {
		ep->state &= ~USBD_EP_STATE_IN;
		if(uvc->state == UVC_STATE_STANDBY && uvc->commit_done) {
			uvc->event = UVC_EVENT_STREAMING;
			if(usbd_uvc_sema)
				xSemaphoreGive(usbd_uvc_sema);
		}

	}
	else if((ep->state & USBD_EP_STATE_TFOC) ==  USBD_EP_STATE_TFOC) {
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC | USBD_EP_STATE_TFOC);
#else
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TFOC);
#endif		
		if(usbd_uvc_slice_finish && (uvc->state == UVC_STATE_PREVIEW)) {
			xSemaphoreGive(usbd_uvc_slice_finish);
			USBD_UVC_PRINT("give usbd_uvc_slice_finish\n");
		}
	}
	else if((ep->state & USBD_EP_STATE_TOC) ==  USBD_EP_STATE_TOC) {
		ep->state &= ~(USBD_EP_STATE_BUSY | USBD_EP_STATE_TOC );
	}
}
/**
* @brief interface function - endpoint 5 callback function
* @param the pointer of usb function
* @param the pointer of usb endpoint
*/
static void uvc_functione_ep5_callback(struct usb_function *f,struct usb_ep *ep)
{

}

#define UVC_COPY_DESCRIPTOR(mem, dst, desc) \
    memcpy(mem, desc, (desc)->bLength); \
    *(dst)++ = (void*)mem; \
    mem += (desc)->bLength;

#define UVC_COPY_DESCRIPTORS(mem, dst, src) \
    for (__src = src; *__src; ++__src) { \
        memcpy(mem, *__src, (*__src)->bLength); \
        *dst++ = (void*)mem; \
        mem += (*__src)->bLength; \
    }


static uint16_t iso_maxpayloadsize[5]= {0x0100,0x0320,0x0B20,0x1320,0x1400};
/**
* @brief interface function - copy descriptors
* @param the type of usb speed
* @return the pointer of descriptor header's pointer.
*/
static struct usb_descriptor_header ** uvc_copy_descriptors(enum usb_device_speed speed)
{
	const struct usb_descriptor_header  * const *__src;
	struct uvc_descriptor_header        **uvc_streaming_cls;
	struct usb_descriptor_header        **uvc_streaming_std;
	unsigned int                        control_size;
	unsigned int                        streaming_size;
	unsigned int                        n_desc;
	unsigned int                        bytes;
	unsigned int                        i;
	struct uvc_input_header_descriptor  *uvc_streaming_header;
	struct uvc_header_descriptor        *uvc_control_header;

	const struct usb_descriptor_header  * const *src;
	struct usb_descriptor_header        **dst;
	struct usb_descriptor_header        **hdr;

	uint8_t                             *mem;

	uvc_streaming_cls = (speed == USB_SPEED_FULL)  ? uvc->desc.fs_streaming : uvc->desc.hs_streaming;
	uvc_streaming_std = (speed == USB_SPEED_FULL)  ? uvc_fs_streaming : uvc_hs_streaming;

	/* Descriptors layout
	 * BULK mode
	 * uvc_iad
	 * uvc_control_intf
	 * Class-specific UVC control descriptors
	 * uvc_control_ep
	 * uvc_control_cs_ep
	 * uvc_streaming_intf_alt0
	 * Class-specific UVC streaming descriptors
	 * uvc_{fs|hs}_streaming
	 */

	/* Count descriptors and compute their size. */
	control_size = 0;
	streaming_size = 0;

	bytes = uvc_iad.bLength + uvc_control_intf.bLength + uvc_control_ep.bLength + uvc_control_cs_ep.bLength /*+ (*uvc_streaming_std)->bLength*/;

	n_desc = 5;

	for (src = (const struct usb_descriptor_header**)uvc->desc.control; *src; ++src) {
		control_size += (*src)->bLength;
		bytes += (*src)->bLength;
		n_desc++;
	}

	for (src = (const struct usb_descriptor_header**)uvc_streaming_cls; *src; ++src) {
		streaming_size += (*src)->bLength;
		bytes += (*src)->bLength;
		n_desc++;
	}

	for (src = (const struct usb_descriptor_header**)uvc_streaming_std; *src; ++src) {
		//for (src = uvc_streaming_std; *src; ++src) {
		bytes += (*src)->bLength;
		n_desc++;
	}

	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
		for (i=0; i<5 ; i++) {
			for (src = (const struct usb_descriptor_header**)uvc_streaming_std; *src; ++src) {
				//for (src = uvc_streaming_std; *src; ++src) {
				bytes += (*src)->bLength;
				n_desc++;
			}
		}
	}

	mem = (uint8_t*)usbd_malloc((n_desc + 1) * sizeof(*src)+ bytes);
	if (mem == NULL)
		return NULL;

	hdr = (void*)mem;
	dst = (void*)mem;
	mem += (n_desc + 1) * sizeof(*src);

	/* Copy the descriptors. */
	UVC_COPY_DESCRIPTOR(mem, dst, &uvc_iad);
	UVC_COPY_DESCRIPTOR(mem, dst, &uvc_control_intf);

	uvc_control_header                      = (void*)mem;
	UVC_COPY_DESCRIPTORS(mem, dst, (const struct usb_descriptor_header**)uvc->desc.control);
	uvc_control_header->wTotalLength        = control_size;
	uvc_control_header->bInCollection       = 1;
	uvc_control_header->baInterfaceNr[0]    = uvc->streaming_intf;

	UVC_COPY_DESCRIPTOR(mem, dst, &uvc_control_ep);
	UVC_COPY_DESCRIPTOR(mem, dst, &uvc_control_cs_ep);

	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
		((struct usb_interface_descriptor*)uvc_streaming_std[0])->bNumEndpoints = 0;
		((struct usb_interface_descriptor*)uvc_streaming_std[0])->bAlternateSetting = 0;
		uvc_streaming_std[1] = NULL;
		UVC_COPY_DESCRIPTORS(mem, dst, (const struct usb_descriptor_header**)uvc_streaming_std);

		uvc_streaming_header = (void*)mem;
		((struct uvc_input_header_descriptor*)uvc_streaming_cls[0] )->bEndpointAddress = USB_DIR_IN | 0x07;
		UVC_COPY_DESCRIPTORS(mem, dst,(const struct usb_descriptor_header**)uvc_streaming_cls);
		uvc_streaming_header->wTotalLength = streaming_size;

		for (i=0; i<5 ; i++) {
			uvc_streaming_std = (speed == USB_SPEED_FULL)  ? uvc_fs_streaming : uvc_hs_streaming;
			uvc_streaming_std[1] = (struct usb_descriptor_header *) &uvc_streaming_ep;
			((struct usb_interface_descriptor*)uvc_streaming_std[0])->bAlternateSetting = i+1;
			((struct usb_interface_descriptor*)uvc_streaming_std[0])->bNumEndpoints = 1;
			((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->bEndpointAddress = uvc_streaming_ep.bEndpointAddress;
			//((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->wMaxPacketSize = iso_maxpayloadsize[i];
			((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->wMaxPacketSize = (speed == USB_SPEED_FULL)? 0x320 : iso_maxpayloadsize[i];
			((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->bmAttributes = USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ASYNC;

			UVC_COPY_DESCRIPTORS(mem, dst,(const struct usb_descriptor_header**)uvc_streaming_std);
		}
	}
	else if(snx_usbd_submode == USBD_SUBMODE_BULK) {
//		((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->bEndpointAddress = USB_DIR_IN | 0x0c;
		((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->bEndpointAddress = uvc_streaming_ep.bEndpointAddress;
		((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->bmAttributes = USB_ENDPOINT_XFER_BULK;
//        ((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->wMaxPacketSize = \
//                                            (speed == USB_SPEED_FULL) ? (uvc_streaming_ep.wMaxPacketSize/8) : uvc_streaming_ep.wMaxPacketSize;
		UVC_COPY_DESCRIPTORS(mem, dst, (const struct usb_descriptor_header**)uvc_streaming_std);

		uvc_streaming_header = (void*)mem;
		((struct uvc_input_header_descriptor*)uvc_streaming_cls[0] )->bEndpointAddress = \
		        ((struct usb_endpoint_descriptor*)uvc_streaming_std[1])->bEndpointAddress;
		UVC_COPY_DESCRIPTORS(mem, dst,(const struct usb_descriptor_header**)uvc_streaming_cls);
		uvc_streaming_header->wTotalLength = streaming_size;
	}

	*dst = NULL;
	return hdr;
}
uint8_t raw_cnt, mjpg_cnt, h264_cnt, raw_cnt;
/**
* @brief interface function - init uvc description function
*/
void uvc_description_init(void)
{
	int key_res_width, key_res_height;
	unsigned int desc_idx = 0;
	uint8_t nfmt, nframe;
	uint32_t i, j;
	uint32_t uvc_fmt_mask = 0;

	key_res_width = UVC_FHD_WIDTH;
	key_res_height = UVC_FHD_HEIGHT;

	USBD_UVC_PRINT("Sensor Key Resolution: %d x %d\r\n", key_res_width, key_res_height);

	if ((key_res_width >= UVC_FHD_WIDTH) && (key_res_height >= UVC_FHD_HEIGHT)) {
		key_res_width = UVC_FHD_WIDTH;
		key_res_height = UVC_FHD_HEIGHT;
	} else if ((key_res_width >= UVC_HD_WIDTH) && (key_res_height >= UVC_HD_HEIGHT)) {
		key_res_width = UVC_HD_WIDTH;
		key_res_height = UVC_HD_HEIGHT;
	} else {
		key_res_width = UVC_VGA_WIDTH;
		key_res_height = UVC_VGA_HEIGHT;
	}

	/*Xu defined  */
	memcpy(&uvc_sys_extension_unit.guidExtensionCode,(uint8_t*)&SNX_SYS_XU_GUID,sizeof(uvc_sys_extension_unit.guidExtensionCode));

	i = 0;
	raw_cnt = 0;
	mjpg_cnt = 0;
	h264_cnt = 0;
	raw_cnt = 0;
	nfmt = 0;
	memset(cur_info, 0, UVC_MAX_FRAME_NUM * sizeof(struct image_info));

	while(img_info_list[i]) {
		memcpy(&cur_info[i],img_info_list[i], sizeof (struct image_info));

		switch(cur_info[i].type) {
		case UVC_VS_FORMAT_MJPEG:
			if((uvc_fmt_mask & UVC_FMT_BITMASK_MJPG) == 0) {
				uvc_fmt_mask |= UVC_FMT_BITMASK_MJPG;
				img_info_desc[nfmt] = img_info_mjpg;
				desc_ops[nfmt].format_func = des_uvc_format_mjpg;
				desc_ops[nfmt].frame_func = des_uvc_frame_mjpg;
				desc_ops[nfmt].frame_cnt = &mjpg_cnt;

				nfmt++;
			}
			img_info_mjpg[mjpg_cnt++] = &cur_info[i];
			break;
		case UVC_VS_FORMAT_FRAME_BASED:
			if((uvc_fmt_mask & UVC_FMT_BITMASK_H264) == 0) {
				uvc_fmt_mask |= UVC_FMT_BITMASK_H264;
				img_info_desc[nfmt] = img_info_h264;
				desc_ops[nfmt].format_func = des_uvc_format_h264;
				desc_ops[nfmt].frame_func = des_uvc_frame_h264;
				desc_ops[nfmt].frame_cnt = &h264_cnt;

				nfmt++;
			}
			img_info_h264[h264_cnt++] = &cur_info[i];
			break;
		case UVC_VS_FORMAT_UNCOMPRESSED:
			if((uvc_fmt_mask & UVC_FMT_BITMASK_RAW) == 0) {
				uvc_fmt_mask |= UVC_FMT_BITMASK_RAW;
				img_info_desc[nfmt] = img_info_raw;
				desc_ops[nfmt].format_func = des_uvc_format_raw;
				desc_ops[nfmt].frame_func = des_uvc_frame_raw;
				desc_ops[nfmt].frame_cnt = &raw_cnt;

				nfmt++;
			}
			img_info_raw[raw_cnt++] = &cur_info[i];
			break;
		default:
			break;
		}
		i++;
	}

	desc_idx = 0;
	/*	input header */
	uvc_hs_streaming_cls[desc_idx++] = ( struct uvc_descriptor_header *) des_uvc_input_header(3);

	/* Video Format & Video Frame */
	for(i=0; i<nfmt; i++) {
		nframe = *(desc_ops[i].frame_cnt);
		uvc_hs_streaming_cls[desc_idx++] = desc_ops[i].format_func(i, nframe);
		for (j=0; j<nframe; j++) {
			uvc_hs_streaming_cls[desc_idx++] = desc_ops[i].frame_func(j, img_info_desc[i][j]->width, img_info_desc[i][j]->height, \
			                                   img_info_desc[i][j]->imagesize, img_info_desc[i][j]->fps_num, \
			                                   (uint8_t*)&img_info_desc[i][j]->fps, (uint32_t*)&img_info_desc[i][j]->payload_size);
		}
	}

	/* Color matching */
	uvc_hs_streaming_cls[desc_idx++] = ( struct uvc_descriptor_header *) &uvc_color_matching;
	uvc_hs_streaming_cls[desc_idx++] = NULL;
}
/**
* @brief interface function - bind uvc function
* @param the pointer of usb configuration
* @param the pointer of usb function
* @return value of error code.
*/
static int uvc_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_ep *ep;
	int ret = -EINVAL;

	/* Allocate endpoints. */
	ep = usb_ep_autoconfig(cdev->gadget, &uvc_control_ep, "UVC-INT");
	if (!ep) {
		USBD_UVC_PRINT("Unable to allocate control EP\n");
		goto error;
	}
	uvc->control_ep = ep;
	//ep->driver_data = uvc;
	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
		uvc_streaming_ep.bmAttributes = USB_ENDPOINT_XFER_ISOC;
		ep = usb_ep_autoconfig(cdev->gadget, &uvc_streaming_ep, "UVC-ISO");         
	}
	else {
		uvc_streaming_ep.bmAttributes = USB_ENDPOINT_XFER_BULK;
		ep = usb_ep_autoconfig(cdev->gadget, &uvc_streaming_ep, "UVC-BULK");
	}

	//ep = usb_ep_autoconfig(cdev->gadget, &uvc_streaming_ep);
	if (!ep) {
		USBD_UVC_PRINT("Unable to allocate streaming EP\n");
		goto error;
	}
	uvc->video.ep = ep;
	//ep->driver_data = uvc;


	if ((ret = usb_interface_id(c, f)) < 0)
		goto error;
	uvc_iad.bFirstInterface = ret;
	uvc_control_intf.bInterfaceNumber = ret;
	uvc->control_intf = ret;

	if ((ret = usb_interface_id(c, f)) < 0)
		goto error;
	uvc_streaming_intf_alt0.bInterfaceNumber = ret;
	uvc->streaming_intf = ret;

	/* Copy descriptors. */
	f->descriptors = uvc_copy_descriptors(USB_SPEED_FULL);
	f->hs_descriptors = uvc_copy_descriptors(USB_SPEED_HIGH);

	uvc->control_req = usb_ep_alloc_request(uvc->control_ep);
	uvc->control_buf = (uint8_t*)usbd_malloc(UVC_MAX_REQUEST_SIZE);
	if (uvc->control_req == NULL || uvc->control_buf == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	uvc->control_req->buf           = uvc->control_buf;
	//uvc->control_req->complete    = uvc_function_ep0_complete;
	//uvc->control_req->context     = uvc;
	uvc->video.req[0] = usb_ep_alloc_request(uvc->video.ep);
//    uvc->video.req[0]->buf = (uint8_t*)pvPortMalloc(300*1024);
//	uvc->video.req[0]->buf = (uint8_t*)pvPortMalloc(sizeof(uint8_t));

	uvc->video.payload_buf = (uint8_t*)usbd_malloc(UVC_MAX_PAYLOAD_SIZE);
	if (uvc->video.payload_buf == NULL ) {
		ret = -ENOMEM;
		goto error;
	}

	uvc->probe.bmHint                   = 0;
	uvc->probe.bFormatIndex             = 1;
	uvc->probe.bFrameIndex              = 1;
	uvc->probe.dwFrameInterval          = 0x051615;
	uvc->probe.wKeyFrameRate            = 0x0000;
	uvc->probe.wPFrameRate              = 0x0000;
	uvc->probe.wCompQuality             = 0x0000;
	uvc->probe.wCompWindowSize          = 0;
	uvc->probe.wDelay                   = 0x0000;
	uvc->probe.dwMaxVideoFrameSize      = 0x0000;
	uvc->probe.dwMaxPayloadTransferSize = 1920 * 1080 * 2;
	/*
	if (size == 34) {
	    uvc->probe.dwClockFrequency                     = 15000000;     // 15MHz
	    uvc->probe.bmFramingInfo                        = 0x03;
	    uvc->probe.bPreferedVersion                     = 0x00;
	    uvc->probe.bMinVersion                          = 0x00;
	    uvc->probe.bMaxVersion                          = 0x00;
	}
	*/
	return 0;
error:
	uvc_function_unbind(c, f);
	return ret;
}
/**
* @brief interface function - unbind uvc function
* @param the pointer of usb configuration
* @param the pointer of usb function
*/
static void uvc_function_unbind(struct usb_configuration *c, struct usb_function *f)
{

	if(uvc->video.payload_buf)	{
		vPortFree(uvc->video.payload_buf);
		uvc->video.payload_buf = NULL;
	}

}
/**
* @brief interface function - get alternate setting
* @param the pointer of usb function
* @param the number of interface
* @param the number of alternate setting
* @return the value of error code.
*/
static int uvc_function_get_alt(struct usb_function *f, unsigned interface, unsigned alt)
{
	if (interface == uvc->control_intf)
		return 0;
	else if (interface != uvc->streaming_intf)
		return -EINVAL;
	else

		return uvc->alt_set;
}
/**
* @brief interface function - set alternate setting
* @param the pointer of usb function
* @param the number of interface
* @param the number of alternate setting
* @return the value of error code.
*/
static int uvc_function_set_alt(struct usb_function *f, unsigned interface, unsigned alt)
{
	enum usb_device_speed speed_mode;

	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	if (interface == uvc->control_intf) {
		if (alt) return -EINVAL;
		return 0;
	}

	if (interface != uvc->streaming_intf)
		return -EINVAL;

	speed_mode = f->config->cdev->gadget->speed;

	uvc->alt_set = alt;

	if(alt == 0) {
		if(snx_usbd_submode == USBD_SUBMODE_BULK) {
//            if (uvc->video.ep)
//                usb_ep_enable(uvc->video.ep, &uvc_streaming_ep);
			if (uvc->video.ep) {
				if(speed_mode == USB_SPEED_HIGH) usb_ep_enable(uvc->video.ep, &uvc_streaming_ep);
				else if(speed_mode == USB_SPEED_FULL) usb_ep_enable(uvc->video.ep, &uvc_fs_streaming_ep);
			}

			if (uvc->control_ep)
				usb_ep_enable(uvc->control_ep, &uvc_control_ep);
		}
		
		if(uvc->state == UVC_STATE_PREVIEW){
			uvc->event = UVC_EVENT_STOP;
			if(usbd_uvc_sema){
				xSemaphoreGive(usbd_uvc_sema);
			}
		}
	}
	else if(alt > 0) {
		if (uvc->video.ep) {
			if(speed_mode == USB_SPEED_HIGH) uvc_streaming_ep.wMaxPacketSize = iso_maxpayloadsize[alt-1];
			else if(speed_mode == USB_SPEED_FULL) uvc_streaming_ep.wMaxPacketSize = 0x320;

			usb_ep_enable(uvc->video.ep, &uvc_streaming_ep);

			//+++ need to move to ep7-ISOERR
			uvc->event = UVC_EVENT_STREAMING;
			if(usbd_uvc_sema)
				xSemaphoreGive(usbd_uvc_sema);
			//+++
		}
		if (uvc->control_ep) {
			usb_ep_enable(uvc->control_ep, &uvc_control_ep);
		}
	}
	else {
		return -EINVAL;
	}

	return 0;
}
static void uvc_function_disable(struct usb_function *f)
{

}
/**
* @brief interface function - process standard UVC request
* @param the request for control endpoint
* @return the value of error code.
*/
static int uvc_events_process_standard(const struct usb_ctrlrequest *ctrl)
{
	switch (ctrl->bRequest) {
	case USB_REQ_CLEAR_FEATURE:
		USBD_UVC_PRINT("clear feature\n");
		if(uvc) {
			uvc->event = UVC_EVENT_STOP;
			usb_ep_set_halt(uvc->func.config->cdev->gadget->ep0, ctrl->wIndex, 0);
		}
		if(usbd_uvc_sema)
			xSemaphoreGive(usbd_uvc_sema);
		break;
	default:
		break;
	}
	return 0;
}
static uint8_t error_code_control;

/**
* @brief interface function - process standard command
* @param the request for control endpoint
* @return the value of error code.
*/
uint8_t uvc_std_process(const struct usb_ctrlrequest *ctrl)
{
	uint8_t cs              = ctrl->wValue >> 8;
	struct usb_ep *ep       = uvc->func.config->cdev->gadget->ep0;
	struct usb_request *req = uvc->func.config->cdev->req;

	if(cs == UVC_VC_VIDEO_POWER_MODE_CONTROL) {
		USBD_UVC_PRINT("UVC_VC_VIDEO_POWER_MODE_CONTROL:\n");
		USBD_UVC_PRINT("ctrl->bRequest : %08X ",ctrl->bRequest);

		switch (ctrl->bRequest) {
		case UVC_SET_CUR:
			req->length = min((uint16_t)0x01,ctrl->wLength);
			usb_ep_queue(ep, req);
			break;

		case UVC_GET_CUR:
			req->length = min((uint16_t)0x01,ctrl->wLength);
			req->buf[0] = 0x20 ;
			usb_ep_queue(ep, req);
			break;

		case UVC_GET_INFO:
			req->length = min((uint16_t)0x01,ctrl->wLength);
			req->buf[0] = 0x03 ;
			usb_ep_queue(ep, req);
			break;

		default:
			error_code_control = INVALID_REQUEST_ERR;
		}
	}
	else if(cs == UVC_VC_REQUEST_ERROR_CODE_CONTROL) {
		USBD_UVC_PRINT("UVC_VC_REQUEST_ERROR_CODE_CONTROL:\n");
		USBD_UVC_PRINT("ctrl->bRequest : %08X ",ctrl->bRequest);

		switch (ctrl->bRequest) {
		case UVC_GET_CUR:
			req->length = min((uint16_t)0x01,ctrl->wLength);
			req->buf[0] = error_code_control ;
			usb_ep_queue(ep, req);
			error_code_control = NO_ERROR_ERR;
			break;

		case UVC_GET_INFO:
			req->length = min((uint16_t)0x01,ctrl->wLength);
			req->buf[0] = 0x01 ;
			usb_ep_queue(ep, req);
			break;

		default:
			error_code_control = INVALID_REQUEST_ERR;
		}
	}
	else {
		error_code_control = INVALID_REQUEST_ERR;
	}

	return error_code_control;
}
/**
* @brief interface function - process ct command
* @param the request for control endpoint
* @return the value of error code.
*/
uint8_t uvc_ct_process(const struct usb_ctrlrequest *ctrl)
{
	error_code_control	= INVALID_CONTROL_ERR;
	return error_code_control;
}

/**
* @brief interface function - process pu command
* @param the request for control endpoint
* @return the value of error code.
*/
uint8_t uvc_pu_process(const struct usb_ctrlrequest *ctrl)
{
	error_code_control	= INVALID_CONTROL_ERR;
	return error_code_control;
}
/**
* @brief interface function - process xu command
* @param the request for control endpoint
* @return the value of error code.
*/
uint8_t uvc_xu_process(const struct usb_ctrlrequest *ctrl)
{
	/*
	    uint8_t xu_id;
	    uint8_t no;
	    uint8_t type;

	    USBD_UVC_PRINT("%s\n",__FUNCTION__);

	    xu_id   = (ctrl->wIndex >> 8) - UVC_USER_XU_ID_1 + 1;
	    no      = ctrl->wValue >> 8;
	    type    = ctrl->bRequest;

	    error_code_control      = uvc_xu_cmd_cb(xu_id,no,type);
	*/
	return error_code_control;
}
/**
* @brief interface function - process class UVC request
* @param the request for control endpoint
* @return the value of error code.
*/
int  uvc_events_process_control(const struct usb_ctrlrequest *ctrl)
{
	int ret = ESUCCESS;
	//uint8_t cs                    = ctrl->wValue >> 8;
	uint8_t entity;
	uint8_t interface;

	USBD_UVC_PRINT("%s\n",__FUNCTION__);

	entity          = ctrl->wIndex >> 8;
	interface       = ctrl->wIndex & 0x00ff;

	if(!uvc  ) return -ECONNRESET;

	// check recipient, must be interface recipient
	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE) {
		USBD_UVC_PRINT("Invalid recipient!\n");
		return INVALID_REQUEST_ERR;
	}

	// check interface
	if (interface != 0) {
		USBD_UVC_PRINT("Invalid interface!\n");
		return INVALID_CONTROL_ERR;
	}

	switch (entity) {
	case UVC_SNX_STD_ID:    //Interface control request
		error_code_control = uvc_std_process(ctrl);
		break;

	case UVC_SNX_CT_ID:
		error_code_control = uvc_ct_process(ctrl);
		break;

	case UVC_SNX_PU_ID:
		error_code_control = uvc_pu_process(ctrl);
		break;

	case UVC_SNX_SYS_XU_ID:
		//case UVC_SNX_USR_XU_ID:
		error_code_control = uvc_xu_process(ctrl);
		break;

	default:
		error_code_control = INVALID_REQUEST_ERR;
	}

	if(error_code_control != NO_ERROR_ERR)
		ret = -EOPNOTSUPP;

	return ret;
}
/**
* @brief interface function - process class UVC streaming request
* @param the request for control endpoint
* @return the value of error code.
*/
int uvc_events_process_streaming(const struct usb_ctrlrequest *ctrl)
{
	int ret;
	unsigned char cs;
	unsigned char type;
	unsigned int imagesize;
	enum usb_device_speed speed_mode;
	struct usb_ep *ep;
	struct usb_request *req;
	struct usb_ctrlrequest ctrl_data;

	USBD_UVC_PRINT("%s\n",__FUNCTION__);
	if(!uvc  ) return -ECONNRESET;

	memcpy((uint8_t*)&ctrl_data, (uint8_t*)ctrl, sizeof(struct usb_ctrlrequest));
	
	ret = 0;
	cs = ctrl_data.wValue >> 8;
	ep = uvc->func.config->cdev->gadget->ep0;
	req = uvc->func.config->cdev->req;
	speed_mode = uvc->func.config->cdev->gadget->speed;


	if (cs == UVC_VS_PROBE_CONTROL) {
		switch (ctrl_data.bRequest) {
		case UVC_SET_CUR:
			req->length = min((uint16_t)sizeof(uvc->probe),ctrl_data.wLength);
			memset(req->buf,0x00,req->length);
			usb_ep_queue(ep, req);

			uvc_probe_setcur = true;
		
			break;

		case UVC_GET_CUR:
			if(uvc_probe_setcur){
				memcpy(&uvc->probe,req->buf,req->length);
				uvc->probe.dwMaxPayloadTransferSize = img_info_desc[uvc->probe.bFormatIndex-1][uvc->probe.bFrameIndex-1]->payload_size[0];
				if(snx_usbd_submode == USBD_SUBMODE_ISO) {
					type = img_info_desc[uvc->probe.bFormatIndex-1][uvc->probe.bFrameIndex-1]->type;
					imagesize = img_info_desc[uvc->probe.bFormatIndex-1][uvc->probe.bFrameIndex-1]->imagesize;

					if(type == UVC_VS_FORMAT_MJPEG) {
						if(imagesize == ((FHD_WIDTH * FHD_HEIGHT) + SNX_MJPG_HEADER_LEN))
							uvc->probe.dwMaxPayloadTransferSize = (iso_maxpayloadsize[4]&0x07FF)*((iso_maxpayloadsize[4]>>11)+1);//0x0c00
						else if(imagesize == ((HD_WIDTH * HD_HEIGHT) + SNX_MJPG_HEADER_LEN))
							uvc->probe.dwMaxPayloadTransferSize = (iso_maxpayloadsize[2]&0x07FF)*((iso_maxpayloadsize[2]>>11)+1); //0x0640
						else if(imagesize == ((VGA_WIDTH * VGA_HEIGHT) + SNX_MJPG_HEADER_LEN))
							uvc->probe.dwMaxPayloadTransferSize = iso_maxpayloadsize[0]; //0x0100
					}
					else {
						uvc->probe.dwMaxPayloadTransferSize = 0x0c00;
					}

					if(speed_mode == USB_SPEED_FULL)
						uvc->probe.dwMaxPayloadTransferSize = 0x320;
					uvc->video.ep->s.payload_xfr_size = uvc->probe.dwMaxPayloadTransferSize;
				}
				else if(snx_usbd_submode == USBD_SUBMODE_BULK){
					if(speed_mode == USB_SPEED_FULL)
						uvc->probe.dwMaxPayloadTransferSize = 256 * 1024; //ohci max xfr size
				}
				
				uvc->probe.dwMaxVideoFrameSize = img_info_desc[uvc->probe.bFormatIndex-1][uvc->probe.bFrameIndex-1]->imagesize;
				uvc_probe_setcur = false;
			}

			req->length = min((uint16_t)sizeof(uvc->probe),ctrl_data.wLength);
			memcpy(req->buf,(void*)&uvc->probe,req->length);
			break;

		case UVC_GET_MIN:
		case UVC_GET_MAX:
		case UVC_GET_DEF:
		case UVC_GET_RES:
			req->length = min((uint16_t)sizeof(uvc->probe),ctrl_data.wLength);
			memcpy(req->buf,(void*)&uvc->probe,req->length);
			uvc->probe.wKeyFrameRate        =       0;
			uvc->probe.wPFrameRate  =       0;
			uvc->probe.wCompQuality =       0;
			uvc->probe.wCompWindowSize      =       0;
			break;

		case UVC_GET_LEN:
			req->length = min((uint16_t)2,ctrl_data.wLength);
			((uint8_t*)req->buf)[0]=0x00 ;
			((uint8_t*)req->buf)[1]=sizeof(uvc->probe) ;
			break;

		case UVC_GET_INFO:
			req->length = 1;
			((uint8_t*)req->buf)[0]=0x00;
			break;
		}
	}
	else if (cs == UVC_VS_COMMIT_CONTROL) {
		switch (ctrl_data.bRequest) {
		case UVC_SET_CUR:
			USBD_UVC_PRINT("UVC_SET_CUR\n");
			req->length = min((uint16_t)sizeof(uvc->commit),ctrl_data.wLength);
			memset(req->buf,0x00,req->length);
			usb_ep_queue(ep, req);
			uvc_commit_setcur = true;
			uvc->commit_done = 1;
			break;

		case UVC_GET_CUR:
		case UVC_GET_MIN:
		case UVC_GET_MAX:
		case UVC_GET_DEF:
		case UVC_GET_RES:
			req->length = min((uint16_t)sizeof(uvc->commit),ctrl_data.wLength);
			memcpy(req->buf,(void*)&uvc->commit,req->length);
			break;

		case UVC_GET_LEN:
			((uint8_t*)req->buf)[0]=0x00 ;
			((uint8_t*)req->buf)[1]=sizeof(uvc->commit);
			req->length = min((uint16_t)2,ctrl_data.wLength);
			break;

		case UVC_GET_INFO:
			((uint8_t*)req->buf)[0]=0x00;
			req->length = 1;
			break;
		}
	}

	if(ctrl_data.bRequest != UVC_SET_CUR){
		usb_ep_queue(ep, req);
	}
	return ret;
}
/**
* @brief interface function - setup function
* @param the pointer of usb function
* @param the request for control endpoint
* @return value of error code.
*/
static int uvc_function_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	int ret = ESUCCESS;
	USBD_UVC_PRINT("uvc_function_setup:setup request %02x %02x value %04x index %04x %04x\n",ctrl->bRequestType, ctrl->bRequest, (ctrl->wValue),(ctrl->wIndex), (ctrl->wLength));

	if(!uvc  ) return -ECONNRESET;
	/* Stall too big requests. */
	if (ctrl->wLength > UVC_MAX_REQUEST_SIZE)
		return -EINVAL;

	switch (ctrl->bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		uvc_events_process_standard(ctrl);
		break;

	case USB_TYPE_CLASS:
		if ((ctrl->bRequestType & USB_RECIP_MASK) == USB_RECIP_INTERFACE) {
			switch (ctrl->wIndex & 0xff) {
			case UVC_INTF_VIDEO_CONTROL:
				ret = uvc_events_process_control(ctrl);
				break;

			case UVC_INTF_VIDEO_STREAMING:
				ret = uvc_events_process_streaming(ctrl);
				break;

			}
		}
		break;

	default:
		ret = -EOPNOTSUPP;
		break;
	}
	return ret;
}

void uvc_device_task(void *pvParameters);
/**
* @brief interface function - configuration bind function
* @param the pointer of usb configuration
* @return value of error code.
*/
static int uvc_config_bind(struct usb_configuration *c)
{
	const struct uvc_descriptor_header * const *control = uvc_control_cls;
	struct uvc_descriptor_header **fs_streaming         = uvc_hs_streaming_cls;
	struct uvc_descriptor_header **hs_streaming         = uvc_hs_streaming_cls;

	int ret = 0;

	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	uvc = (struct uvc_device*)usbd_malloc(sizeof(struct uvc_device));
	if (uvc == NULL)
		return -ENOMEM;

	uvc->state = UVC_STATE_INIT;
	uvc->event = UVC_EVENT_ON;
	uvc->video.imgbuf = NULL;

	if (control == NULL || control[0] == NULL ||  control[0]->bDescriptorSubType != UVC_VC_HEADER)
		goto error;

	if (fs_streaming == NULL || fs_streaming[0] == NULL || fs_streaming[0]->bDescriptorSubType != UVC_VS_INPUT_HEADER)
		goto error;

	if (hs_streaming == NULL || hs_streaming[0] == NULL || hs_streaming[0]->bDescriptorSubType != UVC_VS_INPUT_HEADER)
		goto error;

	uvc->desc.control = control;
	uvc->desc.fs_streaming = fs_streaming;
	uvc->desc.hs_streaming = hs_streaming;

	if ((ret = usb_string_id(c->cdev)) < 0)
		goto error;

	uvc_en_us_strings[UVC_STRING_ASSOCIATION_IDX].id = ret;

	if ((ret = usb_string_id(c->cdev)) < 0)
		goto error;
	uvc_en_us_strings[UVC_STRING_CONTROL_IDX].id = ret;
	uvc_control_intf.iInterface = ret;
	uvc_iad.iFunction = uvc_control_intf.iInterface;

	if ((ret = usb_string_id(c->cdev)) < 0)
		goto error;
	uvc_en_us_strings[UVC_STRING_STREAMING_IDX].id = ret;

	uvc->func.name                  = "USB Video Function";
	uvc->func.strings               = &webcam_function_strings;
	uvc->func.bind                  = uvc_function_bind;
	uvc->func.unbind                = uvc_function_unbind;
	uvc->func.get_alt               = uvc_function_get_alt;
	uvc->func.set_alt               = uvc_function_set_alt;
	uvc->func.disable               = uvc_function_disable;
	uvc->func.setup                 = uvc_function_setup;

	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
		uvc->func.ep_callback[7]	= uvc_functione_ep7_callback;
	}
	else {
		uvc->func.ep_callback[14]	= uvc_functione_ep14_callback;
	}
	uvc->func.ep_callback[5]        = uvc_functione_ep5_callback;

	ret = usb_add_function(c, &uvc->func);

	usbd_uvc_sema = xSemaphoreCreateCounting(50,1);
	usbd_uvc_image_slice = xSemaphoreCreateCounting(1,0);
	usbd_uvc_slice_finish = xSemaphoreCreateCounting(1,0);
	USBD_UVC_PRINT(" usbd_uvc_image_slice = %08x~~~\r\r\n",usbd_uvc_image_slice);
	//usbd_uvc_test= xSemaphoreCreateCounting(1,1);
	
	if(!usbd_uvc_sema) {
		USBD_UVC_PRINT(" create usbd_uvc_sema is fail~~~\r\n");
	}
	usbd_uvc_stop = xSemaphoreCreateCounting(1,0);
	if(!usbd_uvc_stop) {
		USBD_UVC_PRINT(" create usbd_uvc_stop is fail~~~\r\n");
	}

	USBD_UVC_PRINT("create uvc_device_task\r\r\n");
	if (pdPASS != xTaskCreate(uvc_device_task, "uvc_device_task", 4096, (void*) NULL, 250, NULL)) {
		USBD_UVC_PRINT("Could not uvc_device_task\r\n");
	}
#ifdef CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN
	if(pdPASS != uvc_vc_init())	{
		printf("usbd uvc vc init fail\r\n");
	}
#endif	


	return 0;
error:
	return ret;
}


static struct usb_configuration uvc_config_driver = {
	webcam_config_label,                    // label
	0,                                      // usb_function
	uvc_config_bind,                        // bind
	1,                                      // bConfigurationValue
	0,                                      // iConfiguration /* dynamic */
	USB_CONFIG_ATT_SELFPOWER,               // bmAttributes
	CONFIG_USB_GADGET_VBUS_DRAW / 2,        // bMaxPower
};
/**
* @brief interface function - uvc bind function
* @param the pointer of usb compisite device
* @return value of error code.
*/
static int uvc_bind(struct usb_composite_dev *cdev)
{
	int ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	webcam_strings[STRING_MANUFACTURER_IDX].id = ret;
	webcam_device_desc.iManufacturer = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	webcam_strings[STRING_PRODUCT_IDX].id = ret;
	webcam_device_desc.iProduct = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	webcam_strings[STRING_SERIAL_NUM_IDX].id = ret;
	webcam_device_desc.iSerialNumber = ret;

	if ((ret = usb_string_id(cdev)) < 0)
		goto error;
	webcam_strings[STRING_DESCRIPTION_IDX].id = ret;

	uvc_config_driver.iConfiguration = ret;
	webcam_qualifier_desc.bNumConfigurations = uvc_config_driver.iConfiguration;
	uvc_description_init();

	if ((ret = usb_add_config(cdev, &uvc_config_driver)) < 0)
		goto error;

	USBD_UVC_PRINT("Sonix UVC(Webcam) Driver\n");
	return 0;
error:
	uvc_unbind(cdev);
	return ret;
}
/**
* @brief interface function - uvc unbind function
* @param the pointer of usb compisite device
* @return value of error code.
*/
static int uvc_unbind(struct usb_composite_dev *cdev)
{
	return 0;
}
/**
* @brief interface function - UVC State Machine : do standby
*/
static void uvc_do_standby(void)
{
	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);
	if (uvc == NULL ) {
		return;
	}
	uvc->video.header[0]    = UVC_PAYLOAD_HEADER_LENGTH;
	uvc->video.header[1]    = UVC_STREAM_EOH | UVC_STREAM_SCR | UVC_STREAM_PTS ;
	uvc->video.PTS          = 0;
	uvc->video.SCR          = 0;
	uvc->video.SOF1Khz      = 0;
	uvc->state              = UVC_STATE_STANDBY;
}

static void uvc_set_header()
{
#if 1
	uint8_t i = 0;
	uvc->video.PTS += 499995;
	uvc->video.SOF1Khz += 33;
	uvc->video.SCR += 499995;

	for(i=0; i<sizeof(uvc->video.PTS); i++) {
		uvc->video.header[i+2] = uvc->video.PTS >> i*8;
		uvc->video.header[i+6] = uvc->video.SCR >> i*8;
	}
	/*
	uvc->video.header[2] = uvc->video.PTS;
	uvc->video.header[3] = uvc->video.PTS >> 8;
	uvc->video.header[4] = uvc->video.PTS >> 16;
	uvc->video.header[5] = uvc->video.PTS >> 24;
	uvc->video.header[6] = uvc->video.SCR;
	uvc->video.header[7] = uvc->video.SCR >> 8;
	uvc->video.header[8] = uvc->video.SCR >> 16;
	uvc->video.header[9] = uvc->video.SCR >> 24;
	uvc->video.header[10] = uvc->video.SOF1Khz;
	uvc->video.header[11] = uvc->video.SOF1Khz>> 8;
	*/
	if((uvc->video.header[1] & UVC_STREAM_FID) == UVC_STREAM_FID)
		uvc->video.header[1] &= ~(UVC_STREAM_FID);
	else
		uvc->video.header[1] |= UVC_STREAM_FID;
	uvc->video.header[1] &= ~UVC_STREAM_EOF;
#else
	uint32_t us_gap;
	us_gap = ulLT_GetCnt() * 1000;

	uvc->video.PTS += ((us_gap - us_gap_init) * 15);
	uvc->video.SOF1Khz += (us_gap - us_gap_init) / 1000 ;
	uvc->video.SCR += ((us_gap - us_gap_init) * 15);
	us_gap_init = us_gap ;

	memcpy(&uvc->video.header[2], &uvc->video.PTS, sizeof(uvc->video.PTS));
	memcpy(&uvc->video.header[6], &uvc->video.SCR, sizeof(uvc->video.SCR));
	memcpy(&uvc->video.header[10], &uvc->video.SOF1Khz, sizeof(uvc->video.SOF1Khz));

	uvc->video.header[1] |= UVC_STREAM_EOF;
	uvc->video.header[1] ^= UVC_STREAM_FID;
#endif
}

static void uvc_update_header()
{
#if 1
	uint8_t i = 0;
//	uvc->video.PTS += 499995;
	uvc->video.SOF1Khz += 33 ;
	uvc->video.SCR += 499995;

	for(i=0; i<sizeof(uvc->video.PTS); i++) {
//		uvc->video.header[i+2] = uvc->video.PTS >> i*8;
		uvc->video.header[i+6] = uvc->video.SCR >> i*8;
	}

	uvc->video.header[10] = uvc->video.SOF1Khz;
	uvc->video.header[11] = uvc->video.SOF1Khz>> 8;
#else
	uint32_t us_gap;
	us_gap = ulLT_GetCnt() * 1000;

	uvc->video.PTS += ((us_gap - us_gap_init) * 15);
	uvc->video.SOF1Khz += (us_gap - us_gap_init) / 1000 ;
	us_gap_init = us_gap ;

	memcpy(&uvc->video.header[2], &uvc->video.PTS, sizeof(uvc->video.PTS));
	memcpy(&uvc->video.header[6], &uvc->video.SCR, sizeof(uvc->video.SCR));
	memcpy(&uvc->video.header[10], &uvc->video.SOF1Khz, sizeof(uvc->video.SOF1Khz));
#endif
}

uint32_t odd = 0;
void usbd_uvc_drv_send_image(uint32_t *img_buf, unsigned int img_len, uint8_t img_header, uint8_t img_xfr_type)
{
	uint32_t remainder_imgsize = img_len;
	uint32_t image_offset = 0;
	int32_t image_tx_len = 0;

	if (uvc == NULL || uvc->video.imgbuf == NULL) {
		return;
	}

	if(uvc->event == UVC_EVENT_STOP) {
	  return;
	}	

	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);	
	
	image_offset = 0;

	if(img_xfr_type & UVC_XFR_COPY_MEMORY)
		uvc->video.req[0]->buf = uvc->video.payload_buf;
	
	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
		remainder_imgsize = (img_header & UVC_IMG_WITH_HEADER) ? (remainder_imgsize - UVC_PAYLOAD_HEADER_LENGTH) : remainder_imgsize;
		do {
	
			if(img_xfr_type & UVC_XFR_COPY_MEMORY){
				image_tx_len =  ((UVC_MAX_PAYLOAD_SIZE) > remainder_imgsize) ? remainder_imgsize : (UVC_MAX_PAYLOAD_SIZE);			
				uvc->video.req[0]->length = image_tx_len;							
				if(img_header & UVC_IMG_WITH_HEADER)
					memcpy((uint8_t*)uvc->video.req[0]->buf, (uint8_t*)img_buf+(image_offset+UVC_PAYLOAD_HEADER_LENGTH), image_tx_len);
				else
					memcpy((uint8_t*)uvc->video.req[0]->buf, (uint8_t*)img_buf+image_offset, image_tx_len);
			}
			else if(img_xfr_type & UVC_XFR_COPY_POINTER){
				image_tx_len = remainder_imgsize;
				uvc->video.req[0]->length = image_tx_len;
				uvc->video.req[0]->buf = (img_header & UVC_IMG_WITH_HEADER) ? \
							(uint8_t*)img_buf + (image_offset + UVC_PAYLOAD_HEADER_LENGTH) : (uint8_t*)img_buf + image_offset;
			}
			
			remainder_imgsize -= image_tx_len;
			image_offset += image_tx_len;
			
			if(uvc) {
				if(odd)
					uvc->video.ep->state |=  USBD_EP_STATE_UVC_FID;
				else
					uvc->video.ep->state &=  ~USBD_EP_STATE_UVC_FID;

				if(remainder_imgsize <= 0)
					uvc->video.ep->state |=  USBD_EP_STATE_UVC_EOF;
				
				usb_ep_queue(uvc->video.ep, uvc->video.req[0]);
				if(usbd_uvc_slice_finish) {
					xSemaphoreTake(usbd_uvc_slice_finish, portMAX_DELAY);
				}

				if(uvc->event== UVC_EVENT_STOP) {
					break;
				}
			}		
		}while(remainder_imgsize > 0);
		
		odd =~odd;
		uvc->video.ep->state &=~(USBD_EP_STATE_UVC_EOF|USBD_EP_STATE_UVC_FID);		
	}
	else if(snx_usbd_submode == USBD_SUBMODE_BULK){
		//fixme : images preview have garbage in case snx_usbd_submode=USBD_SUBMODE_BULK, img_xfr_type=UVC_XFR_COPY_MEMORY
		if(img_xfr_type & UVC_XFR_COPY_MEMORY)
			image_offset = (img_header & UVC_IMG_WITH_HEADER) ? (image_offset + UVC_PAYLOAD_HEADER_LENGTH) : image_offset;
		
		uvc_set_header();
		do {
			uvc_update_header();	
			
            if((uvc->commit.dwMaxPayloadTransferSize - UVC_PAYLOAD_HEADER_LENGTH) >= remainder_imgsize){
                uvc->video.ep->state |= USBD_EP_STATE_0_PACKET_TAIL;            
            }
            else{
                uvc->video.ep->state &= ~USBD_EP_STATE_0_PACKET_TAIL;
            }  
			
			if(img_xfr_type & UVC_XFR_COPY_MEMORY){
				image_tx_len =  ((UVC_MAX_PAYLOAD_SIZE - UVC_PAYLOAD_HEADER_LENGTH) > remainder_imgsize) ? \
										remainder_imgsize : (UVC_MAX_PAYLOAD_SIZE - UVC_PAYLOAD_HEADER_LENGTH);
				memcpy((uint8_t*)&(uvc->video.req[0]->buf[UVC_PAYLOAD_HEADER_LENGTH]), (uint8_t*)img_buf+image_offset, image_tx_len);
				remainder_imgsize -= image_tx_len;
				image_offset += image_tx_len;					
			}
			else if(img_xfr_type & UVC_XFR_COPY_POINTER){
				image_tx_len =  remainder_imgsize;
				
				if(image_offset == 0){
					uvc->video.req[0]->buf = (uint8_t*)img_buf;
					remainder_imgsize -= image_tx_len;
					image_offset += image_tx_len;
				}
				else{
					uvc->video.req[0]->buf = (uint8_t*)img_buf + (image_offset - UVC_PAYLOAD_HEADER_LENGTH);
					remainder_imgsize -= image_tx_len;
					image_offset += (image_tx_len - UVC_PAYLOAD_HEADER_LENGTH);
					image_tx_len += UVC_PAYLOAD_HEADER_LENGTH;

				}				
			}
			
			if(image_tx_len >= remainder_imgsize) uvc->video.header[1] |= UVC_STREAM_EOF;
						
			memcpy((uint8_t*)uvc->video.req[0]->buf, (uint8_t*)uvc->video.header, UVC_PAYLOAD_HEADER_LENGTH);			
			
//			remainder_imgsize -= image_tx_len;
//			image_offset += image_tx_len;
			if(uvc) {				
				//uvc->video.req[0]->length = image_tx_len + UVC_PAYLOAD_HEADER_LENGTH;
				uvc->video.req[0]->length = (img_xfr_type & UVC_XFR_COPY_MEMORY) ? image_tx_len + UVC_PAYLOAD_HEADER_LENGTH : image_tx_len;
				usb_ep_queue(uvc->video.ep, uvc->video.req[0]);
				if(usbd_uvc_slice_finish) {
					xSemaphoreTake(usbd_uvc_slice_finish, portMAX_DELAY);
				}

				if(uvc->event== UVC_EVENT_STOP) {
					break;
				}
			}				
		}while(remainder_imgsize > 0);		
	}

//	uint32_t remainder_imgsize = img_len;
//	uint32_t image_offset = 0;
//	int32_t image_tx_len = 0;

//	if (uvc == NULL || uvc->video.imgbuf == NULL) {
//		return;
//	}

//	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

//	image_offset = 0;

//	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
//		do {
//			image_tx_len =  ((UVC_MAX_PAYLOAD_SIZE) > remainder_imgsize) ? remainder_imgsize : (UVC_MAX_PAYLOAD_SIZE);
//#if (USBD_UVC_IMGAE_HEADER == 1)			
//			uvc->video.req[0]->buf = (uint8_t*)img_buf + UVC_PAYLOAD_HEADER_LENGTH;
//			uvc->video.req[0]->length = image_tx_len - UVC_PAYLOAD_HEADER_LENGTH;
//#else
//			uvc->video.req[0]->buf = (uint8_t*)img_buf;
//			uvc->video.req[0]->length = image_tx_len;			
//#endif			
//			remainder_imgsize -= image_tx_len;
//			image_offset += image_tx_len;

//			if(uvc) {
////                uvc->video.req[0]->length = image_tx_len + UVC_PAYLOAD_HEADER_LENGTH;
////				uvc->video.req[0]->length = image_tx_len;
//				if(odd)
//					uvc->video.ep->state |=  USBD_EP_STATE_UVC_FID;
//				else
//					uvc->video.ep->state &=  ~USBD_EP_STATE_UVC_FID;

//				if(remainder_imgsize <= 0)
//					uvc->video.ep->state |=  USBD_EP_STATE_UVC_EOF;
//				usb_ep_queue(uvc->video.ep, uvc->video.req[0]);
//				if(usbd_uvc_slice_finish) {
//					xSemaphoreTake(usbd_uvc_slice_finish, portMAX_DELAY);
//				}

//				if(uvc->event== UVC_EVENT_STOP) {
//					break;
//				}
//			}
//		} while(remainder_imgsize > 0);

//		odd =~odd;
//		uvc->video.ep->state &=~(USBD_EP_STATE_UVC_EOF|USBD_EP_STATE_UVC_FID);
//	}
//	else if(snx_usbd_submode == USBD_SUBMODE_BULK) {

//#if (USBD_UVC_IMGAE_HEADER == 1)
//		//memcpy(uvc->video.imgbuf, img_buf, img_len);
//		uvc->video.req[0]->buf = (uint8_t*)img_buf;
//#endif
//		uvc_set_header();
//		do {
//			uvc_update_header();
//			if((uvc->commit.dwMaxPayloadTransferSize - UVC_PAYLOAD_HEADER_LENGTH) >= remainder_imgsize) {
//				uvc->video.header[1] |= UVC_STREAM_EOF;
//				uvc->video.ep->state |= USBD_EP_STATE_0_PACKET_TAIL;
//			}
//			else {
//				uvc->video.ep->state &= ~USBD_EP_STATE_0_PACKET_TAIL;
//			}
//			image_tx_len =  ((UVC_MAX_PAYLOAD_SIZE - UVC_PAYLOAD_HEADER_LENGTH) > remainder_imgsize) ? remainder_imgsize : (UVC_MAX_PAYLOAD_SIZE - UVC_PAYLOAD_HEADER_LENGTH);
//			if((UVC_MAX_PAYLOAD_SIZE - UVC_PAYLOAD_HEADER_LENGTH) >= remainder_imgsize)
//				uvc->video.header[1] |= UVC_STREAM_EOF;

//			memcpy((uint8_t*)uvc->video.req[0]->buf, (uint8_t*)uvc->video.header, UVC_PAYLOAD_HEADER_LENGTH);
//            memcpy(&(uvc->video.req[0]->buf[UVC_PAYLOAD_HEADER_LENGTH]), &(img_buf[image_offset]), image_tx_len);
//			
//			remainder_imgsize -= image_tx_len;
//			image_offset += image_tx_len;

//			if(uvc) {
//#if (USBD_UVC_IMGAE_HEADER == 1)
//				uvc->video.req[0]->length = image_tx_len;
//#else
//				uvc->video.req[0]->length = image_tx_len + UVC_PAYLOAD_HEADER_LENGTH;
//#endif
//				usb_ep_queue(uvc->video.ep, uvc->video.req[0]);

//				if(usbd_uvc_slice_finish) {
//					xSemaphoreTake(usbd_uvc_slice_finish, portMAX_DELAY);
//				}

//				if(uvc->event== UVC_EVENT_STOP) {
//					break;
//				}
//			}
//		} while(remainder_imgsize > 0);
//	}
}
/**
* @brief interface function - UVC State Machine : do start preview
*/
static void uvc_do_startpreview(void)
{
//	uint32_t total_img_len = 0;
	struct usb_request *req = uvc->func.config->cdev->req;

	USBD_UVC_PRINT("%s, resolution=%dx%d, fps=%d\n", __FUNCTION__, uvc->video.img.width, uvc->video.img.height, uvc->video.img.fps);

	if(uvc_commit_setcur){
		memcpy(&uvc->commit,req->buf,req->length);

		uvc->commit.dwMaxPayloadTransferSize = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->payload_size[0];
		if(snx_usbd_submode == USBD_SUBMODE_ISO) {
			uvc->commit.dwMaxPayloadTransferSize = 0x0c00;
		}
		uvc->commit.dwMaxVideoFrameSize = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->imagesize;

		uvc->video.img.type = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->type;
		uvc->video.img.width = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->width ;
		uvc->video.img.height = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->height ;
		uvc->video.img.imagesize = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->imagesize;
		//uvc->video.img.bps = 1024 * 1024;
		uvc->video.img.payload_size = img_info_desc[uvc->commit.bFormatIndex-1][uvc->commit.bFrameIndex-1]->payload_size[0];
		uvc->video.img.fps = 10000000 / uvc->commit.dwFrameInterval;
		
		uvc_commit_setcur = false;
	}

	if(uvc) {
		uvc->video.imggap = (1000*1.5/uvc->video.img.fps);
		uvc->video.imgbuf = (uint8_t*)usbd_malloc(sizeof(uint8_t));

		if (uvc->video.imgbuf == NULL) {
			printf("warning !!! Unable to allocate space for image buffer !!!\r\n");
		}
		uvc->state = UVC_STATE_PREVIEW;
	}

	if(snx_usbd_submode == USBD_SUBMODE_ISO) {
		outl((unsigned int*)USBD_REG_EP_SEL, 0x87);
		outl(USBD_REG_CONF,  USBD_CONF_BIT_DEVEN | USBD_CONF_BIT_DMULT);
	}

#ifdef CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN
	uvc_vc_set_image_info(&(uvc->video.img));
#else
	if(start_preview_callback)
		start_preview_callback(&(uvc->video.img));
#endif
}
/**
* @brief interface function - UVC State Machine : do stop preview
*/
static void uvc_do_stoppreview(void)
{
	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	usb_ep_reset(uvc->video.ep);

	if(uvc) {
		if (uvc->video.imgbuf != NULL) {
			vPortFree(uvc->video.imgbuf); //comment: mark for 7320 project
			//uvc->video.imgbuf = NULL;
		}
		uvc->video.imggap = 0;
		uvc->state = UVC_STATE_STANDBY;
		uvc->commit_done = 0;
	}

	if(usbd_uvc_slice_finish != NULL)
		xSemaphoreGive(usbd_uvc_slice_finish);

#ifdef CONFIG_MODULE_USB_DEVICE_DUMMY_PATTERN
	usbd_uvc_vc_task_stop();
#else 	
	if(stop_preview_callback)
		stop_preview_callback();
#endif
}
/**
* @brief interface function - UVC State Machine : do suspend
*/
static void uvc_do_suspend(void)
{
	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	if(uvc) uvc->state = UVC_STATE_SUSPEND;
}
/**
* @brief interface function - UVC State Machine : do resume
*/
static void uvc_do_resume(void)
{
	USBD_UVC_PRINT("%s\r\n",__FUNCTION__);

	if(uvc) uvc->state = UVC_STATE_STANDBY;
}
/**
* @brief interface function - UVC State Machine Task
* @param the structure of input data
*/
void uvc_device_task( void *pvParameters )
{
	uvc_task_run = 1;

	while(uvc_task_run) {
		xSemaphoreTake(usbd_uvc_sema, portMAX_DELAY);
		USBD_UVC_PRINT("event:%d->state:%d\n",uvc->event,uvc->state);
	
		switch(uvc->event) {
		case UVC_EVENT_ON:
			if(uvc->state == UVC_STATE_INIT) {
				uvc_do_standby();
			}
			break;

		case UVC_EVENT_OFF:
			if(uvc->state == UVC_STATE_STANDBY) {

			}
			else if(uvc->state == UVC_STATE_PREVIEW) {

			}
			break;

		case UVC_EVENT_STREAMING:
			if(uvc->state == UVC_STATE_STANDBY) {
				uvc_do_startpreview();
			}
			break;

		case UVC_EVENT_STOP:
			if(uvc->state == UVC_STATE_PREVIEW) {
				uvc_do_stoppreview();
			}
			break;

		case UVC_EVENT_SUSPEND:
			if(uvc->state == UVC_STATE_PREVIEW) {
				uvc_do_suspend();
			}
			else if(uvc->state == UVC_STATE_STANDBY) {
				uvc_do_suspend();
			}
			break;
		case UVC_EVENT_RESUME:
			if(uvc->state == UVC_STATE_SUSPEND) {
				uvc_do_resume();
			}
			break;

		case UVC_EVENT_IDLE:
		case UVC_EVENT_VSYNC:
		default:
			break;
		}
	}
	vTaskDelete(NULL);
	//if(usbd_uvc_stop)
	//xSemaphoreGive(usbd_uvc_stop);

}

void uvc_device_task_stop(void)
{
	uvc_task_run = 0;
	if(usbd_uvc_sema) {
		vSemaphoreDelete(usbd_uvc_sema);
		usbd_uvc_sema =NULL;
	}

	if(usbd_uvc_slice_finish) {
		vSemaphoreDelete(usbd_uvc_slice_finish);
		usbd_uvc_slice_finish =NULL;
	}
}
/*
host -> device
*/
uint32_t usbd_uvc_drv_read_data(uint8_t* buf,uint32_t* len)
{
	struct usb_ep *ep;
	struct usb_request *req;
	uint32_t err;

	ep = uvc->func.config->cdev->gadget->ep0;
	req = uvc->func.config->cdev->req;

	req->length = *len;
	err = usb_ep_queue(ep, req);
	memcpy(buf,req->buf,req->length);
	*len = req->length;
	return err;
}

/*
host <- device
*/
uint32_t usbd_uvc_drv_write_data(uint8_t* buf,uint32_t len)
{
	struct usb_ep *ep;
	struct usb_request *req;
	uint32_t err;

	ep = uvc->func.config->cdev->gadget->ep0;
	req = uvc->func.config->cdev->req;

	memcpy(req->buf,buf,len);
	req->length = len;
	err = usb_ep_queue(ep, req);
	return err;
}

#endif //#if defined( CONFIG_MODULE_USBD_UVC_CLASS)
