#include "sonix_config.h"
#include <stdlib.h>

#include "USBH_UVC.h"

#if defined (CONFIG_USBH_CLI_UVC)
enum {
	SLICE_PIC	= 1,
	SLICE_PA,
	SLICE_PB,
	SLICE_PC,
	SLICE_IDR_PIC,
	SEI,
	SPS,
	PPS,
};

enum {
	TYPE_ERROR = -2,
	NOT_SLICE,
	TYPE_P,
	TYPE_B,
	TYPE_I,
	TYPE_SP,
	TYPE_SI
};

typedef struct bits_handler {
	const char *ptr, *base;
	unsigned length;
	int index;
} bits_handler;

typedef struct
{
	uint32_t	stream_id;
	uint32_t 	*ptr;
	uint32_t 	size;
	uint32_t 	framecnt;
	uint32_t	xact_errcnt;
	uint32_t	xact_TIMEOUT_cnt;
	uint32_t	xact_RXERR_cnt;
	uint32_t	xact_PID_ERR_cnt;
	uint32_t	xact_UVC_H_ERR_cnt;
	uint32_t	xact_CRC16_ERR_cnt;
	uint32_t	babblecnt;
	uint32_t	underflowcnt;
	uint32_t	discardcnt;
	uint32_t	errdiscardcnt;	
	char		fmt[8];
	char		width[8];
	char		height[8];	
	char		fps[8];	
	uint32_t	debug_msg:1;
	uint32_t	sd_record:1;
	uint32_t	usb_preview:1;
	uint32_t	lcm_preview:1;
	uint32_t	isH264:1;
	uint32_t	isIFrame:1;
	uint32_t	isOpen:1;
	uint32_t	reserve:25;
	
}USBH_UVC_APP_STREAM_STRUCTURE;

typedef struct
{  
	USBH_UVC_APP_STREAM_STRUCTURE	stream[max_stream_count];	
}USBH_UVC_APP_DEV_STRUCTURE;

typedef struct
{
	uint32_t 			sd_record_enable;	
	uint32_t 			usb_preview_enable;		
	uint32_t 			lcm_preview_enable;	
	uint32_t 			debug_enable;	
	uint32_t 			err_debug_enable;
	USBH_UVC_APP_DEV_STRUCTURE	dev[10];
	
}USBH_UVC_APP_STRUCTURE;

extern USBH_UVC_APP_STRUCTURE usbh_uvc_app;

extern void vRegisterUSBHUVCCLICommands(void);
#endif
