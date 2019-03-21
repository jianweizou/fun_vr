/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file gpio.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */

#ifndef __gpio_H_wQDNMSy6_lUDo_HBrK_sccT_u7LGyu8GK4oZ__
#define __gpio_H_wQDNMSy6_lUDo_HBrK_sccT_u7LGyu8GK4oZ__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define GPIO_PIN_DATA_IGNORE        (-1)


typedef enum gpio_pin
{
    // port 0
    GPIO_PORT_0_P00      = 0,
    GPIO_PORT_0_P01,
    GPIO_PORT_0_P02,
    GPIO_PORT_0_P03,
    GPIO_PORT_0_P04,
    GPIO_PORT_0_P05,
    GPIO_PORT_0_P06,
    GPIO_PORT_0_P07,
    GPIO_PORT_0_P08,
    GPIO_PORT_0_P09,
    GPIO_PORT_0_P10,
    GPIO_PORT_0_P11,
    GPIO_PORT_0_P12,
    GPIO_PORT_0_P13,
    GPIO_PORT_0_P14,
    GPIO_PORT_0_P15,

    // port 1
    GPIO_PORT_1_P00,
    GPIO_PORT_1_P01,
    GPIO_PORT_1_P02,
    GPIO_PORT_1_P03,
    GPIO_PORT_1_P04,
    GPIO_PORT_1_P05,
    GPIO_PORT_1_P06,
    GPIO_PORT_1_P07,
    GPIO_PORT_1_P08,
    GPIO_PORT_1_P09,
    GPIO_PORT_1_P10,
    GPIO_PORT_1_P11,
    GPIO_PORT_1_P12,
    GPIO_PORT_1_P13,
    GPIO_PORT_1_P14,
    GPIO_PORT_1_P15,

    // port 2
    GPIO_PORT_2_P00,
    GPIO_PORT_2_P01,
    GPIO_PORT_2_P02,
    GPIO_PORT_2_P03,
    GPIO_PORT_2_P04,
    GPIO_PORT_2_P05,
    GPIO_PORT_2_P06,
    GPIO_PORT_2_P07,
    GPIO_PORT_2_P08,
    GPIO_PORT_2_P09,
    GPIO_PORT_2_P10,
    GPIO_PORT_2_P11,
    GPIO_PORT_2_P12,
    GPIO_PORT_2_P13,
    GPIO_PORT_2_P14,
    GPIO_PORT_2_P15,

    // port 3
    GPIO_PORT_3_P00,
    GPIO_PORT_3_P01,
    GPIO_PORT_3_P02,
    GPIO_PORT_3_P03,
    GPIO_PORT_3_P04,
    GPIO_PORT_3_P05,
    GPIO_PORT_3_P06,
    GPIO_PORT_3_P07,
    GPIO_PORT_3_P08,
    GPIO_PORT_3_P09,
    GPIO_PORT_3_P10,
    GPIO_PORT_3_P11,
    GPIO_PORT_3_P12,
    GPIO_PORT_3_P13,
    GPIO_PORT_3_P14,
    GPIO_PORT_3_P15,

    // port 4
    GPIO_PORT_4_P00,
    GPIO_PORT_4_P01,
    GPIO_PORT_4_P02,
    GPIO_PORT_4_P03,
    GPIO_PORT_4_P04,
    GPIO_PORT_4_P05,
    GPIO_PORT_4_P06,
    GPIO_PORT_4_P07,
    GPIO_PORT_4_P08,
    GPIO_PORT_4_P09,
    GPIO_PORT_4_P10,
    GPIO_PORT_4_P11,
    GPIO_PORT_4_P12,
    GPIO_PORT_4_P13,
    GPIO_PORT_4_P14,
    GPIO_PORT_4_P15,

    GPIO_PIN_NUM
} gpio_pin_t;


typedef enum gpio_port
{
    GPIO_PORT_0 = 0,
    GPIO_PORT_1,
    GPIO_PORT_2,
    GPIO_PORT_3,
    GPIO_PORT_4,
    GPIO_PORT_NUM

} gpio_port_t;

typedef enum gpio_mode
{
    GPIO_MODE_INPUT     = 0,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_IGNORE

} gpio_mode_t;

/**
 *  configure GPIO mode
 *  0: always pull-up
 *  1: reserved
 *  2: manual mode
 *  3: repeat mode
 */
typedef enum gpio_config
{
    GPIO_CONFIG_PULL_UP_ON  = 0,
    GPIO_CONFIG_IGNORE      = 1,
    GPIO_CONFIG_INACTIVE    = 2,
    GPIO_CONFIG_REPEATER    = 3,

} gpio_config_t;

/**
 *  GPIO Interrupt Event Sense
 */
typedef enum gpio_edge_sense
{
    GPIO_EDGE_SENSE_OFF      = 0,
    GPIO_EDGE_SENSE_HIGH_LEVEL,
    GPIO_EDGE_SENSE_LOW_LEVEL,
    GPIO_EDGE_SENSE_RISING,
    GPIO_EDGE_SENSE_FALLING,
    GPIO_EDGE_SENSE_BOTH,

} gpio_edge_sense_t;

