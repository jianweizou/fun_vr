/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file csc.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */

#include <stdio.h>
#include <string.h>
#include "snc_csc.h"
#include "snc_idma.h"
#include "register_7320.h"
#define CSC_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum idma_status
{
    IDMA_IDLE,
    IDMA_BUSY,
} idma_status_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define CSC_YUV422_PIXEL 16 //(CSC_SLICE_422 * 2)
#define CSC_YUV420_PIXEL 24 //(CSC_SLICE_420 * 1.5)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t csc_proc_pause = 0;
static uint32_t csc_total_len;
static uint32_t csc_output_times = 0;
static uint32_t csc_input_times = 0;
static uint32_t csc_proc_loop_times = 0;
static uint32_t csc_output_loop_times = 0;
static uint8_t *csc_input_buffer;
static uint32_t csc_input_buffer_length = 0;
static volatile idma_status_t csc_idma_input_status;
static volatile idma_status_t csc_idma_output_status;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  \brief  CSC Initial
 *
 *  \param [in] yuv_format  yuv data format 420/422
 *  \param [in] encode_width frame width
 *  \param [in] CSC_Mode R2Y/Y2R
 *  \return
 *
 *  \details
 */
static void 
_CSC_Set_Format(
    uint8_t yuv_format, 
    uint16_t encode_width, 
    uint8_t CSC_Mode)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    SN_CSC->CSC_CTRL_b.DMA_Mode = 1; //CSC Transfer By DMA

    if(encode_width!=0)
        SN_CSC->JPEG_LINE_SIZE=(encode_width&0xFFF8);
    else
        SN_CSC->JPEG_LINE_SIZE=320;
        SN_CSC->CSC_CTRL_b.CSC_Mode = CSC_Mode;


    if(yuv_format == 1)
    {
        SN_CSC->CSC_CTRL_b.YCbCr_Mode = 1; //422 format
    }
    else
    {
        SN_CSC->CSC_CTRL_b.YCbCr_Mode = 0; //420 format
    }
}

/**
 *  \brief  CSC control
 *
 *  \param [in] set enum control_t
 *  \return
 *
 *  \details
 */
static void 
_CSC_Enable(
    bool set)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    if(!set)
    {
        IDMA_Stop(IDMA_CH_CSC_IN);
        IDMA_Stop(IDMA_CH_CSC_OUT);
        SN_CSC->CSC_CTRL_b.SCAL_EN = 0;
        SN_CSC->CSC_CTRL_b.SCAL_FS_EN = 0;
        SN_CSC->CSC_CTRL_b.SCAL_FIR_EN = 0;
        SN_CSC->CSC_CTRL_b.SCAL_VSYNC = 0;
        SN_CSC->CSC_CTRL_b.SCAL_VSYNC = 1;
        SN_CSC->CSC_CTRL_b.SCAL_VSYNC = 0;
    }
    
    SN_CSC->CSC_CTRL_b.CSC_EN = set; //Enable CSC
}

/**
 *  \brief  CSC output idma setting
 *
 *  \param [in] wram_addr idma output buffer address
 *  \param [in] len frame idma output buffer length
 *  \return
 *
 *  \details
 */
int 
CSC_Idma_Out(
    uint32_t wram_addr, 
    uint32_t len)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    idma_setting_t idma_csc_output;
    
    if(SN_CSC->CSC_CTRL_b.CSC_Mode == 3) //Encoder
    {
        SN_CSC->CSC_DMA_DATA_OUT_Addr_H = (wram_addr>>16); //H First
        SN_CSC->CSC_DMA_DATA_OUT_Addr_L = wram_addr; //L Later
        IDMA_Extra_Config(IDMA_CH_CSC_OUT, IDMA_CFG_CIS_AD_EN);
        idma_csc_output.ram_addr = IDMA_ADDR_IGNORE;
    }
    else if(SN_CSC->CSC_CTRL_b.CSC_Mode == 2) //Decoder
    {
        IDMA_Extra_Config(IDMA_CH_CSC_OUT, IDMA_CFG_CIS_AD_DIS);
        idma_csc_output.ram_addr = wram_addr;
    }
    
    idma_csc_output.direction = IDMA_DIRECTION_RAM_2_DIP;
    idma_csc_output.dip_addr = 0;
    idma_csc_output.length = len;
    if(IDMA_Start(IDMA_CH_CSC_OUT, &idma_csc_output) != 0)
    {
        return -1;
    }
    return 0;
}

