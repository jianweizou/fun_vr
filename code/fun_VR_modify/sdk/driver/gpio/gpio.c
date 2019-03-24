/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gpio.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */


#include "snc_gpio.h"
#include "reg_util.h"

#include "register_7320.h"
#define GPIO_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================
#define BIT_BAND_ENABLE         1
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t     g_gpio_base_offset[GPIO_PORT_NUM] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
sn_gpio_t*
_gpio_get_handle(
    gpio_pin_t      pin_id,
    uint32_t        *pPort,
    uint32_t        *pPin)
{
    sn_gpio_t   *pDev = 0;
    uint32_t    port = (uint32_t)(-1);
    uint32_t    pin = (uint32_t)(-1);

    if( pin_id == GPIO_PIN_NUM )
    {
        port = *pPort;
    }
    else
    {
        port = pin_id >> 4;
        pin  = pin_id & 0xF;
    }

    switch(port)
    {
        case 0:     pDev = (sn_gpio_t*)(SN_GPIO_0_BASE + g_gpio_base_offset[port]); break;
        case 1:     pDev = (sn_gpio_t*)(SN_GPIO_1_BASE + g_gpio_base_offset[port]); break;
        case 2:     pDev = (sn_gpio_t*)(SN_GPIO_2_BASE + g_gpio_base_offset[port]); break;
        case 3:     pDev = (sn_gpio_t*)(SN_GPIO_3_BASE + g_gpio_base_offset[port]); break;
        case 4:     pDev = (sn_gpio_t*)(SN_GPIO_4_BASE + g_gpio_base_offset[port]); break;
        default:    break;
    }

    if( pPort )     *pPort = port;
    if( pPin )      *pPin  = pin;

    return pDev;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
GPIO_SetPin(
    gpio_pin_t      pin_id,
    gpio_mode_t     mode,
    gpio_config_t   config,
    int             data)
{
    int         rval = -1;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        //---------------------------
        // reset pin
        if( config != GPIO_CONFIG_IGNORE )
        {
            #if (BIT_BAND_ENABLE)
            BITBAND_PERI_CLEAR_BIT(&pDev->MODE, pin);
            BITBAND_PERI_CLEAR_BIT(&pDev->DATA, pin);
            reg_write_mask_bits(&pDev->CFG, (GPIO_CONFIG_INACTIVE << (pin << 1)), (0x3 << (pin << 1)));
            #else
            reg_clear_bit(&pDev->MODE, pin);
            reg_write_mask_bits(&pDev->BCLR, (0x1 << pin), (0x1 << pin));

            switch(pin)
            {
                case  0:    pDev->CFG_b.CFG0  = GPIO_CONFIG_INACTIVE; break;
                case  1:    pDev->CFG_b.CFG1  = GPIO_CONFIG_INACTIVE; break;
                case  2:    pDev->CFG_b.CFG2  = GPIO_CONFIG_INACTIVE; break;
                case  3:    pDev->CFG_b.CFG3  = GPIO_CONFIG_INACTIVE; break;
                case  4:    pDev->CFG_b.CFG4  = GPIO_CONFIG_INACTIVE; break;
                case  5:    pDev->CFG_b.CFG5  = GPIO_CONFIG_INACTIVE; break;
                case  6:    pDev->CFG_b.CFG6  = GPIO_CONFIG_INACTIVE; break;
                case  7:    pDev->CFG_b.CFG7  = GPIO_CONFIG_INACTIVE; break;
                case  8:    pDev->CFG_b.CFG8  = GPIO_CONFIG_INACTIVE; break;
                case  9:    pDev->CFG_b.CFG9  = GPIO_CONFIG_INACTIVE; break;
                case 10:    pDev->CFG_b.CFG10 = GPIO_CONFIG_INACTIVE; break;
                case 11:    pDev->CFG_b.CFG11 = GPIO_CONFIG_INACTIVE; break;
                case 12:    pDev->CFG_b.CFG12 = GPIO_CONFIG_INACTIVE; break;
                case 13:    pDev->CFG_b.CFG13 = GPIO_CONFIG_INACTIVE; break;
                case 14:    pDev->CFG_b.CFG14 = GPIO_CONFIG_INACTIVE; break;
                case 15:    pDev->CFG_b.CFG15 = GPIO_CONFIG_INACTIVE; break;
            }
            #endif
        }

        //---------------------------
        // set mode
        if( mode != GPIO_MODE_IGNORE )
        {
            #if (BIT_BAND_ENABLE)
            if( mode == GPIO_MODE_OUTPUT )
                BITBAND_PERI_SET_BIT(&pDev->MODE, pin);
            else
                BITBAND_PERI_CLEAR_BIT(&pDev->MODE, pin);
            #else
            if( mode == GPIO_MODE_OUTPUT )
                reg_set_bit(&pDev->MODE, pin);
            else
                reg_clear_bit(&pDev->MODE, pin);
            #endif
        }

        //---------------------------
        // set data
        if( data != GPIO_PIN_DATA_IGNORE )
        {
            #if (BIT_BAND_ENABLE)
            if( data )
                BITBAND_PERI_SET_BIT(&pDev->DATA, pin);
            else
                BITBAND_PERI_CLEAR_BIT(&pDev->DATA, pin);
            #else
            if( data )
                reg_write_mask_bits(&pDev->BSET, (0x1 << pin), (0x1 << pin));
            else
                reg_write_mask_bits(&pDev->BCLR, (0x1 << pin), (0x1 << pin));
            #endif
        }

        //---------------------------
        // set configuration
        if( config != GPIO_CONFIG_IGNORE )
        {
            switch(pin)
            {
                case  0:    pDev->CFG_b.CFG0  = config; break;
                case  1:    pDev->CFG_b.CFG1  = config; break;
                case  2:    pDev->CFG_b.CFG2  = config; break;
                case  3:    pDev->CFG_b.CFG3  = config; break;
                case  4:    pDev->CFG_b.CFG4  = config; break;
                case  5:    pDev->CFG_b.CFG5  = config; break;
                case  6:    pDev->CFG_b.CFG6  = config; break;
                case  7:    pDev->CFG_b.CFG7  = config; break;
                case  8:    pDev->CFG_b.CFG8  = config; break;
                case  9:    pDev->CFG_b.CFG9  = config; break;
                case 10:    pDev->CFG_b.CFG10 = config; break;
                case 11:    pDev->CFG_b.CFG11 = config; break;
                case 12:    pDev->CFG_b.CFG12 = config; break;
                case 13:    pDev->CFG_b.CFG13 = config; break;
                case 14:    pDev->CFG_b.CFG14 = config; break;
                case 15:    pDev->CFG_b.CFG15 = config; break;
            }
        }

        rval = 0;
    } while(0);

    return rval;
}


