/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_jpeg.h
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_jpeg_H_wXJ8KjcZ_l808_HwVT_ss1P_ue8YSBvGSSB2__
#define __snc_jpeg_H_wXJ8KjcZ_l808_HwVT_ss1P_ue8YSBvGSSB2__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
#include "snc_csc.h"
#include "snc_idma.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_DQT_SIZE            128

typedef enum jpeg_slice_sel
{
    JPEG_SLICE_INPUT,
    JPEG_SLICE_OUTPUT,
} jpeg_slice_sel_t;

typedef enum jpeg_process_status
{
    JPEG_IDMA_ERROR =   -3,
    JPEG_CB_ERROR =     -2,
    JPEG_PROC_ERROR =   -1,
    JPEG_PROC_SUCCESS = 0,
    JPEG_PROC_IDLE =    1,
    JPEG_PROC_PAUSE =   3,
} jpeg_process_status_t;

typedef enum jpeg_yuv_format
{
    JPEG_YCBCR_420 =      0,
    JPEG_YCBCR_422 =      1,
} jpeg_yuv_format_t;

typedef enum jpeg_slice_format
{
    JPEG_SLICE_422 =     8,
    JPEG_SLICE_420 =     16,
} jpeg_slice_format_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct jpeg_header_info
{
    uint32_t    sof0_addr;          /* jpeg header 0xFFC0 tag adr */
    uint32_t    sos_addr;           /* jpeg header 0xFFDA tag adr */
    uint32_t    dqt_addr;           /* jpeg header 0xFFDB tag adr */
    uint32_t    input_buf_addr;     /* user input, buffer address for user to load jpeg data */
    uint32_t    input_buf_len;      /* user input, buufer length for user to load jpeg data */
    uint32_t    frame_w;            /* jpeg frame width */
    uint32_t    frame_h;            /* jpeg frame height */
    uint32_t    qtable;             /* jpeg q table buffer address total JPG_DQT_SIZE bytes */
    uint32_t    header_length;      /* header total length */
    uint8_t     skip_byte;          /* get value by function JPEG_Header_Check,
                                       check jpeg bistream start address align 4 bytes */
    jpeg_yuv_format_t jpeg_fmt;     /* yuv format 420/422 */
} jpeg_header_info_t;

typedef struct jpeg_decoder_handle_info
{
    uint32_t    input_buf;          /* user input, buffer address for decoder to load jpeg data, support extra ram */
    uint32_t    input_buf_len;      /* buffer length for decoder to load jpeg bistream data, suggest 0x200~0x400 */
    uint32_t    output_buf;         /* user input, buffer address for decoder to output jpeg data */
    uint32_t    output_buf_len;     /* get value by function JPEG_DEC_Get_Output_Buf_Len,
                                       buffer length for decoder to output data,
                                       sueggest length
                                       frame width * slice (420:16, 422:8) * one pixel n bytes (in 420 mode n=:1.5, else n=2) */
    uint32_t    set_qtable_gain;    /* user input, set q table gain value, range 0~128 */
    bool    en_irq;                 /* user input, enable irq or not */
    idma_burst_t   idma_burst;      /* user input, enum jpeg_idma_burst_t */
} jpeg_decoder_handle_info_t;

typedef struct jpeg_encoder_handle_info
{
    uint32_t    input_buf;          /* user input, buffer address for encoder to load jpeg data */
    uint32_t    input_buf_len;      /* get value by function JPEG_ENC_Get_Input_Buf_Len,
                                       buffer length for decoder to load jpeg data,
                                       sueggest length 
                                       frame width * slice (420:16, 422:8) * one pixel n bytes (in 420 mode n=:1.5, else n=2) */
    uint32_t    output_buf;         /* user input, buffer address for decoder to output jpeg data, support extra ram */
    uint32_t    output_buf_len;     /* buffer length for encoder to output jpeg data, suggest 0x200~0x400 */
    uint32_t    jpeg_total_size;    /* get value by function Jpeg_Enc_Proc,
                                       encoder output total data size */
    uint32_t    set_qtable_gain;    /* user input, set q table gain value, range 0~128 */
    uint32_t    ac_reduce;          /* user input, Use to reduce bitstream data, range 0~d63*/
    uint32_t    set_stuff_bit;      /* user input, Fill data until 8bit at last byte in encode data 0/1 */
    bool    en_irq;                 /* user input, enable irq or not */
    idma_burst_t   idma_burst;      /* user input, enum idma_burst_t */
} jpeg_encoder_handle_info_t;

