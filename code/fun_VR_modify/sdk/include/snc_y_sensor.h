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

#ifndef __snc_y_sensor_H_wRFYKH18_lPyp_HPzN_ssQW_uMqhIq1xc3pt__
#define __snc_y_sensor_H_wRFYKH18_lPyp_HPzN_ssQW_uMqhIq1xc3pt__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum y_sensor_idma_burst
{
    Y_SENSOR_DMA_BURST_LEN_1 =    1,      /* move data 1x4 bytes once, only work with extra ram */
    Y_SENSOR_DMA_BURST_LEN_4 =    4,      /* move data 4x4 bytes once, only work with extra ram */
} y_sensor_idma_burst_t;

typedef enum y_sensor_init_status
{
    Y_SENSOR_INIT_IDMA_ERROR =  -2,
    Y_SENSOR_INIT_ERROR =  -1,
    Y_SENSOR_INIT_SUCCESS = 0,
} y_sensor_init_status_t;

typedef enum y_sensor_process_status
{
    Y_SENSOR_CB_ERROR =      -2,
    Y_SENSOR_PROC_ERROR =  -1,
    Y_SENSOR_PROC_SUCCESS = 0,
} y_sensor_process_status_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct y_sensor_handle_info
{
    uint32_t line_buf_addr[3];              /* Get value by function OID_Set_Buffer, buffer address for OID IDMA to output OID data */
    uint32_t buf_len;                       /* Get value by function OID_Set_Buffer, buffer length for OID IDMA to output OID data,
                                               suggest length : 176 bytes per line */
    y_sensor_idma_burst_t idma_burst;            /* user input, enum cis_idma_burst_t */
} y_sensor_handle_info_t;

typedef struct cb_y_sensor_info
{
    uint32_t buffer_addr;
    uint32_t len;
    void *ptunnel_info;
}cb_y_sensor_info_t;

typedef int (*CB_Y_SENSOR_EMPTY_BUF)(uint32_t line_cnt, cb_y_sensor_info_t* cb_info);

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uint32_t Y_Sensor_Get_Buf_Len(void);
int Y_Sensor_Set_Buffer(y_sensor_handle_info_t *y_sensor_info, uint8_t *pline_buf, uint32_t buf_len);
y_sensor_init_status_t Y_Sensor_Init(y_sensor_handle_info_t *oid_info);
int Y_Sensor_Vsync_Irq_Proc(void);
int Y_Sensor_Idma_Irq_Proc(CB_Y_SENSOR_EMPTY_BUF cb_empty_buf, void* user_info);
int Y_Sensor_Deinit(void);

uint8_t Y_Sensor_PowerOn(uint16_t * ack_buf);
uint8_t Y_Sensor_TransmitCmd(uint8_t cmd, uint16_t * ack_buf);
uint8_t Write_Multi_Cmd(uint8_t cmd, uint16_t data1, uint16_t data2, uint16_t * ack_buf);

uint32_t Y_Sensor_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
