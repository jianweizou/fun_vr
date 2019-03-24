/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file cis.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */


#include "snc_cis.h"
#include "snc_idma.h"

#include "register_7320.h"
#define CIS_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define CIS_YUV422_PIXEL (8 * 2)
#define CIS_YUV420_PIXEL (16 * 1.5)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
uint8_t cis_en_flag = 0;
cis_line_t idma_cis_line_n = CIS_LINE_0;
uint32_t href_cnt = 0;
uint32_t cis_scal_ratio_w;
uint32_t cis_scal_ratio_h;
static cis_handle_info_t *pcis_info;
static uint8_t volatile sensor_idma_irq_status = 1;
static uint8_t volatile sensor_vsync_irq_status = 1;

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  \brief cis window mode enable/disable
 *
 *  \param [in] set : true : enable cis,
 *                    false : disable cis
 *  \return -1 : enable/disable fail
 *           0 : enable/disable success 
 *
 *  \details window mode can't use with scaling mode
 */
static int 
_CIS_Window_Enable(
    bool set)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    if(set)
    {
        if(SN_CIS->CIS_SCALE_SIZE_b.SCLAE_EN==0)
        {    
            SN_CIS->CIS_CTRL_b.WINDOWS_EN=1;
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        SN_CIS->CIS_CTRL_b.WINDOWS_EN = 0;
        if(SN_CIS->CIS_CTRL_b.WINDOWS_EN != 0)
        {
            return -1;
        }
        return 0;
    }
}

/**
 *  \brief cis window mode setting
 *
 *  \param [in] W_offset : window Width offset
 *  \param [in] H_offset : window Height offset
 *  \return
 *
 *  \details window width length from W_offset to W_offset + CIS_Col_N
 *           window height length form H_offset to cis idma length / CIS_Col_N
 */
static void 
_CIS_Window_Setting(
    uint32_t w_offset,
    uint32_t h_offset)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    if(SN_CIS->CIS_CTRL_b.CIS_DATA!=0){
        SN_CIS->CIS_HOFFSET=w_offset&0xFFFFFFFE;
    } 
    else{
        SN_CIS->CIS_HOFFSET=w_offset;
    }
    
    SN_CIS->CIS_VOFFSET=h_offset;
}

/**
 *  \brief cis scaling mode enable/disable
 *
 *  \param [in] set : true : enable cis,
 *                    false : disable cis
 *  \return -1 : enable/disable fail
 *           0 : enable/disable success 
 *          
 *
 *  \details scaling mode can't use with window mode
 */
static int 
_CIS_Scale_Enable(
    bool set)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    if(set)
    {
        if(SN_CIS->CIS_CTRL_b.WINDOWS_EN == 0)
        {
            SN_CIS->CIS_SCALE_SIZE_b.SCLAE_EN = 1;
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        SN_CIS->CIS_SCALE_SIZE_b.SCLAE_EN = 0;
        if(SN_CIS->CIS_SCALE_SIZE_b.SCLAE_EN != 0)
        {
            return -1;
        }
        return 0;
    }
}

/**
 *  \brief set cis column number
 *
 *  \param [in] col_n : column number
 *  \return 
 *
 *  \details set cis colume is equal to frame width
 */
static void 
_CIS_Col_N_Set(
    uint32_t col_n)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    if(SN_CIS->CIS_CTRL_b.CIS_DATA!=0)
    {
        SN_CIS->CIS_COL_N=col_n&0xFFFFFFFE;
    }
    else
    {
        SN_CIS->CIS_COL_N=col_n;
    }
}

/**
 *  \brief clear line buffer full flag 
 *
 *  \param [in] idx : enum cis_line_t
 *  \return 
 *
 *  \details clear cis line idma full flag
 */