/**
 *  \brief  CSC input idma setting
 *
 *  \param [in] wram_addr   idma input buffer address
 *  \param [in] len         idma input buffer length
 *  \return
 *
 *  \details
 */
int 
CSC_Idma_In(
    uint32_t wram_addr, 
    uint32_t len)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    idma_setting_t idma_csc_input;
    
    if(SN_CSC->CSC_CTRL_b.CSC_Mode == 2) //Decoder
    {
        SN_CSC->CSC_DMA_DATA_IN_Addr_H = (wram_addr>>16); //H First
        SN_CSC->CSC_DMA_DATA_IN_Addr_L = wram_addr; //L Later
        IDMA_Extra_Config(IDMA_CH_CSC_IN, IDMA_CFG_CIS_AD_EN);
        idma_csc_input.ram_addr = IDMA_ADDR_IGNORE;
    }
    else if(SN_CSC->CSC_CTRL_b.CSC_Mode == 3) //Encoder
    {
        IDMA_Extra_Config(IDMA_CH_CSC_IN, IDMA_CFG_CIS_AD_DIS);
        idma_csc_input.ram_addr = wram_addr;
    }
    idma_csc_input.direction = IDMA_DIRECTION_DIP_2_RAM;
    idma_csc_input.dip_addr = 0;
    idma_csc_input.length = len;
    if(IDMA_Start(IDMA_CH_CSC_IN, &idma_csc_input) != 0)
    {
        return -1;
    }
    return 0;
}

/**
 *  \brief  CSC scaling filter set
 *
 *  \param [in] input_w : scaling input width size
 *  \param [in] input_h : scaling input height size
 *  \param [in] output_w : scaling output width size
 *  \param [in] output_h : scaling output height size
 *  \return
 *
 *  \details
 */
static void 
_CSC_Scal_Ratio(
    uint32_t input_w, 
    uint32_t input_h, 
    uint32_t output_w, 
    uint32_t output_h)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    float H_ratio;
    float W_ratio;
    
    H_ratio = 128 / ((float)output_w / (float)input_w);
    W_ratio = 128 / ((float)output_h / (float)input_h);
    
    SN_CSC->SCAL_H_RATIO = (uint32_t)H_ratio;
    SN_CSC->SCAL_V_RATIO = (uint32_t)W_ratio;

    SN_CSC->SCAL_HISIZE = input_w;
    SN_CSC->SCAL_VISIZE = input_h;

    SN_CSC->SCAL_HOSIZE = output_w/2;
    SN_CSC->SCAL_VOSIZE = output_h/2;
}

/**
 *  \brief set csc input/output one slice done
 *
 *  \param [in] idma_sel : enum csc_slice_sel_t
 *  \return
 *
 *  \details
 */
void 
CSC_Set_Slice_Done(
    csc_slice_sel_t idma_sel)
{
    if(idma_sel == CSC_SLICE_INPUT)
    {
        csc_idma_input_status = IDMA_IDLE;
    }
    else if(idma_sel == CSC_SLICE_OUTPUT)
    {
        csc_idma_output_status = IDMA_IDLE;
    }
}

/**
 *  \brief  get csc intput/output idma status
 *
 *  \param [in] idma_sel : enum csc_slice_sel_t
 *  \return csc_idma_input_status : csc idma input/output status
 *
 *  \details
 */
idma_status_t 
CSC_Get_Irq(
    csc_slice_sel_t idma_sel)
{
    if(idma_sel == CSC_SLICE_INPUT)
    {
        return csc_idma_input_status;
    }
    else if(idma_sel == CSC_SLICE_OUTPUT)
    {
        return csc_idma_output_status;
    }
    return csc_idma_input_status;
}

/**
 *  \brief  set csc intput/output start
 *
 *  \param [in] idma_sel : enum csc_slice_sel_t
 *  \return
 *
 *  \details
 */
void 
CSC_Irq_Start(
    csc_slice_sel_t idma_sel)
{
    if(idma_sel == CSC_SLICE_INPUT)
    {
        csc_idma_input_status = IDMA_BUSY;
    }
    else if(idma_sel == CSC_SLICE_OUTPUT)
    {
        csc_idma_output_status = IDMA_BUSY;
    }
}

