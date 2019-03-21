/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_timer.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_timer_H_w31m6DVq_lgWz_HHqQ_s3DR_ui0xE7glqf8j__
#define __snc_timer_H_w31m6DVq_lgWz_HHqQ_s3DR_ui0xE7glqf8j__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define TIMER_RESET_TC          0xFFFFFFFF

/**
 *  timer id
 */
typedef enum timer_id
{
    TIMER_CT32BITS_0        = 0,
    TIMER_CT32BITS_1,
    TIMER_CT32BITS_2,
    TIMER_CT32BITS_3,
    TIMER_CT32BITS_4,
    TIMER_CT32BITS_5,
    TIMER_CT32BITS_6,
    TIMER_CT32BITS_7,

} timer_id_t;


/**
 *  timer match channel
 */
typedef enum timer_matcher
{
    TIMER_MATCHER_0     = 0,
    TIMER_MATCHER_1,
    TIMER_MATCHER_2,
    TIMER_MATCHER_3,
    TIMER_MATCHER_NUM

} timer_matcher_t;

/**
 *  sense rising/falling/both edges when someone connect the Pin_CAP0
 *  Pin_CAP0: a gpio pin for timer capturing (follow the GPIO Mapping in spec),
 *            and the input signal clock should be less than (PCLK / 2)
 *            ps. timer should sample twice (2T of PCLK) to decide rising/fulling state
 */
typedef enum timer_cap_sensing
{
    /**
     *  ignore Pin_CAP0, and sense PCLK rising edge to accumulate TC value (measure the time of input of CAP0)
     */
    TIMER_CAP_SENSING_PCLK      = 0,

    /**
     *  sensing rising of Pin_CAP0 and accumulate TC value
     */
    TIMER_CAP_SENSING_RISING,

    /**
     *  sensing falling of Pin_CAP0 and accumulate TC value
     */
    TIMER_CAP_SENSING_FALLING,

    /**
     *  sensing rising and falling of Pin_CAP0 and accumulate TC value
     */
    TIMER_CAP_SENSING_BOTH,

} timer_cap_sensing_t;

/**
 *  when CAP0 be sensed, operate tc (times count) value bypass/snapshot/zero
 */
typedef enum timer_count_operate
{
    TIMER_COUNT_BYPASS     = 0,

    /**
     *  load TC value to capture value of CAP0
     */
    TIMER_COUNT_SNAPSHOT,

    /**
     *  Reset TC to 0
     */
    TIMER_COUNT_ZERO

} timer_count_operate_t;


/**
 *  irq type
 */
typedef enum timer_irq_type
{
    TIMER_IRQ_TYPE_MATCHER_0     = 0,
    TIMER_IRQ_TYPE_MATCHER_1,
    TIMER_IRQ_TYPE_MATCHER_2,
    TIMER_IRQ_TYPE_MATCHER_3,
    TIMER_IRQ_TYPE_CAP0,
    TIMER_IRQ_TYPE_NUM

} timer_irq_type_t;

/**
 *  every timer support 3 PWM Pin (reference GPIO Mapping table)
 */
typedef enum timer_pwm_id
{
    TIMER_PWM_00    = 0,
    TIMER_PWM_01,
    TIMER_PWM_02,
    TIMER_PWM_NUM,
    TIMER_PWM_IGNORE,

} timer_pwm_id_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  single PWM setting
 */
typedef struct one_pwm_setting
{
    /**
     *  when fulfilling duty cycle, trigger IRQ or not
     */
    uint16_t        is_irq_duty_end;

    /**
     *  the ratio of active fraction of one period in a signal
     */
    uint16_t        duty_cycle; // support 1 ~ 99
    timer_pwm_id_t  target_pwm_id;

} one_pwm_setting_t;

/**
 *  PWM initial setting
 */
typedef struct timer_pwm_init
{
    /**
     *  current peripheral clock
     *  e.g. 96MHz: pclk = 96000000
     */
    uint32_t    pclk;

    uint32_t            periodic_us;    // expected periodic with microseconds
    uint32_t            is_irq_one_periodic;
    one_pwm_setting_t   target_pwm[3];

} timer_pwm_init_t;
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
 *  \brief      timer reset
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
Timer_Reset(
    timer_id_t      timer_id);


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
    uint32_t            match_value);



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
    uint32_t            is_reset);


/**
 *  \brief      Set sensing type when capture the input signal.
 *              This API will RESET CAP0 funciton configuration
 *
 *  \param [in] timer_id            target timer id (enum timer_id)
 *  \param [in] sensing_type        target sensing type to accumulate TC (enum timer_pin_sensing)
 *  \return                         0: ok, other: fail
 *
 *  \details
 */
int
TimerCap_SetSensing(
    timer_id_t          timer_id,
    timer_cap_sensing_t sensing_type);


/**
 *  \brief      Set configuration of CAP0 function
 *
 *  \param [in] is_enable_cap0  enable CAP0 function or not
 *  \param [in] is_interrupt    enable irq or not
 *  \param [in] rising_opt      when CAP0 get a rising edge, how to handle TC value
 *  \param [in] falling_opt     when CAP0 get a falling edge, how to handle TC value
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
    timer_count_operate_t   falling_opt);


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
    timer_id_t  timer_id);


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
    timer_id_t  timer_id);


/**
 *  \brief      Get timer's count value
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \param [in] value       rebase timer count
 *                              SET 0 if you have no idea
 *                              ps. if set TIMER_RESET_TC, it will reset TC value to 0
 *  \return                 current count of this timer
 *
 *  \details
 */
uint32_t
Timer_GetCount(
    timer_id_t      timer_id,
    uint32_t        value);



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
    timer_irq_type_t    irq_type);

/**
 *  \brief  Get all interruption status
 *
 *  \param [in] timer_id    target timer id (enum timer_id)
 *  \return                 all interruption status
 *                              bit 0: TIMER_IRQ_TYPE_MATCHER_0
 *                              bit 1: TIMER_IRQ_TYPE_MATCHER_1
 *                              bit 2: TIMER_IRQ_TYPE_MATCHER_2
 *                              bit 3: TIMER_IRQ_TYPE_MATCHER_3
 *                              bit 4: TIMER_IRQ_TYPE_CAP0
 *  \details
 */
uint32_t
Timer_GetIrqAllStatus(
    timer_id_t          timer_id);


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
    timer_irq_type_t    irq_type);


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
    timer_pwm_init_t    *pInit_info);


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
    timer_id_t      timer_id);



uint32_t
Timer_GetVersion(void);




#ifdef __cplusplus
}
#endif

#endif
