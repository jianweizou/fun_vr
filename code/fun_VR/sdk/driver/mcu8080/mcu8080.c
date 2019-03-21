/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file mcu8080.c
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/02/13
 * @license
 * @description
 */

#include "snc_mcu8080.h"
#include "reg_util.h"
#include "log.h"
#include "register_7320.h"

#define MCU8080_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

#define BIT_MCU8080_START           0
#define BIT_MCU8080_RW              1
#define BIT_MCU8080_PPUEN           8
#define BIT_MCU8080_DMODE           12
#define BIT_MCU8080_MPUEN           15

#define BIT_MCU8080_DMA_START       0
#define BIT_MCU8080_DMA_MODE        1

#define RW_MASK                     0x07
#define WRITE_CMD_MASK              0x05
#define WRITE_DATA_MASK             0x07
#define WRITE_DATA_HB_MASK          0xF
#define WRITE_DATA_LB_MASK          0x1F
#define WRITE_DATA_DMA_MASK         0x06
#define WRITE_DATA_DMA_HB_MASK      0x0E
#define WRITE_DATA_DMA_LB_MASK      0x1E

#define READ_DATA_MASK              0x03
#define READ_DATA_HB_MASK           0x0B
#define READ_DATA_LB_MASK           0x1B
#define READ_DATA_DMA_MASK          0x02
#define READ_DATA_DMA_HB_MASK       0x0A
#define READ_DATA_DMA_LB_MASK       0x1A
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
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Init(
    mcu8080_data_mode_t mode,
    uint8_t ui_HighPulse,
    uint8_t ui_LowPulse)
{
    int rval = 0;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*)SN_MCU_8080_BASE;

    // Enable mcu8080
    reg_set_bit(&pDev->MPU_8080_CTRL, BIT_MCU8080_MPUEN);

    // Set mcu8080 mode
    if(mode == MCU8080_8BIT_MODE)
    {
        reg_clear_bit(&pDev->MPU_8080_CTRL, BIT_MCU8080_DMODE);
    }

    else if(mode == MCU8080_16BIT_MODE)
    {
        reg_set_bit(&pDev->MPU_8080_CTRL, BIT_MCU8080_DMODE);
    }

    else
    {
        // Wrong Mode
        rval = -1;
        return rval;
    }

    // Set MCU 8080 WR# and RD# high low pulse Duration
    reg_write_mask_bits(&pDev->MPU_8080_WIDTH_CTRL, ui_HighPulse<<8, 0xFF<<8);
    reg_write_mask_bits(&pDev->MPU_8080_WIDTH_CTRL, ui_LowPulse, 0xFF);

    return rval;
}

/**
 *  \brief  mcu8080 deinit
 *
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Deinit(void)
{
    int rval = 0;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*)SN_MCU_8080_BASE;
    
    reg_clear_bit(&pDev->MPU_8080_CTRL, BIT_MCU8080_MPUEN);

    return rval;
}

/**
 *  \brief  Set MCU 8080 source from PPU
 *
 *  \return                 0: ok, other: fail
 *
 *  \details This function must enable at last step of MCU 8080 initialize
 */
int
MCU8080_From_PPU(void)
{
    int rval = 0;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*)SN_MCU_8080_BASE;
    
    reg_set_bit(&pDev->MPU_8080_CTRL, BIT_MCU8080_PPUEN);
    return rval;
}

