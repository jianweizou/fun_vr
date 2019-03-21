/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author ChiaHao Hsu
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

typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t    s8;

typedef const int32_t sc32;
typedef const int16_t sc16;
typedef const int8_t   sc8;

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t    vs8;

typedef __I int32_t  vsc32;
typedef __I int16_t  vsc16;
typedef __I int8_t    vsc8;

typedef uint32_t       u32;
typedef uint16_t       u16;
typedef uint8_t         u8;

typedef const uint32_t uc32;
typedef const uint16_t uc16;
typedef const uint8_t   uc8;

typedef __IO uint32_t  vu32;
typedef __IO uint16_t  vu16;
typedef __IO uint8_t    vu8;

typedef __I uint32_t vuc32;
typedef __I uint16_t vuc16;
typedef __I uint8_t   vuc8;


//=============================================================================
//                      Constant Definition
//=============================================================================
/**
 *  7320 definition
 */
#define SN_I2C0_BASE                    0x40010000UL
#define SN_I2C1_BASE                    0x40011000UL
#define SN_I2C2_BASE                    0x40012000UL

//=============================================================================
//                      Macro Definition
//=============================================================================

//=============================================================================
//                      Structure Definition
//=============================================================================
/**
 *  SN_I2C Structure
 */

typedef struct                                      /*!< SN_I2C0 Structure                                                     */
{

    union
    {
        __IO uint32_t  CTRL;                            /*!< Offset:0x00 I2Cn Control Register                                     */

        struct
        {
            uint32_t             :  1;
            __IO uint32_t  NACK       :  1;               /*!< NACK assert flag                                                      */
            __IO uint32_t  ACK        :  1;               /*!< ACK assert flag                                                       */
            uint32_t             :  1;
            __IO uint32_t  STO        :  1;               /*!< STOP assert flag                                                      */
            __IO uint32_t  STA        :  1;               /*!< START assert flag                                                     */
            uint32_t             :  1;
            __IO uint32_t  I2CMODE    :  1;               /*!< I2C mode                                                              */
            __IO uint32_t  I2CEN      :  1;               /*!< I2Cn interface enable                                                 */
            __IO uint32_t  FIFO_ACT_SEL   :  1;             /*!< I2C Slave FIFO Mode FIFO Full action select bit*/
            __IO uint32_t  FIFO_FILL_STOP :  1;                 /*!< I2C Slave FIFO Mode Force reply NACK bit*/
            __IO uint32_t  RXFIFOCLR      :  1;           /*!< I2C Slave FIFO Mode Rx FIFO Clear*/
            __IO uint32_t  TXFIFOCLR      :  1;           /*!< I2C Slave FIFO Mode Tx FIFO Clear*/
            __IO uint32_t  RX_FIFO_EN     :  1;             /*!< I2C Slave FIFO Mode RX FIFO Enable bit*/
            __IO uint32_t  TX_FIFO_EN     :  1;             /*!< I2C Slave FIFO Mode Tx FIFO Enable bit*/
            __IO uint32_t  FIFO_THD         :  3;           /*!< I2C Slave FIFO Mode Threshold*/
        } CTRL_b;                                       /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  STAT;                            /*!< Offset:0x04 I2Cn Status Register                                      */

        struct
        {
            __I  uint32_t  RX_DN      :  1;               /*!< RX done status                                                        */
            __I  uint32_t  ACK_STAT   :  1;               /*!< ACK done status                                                       */
            __I  uint32_t  NACK_STAT  :  1;               /*!< NACK done status                                                      */
            __I  uint32_t  STOP_DN    :  1;               /*!< STOP done status                                                      */
            __I  uint32_t  START_DN   :  1;               /*!< START done status                                                     */
            __I  uint32_t  I2C_MST    :  1;               /*!< I2C master/slave status                                               */
            __I  uint32_t  SLV_RX_HIT :  1;               /*!< Slave RX address hit flag                                             */
            __I  uint32_t  SLV_TX_HIT :  1;               /*!< Slave TX address hit flag                                             */
            __I  uint32_t  LOST_ARB   :  1;               /*!< Lost arbitration status                                               */
            __I  uint32_t  TIMEOUT    :  1;               /*!< Time-out status                                                       */
            uint32_t             :  5;
            __IO uint32_t  I2CIF      :  1;               /*!< I2C interrupt flag                                                    */
            __IO uint32_t  TX_FIFO_STATE            :  4;       /*!< I2C Tx FIFO Status*/
            __IO uint32_t  RX_FIFO_STATE            :  4;       /*!< I2C Rx FIFO Status*/
            __IO uint32_t  TX_FIFO_EMPTY        :  1;       /*!< */
            __IO uint32_t  TX_FIFO_FULL         :  1;       /*!< */
            __IO uint32_t  RX_FIFO_EMPTY        :  1;       /*!< */
            __IO uint32_t  RX_FIFO_FULL         :  1;       /*!< */
            __IO uint32_t  I2C_OVERFLOW         :  1;       /*!< */
        } STAT_b;                                       /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  TXDATA;                          /*!< Offset:0x08 I2Cn TX Data Register                                     */

        struct
        {
            __IO uint32_t  Data       :  8;               /*!< TX Data                                                               */
        } TXDATA_b;                                     /*!< BitSize                                                               */
    };

    union
    {
        __I  uint32_t  RXDATA;                          /*!< Offset:0x0C I2Cn RX Data Register                                     */

        struct
        {
            __I  uint32_t  Data       :  8;               /*!< RX Data received when RX_DN=1                                         */
        } RXDATA_b;                                     /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  SLVADDR0;                        /*!< Offset:0x10 I2Cn Slave Address 0 Register                             */

        struct
        {
            __IO uint32_t  ADDR       : 10;               /*!< I2Cn slave address 0                                                  */
            uint32_t             : 20;
            __IO uint32_t  GCEN       :  1;               /*!< General call address enable                                           */
            __IO uint32_t  ADD_MODE   :  1;               /*!< Slave address mode                                                    */
        } SLVADDR0_b;                                   /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  SLVADDR1;                        /*!< Offset:0x14 I2Cn Slave Address 1 Register                             */

        struct
        {
            __IO uint32_t  ADDR       : 10;               /*!< I2Cn slave address 1                                                  */
        } SLVADDR1_b;                                   /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  SLVADDR2;                        /*!< Offset:0x18 I2Cn Slave Address 2 Register                             */

        struct
        {
            __IO uint32_t  ADDR       : 10;               /*!< I2Cn slave address 2                                                  */
        } SLVADDR2_b;                                   /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  SLVADDR3;                        /*!< Offset:0x1C I2Cn Slave Address 3 Register                             */

        struct
        {
            __IO uint32_t  ADDR       : 10;               /*!< I2Cn slave address 3                                                  */
        } SLVADDR3_b;                                   /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  SCLHT;                           /*!< Offset:0x20 I2Cn SCL High Time Register                               */

        struct
        {
            __IO uint32_t  SCLH       :  16;               /*!< SCLn High period time=(SCLHT+1)*I2Cn_PCLK cycle                      */
        } SCLHT_b;                                      /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  SCLLT;                           /*!< Offset:0x24 I2Cn SCL Low Time Register                                */

        struct
        {
            __IO uint32_t  SCLL       :  16;               /*!< SCLn Low period time=(SCLLT+1)*I2Cn_PCLK cycle                       */
        } SCLLT_b;                                      /*!< BitSize                                                               */
    };
    __I  uint32_t  RESERVED0;

    union
    {
        __IO uint32_t  TOCTRL;                          /*!< Offset:0x2C I2Cn Timeout Control Register                             */

        struct
        {
            __IO uint32_t  TO         : 16;               /*!< Timeout period time = TO*I2Cn_PCLK cycle                              */
        } TOCTRL_b;                                     /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  MMCTRL;                          /*!< Offset:0x30 I2Cn Monitor Mode Control Register                        */

        struct
        {
            __IO uint32_t  MMEN       :  1;               /*!< Monitor mode enable                                                   */
            __IO uint32_t  SCLOEN     :  1;               /*!< SCLn output enable                                                    */
            __IO uint32_t  MATCH_ALL  :  1;               /*!< Match address selection                                               */
        } MMCTRL_b;                                     /*!< BitSize                                                               */
    };

    union
    {
        __IO uint32_t  EMODE;                          /*!< Offset:0x34 I2Cn Engineer Mode Register                                */

        struct
        {
            __IO uint32_t  ICOD       :  1;               /*!< I2C Open Drain enable                                                 */
            __IO uint32_t  SLVSTAEN   :  1;               /*!< Slave START Enable                                                    */
        } EMODE_b;                                      /*!< BitSize                                                               */
    };
} sn_i2c_t;


//=============================================================================
//  Global Data Definition
//=============================================================================

//=============================================================================
//  Private Function Definition
//=============================================================================

//=============================================================================
//  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