/**
 *  \brief  set csc intput/output slice start
 *
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \return
 *
 *  \details
 */
void 
CSC_Set_Slice_Start(void)
{
    CSC_Idma_In((uint32_t)csc_input_buffer, csc_input_buffer_length);
}

/**
 *  \brief  get csc idma input buffer length
 *
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \return input buffer length
 *
 *  \details calculate intput buffer length
 */
uint32_t 
CSC_Get_Input_Buf_Len(
    csc_handle_info_t *csc)
{
    uint32_t align_w = 0;
    do
    {
        if(csc->frame_h <= 0 || csc->frame_w <= 0)
        {
            break;
        }
        if(csc->cvt_mode == CSC_R2Y)
        {
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                align_w = (csc->frame_w+7) & ~7;
                return align_w * CSC_YUV422_PIXEL;
            }
            else if(csc->yuv_fmt == CSC_YCBCR_420)
            {
                align_w = (csc->frame_w+15) & ~15;
                return align_w * CSC_SLICE_420 * 2;
            }
        }
        else if(csc->cvt_mode == CSC_Y2R)
        {
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                align_w = (csc->frame_w+7) & ~7;
                return align_w * CSC_YUV422_PIXEL;
            }
            else if(csc->yuv_fmt == CSC_YCBCR_420)
            {
                align_w = (csc->frame_w+15) & ~15;
                return align_w * CSC_SLICE_420 * 2;
            }
        }
    }while(0);
    memset((void *)csc, 0x0, sizeof(csc));
    return 0;
}

/**
 *  \brief  get csc idma output buffer length
 *
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \return output buffer length
 *
 *  \details calculate output buffer length
 */
uint32_t 
CSC_Get_Output_Buf_Len(
    csc_handle_info_t *csc)
{
    uint32_t align_w = 0;
    do
    {
        if(csc->frame_h <= 0 || csc->frame_w <= 0)
        {
            break;
        }
        if(csc->cvt_mode == CSC_R2Y)
        {
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                align_w = (csc->frame_w+7) & ~7;
                return align_w * CSC_YUV422_PIXEL;
            }
            else if(csc->yuv_fmt == CSC_YCBCR_420)
            {
                align_w = (csc->frame_w+15) & ~15;
                return align_w * CSC_YUV420_PIXEL;
            }
        }
        else if(csc->cvt_mode == CSC_Y2R)
        {
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                align_w = (csc->frame_w+7) & ~7;
                if(csc->scal_height > 0 && csc->scal_width > 0 && csc->scal_en)
                {
                    return csc->scal_width * CSC_YUV422_PIXEL;
                }
                return align_w * CSC_YUV422_PIXEL;
            }
            else if(csc->yuv_fmt == CSC_YCBCR_420)
            {
                align_w = (csc->frame_w+15) & ~15;
                if(csc->scal_height > 0 && csc->scal_width > 0 && csc->scal_en)
                {
                    return csc->scal_width * CSC_SLICE_420 * 2;
                }
                return align_w * CSC_SLICE_420 * 2;
                
            }
        }
    }while(0);
    memset((void *)csc, 0x0, sizeof(csc));
    return 0;
}

/**
 *  \brief  Csc Set buffer length
 *
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \param [in] pinbuf : a point to a input buffer
 *  \param [in] in_buf_len : input buffer length
 *  \param [in] poutbuf : a point to a output buffer
 *  \param [in] out_buf_len : output buffer length
 *  \return -1 : Set buffer length fail
 *           0 : Set buffer length success
 *
 *  \details
 */
