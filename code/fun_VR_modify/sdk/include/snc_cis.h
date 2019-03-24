/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_cis.h
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_cis_H_wRFYKH18_lPyp_HPzN_ssQW_uMqhIq1xc3pt__
#define __snc_cis_H_wRFYKH18_lPyp_HPzN_ssQW_uMqhIq1xc3pt__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_idma.h"
#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum cis_divclk
{
    CIS_CLK_DIV_2 = 1,                      /* system clock div 2 */
    CIS_CLK_DIV_4,                          /* system clock div 4 */
    CIS_CLK_DIV_8,                          /* system clock div 8 */
} cis_divclk_t;

typedef enum cis_edge
{
    CIS_RISING_TRIG,                             /* rising edge trigger */
    CIS_FALLING_TRIG,                            /* falling edge trigger */
} cis_edge_t;

typedef enum cis_level
{
    CIS_HIGH_LEVEL,                         /* rising edge trigger */
    CIS_LOW_LEVEL,                          /* falling edge trigger */
} cis_level_t;

typedef enum cis_line
{
    CIS_LINE_0,                             /* select line 0 */
    CIS_LINE_1,                             /* select line 1 */
    CIS_LINE_2,                             /* select line 2 */
    CIS_LINE_ALL,                           /* select all line */
} cis_line_t;

typedef enum cis_data_fmt
{
    CIS_RGB,                                /* sensor output RGB, 7320 bypass to output buffer */
    CIS_YUV,                                /* sensor output YUV, 7320 will convert to RGB domain and output to buffer */
} cis_data_fmt_t;

typedef enum cis_pixel_fmt
{
    CIS_PER_2_CLK_1_PIXEL,                  /* two pixel clock get one pixel data */
    CIS_PER_1_CLK_1_PIXEL,                  /* one pixel clock get one pixel data */
} cis_pixel_fmt_t;

typedef enum cis_yuv_fmt
{
    CIS_YCBCR_420,                           /* yuv 420 */
    CIS_YCBCR_422,                           /* yuv 422 */
} cis_yuv_fmt_t;

typedef enum cis_scal_line
{
    CIS_START_LINE_0,                       /* start from first line */
    CIS_START_LINE_1,                       /* start from second line */
} cis_scal_line_t;

typedef enum cis_scal_skipline
{
    CIS_NO_SKIP_LINE,                       /* means height div 1 */
    CIS_TWO_LINE_TO_ONE,                    /* means height div 2 */
    CIS_FOUR_LINE_TO_ONE = 3,               /* means height div 4 */
} cis_scal_skipline_t;

typedef enum _cis_scal_pixel
{
    CIS_START_PIXEL_0,                      /* start from first pixel */
    CIS_START_PIXEL_1,                      /* start from second pixel */
} cis_scal_pixel_t;

typedef enum cis_scal_skippixel
{
    CIS_NO_SKIP_PIXEL,                      /* means width div 1 */
    CIS_TWO_PIXEL_TO_ONE,                   /* means width div 2 */
    CIS_FOUR_PIXEL_TO_ONE = 3,              /* means width div 4 */
} cis_scal_skippixel_t;

typedef enum cis_irq_status
{
    CIS_LINE_BUF_FULL,                      /* cis line buffer full */
    CIS_FRAME_DONE,                         /* cis frame done */
} cis_irq_status_t;

typedef enum cis_init_status
{
    CIS_INIT_IDMA_ERROR = -5,
    CIS_INIT_SET_REG_ERROR = -4,
    CIS_INIT_SCALE_ERROR = -3,
    CIS_INIT_WINDOW_ERROR = -2,
    CIS_INIT_ERROR =  -1,
    CIS_INIT_SUCCESS = 0,
} cis_init_status_t;

