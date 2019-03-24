/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_csc.h
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_csc_H_wXJ8KjcZ_l808_HwVT_ss1P_ue8YSBvGSSB2__
#define __snc_csc_H_wXJ8KjcZ_l808_HwVT_ss1P_ue8YSBvGSSB2__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum csc_mode
{
    CSC_Y2R =       2,
    CSC_R2Y =       3,
} csc_mode_t;

typedef enum csc_yuv_format
{
    CSC_YCBCR_420 =      0,
    CSC_YCBCR_422 =      1,
} csc_yuv_format_t;

typedef enum slice_format
{
    CSC_SLICE_422 =     8,
    CSC_SLICE_420 =     16,
} slice_format_t;

typedef enum csc_slice_sel
{
    CSC_SLICE_INPUT,
    CSC_SLICE_OUTPUT,
} csc_slice_sel_t;

typedef enum csc_process_status
{
    CSC_IDMA_ERROR =    -3,
    CSC_CB_ERROR =      -2,
    CSC_PROC_ERROR =    -1,
    CSC_PROC_SUCCESS =  0,
    CSC_PROC_IDLE =     1,
    CSC_PROC_PAUSE =    3,
} csc_process_status_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct csc_handle_info   //
{
    uint32_t input_buf;         /* user input, buffer address for CSC to load jpeg data */
    uint32_t input_buf_len;     /* get value by function CSC_Get_Input_Buf_Len, 
                                   buffer length for decoder to output data,
                                   sueggest length frame width * slice (420:16, 422:8) * one pixel 2 bytes */
    uint32_t output_buf;        /* user input, buffer address for CSC to output jpeg data */
    uint32_t output_buf_len;    /* get value by function CSC_Get_Output_Buf_Len, 
                                   buffer length for decoder to output data,
                                   sueggest length frame width * slice (420:16, 422:8) * one pixel n bytes
                                   (in R2Y&420 mode n=1.5, else n=2) */
    uint32_t frame_w;           /* user input, csc frame width */
    uint32_t frame_h;           /* user input, csc frame height */
    bool scal_en;               /* user input, scaling enable */
    uint32_t scal_width;        /* user input, scaling width */
    uint32_t scal_height;       /* user input, scaling height */
    bool dithering;             /* user input, dithering mode */
    bool en_irq;                /* user input, enable irq or not */
    csc_mode_t cvt_mode;        /* user input, enum csc_mode_t */
    csc_yuv_format_t yuv_fmt;   /* user input, enum yuv_format_t */
} csc_handle_info_t;

typedef struct cb_csc_info
{
    uint32_t buffer_addr;
    uint32_t len;
    void *ptunnel_info;
}cb_csc_info_t;

typedef int (*CB_CSC_EMPTY_BUF)(cb_csc_info_t* cb_info);
typedef int (*CB_CSC_FILL_BUF)(cb_csc_info_t* cb_info);

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uint32_t CSC_Get_Output_Buf_Len(csc_handle_info_t *csc);
uint32_t CSC_Get_Input_Buf_Len(csc_handle_info_t *csc);
int CSC_Set_Buffer(csc_handle_info_t *csc, uint8_t *pinbuf, uint32_t in_buf_len, uint8_t *poutbuf, uint32_t out_buf_len);
int CSC_Init(csc_handle_info_t *csc);
csc_process_status_t CSC_Proc(csc_handle_info_t *csc, CB_CSC_FILL_BUF cb_fill_buf, CB_CSC_EMPTY_BUF cb_empty_buf, void *pUser_info);
void CSC_Proc_Pause(void);
void CSC_Proc_Resume(void);
int CSC_Deinit(void);
void CSC_Set_Slice_Done(csc_slice_sel_t idma_sel);
void CSC_Set_Slice_Start(void);
uint32_t CSC_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
