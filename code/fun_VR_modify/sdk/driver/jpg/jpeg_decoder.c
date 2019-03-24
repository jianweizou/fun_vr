/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file jpeg_decoder.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */

#include <string.h>
#include "snc_jpeg.h"
#include "snc_idma.h"

#include "register_7320.h"
#define JPEG_VERSION        0x73200001
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
#define DEC_YUV422_PIXEL 16 //(JPEG_SLICE_422 * 2)
#define DEC_YUV420_PIXEL 24 //(JPEG_SLICE_420 * 1.5)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static bool proc_run = false;
static uint8_t dec_proc_pause = 0;
static uint8_t *dec_output_buffer;
static uint32_t dec_output_buffer_length;
static uint32_t dec_proc_loop_times = 0;
static uint32_t output_times = 0;
static uint32_t csc_cunt = 0;
static volatile idma_status_t dec_idma_input_status;
static volatile idma_status_t dec_idma_output_status;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
__weak int CSC_Idma_Out(uint32_t wram_addr, uint32_t len);
__weak int CSC_Idma_In(uint32_t wram_addr, uint32_t len);
__weak void CSC_Irq_Start(csc_slice_sel_t idma_sel);
__weak idma_status_t CSC_Get_Irq(csc_slice_sel_t idma_sta);

/**
 *  \brief  jpeg decoder insert qtable
 *
 *  \param [in] qt_addr : qtable address
 *  \return
 *
 *  \details jpeg decoder insert q table to hardware
 */
static void 
_JPEG_DEC_Insert_Qtable(
    uint32_t    qt_addr)
{
    SN_JPEG_DEC_Type_t *SN_JPEG_DEC = (SN_JPEG_DEC_Type_t*)(SN_JPEG_DEC_BASE);
    uint32_t i;
    uint8_t* snvin_QT; 

    snvin_QT = (uint8_t *)(qt_addr);//Q-table
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.Insert_Q_Table = 1;
    for(i=0;i<128;i++){
        SN_JPEG_DEC->JPEG_Write_Q_Table = (uint8_t)*(snvin_QT + i);//write Q-table
    }
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.Insert_Q_Table = 0;//0: Disable Insert Q_table Mode -> Normal Mode
}

/**
 *  \brief  set jpeg decoder intput idma
 *
 *  \param [in] wram_addr : jpeg decoder input buffer address
 *  \param [in] len jpeg : decoder input buffer length
 *  \return
 *
 *  \details setting decoder input idma buffer address and buffer length
 */
int 
JPEG_DEC_Idma_In(
    uint32_t wram_addr, 
    uint32_t len)
{
    idma_setting_t idma_jpeg_input;
    idma_jpeg_input.dip_addr = 0;
    idma_jpeg_input.direction = IDMA_DIRECTION_DIP_2_RAM;
    idma_jpeg_input.ram_addr = wram_addr;
    idma_jpeg_input.length = len;
    
    if(IDMA_Start(IDMA_CH_JPEG_DEC_IN, &idma_jpeg_input) != 0)
    {
        return -1;
    }
    return 0;
}

/**
 *  \brief  set jpeg decoder output idma
 *
 *  \param [in] wram_addr : jpeg decoder output buffer address
 *  \param [in] len jpeg : decoder output buffer length
 *  \return
 *
 *  \details setting decoder output idma buffer address and buffer length
 */
int 
JPEG_DEC_Idma_Out(
    uint32_t wram_addr, 
    uint32_t len)
{
    idma_setting_t idma_jpeg_output;
    idma_jpeg_output.dip_addr = 0;
    idma_jpeg_output.direction = IDMA_DIRECTION_DIP_2_RAM;
    idma_jpeg_output.ram_addr = wram_addr;
    idma_jpeg_output.length = len;
    if(IDMA_Start(IDMA_CH_JPEG_DEC_OUT, &idma_jpeg_output) != 0)
    {
        return -1;
    }
    return 0;
}

/**
 *  \brief  set jpeg decoder output slice start
 *
 *  \param [in] jpeg_decoder : a pointer to a jpeg_decoder_handle_info_t structure
 *  \return
 *
 *  \details start decoding next slice
 */
void 
JPEG_DEC_Set_Slice_Start(void)
{
    SN_JPEG_DEC_Type_t *SN_JPEG_DEC = (SN_JPEG_DEC_Type_t*)(SN_JPEG_DEC_BASE);
    if(!SN_JPEG_DEC->JPEG_DEC_STAT_b.OF_End)
    {
        JPEG_DEC_Idma_Out((uint32_t)dec_output_buffer, dec_output_buffer_length);
    }
}

