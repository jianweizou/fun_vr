/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file jpeg_encoder.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */

#include <string.h>
#include "snc_jpeg.h"
#include "snc_gpio.h"
#include "snc_idma.h"
#include "register_7320.h"
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
#define ENC_YUV422_PIXEL 16 //(JPEG_SLICE_422 * 2)
#define ENC_YUV420_PIXEL 24 //(JPEG_SLICE_420 * 1.5)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t enc_proc_pause = 0;
static uint32_t enc_proc_loop_times = 0;
static uint32_t enc_output_times = 0;
static uint32_t enc_input_times = 0;
static volatile idma_status_t enc_idma_input_status;
static volatile idma_status_t enc_idma_output_status;
extern uint32_t JPEG_Header_Size(void);
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

/**
 *  \brief  jpeg encoder insert q table
 *
 *  \param [in] qt_addr q table address
 *  \return
 *
 *  \details
 */
static void 
_JPEG_ENC_Insert_Qtable(
    uint32_t qt_addr)
{
    SN_JPEG_ENC_Type_t *SN_JPEG_ENC = (SN_JPEG_ENC_Type_t*)(SN_JPEG_ENC_BASE);
    uint32_t i;
    uint8_t* snvin_QT; 

    snvin_QT = (uint8_t *)(qt_addr);//Q-table
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.Insert_Q_Table = 1;
    for(i=0;i<128;i++){
        SN_JPEG_ENC->JPEG_Write_Q_Table = (uint8_t)*(snvin_QT + i);//write Q-table
    }
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.Insert_Q_Table = 0;//0: Disable Insert Q_table Mode -> Normal Mode
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.QUT_RD_EN = 1;
}

/**
 *  \brief  set jpeg encoder intput/output status
 *
 *  \param [in] idma_sel : enum jpeg_slice_sel_t
 *  \return
 *
 *  \details
 */
void 
JPEG_ENC_Set_Slice_Done(
    jpeg_slice_sel_t idma_sel)
{
    if(idma_sel == JPEG_SLICE_INPUT)
    {
        enc_idma_input_status = IDMA_IDLE;
    }
    else if(idma_sel == JPEG_SLICE_OUTPUT)
    {
        enc_idma_output_status = IDMA_IDLE;
    }
}

/**
 *  \brief  get jpeg encoder input buffer length
 *
 *  \param [in] jpeg_header : a pointer to a jpeg_header_info_t structure
 *  \return encoder input buffer length
 *
 *  \details calculate encoder input buffer length
 */
uint32_t 
JPEG_ENC_Get_Input_Buf_Len(
    jpeg_header_info_t *jpeg_header)
{
    do
    {
        if(jpeg_header->frame_w <= 0 || jpeg_header->frame_h <= 0)
        {
            break;
        }
        if(jpeg_header->jpeg_fmt == JPEG_YCBCR_422)
        {
            return jpeg_header->frame_w * ENC_YUV422_PIXEL;
        }
        else if(jpeg_header->jpeg_fmt == JPEG_YCBCR_420)
        {
            return jpeg_header->frame_w * ENC_YUV420_PIXEL;
        }
    }while(0);
    memset((void *)jpeg_header, 0x0, sizeof(jpeg_header));
    return 0;
}

/**
 *  \brief  set jpeg encoder idma buffer
 *
 *  \param [in] jpeg_encoder : a pointer to a jpeg_encoder_handle_info_t structure
 *  \param [in] jpeg_header : a pointer to a jpeg_header_info_t structure
 *  \param [in] pinbuf : a point to an input buffer
 *  \param [in] inbuf_len : input buffer length
 *  \return -1 : parameter error
 *           0 : set buffer success
 *
 *  \details set buffer address and buffer length to structure jpeg_encoder
 */
