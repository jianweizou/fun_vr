/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Wei-Lun Hsu
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
//=============================================================================
//      Constant Definition
//=============================================================================
/**
 *  7320 definition
 */
#define UART0_REG_BASE       0x4000A000UL
#define UART1_REG_BASE       0x4000B000UL
//=============================================================================
//      Macro Definition
//=============================================================================

//=============================================================================
//      Structure Definition
//=============================================================================
/**
 *  7320 SN_UART Structure
 */
typedef struct sn_uart
{
    union {
        union {
            __I uint32_t RB;           /*!< Offset:0x00 UARTn Receiver Buffer Register */

            struct {
                __I uint32_t RB : 8;   /*!< The oldest received byte in UART RX FIFO */
            } RB_b; /*!< BitSize */
        };

        union {
            __O uint32_t TH;           /*!< Offset:0x00 UARTn Transmit Holding Register */

            struct {
                __O uint32_t TH : 8;   /*!< The oldest byte to be transmitted in UART TX FIFO when transmitter is available */
            } TH_b; /*!< BitSize */
        };

        union {
            __IO uint32_t DLL;          /*!< Offset:0x00 UARTn Divisor Latch LSB Register */

            struct {
                __IO uint32_t DLL : 8;  /*!< DLL and DLM register determines the baud rate of UARTn */
            } DLL_b; /*!< BitSize */
        };

    } ;

    union {
        union {
            __IO uint32_t IE;               /*!< Offset:0x04 UARTn Interrupt Enable Register */

            struct {
                __IO uint32_t RDAIE  : 1;    /*!< RDA interrupt enable */
                __IO uint32_t THREIE : 1;    /*!< THRE interrupt enable */
                __IO uint32_t RLSIE  : 1;    /*!< RLS interrupt enable */
                     uint32_t        : 1;
                __IO uint32_t TEMTIE : 1;    /*!< TEMT interrupt enable */
                     uint32_t        : 3;
                __IO uint32_t ABEOIE : 1;    /*!< ABE0 interrupt enable */
                __IO uint32_t ABTOIE : 1;    /*!< ABT0 interrupt enable */
            } IE_b; /*!< BitSize */
        };

        union {
            __IO uint32_t DLM;           /*!< Offset:0x04 UARTn Divisor Latch MSB Register */

            struct {
                __IO uint32_t DLM : 8;   /*!< DLL and DLM register determines the baud rate of UARTn */
            } DLM_b; /*!< BitSize */
        };
    } ;

    union {
        union {
            __O uint32_t FIFOCTRL;    /*!< Offset:0x08 UARTn FIFO Control Register */

            struct {
                __O uint32_t FIFOEN   : 1;   /*!< FIFO enable */
                __O uint32_t RXFIFORST: 1;   /*!< RX FIFO reset */
                __O uint32_t TXFIFORST: 1;   /*!< TX FIFO reset */
                    uint32_t          : 3;
                __O uint32_t RXTL     : 2;   /*!< RX trigger level */
            } FIFOCTRL_b; /*!< BitSize */
        };

        union {
            __I uint32_t II;    /*!< Offset:0x08 UARTn Interrupt Identification Register */

            struct {
                __I uint32_t INTSTATUS: 1;   /*!< Interrupt status */
                __I uint32_t INTID    : 3;   /*!< Interrupt ID of RX FIFO */
                    uint32_t          : 2;
                __I uint32_t FIFOEN   : 2;   /*!< Equal to FIFOEN bits in UARTn_FIFOCTRL register */
                __I uint32_t ABEOIF   : 1;   /*!< ABEO interrupt flag */
                __I uint32_t ABTOIF   : 1;   /*!< ABTO interrupt flag */
            } II_b; /*!< BitSize */
        };
    } ;

    union {
        __IO uint32_t LC;      /*!< Offset:0x0C UARTn Line Control Register */

        struct {
            __IO uint32_t WLS  : 2;   /*!< Word length selection */
            __IO uint32_t SBS  : 1;   /*!< Stop bit selection */
            __IO uint32_t PE   : 1;   /*!< Parity enable */
            __IO uint32_t PS   : 2;   /*!< Parity selection */
            __IO uint32_t BC   : 1;   /*!< Break control */
            __IO uint32_t DLAB : 1;   /*!< Divisor Latch access */
        } LC_b; /*!< BitSize */
    };

    __I uint32_t RESERVED0;

    union {
        __I uint32_t LS;            /*!< Offset:0x14 UARTn Line Status Register */

        struct {
            __I uint32_t RDR  : 1;   /*!< Receiver data ready flag */
            __I uint32_t OE   : 1;   /*!< Overrun error flag */
            __I uint32_t PE   : 1;   /*!< Parity error flag */
            __I uint32_t FE   : 1;   /*!< Framing error flag */
            __I uint32_t BI   : 1;   /*!< Break interrupt flag */
            __I uint32_t THRE : 1;   /*!< THR empty flag */
            __I uint32_t TEMT : 1;   /*!< Transmitter empty flag */
            __I uint32_t RXFE : 1;   /*!< Receiver FIFO error flag */
        } LS_b; /*!< BitSize */
    };

    __I uint32_t RESERVED1;

    union {
        __IO uint32_t SP;      /*!< Offset:0x1C UARTn Scratch Pad Register */

        struct {
            __IO uint32_t PAD : 8;   /*!< Pad informaton */
        } SP_b; /*!< BitSize */
    };

    union {
        __IO uint32_t ABCTRL;               /*!< Offset:0x20 UARTn Auto-baud Control Register */

        struct {
            __IO uint32_t START       : 1;   /*!< Auto-baud run bit */
            __IO uint32_t MODE        : 1;   /*!< Auto-baud mode selection */
            __IO uint32_t AUTORESTART : 1;   /*!< Restart mode selection */
            uint32_t                  : 5;
            __O uint32_t ABEOIFC      : 1;   /*!< Clear ABEOIF flag */
            __O uint32_t ABTOIFC      : 1;   /*!< Clear ABTOIF flag */
        } ABCTRL_b; /*!< BitSize */
    };

    __I uint32_t RESERVED2;

    union {
        __IO uint32_t FD;                   /*!< Offset:0x28 UARTn Fractional Divider Register */

        struct {
            __IO uint32_t DIVADDVAL : 4;   /*!< Baud rate generation prescaler divisor value */
            __IO uint32_t MULVAL    : 4;   /*!< Baud rate generation prescaler multiplier value */
            __IO uint32_t OVER8     : 1;   /*!< Oversampling value */
        } FD_b; /*!< BitSize */
    };

    __I uint32_t RESERVED3;

    union {
        __IO uint32_t CTRL;    /*!< Offset:0x30 UARTn Control Register */

        struct {
            __IO uint32_t UARTEN : 1;   /*!< UART enable */
                 uint32_t        : 5;
            __IO uint32_t RXEN   : 1;   /*!< RX enable */
            __IO uint32_t TXEN   : 1;   /*!< TX enable */
        } CTRL_b; /*!< BitSize */
    };

    union {
        __IO uint32_t HDEN;    /*!< Offset:0x34 UARTn Control Register */

        struct {
            __IO uint32_t HDEN : 1;   /*!< Half-duplex mode enable */
        } HDEN_b; /*!< BitSize */
    };

} sn_uart_t;

//=============================================================================
//      Global Data Definition
//=============================================================================

//=============================================================================
//      Private Function Definition
//=============================================================================

//=============================================================================
//      Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