int
GPIO_SetPinData(gpio_pin_t  pin_id,int data)
{
    int         rval = -1;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }
        //---------------------------
        // set data
        if( data != GPIO_PIN_DATA_IGNORE )
        {
            #if (BIT_BAND_ENABLE)
            if( data )
                BITBAND_PERI_SET_BIT(&pDev->DATA, pin);
            else
                BITBAND_PERI_CLEAR_BIT(&pDev->DATA, pin);
            #else
            if( data )
                reg_write_mask_bits(&pDev->BSET, (0x1 << pin), (0x1 << pin));
            else
                reg_write_mask_bits(&pDev->BCLR, (0x1 << pin), (0x1 << pin));
            #endif
        }
        rval = 0;
    } while(0);
    return rval;
}

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
    gpio_pin_t      pin_id)
{
    uint32_t    data = 0;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        #if (BIT_BAND_ENABLE)
        data = BITBAND_PERI_GET_BIT(&pDev->DATA, pin);
        #else
        data = reg_read_mask_bits(&pDev->DATA, (0x1 << pin));
        #endif
        // data >>= pin;

    } while(0);

    return data;
}


int GPIO_GetPinCfg(gpio_pin_t  pin_id,uint32_t *pMode,uint32_t *pConfig)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;
    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        if( pMode )         *pMode   = BITBAND_PERI_GET_BIT((volatile uint32_t*)&pDev->MODE,pin);
        if( pConfig )       *pConfig = BITBAND_PERI_GET_BIT((volatile uint32_t*)&pDev->CFG,pin);
    } while(0);

    return rval;
}









/**
 *  \brief      Toggle the pin data
 *
 *  \param [in] pin_id      enum gpio_pin_t
 *  \return                 0: ok, othre: fail
 *
 *  \details
 */
int
GPIO_TogglePin(
    gpio_pin_t      pin_id)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            rval = -1;
            break;
        }

        reg_toggle_bit(&pDev->DATA, pin);

    } while(0);

    return rval;
}
/**
 *  \brief  set all GPIO pins in a port
 *
 *  \param [in] port        enum gpio_port_t
 *  \param [in] mode        all pin modes of a GPIO port
 *  \param [in] config      all pin configurations of a GPIO port
 *  \param [in] data        all pin datas of a GPIO port
 *  \return                 0: ok, othre: fail
 *
 *  \details
 */
int
GPIO_SetPort(
    gpio_port_t  port,
    uint32_t     mode,
    uint32_t     config,
    uint32_t     data)
{
    int         rval = -1;
    sn_gpio_t   *pDev = 0;

    do {
        pDev = _gpio_get_handle(GPIO_PIN_NUM, (uint32_t*)&port, 0);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        // FIXME: when GPIO chang configuration, mode/data will crazy
        reg_write_bits(&pDev->MODE, mode);
        reg_write_bits(&pDev->DATA, data);
        reg_write_bits(&pDev->CFG, config);

        rval = 0;
    } while(0);

    return rval;
}