/**
 *  \brief set jpeg decoder input/output one slice done
 *
 *  \param [in] idma_sel : enum jpeg_slice_sel_t
 *  \return
 *
 *  \details set jpeg decoder one slice done
 */
void 
JPEG_DEC_Set_Slice_Done(
    jpeg_slice_sel_t idma_sel)
{
    if(idma_sel == JPEG_SLICE_INPUT)
    {
        dec_idma_input_status = IDMA_IDLE;
    }
    else if(idma_sel == JPEG_SLICE_OUTPUT)
    {
        dec_idma_output_status = IDMA_IDLE;
    }
}

/**
 *  \brief  get jpeg decoder output buffer length
 *
 *  \param [in] jpeg_header : a pointer to an jpeg_header_info_t structure
 *  \return decoder output buffer length
 *
 *  \details calculate decoder output buffer length
 */
uint32_t 
JPEG_DEC_Get_Output_Buf_Len(
    jpeg_header_info_t *jpeg_header)
{
    uint32_t align_w = 0;
    do
    {
        if(jpeg_header->frame_w <= 0 || jpeg_header->frame_h <= 0)
        {
            break;
        }
        if(jpeg_header->jpeg_fmt == JPEG_YCBCR_422)
        {
            align_w = (jpeg_header->frame_w+7) & ~7;
            return align_w * DEC_YUV422_PIXEL;
        }
        else if(jpeg_header->jpeg_fmt == JPEG_YCBCR_420)
        {
            align_w = (jpeg_header->frame_w+15) & ~15;
            return align_w * DEC_YUV420_PIXEL;
        }
    }while(0);
    memset((void *)jpeg_header, 0x0, sizeof(jpeg_header));
    return 0;
}

/**
 *  \brief set jpeg decoder idma buffer length
 *
 *  \param [in] jpeg_decoder : a pointer to a jpeg_decoder_handle_info_t structure
 *  \param [in] jpeg_header : a pointer to a jpeg_header_info_t structure
 *  \param [in] poutbuf : a point to a output buffer
 *  \param [in] outbuf_len : output buffer length
 *  \return -1 : set buffer length fail, buffer length not enough
 *           0 : set buffer length success
 *
 *  \details set buffer address and buffer length to structure jpeg_decoder
 */
int 
JPEG_DEC_Set_Buffer(
    jpeg_decoder_handle_info_t *jpeg_decoder, 
    jpeg_header_info_t *jpeg_header,
    uint8_t *poutbuf,
    uint32_t outbuf_len)
{
    uint32_t align_w = 0;
    uint32_t align_h = 0;
    dec_proc_loop_times = 0;
    int ret = 0;
    do
    {
        if(jpeg_header->frame_h <= 0 || jpeg_header->frame_w <= 0)
        {
            ret = -1;
            break;
        }
        
        if(jpeg_header->jpeg_fmt == JPEG_YCBCR_422)
        {
            align_w = (jpeg_header->frame_w+7) & ~7;
            align_h = (jpeg_header->frame_h+7) & ~7;
            if(align_w * DEC_YUV422_PIXEL <= outbuf_len)
            {
                jpeg_decoder->output_buf_len = align_w * DEC_YUV422_PIXEL;
                dec_proc_loop_times = align_h / JPEG_SLICE_422;
                if(align_h % JPEG_SLICE_422 > 0)
                {
                    dec_proc_loop_times ++;
                }
            }
            else
            {
                ret = -1;
                break;
            }
        }
        else if(jpeg_header->jpeg_fmt == JPEG_YCBCR_420)
        {
            align_w = (jpeg_header->frame_w+15) & ~15;
            align_h = (jpeg_header->frame_h+15) & ~15;
            
            if(jpeg_header->frame_w * DEC_YUV420_PIXEL <= outbuf_len)
            {
                jpeg_decoder->output_buf_len = align_w * DEC_YUV420_PIXEL;
                dec_proc_loop_times = align_h / JPEG_SLICE_420;
                if(align_h % JPEG_SLICE_420 > 0)
                {
                    dec_proc_loop_times ++;
                }
            }
            else
            {
                ret = -1;
                break;
            }
        }
        jpeg_decoder->output_buf = (uint32_t)poutbuf;
        dec_output_buffer = poutbuf;
        dec_output_buffer_length = jpeg_decoder->output_buf_len;
    }while(0);
    
    if(ret == -1)
    {
        memset((void *)jpeg_header, 0x0, sizeof(jpeg_header));
        memset((void *)jpeg_decoder, 0x0, sizeof(jpeg_decoder));
    }
    return ret;
}

