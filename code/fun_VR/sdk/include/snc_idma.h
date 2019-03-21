/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_idma.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/05
 * @license
 * @description
 */

#ifndef __snc_idma_H_03e171a9_41eb_4334_994d_21b28333e13e__
#define __snc_idma_H_03e171a9_41eb_4334_994d_21b28333e13e__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"

    
//=============================================================================
//                  Constant Definition
//=============================================================================
#define DRAM_BASE   0x30000000
#define SRAM_BASE   0x18000000
#define SRAM_SIZE   0x40000

#define IDMA_ADDR_IGNORE    0xCCCCCCCC
    
typedef enum idma_ch
{
    IDMA_CH_0,              
    IDMA_CH_1,              
    IDMA_CH_SPIFC,          /*  DRAM Access Redirect */
    IDMA_CH_SDNF,           /*  DRAM Access Redirect */
    IDMA_CH_SDIO,           /*  DRAM Access Redirect */
    IDMA_CH_CIS,            
    IDMA_CH_CSC_IN,         /*  DRAM Access Redirect */
    IDMA_CH_CSC_OUT,        /*  DRAM Access Redirect */
    IDMA_CH_JPEG_ENC_IN,    
    IDMA_CH_JPEG_ENC_OUT,   /*  DRAM Access Redirect */
    IDMA_CH_JPEG_DEC_IN,    /*  DRAM Access Redirect */
    IDMA_CH_JPEG_DEC_OUT,   
    IDMA_CH_SPI0,           /*  DRAM Access Redirect */
    IDMA_CH_SPI1,           /*  DRAM Access Redirect */
} idma_ch_t;

typedef enum idma_transfer_ch
{
    IDMA_TRANSFER_CH_0,              
    IDMA_TRANSFER_CH_1,              
    IDMA_TRANSFER_CH_NONE,              
} idma_transfer_ch_t;

typedef enum idma_ret
{
    IDMA_RET_SUCCESS                = 0,
    IDMA_RET_BUSY,
    
    IDMA_RET_INVALID_CHANNEL,
    IDMA_RET_INVALID_ADDRESS,
    IDMA_RET_INVALID_LENGTH,
    IDMA_RET_INVALID_TRANSFER_BUF,
    IDMA_RET_UNALIGNED_ADDRESS,
    IDMA_RET_DRAM_CH_UNSELECTED,        /* config with: System_DRAM_Channel_Sel */
    IDMA_RET_TRANSFER_CH_UNSELECTED,    /* config with: System_DRAM_Channel_Sel */
    
    IDMA_RET_DRAM_REDIRECT,
    IDMA_RET_ERR_LOCAL_BUF_NOT_SRAM,
    IDMA_RET_FAIL,
} idma_ret_t;

typedef enum idma_cfg
{
    IDMA_CFG_CONTINUE_EN        = ( 1 << 0 ),
    IDMA_CFG_CONTINUE_DIS       = ( 1 << 1 ),
    
    IDMA_CFG_SCALAR_HALF        = ( 1 << 2 ),
    IDMA_CFG_SCALAR_A_QUARTER   = ( 1 << 3 ),
    IDMA_CFG_SCALAR_DIS         = ( 1 << 4 ),
    
    IDMA_CFG_CIS_AD_EN          = ( 1 << 5 ),
    IDMA_CFG_CIS_AD_DIS         = ( 1 << 6 ),
    
    IDMA_CFG_INVALID            = ( 1 << 7 ),
}idma_cfg_t;


//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef enum idma_burst
{
    IDMA_BURST_UNIT_4B      = 1,
    IDMA_BURST_UNIT_16B     = 4,
    IDMA_BURST_UNIT_32B     = 8,
    IDMA_BURST_UNIT_64B     = 16,
} idma_burst_t;

typedef enum idma_direction
{
    IDMA_DIRECTION_RAM_2_DIP,
    IDMA_DIRECTION_DIP_2_RAM,
} idma_direction_t;


typedef struct idma_transfer
{
    idma_transfer_ch_t    ch;
    uint32_t            *p_buff;
    uint32_t            size_buff;
}idma_transfer_t;


typedef struct idma_setting
{
    uint32_t            dip_addr;       // it MUST be 4-alignment
    uint32_t            ram_addr;       // it MUST be 4-alignment, set "IDMA_ADDR_IGNORE" 
                                        // to skip address check .
    idma_direction_t    direction;
    uint32_t            length;         // it MUST be 4-alignment
    
    idma_transfer_t     transfer; 
} idma_setting_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

typedef struct /* For idma1 - irq */
{
    idma_ch_t       ch_dev;
    idma_ch_t       ch_dma1;
    idma_setting_t  setting;
    uint32_t        done_size;
    volatile uint32_t is_busy;
}idma_task_t;


//=============================================================================
//                  Private Function Definition
//=============================================================================


//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  \brief  Start IDMA
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *  \param [in] pSetting    setting info
 *                              if (pSetting == 0), it will wait IDMA idle and return
 *  \return
 *
 *  \details
 */
uint32_t
IDMA_Start(
    idma_ch_t       channel,
    idma_setting_t  *pSetting);

    
/**
 *  \brief  Restart IDMA
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *  
 *  \return 0- success, 1- IDMA busy.
 *
 *  \details Restart the IDMA without resetting the register, usually used with continue mode.
 */
    
uint32_t
IDMA_Restart(idma_ch_t       channel);
    
/**
 *  \brief      Get IDMA status
 *
 *  \param [in] channel     enum idma_ch
 *  \return                 0: idle, other: busy
 *
 *  \details
 */
uint32_t
IDMA_Get_Status(
    idma_ch_t   channel);


/**
 *  \brief Configure the IDMA extra functionality.
 *  
 *  \param [in] channel : IDMA channel select
 *  \param [in] config  : enum idma_cfg_t, can be or by each selections.
 *  \return             : 0- success, else- fail
 *  
 */
uint32_t
IDMA_Extra_Config(
    idma_ch_t   channel,
    uint32_t    config);



/**
 *  \brief Set the burst number of DRAM, uint is 4Byte(Word).
 *  
 *  \param [in] channel     : IDMA channel select
 *  \param [in] burst_num   : Data word number in each burst transfer.
 *                            Range for IDMA 0/1 - From 1 to 256
 *                            Range for others DIP - From 1 to 16
 *  \return                 : 0- success, else- fail
 *  
 */

uint32_t
IDMA_Set_Burst_Unit(
    idma_ch_t       channel,
    idma_burst_t    burst_num);


    /**
 *  \brief  Stop IDMA transmission for special usage.
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *
 *  \details
 */
uint32_t
IDMA_Stop(
    idma_ch_t       channel);
/**
 *  \brief  Return current transsmit byte number of IDMA for special usage.
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *
 *  \return the complete byte number of current transmit DMA procedure.
 */
uint32_t
IDMA_Get_Cutrrent_Length(
    idma_ch_t       channel);
    
/**
 *  \brief: Return driver version.
 */
uint32_t IDMA_GetVersion(void);


    
#ifdef __cplusplus
}
#endif

#endif
