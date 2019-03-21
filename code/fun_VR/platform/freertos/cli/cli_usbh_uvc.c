#include "sonix_config.h"

#if defined (CONFIG_USBH_CLI_UVC) && defined (CONFIG_MODULE_USB_UVC_CLASS)

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include <semphr.h>

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* USBH Driver includes. */
#include "USBH_UVC.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "cli_usbh_uvc.h"

#if defined	(CONFIG_USBH_CLI_SUPPORT_USBD_PREV)
#include "usb_device.h"
#include "usbd_uvc.h"
#endif

#define assert(x)

USBH_UVC_APP_STRUCTURE usbh_uvc_app;

uint8_t	cli_uvc_inited = 0;
uint8_t usbh_inited = 0, usbd_inited = 0;

extern usbd_class_mode_t		snx_usbd_mode;
extern usbd_class_submode_t	snx_usbd_submode;

static BaseType_t prvUSBHUVCGetInfoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvUSBHUVCStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvUSBHUVCStopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static const CLI_Command_Definition_t xUSBH_UVC_Get_Info =
{
	"usbh_uvc_get_info",
	"\r\nusbh_uvc_get_info:\r\n get uvc descriptor\r\n",
	prvUSBHUVCGetInfoCommand,
	0
};

static const CLI_Command_Definition_t xUSBH_UVC_Start =
{
	"usbh_uvc_start",
	"\r\nusbh_uvc_start:\r\n open a video stream <device_id> <Fmt> <ResX> <ResY> <fps> <(preview)/(null)>\r\n",
	prvUSBHUVCStartCommand,
	6
};

static const CLI_Command_Definition_t xUSBH_UVC_Stop =
{
	"usbh_uvc_stop",
	"\r\nusbh_uvc_stop:\r\n close a video stream <stream_id>\r\n",
	prvUSBHUVCStopCommand,
	1
};

/*-----------------------------------------------------------*/

void vRegisterUSBHUVCCLICommands( void )
{
	FreeRTOS_CLIRegisterCommand( &xUSBH_UVC_Get_Info );
	FreeRTOS_CLIRegisterCommand( &xUSBH_UVC_Start );
	FreeRTOS_CLIRegisterCommand( &xUSBH_UVC_Stop );
}
/*-----------------------------------------------------------*/

static BaseType_t prvUSBHUVCGetInfoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	USBH_UVC_INFO_Struct			*uvc_info = NULL;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;

	uvc_info	=	(USBH_UVC_INFO_Struct*) uvc_get_info();
	uvc_print_info_data(uvc_info);

	return pdFALSE;
}

static void bits_handler_init(bits_handler *bs, char *buffer, unsigned buffer_length)
{
	bs->ptr = bs->base = buffer;
	bs->length = buffer_length;
	bs->index = 0;
}

static unsigned long read_bits(bits_handler *bs, int bits)
{
	int valid_bits = 8 - bs->index;
	int remain_bits = bits - valid_bits;
	unsigned long v = 0;
	int bytealign, i;

	assert(bits < 33);

	if (!(bits > valid_bits)) {
		v = ((*bs->ptr >> (valid_bits - bits)) & ((1 << bits) -1));
		bs->index += bits;
		return v;
	}

	v = (*bs->ptr++ & ((1 << valid_bits) - 1)) << 8;
	bs->index = 0;
	bytealign = ((remain_bits | 0x07) & ~(unsigned long)0x07) >> 3;
	for (i=0; i<bytealign; ++i) {
		//v = (v | *bs->ptr++) << 8;
		if (remain_bits > 8) {
			v = (v | *bs->ptr++) << 8;
			remain_bits -= 8;
		} else {
			v = (v | *bs->ptr) >> (8 - remain_bits);
			bs->index = remain_bits;
		}
	}

	return v;
}

static unsigned long ue_exp_golomb(bits_handler *bs)
{
	int leading_zero_bits = -1;
	int b;

	for (b = 0; !b; ++leading_zero_bits) {
		b = read_bits(bs, 1);
	}

	return (1 << leading_zero_bits) - 1 + read_bits(bs, leading_zero_bits);
}


