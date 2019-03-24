/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */

#ifndef __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__
#define __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 * 7320 definition
 */
#define SN_REG_WDT0_BASE                    0x40008000UL
#define SN_REG_WDT1_BASE                    0x40009000UL

#define SN_WDT_KEY                          0x5AFA0000UL
#define SN_FEED_KEY                         0x55AAUL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  SN_WDT Structure
 */
typedef struct sn_wdt
{
    union {
        __IO uint32_t CFG;   /*!< Offset:0x00 WDT Configuration Register */

        struct {
            __IO uint32_t WDTEN  : 1;   /*!< WDT enable */
            __IO uint32_t WDTIE  : 1;   /*!< WDT interrupt enable */
            __IO uint32_t WDTINT : 1;   /*!< WDT interrupt flag  */
                 uint32_t        : 13;
            __O  uint32_t WDKEY  : 16;  /*!< Watchdog register key */
        } CFG_b;  /*!< BitSize  */
    };

    union {
        __IO uint32_t CLKSOURCE;   /*!< Offset:0x04 WDT Clock Source Register */

        struct {
            __IO uint32_t CLKSEL : 2;   /*!< WDT clock source */
                 uint32_t        : 14;
            __O uint32_t WDKEY   : 16;  /*!< Watchdog register key */
        } CLKSOURCE_b;  /*!< BitSize  */
    };

    union {
        __IO uint32_t TC;   /*!< Offset:0x08 WDT Timer Constant Register */

        struct {
            __IO uint32_t WTC   : 8;   /*!< Watchdog timer constant reload value  */
                 uint32_t       : 8;
            __O  uint32_t WDKEY : 16;  /*!< Watchdog register key */
        } TC_b;  /*!< BitSize  */
    };

    union {
        __O uint32_t FEED;  /*!< Offset:0x0C WDT Feed Register  */

        struct {
            __O uint32_t FV    : 16;  /*!< Watchdog feed value */
            __O uint32_t WDKEY : 16;  /*!< Watchdog register key */
        } FEED_b; /*!< BitSize  */
    };
} sn_wdt_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