static void 
_CIS_Clr_Line_Full_Flag(
    cis_line_t idx)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    if(idx == 0)
    {
        SN_CIS->CIS_CTRL_b.L0_FULL_FLG = 0;
    }
    else if(idx == 1)
    {
        SN_CIS->CIS_CTRL_b.L1_FULL_FLG = 0;
    }
    else if(idx == 2)
    {
        SN_CIS->CIS_CTRL_b.L2_FULL_FLG = 0;
    }
    else if(idx == 3)
    {
        SN_CIS->CIS_CTRL_b.L0_FULL_FLG = 0;
        SN_CIS->CIS_CTRL_b.L1_FULL_FLG = 0;
        SN_CIS->CIS_CTRL_b.L2_FULL_FLG = 0;
    }
}

/**
 *  \brief check cis line buffer full flag
 *
 *  \param [in] idx : enum cis_line_t
 *  \return line buffer full flag by index
 *
 *  \details check cis line idma is full or not
 */
static uint32_t 
_CIS_Chk_Line_Full_Flag(
    cis_line_t idx)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    if(idx == 0)
    {
        return (SN_CIS->CIS_CTRL_b.L0_FULL_FLG);
    }
    else if(idx == 1)
    {
        return (SN_CIS->CIS_CTRL_b.L1_FULL_FLG);
    }
    else if(idx == 2)
    {
        return (SN_CIS->CIS_CTRL_b.L2_FULL_FLG);
    }
    return 0;
}

/**
 *  \brief cis idma setting
 *
 *  \param [in] w_size : frame width
 *  \param [in] h_size : frame height
 *  \return IDMA_Start status
 *
 *  \details set cis idma buffer length ( w * h * 2)
 */
static int 
_CIS_idma_Setting(
    uint32_t w_size, 
    uint32_t h_size)
{
    idma_setting_t idma_cis_setting;
    
    _CIS_Col_N_Set(w_size);
    
    IDMA_Extra_Config(IDMA_CH_CIS, IDMA_CFG_CIS_AD_EN);
    idma_cis_setting.dip_addr = 0;
    idma_cis_setting.direction = IDMA_DIRECTION_DIP_2_RAM;
    idma_cis_setting.length = w_size * h_size * 2;
    idma_cis_setting.ram_addr = IDMA_ADDR_IGNORE;
    
    return IDMA_Start(IDMA_CH_CIS, &idma_cis_setting);
}

/**
 *  \brief get cis buffer length
 *
 *  \param [in] cis_info : a pointer to a cis_handle_info_t structure
 *  \return line buffer length
 *
 *  \details calculate cis buffer length
 */
uint32_t 
CIS_Get_Buf_Len(
    cis_handle_info_t *cis_info)
{
    do
    {
        if(cis_info->frame_w <= 0 && cis_info->frame_h <= 0)
        {
            break;
        }
        if(cis_info->l2b.enable)
        {
            if(cis_info->l2b.fmt == CIS_YCBCR_422)
            {
                return cis_info->frame_w * CIS_YUV422_PIXEL;
            }
            else if(cis_info->l2b.fmt == CIS_YCBCR_420)
            {
                return cis_info->frame_w * CIS_YUV420_PIXEL;
            }
        }
        else
        {
            if(cis_info->scaling.enable)
            {
                switch(cis_info->scaling.pixel_skip)
                {
                    case CIS_NO_SKIP_PIXEL:
                        cis_scal_ratio_w = 1;
                        break;
                    case CIS_TWO_PIXEL_TO_ONE:
                        cis_scal_ratio_w = 2;
                        break;
                    case CIS_FOUR_PIXEL_TO_ONE:
                        cis_scal_ratio_w = 4;
                        break;
                }
                switch(cis_info->scaling.line_skip)
                {
                    case CIS_NO_SKIP_LINE:
                        cis_scal_ratio_h = 1;
                        break;
                    case CIS_TWO_LINE_TO_ONE:
                        cis_scal_ratio_h = 2;
                        break;
                    case CIS_FOUR_LINE_TO_ONE:
                        cis_scal_ratio_h = 4;
                        break;
                }
                return cis_info->frame_w * 2 / cis_scal_ratio_w * 3;
            }
            else
            {
                return cis_info->frame_w * 2 * 3;
            }
        }
    }while(0);
    return 0;
}
/**
 *  \brief set cis line buffer length
 *
 *  \param [in] cis_info : a pointer to a cis_handle_info_t structure
 *  \param [in] pline_buf : line buffer address
 *  \param [in] buf_len : line buffer length
 *  \return -1 : set length fail
 *           0 : set length success 
 *
 *  \details set buffer address and buffer length to structure cis_info
 */