static int get_slice_type(char *ptr, unsigned remain_sz)
{
	char *p = NULL;
	unsigned short nal_unit_type;
	int slice_type = NOT_SLICE;

	// 4 bytes start code and 4 bytes content should be parsed
	if (remain_sz < 8) {
#if _DEBUG_VERBOSE
		print_msg_queue("%s: remain data size not enough\n", __func__);
#endif
		return TYPE_ERROR;
	}

	p = ptr;
	p += 4;
	nal_unit_type = (*p++) & 0x1F;
	switch(nal_unit_type) {
		case SLICE_PIC:
		{
			bits_handler bs;
			//unsigned first_mb_in_slice;
			//unsigned pic_parameter_set_id;

			bits_handler_init(&bs, p, 32);
			//first_mb_in_slice = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			slice_type = ue_exp_golomb(&bs);
			//pic_parameter_set_id = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			break;
		}
		case SLICE_PA:
		case SLICE_PB:
		case SLICE_PC:
#if _DEBUG_VERBOSE
			print_msg_queue("*** find coded slice partition, "
				"nal_unit_type %u ***\n", nal_unit_type);
#endif
			break;
		case SLICE_IDR_PIC:
		{
			bits_handler bs;
			//unsigned first_mb_in_slice;
			//unsigned pic_parameter_set_id;

			bits_handler_init(&bs, p, 32);
			//first_mb_in_slice = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			slice_type = ue_exp_golomb(&bs);
			//pic_parameter_set_id = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			break;
		}
		case SEI:
#if _DEBUG_VERBOSE
			print_msg_queue("*** find SEI ***\n");
#endif
			break;
		case SPS:
		case PPS:
			break;
		default:
#if _DEBUF_VREBOSE
			print_msg_queue("*** H.264 nal_unit_type %u ***\n",
					nal_unit_type);
#endif
			break;
	}

	return slice_type;
}

int snx_avc_get_slice_type(unsigned char *paddr, unsigned size)
{
	unsigned *ptr = (unsigned*)paddr;
	unsigned left_sz = size, value, status = 0;
	short last_status;
	int slice_type = TYPE_ERROR;

	if ((unsigned) ptr & 0x03L) {
#if _DEBUG_VERBOSE
		print_msg_queue("%s: bit-stream buffer not aligned\n");
#endif
	}
get_next_four_bytes:
	if (left_sz > 3) {
		value = *(unsigned*) ptr++;
		left_sz -= 4;
	} else if (left_sz > 0) {
		char *ptr_noaligned = (char*) ptr;
		value = 0x0L;
		while (left_sz) {
			value = value << 8;
			value |= *ptr_noaligned;
			ptr_noaligned++;
			left_sz--;
		}
		ptr = (unsigned*) ptr_noaligned;
	} else {
		goto parse_end;
	}
//parse_start:
	if (value == 0x01000000L) {
		last_status = 0;
		status = 5;
	}
state_start:
	switch (status) {
		case 0:
			if ((value & 0xFF000000L) == 0L) {
				status = 1;
			if ((value & 0xFFFF0000L) == 0L) {
				status = 2;
			if ((value & 0xFFFFFF00L) == 0L) {
				status = 3;
			if ((value & 0xFFFFFFFFL) == 0L)
				status = 4;
			}}}
			goto get_next_four_bytes;
		case 1:
			if ((value & 0x00FFFFFFL) == 0x00010000) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 2:
			if ((value & 0x0000FFFFL) == 0x00000100) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 3:
			if ((value & 0x000000FFL) == 0x00000001) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 4:
			// get next four bytes,
			// if value is one of case 1, 2, 3 or equal to
			// 0x01000000, then founded
			last_status = status;
			goto get_next_four_bytes;
		case 5:
		{
			char *start_code = NULL;
			if (last_status == 0) {
				start_code = (char*)(--ptr);
				left_sz += 4;
			} else if (last_status == 1) {
				start_code = (char*)(ptr - 2);
				start_code += 3;
				left_sz += 5;
			} else if (last_status == 2) {
				start_code = (char*)(ptr - 2);
				start_code += 2;
				left_sz += 6;
			} else if (last_status == 3) {
				start_code = (char*)(ptr - 2);
				start_code += 1;
				left_sz += 7;
			} else {
#if _DEBUG_VERBOSE
				print_msg_queue("%s: start code parsing "
						"failed\n", __func__);
#endif
			}


			// TODO: need to handle error case
			if ((slice_type = get_slice_type(start_code, left_sz)) < TYPE_P) {
				// integer pointer address must align 4 bytes
				status = 0;
				ptr = (unsigned *)(((unsigned)start_code + 4) & 0xFFFFFFFCL);
				left_sz -= 4 + ((unsigned)start_code & 0x03L);
				goto get_next_four_bytes;
			}
			break;
		}
	}
parse_end:
	return ((slice_type > 4) ? (slice_type - 5) : slice_type);
}

