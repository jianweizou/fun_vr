/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file spifc.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/25
 * @license
 * @description
 */


#include "snc_spifc.h"
#include "reg_util.h"
#include "snc_idma.h"


#include "register_7320.h"
#define SPIFC_VERSION       0x73200000

//=============================================================================
//                  Constant Definition
//=============================================================================


#define BIT_SPIFC_QUAD_MODE                 14
#define BIT_SPIFC_ENABLE                    15

#define OFFSET_RD_MODE_BIT                  4

#define BIT_SPIFC_SEND_CUSTOMER_CMD         4
#define BIT_SPIFC_SEND_ADDR                 5
#define BIT_SPIFC_SEND_CUSTOMER_RD_CMD      7
#define BIT_SPIFC_SEND_ERASE_ALL_CMD        8
#define BIT_SPIFC_SEND_ERASE_BLOCK_CMD      9
#define BIT_SPIFC_SEND_ERASE_SECTOR_CMD     10
#define BIT_SPIFC_SEND_WRITE_STATUS_CMD     11
#define BIT_SPIFC_SEND_WAKE_UP_CMD          12
#define BIT_SPIFC_SEND_POWER_DOWN_CMD       13
#define BIT_SPIFC_SEND_READ_STATUS_CMD      14
#define BIT_SPIFC_SEND_READ_ID_CMD          15

#define MSK_SPIFC_BUSY                      0x00000002
#define MSK_SPIFC_QUAD_MODE                 0x00004000

#define MSK_SPIFC_CUSTOMER_CMD              0x00000010
#define MSK_SPIFC_WAKE_UP_CMD               0x00001000
#define MSK_SPIFC_POWER_DOWN_CMD            0x00002000
#define MSK_SPIFC_READ_STATUS_CMD           0x00004000
#define MSK_SPIFC_WRITE_STATUS_CMD          0x00000800

#define MSK_SPIFC_ADDR_MODE                 0x00000006
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
static uint32_t
_SPIFC_RWStatus(
    sn_flash_t      *pDev,
    uint32_t        status)
{
    uint32_t    state = 0;

    if( status != SPIFC_READ_STATUS )
    {
        reg_write_bits(&pDev->FLASH_CMD_DATA, (status & 0xFF));
        reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_WRITE_STATUS_CMD);  // send write_state cmd
        while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_WRITE_STATUS_CMD) ) {}
    }

    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_READ_STATUS_CMD);  // send read_state cmd
    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_READ_STATUS_CMD) ) {}

    state = reg_read_bits(&pDev->FLASH_STAT);

    return (state & 0xFF);
}
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
    spifc_clk_t     divider)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    do {
        if( divider == SPIFC_CLK_DIVIDER_1 || divider == SPIFC_CLK_DIVIDER_2 )
        {
            rval = -1;
            break;
        }

        // reg_clear_bit(&pDev->FLASH_CTRL, 15);

        // set clock divider
        reg_write_mask_bits(&pDev->FLASH_CTRL, (divider << 12), (0x3 << 12));

        // enable
        reg_set_bit(&pDev->FLASH_CTRL, BIT_SPIFC_ENABLE);

        // default 3 bytes address mode
        reg_write_mask_bits(&pDev->FLASH_MODE, (SPIFC_ADDR_MODE_3_BYES << 1), MSK_SPIFC_ADDR_MODE);

        /**
         *  send wake_up cmd
         *  ps. rom code already done
         */
        // reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_WAKE_UP_CMD);
        // while( reg_read_bits(&pDev->FLASH_CMD) & MSK_SPIFC_WAKE_UP_CMD ) {}

    } while(0);

    return rval;
}

