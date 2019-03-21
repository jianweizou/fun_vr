/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file cis_oid.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */


#include "snc_y_sensor.h"
#include "snc_idma.h"

#include "register_7320.h"
#define Y_SENSOR_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum y_sensor_line
{
    Y_SENSOR_LINE_0,                             /* select line 0 */
    Y_SENSOR_LINE_1,                             /* select line 1 */
    Y_SENSOR_LINE_2,                             /* select line 2 */
    Y_SENSOR_LINE_ALL,                           /* select all line */
} y_sensor_line_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
uint8_t y_sensor_en_flag = 0;
y_sensor_line_t idma_oid_line_n = Y_SENSOR_LINE_0;
uint32_t y_sensor_href_cnt = 0;
static y_sensor_handle_info_t *py_sensor_info;

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
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
    y_sensor_line_t idx)
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
    y_sensor_line_t idx)
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
 *  \brief oid idma setting
 *
 *  \param [in] uW_Size : frame width
 *  \param [in] uH_Size : frame height
 *  \return 
 *
 *  \details set oid idma buffer length (176*176)
 */
static void 
_OID_Idma_Setting(
    uint16_t w_size, 
    uint16_t h_size)
{
    idma_setting_t idma_cis_setting;
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    
    SN_CIS->CIS_COL_N=w_size/2;
    
    IDMA_Extra_Config(IDMA_CH_CIS, IDMA_CFG_CIS_AD_EN);
    idma_cis_setting.dip_addr = 0;
    idma_cis_setting.direction = IDMA_DIRECTION_DIP_2_RAM;
    idma_cis_setting.length = w_size * h_size;
    idma_cis_setting.ram_addr = IDMA_ADDR_IGNORE;
    
    IDMA_Start(IDMA_CH_CIS, &idma_cis_setting);
}

/**
 *  \brief get oid buffer length
 *
 *  \param [in] oid_info : a pointer to an oid_handle_info_t structure
 *  \return oid buffer length 
 *
 *  \details calculate oid buffer length
 */
uint32_t 
Y_Sensor_Get_Buf_Len(void)
{
    return 176 * 3;
}

/**
 *  \brief set oid idma buffer length
 *
 *  \param [in] oid_info : a pointer to an oid_handle_info_t structure
 *  \param [in] pline_buf : buffer address
 *  \param [in] buf_len : buffer length
 *  \return -1 : set buffer fail, buffer length not enough
 *           0 : set buffer success 
 *
 *  \details set buffer address and buffer length to structure oid_info
 */
int 
Y_Sensor_Set_Buffer(
    y_sensor_handle_info_t *y_sensor_info, 
    uint8_t *pline_buf, 
    uint32_t buf_len)
{
    if(176 * 3 <= buf_len)
    {
        y_sensor_info->buf_len = 176 * 3;
        y_sensor_info->line_buf_addr[0] = (uint32_t)pline_buf;
        y_sensor_info->line_buf_addr[1] = (uint32_t)pline_buf + 176;
        y_sensor_info->line_buf_addr[2] = (uint32_t)pline_buf + 176 * 2;
    }
    else
    {
        return -1;
    }
    return 0;
}

/**
 *  \brief oid initial
 *
 *  \param [in] oid_info : a pointer to an oid_handle_info_t structure
 *  \return -1 : initial fail
 *           0 : initial success 
 *
 *  \details oid frame width = 176 height = 176, only y data
 */
y_sensor_init_status_t 
Y_Sensor_Init(
    y_sensor_handle_info_t *y_sensor_info)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    py_sensor_info = y_sensor_info;
    idma_burst_t idma_brst;
    
    /* CIS Initialize */
    SN_CIS->CIS_CTRL_b.CIS_EN = true;
    SN_CIS->CIS_CTRL_b.OID_SEL = 3;
    SN_CIS->CIS_CTRL_b.OID_EN = 1;
    SN_CIS->CIS_CTRL_b.CIS_CLK_SEL = 1;
    SN_CIS->CIS_CTRL_b.PIXEL_FMT = 0;

    /* Data Format */
    SN_CIS->CIS_CTRL_b.CIS_PIXEL_CLK = 0;
    SN_CIS->CIS_CTRL_b.CIS_HERF = 0;
    SN_CIS->CIS_CTRL_b.CIS_VSYNC = 1;
    
    /* Line buffer address setting */
    SN_CIS->LINE0_ADDR = y_sensor_info->line_buf_addr[0];
    SN_CIS->LINE1_ADDR = y_sensor_info->line_buf_addr[1];
    SN_CIS->LINE2_ADDR = y_sensor_info->line_buf_addr[2];

    /* Clear Line Buffer Flag */
    _CIS_Clr_Line_Full_Flag(Y_SENSOR_LINE_ALL);

    SN_CIS->CIS_CTRL_b.CIS_DATA = 0;
    SN_CIS->CIS_CTRL_b.TRANSPARENT_EN = 0;
    if(y_sensor_info->idma_burst != 0)
    {
        switch(y_sensor_info->idma_burst)
        {
            case Y_SENSOR_DMA_BURST_LEN_1 :
                idma_brst = IDMA_BURST_UNIT_4B;
            break;
            case Y_SENSOR_DMA_BURST_LEN_4 :
                idma_brst = IDMA_BURST_UNIT_16B;
            break;
        }
        if(IDMA_Set_Burst_Unit(IDMA_CH_CIS, idma_brst) == IDMA_RET_FAIL)
        {
            return Y_SENSOR_INIT_IDMA_ERROR;
        }
    }

    _OID_Idma_Setting(176, 176);

    NVIC_ClearPendingIRQ(CIS_DMA_IRQn); 
    NVIC_EnableIRQ(CIS_DMA_IRQn);
    NVIC_ClearPendingIRQ(CIS_VSYNC_IRQn);
    NVIC_EnableIRQ(CIS_VSYNC_IRQn);
    if(SN_CIS->CIS_CTRL_b.OID_EN)
    {
        return Y_SENSOR_INIT_SUCCESS;
    }
    return Y_SENSOR_INIT_ERROR;
}