/**
 *  \brief  jpeg decoder initial
 *
 *  \param [in] jpeg_decoder : a pointer to a jpeg_decoder_handle_info_t structure
 *  \param [in] jpeg_header : a pointer to a jpeg_header_info_t structure
 *  \return -1 : initial fail
 *           0 : initial success 
 *
 *  \details jpeg decoder initial script
 */
int 
JPEG_DEC_Init(
    jpeg_decoder_handle_info_t *jpeg_decoder, 
    jpeg_header_info_t *jpeg_header)
{
    SN_JPEG_DEC_Type_t *SN_JPEG_DEC = (SN_JPEG_DEC_Type_t*)(SN_JPEG_DEC_BASE);
    dec_proc_pause = 0;

    output_times = 0;
    csc_cunt = 0;
    proc_run = false;
    dec_idma_input_status = IDMA_IDLE;
    dec_idma_output_status = IDMA_IDLE;
    
    _JPEG_DEC_Insert_Qtable(jpeg_header->qtable);
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.Input_Frame_End = 0;
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.YUV_Format = jpeg_header->jpeg_fmt;
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.Q_TABLE_GAIN = jpeg_decoder->set_qtable_gain;
    SN_JPEG_DEC->SKIP_BYTE_b.SKIP_BYTE = jpeg_header->skip_byte;
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.JPEG_DEC_EN = 1;
    
    if(jpeg_decoder->en_irq)
    {
        NVIC_ClearPendingIRQ(JPEG_DEC_IN_DMA_IRQn);
        NVIC_EnableIRQ(JPEG_DEC_IN_DMA_IRQn);
        NVIC_ClearPendingIRQ(JPEG_DEC_OUT_DMA_IRQn);
        NVIC_EnableIRQ(JPEG_DEC_OUT_DMA_IRQn);
        NVIC_ClearPendingIRQ(CSC_DMA_IN_IRQn);
        NVIC_EnableIRQ(CSC_DMA_IN_IRQn);
        NVIC_ClearPendingIRQ(CSC_DMA_OUT_IRQn);
        NVIC_EnableIRQ(CSC_DMA_OUT_IRQn);
    }
    
    if(SN_JPEG_DEC->JPEG_DEC_CTRL_b.JPEG_DEC_EN)
    {
        return 0;
    }
    memset((void *)jpeg_header, 0x0, sizeof(jpeg_header));
    memset((void *)jpeg_decoder, 0x0, sizeof(jpeg_decoder));
    return -1;
}

/**
 *  \brief  jpeg decoder process
 *
 *  \param [in] jpeg_decoder : a pointer to an jpeg_decoder_handle_info_t structure
 *  \param [in] cb_fill_buf : jpeg fill buffer callback function
 *  \param [in] cb_empty_buf : jpeg empty buffer callback function
 *  \param [in] pUser_info : user info input by pass to cb_fill_buf and cb_empty_buf
 *  \param [in] csc : a pointer to an csc_handle_info_t structure
 *  \return     -1 : JPEG_PROC_ERROR
 *               0 : JPEG_PROC_SUCCESS
 *               1 : JPEG_PROC_IDLE
 *               3 : JPEG_PROC_PAUSE
 *
 *  \details
 *   in function Jpeg_Dec_Proc need user to set a callback function,
 *   and it provides an interface to let user input user_info.
 *   the parameter user_info without the types limit, 
 *   and funcion Jpeg_Dec_Proc will bypass user_info to callback function.
 */
