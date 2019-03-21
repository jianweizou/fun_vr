/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_wdt.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/25
 * @license
 * @description
 */

#ifndef __snc_wdt_H_wedVAyLS_lfzc_HR3s_spkN_uYKveIKkMoQr__
#define __snc_wdt_H_wedVAyLS_lfzc_HR3s_spkN_uYKveIKkMoQr__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  Watch Dog Timer ID
 */
typedef enum wdt_id
{
    WDT_ID_0    = 0,    // for core 0
    WDT_ID_1,           // for core 1

} wdt_id_t;

/**
 *  WDT input clock source
 */
typedef enum wdt_clock_src
{
    WDT_CLOCK_ILRC = 0,     // 32768 Hz

} wdt_clock_src_t;

/**
 *  WDT operation when timeout
 */
typedef enum wdt_mode
{
    WDT_MODE_RESET      = 0,
    WDT_MODE_IRQ,

} wdt_mode_t;
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
 *  \brief      WDT initial
 *
 *  \param [in] id              target WDT id (enum wdt_id)
 *  \param [in] clock_src       select PCLK_WDT (only suppport ILRC 32768 Hz)
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
WDT_Init(
    wdt_id_t        id,
    wdt_clock_src_t clock_src);


/**
 *  \brief          Reset WDT timeout
 *
 *  \param [in] id          target WDT id (enum wdt_id)
 *  \param [in] timeout     timeout stage (0 ~ 255)
 *                              when clock source is ILRC (32768 Hz), the duration is ((timeout + 1) * 3906 us)
 *
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
WDT_ResetTimeout(
    wdt_id_t    id,
    uint32_t    timeout);


/**
 *  \brief      enable WDT
 *
 *  \param [in] id          target WDT id (enum wdt_id)
 *  \param [in] mode        operation mode when timeout
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
WDT_Enable(
    wdt_id_t    id,
    wdt_mode_t  mode);



/**
 *  \brief  Disable WDT
 *
 *  \param [in] id      target WDT id (enum wdt_id)
 *  \return
 *
 *  \details
 */
int
WDT_Disable(
    wdt_id_t    id);


/**
 *  \brief      Get the WDT irq status
 *
 *  \param [in] id      target WDT id (enum wdt_id)
 *  \return             0: nothing, 1: irq trigger
 *
 *  \details
 */
uint32_t
WDT_GetIrqStatus(
    wdt_id_t    id);


/**
 *  \brief  In IRQ mode, clear irq flag
 *          ps. It should be fed timeout value to WDT before clearing IRQ flag
 *
 *  \param [in] id      target WDT id (enum wdt_id)
 *  \return             0: ok, other: fail
 *
 *  \details
 */
int
WDT_ClearIrqFlag(
    wdt_id_t    id);



uint32_t
WDT_GetVersion(void);



#ifdef __cplusplus
}
#endif

#endif