int 
CSC_Set_Buffer(
    csc_handle_info_t *csc, 
    uint8_t *pinbuf,
    uint32_t in_buf_len,
    uint8_t *poutbuf,
    uint32_t out_buf_len)
{
    uint32_t align_w = 0;
    uint32_t align_scal_w = 0;
    do
    {
        if(csc->frame_h <= 0 || csc->frame_w <= 0)
        {
            break;
        }
        
        if(csc->cvt_mode == CSC_R2Y)
        {
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                if(csc->frame_w * CSC_SLICE_422 * 2 <= in_buf_len && csc->frame_w * CSC_SLICE_422 * 2 <= out_buf_len)
                {
                    csc->output_buf_len = csc->frame_w * CSC_YUV422_PIXEL;
                    csc->input_buf_len = csc->frame_w * CSC_YUV422_PIXEL;
                }
                else
                {
                    break;
                }
            }
            else if(csc->yuv_fmt == CSC_YCBCR_420)
            {
                if(csc->frame_w * CSC_YUV422_PIXEL <= in_buf_len && csc->frame_w * CSC_YUV420_PIXEL <= out_buf_len)
                {
                    csc->output_buf_len = csc->frame_w * CSC_YUV420_PIXEL;
                    csc->input_buf_len = csc->frame_w * CSC_SLICE_420 * 2;
                }
                else
                {
                    break;
                }
            }
            csc_total_len = csc->frame_w * csc->frame_h *2;
        }
        else if(csc->cvt_mode == CSC_Y2R)
        {
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                align_w = (csc->frame_w+7) & ~7;
                if(csc->scal_en)
                {
                    align_scal_w = (csc->scal_width+7) & ~7;
                }
                if(align_w * CSC_SLICE_422 * 2 <= in_buf_len &&
                    (csc->scal_en?align_scal_w * CSC_YUV422_PIXEL:align_w * CSC_YUV422_PIXEL) <= out_buf_len)
                {
                    csc->input_buf_len = align_w * CSC_YUV422_PIXEL;
                    csc->output_buf_len = align_w * CSC_YUV422_PIXEL;
                    if(csc->scal_height > 0 && csc->scal_width > 0 && csc->scal_en)
                    {
                        csc->output_buf_len = csc->scal_width * CSC_YUV422_PIXEL;
                    }
                }
                else
                {
                    break;
                }
            }
            else if(csc->yuv_fmt == CSC_YCBCR_420)
            {
                align_w = (csc->frame_w+15) & ~15;
                if(align_w * CSC_SLICE_420 * 2 <= in_buf_len && 
                    (csc->scal_en?align_scal_w * CSC_YUV420_PIXEL:align_w * CSC_YUV420_PIXEL) <= out_buf_len)
                {
                    csc->output_buf_len = align_w * CSC_SLICE_420 * 2;
                    csc->input_buf_len = align_w * CSC_SLICE_420 * 2;
                    if(csc->scal_height > 0 && csc->scal_width > 0 && csc->scal_en)
                    {
                        csc->output_buf_len = csc->scal_width * CSC_SLICE_420 * 2;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        csc->input_buf = (uint32_t)pinbuf;
        csc->output_buf = (uint32_t)poutbuf;
        csc_input_buffer = pinbuf;
        csc_input_buffer_length = csc->input_buf_len;
        return 0;
    }while(0);

    memset((void *)csc, 0x0, sizeof(csc));
    return -1;
}

/**
 *  \brief  Csc Initial
 *
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \return -1 : initial fail
 *           0 : initial success 
 *
 *  \details
 */
int 
CSC_Init(
    csc_handle_info_t *csc)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    uint32_t align_w = 0;
    csc_input_times = 0;
    csc_output_times = 0;
    csc_idma_input_status = IDMA_IDLE;
    csc_idma_output_status = IDMA_IDLE;
    if(csc->yuv_fmt == CSC_YCBCR_422)
    {
        align_w = (csc->frame_w+7) & ~7;
    }
    else if(csc->yuv_fmt == CSC_YCBCR_420)
    {
        align_w = (csc->frame_w+15) & ~15;
    }
    
    _CSC_Set_Format(csc->yuv_fmt, align_w, csc->cvt_mode);
    
    if(csc->scal_en && csc->cvt_mode == CSC_Y2R)
    {
        SN_CSC->CSC_CTRL_b.SCAL_EN = 1;
        SN_CSC->CSC_CTRL_b.SCAL_FS_EN = 1;
        SN_CSC->CSC_CTRL_b.SCAL_FIR_EN = 1;
        
        SN_CSC->SCAL_FIR_0 = 256;
        SN_CSC->SCAL_FIR_1 = 0;
        SN_CSC->SCAL_FIR_2 = 0;
        SN_CSC->SCAL_FIR_3 = 0;
        SN_CSC->SCAL_FIR_4 = 0;
        
        _CSC_Scal_Ratio(csc->frame_w, csc->frame_h, csc->scal_width, csc->scal_height);
        
        SN_CSC->CSC_CTRL_b.SCAL_VSYNC = 0;
        SN_CSC->CSC_CTRL_b.SCAL_VSYNC = 1;
        SN_CSC->CSC_CTRL_b.SCAL_VSYNC = 0;
        
        if(csc->yuv_fmt == CSC_YCBCR_422)
        {
            csc_output_loop_times = ((csc->scal_height+7) & ~7) / 8;
            csc_proc_loop_times = ((csc->frame_h+7) & ~7) / 8;
        }
        else if(csc->yuv_fmt == CSC_YCBCR_420)
        {
            csc_output_loop_times = ((csc->scal_height+15) & ~15) / 16;
            csc_proc_loop_times = ((csc->frame_h+15) & ~15) / 16;
        }
    }
    else
    {
        if(csc->yuv_fmt == CSC_YCBCR_422)
        {
            csc_proc_loop_times = ((csc->frame_h+7) & ~7) / 8;
            csc_output_loop_times = ((csc->frame_h+7) & ~7) / 8;
        }
        else if(csc->yuv_fmt == CSC_YCBCR_420)
        {
            csc_proc_loop_times = ((csc->frame_h+15) & ~15) / 16;
            csc_output_loop_times = ((csc->frame_h+15) & ~15) / 16;
        }
    }
    
    if(csc->dithering)
    {
        SN_CSC->CSC_CTRL_b.RGB_DI_EN = 1;
        if(csc->scal_en)
        {
            SN_CSC->DI_HSize = csc->scal_width;
            SN_CSC->DI_VSize = csc->scal_height;
            if((csc->frame_w*8/10) == csc->scal_width)
            {
                return -1;
            }
        }
        else
        {
            SN_CSC->DI_HSize = csc->frame_w;
            SN_CSC->DI_VSize = csc->frame_h;
        }
    }
    
    _CSC_Enable(true);
    if(csc->en_irq)
    {
        NVIC_ClearPendingIRQ(CSC_DMA_IN_IRQn);
        NVIC_EnableIRQ(CSC_DMA_IN_IRQn);

        NVIC_ClearPendingIRQ(CSC_DMA_OUT_IRQn);
        NVIC_EnableIRQ(CSC_DMA_OUT_IRQn);
    }
    if(SN_CSC->CSC_CTRL_b.CSC_EN)
    {
        return 0;
    }
    memset((void *)csc, 0x0, sizeof(csc));
    return -1;
}

/**
 *  \brief  CSC_Slice_Start
 *
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \param [in] cb_fill_buf : csc fill buffer callback function
 *  \param [in] cb_empty_buf : csc empty buffer callback function
 *  \param [in] pUser_info : user info input by pass to cb_fill_buf and cb_empty_buf
 *  \return -1 : CSC_PROC_ERROR
 *          0 : CSC_PROC_SUCCESS
 *          1 : CSC_PROC_IDLE
 *          3 : CSC_PROC_PAUSE
 *
 *  \details
 */
csc_process_status_t 
CSC_Proc(
    csc_handle_info_t   *csc, 
    CB_CSC_FILL_BUF     cb_fill_buf,
    CB_CSC_EMPTY_BUF    cb_empty_buf, 
    void                *pUser_info)
{
    uint32_t i;
    csc_process_status_t ret = CSC_PROC_IDLE;
    cb_csc_info_t cb_info;

    while(csc_output_times <= csc_output_loop_times)
    {
        if(csc_input_times > 0)
        {
            if(csc_idma_output_status == IDMA_IDLE)
            {
                if(csc_output_times > 0)
                {
                    cb_info.buffer_addr = csc->output_buf;
                    cb_info.len = csc->output_buf_len;
                    cb_info.ptunnel_info = pUser_info;
                    if(cb_empty_buf(&cb_info) != 0)
                    {
                        ret = CSC_CB_ERROR;
                        break;
                    }
                    csc->output_buf = cb_info.buffer_addr;
                }
                csc_output_times ++;
                if(csc_output_times == csc_output_loop_times)
                {
                    if(csc->yuv_fmt == CSC_YCBCR_422 && csc->scal_height > 0 && (csc->scal_height %8 != 0))
                    {
                        csc->output_buf_len = csc->output_buf_len/8*(csc->scal_height%8);
                    }
                    else if(csc->yuv_fmt == CSC_YCBCR_420 && csc->scal_height > 0 && (csc->scal_height %16 != 0))
                    {
                        csc->output_buf_len = csc->output_buf_len/16*(csc->scal_height%16);
                    }
                }
                csc_idma_output_status = IDMA_BUSY;
                if(csc_output_times <= csc_output_loop_times)
                {
                    if(CSC_Idma_Out(csc->output_buf, csc->output_buf_len) != 0)
                    {
                        ret = CSC_IDMA_ERROR;
                        break;
                    }
                }
                
            }
        }
        if(csc_proc_pause)
        {
            ret = CSC_PROC_PAUSE;
            break;
        }
        if(csc_idma_input_status == IDMA_IDLE)
        {
            if(csc_input_times == csc_proc_loop_times)
            {
//                ret = CSC_PROC_SUCCESS;
                continue;
            }
            if(csc->yuv_fmt == CSC_YCBCR_422)
            {
                cb_info.buffer_addr = csc->input_buf;
                cb_info.len = csc->input_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_fill_buf(&cb_info) == -1)
                {
                    ret = CSC_CB_ERROR;
                    break;
                }
                csc->input_buf = cb_info.buffer_addr;
            }
            else
            {
                if(csc->cvt_mode == CSC_Y2R)
                {
                    csc->input_buf_len = csc->frame_w * CSC_YUV420_PIXEL;
                }
                cb_info.buffer_addr = csc->input_buf;
                cb_info.len = csc->input_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_fill_buf(&cb_info) != 0)
                {
                    ret = CSC_CB_ERROR;
                    break;
                }
                csc->input_buf = cb_info.buffer_addr;
                if(csc->cvt_mode == CSC_Y2R)
                {
                    csc->input_buf_len = csc->frame_w * CSC_SLICE_420 * 2;
                }
            }
            csc_input_times ++;
            csc_idma_input_status = IDMA_BUSY;
            
            if(csc->cvt_mode == CSC_R2Y && (csc->frame_h & 15) != 0)
            {
                if(csc_input_times == csc_proc_loop_times)
                {
                    for(i =0 ;i<(csc_input_times * csc->input_buf_len - csc_total_len);i++)
                    {
                        *(uint8_t *)(csc->input_buf + csc->input_buf_len - 
                                        (csc_input_times * csc->input_buf_len - csc_total_len)+i) = 0;
                    }
                }
            }
            if(CSC_Idma_In(csc->input_buf, csc->input_buf_len) != 0)
            {
                ret = CSC_IDMA_ERROR;
                break;
            }
        }
    }
    if(csc_output_times > csc_output_loop_times)
    {
        ret = CSC_PROC_SUCCESS;
    }
    return ret;
}