jpeg_process_status_t
JPEG_DEC_Proc(
    jpeg_decoder_handle_info_t  *jpeg_decoder, 
    CB_JPEG_FILL_BUF            cb_fill_buf, 
    CB_JPEG_EMPTY_BUF           cb_empty_buf, 
    void                        *pUser_info,
    csc_handle_info_t           *csc)
{
    SN_JPEG_DEC_Type_t *SN_JPEG_DEC = (SN_JPEG_DEC_Type_t*)(SN_JPEG_DEC_BASE);
    cb_jpeg_info_t cb_info;
    jpeg_process_status_t ret = JPEG_PROC_IDLE;
    if(!proc_run)
    {
    proc_run = true;
    do{
        if(csc)
        {
            if(csc->yuv_fmt == JPEG_YCBCR_422)
            {
                if(csc->scal_en && csc->scal_height >0 && csc->scal_width >0)
                {
                    dec_proc_loop_times = csc->scal_height / JPEG_SLICE_422;
                    if(csc->scal_height % JPEG_SLICE_422 > 0)
                    {
                        dec_proc_loop_times++;
                    }
                }
            }
            else if(csc->yuv_fmt == JPEG_YCBCR_420)
            {
                if(csc->scal_en && csc->scal_height >0 && csc->scal_width >0)
                {
                    dec_proc_loop_times = csc->scal_height / JPEG_SLICE_420;
                    if(csc->scal_height % JPEG_SLICE_420 > 0)
                    {
                        dec_proc_loop_times++;
                    }
                }
            }
            CSC_Irq_Start(CSC_SLICE_OUTPUT);
            CSC_Irq_Start(CSC_SLICE_INPUT);
            if(CSC_Idma_Out(csc->output_buf, csc->output_buf_len) != 0)
            {
                ret = JPEG_IDMA_ERROR;
                break;
            }
        }
        cb_info.buffer_addr = jpeg_decoder->input_buf;
        cb_info.len = jpeg_decoder->input_buf_len;
        cb_info.ptunnel_info = pUser_info;
        if(cb_fill_buf(&cb_info) != 0)
        {
            ret = JPEG_CB_ERROR;
            break;
        }
        jpeg_decoder->input_buf = cb_info.buffer_addr;
        jpeg_decoder->input_buf_len = cb_info.len;
        
        dec_idma_output_status = IDMA_BUSY;
        dec_idma_input_status = IDMA_BUSY;
        if(jpeg_decoder->idma_burst != 0)
        {
            if(IDMA_Set_Burst_Unit(IDMA_CH_JPEG_DEC_IN, jpeg_decoder->idma_burst) == IDMA_RET_FAIL)
            {
                ret = JPEG_IDMA_ERROR;
                break;
            }
        }
        if(JPEG_DEC_Idma_In(jpeg_decoder->input_buf, jpeg_decoder->input_buf_len) != 0)
        {
            ret = JPEG_IDMA_ERROR;
            break;
        }
        
        if(JPEG_DEC_Idma_Out(jpeg_decoder->output_buf, jpeg_decoder->output_buf_len) != 0)
        {
            ret = JPEG_IDMA_ERROR;
            break;
        }
    }while(0);
    
    if(ret != JPEG_PROC_IDLE)
    {
        return ret;
    }
    }
    /* JPEG Decode Process */
    while(output_times < dec_proc_loop_times)
    {
        /* check previous slice fail */
        if(SN_JPEG_DEC->JPEG_DEC_STAT_b.Error_signal)
        {
            ret = JPEG_PROC_ERROR;
            break;
        }
        if(dec_idma_output_status == IDMA_IDLE)
        {
            if(!csc)
            {
                cb_info.buffer_addr = jpeg_decoder->output_buf;
                cb_info.len = jpeg_decoder->output_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_empty_buf(&cb_info) != 0)
                {
                    ret = JPEG_CB_ERROR;
                    break;
                }
                jpeg_decoder->output_buf = cb_info.buffer_addr;
            }
            dec_idma_output_status = IDMA_BUSY;
            if(csc)
            {
                if(CSC_Idma_In(csc->input_buf, csc->input_buf_len) != 0)
                {
                    ret = JPEG_IDMA_ERROR;
                    break;
                }
            }
            else
            {
                output_times ++;
                if(output_times < dec_proc_loop_times)
                {
                    if(JPEG_DEC_Idma_Out(jpeg_decoder->output_buf, jpeg_decoder->output_buf_len) != 0)
                    {
                        ret = JPEG_IDMA_ERROR;
                        break;
                    }
                }
            }
        }
        if(dec_proc_pause == true)
        {
            ret = JPEG_PROC_PAUSE;
            break;
        }
        if(dec_idma_input_status == IDMA_IDLE)
        {
            if(!SN_JPEG_DEC->JPEG_DEC_STAT_b.OF_End)
            {
                cb_info.buffer_addr = jpeg_decoder->input_buf;
                cb_info.len = jpeg_decoder->input_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_fill_buf(&cb_info) != 0)
                {
                    ret = JPEG_CB_ERROR;
                    break;
                }
                jpeg_decoder->input_buf = cb_info.buffer_addr;
                jpeg_decoder->input_buf_len = cb_info.len;
                
                dec_idma_input_status = IDMA_BUSY;
                if(JPEG_DEC_Idma_In(jpeg_decoder->input_buf, jpeg_decoder->input_buf_len)!= 0)
                {
                    ret = JPEG_IDMA_ERROR;
                    break;
                }
            }
        }
        if(csc)
        {
            if(CSC_Get_Irq(CSC_SLICE_INPUT) == IDMA_IDLE)
            {
                CSC_Irq_Start(CSC_SLICE_INPUT);
                if(!SN_JPEG_DEC->JPEG_DEC_STAT_b.OF_End)
                {
                    if(JPEG_DEC_Idma_Out(jpeg_decoder->output_buf, jpeg_decoder->output_buf_len) != 0)
                    {
                        ret = JPEG_IDMA_ERROR;
                        break;
                    }
                }
            }
            if(CSC_Get_Irq(CSC_SLICE_OUTPUT) == IDMA_IDLE)
            {
                output_times ++;
                csc_cunt += csc->output_buf_len;
                
                cb_info.buffer_addr = csc->output_buf;
                cb_info.len = csc->output_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_empty_buf(&cb_info) != 0)
                {
                    ret = JPEG_CB_ERROR;
                    break;
                }
                csc->output_buf = cb_info.buffer_addr;
                
                CSC_Irq_Start(CSC_SLICE_OUTPUT);

                if(output_times < dec_proc_loop_times)
                {
                    if(output_times == dec_proc_loop_times-1)
                    {
                        if(csc->yuv_fmt == JPEG_YCBCR_422 && csc->scal_height > 0 && (csc->scal_height %8 != 0))
                        {
                            csc->output_buf_len = csc->output_buf_len/8*(csc->scal_height%8);
                        }
                        else if(csc->yuv_fmt == JPEG_YCBCR_420 && csc->scal_height > 0 && (csc->scal_height %16 != 0))
                        {
                            csc->output_buf_len = csc->output_buf_len/16*(csc->scal_height%16);
                        }
                    }
                    if(CSC_Idma_Out(csc->output_buf, csc->output_buf_len) != 0)
                    {
                        ret = JPEG_IDMA_ERROR;
                        break;
                    }
                }
            }
            if(SN_JPEG_DEC->JPEG_DEC_STAT_b.OF_End && csc->scal_en)
            {
                if(csc_cunt + IDMA_Get_Cutrrent_Length(IDMA_CH_CSC_OUT) >= ((csc->scal_width * csc->scal_height *2)))
                {
                    output_times ++;
                }
            }
        }
    }
    return ret == JPEG_PROC_IDLE?JPEG_PROC_SUCCESS : ret;

}