int 
JPEG_ENC_Set_Buffer(
    jpeg_encoder_handle_info_t *jpeg_encoder, 
    jpeg_header_info_t *jpeg_header,
    uint8_t *pinbuf,
    uint32_t inbuf_len)
{
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
            if(jpeg_header->frame_w * ENC_YUV422_PIXEL <= inbuf_len)
            {
                jpeg_encoder->input_buf_len = jpeg_header->frame_w * ENC_YUV422_PIXEL;
                enc_proc_loop_times = ((jpeg_header->frame_w + 7) & ~7) * 
                                        ((jpeg_header->frame_h + 7) & ~7) * 2 / jpeg_encoder->input_buf_len;
            }
            else
            {
                ret = -1;
                break;
            }
        }
        else if(jpeg_header->jpeg_fmt == JPEG_YCBCR_420)
        {
            if(jpeg_header->frame_w * ENC_YUV420_PIXEL <= inbuf_len)
            {
                jpeg_encoder->input_buf_len = jpeg_header->frame_w * ENC_YUV420_PIXEL;
                enc_proc_loop_times = ((jpeg_header->frame_w + 15) & ~15) *
                                        ((jpeg_header->frame_h + 15) & ~15) * 3 / 2 / jpeg_encoder->input_buf_len;
            }
            else
            {
                ret = -1;
                break;
            }
        }
        jpeg_encoder->input_buf = (uint32_t)pinbuf;
    }while(0);
    if(ret == -1)
    {
        memset((void *)jpeg_header, 0x0, sizeof(jpeg_header));
        memset((void *)jpeg_encoder, 0x0, sizeof(jpeg_encoder));
    }
    return 0;
}

/**
 *  \brief  jpeg encoder setting
 *
 *  \param [in] jpeg_encoder : a pointer to a jpeg_encoder_handle_info_t structure
 *  \param [in] jpeg_header : a pointer to a jpeg_header_info_t structure
 *  \return -1:deinitial fail
 *           0:deinitial success
 *
 *  \details jpeg encoder initial script
 */
int 
JPEG_ENC_Init(
    jpeg_encoder_handle_info_t *jpeg_encoder, 
    jpeg_header_info_t *jpeg_header)
{
    SN_JPEG_ENC_Type_t *SN_JPEG_ENC = (SN_JPEG_ENC_Type_t*)(SN_JPEG_ENC_BASE);
    
    enc_output_times = 0;
    enc_input_times = 0;
    enc_idma_input_status = IDMA_IDLE; 
    enc_idma_output_status = IDMA_IDLE;

    _JPEG_ENC_Insert_Qtable((uint32_t)jpeg_header->qtable);
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.Q_TABLE_GAIN = jpeg_encoder->set_qtable_gain;
    if(jpeg_encoder->ac_reduce > 63)
    {
        return -1;
    }
    else
    {
        SN_JPEG_ENC->AC_REDUCE = jpeg_encoder->ac_reduce;
    }
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.Input_Frame_End = 0; //JPEG Input frame end = 0
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.Bit_Stuff = jpeg_encoder->set_stuff_bit; //1: JFIF format
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.YUV_Format = jpeg_header->jpeg_fmt;
    
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.JPEG_ENC_EN = 1;
    SN_JPEG_ENC->JPEG_ENC_CTRL_b.Input_Frame_End = 0;
    if(jpeg_encoder->en_irq)
    {
        NVIC_ClearPendingIRQ( JPEG_ENC_IN_DMA_IRQn );
        NVIC_EnableIRQ( JPEG_ENC_IN_DMA_IRQn );
        NVIC_ClearPendingIRQ( JPEG_ENC_OUT_DMA_IRQn );
        NVIC_EnableIRQ( JPEG_ENC_OUT_DMA_IRQn );
        NVIC_ClearPendingIRQ( JPEG_ENC_END_IRQn );
        NVIC_EnableIRQ( JPEG_ENC_END_IRQn );
    }
    if(SN_JPEG_ENC->JPEG_ENC_CTRL_b.JPEG_ENC_EN)
    {
        return 0;
    }
    memset((void *)jpeg_header, 0x0, sizeof(jpeg_header));
    memset((void *)jpeg_encoder, 0x0, sizeof(jpeg_encoder));
    return -1;
}