/**
 *  GPIO Driving control
 *  1) 1.8mA/5.5mA
 *  2) 3.6mA/6.6mA
 *  3) 5.4mA/7.7mA
 *  4) 7.2mA/8.8mA
 */
typedef enum gpio_driving_ctrl
{
    GPIO_DRIVING_18_55  = 0,
    GPIO_DRIVING_36_66,
    GPIO_DRIVING_54_77,
    GPIO_DRIVING_72_88,
    GPIO_DRIVING_NONE,

} gpio_driving_ctrl_t;

/**
 *  Register memory base of GPIO port
 */
typedef enum gpio_reg_base
{
    GPIO_REG_BASE_0    = 0,
    GPIO_REG_BASE_1

} gpio_reg_base_t;
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
 *  \brief  set a pin of a GPIO
 *
 *  \param [in] pin_id      enum gpio_pin_t
 *  \param [in] mode        a mode of a pin
 *  \param [in] config      a configuration of a pin
 *  \param [in] data        the data of the pin
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_SetPin(
    gpio_pin_t      pin_id,
    gpio_mode_t     mode,
    gpio_config_t   config,
    int             data);


/**
 *  \brief  get GPIO pin data
 *
 *  \param [in] pin_id      enum gpio_pin_t
 *  \return                 0: low, other: high
 *
 *  \details
 */
uint32_t
GPIO_GetPin(
    gpio_pin_t      pin_id);



int GPIO_SetPinData(gpio_pin_t  pin_id,int data);
int GPIO_GetPinCfg(gpio_pin_t  pin_id,uint32_t *pMode,uint32_t *pConfig);


/**
 *  \brief      Toggle the pin data
 *
 *  \param [in] pin_id      enum gpio_pin_t
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_TogglePin(
    gpio_pin_t      pin_id);


/**
 *  \brief  set all GPIO pins in a port
 *
 *  \param [in] port        enum gpio_port_t
 *  \param [in] mode        all pin modes of a GPIO port
 *  \param [in] config      all pin configurations of a GPIO port
 *  \param [in] data        all pin datas of a GPIO port
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_SetPort(
    gpio_port_t  port,
    uint32_t     mode,
    uint32_t     config,
    uint32_t     data);


/**
 *  \brief      Get value of a GPIO Port
 *
 *  \param [in] port        enum gpio_port_t
 *  \param [in] pMode       all pin modes of a GPIO port
 *  \param [in] pConfig     all pin configurations of a GPIO port
 *  \param [in] pData       all pin datas of a GPIO port
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_GetPort(
    gpio_port_t  port,
    uint32_t     *pMode,
    uint32_t     *pConfig,
    uint32_t     *pData);


/**
 *  \brief  Initialize IRQ of a GPIO port
 *
 *  \param [in] port    enum gpio_port_t
 *  \return             0: ok, other: fail
 *
 *  \details
 */
int
GPIO_Irq_Init(
    gpio_port_t  port);


/**
 *  \brief  Terminate IRQ of a GPIo port
 *
 *  \param [in] port    enum gpio_port_t
 *  \return             0: ok, other: fail
 *
 *  \details
 */
int
GPIO_Irq_Deinit(
    gpio_port_t  port);


/**
 *  \brief  Set interruption of a pin
 *
 *  \param [in] pin_id              enum gpio_pin_t
 *  \param [in] is_enable           enable interrupt or not
 *  \param [in] sensing_type        enum gpio_edge_sense_t
 *  \return                         0: ok, other: fail
 *
 *  \details
 */
int
GPIO_Irq_SetPin(
    gpio_pin_t          pin_id,
    bool                is_enable,
    gpio_edge_sense_t   sensing_type);


/**
 *  \brief  Get Interrupt status of a pin of a GPIO
 *
 *  \param [in] pin_id      enum gpio_pin_t
 *  \return                 true: get a irq, false: nothing
 *
 *  \details
 */
uint32_t
GPIO_Irq_GetPinIrq(
    gpio_pin_t      pin_id);


/**
 *  \brief      Get all pins status of a GPIO port
 *
 *  \param [in] port_id     GPIO port id (enum gpio_port)
 *  \return                 IRQ status of a GPIO port
 *
 *  \details
 */
uint32_t
GPIO_Irq_GetAllPinIrq(
    gpio_port_t      port);


/**
 *  \brief      Clear IRQ status of a pin
 *
 *  \param [in] pin_id      pin index of a GPIO port
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_Irq_ClearPinIrq(
    gpio_pin_t      pin_id);


/**
 *  \brief      change GPIO register memory base
 *
 *  \param [in] port_id     GPIO port id (enum gpio_port)
 *  \param [in] base_id     index of Register address base
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_ChangeRegBase(
    gpio_port_t         port_id,
    gpio_reg_base_t     base_id);


/**
 *  \brief      Set driving of a pin
 *
 *  \param [in] pin_id          enum gpio_pin_t
 *  \param [in] driving_type    enum gpio_driving_ctrl
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
GPIO_SetPinDriving(
    gpio_pin_t              pin_id,
    gpio_driving_ctrl_t     driving_type);


uint32_t
GPIO_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif
