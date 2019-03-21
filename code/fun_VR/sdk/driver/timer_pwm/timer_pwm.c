/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file timer.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/12
 * @license
 * @description
 */


#include "snc_timer_pwm.h"
#include "reg_util.h"

#include "register_7320.h"
#define TIMER_VERSION        0x73200001

//=============================================================================
//                  Constant Definition
//=============================================================================
#define PWM_ENABLE          1
#define PWM_EM_ENABLE       0
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
static sn_ct32bits_t*
_timer_get_handle(
    timer_id_t  timer_id)
{
    sn_ct32bits_t   *pDev = 0;
    switch(timer_id)
    {
        case TIMER_CT32BITS_0:   pDev = (sn_ct32bits_t*)SN_CT32B0_BASE; break;
        case TIMER_CT32BITS_1:   pDev = (sn_ct32bits_t*)SN_CT32B1_BASE; break;
        case TIMER_CT32BITS_2:   pDev = (sn_ct32bits_t*)SN_CT32B2_BASE; break;
        case TIMER_CT32BITS_3:   pDev = (sn_ct32bits_t*)SN_CT32B3_BASE; break;
        case TIMER_CT32BITS_4:   pDev = (sn_ct32bits_t*)SN_CT32B4_BASE; break;
        case TIMER_CT32BITS_5:   pDev = (sn_ct32bits_t*)SN_CT32B5_BASE; break;
        case TIMER_CT32BITS_6:   pDev = (sn_ct32bits_t*)SN_CT32B6_BASE; break;
        case TIMER_CT32BITS_7:   pDev = (sn_ct32bits_t*)SN_CT32B7_BASE; break;
        default:    break;
    }

    return pDev;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  \brief      timer reset
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *
 *  \details
 */
int
Timer_Reset(
    timer_id_t      timer_id)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;
    uint32_t        prescale = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        //--------------------------
        // disable timer
        reg_write_mask_bits(&pDev->TMRCTRL, (0x0 << 0), (0x1 << 0));

        NVIC_ClearPendingIRQ((IRQn_Type)(CT32B0_IRQn + (uint32_t)timer_id));
        NVIC_DisableIRQ((IRQn_Type)(CT32B0_IRQn + (uint32_t)timer_id));

        //--------------------------
        // reset timer counter and pre-scale counter
        reg_write_mask_bits(&pDev->TMRCTRL, (0x1 << 1), (0x1 << 1));
        /**
         *  wait reset ready, only TC and PC registers reset to 0
         */
        while( reg_read_mask_bits(&pDev->TMRCTRL, (0x1 << 1)) ) {}

        //---------------------------
        // set prescale
        reg_write_bits(&pDev->PRE, prescale);

    } while(0);

    return rval;
}


/**
 *  \brief      set timer match value by channel (enum timer_matcher)
 *
 *  \param [in] timer_id        target timer id (enum timer_id)
 *  \param [in] channel         target channel id (enum timer_matcher)
 *  \param [in] match_value     comparing value
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
Timer_SetMatchValue(
    timer_id_t          timer_id,
    timer_matcher_t     channel,
    uint32_t            match_value)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        switch( channel )
        {
            case TIMER_MATCHER_0:  reg_write_bits(&pDev->MR0, match_value); break;
            case TIMER_MATCHER_1:  reg_write_bits(&pDev->MR1, match_value); break;
            case TIMER_MATCHER_2:  reg_write_bits(&pDev->MR2, match_value); break;
            case TIMER_MATCHER_3:  reg_write_bits(&pDev->MR3, match_value); break;
            default:    rval = -1; break;
        }

    } while(0);

    return rval;
}


/**
 *  \brief      set operation event when count matches
 *
 *  \param [in] timer_id            target timer id (enum timer_id)
 *  \param [in] channel             target channel id (enum timer_matcher)
 *  \param [in] is_interruption     do interrupt or not
 *  \param [in] is_stop             do stop or not (If Enable Stop and Reset, the timer will stop and reset TC value)
 *  \param [in] is_reset            do reset or not (If Enable Stop and Reset, the timer will stop and reset TC value)
 *  \return Return                  0: ok, other: fail
 *
 *  \details
 */
int
Timer_SetMatchOperation(
    timer_id_t          timer_id,
    timer_matcher_t     channel,
    uint32_t            is_interruption,
    uint32_t            is_stop,
    uint32_t            is_reset)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        uint32_t    data = 0;

        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        is_interruption = !!is_interruption;
        is_stop         = !!is_stop;
        is_reset        = !!is_reset;
        channel *= 3;

        data = (is_stop << 2) | (is_reset << 1) | is_interruption;
        reg_write_mask_bits(&pDev->MCTRL, (data << channel), (0x7 << channel));

    } while(0);

    return rval;
}

/**
 *  \brief      Set sensing type when capture the input signal.
 *              This API will RESET CAP0 funciton configuration
 *
 *  \param [in] timer_id            target timer id (enum timer_id)
 *  \param [in] sensing_type        target sensing type (enum timer_pin_sensing)
 *  \return                         0: ok, other: fail
 *
 *  \details
 */
int
TimerCap_SetSensing(
    timer_id_t          timer_id,
    timer_cap_sensing_t sensing_type)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        /* disable CAP0 function for re-configuring */
        reg_write_bits(&pDev->CAPCTRL, 0UL);
        reg_write_mask_bits(&pDev->CNTCTRL, ((sensing_type & 0x3) << 0), (0x3 << 0));

    } while(0);

    return rval;
}

/**
 *  \brief      Set configuration of CAP0 function
 *
 *  \param [in] is_enable_cap0  enable CAP0 function or not
 *  \param [in] is_interrupt    enable irq or not
 *  \param [in] rising_opt      when CAP0 get a rising edge, how to handle TC value
 *  \param [in] falling_opt     when CAP0 get a fulling edge, how to handle TC value
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
TimerCap_SetConfiguration(
    timer_id_t              timer_id,
    uint32_t                is_enable_cap0,
    uint32_t                is_interrupt,
    timer_count_operate_t   rising_opt,
    timer_count_operate_t   falling_opt)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        uint32_t    data = 0;

        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        if( !is_enable_cap0 )
        {
            // reset CAP0 function
            reg_write_bits(&pDev->CAPCTRL, 0UL);
            break;
        }

        is_enable_cap0 = !!is_enable_cap0;
        is_interrupt = !!is_interrupt;
        data = (is_interrupt << 4) |
                ((falling_opt & 0x3) << 2) |
                (rising_opt & 0x3);

        reg_write_bits(&pDev->CAPCTRL, data);
        reg_set_bit(&pDev->IC, 4); // clear pendding irq
        reg_write_mask_bits(&pDev->CAPCTRL, ((is_enable_cap0 & 0x3) << 5), (0x3 << 5));

    } while(0);

    return rval;
}

/**
 *  \brief      In Capture mode, get the snapshot count of a timer when Pin_CAP0 be sensed edge
 *
 *  \param [in] timer_id        target timer id (enum timer_id)
 *  \return                     0: no data, other: count value
 *
 *  \details
 */
uint32_t
TimerCap_GetSnapshot(
    timer_id_t  timer_id)
{
    uint32_t        data = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            break;
        }

        data = reg_read_bits((volatile uint32_t*)&pDev->CAP0);

    } while(0);

    return data;
}

/**
 *  \brief      launch timer, It MUST be set at last
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
Timer_Launch(
    timer_id_t  timer_id)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        reg_write_bits(&pDev->IC, 0x1f);
        reg_write_mask_bits(&pDev->TMRCTRL, (0x1 << 0), (0x1 << 0));

        // enable NVIC IRQ
        NVIC_ClearPendingIRQ((IRQn_Type)(CT32B0_IRQn + (uint32_t)timer_id));
        NVIC_EnableIRQ((IRQn_Type)(CT32B0_IRQn + (uint32_t)timer_id));

    } while(0);

    return rval;
}

/**
 *  \brief      Get timer's count
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \param [in] value       rebase timer count
 *                              SET 0 if you have no idea
 *  \return                 current count of this timer
 *
 *  \details
 */
uint32_t
Timer_GetCount(
    timer_id_t      timer_id,
    uint32_t        value)
{
    uint32_t        data = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            break;
        }

        data = reg_read_bits(&pDev->TC);

        if( value )
            reg_write_bits(&pDev->TC, (value == TIMER_RESET_TC) ? 0 : value);

    } while(0);

    return data;
}


/**
 *  \brief      Get status of irq of a timer in ISR
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \param [in] irq_type    target irq type (enum timer_irq_type)
 *  \return                 0: nothing, 1: irq trigger
 *
 *  \details
 */