int 
CIS_Set_Buffer(
    cis_handle_info_t *cis_info, 
    uint8_t *pline_buf,
    uint32_t buf_len)
{
    if(cis_info->l2b.enable)
    {
        if(cis_info->l2b.fmt == CIS_YCBCR_422)
        {
            if(cis_info->frame_w * CIS_YUV422_PIXEL <= buf_len)
            {
                cis_info->buf_len = cis_info->frame_w * CIS_YUV422_PIXEL;
                cis_info->line_buf_addr[0] = (uint32_t)pline_buf;
            }
            else
            {
                return -1;
            }
        }
        else if(cis_info->l2b.fmt == CIS_YCBCR_420)
        {
            if(cis_info->window.enable)
            {
                if(cis_info->window.window_width * CIS_YUV420_PIXEL <= buf_len)
                {
                    cis_info->buf_len = cis_info->window.window_width * CIS_YUV420_PIXEL;
                    cis_info->line_buf_addr[0] = (uint32_t)pline_buf;
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                if(cis_info->frame_w * CIS_YUV420_PIXEL <= buf_len)
                {
                    cis_info->buf_len = cis_info->frame_w * CIS_YUV420_PIXEL;
                    cis_info->line_buf_addr[0] = (uint32_t)pline_buf;
                }
                else
                {
                    return -1;
                }
            }
        }
    }
    else
    {
        if(cis_info->scaling.enable)
        {
            switch(cis_info->scaling.pixel_skip)
            {
                case CIS_NO_SKIP_PIXEL:
                    cis_scal_ratio_w = 1;
                    break;
                case CIS_TWO_PIXEL_TO_ONE:
                    cis_scal_ratio_w = 2;
                    break;
                case CIS_FOUR_PIXEL_TO_ONE:
                    cis_scal_ratio_w = 4;
                    break;
            }
            switch(cis_info->scaling.line_skip)
            {
                case CIS_NO_SKIP_LINE:
                    cis_scal_ratio_h = 1;
                    break;
                case CIS_TWO_LINE_TO_ONE:
                    cis_scal_ratio_h = 2;
                    break;
                case CIS_FOUR_LINE_TO_ONE:
                    cis_scal_ratio_h = 4;
                    break;
            }
            if(cis_info->frame_w * 2 * 3 / cis_scal_ratio_w <= buf_len)
            {
                cis_info->buf_len = cis_info->frame_w * 2 / cis_scal_ratio_w;
                cis_info->line_buf_addr[0] = (uint32_t)pline_buf;
                cis_info->line_buf_addr[1] = (uint32_t)pline_buf + cis_info->buf_len;
                cis_info->line_buf_addr[2] = (uint32_t)pline_buf + cis_info->buf_len*2;
            }
            else
            {
                return -1;
            }
        }
        else if(cis_info->window.enable)
        {
            cis_info->buf_len = cis_info->window.window_width * 2;
            cis_info->line_buf_addr[0] = (uint32_t)pline_buf;
            cis_info->line_buf_addr[1] = (uint32_t)pline_buf + cis_info->buf_len;
            cis_info->line_buf_addr[2] = (uint32_t)pline_buf + cis_info->buf_len*2;
        }
        else
        {
            if(cis_info->frame_w * 2 * 3 <= buf_len)
            {
                cis_info->buf_len = cis_info->frame_w * 2;
                cis_info->line_buf_addr[0] = (uint32_t)pline_buf;
                cis_info->line_buf_addr[1] = (uint32_t)pline_buf + cis_info->buf_len;
                cis_info->line_buf_addr[2] = (uint32_t)pline_buf + cis_info->buf_len*2;
            }
            else
            {
                return -1;
            }
        }
    }
    return 0;
}

/**
 *  \brief cis initial
 *
 *  \param [in] cis_info : a pointer to a cis_handle_info_t structure
 *  \return -5 : CIS_INIT_IDMA_ERROR
 *          -4 : CIS_INIT_SET_REG_ERROR
 *          -3 : CIS_INIT_SCALE_ERROR
 *          -2 : CIS_INIT_WINDOW_ERROR
 *          -1 : CIS_INIT_ERROR
 *           0 : CIS_INIT_SUCCESS
 *
 *  \details cis initial script
 */
cis_init_status_t 
CIS_Init(
    cis_handle_info_t *cis_info,
    CB_CIS_REG_SET cb_cis_reg_set)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    cis_init_status_t ret = CIS_INIT_SUCCESS;
    
    pcis_info = cis_info;
    idma_cis_line_n = CIS_LINE_0;
    cis_en_flag = 0;
    href_cnt = 0;
    
    do{
        SN_CIS->CIS_CTRL_b.CIS_CLK_SEL = cis_info->cisdivclk;
        SN_CIS->CIS_CTRL_b.PIXEL_FMT = cis_info->pixel_fmt;

        SN_CIS->CIS_CTRL_b.CIS_PIXEL_CLK = cis_info->data_latch_edge;
        SN_CIS->CIS_CTRL_b.CIS_HERF = cis_info->hsync_valid_level;
        SN_CIS->CIS_CTRL_b.CIS_VSYNC = cis_info->vsync_valid_level;
        
        SN_CIS->CIS_CTRL_b.PCLK_SEL = 1;
        SN_CIS->CIS_CTRL_b.PCLK_LATCH = 1;
        
        /* Set Line0~2 Buffer address,if enable l2b mode only set line0 */
        if(cis_info->l2b.enable)
        {
            SN_CIS->LINE0_ADDR = cis_info->line_buf_addr[0];
            SN_CIS->CIS_CTRL_b.L2B_FMT = cis_info->l2b.fmt;
            SN_CIS->L2B_LINESKIP = cis_info->l2b.skipline;
            SN_CIS->CIS_CTRL_b.L2B_EN = 1;
        }
        else
        {
            SN_CIS->LINE0_ADDR = cis_info->line_buf_addr[0];
            SN_CIS->LINE1_ADDR = cis_info->line_buf_addr[1];
            SN_CIS->LINE2_ADDR = cis_info->line_buf_addr[2];
            SN_CIS->CIS_CTRL_b.L2B_EN = 0;
        }
        
        /* Clear Line Buffer Flag */
        _CIS_Clr_Line_Full_Flag(CIS_LINE_ALL);
        
        SN_CIS->CIS_CTRL_b.CIS_DATA = cis_info->data_fmt;
        SN_CIS->CIS_CTRL_b.TRANSPARENT_EN = cis_info->remove_transparent; 
        
        if(cis_info->window.enable)
        {
            if(cis_info->window.window_width + cis_info->window.w_offset > cis_info->frame_w ||
                cis_info->window.window_height + cis_info->window.h_offset > cis_info->frame_h ||
                _CIS_Window_Enable(true) == -1)
            {
                ret = CIS_INIT_WINDOW_ERROR;
                break;
            }
            _CIS_Window_Setting(cis_info->window.w_offset, cis_info->window.h_offset);
        }
        if(cis_info->scaling.enable)
        {
            if(_CIS_Scale_Enable(true))
            {
                ret = CIS_INIT_SCALE_ERROR;
                break;
            }
            if(cis_info->scaling.start_line_sel == CIS_START_LINE_1 &&
               cis_info->scaling.line_skip == CIS_NO_SKIP_LINE)
            {
                ret = CIS_INIT_SCALE_ERROR;
                break;
            }
            if(cis_info->scaling.start_pixel_sel == CIS_START_PIXEL_1 &&
               cis_info->scaling.pixel_skip == CIS_NO_SKIP_PIXEL)
            {
                ret = CIS_INIT_SCALE_ERROR;
                break;
            }
            SN_CIS->CIS_SCALE_SIZE_b.LINE_SEL = cis_info->scaling.start_line_sel;
            SN_CIS->CIS_SCALE_SIZE_b.LINE_SKIP = cis_info->scaling.line_skip;
            SN_CIS->CIS_SCALE_SIZE_b.PIXEL_SEL = cis_info->scaling.start_pixel_sel;
            SN_CIS->CIS_SCALE_SIZE_b.PIXEL_SKIP = cis_info->scaling.pixel_skip;
        }
        SN_CIS->CIS_CTRL_b.CIS_EN = 1;
        if(cb_cis_reg_set() != 0)
        {
            ret = CIS_INIT_SET_REG_ERROR;
            break;
        }
        if(cis_info->idma_burst == 0)
        {
            if(IDMA_Set_Burst_Unit(IDMA_CH_CIS, IDMA_BURST_UNIT_4B) == IDMA_RET_FAIL)
            {
                ret = CIS_INIT_IDMA_ERROR;
                break;
            }
        }
        else
        {
            if(IDMA_Set_Burst_Unit(IDMA_CH_CIS, cis_info->idma_burst) == IDMA_RET_FAIL)
            {
                ret = CIS_INIT_IDMA_ERROR;
                break;
            }
        }
        if(cis_info->scaling.enable)
        {
            cis_info->frame_w /= cis_scal_ratio_w;
            cis_info->frame_h /= cis_scal_ratio_h;
            if(_CIS_idma_Setting(cis_info->frame_w, cis_info->frame_h) != IDMA_RET_SUCCESS)
            {
                ret = CIS_INIT_IDMA_ERROR;
                break;
            }
        }
        else if(cis_info->window.enable)
        {
            cis_info->frame_w = cis_info->window.window_width;
            cis_info->frame_h = cis_info->window.window_height;
            if(_CIS_idma_Setting(cis_info->window.window_width, cis_info->window.window_height) != IDMA_RET_SUCCESS)
            {
                ret = CIS_INIT_IDMA_ERROR;
                break;
            }
        }
        else
        {
            if(_CIS_idma_Setting(cis_info->frame_w, cis_info->frame_h) != IDMA_RET_SUCCESS)
            {
                ret = CIS_INIT_IDMA_ERROR;
                break;
            }
        }
        NVIC_ClearPendingIRQ(CIS_DMA_IRQn); 
        NVIC_EnableIRQ(CIS_DMA_IRQn);
        NVIC_ClearPendingIRQ(CIS_VSYNC_IRQn);
        NVIC_EnableIRQ(CIS_VSYNC_IRQn);
        if(SN_CIS->CIS_CTRL_b.CIS_EN)
        {
            ret = CIS_INIT_SUCCESS;
            break;
        }
    }while(0);
    return ret;
}

