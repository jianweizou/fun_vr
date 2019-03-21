/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file uart.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */

#ifndef __uart_H_wqRXNx6S_lD9m_HkhA_satS_u1FWZ8412NoE__
#define __uart_H_wqRXNx6S_lD9m_HkhA_satS_u1FWZ8412NoE__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  uart port number
 */
typedef enum uart_port
{
    UART_PORT_0     = 0,
    UART_PORT_1,
    UART_PORT_NUM
} uart_port_t;


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
 *  \brief      uart initialize
 *
 *  \param [in] port_id         target uart port
 *  \param [in] sys_clock       current system clock
 *  \param [in] baud_rate       target baud rate
 *  \return 0: ok, other: fail
 *
 *  \details
 */
int
UART_Init(
    uart_port_t     port_id,
    uint32_t        sys_clock,
    uint32_t        baud_rate);


/**
 *  \brief  uart read byte data
 *
 *  \param [in] port_id     target uart port
 *  \return     received data
 *
 *  \details
 */
uint32_t
UART_ReadByte(
    uart_port_t     port_id);

/**
 *  \brief      uart send byte data
 *
 *  \param [in] port_id     target uart port
 *  \param [in] data        sent data
 *  \return     0: ok, other: fail
 *
 *  \details
 */
int
UART_WriteByte(
    uart_port_t     port_id,
    uint8_t         data);


/**
 *  \brief  Uart write multi-bytes
 *
 *  \param [in] port_id     target uart port
 *  \param [in] pData       the pointer of data
 *  \param [in] length      date length
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
UART_WriteMultiBytes(
    uart_port_t     port_id,
    uint8_t         *pData,
    uint32_t        length);


/**
 *  \brief  version of uart module
 *
 *  \return     version number
 *
 *  \details
 */
uint32_t UART_GetVersion(void);
int UART_READ(uart_port_t     port_id);
int UART_AVAILABLE(uart_port_t  port_id);
int UART_PEEK(uart_port_t     port_id);
int UART_END(uart_port_t     port_id);
int UART_WRITEBYTE(uart_port_t port_id, uint8_t data);



#ifdef __cplusplus
}
#endif

#endif