/**
 *  \brief      Get value of a GPIO Port
 *
 *  \param [in] port        enum gpio_port_t
 *  \param [in] pMode       all pin modes of a GPIO port
 *  \param [in] pConfig     all pin configurations of a GPIO port
 *  \param [in] pData       all pin datas of a GPIO port
 *  \return     0: ok, other: fail
 *
 *  \details
 */
int
GPIO_GetPort(
    gpio_port_t  port,
    uint32_t     *pMode,
    uint32_t     *pConfig,
    uint32_t     *pData)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        pDev = _gpio_get_handle(GPIO_PIN_NUM, (uint32_t*)&port, 0);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        if( pMode )         *pMode   = reg_read_bits((volatile uint32_t*)&pDev->MODE);
        if( pConfig )       *pConfig = reg_read_bits((volatile uint32_t*)&pDev->CFG);
        if( pData )         *pData   = reg_read_bits((volatile uint32_t*)&pDev->DATA);

    } while(0);

    return rval;
}

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
    gpio_port_t  port)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        pDev = _gpio_get_handle(GPIO_PIN_NUM, (uint32_t*)&port, 0);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        // enable irq
        NVIC_ClearPendingIRQ((IRQn_Type)((uint32_t)GPIO0_IRQn + port));
        NVIC_EnableIRQ((IRQn_Type)((uint32_t)GPIO0_IRQn + port));
    } while(0);

    return rval;
}

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
    gpio_port_t  port)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        pDev = _gpio_get_handle(GPIO_PIN_NUM, (uint32_t*)&port, 0);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        // disable irq
        NVIC_ClearPendingIRQ((IRQn_Type)((uint32_t)GPIO0_IRQn + port));
        NVIC_DisableIRQ((IRQn_Type)((uint32_t)GPIO0_IRQn + port));
    } while(0);

    return rval;
}


int
GPIO_Irq_SetPin(
    gpio_pin_t          pin_id,
    bool                is_enable,
    gpio_edge_sense_t   sensing_type)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);
        uint32_t        port = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, &port, &pin);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

    #if (BIT_BAND_ENABLE)
        // disable irq
        BITBAND_PERI_CLEAR_BIT(&pDev->IE, pin);
        if( !is_enable )    break;

        // configure
        switch( sensing_type )
        {
            case GPIO_EDGE_SENSE_HIGH_LEVEL:
                BITBAND_PERI_SET_BIT(&pDev->IS, pin);
                BITBAND_PERI_CLEAR_BIT(&pDev->IBS, pin);
                BITBAND_PERI_CLEAR_BIT(&pDev->IEV, pin);
                break;
            case GPIO_EDGE_SENSE_LOW_LEVEL:
                BITBAND_PERI_SET_BIT(&pDev->IS, pin);
                BITBAND_PERI_CLEAR_BIT(&pDev->IBS, pin);
                BITBAND_PERI_SET_BIT(&pDev->IEV, pin);
                break;
            case GPIO_EDGE_SENSE_BOTH:
                BITBAND_PERI_CLEAR_BIT(&pDev->IS, pin);
                BITBAND_PERI_SET_BIT(&pDev->IBS, pin);
                break;
            case GPIO_EDGE_SENSE_RISING:
                BITBAND_PERI_CLEAR_BIT(&pDev->IS, pin);
                BITBAND_PERI_CLEAR_BIT(&pDev->IBS, pin);
                BITBAND_PERI_CLEAR_BIT(&pDev->IEV, pin);
                break;
            case GPIO_EDGE_SENSE_FALLING:
                BITBAND_PERI_CLEAR_BIT(&pDev->IS, pin);
                BITBAND_PERI_CLEAR_BIT(&pDev->IBS, pin);
                BITBAND_PERI_SET_BIT(&pDev->IEV, pin);
                break;
            default:
                rval = -1;
                return rval;
        }

        BITBAND_PERI_SET_BIT(&pDev->IE, pin);
        BITBAND_PERI_SET_BIT(&pDev->IC, pin); // clear irq flag

    #else // #if (BIT_BAND_ENABLE)
        // disable irq
        reg_clear_bit(&pDev->IE, pin);
        if( !is_enable )    break;

        // configure
        switch( sensing_type )
        {
            case GPIO_EDGE_SENSE_HIGH_LEVEL:
                reg_set_bit(&pDev->IS, pin);
                reg_clear_bit(&pDev->IBS, pin);
                reg_clear_bit(&pDev->IEV, pin);
                break;
            case GPIO_EDGE_SENSE_LOW_LEVEL:
                reg_set_bit(&pDev->IS, pin);
                reg_clear_bit(&pDev->IBS, pin);
                reg_set_bit(&pDev->IEV, pin);
                break;
            case GPIO_EDGE_SENSE_BOTH:
                reg_clear_bit(&pDev->IS, pin);
                reg_set_bit(&pDev->IBS, pin);
                break;
            case GPIO_EDGE_SENSE_RISING:
                reg_clear_bit(&pDev->IS, pin);
                reg_clear_bit(&pDev->IBS, pin);
                reg_clear_bit(&pDev->IEV, pin);
                break;
            case GPIO_EDGE_SENSE_FALLING:
                reg_clear_bit(&pDev->IS, pin);
                reg_clear_bit(&pDev->IBS, pin);
                reg_set_bit(&pDev->IEV, pin);
                break;
            default:
                rval = -1;
                return rval;
        }

        reg_set_bit((volatile uint32_t*)&pDev->IE, pin);
        reg_set_bit((volatile uint32_t*)&pDev->IC, pin); // clear irq flag
    #endif // #if (BIT_BAND_ENABLE)
    } while(0);

    return rval;
}