uint8_t streamid_to_streamidx(uint8_t device_id, uint8_t stream_id) {
	uint8_t i = 0;
	
	for(i = 0; i < max_stream_count; i++) {
		if(usbh_uvc_app.dev[device_id].stream[i].stream_id == stream_id)
			return i;
	}
	
	return max_stream_count;
}

void uvc_stream_xfr_task(void *pvParameters)
{
	USBH_UVC_MW_STRUCTURE		usbh_uvc_mw;
	int currentFrameType = 0;
	uint8_t stream_idx = max_stream_count;
	uint8_t err_event = 0;
	
	memset(&usbh_uvc_mw, 0x00, sizeof(usbh_uvc_mw));
	
	for(;;){
NEXT:		
		xQueueReceive(usbh_queue_uvc_mw, &usbh_uvc_mw, USBH_MAX);
        
		stream_idx = streamid_to_streamidx(usbh_uvc_mw.dev_id, usbh_uvc_mw.stream_id);

		if(usbh_uvc_mw.xact_err){
			usbh_uvc_mw.xact_err = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_errcnt++;
			
//			if(usbh_uvc_mw.xact_err_cat.TIMEOUT == 1) {
//				usbh_uvc_mw.xact_err_cat.TIMEOUT = 0;
//				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_TIMEOUT_cnt++;
//				
//				//UVC_INFO("ERR: xact_err_TIMEOUT_cnt = %d", usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_TIMEOUT_cnt);
//			}
//	
//			if(usbh_uvc_mw.xact_err_cat.RXERR == 1) {
//				usbh_uvc_mw.xact_err_cat.RXERR = 0;
//				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_RXERR_cnt++;
//				
//				//UVC_INFO("ERR: xact_err_RXERR_cnt = %d", usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_RXERR_cnt);
//			}
//	
//			if(usbh_uvc_mw.xact_err_cat.PID_ERR == 1) {
//				usbh_uvc_mw.xact_err_cat.PID_ERR = 0;
//				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_PID_ERR_cnt++;
//				
//				//UVC_INFO("ERR: xact_err_PID_ERR_cnt = %d", usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_PID_ERR_cnt);
//			}
//	
//			if(usbh_uvc_mw.xact_err_cat.UVC_H_ERR == 1) {
//				usbh_uvc_mw.xact_err_cat.UVC_H_ERR = 0;
//				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_UVC_H_ERR_cnt++;
//				
//				//UVC_INFO("ERR: xact_err_UVC_H_ERR_cnt = %d", usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_UVC_H_ERR_cnt);
//			}
//	
//			if(usbh_uvc_mw.xact_err_cat.CRC16_ERR == 1) {
//				usbh_uvc_mw.xact_err_cat.CRC16_ERR = 0;
//				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_CRC16_ERR_cnt++;
//				
//				//UVC_INFO("ERR: xact_err_CRC16_ERR_cnt = %d", usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_id].xact_CRC16_ERR_cnt);
//			}
			
			err_event = 1;
		}

		if(usbh_uvc_mw.babble){
			usbh_uvc_mw.babble = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt ++;
			
			err_event = 1;
		}
		
		if(usbh_uvc_mw.underflow){
			usbh_uvc_mw.underflow = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].underflowcnt ++;
			
			//err_event = 1;
		}
		
		if(usbh_uvc_mw.discard){
			usbh_uvc_mw.discard = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].discardcnt ++;
			
			//err_event = 1;
		}			

		if(usbh_uvc_mw.errdiscard){
			usbh_uvc_mw.errdiscard = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errdiscardcnt ++;
			
			//err_event = 1;
		}		

		if(err_event) {
			UVC_INFO("ERR: %d, xact_errcnt= %d, babble= %d, underflow= %d, discard= %d, errdiscard= %d", 
											usbh_uvc_mw.stream_id,
											usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].xact_errcnt,
											usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt,
											usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].underflowcnt,
											usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].discardcnt,
											usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errdiscardcnt);
			
			err_event = 0;
			
			goto NEXT;
		}
		
		if(usbh_uvc_mw.size > 12){
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt	++;
			
			//printf("f=%d, %d\r\n", usbh_uvc_mw.stream_id, usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt);
			//printf("f=%d, %d\r\n", usbh_uvc_mw.stream_id, usbh_uvc_mw.size);
			
			if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].debug_msg){				
					UVC_INFO(" =================================");
					UVC_INFO(" usbh_uvc_dev.id = %d",usbh_uvc_mw.dev_id);					
					UVC_INFO(" usbh_uvc_stream.id = %d",usbh_uvc_mw.stream_id);
					UVC_INFO(" Total FrameCNT = %d",usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt);					
					UVC_INFO(" usbh_uvc_mw.ptr = %08x",(uint32_t)usbh_uvc_mw.ptr);
					UVC_INFO(" usbh_uvc_mw.size = %d",usbh_uvc_mw.size);
					UVC_INFO(" usbh_uvc_mw.ring_buff_end= %x",usbh_uvc_mw.ring_buff_end);
					UVC_INFO(" FW = %x",USBH_DEV[0].EP[1].pSXITD->FW_FRAME_END);                      
					UVC_INFO(" HW = %x",USBH_DEV[0].EP[1].pSXITD->HW_FRAME_END);	                    
					UVC_INFO(" HC = %x",USBH_DEV[0].EP[1].pSXITD->HW_CURRENT_POINTER);                    				                 			
				}
			
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].usb_preview){
					if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isH264) {
						if (usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame == 0) {
								currentFrameType = snx_avc_get_slice_type((unsigned char*)usbh_uvc_mw.ptr, usbh_uvc_mw.size);
								if (currentFrameType == TYPE_I) {
									usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame = 1;
								}		
						}
						if (usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame) {
#if defined (CONFIG_USBH_CLI_SUPPORT_USBD_PREV)
							usbd_uvc_drv_send_image(usbh_uvc_mw.ptr ,usbh_uvc_mw.size, UVC_IMG_WITH_HEADER, UVC_XFR_COPY_POINTER);
#endif 							
						}
					} else {
#if defined (CONFIG_USBH_CLI_SUPPORT_USBD_PREV)
						usbd_uvc_drv_send_image(usbh_uvc_mw.ptr ,usbh_uvc_mw.size, UVC_IMG_WITH_HEADER, UVC_XFR_COPY_POINTER);
#endif
					}					
				}
			}else{
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].debug_msg){
						UVC_INFO(" =================================");
						UVC_INFO(" usbh_uvc_dev.id = %d",usbh_uvc_mw.dev_id);						
						UVC_INFO(" usbh_uvc_mw.stream_id = %d",usbh_uvc_mw.stream_id);
						UVC_INFO(" Total FrameCNT = %d",usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt);	
						UVC_INFO(" usbh_uvc_mw.ptr = %08x",(uint32_t)usbh_uvc_mw.ptr);
						UVC_INFO(" usbh_uvc_mw.size = %d",usbh_uvc_mw.size);
						UVC_INFO(" usbh_uvc_mw.ring_buff_end= %x",usbh_uvc_mw.ring_buff_end);
				}
				UVC_INFO("%d:SKIP FRAME", usbh_uvc_mw.stream_id);
					
				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame = 0;
			}