void CSC_Proc_Pause(void)
{
    csc_proc_pause = 1;
}

void CSC_Proc_Resume(void)
{
    csc_proc_pause = 0;
}

/**
 *  \brief  csc deinitial
 *
 *  \param
 *  \return -1:deinitial fail
 *           0:deinitial success
 *
 *  \details
 */
int 
CSC_Deinit(void)
{
    SN_CSC_Type_t *SN_CSC = (SN_CSC_Type_t*)(SN_CSC_BASE);
    uint32_t timeout = 0;
    while(IDMA_Get_Status(IDMA_CH_CSC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN))
    {
        timeout++;
        if(timeout > 0xFFFFFF)
        {
            return -1;
        }
    }
    csc_idma_input_status = IDMA_IDLE;
    csc_idma_output_status = IDMA_IDLE;
    NVIC_ClearPendingIRQ(CSC_DMA_IN_IRQn);
    NVIC_DisableIRQ(CSC_DMA_IN_IRQn);
    NVIC_ClearPendingIRQ(CSC_DMA_OUT_IRQn);
    NVIC_DisableIRQ(CSC_DMA_OUT_IRQn);
    SN_CSC->CSC_CTRL_b.RGB_DI_EN = 0;
    memset((void *)SN_CSC, 0x0, sizeof(SN_CSC_Type_t));
    _CSC_Enable(false);
    while(SN_CSC->CSC_CTRL_b.CSC_EN)
    {
        timeout++;
        if(timeout > 0xFFFFFF)
        {
            return -1;
        }
    }
    return 0;
}

/**
 *  \brief get csc library version
 *
 *  \param 
 *  \return csc library version
 *
 *  \details
 */
uint32_t
CSC_GetVersion(void)
{
    return CSC_VERSION;
}