void JPEG_DEC_Proc_Pause(void)
{
    dec_proc_pause = 1;
}

void JPEG_DEC_Proc_Resume(void)
{
    dec_proc_pause = 0;
}

/**
 *  \brief  jpeg decoder deinitial
 *
 *  \param
 *  \return -1:deinitial fail
 *           0:deinitial success
 *
 *  \details
 */
int 
JPEG_DEC_Deinit(void)
{
    SN_JPEG_DEC_Type_t *SN_JPEG_DEC = (SN_JPEG_DEC_Type_t*)(SN_JPEG_DEC_BASE);
    uint32_t timeout = 0;
    
    if(!IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT))
    {
        timeout++;
        if(timeout > 0xFFFFFF)
        {
            return -1;
        }
    }
    dec_idma_input_status = IDMA_IDLE;
    dec_idma_output_status = IDMA_IDLE;
    NVIC_ClearPendingIRQ(JPEG_DEC_IN_DMA_IRQn);
    NVIC_DisableIRQ(JPEG_DEC_IN_DMA_IRQn);
    NVIC_ClearPendingIRQ(JPEG_DEC_OUT_DMA_IRQn);
    NVIC_DisableIRQ(JPEG_DEC_OUT_DMA_IRQn);
    IDMA_Stop(IDMA_CH_JPEG_DEC_IN);
    IDMA_Stop(IDMA_CH_JPEG_DEC_OUT);
    SN_JPEG_DEC->JPEG_DEC_CTRL_b.JPEG_DEC_EN = 0;
    while(SN_JPEG_DEC->JPEG_DEC_CTRL_b.JPEG_DEC_EN)
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
 *  \brief get jpeg library version
 *
 *  \param 
 *  \return jpeg library version
 *
 *  \details
 */
uint32_t
JPEG_GetVersion(void)
{
    return JPEG_VERSION;
}