/**
 *  \brief  SPI FC de-initial
 *
 *  \param [in] is_power_down   send power down command to SPI FLASH or not
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
SPIFC_Deinit(
    uint32_t    is_power_down)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    #if 0
    reg_clear_bit(&pDev->FLASH_CTRL, BIT_SPIFC_ENABLE);

    // send power down cmd
    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_POWER_DOWN_CMD);
    #else

    reg_clear_bit(&pDev->FLASH_CTRL, BIT_SPIFC_ENABLE);

    if( is_power_down )
    {
        // send power down cmd
        reg_write_bits(&pDev->FLASH_CMD, (0x1 << BIT_SPIFC_SEND_POWER_DOWN_CMD));
        while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_POWER_DOWN_CMD) ) {}
    }
    #endif

    return rval;
}

/**
 *  \brief  Read flash data with customer cmd
 *
 *  \param [in] flash_cmd       flash cmd, only 8 bits is valid
 *  \param [in] flash_addr      address (index) of reading in flash
 *  \param [in] length          transfor length, enum spifc_xfr_length
 *  \return                     value in the target address
 *
 *  \details
 */
uint32_t
SPIFC_Read(
    uint32_t            flash_cmd,
    uint32_t            flash_addr,
    spifc_xfr_length_t  length)
{
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    // flash cmd 0x06: writing_enable
    reg_write_bits(&pDev->FLASH_CMD_DATA, 0x06);

    // communicate with flash
    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_CUSTOMER_CMD);
    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_CUSTOMER_CMD) ) {}

    // get read_status
    while( (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ) {}

    reg_write_bits(&pDev->FLASH_CMD_DATA, 0x00);

    // send flash cmd
    reg_write_bits(&pDev->FLASH_CMD_DATA, (flash_cmd & 0xFF));
    reg_write_mask_bits(&pDev->FLASH_CMD, (length << 2), (0x3 << 2));
    if( flash_addr == SPIFC_IGNORE_ARG_ADDR )
        reg_clear_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ADDR);
    else
    {
        reg_write_bits(&pDev->FLASH_ADDRH, ((uint32_t)flash_addr >> 16));
        reg_write_bits(&pDev->FLASH_ADDRL, ((uint32_t)flash_addr & 0xFFFF));
        reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ADDR);
    }

    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_CUSTOMER_RD_CMD);

    // communicate with flash
    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_CUSTOMER_CMD);
    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_CUSTOMER_CMD) ) {}

    // get read_status
    while( (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ) {}

    reg_write_bits(&pDev->FLASH_CMD_DATA, 0x00);

    return (length == SPIFC_XFR_0_BYTE) ? 0 : (reg_read_bits(&pDev->FLASH_RDATA) & 0xFFFF);
}

/**
 *  \brief  Write data to flash with customer cmd
 *
 *  \param [in] flash_cmd       flash cmd, only 8 bits is valid
 *  \param [in] flash_addr      address (index) of writing in flash
 *  \param [in] length          transfor length, enum spifc_xfr_length
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
    uint32_t            data)
{
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    // flash cmd 0x06: writing_enable
    reg_write_bits(&pDev->FLASH_CMD_DATA, 0x06);

    // communicate with flash
    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_CUSTOMER_CMD);
    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_CUSTOMER_CMD) ) {}

    // get read_status
    while( (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ) {}

    // send flash cmd
    data = (length == SPIFC_XFR_0_BYTE) ? 0 : (data & 0xFFFF);
    reg_write_bits(&pDev->FLASH_WDATA, data);
    reg_write_bits(&pDev->FLASH_CMD_DATA, (flash_cmd & 0xFF));
    reg_write_mask_bits(&pDev->FLASH_CMD, (length << 2), (0x3 << 2));

    if( flash_addr == SPIFC_IGNORE_ARG_ADDR )
        reg_clear_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ADDR);
    else
    {
        reg_write_bits(&pDev->FLASH_ADDRH, ((uint32_t)flash_addr >> 16));
        reg_write_bits(&pDev->FLASH_ADDRL, ((uint32_t)flash_addr & 0xFFFF));
        reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ADDR);
    }

    reg_clear_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_CUSTOMER_RD_CMD);

    // communicate with flash
    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_CUSTOMER_CMD);
    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_CUSTOMER_CMD) ) {}

    // get read_status
    while( (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ) {}

    reg_write_bits(&pDev->FLASH_WDATA, 0);
    reg_write_bits(&pDev->FLASH_CMD_DATA, 0);

    return 0;
}

/**
 *  \brief  Read flash data with dma
 *
 *  \param [in] mode            dma read mode (enum spifc_rd_mode)
 *  \param [in] flash_addr      address (index) of writing in flash
 *  \param [in] data_size       sequence data size
 *  \param [in] buf_addr       destination address (system memory)
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
    uint32_t            is_braking)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    // wait IMDA idle
    IDMA_Start(IDMA_CH_SPIFC, 0);
    reg_write_mask_bits(&pDev->FLASH_CTRL, 0x0, 0x00FF);

    reg_write_bits(&pDev->FLASH_WDATA, 0);
    reg_write_bits(&pDev->FLASH_CMD_DATA, 0);

    if( mode == SPIFC_RD_MODE_4X )
        reg_set_bit(&pDev->FLASH_CTRL, BIT_SPIFC_QUAD_MODE);
    else
        reg_clear_bit(&pDev->FLASH_CTRL, BIT_SPIFC_QUAD_MODE);

    // set dma read mode
    mode += OFFSET_RD_MODE_BIT;
    reg_write_mask_bits(&pDev->FLASH_CTRL, (0x1 << mode), (0x1 << mode));

    {
        idma_setting_t  setting = {0};

        setting.dip_addr   = flash_addr | 0x60000000;
        setting.ram_addr   = buf_addr;
        setting.length     = data_size;
        setting.direction  = IDMA_DIRECTION_DIP_2_RAM;

        IDMA_Start(IDMA_CH_SPIFC, &setting);

        if( is_braking )    IDMA_Start(IDMA_CH_SPIFC, 0);
    }

    // wait for finish
    while( is_braking )
    {
        is_braking = (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ? 1 : 0;
    }
    return rval;
}

/**
 *  \brief  Write flash data with dma
 *
 *  \param [in] mode            dma read mode (enum spifc_wr_mode)
 *  \param [in] flash_addr      address (index) of writing in flash
 *  \param [in] data_size       sequence data size
 *  \param [in] buf_addr        source address (system memory)
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
    uint32_t            is_braking)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    // wait IMDA idle
    IDMA_Start(IDMA_CH_SPIFC, 0);
    reg_write_mask_bits(&pDev->FLASH_CTRL, 0x0, 0x000F);

    reg_write_bits(&pDev->FLASH_WDATA, 0);
    reg_write_bits(&pDev->FLASH_CMD_DATA, 0);

    if( mode == SPIFC_WR_MODE_4X )
        reg_set_bit(&pDev->FLASH_CTRL, BIT_SPIFC_QUAD_MODE);
    else
        reg_clear_bit(&pDev->FLASH_CTRL, BIT_SPIFC_QUAD_MODE);

    // set dma write mode
    reg_write_mask_bits(&pDev->FLASH_CTRL, (0x1 << mode), (0x1 << mode));

    {
        idma_setting_t  setting = {0};

        setting.dip_addr   = flash_addr | 0x60000000;
        setting.ram_addr   = buf_addr;
        setting.length     = data_size;
        setting.direction  = IDMA_DIRECTION_RAM_2_DIP;

        IDMA_Start(IDMA_CH_SPIFC, &setting);

        if( is_braking )    IDMA_Start(IDMA_CH_SPIFC, 0);
    }

    // wait for finish
    while( is_braking )
    {
        is_braking = (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ? 1 : 0;
    }
    return rval;
}

/**
 *  \brief   Erase flash
 *
 *  \param [in] type            enum spifc_erase
 *  \param [in] flash_addr      adderess of flash
 *  \param [in] is_braking      brake until finish
 *  \return
 *
 *  \details
 */
int
SPIFC_Erase(
    spifc_erase_t   type,
    uint32_t        flash_addr,
    uint32_t        is_braking)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    flash_addr >>= 1;
    reg_write_bits(&pDev->FLASH_ADDRH, ((uint32_t)flash_addr >> 16));
    reg_write_bits(&pDev->FLASH_ADDRL, ((uint32_t)flash_addr & 0xFFFF));

    if( type == SPIFC_ERASE_SECTOR )        reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ERASE_SECTOR_CMD);
    else if( type == SPIFC_ERASE_BLOCK )    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ERASE_BLOCK_CMD);
    else if( type == SPIFC_ERASE_ALL )      reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_ERASE_ALL_CMD);

    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_BUSY) ) {}

    // wait for finish
    while( is_braking )
    {
        is_braking = (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ? 1 : 0;
    }

    return rval;
}

/**
 *  \brief      Read/Write status_register in a flash
 *
 *  \param [in] status      only 8 bits is valid (status = status & 0xFF)
 *                              - read status if (status == SPIFC_IGNORE_STATUS)
 *                              - write status if (status != SPIFC_IGNORE_STATUS)
 *  \return                 status of flash
 *
 *  \details
 */
uint32_t
SPIFC_RWStatus(
    uint32_t        status)
{
    uint32_t        status_tmp = status;
    uint32_t        data = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

#if 0
    while( (data = _SPIFC_RWStatus(pDev, status_tmp)) & 0x1 )
    {
        if( status == SPIFC_READ_STATUS )
            break;

        status_tmp = SPIFC_READ_STATUS;
    }
#else
    data = _SPIFC_RWStatus(pDev, status_tmp);
#endif
    return data;
}

/**
 *  \brief  Enable flash Quad mode
 *
 *  \param [in] is_enable   enable or not
 *  \return
 *
 *  \details
 */
int
SPIFC_SetQuadMode(
    uint32_t     is_enable)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    if( is_enable )
        reg_set_bit(&pDev->FLASH_CTRL, BIT_SPIFC_QUAD_MODE);
    else
        reg_clear_bit(&pDev->FLASH_CTRL, BIT_SPIFC_QUAD_MODE);

    return rval;
}

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
    spifc_addr_mode_t     mode)
{
    int             rval = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    mode = (spifc_addr_mode_t)((mode & 0x3) << 1);

    reg_write_mask_bits(&pDev->FLASH_MODE, mode, MSK_SPIFC_ADDR_MODE);

    #if 0 // debug check
    mode >>= 1;
    if( mode == SPIFC_ADDR_MODE_3_BYES )
    {
        while( reg_read_mask_bits(&pDev->FLASH_MODE, 0x1) ) {}
    }
    else if( mode == SPIFC_ADDR_MODE_4_BYES )
    {
        while( !(reg_read_mask_bits(&pDev->FLASH_MODE, 0x1)) ) {}
    }
    #endif

    return rval;
}

/**
 *  \brief  Get factory id of flash
 *
 *  \return     0: unknown, other: factory id
 *
 *  \details
 */
uint32_t
SPIFC_GetFactoryId(void)
{
    uint32_t        data = 0;
    sn_flash_t      *pDev = (sn_flash_t*)SN_SPIFC_BASE;

    // send flash cmd: get_factory_id
    reg_set_bit(&pDev->FLASH_CMD, BIT_SPIFC_SEND_READ_ID_CMD);
    while( reg_read_mask_bits(&pDev->FLASH_CMD, MSK_SPIFC_BUSY) ) {}

    while( (_SPIFC_RWStatus(pDev, SPIFC_READ_STATUS) & 0x1) ) {}

    data = (reg_read_bits(&pDev->FLASH_ID) & 0xFFFF) << 8 |
           (reg_read_bits(&pDev->FLASH_RDATA) & 0xFFFF) >> 8;
    return data;
}

#if 0
int
spifc_get_capacity()
{
}
#endif

uint32_t
SPIFC_GetVersion(void)
{
    return SPIFC_VERSION;
}




