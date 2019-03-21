/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file idma.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/5
 * @license
 * @description
 */

#include "snc_pmu.h"
#include "register_7320.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define PMU_VERSION         0x73200001

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
 *  \brief Instruct CPU to enter low power mode.
 *
 *  \param [in] mode: Low power mode @pmu_mode_t
 *
 */
void PMU_Enter_Mode(pmu_mode_t mode)
{
    do
    {
        SN_PMU->PMU_CTRL &= ~0x7;

        while(PMU_WAKE_PIN() == 0);
        
        /* Enter low power mode */
        SN_PMU->PMU_CTRL |= 1 << (mode -1);

        __WFI();
    }while(mode == PMU_DEEP_POWER_DOWN);
}

/**
 *  \brief: Return the Wake Pin status
 */
uint32_t PMU_WAKE_PIN(void)
{
    return SN_PMU->PMU_CTRL_b.WKP;
}

/**
 *  \brief: Returns the status of pmu flag
 *
 *  \param [in] flag: Wake up flags @pmu_wkp_flag_t
 *  \return: 0- none, 1-wake event occurred, else- invaild.
 *
 *  \details: Read the status of wakeup event.
 */
uint32_t PMU_Get_Wake_Flag(pmu_wkp_flag_t flag)
{
    switch(flag)
    {
        case FLAG_WAKE_PIN:
            return SN_PMU->PMU_CTRL_b.WKP_FLAG;

        case FLAG_RTC:
            return SN_PMU->PMU_CTRL_b.RTC_FLAG;

        default:
            return 0xFFFFFFFF;
    }
}


/**
 *  \brief: Clear the wakeup event flag.
 *
 *  \param [in] flag: Wake up flags @pmu_wkp_flag_t
 *
 */
void PMU_Clr_Wake_Flag(pmu_wkp_flag_t flag)
{
    switch(flag)
    {
        case FLAG_WAKE_PIN:

            SN_PMU->PMU_CTRL |= (PMU_FLAG_WKP_Msk | PMU_FLAG_WKP_INT_Msk);
            break;

        case FLAG_RTC:
            SN_PMU->PMU_CTRL |= PMU_FLAG_RTC_Msk;
            break;
    }
}


/**
 *  \brief: Return driver version.
 */
uint32_t PMU_GetVersion(void)
{
    return PMU_VERSION;
}