#if defined(CONFIG_SNX_ISO_ACCELERATOR)
			if(usbh_uvc_mw.stream_xfr_type == USBH_ISO_IN_TYPE) {
				uvc_stream_complete(&usbh_uvc_mw);
			}
#endif
			else if(usbh_uvc_mw.stream_xfr_type == USBH_BK_IN_TYPE) {
				uvc_clean_bk_buf();
			}
	}
}

uint8_t uvc_open_stream(USBH_Device_Structure *uvc_dev, uint8_t device_id, uint8_t stream_idx) {
	uint8_t				uvc_fmt = 0, uvc_res = 0, intf_start_idx = 0;
	uint32_t			uvc_fps = 0, temp_iso_size = 0, temp_bk_size = 0;
	
	if (uvc_dev->CLASS_DRV == USBH_UVC_ISO_CLASS) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "h264", 4) == 0){
			usbh_uvc_app.dev[device_id].stream[stream_idx].size = H264FrameSize;
		}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "mjpeg", 5) == 0){
			usbh_uvc_app.dev[device_id].stream[stream_idx].size = MJPEGFrameSize;
		}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "yuv", 3) == 0){
			usbh_uvc_app.dev[device_id].stream[stream_idx].size = YUVFrameSize;
		} else {
			UVC_INFO("Format Not Support!");
			return FAIL;
		}