typedef struct cb_jpeg_info
{
    uint32_t buffer_addr;
    uint32_t len;
    void *ptunnel_info;
}cb_jpeg_info_t;

typedef int (*CB_JPEG_EMPTY_BUF)(cb_jpeg_info_t* cb_info);
typedef int (*CB_JPEG_FILL_BUF)(cb_jpeg_info_t* cb_info);

typedef struct jpg_io_desc
{
    // buffer size MUST more then 512 bytes
    int (*cb_get_buf)(uint8_t **ppBuf, uint32_t *pBuf_size);
    int (*cb_fill_buf)(uint8_t *pSrc_buf, uint32_t request_size, uint32_t *pReal_size, void *pUser_info);

} jpg_io_desc_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int JPEG_Header_Parsing(jpeg_header_info_t  *pJHdr,jpg_io_desc_t *pIO_desc,void *pUser_info);

int JPEG_Header_Update(jpeg_header_info_t *header, uint32_t bitstream_addr, uint32_t size);
int JPEG_Header_Check(jpeg_header_info_t *header, CB_JPEG_FILL_BUF cb_fill_data, uint32_t file_size, void *pUser_info);

uint32_t JPEG_ENC_Get_Input_Buf_Len(jpeg_header_info_t *jpeg_header);
int JPEG_ENC_Set_Buffer(jpeg_encoder_handle_info_t *jpeg_encoder, jpeg_header_info_t *jpeg_header, uint8_t *pinbuf, uint32_t inbuf_len);
int JPEG_ENC_Init(jpeg_encoder_handle_info_t *jpeg_encoder, jpeg_header_info_t *jpeg_header);
jpeg_process_status_t JPEG_ENC_Proc(jpeg_encoder_handle_info_t *jpeg_encoder, CB_JPEG_FILL_BUF cb_fill_buf, CB_JPEG_EMPTY_BUF cb_empty_buf, void *pUser_info);
void JPEG_ENC_Proc_Pause(void);
void JPEG_ENC_Proc_Resume(void);
int JPEG_ENC_Deinit(void);
void JPEG_ENC_Set_Slice_Done(jpeg_slice_sel_t idma_sel);

uint32_t JPEG_DEC_Get_Output_Buf_Len(jpeg_header_info_t *jpeg_header);
int JPEG_DEC_Set_Buffer(jpeg_decoder_handle_info_t *jpeg_decoder, jpeg_header_info_t *jpeg_header, uint8_t *poutbuf, uint32_t outbuf_len);
int JPEG_DEC_Init(jpeg_decoder_handle_info_t *jpeg_decoder, jpeg_header_info_t *jpeg_header);
jpeg_process_status_t JPEG_DEC_Proc(jpeg_decoder_handle_info_t *jpeg_decoder, CB_JPEG_FILL_BUF cb_fill_buf, CB_JPEG_EMPTY_BUF cb_empty_buf, void *pUser_info, csc_handle_info_t *csc);
void JPEG_DEC_Proc_Pause(void);
void JPEG_DEC_Proc_Resume(void);
int JPEG_DEC_Deinit(void);
void JPEG_DEC_Set_Slice_Done(jpeg_slice_sel_t idma_sel);
void JPEG_DEC_Set_Slice_Start(void);

uint32_t JPEG_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