typedef enum cis_process_status
{
    CIS_CB_ERROR =      -2,
    CIS_PROC_ERROR =    -1,
    CIS_PROC_SUCCESS =  0,
} cis_process_status_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct l2b_mode
{
    bool enable;                            /* user input, enable line to block mode */
    cis_yuv_fmt_t fmt;                      /* user input, enum yuv_fmt_t */
    uint32_t skipline;                      /* user input, skip line in line to block mode, suggest 0 */
} l2b_mode_t;

typedef struct window_mode
{
    bool enable;                            /* user input, enable windowing mode */
    uint32_t w_offset;                      /* user input, the offset value 0 < w_offset < Frame_w */
    uint32_t h_offset;                      /* user input, the offset value 0 < h_offset < Frame_h */
    uint32_t window_width;                  /* user input, the window_width + w_offset < CIS frame width */
    uint32_t window_height;                 /* user input, the window_height + h_offset < CIS Frame height */
} window_mode_t;

typedef struct scal_mode
{
    bool enable;                            /* user input, enable scaling mode */
    cis_scal_line_t start_line_sel;         /* user input, enum cis_scal_line_t */
    cis_scal_skipline_t line_skip;          /* user input, enum cis_scal_skipline_t */
    cis_scal_pixel_t start_pixel_sel;       /* user input, enum cis_scal_pixel_t */
    cis_scal_skippixel_t pixel_skip;        /* user input, enum cis_scal_skippixel_t */
} scal_mode_t;

typedef struct cis_handle_info
{
    uint32_t frame_w;                       /* user input, cis frame width */
    uint32_t frame_h;                       /* user input, cis frame height */
    uint32_t line_buf_addr[3];              /* Get value by function CIS_Set_Buffer, buffer address for CIS IDMA to output CIS data */
    uint32_t buf_len;                       /* Get value by function CIS_Set_Buffer, buffer length for CIS IDMA to output CIS data,
                                               sueggest length :
                                               in normal, frame_w x 2 bytes per line
                                               in l2b mode only use line0, 
                                               frame_w x slice (420:16, 422:8) x one pixel n bytes (in 420 mode n=:1.5, else n=2)*/
    window_mode_t window;                   /* user input, struct window_mode_t */
    scal_mode_t scaling;                    /* user input, struct scal_mode_t */
    l2b_mode_t l2b;                         /* user input, struct l2b_mode_t, should use in yuv mode */
    cis_divclk_t cisdivclk;                 /* user input, enum cis_divclk_t */
    cis_pixel_fmt_t pixel_fmt;              /* user input, enum cis_pixel_fmt_t */
    cis_edge_t data_latch_edge;             /* user input, enum cis_edge_t */
    cis_level_t hsync_valid_level;          /* user input, enum cis_level_t */
    cis_level_t vsync_valid_level;          /* user input, enum cis_level_t */
    cis_data_fmt_t data_fmt;                /* user input, enum cis_data_fmt_t */
    idma_burst_t idma_burst;                /* user input, enum idma_burst_t */
    bool remove_transparent;                /* user input, remove transparent, if the data is 0xFF, it will be set 0xFE */
} cis_handle_info_t;

typedef struct cb_cis_oid_info
{
    uint32_t buffer_addr;
    uint32_t len;
    void *ptunnel_info;
}cb_cis_info_t;

typedef int (*CB_CIS_REG_SET)(void);
typedef int (*CB_CIS_EMPTY_BUF)(uint32_t line_cnt, cb_cis_info_t* cb_info);

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uint32_t CIS_Get_Buf_Len(cis_handle_info_t *cis_info);
int CIS_Set_Buffer(cis_handle_info_t *cis_info, uint8_t *pbuf, uint32_t buf_len);
cis_init_status_t CIS_Init(cis_handle_info_t *cis_info, CB_CIS_REG_SET cb_cis_reg_set);
cis_process_status_t CIS_Idma_Irq_Proc(CB_CIS_EMPTY_BUF cb_empty_buf, void* user_info);
cis_process_status_t CIS_Vsync_Irq_Proc(void);
int CIS_Deinit(void);

uint32_t CIS_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