#else
		usbh_uvc_app.dev[device_id].stream[stream_idx].size			=	((Standard_iTD_EP_Max_Count*3072*8)+(Max_STD_ISO_FrameSize*2)+4096);
		//usbh_uvc_app.dev[dev_id].stream[i].size		=	((Standard_iTD_EP_Max_Count*1024*8)+4096);
#endif

#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr 		= 	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size);				
		}while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);	
#endif

	} else if (uvc_dev->CLASS_DRV == USBH_UVC_BULK_CLASS) {
		usbh_uvc_app.dev[device_id].stream[stream_idx].size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;

#if defined( CONFIG_SN_GCC_SDK )
		do {
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = (uint32_t*) pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size, GFP_DMA, MODULE_DRI_USBH);
		} while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size);
		} while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif

	} else if (uvc_dev->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "h264", 4) == 0){
			temp_iso_size = H264FrameSize;
		}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "mjpeg", 5) == 0){
			temp_iso_size = MJPEGFrameSize;
		}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "yuv", 3) == 0){
			temp_iso_size = YUVFrameSize;
		} else {
			UVC_INFO("Format Not Support!");
			return FAIL;
		}
#else
		temp_iso_size		=	((Standard_iTD_EP_Max_Count*3072*8)+(Max_STD_ISO_FrameSize*2)+4096);
#endif
		
		temp_bk_size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;
		
		(temp_iso_size > temp_bk_size) ? (usbh_uvc_app.dev[device_id].stream[stream_idx].size = temp_iso_size) : 
		(usbh_uvc_app.dev[device_id].stream[stream_idx].size = temp_bk_size);
		
#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr 		= 	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size);
			UVC_INFO("usbh_uvc_app.dev[%d].stream[%d].ptr = %x", device_id, stream_idx, (uint32_t)usbh_uvc_app.dev[device_id].stream[stream_idx].ptr); 				
		}while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);	
