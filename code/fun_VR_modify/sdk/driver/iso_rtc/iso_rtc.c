/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file iso_rtc.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/05/03
 * @license
 * @description
 */

#include "reg_util.h"
#include "snc_iso_rtc.h"
#include "register_7320.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define RTC_VERSION         0x73200000

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

rtc_ret_value_t RTC_Initial(rtc_setting_t *pSet)
{
    reg_clear_bit(&SN_ISO->ISO_CTRL, RTC_CTRL_ENABLE_POS);
    SN_ISO->ISO_CTRL_b.RTC_SRC          = pSet->rtc_clk_source;
    SN_ISO->ISO_CTRL_b.RTC_Timeout      = pSet->timeout_interval;
    SN_ISO->ISO_CTRL_b.RTC_WKP          = pSet->wakeup_cpu;
    
    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
    
    return RTC_SUCCESS;
}


void RTC_Deinitial(void)
{
    reg_clear_bit(&SN_ISO->ISO_CTRL, RTC_CTRL_ENABLE_POS);
    SN_ISO->ISO_CTRL_b.RTC_SRC          = RTC_SRC_ILRC;
    SN_ISO->ISO_CTRL_b.RTC_Timeout      = RTC_INT_500_MS;
    SN_ISO->ISO_CTRL_b.RTC_WKP          = 0;
    SN_ISO->ISO_CTRL_b.RTC_EN           = 0;
    
    reg_write_mask_bits(&SN_PMU->PMU_CTRL, 
                    PMU_FLAG_RTC_MSK,
                    PMU_FLAG_RTC_MSK
                   );

    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_DisableIRQ(RTC_IRQn);
}


void RTC_Start()
{
    reg_set_bit(&SN_ISO->ISO_CTRL, RTC_CTRL_ENABLE_POS);
}


void RTC_Stop()
{
    reg_clear_bit(&SN_ISO->ISO_CTRL, RTC_CTRL_ENABLE_POS);
}


void RTC_Clear_INT()
{
    /* Clear RTC Interrupt flag */
    reg_write_mask_bits(&SN_PMU->PMU_CTRL, 
                        PMU_FLAG_RTC_MSK | PMU_FLAG_WAKE_INT_MSK,
                        PMU_FLAG_RTC_MSK | PMU_FLAG_WAKE_INT_MSK
                       );
}


uint32_t RTC_Check_Running()
{
    return SN_ISO->ISO_CTRL_b.RTC_EN;
}


uint32_t RTC_Get_Int_Count()
{
    return SN_PMU->RTC_C;
}
     

uint32_t RTC_Get_WKP()
{
    if(SN_PMU->PMU_CTRL_b.WKP)
        return 0;
    else
        return 1;
    
}


void WKP_Clear_INT()
{
    /* Clear RTC Interrupt flag */
    reg_write_mask_bits(&SN_PMU->PMU_CTRL, 
                        PMU_FLAG_WKP_MSK | PMU_FLAG_WAKE_INT_MSK,
                        PMU_FLAG_WKP_MSK | PMU_FLAG_WAKE_INT_MSK
                       );
}


/**
 *  \brief: Return driver version.
 */
uint32_t RTC_GetVersion(void)
{
    return RTC_VERSION;
}