/**
 *  \brief cis idma irq process
 *
 *  \param [in] cis_info : a pointer to an cis_handle_info_t structure
 *  \param [in] cb_empty_buf : empty line buffer callback
 *  \param [in] user_info : load user function/data... by pass to cb_empty_buf fucntion
 *  \return -1 : process error
 *          0 : process success
 *
 *  \details process cis idma buffer when idma buffer full
 */
cis_process_status_t 
CIS_Idma_Irq_Proc(
    CB_CIS_EMPTY_BUF cb_empty_buf, 
    void* user_info)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    cb_cis_info_t cb_info;
    cis_process_status_t ret = CIS_PROC_SUCCESS;
    do{
        cis_en_flag = 1;
        if(pcis_info->l2b.enable)
        {
            href_cnt++;
            if(_CIS_Chk_Line_Full_Flag(idma_cis_line_n)!=0)
            {
                _CIS_Clr_Line_Full_Flag(idma_cis_line_n);
                idma_cis_line_n++;
            }
            if(pcis_info->l2b.fmt == CIS_YCBCR_422)
            {
                if((href_cnt & 7) == 0)
                {
                    cb_info.buffer_addr = pcis_info->line_buf_addr[0];
                    cb_info.len = pcis_info->buf_len;
                    cb_info.ptunnel_info = user_info;
                    if(cb_empty_buf(href_cnt/8, &cb_info) != 0)
                    {
                        ret = CIS_CB_ERROR;
                        break;
                    }
                    pcis_info->line_buf_addr[0] = cb_info.buffer_addr;
                    SN_CIS->LINE0_ADDR = pcis_info->line_buf_addr[0];
                }
            }
            else if(pcis_info->l2b.fmt == CIS_YCBCR_420)
            {
                if((href_cnt & 15) == 0)
                {
                    cb_info.buffer_addr = pcis_info->line_buf_addr[0];
                    cb_info.len = pcis_info->buf_len;
                    cb_info.ptunnel_info = user_info;
                    if(cb_empty_buf(href_cnt/16, &cb_info) != 0)
                    {
                        ret = CIS_CB_ERROR;
                        break;
                    }
                    pcis_info->line_buf_addr[0] = cb_info.buffer_addr;
                    SN_CIS->LINE0_ADDR = pcis_info->line_buf_addr[0];
                }
            }
        }
        else
        {
            if(href_cnt < pcis_info->frame_h)
            {
                if(_CIS_Chk_Line_Full_Flag(idma_cis_line_n)!=0)
                {
                    _CIS_Clr_Line_Full_Flag(idma_cis_line_n);
                    
                    cb_info.buffer_addr = pcis_info->line_buf_addr[idma_cis_line_n];
                    cb_info.len = pcis_info->buf_len;
                    cb_info.ptunnel_info = user_info;
                    if(cb_empty_buf(href_cnt, &cb_info) != 0)
                    {
                        ret = CIS_CB_ERROR;
                        break;
                    }
                    pcis_info->line_buf_addr[idma_cis_line_n] = cb_info.buffer_addr;
                    
                    if(idma_cis_line_n==0)
                    {
                        SN_CIS->LINE0_ADDR = pcis_info->line_buf_addr[idma_cis_line_n];
                    }
                    else if(idma_cis_line_n==1)
                    {
                        SN_CIS->LINE1_ADDR = pcis_info->line_buf_addr[idma_cis_line_n];
                    }
                    else if(idma_cis_line_n==2)
                    {
                        SN_CIS->LINE2_ADDR = pcis_info->line_buf_addr[idma_cis_line_n];
                    }
                    idma_cis_line_n++;
                }
            }
            href_cnt++;
        }
    }while(0);
    if(idma_cis_line_n > CIS_LINE_2)
    {
        idma_cis_line_n = CIS_LINE_0;
    }
    return ret;
}