#endif
	}
	
	if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "h264", 4) == 0){
		uvc_fmt = USBH_UVC_STREAM_H264;
	}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "mjpeg", 5) == 0){
		uvc_fmt = USBH_UVC_STREAM_MJPEG;
	}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "yuv", 3) == 0){	
		uvc_fmt = USBH_UVC_STREAM_YUV;
	} else {
		UVC_INFO("Format Not Support!");
		goto ERR;
	}
	
	if( (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "1920", 4) == 0) && (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "1080", 4) == 0) ){
		uvc_res = USBH_UVC_STREAM_1920X1080;
	}else if( (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "1280", 4) == 0) && (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "720", 3) == 0) ){
		uvc_res = USBH_UVC_STREAM_1280X720;			
	}else if( (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "640", 3) == 0) && (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "480", 3) == 0) ){
		uvc_res = USBH_UVC_STREAM_640X480;			
	} else if( (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "320", 3) == 0) && (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "240", 3) == 0) ) {
		uvc_res	= USBH_UVC_STREAM_320X240;
	} else if( (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "160", 3) == 0) && (strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "120", 3) == 0) ) {
		uvc_res	= USBH_UVC_STREAM_160X120;
	}else{
		UVC_INFO("Resolution Not Support!"); 
		goto ERR;
	}

	if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "30", 2) == 0){
		uvc_fps = USBH_UVC_STREAM_30_FPS;
	}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "20", 2) == 0){	
		uvc_fps = USBH_UVC_STREAM_20_FPS;
	}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "15", 2) == 0){
		uvc_fps = USBH_UVC_STREAM_15_FPS;
	}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "10", 2) == 0){
		uvc_fps = USBH_UVC_STREAM_10_FPS;
	}else if(strncmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "5", 1) == 0){
		uvc_fps = USBH_UVC_STREAM_5_FPS;				
	}else{
		UVC_INFO("FPS Not Support!"); 
		goto ERR;
	}			
	
	usbh_uvc_app.dev[device_id].stream[stream_idx].stream_id = uvc_start(uvc_dev, uvc_fmt, uvc_res, uvc_fps,usbh_uvc_app.dev[device_id].stream[stream_idx].ptr, usbh_uvc_app.dev[device_id].stream[stream_idx].size, intf_start_idx);			
	UVC_INFO("The Stream ID is	: 0x%x",usbh_uvc_app.dev[device_id].stream[stream_idx].stream_id);
	
	if(usbh_uvc_app.dev[device_id].stream[stream_idx].stream_id != 0)
		return SUCCESS;
	
ERR:
	if(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr != NULL){
		vPortFree(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr);
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
	}
	
	return FAIL;
}

static BaseType_t prvUSBHUVCStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter[6];
	BaseType_t 	xParameterStringLength[6];
	
	uint8_t status = FAIL;
	USBH_Device_Structure 			*uvc_dev = NULL;
	uint8_t i = 0, device_id = 0, stream_idx = 0, temp_device_id = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	if(cli_uvc_inited == 0) {
		memset((void *)&usbh_uvc_app, 0, sizeof(USBH_UVC_APP_STRUCTURE));
		
		usbh_uvc_app.usb_preview_enable = 1;
		usbh_uvc_app.debug_enable				= 1;
		
		cli_uvc_inited = 1;
	}
	
	if(usbh_queue_uvc_app == NULL){
		usbh_queue_uvc_app = xQueueCreate(1, 4);
	}
	
	if(usbh_queue_uvc_mw == NULL) {
		usbh_queue_uvc_mw = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(USBH_UVC_MW_STRUCTURE));
	}
	
	if(usbh_inited == 0) {
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_HOST_PHY1_DEV)	
	usbh_freertos_init();
#endif
#if defined( CONFIG_DUAL_HOST )
#if defined (CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined (CONFIG_USBH_PHY0_DEV_PHY1_HOST)
	usbh_freertos_init_2();
#endif
#endif
		usbh_inited = 1;
	}
	
	/* Obtain the parameter */
	for(i = 0; i < 6; i++) {
		pcParameter[i] = FreeRTOS_CLIGetParameter(
												pcCommandString,						/* The command string itself. */
												i+1,													/* Return the first parameter. */
												&xParameterStringLength[i]	/* Store the parameter string length. */
		);
	
		
		configASSERT( pcParameter[i] );
	}

	device_id = atoi(pcParameter[0]);
	uvc_dev = (USBH_Device_Structure*)usbh_uvc_init(device_id);
	if(uvc_dev == NULL) {
		UVC_INFO("waiting device id: %d to be ready!!", device_id);
		
		xQueueReceive(usbh_queue_uvc_app, &temp_device_id, USBH_10000ms);
		
		uvc_dev = (USBH_Device_Structure*)usbh_uvc_init(device_id);
		if(uvc_dev == NULL)
			goto dev_hint;
		
		UVC_INFO("device id: %d is ready!!", device_id);
	}
	
	for(i = 0; i < max_stream_count; i++) {
		if(usbh_uvc_app.dev[device_id].stream[i].isOpen == 0) {
			stream_idx = i;
			break;
		}
	}
	
	if(i == max_stream_count)
		goto err_hint;

