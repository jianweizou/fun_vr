/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_spifc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_spifc_H_wEP9js6k_lz1R_Ha4r_sIoy_uJw70pMjkN6R__
#define __snc_spifc_H_wEP9js6k_lz1R_Ha4r_sIoy_uJw70pMjkN6R__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define SPIFC_READ_STATUS               ((uint32_t)(-1))
#define SPIFC_WRITE_STATUS              ((uint32_t)(-2))
#define SPIFC_IGNORE_ARG_ADDR           ((uint32_t)(-1))


/**
 *  SPI FC divider. clock = PCLK / SPIFC_CLK_DIVIDER
 *  But H/W only support 40 MHz, it means using
 *  'SPIFC_CLK_DIVIDER_1' and 'SPIFC_CLK_DIVIDER_2' will make module crash in SN7320
 */
typedef enum spifc_clk
{
    SPIFC_CLK_DIVIDER_1     = 0,
    SPIFC_CLK_DIVIDER_2,
    SPIFC_CLK_DIVIDER_4,
    SPIFC_CLK_DIVIDER_8,
} spifc_clk_t;

/**
 *  spifc dma read mode
 */
typedef enum spifc_rd_mode
{
    SPIFC_RD_MODE_4X        = 0,
    SPIFC_RD_MODE_2X,
    SPIFC_RD_MODE_FAST,
    SPIFC_RD_MODE_1X,

} spifc_rd_mode_t;


/**
 *  spifc dma write mode
 */
typedef enum spifc_wr_mode
{
    SPIFC_WR_MODE_PAGE      = 0,
    SPIFC_WR_MODE_BYTE,
    SPIFC_WR_MODE_4X,
    SPIFC_WR_MODE_1X,

} spifc_wr_mode_t;

/**
 *  spifc erase mode
 */
typedef enum spifc_erase
{
    SPIFC_ERASE_SECTOR  = 0,
    SPIFC_ERASE_BLOCK,
    SPIFC_ERASE_ALL,
} spifc_erase_t;

/**
 *  spifc transfering data length
 */
typedef enum spifc_xfr_length
{
    SPIFC_XFR_0_BYTE    = 0,
    SPIFC_XFR_1_BYTE,
    SPIFC_XFR_2_BYTE,

} spifc_xfr_length_t;

/**
 *  spifc support max capacity
 *      - 3 bytes to access flash address (~ 16 MBytes, 128 Mbits)
 *      - 4 bytes to access flash address (~ 128 MBytes, 1 Gbits)
 */
typedef enum spifc_addr_mode
{
    SPIFC_ADDR_MODE_3_BYES  = 1,
    SPIFC_ADDR_MODE_4_BYES

} spifc_addr_mode_t;
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
 *  \brief  SPI FC initial, the GPIO setting, e.g. WP, HD, MUST before calling init
 *
 *  \param [in] divider     enum spifc_clk
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_Init(
    spifc_clk_t     divider);


/**
 *  \brief  Read flash data with customer cmd
 *
 *  \param [in] flash_cmd       flash cmd, only 8 bits is valid
 *  \param [in] flash_addr      address (index) of reading in flash,
 *                              you can use 'SPIFC_IGNORE_ARG_ADDR' to skip address
 *  \param [in] length          transfering length, enum spifc_xfr_length
 *  \return                     value in the target address
 *
 *  \details
 */
uint32_t
SPIFC_Read(
    uint32_t            flash_cmd,
    uint32_t            flash_addr,
    spifc_xfr_length_t  length);


/**
 *  \brief  Write data to flash with customer cmd
 *
 *  \param [in] flash_cmd       flash cmd, only 8 bits is valid
 *  \param [in] flash_addr      address (index) of writing in flash,
 *   *                              you can use 'SPIFC_IGNORE_ARG_ADDR' to skip address
 *  \param [in] length          transfering length, enum spifc_xfr_length
 *  \param [in] data            writing data
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_Write(
    uint32_t            flash_cmd,
    uint32_t            flash_addr,
    spifc_xfr_length_t  length,
    uint32_t            data);


/**
 *  \brief  Read flash data with dma
 *
 *  \param [in] mode            dma read mode (enum spifc_rd_mode)
 *  \param [in] flash_addr      address (index) of reading in flash
 *  \param [in] data_size       sequence data size
 *  \param [in] buf_addr        buffer address (system memory)
 *  \param [in] is_braking      braking until finish
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_ReadWithDma(
    spifc_rd_mode_t     mode,
    uint32_t            flash_addr,
    uint32_t            data_size,
    uint32_t            buf_addr,
    uint32_t            is_braking);


/**
 *  \brief  Write flash data with dma
 *
 *  \param [in] mode            dma read mode (enum spifc_wr_mode)
 *  \param [in] flash_addr      address (index) of writing in flash
 *  \param [in] data_size       sequence data size
 *  \param [in] buf_addr        buffer address (system memory)
 *  \param [in] is_braking      braking until finish
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_WriteWithDma(
    spifc_wr_mode_t     mode,
    uint32_t            flash_addr,
    uint32_t            data_size,
    uint32_t            buf_addr,
    uint32_t            is_braking);


/**
 *  \brief   Erase flash
 *
 *  \param [in] type            enum spifc_erase
 *  \param [in] flash_addr      adderess of flash
 *  \param [in] is_braking      brake until finish
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_Erase(
    spifc_erase_t   type,
    uint32_t        flash_addr,
    uint32_t        is_braking);


/**
 *  \brief      Read/Write status_register in a flash
 *
 *  \param [in] status      only 8 bits is valid (status = status & 0xFF)
 *                              - read status if (status == SPIFC_READ_STATUS)
 *                              - write status if (status == SPIFC_WRITE_STATUS)
 *  \return                 1. status of flash when read case
 *                          2. error code when write case
 *
 *  \details
 */
uint32_t
SPIFC_RWStatus(
    uint32_t        status);


/**
 *  \brief  Set flash mode of address length
 *
 *  \param [in] mode        enum spifc_addr_mode
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_SetAddrMode(
    spifc_addr_mode_t     mode);


/**
 *  \brief  Get factory id of flash
 *
 *  \return     0: unknown, other: factory id
 *
 *  \details
 */
uint32_t
SPIFC_GetFactoryId(void);


uint32_t
SPIFC_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif
