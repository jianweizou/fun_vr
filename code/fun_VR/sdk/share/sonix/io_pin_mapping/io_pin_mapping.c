/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file io_pin_mapping.c
 *
 * @author James Lin
 * @version 0.1
 * @date 2018/06/07
 * @license
 * @description
 */

#include <string.h>
#include "snc_types.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
//System
#define REG_SYS0_PIN_CTRL               0x45000020

//=============================================================================
//                  Macro Definition
//=============================================================================                                      }while(0)

/**
 *  register operator
 */
#define reg_mask_write(pAddr, value, mask)   \
    do{ *(volatile uint32_t*)(pAddr) = (*(volatile uint32_t*)(pAddr) & ~mask) | (value & mask); } while(0)

#define reg_write(pAddr, value)     do{ *(volatile uint32_t*)(pAddr) = (value); } while(0)
#define reg_set_bit(pAddr, bit_idx) do{ *(volatile uint32_t*)(pAddr) |= (0x1UL << (bit_idx)); }while(0)
#define reg_clr_bit(pAddr, bit_idx) do{ *(volatile uint32_t*)(pAddr) &= (~(0x1UL << (bit_idx))); }while(0)


//=============================================================================
//                  Structure Definition
//=============================================================================
typedef int (*INIT_FUNC)(void);

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

/**
 *  Initial I/O mapping SD/SDIO.
 */
#if (defined(CONFIG_ENABLE_SD_0) && (CONFIG_ENABLE_SD_0)) || \
    (defined(CONFIG_ENABLE_SDIO) && (CONFIG_ENABLE_SDIO))

static int
_init_io_pin_sd(void)
{
    // pin switch
#if (CONFIG_ENABLE_SD_0)

    #if (CONFIG_PINS_GROUP_ID_SD0 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 0);
    #elif (CONFIG_PINS_GROUP_ID_SD0 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 0);
    #endif
#endif

    return 0;
}
#else
    #define _init_io_pin_sd      0
#endif


/**
 *  Initial I/O mapping SPI DMA.
 */
#if defined(CONFIG_ENABLE_SPI_DMA_0) && (CONFIG_ENABLE_SPI_DMA_0)

static int
_init_io_pin_spi_dma(void)
{
    // pin switch
#if (CONFIG_PINS_GROUP_ID_SPI_DMA_0 == 1)
    reg_clr_bit(REG_SYS0_PIN_CTRL, 8);
#elif (CONFIG_PINS_GROUP_ID_SPI_DMA_0 == 2)
    reg_set_bit(REG_SYS0_PIN_CTRL, 8);
#endif

    return 0;
}
#else
    #define _init_io_pin_spi_dma      0
#endif


/**
 *  Initial I/O mapping TFT.
 */
#if defined(CONFIG_ENABLE_TFT) && (CONFIG_ENABLE_TFT)
static int
_init_io_pin_tft(void)
{
    // pin switch
#if (CONFIG_PINS_GROUP_ID_TFT == 1)
    reg_clr_bit(REG_SYS0_PIN_CTRL, 1);
#elif (CONFIG_PINS_GROUP_ID_NAND == 2)
    reg_set_bit(REG_SYS0_PIN_CTRL, 1);
#endif

    return 0;
}
#else
    #define _init_io_pin_tft      0
#endif



/**
 *  Initial I/O mapping OID.
 */
#if defined(CONFIG_ENABLE_OID) && (CONFIG_ENABLE_OID)
static int
_init_io_pin_oid(void)
{
    // pin switch
#if (CONFIG_PINS_GROUP_ID_OID == 1)
    reg_clr_bit(REG_SYS0_PIN_CTRL, 9);
#elif (CONFIG_PINS_GROUP_ID_OID == 2)
    reg_set_bit(REG_SYS0_PIN_CTRL, 9);
#endif

    return 0;
}
#else
    #define _init_io_pin_oid      0
#endif



/**
 *  Initial I/O mapping I2S.
 */
#if (defined(CONFIG_ENABLE_I2S_0) && (CONFIG_ENABLE_I2S_0)) || \
    (defined(CONFIG_ENABLE_I2S_1) && (CONFIG_ENABLE_I2S_1)) || \
    (defined(CONFIG_ENABLE_I2S_2) && (CONFIG_ENABLE_I2S_2)) || \
    (defined(CONFIG_ENABLE_I2S_3) && (CONFIG_ENABLE_I2S_3)) || \
    (defined(CONFIG_ENABLE_I2S_4) && (CONFIG_ENABLE_I2S_4))

static int
_init_io_pin_i2s(void)
{
#if defined(CONFIG_ENABLE_I2S_0) && (CONFIG_ENABLE_I2S_0)
#endif

#if defined(CONFIG_ENABLE_I2S_1) && (CONFIG_ENABLE_I2S_1)
#endif

#if defined(CONFIG_ENABLE_I2S_2) && (CONFIG_ENABLE_I2S_2)
#endif

#if defined(CONFIG_ENABLE_I2S_3) && (CONFIG_ENABLE_I2S_3)
#endif

    // pin switch
#if defined(CONFIG_ENABLE_I2S_4) && (CONFIG_ENABLE_I2S_4)
    #if (CONFIG_PINS_GROUP_ID_I2S_4 == 1)
        reg_mask_write(REG_SYS0_PIN_CTRL, (0x0 << 4), 0x00000030);
    #elif (CONFIG_PINS_GROUP_ID_I2S_4 == 2)
        reg_mask_write(REG_SYS0_PIN_CTRL, (0x1 << 4), 0x00000030);
    #elif (CONFIG_PINS_GROUP_ID_I2S_4 == 3)
        reg_mask_write(REG_SYS0_PIN_CTRL, (0x2 << 4), 0x00000030);
    #endif
#endif
    return 0;
}
#else
    #define _init_io_pin_i2s      0
#endif



/**
 *  Initial I/O mapping timer/PWM.
 */
#if (defined(CONFIG_ENABLE_CT32B0_PWM) && (CONFIG_ENABLE_CT32B0_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B1_PWM) && (CONFIG_ENABLE_CT32B1_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B2_PWM) && (CONFIG_ENABLE_CT32B2_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B3_PWM) && (CONFIG_ENABLE_CT32B3_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B4_PWM) && (CONFIG_ENABLE_CT32B4_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B5_PWM) && (CONFIG_ENABLE_CT32B5_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B6_PWM) && (CONFIG_ENABLE_CT32B6_PWM)) || \
    (defined(CONFIG_ENABLE_CT32B7_PWM) && (CONFIG_ENABLE_CT32B7_PWM))

static int
_init_io_pin_timer_pwm(void)
{
    // pin switch
#if defined(CONFIG_ENABLE_CT32B0_PWM) && (CONFIG_ENABLE_CT32B0_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B0_PWM0 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 10);
    #elif (CONFIG_PINS_GROUP_ID_CT32B0_PWM0 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 10);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B0_PWM1 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 11);
    #elif (CONFIG_PINS_GROUP_ID_CT32B0_PWM1 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 11);
    #endif
#endif

#if defined(CONFIG_ENABLE_CT32B2_PWM) && (CONFIG_ENABLE_CT32B2_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B2_PWM2 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 12);
    #elif (CONFIG_PINS_GROUP_ID_CT32B2_PWM2 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 12);
    #endif
#endif

#if defined(CONFIG_ENABLE_CT32B3_PWM) && (CONFIG_ENABLE_CT32B3_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B3_PWM0 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 13);
    #elif (CONFIG_PINS_GROUP_ID_CT32B3_PWM0 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 13);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B3_PWM1 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 14);
    #elif (CONFIG_PINS_GROUP_ID_CT32B3_PWM1 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 14);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B3_PWM2 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 15);
    #elif (CONFIG_PINS_GROUP_ID_CT32B3_PWM2 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 15);
    #endif
#endif

#if defined(CONFIG_ENABLE_CT32B6_PWM) && (CONFIG_ENABLE_CT32B6_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B6_PWM0 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 16);
    #elif (CONFIG_PINS_GROUP_ID_CT32B6_PWM0 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 16);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B6_PWM1 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 17);
    #elif (CONFIG_PINS_GROUP_ID_CT32B6_PWM1 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 17);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B6_PWM2 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 18);
    #elif (CONFIG_PINS_GROUP_ID_CT32B6_PWM2 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 18);
    #endif
#endif

#if defined(CONFIG_ENABLE_CT32B7_PWM) && (CONFIG_ENABLE_CT32B7_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B7_PWM0 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 19);
    #elif (CONFIG_PINS_GROUP_ID_CT32B7_PWM0 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 19);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B7_PWM1 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 20);
    #elif (CONFIG_PINS_GROUP_ID_CT32B7_PWM1 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 20);
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B7_PWM2 == 1)
        reg_clr_bit(REG_SYS0_PIN_CTRL, 21);
    #elif (CONFIG_PINS_GROUP_ID_CT32B7_PWM2 == 2)
        reg_set_bit(REG_SYS0_PIN_CTRL, 21);
    #endif
#endif
    return 0;
}
#else
    #define _init_io_pin_timer_pwm      0
#endif



static int
_init_io_pin_misc(void)
{
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  This order CAN NOT be changed
 */
static const INIT_FUNC   g_init_list[] =
{
    (INIT_FUNC)_init_io_pin_sd,
    (INIT_FUNC)_init_io_pin_tft,
    (INIT_FUNC)_init_io_pin_oid,
    (INIT_FUNC)_init_io_pin_i2s,
    (INIT_FUNC)_init_io_pin_spi_dma,
    (INIT_FUNC)_init_io_pin_timer_pwm,
    (INIT_FUNC)_init_io_pin_misc,
    (INIT_FUNC)(-1)
};

/**
 *  \brief io pin switch setting
 *
 *  \return Return      void
 *
 *  \details
 */
void IO_Pin_Setting(void)
{
    /**
     * initial modules
     */
    for(int i = 0; g_init_list[i] != (INIT_FUNC)(-1); i++)
    {
        if( g_init_list[i] )
        {
            int     rval = 0;
            rval = g_init_list[i]();
            if( rval )      break;
        }
    }

    return;
}