/**
 *  \brief  Get Interrupt status of a pin of a GPIO
 *
 *  \param [in] pin_id      pin index of a GPIO port
 *  \return                 true: get a irq, false: nothing
 *
 *  \details
 */
uint32_t
GPIO_Irq_GetPinIrq(
    gpio_pin_t      pin_id)
{
    uint32_t    data = 0;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            break;
        }

        #if (BIT_BAND_ENABLE)
        data = BITBAND_PERI_GET_BIT(&pDev->RIS, pin);
        #else
        data = reg_read_mask_bits((volatile uint32_t*)&pDev->RIS, (0x1 << pin));
        #endif

    } while(0);

    return data;
}

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
    gpio_port_t      port)
{
    uint32_t    data = 0;
    sn_gpio_t   *pDev = 0;

    do {
        pDev = _gpio_get_handle(GPIO_PIN_NUM, (uint32_t*)&port, 0);
        if( !pDev )
        {
            break;
        }

        data = reg_read_bits((volatile uint32_t*)&pDev->RIS);

    } while(0);

    return data;
}

int
GPIO_Irq_ClearPinIrq(
    gpio_pin_t      pin_id)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            rval = -1;
            break;
        }

        #if (BIT_BAND_ENABLE)
        BITBAND_PERI_SET_BIT(&pDev->IC, pin);
        #else
        reg_write_mask_bits(&pDev->IC, (0x1 << pin), (0x1 << pin));
        #endif
    } while(0);

    return rval;
}

/**
 *  \brief      change GPIO register memory base
 *
 *  \param [in] port_id     pin index of a GPIO port
 *  \param [in] base_id     index of Register address base
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
GPIO_ChangeRegBase(
    gpio_port_t         port_id,
    gpio_reg_base_t     base_id)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        pDev = _gpio_get_handle(GPIO_PIN_NUM, (uint32_t*)&port_id, 0);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        /**
         *  the name and description of chip spec are meaningless.
         *  It only change the register base address of a GPIO port
         */
        if( base_id == GPIO_REG_BASE_1 )
        {
            g_gpio_base_offset[port_id] = SN_GPIO_BASE_OFFSET;

            #if (BIT_BAND_ENABLE)
            BITBAND_PERI_SET_BIT(&pDev->BS, 0);
            #else
            reg_set_bit(&pDev->BS, 0);
            #endif
        }
        else
        {
            g_gpio_base_offset[port_id] = 0;

            #if (BIT_BAND_ENABLE)
            BITBAND_PERI_CLEAR_BIT(&pDev->BS, 0);
            #else
            reg_clear_bit(&pDev->BS, 0);
            #endif
        }

    } while(0);

    return rval;
}

int
GPIO_SetPinDriving(
    gpio_pin_t              pin_id,
    gpio_driving_ctrl_t     driving_type)
{
    int         rval = 0;
    sn_gpio_t   *pDev = 0;

    do {
        uint32_t        pin = (uint32_t)(-1);
        uint32_t        data = 0;

        pDev = _gpio_get_handle(pin_id, 0, &pin);
        if( !pDev )
        {
            rval = -1;
            snc_assert(pDev != 0);
            break;
        }

        pin <<= 1;
        reg_write_mask_bits(&pDev->DRVCTRL, (driving_type << pin), (0x3 << pin));

        pin >>= 1;
        data = (driving_type == GPIO_DRIVING_NONE) ? 0x0 : 0x1;

        /**
         *  the name and description of chip spec are meaningless.
         *  this register is only to enable the pin driving
         */
        reg_write_mask_bits(&pDev->DRVEN, (data << pin), (0x1 << pin));

    } while(0);

    return rval;
}

uint32_t
GPIO_GetVersion(void)
{
    return GPIO_VERSION;
}



