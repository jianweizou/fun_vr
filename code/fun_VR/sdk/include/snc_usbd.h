/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_usbd_drv.h
 *
 * @author yiling_chen
 * @version 0.1
 * @date 2018/09/03
 * @license
 * @description
 */

#ifndef __snc_usb_device_h__
#define __snc_usb_device_h__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                  Constant Definition
//=============================================================================

#define UVC_IMG_WITHOUT_HEADER                               	(1<<0)
#define UVC_IMG_WITH_HEADER                                  	(1<<1)

#define UVC_XFR_COPY_MEMORY										(1<<0)
#define UVC_XFR_COPY_POINTER									(1<<1)


typedef enum {
	USBD_MODE_MSC = 0,
	USBD_MODE_UVC,
	USBD_MODE_HID,
	USBD_MODE_UAC,
	USBD_MODE_UNKNOWN = 255,
} usbd_class_mode_t;

typedef enum {
	USBD_SUBMODE_NONE= 0,
	USBD_SUBMODE_BULK,
	USBD_SUBMODE_ISO,
	USBD_SUBMODE_UNKNOWN = 255,
} usbd_class_submode_t;


//=============================================================================
//                  Structure Definition
//=============================================================================


struct preview_image_info {
	uint8_t                         type;
	uint16_t                        width;
	uint16_t                        height;
	uint32_t                        imagesize;
	uint8_t                         fps;
	uint32_t                        bps;
	uint32_t                        payload_size;
} __attribute__ ((packed));


/* UVC callback function type define */
typedef uint8_t (*usbd_drv_uvc_xu_cmd_cb_t)(uint8_t xu_id, uint8_t no, uint8_t type);
typedef void (*usbd_drv_uvc_start_preview_cb_t)(struct preview_image_info *img);
typedef void (*usbd_drv_uvc_stop_preview_cb_t)(void);

/* UAC callback function type define */
typedef int (*usbd_drv_uac_start_record_cb_t)(uint32_t SamplesPerSec);
typedef void (*usbd_drv_uac_stop_record_cb_t)(void);
typedef int (*usbd_drv_uac_start_play_cb_t)(uint32_t SamplesPerSec);
typedef void (*usbd_drv_uac_stop_play_cb_t)(void);
typedef int (*usbd_drv_uac_ctrl_volume_cb_t)(unsigned char channel, unsigned char ctrl, unsigned short *volume);
typedef int (*usbd_drv_uac_ctrl_mute_cb_t)(unsigned char channel, unsigned char ctrl, unsigned char *enable);

/* MSC callback function type define */
typedef uint8_t (*usbd_drv_msc_detect_cb_t)(uint8_t* readonly, uint32_t* maxblock);
typedef int (*usbd_drv_msc_std_cmd_rdata_cb_t)(uint32_t addr, uint32_t len, uint8_t *buf);
typedef int (*usbd_drv_msc_std_cmd_wdata_cb_t)(uint32_t addr, uint32_t len, uint8_t *buf);



//=============================================================================
//                  Public Function Definition
//=============================================================================

/* USB device middleware functions */
void usbd_mid_test_init(usbd_class_mode_t mode, usbd_class_submode_t xfer);
void usbd_mid_test_uninit(void);


/* USB device functions */
void usbd_set_class_mode(usbd_class_mode_t mode, usbd_class_submode_t sub_mode);
void USBDClassDrvInit(void);
void usbd_drv_task_init(void);
void usbd_drv_task_uninit(void);


/* UVC device callback functions */
void usbd_drv_uvc_start_preview_reg_cb(usbd_drv_uvc_start_preview_cb_t cb);
void usbd_drv_uvc_stop_preview_reg_cb(usbd_drv_uvc_stop_preview_cb_t cb);
void usbd_uvc_drv_send_image(uint32_t *img_buf, unsigned int img_len, uint8_t img_header, uint8_t img_xfr_type);


/* uac callback functions */
void usbd_drv_uac_start_record_reg_cb(usbd_drv_uac_start_record_cb_t cb);
void usbd_drv_uac_stop_record_reg_cb(usbd_drv_uac_stop_record_cb_t cb);
void usbd_drv_uac_start_play_reg_cb(usbd_drv_uac_start_play_cb_t cb);
void usbd_drv_uac_stop_play_reg_cb(usbd_drv_uac_stop_play_cb_t cb);
void usbd_drv_uac_ctrl_volume_reg_cb(usbd_drv_uac_ctrl_volume_cb_t cb);
void usbd_drv_uac_ctrl_mute_reg_cb(usbd_drv_uac_ctrl_mute_cb_t cb);
void usbd_uac_drv_do_record(unsigned char *audio_buf, unsigned int audio_len);
int usbd_uac_drv_do_play(unsigned char *audio_buf, unsigned int audio_len);	


/* MSC device callback functions */
void usbd_drv_msc_detect_reg_cb(usbd_drv_msc_detect_cb_t cb);
void usbd_drv_msc_std_cmd_rdata_reg_cb(usbd_drv_msc_std_cmd_rdata_cb_t cb);
void usbd_drv_msc_std_cmd_wdata_reg_cb(usbd_drv_msc_std_cmd_wdata_cb_t cb);

/* HID functions */
void usbd_drv_hid_cmd_rdata(uint8_t *buf,uint32_t len);

#ifdef __cplusplus
}
#endif


#endif  //__snc_usb_device_h__