#if defined (CONFIG_USBH_CLI_SUPPORT_USBD_PREV)
	if((strncmp(pcParameter[5], "PREVIEW", 7) == 0) || (strncmp(pcParameter[5], "preview", 7) == 0)){
		for(i = 0; i < max_stream_count; i++) {
			if(usbh_uvc_app.dev[device_id].stream[i].usb_preview)
				break;
		}
		
		if(i == max_stream_count) {
			usbh_uvc_app.dev[device_id].stream[stream_idx].usb_preview = 1;
			
			if(!usbd_inited) {
				snx_usbd_mode = USBD_MODE_UVC;
				snx_usbd_submode= USBD_SUBMODE_BULK;
				
				usbd_drv_task_init();
				usbd_inited = 1;
			}
		} else {
			UVC_INFO("Another stream previewing!");
		}	
	}
#endif
	
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].fmt,		pcParameter[1],	8);
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].width,	pcParameter[2],	8);
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].height,pcParameter[3],	8);
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].fps,		pcParameter[4],	4);
		
	if((strncmp(pcParameter[1], "H264", 4) == 0) || (strncmp(pcParameter[1], "h264", 4) == 0)){
		usbh_uvc_app.dev[device_id].stream[stream_idx].isH264 = 1;
	}
	
	status = uvc_open_stream(uvc_dev, device_id, stream_idx);
	
	if(status == FAIL) {
		memset(&usbh_uvc_app.dev[device_id].stream[stream_idx], 0, sizeof(USBH_UVC_MW_STRUCTURE));
		
		goto err_hint;
	}
	
	usbh_uvc_app.dev[device_id].stream[stream_idx].isOpen = 1;

	if(xTASK_HDL_UVC_STREAM_XFR == NULL){
		xTaskCreate(
			uvc_stream_xfr_task,
			( const	char * )"USBH_UVC_STERAM_XFR_TASK",
			2048,
			(void*)&usbh_uvc_app,
			3,
			&xTASK_HDL_UVC_STREAM_XFR
		);
	}
	
	return pdFALSE;
	
dev_hint:
	UVC_INFO("\r\rn ERROR : Device Not Exist!");
	return pdFALSE;
	
err_hint:
	UVC_INFO("\r\n ERROR : Cannot Open UVC Stream!");
	return pdFALSE;
}

static BaseType_t prvUSBHUVCStopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	
	USBH_Device_Structure			*uvc_dev = NULL;
	uint8_t device_id = 0, stream_id = 0, stream_idx = 0;
	uint8_t status = FAIL;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	/* Obtain the parameter */
	pcParameter = FreeRTOS_CLIGetParameter(
												pcCommandString,						/* The command string itself. */
												1,													/* Return the first parameter. */
												&xParameterStringLength	/* Store the parameter string length. */
								);
								
	configASSERT( pcParameter );
	
	stream_id = atoi(pcParameter);
	
	device_id = uvc_streamid_to_devid(stream_id);
	if(device_id == 0xFF)
		goto err_hint;
	
	uvc_dev	= (USBH_Device_Structure*)usbh_uvc_init(device_id);
	if((uvc_dev != NULL) && (stream_id > 0)){
		status = uvc_stop(uvc_dev, stream_id);
		if(status == FAIL)
			goto err_hint;
	} else {
		goto err_hint;
	}
	
	uvc_unregister_streamid(uvc_dev, stream_id);
	
	stream_idx = streamid_to_streamidx(device_id, stream_id);

	if(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr != NULL){
		vPortFree(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr);
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
	}

	UVC_INFO("\r\nTotal Frame count = %d", usbh_uvc_app.dev[device_id].stream[stream_idx].framecnt);

	memset(&usbh_uvc_app.dev[device_id].stream[stream_idx], 0, sizeof(USBH_UVC_APP_STREAM_STRUCTURE));
	
	return pdFALSE;
	
err_hint:
	UVC_INFO("\r\n Error : Cannot Stop UVC Stream !");
	return pdFALSE;
}

#endif // end of #if defined (CONFIG_USBH_CLI_UVC)