/**
 *  \brief cis vsync irq process
 *
 *  \param 
 *  \return -1 : process error
 *          0 : process success
 *
 *  \details process cis vsync signal
 */
cis_process_status_t 
CIS_Vsync_Irq_Proc(void)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);

    if(cis_en_flag){
        SN_CIS->CIS_CTRL_b.CIS_EN = false;
        
        if(SN_CIS->CIS_CTRL_b.CIS_EN)
        {
            return CIS_PROC_ERROR;
        }
        
        idma_cis_line_n = CIS_LINE_0;
        href_cnt = 0;
        SN_CIS->CIS_CTRL_b.CIS_EN = true;
        if(pcis_info->idma_burst == 0)
        {
            IDMA_Set_Burst_Unit(IDMA_CH_CIS, IDMA_BURST_UNIT_4B);
        }
        else
        {
            IDMA_Set_Burst_Unit(IDMA_CH_CIS, pcis_info->idma_burst);
        }
        IDMA_Restart(IDMA_CH_CIS);
        cis_en_flag = 0;
    }
    return CIS_PROC_SUCCESS;
}

/**
 *  \brief  cis deinitial
 *
 *  \param
 *  \return -1:deinitial fail
 *           0:deinitial success
 *
 *  \details
 */
int 
CIS_Deinit(void)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    SN_CIS->CIS_CTRL_b.CIS_EN = 0;
    if(SN_CIS->CIS_CTRL_b.CIS_EN != 1)
    {
        return 0;
    }
    return -1;
}

/**
 *  \brief get cis library version
 *
 *  \param 
 *  \return cis library version
 *
 *  \details
 */
uint32_t
CIS_GetVersion(void)
{
    return CIS_VERSION;
}
