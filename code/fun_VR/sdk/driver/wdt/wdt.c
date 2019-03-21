/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file wdt.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/25
 * @license
 * @description
 */


#include "snc_wdt.h"
#include "reg_util.h"

#include "register_7320.h"
#define WDT_VERSION         0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================
#define WDT_IRQ_PIN         NonMaskableInt_IRQn
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
static sn_wdt_t*
_wdt_get_handle(
    wdt_id_t    id)
{
    sn_wdt_t        *pDev = 0;
#if 1
    // for code size
    if( id == WDT_ID_0 )
        pDev = (sn_wdt_t*)SN_REG_WDT0_BASE;
    else if( id == WDT_ID_1 )
        pDev = (sn_wdt_t*)SN_REG_WDT1_BASE;

#else
    switch( id )
    {
        case WDT_ID_0:  pDev = (sn_wdt_t*)SN_REG_WDT0_BASE; break;
        case WDT_ID_1:  pDev = (sn_wdt_t*)SN_REG_WDT1_BASE; break;
        default:    break;
    }
#endif

    return pDev;
}
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
    wdt_clock_src_t clock_src)
{
    int         rval = 0;
    sn_wdt_t    *pDev = 0;

    do {
        uint32_t    data = SN_WDT_KEY | 0x2;

        pDev = _wdt_get_handle(id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        // only support ILRC clock source (32768 Hz)
        reg_write_mask_bits(&pDev->CLKSOURCE, data, 0xFFFF0003);

    } while(0);

    return rval;
}

/**
 *  \brief          Reset WDT timeout
 *
 *  \param [in] id          target WDT id (enum wdt_id)
 *  \param [in] timeout     timeout stage (0 ~ 255)
 *                              when clock source is ILRC (32768 Hz), the duration is (timeout * 3906 us)
 *
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
WDT_ResetTimeout(
    wdt_id_t    id,
    uint32_t    timeout)
{
    int         rval = 0;
    sn_wdt_t    *pDev = 0;

    do {
        pDev = _wdt_get_handle(id);
        if( !pDev )
        {
            rval = -1;
            break;
        }

        timeout &= 0xFF;
        reg_write_mask_bits(&pDev->TC, (SN_WDT_KEY | timeout), 0xFFFF00FF);

        // trigger H/W to reload Timer constant
        reg_write_bits(&pDev->FEED, (SN_WDT_KEY | SN_FEED_KEY));
    } while(0);

    return rval;
}

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
    wdt_mode_t  mode)
{
    int         rval = 0;
    sn_wdt_t    *pDev = 0;

    do {
        uint32_t    data = SN_WDT_KEY;

        pDev = _wdt_get_handle(id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        if( mode == WDT_MODE_IRQ )
        {
            data |= (0x1 << 1);

            /**
             *  WDT0 and WDT1 share ONE IRQ pin
             */
            NVIC_ClearPendingIRQ(WDT_IRQ_PIN);
            NVIC_EnableIRQ(WDT_IRQ_PIN);
        }
        else
        {
            NVIC_DisableIRQ(WDT_IRQ_PIN);
        }

        reg_write_bits(&pDev->CFG, SN_WDT_KEY);
        reg_write_mask_bits(&pDev->CFG, data, 0xFFFF0002);

        reg_write_mask_bits(&pDev->CFG, (SN_WDT_KEY | 0x1), 0xFFFF0001);

    } while(0);

    return rval;
}

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
    wdt_id_t    id)
{
    int         rval = 0;
    sn_wdt_t    *pDev = 0;
    do {
        pDev = _wdt_get_handle(id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        NVIC_DisableIRQ(WDT_IRQ_PIN);

        reg_write_bits(&pDev->CFG, SN_WDT_KEY);

    } while(0);

    return rval;
}


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
    wdt_id_t    id)
{
    uint32_t    data = 0;
    sn_wdt_t    *pDev = 0;

    do {
        pDev = _wdt_get_handle(id);
        if( !pDev )
        {
            break;
        }

        data = reg_read_mask_bits(&pDev->CFG, 0x4);

    } while(0);

    return (!!data);
}


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
    wdt_id_t    id)
{
    int         rval = 0;
    sn_wdt_t    *pDev = 0;
    do {
        uint32_t    data = SN_WDT_KEY;

        pDev = _wdt_get_handle(id);
        if( !pDev )
        {
            rval = -1;
            break;
        }

        reg_write_mask_bits(&pDev->CFG, data, 0xFFFF0004);

    } while(0);

    return rval;
}

uint32_t
WDT_GetVersion(void)
{
    return WDT_VERSION;
}