uint32_t
Timer_GetIrqStatus(
    timer_id_t          timer_id,
    timer_irq_type_t    irq_type)
{
    uint32_t        data = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            break;
        }

        data = (reg_read_mask_bits((volatile uint32_t*)&pDev->RIS, (0x1 << irq_type))) ? 1 : 0;

    } while(0);

    return data;
}

uint32_t
Timer_GetIrqAllStatus(
    timer_id_t          timer_id)
{
    uint32_t        data = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            break;
        }

        data = reg_read_mask_bits((volatile uint32_t*)&pDev->RIS, 0x1f);

    } while(0);

    return data;
}
/**
 *  \brief      clear irq flag of a timer
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \param [in] irq_type    target irq type (enum timer_irq_type)
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
Timer_ClearIrq(
    timer_id_t          timer_id,
    timer_irq_type_t    irq_type)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            break;
        }

        reg_write_mask_bits(&pDev->IC, (0x1 << irq_type), (0x1 << irq_type));

    } while(0);

    return rval;
}

/**
 *  \brief  When reset PWM, the target timer will be reset.
 *          ps. count mod and PWM mode are conflict
 *
 *  \param [in] timer_id        target timer id (enum timer_id)
 *  \param [in] pInit_info      the initial info of PWM
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
TimerPwm_Reset(
    timer_id_t          timer_id,
    timer_pwm_init_t    *pInit_info)
{
    int             rval = 0;
    sn_ct32bits_t   *pDev = 0;

    do {
        if( !pInit_info )
        {
            rval = -1;
            snc_assert(pInit_info != 0);
            break;
        }

        pDev = _timer_get_handle(timer_id);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        Timer_Reset(timer_id);

        {
            uint32_t        periodic_cnt = 0;
            timer_pwm_id_t  pwm_id = TIMER_PWM_00;

            // enable reset count value when matches MR3 and irq
            periodic_cnt = (0x1 << 1) | (!!pInit_info->is_irq_one_periodic);
            reg_write_mask_bits(&pDev->MCTRL, (periodic_cnt << 9), (0x7 << 9));

            // calculate the count times of a target periodic
            periodic_cnt = pInit_info->periodic_us * (pInit_info->pclk/1000000);
            reg_write_bits(&pDev->MR3, periodic_cnt);

            for(pwm_id = TIMER_PWM_00; pwm_id < TIMER_PWM_NUM; pwm_id++)
            {
                one_pwm_setting_t       *pSetting = &pInit_info->target_pwm[pwm_id];
                uint32_t                value = 0;
                uint32_t                shift = 0;

                if( pSetting->target_pwm_id == TIMER_PWM_IGNORE )
                    continue;

                // calculate match value
                {
                    if( pSetting->duty_cycle == 0 )
                        value = periodic_cnt;
                    else if( pSetting->duty_cycle >= 100 )
                        value = 0;
                    else
                    {
                        // value = periodic_cnt * ((float)pSetting->duty_cycle * 0.01f);
                        value = periodic_cnt * (100 - pSetting->duty_cycle) / 100;
                    }
                }

                switch( pwm_id )
                {
                    case TIMER_PWM_00:  reg_write_bits(&pDev->MR0, value); break;
                    case TIMER_PWM_01:  reg_write_bits(&pDev->MR1, value); break;
                    case TIMER_PWM_02:  reg_write_bits(&pDev->MR2, value);break;
                    default:
                        rval = -1;
                        return rval;
                }

                // enable match irq or not
                shift = 3 * pwm_id;
                value = !!pSetting->is_irq_duty_end;
                reg_write_mask_bits(&pDev->MCTRL, (value << shift), (0x1 << shift));

                // enable PWM and PWM I/O
                shift = 20 + pwm_id;
                value = (0x1 << shift) | (PWM_ENABLE << pwm_id);
                reg_write_mask_bits(&pDev->PWMCTRL, value, value);
            }
        }

    } while(0);

    return rval;
}

/**
 *  \brief      launch PWM mode, It MUST be set at last
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
TimerPwm_Launch(
    timer_id_t      timer_id)
{
    return Timer_Launch(timer_id);
}

uint32_t
Timer_GetVersion(void)
{
    return TIMER_VERSION;
}