/**
 *  \brief  Set MCU 8080 Write Command
 *
 *  \param [in] ui_CMD      command write to mcu8080 ram
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
MCU8080_Write_Command(
    uint16_t ui_CMD)
{  
    int rval = 0;
    uint16_t uiTemp;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*)SN_MCU_8080_BASE;
    
    reg_write_bits(&pDev->MPU_8080_DATA, ui_CMD);
    uiTemp = (pDev->MPU_8080_CTRL) & 0xFFFF;

    reg_write_bits(&pDev->MPU_8080_CTRL, reg_read_bits(&pDev->MPU_8080_CTRL) & 0xFF00);
    //Write command
    reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_CMD_MASK, RW_MASK);
    while(pDev->MPU_8080_CTRL & 0x01);
    reg_write_bits(&pDev->MPU_8080_CTRL, uiTemp);

    return rval;
}

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
    uint16_t ui_Data)
{
    int rval = 0;
    uint16_t uiTemp = 0;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*)SN_MCU_8080_BASE;

    reg_write_bits(&pDev->MPU_8080_DATA, ui_Data);
    uiTemp = (pDev->MPU_8080_CTRL) & 0xFFFF;
    reg_write_bits(&pDev->MPU_8080_CTRL, reg_read_bits(&pDev->MPU_8080_CTRL) & 0xFF00);

    switch(mode)
    {
        //Write 16-bit data
        case MCU8080_WRITE_DATA:
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_MASK, RW_MASK);
        break;

        //Write 16-bit data, Low byte first  
        case MCU8080_WRITE_DATA_LB:
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_LB_MASK, WRITE_DATA_LB_MASK);
        break;

        //Write 16-bit data, High byte first
        case MCU8080_WRITE_DATA_HB:
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_HB_MASK, WRITE_DATA_HB_MASK);
        break;

        default:
        break;
    }

    while(pDev->MPU_8080_CTRL & 0x01);
    reg_write_bits(&pDev->MPU_8080_CTRL, uiTemp);

    return rval;
}

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
    mcu_8080_read_mode_t mode)
{
    sn_mcu8080_t *pDev = (sn_mcu8080_t*)SN_MCU_8080_BASE;

    reg_write_bits(&pDev->MPU_8080_CTRL, reg_read_bits(&pDev->MPU_8080_CTRL) & 0xFF00);

    switch(mode)
    {
        case MCU8080_READ_DATA:
            reg_write_mask_bits(&pDev->MPU_8080_DMA_CTRL, READ_DATA_MASK, RW_MASK);
        break;

        case MCU8080_READ_DATA_LB:
            reg_write_mask_bits(&pDev->MPU_8080_DMA_CTRL, READ_DATA_LB_MASK, READ_DATA_LB_MASK);
        break;

        case MCU8080_READ_DATA_HB:
            reg_write_mask_bits(&pDev->MPU_8080_DMA_CTRL, READ_DATA_HB_MASK, READ_DATA_HB_MASK);
        break;

        default:
        break;

    }

    while(pDev->MPU_8080_CTRL & 0x01);
    return pDev->MPU_8080_DATA;
}

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
    uint32_t ui_Length)
{
    int rval = 0;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*) SN_MCU_8080_BASE;
    reg_write_bits(&pDev->MPU_8080_CTRL, reg_read_bits(&pDev->MPU_8080_CTRL) & 0xFF00);

    switch(mode)
    {
        case MCU8080_WRITE_DATA:
            /*MPURW : Write data*/
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_DMA_MASK, WRITE_DATA_DMA_MASK);
            break;
        
        case MCU8080_WRITE_DATA_LB:
            /*MPURW : Write data*/
            /*ASRM : HW R/W 2 bytes to MPUDATA/device*/
            /*HLF : Low byte first*/
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_DMA_LB_MASK, WRITE_DATA_DMA_LB_MASK);
            break;
        
        case MCU8080_WRITE_DATA_HB:
            /*MPURW : Write data*/
            /*ASRM : HW R/W 2 bytes to MPUDATA/device*/
            
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_DMA_HB_MASK, WRITE_DATA_DMA_HB_MASK);
            break;
        
        default:
            //MPURW : Write data
            break;
    }
    
    reg_write_bits(&pDev->MPU_8080_DMA_RAM_ADDR, (uint32_t)p_ui_WRAMAddr);
    reg_write_bits(&pDev->MPU_8080_DMA_LEN, ui_Length);
    reg_set_bit(&pDev->MPU_8080_DMA_CTRL, BIT_MCU8080_DMA_MODE);//RAM->8080(Write mode)
    reg_set_bit(&pDev->MPU_8080_DMA_CTRL, BIT_MCU8080_DMA_START);
    while(pDev->MPU_8080_DMA_CTRL_b.START);

    return rval;
}

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
    uint32_t ui_Length)
{
    int rval = 0;
    sn_mcu8080_t *pDev = (sn_mcu8080_t*) SN_MCU_8080_BASE;
    reg_write_bits(&pDev->MPU_8080_CTRL, reg_read_bits(&pDev->MPU_8080_CTRL) & 0xFF00);

    switch(ui_Mode)
    {
        // TODO:Need to check read dma mask

        case MCU8080_READ_DATA:
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, READ_DATA_DMA_MASK, READ_DATA_DMA_MASK);
//            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_DMA_MASK, WRITE_DATA_DMA_MASK);
            break;
        case MCU8080_READ_DATA_LB:
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, READ_DATA_DMA_LB_MASK, READ_DATA_DMA_LB_MASK);
//            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_DMA_LB_MASK, WRITE_DATA_DMA_LB_MASK);
            break;
        case MCU8080_READ_DATA_HB:
            reg_write_mask_bits(&pDev->MPU_8080_CTRL, READ_DATA_DMA_HB_MASK, READ_DATA_DMA_HB_MASK);
//            reg_write_mask_bits(&pDev->MPU_8080_CTRL, WRITE_DATA_DMA_HB_MASK, WRITE_DATA_DMA_HB_MASK);
            break;
        default:
            break;
    }
    
    reg_write_bits(&pDev->MPU_8080_DMA_RAM_ADDR, (uint32_t)p_ui_WRAMAddr);
    reg_write_bits(&pDev->MPU_8080_DMA_LEN, ui_Length);
    reg_clear_bit(&pDev->MPU_8080_DMA_CTRL, BIT_MCU8080_DMA_MODE);//8080->RAM(Read mode)
    reg_set_bit(&pDev->MPU_8080_DMA_CTRL, BIT_MCU8080_DMA_START);
    while(pDev->MPU_8080_DMA_CTRL_b.START);

    return rval;
}

/**
 *  \brief  Get the version of driver
 *
 *  \return         Version
 *
 *  \details
 */
uint32_t 
MCU8080_GetVersion(void)
{
    return MCU8080_VERSION;
}