/**
 *  \brief oid idma irq process
 *
 *  \param [in] oid_info : a pointer to an oid_handle_info_t structure
 *  \param [in] cb_empty_buf : empty line buffer callback
 *  \param [in] user_info : load user function/data... by pass to cb_empty_buf fucntion
 *  \return 
 *
 *  \details process oid idma buffer when idma buffer full
 */
int 
Y_Sensor_Idma_Irq_Proc(
    CB_Y_SENSOR_EMPTY_BUF cb_empty_buf, 
    void* user_info)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    cb_y_sensor_info_t cb_info;
    y_sensor_process_status_t ret = Y_SENSOR_PROC_SUCCESS;
    y_sensor_en_flag = 1;
    do{
        if(y_sensor_href_cnt < 176)
        {
            if(_CIS_Chk_Line_Full_Flag(idma_oid_line_n)!=0)
            {
                _CIS_Clr_Line_Full_Flag(idma_oid_line_n);
                
                cb_info.buffer_addr = py_sensor_info->line_buf_addr[idma_oid_line_n];
                cb_info.len = py_sensor_info->buf_len;
                cb_info.ptunnel_info = user_info;
                if(cb_empty_buf(y_sensor_href_cnt, &cb_info)  != 0)
                {
                    ret = Y_SENSOR_CB_ERROR;
                    break;
                }
                py_sensor_info->line_buf_addr[idma_oid_line_n] = cb_info.buffer_addr;
                if(idma_oid_line_n==0)
                {
                    SN_CIS->LINE0_ADDR = py_sensor_info->line_buf_addr[idma_oid_line_n];
                }
                else if(idma_oid_line_n==1)
                {
                    SN_CIS->LINE1_ADDR = py_sensor_info->line_buf_addr[idma_oid_line_n];
                }
                else if(idma_oid_line_n==2)
                {
                    SN_CIS->LINE2_ADDR = py_sensor_info->line_buf_addr[idma_oid_line_n];
                }
                
                idma_oid_line_n++;
            }
        }
        y_sensor_href_cnt++;
    }while(0);
    if(idma_oid_line_n > Y_SENSOR_LINE_2)
    {
        idma_oid_line_n = Y_SENSOR_LINE_0;
    }
    return ret;
}

/**
 *  \brief oid vsync irq process
 *
 *  \param 
 *  \return 
 *
 *  \details process oid vsync signal
 */
int 
Y_Sensor_Vsync_Irq_Proc(void)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    y_sensor_process_status_t ret = Y_SENSOR_PROC_SUCCESS;
    do{
        if(y_sensor_en_flag){
            SN_CIS->CIS_CTRL_b.CIS_EN = false;
            
            if(SN_CIS->CIS_CTRL_b.CIS_EN)
            {
                ret = Y_SENSOR_PROC_ERROR;
                break;
            }
            idma_oid_line_n = Y_SENSOR_LINE_0;
            y_sensor_href_cnt = 0;
            
            SN_CIS->CIS_CTRL_b.CIS_EN = true;
            
            IDMA_Restart(IDMA_CH_CIS);
            y_sensor_en_flag = 0;
        }
    }while(0);
    return ret;
}

/**
 *  \brief  oid deinitial
 *
 *  \param
 *  \return -1:deinitial fail
 *           0:deinitial success
 *
 *  \details
 */
int 
Y_Sensor_Deinit(void)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    SN_CIS->CIS_CTRL_b.CIS_EN = 0;
    SN_CIS->CIS_CTRL_b.OID_EN = 0;
    if(SN_CIS->CIS_CTRL_b.CIS_EN != 1 && SN_CIS->CIS_CTRL_b.OID_EN != 1)
    {
        return 0;
    }
    return -1;
}

/**
 *  \brief get cis_oid library version
 *
 *  \param 
 *  \return cis_oid library version
 *
 *  \details
 */
uint32_t
Y_Sensor_GetVersion(void)
{
    return Y_SENSOR_VERSION;
}
