/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_mcu8080.h
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/04/27
 * @license
 * @description
 */

#ifndef __snc_mcu8080_H_wLDHv8xo_lKKv_Hyya_szK5_uX53U3pavqGu__
#define __snc_mcu8080_H_wLDHv8xo_lKKv_Hyya_szK5_uX53U3pavqGu__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

/**
 *  mcu8080 mode setting
 *  0: 8 bit mode, 1: 16 bit mode
 */
typedef enum mcu8080_data_mode
{
    MCU8080_8BIT_MODE =    0,
    MCU8080_16BIT_MODE
} mcu8080_data_mode_t;

/**
 *  mcu8080 read mode setting
 *  0: read 16-bit data, 1: read data low byte first(8-bit bus only), 2: read data high byte first(8-bit bus only)
 */
typedef enum mcu_8080_read_mode
{
    MCU8080_READ_DATA = 0,
    MCU8080_READ_DATA_LB,
    MCU8080_READ_DATA_HB
} mcu_8080_read_mode_t;

/**
 *  mcu8080 write mode setting
 *  0: write 16-bit data, 1: write data low byte first(8-bit bus only), 2: write data high byte first(8-bit bus only)
 */
typedef enum mcu_8080_write_mode
{
    MCU8080_WRITE_DATA = 0,
    MCU8080_WRITE_DATA_LB,
    MCU8080_WRITE_DATA_HB
} mcu_8080_write_mode_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

/**
 *  \brief  mcu8080 init
 *
 *  \param [in] mode            enum mcu8080_data_mode
 *  \param [in] ui_HighPulse    high pulse duration
 *  \param [in] ui_LowPulse     low pulse duration 
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Init(
    mcu8080_data_mode_t mode,
    uint8_t ui_HighPulse,
    uint8_t ui_LowPulse);

/**
 *  \brief  mcu8080 deinit
 *
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Deinit(void);

/**
 *  \brief  Set MCU 8080 source from PPU
 *
 *  \return                     0: ok, other: fail
 *
 *  \details This function must enable at last step of MCU 8080 initialize
 */
int
MCU8080_From_PPU(void);

/**
 *  \brief  Set MCU 8080 Write Command
 *
 *  \param [in] ui_CMD          command write to mcu8080 ram
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Write_Command(
    uint16_t ui_CMD);

/**
 *  \brief  Send MCU 8080 16-bit width data to data bus
 *
 *  \param [in] mode            enum mcu_8080_write_mode_t
 *  \param [in] uiData          data
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Write_Data(
    mcu_8080_write_mode_t mode,
    uint16_t ui_Data);

/**
 *  \brief  Read MCU 8080 16-bit width data from data bus
 *
 *  \param [in] mode            enum mcu_8080_read_mode_t
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Read_Data(
    mcu_8080_read_mode_t mode);

/**
 *  \brief  Write Data to MCU 8080 via DMA
 *
 *  \param [in] mode            enum mcu_8080_write_mode_t
 *  \param [in] p_ui_WRAMAddr   DAM ram address
 *  \param [in] ui_Length       data length (word)
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_DMA_Write_Mode(
    mcu_8080_write_mode_t mode,
    uint32_t* p_ui_WRAMAddr,
    uint32_t ui_Length);

/**
 *  \brief  Read Data from MCU 8080 via DMA
 *
 *  \param [in] mode            enum mcu_8080_read_mode_t
 *  \param [in] p_ui_WRAMAddr   DAM ram address
 *  \param [in] ui_Length       data length (word)
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_DMA_Read_Mode(
    mcu_8080_read_mode_t ui_Mode,
    uint32_t* p_ui_WRAMAddr,
    uint32_t ui_Length);

/**
 *  \brief  Get the version of driver
 *
 *  \return         Version
 *
 *  \details
 */
uint32_t 
MCU8080_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