/**
 *  \brief  jpeg encoder process
 *
 *  \param [in] jpeg_encoder : a pointer to a jpeg_encoder_handle_info_t structure
 *  \param [in] cb_fill_buf : jpeg fill buffer callback function
 *  \param [in] cb_empty_buf : jpeg empty buffer callback function
 *  \param [in] pUser_info : user info input by pass to cb_fill_buf and cb_empty_buf
 *  \return     -1 : JPEG_PROC_ERROR
 *               0 : JPEG_PROC_SUCCESS
 *               1 : JPEG_PROC_IDLE
 *               3 : JPEG_PROC_PAUSE
 *
 *  \details
 *   in function Jpeg_Enc_Proc need user to set a callback function,
 *   and it provides an interface to let user input user_info
 *   the parameter user_info without the types limit,
 *   and funcion Jpeg_Enc_Proc will bypass user_info to callback function.
 */
jpeg_process_status_t 
JPEG_ENC_Proc(
    jpeg_encoder_handle_info_t  *jpeg_encoder, 
    CB_JPEG_FILL_BUF            cb_fill_buf, 
    CB_JPEG_EMPTY_BUF           cb_empty_buf, 
    void                        *pUser_info)
{
    SN_JPEG_ENC_Type_t *SN_JPEG_ENC = (SN_JPEG_ENC_Type_t*)(SN_JPEG_ENC_BASE);
    uint32_t i=0;
    idma_setting_t idma_jpeg_output;
    idma_setting_t idma_jpeg_input;
    cb_jpeg_info_t cb_info;
    jpeg_process_status_t ret = JPEG_PROC_IDLE;
    
    /* JPEG Encode Process */
    while(SN_JPEG_ENC->JPEG_ENC_STAT_b.OF_End != 1)
    {
        if(enc_idma_output_status == IDMA_IDLE)
        {
            if(!SN_JPEG_ENC->JPEG_ENC_STAT_b.OF_End)
            {
                if(enc_output_times > 0)
                {
                    cb_info.buffer_addr = jpeg_encoder->output_buf;
                    cb_info.len = jpeg_encoder->output_buf_len;
                    cb_info.ptunnel_info = pUser_info;
                    if(cb_empty_buf(&cb_info) != 0)
                    {
                        ret = JPEG_CB_ERROR;
                        break;
                    }
                    jpeg_encoder->output_buf = cb_info.buffer_addr;
                    jpeg_encoder->output_buf_len = cb_info.len;
                    jpeg_encoder->jpeg_total_size += IDMA_Get_Cutrrent_Length(IDMA_CH_JPEG_ENC_OUT); // get idma length 
                }
                enc_idma_output_status = IDMA_BUSY;
                enc_output_times ++;
                if(jpeg_encoder->idma_burst != 0)
                {
                    if(IDMA_Set_Burst_Unit(IDMA_CH_JPEG_ENC_OUT, jpeg_encoder->idma_burst) == IDMA_RET_FAIL)
                    {
                        ret = JPEG_IDMA_ERROR;
                        break;
                    }
                }
                idma_jpeg_output.dip_addr = 0;
                idma_jpeg_output.direction = IDMA_DIRECTION_DIP_2_RAM;
                idma_jpeg_output.ram_addr = jpeg_encoder->output_buf;
                idma_jpeg_output.length = jpeg_encoder->output_buf_len;
                if(IDMA_Start(IDMA_CH_JPEG_ENC_OUT, &idma_jpeg_output) != 0)
                {
                    ret = JPEG_IDMA_ERROR;
                    break;
                }
            }
        }
        if(enc_proc_pause)
        {
            ret = JPEG_PROC_PAUSE;
            break;
        }
        if(enc_idma_input_status == IDMA_IDLE)
        {
            if(enc_input_times <= enc_proc_loop_times)
            {
                if(SN_JPEG_ENC->JPEG_ENC_CTRL_b.Input_Frame_End != 1)
                {
                    cb_info.buffer_addr = jpeg_encoder->input_buf;
                    cb_info.len = jpeg_encoder->input_buf_len;
                    cb_info.ptunnel_info = pUser_info;
                    if(cb_fill_buf(&cb_info) != 0)
                    {
                        ret = JPEG_CB_ERROR;
                        break;
                    }
                    jpeg_encoder->input_buf = cb_info.buffer_addr;
                    
                    enc_input_times ++;
                    enc_idma_input_status = IDMA_BUSY;
                    idma_jpeg_input.dip_addr = 0;
                    idma_jpeg_input.direction = IDMA_DIRECTION_DIP_2_RAM;
                    idma_jpeg_input.ram_addr = jpeg_encoder->input_buf;
                    idma_jpeg_input.length = jpeg_encoder->input_buf_len;

                    if(IDMA_Start(IDMA_CH_JPEG_ENC_IN, &idma_jpeg_input) != 0)
                    {
                        ret = JPEG_IDMA_ERROR;
                        break;
                    }
                    if(enc_input_times == enc_proc_loop_times)
                    {
                        SN_JPEG_ENC->JPEG_ENC_CTRL_b.Input_Frame_End = 1;
                    }
                }
            }
        }
    }
    if(ret == JPEG_PROC_IDLE)
    {
        jpeg_encoder->output_buf += IDMA_Get_Cutrrent_Length(IDMA_CH_JPEG_ENC_OUT);
        for(i = 0;i<jpeg_encoder->output_buf_len;i++)
        {
            if(*(uint8_t*)(jpeg_encoder->output_buf - i) == 0xD9)
            {
                if(*(uint8_t*)(jpeg_encoder->output_buf - i - 1) == 0xFF)
                {
                    jpeg_encoder->jpeg_total_size = jpeg_encoder->jpeg_total_size
                                                        + IDMA_Get_Cutrrent_Length(IDMA_CH_JPEG_ENC_OUT) - (i-1);
                    jpeg_encoder->jpeg_total_size += JPEG_Header_Size();
                    break;
                }
            }
        }
    }
    return ret == JPEG_PROC_IDLE?JPEG_PROC_SUCCESS : ret;
}

void JPEG_ENC_Proc_Pause(void)
{
    enc_proc_pause = 1;
}

void JPEG_ENC_Proc_Resume(void)
{
    enc_proc_pause = 0;
}

/**
 *  \brief  jpeg encoder deinitial
 *
 *  \param
 *  \return -1:deinitial fail
 *           0:deinitial success,
 *
 *  \details
 */
int 
JPEG_ENC_Deinit(void)
{
    SN_JPEG_ENC_Type_t *SN_JPEG_ENC = (SN_JPEG_ENC_Type_t*)(SN_JPEG_ENC_BASE);
    if(!IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT))
    {
        enc_idma_input_status = IDMA_IDLE;
        enc_idma_output_status = IDMA_IDLE;
        NVIC_ClearPendingIRQ(JPEG_ENC_IN_DMA_IRQn);
        NVIC_DisableIRQ(JPEG_ENC_IN_DMA_IRQn);
        NVIC_ClearPendingIRQ(JPEG_ENC_OUT_DMA_IRQn);
        NVIC_DisableIRQ(JPEG_ENC_OUT_DMA_IRQn);
        NVIC_ClearPendingIRQ(JPEG_ENC_END_IRQn);
        NVIC_DisableIRQ(JPEG_ENC_END_IRQn);
        
        IDMA_Stop(IDMA_CH_JPEG_ENC_IN);
        IDMA_Stop(IDMA_CH_JPEG_ENC_OUT);
        SN_JPEG_ENC->JPEG_ENC_CTRL_b.JPEG_ENC_EN = 0;
        enc_output_times = 0;
        enc_input_times = 0;
        
        return 0;
    }
    return -1;
}
