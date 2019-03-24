/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file uart.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */


#include "snc_uart.h"
#include "reg_util.h"


#include "register_7320.h"
#define UART_VERSION        0x73200001
//=============================================================================
//                  Constant Definition
//=============================================================================

/**
 * support system clock
 */
#define UART_SYSTEM_CLK_162M        162000000
#define UART_SYSTEM_CLK_148_5M      148500000
#define UART_SYSTEM_CLK_96M         96000000
#define UART_SYSTEM_CLK_48M         48000000
#define UART_SYSTEM_CLK_24M         24000000
#define UART_SYSTEM_CLK_12M         12000000
#define UART_SYSTEM_CLK_40M         40000000
#define UART_SYSTEM_CLK_20M         20000000


/**
 * register mask
 */
#define MASK_UART_II_INTERRUPT_STATUS           (0x1 << 0)
#define MASK_UART_II_RX_IRQ_ID                  (0x7 << 1)
#define MASK_UART_II_AUTO_BAUD_END_IRQ          (0x1 << 8)
#define MASK_UART_II_AUTO_BAUD_TIMEOUT_IRQ      (0x1 << 9)
#define MASK_UART_II_TX_ERR_IRQ                 (0x1 << 10)

#define MASK_UART_LS_RX_DATA_READY              (0x1 << 0)
#define MASK_UART_LS_OVER_RUN_ERR               (0x1 << 1)
#define MASK_UART_LS_PARITY_ERR                 (0x1 << 2)
#define MASK_UART_LS_FRAMING_ERR                (0x1 << 3)
#define MASK_UART_LS_BREAK_INTERRUPT            (0x1 << 4)
#define MASK_UART_LS_TX_HOLDING_EMPTY           (0x1 << 5)
#define MASK_UART_LS_TRANSMITTER_EMPTY          (0x1 << 6)
#define MASK_UART_LS_RX_FIFO_ERR                (0x1 << 7)
#define MASK_UART_LS_TX_ERR                     (0x1 << 8)
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct uart_clk_info
{
    uint32_t        sys_clock;
    uint32_t        baud_rate;
    uint32_t        FD;
    uint32_t        DLM;
    uint32_t        DLL;
} uart_clk_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static volatile uint32_t        g_recv_irq_byte[2] = {0};
static const uart_clk_info_t    g_clk_table[] =
                                {
                                #if (1)
                                    {UART_SYSTEM_CLK_162M, 921600, 0x000000a1, 0x00000000, 0x0000000a},
                                    {UART_SYSTEM_CLK_162M, 460800, 0x000001a0, 0x00000000, 0x0000002b},
                                    {UART_SYSTEM_CLK_162M, 230400, 0x000001a0, 0x00000000, 0x00000057},
                                    {UART_SYSTEM_CLK_162M, 115200, 0x000001a0, 0x00000000, 0x000000af},
                                    {UART_SYSTEM_CLK_162M, 57600, 0x000000a0, 0x00000000, 0x000000af},
                                    {UART_SYSTEM_CLK_162M, 38400, 0x000000a0, 0x00000001, 0x00000007},
                                    {UART_SYSTEM_CLK_162M, 19200, 0x000000a0, 0x00000002, 0x0000000f},
                                    {UART_SYSTEM_CLK_162M, 14400, 0x000000a0, 0x00000002, 0x000000bf},
                                    {UART_SYSTEM_CLK_162M, 9600, 0x000000a0, 0x00000004, 0x00000024},
                                    {UART_SYSTEM_CLK_162M, 4800, 0x000000a0, 0x00000008, 0x0000003d},
                                    // {UART_SYSTEM_CLK_162M, 2400, 0x000000a0, 0x00000010, 0x0000007a},
                                    // {UART_SYSTEM_CLK_162M, 1200, 0x000000a0, 0x00000020, 0x000000f5},
                                    // {UART_SYSTEM_CLK_162M, 600, 0x000000a0, 0x00000041, 0x000000eb},
                                    // {UART_SYSTEM_CLK_162M, 300, 0x000000a0, 0x00000083, 0x000000d6},
                                    // {UART_SYSTEM_CLK_162M, 110, 0x000000a0, 0x00000167, 0x0000008d},
                                #endif
                                #if (1)
                                    {UART_SYSTEM_CLK_148_5M, 921600, 0x000000a0, 0x00000000, 0x0000000a},
                                    {UART_SYSTEM_CLK_148_5M, 460800, 0x000000a0, 0x00000000, 0x00000014},
                                    {UART_SYSTEM_CLK_148_5M, 230400, 0x000000a0, 0x00000000, 0x00000028},
                                    {UART_SYSTEM_CLK_148_5M, 115200, 0x000000a0, 0x00000000, 0x00000050},
                                    {UART_SYSTEM_CLK_148_5M, 57600, 0x000000a0, 0x00000000, 0x000000a1},
                                    {UART_SYSTEM_CLK_148_5M, 38400, 0x000000a0, 0x00000000, 0x000000f1},
                                    {UART_SYSTEM_CLK_148_5M, 19200, 0x000000a0, 0x00000001, 0x000000e3},
                                    {UART_SYSTEM_CLK_148_5M, 14400, 0x000000a0, 0x00000002, 0x00000084},
                                    {UART_SYSTEM_CLK_148_5M, 9600, 0x000000a0, 0x00000003, 0x000000c6},
                                    {UART_SYSTEM_CLK_148_5M, 4800, 0x000000a0, 0x00000007, 0x0000008d},
                                    // {UART_SYSTEM_CLK_148_5M, 2400, 0x000000a0, 0x0000000f, 0x0000001b},
                                    // {UART_SYSTEM_CLK_148_5M, 1200, 0x000000a0, 0x0000001e, 0x00000036},
                                    // {UART_SYSTEM_CLK_148_5M, 600, 0x000000a0, 0x0000003c, 0x0000006c},
                                    // {UART_SYSTEM_CLK_148_5M, 300, 0x000000a0, 0x00000078, 0x000000d9},
                                    // {UART_SYSTEM_CLK_148_5M, 110, 0x000000a0, 0x00000149, 0x00000097},
                                #endif
                                #if (0)
                                    {UART_SYSTEM_CLK_96M, 921600, 0x000001a0, 0x00000000, 0x0000000d},
                                    {UART_SYSTEM_CLK_96M, 460800, 0x000000a0, 0x00000000, 0x0000000d},
                                    {UART_SYSTEM_CLK_96M, 230400, 0x000000a0, 0x00000000, 0x0000001a},
                                    {UART_SYSTEM_CLK_96M, 115200, 0x10, 0x0, 0x34},
                                    {UART_SYSTEM_CLK_96M, 57600, 0x72, 0x0, 0x51},
                                    {UART_SYSTEM_CLK_96M, 38400, 0x10, 0x0, 0x9c},
                                    {UART_SYSTEM_CLK_96M, 19200, 0x000000a0, 0x00000001, 0x00000038},
                                    {UART_SYSTEM_CLK_96M, 14400, 0x000000a0, 0x00000001, 0x000000a0},
                                    {UART_SYSTEM_CLK_96M, 9600, 0x000000a0, 0x00000002, 0x00000071},
                                    {UART_SYSTEM_CLK_96M, 4800, 0x000000a0, 0x00000004, 0x000000e2},
                                    {UART_SYSTEM_CLK_96M, 2400, 0x000000a0, 0x00000009, 0x000000c4},
                                    {UART_SYSTEM_CLK_96M, 1200, 0x000000a0, 0x00000013, 0x00000088},
                                    {UART_SYSTEM_CLK_96M, 600, 0x000000a0, 0x00000027, 0x00000010},
                                    {UART_SYSTEM_CLK_96M, 300, 0x000000a0, 0x0000004e, 0x00000020},
                                    {UART_SYSTEM_CLK_96M, 110, 0x000000a0, 0x000000d5, 0x00000011},
                                #endif
                                #if (0)
                                    {UART_SYSTEM_CLK_48M, 921600, 0x000000a1, 0x00000000, 0x00000003},
                                    {UART_SYSTEM_CLK_48M, 460800, 0x000001a0, 0x00000000, 0x0000000d},
                                    {UART_SYSTEM_CLK_48M, 230400, 0x000000a0, 0x00000000, 0x0000000d},
                                    {UART_SYSTEM_CLK_48M, 115200, 0x10, 0x0, 0x1b},
                                    {UART_SYSTEM_CLK_48M, 57600, 0xde, 0x0, 0x1b},
                                    {UART_SYSTEM_CLK_48M, 38400, 0x10, 0x0, 0x4e},
                                    {UART_SYSTEM_CLK_48M, 19200, 0x000000a0, 0x00000000, 0x0000009c},
                                    {UART_SYSTEM_CLK_48M, 14400, 0x000000a0, 0x00000000, 0x000000d0},
                                    {UART_SYSTEM_CLK_48M, 9600, 0x000000a0, 0x00000001, 0x00000038},
                                    {UART_SYSTEM_CLK_48M, 4800, 0x000000a0, 0x00000002, 0x00000071},
                                    {UART_SYSTEM_CLK_48M, 2400, 0x000000a0, 0x00000004, 0x000000e2},
                                    {UART_SYSTEM_CLK_48M, 1200, 0x000000a0, 0x00000009, 0x000000c4},
                                    {UART_SYSTEM_CLK_48M, 600, 0x000000a0, 0x00000013, 0x00000088},
                                    {UART_SYSTEM_CLK_48M, 300, 0x000000a0, 0x00000027, 0x00000010},
                                    {UART_SYSTEM_CLK_48M, 110, 0x000000a0, 0x0000006a, 0x00000088},
                                #endif
                                #if (0)
                                    {UART_SYSTEM_CLK_24M, 921600, 0x000000a6, 0x00000000, 0x00000001},
                                    {UART_SYSTEM_CLK_24M, 460800, 0x000000a1, 0x00000000, 0x00000003},
                                    {UART_SYSTEM_CLK_24M, 230400, 0x000001a0, 0x00000000, 0x0000000d},
                                    {UART_SYSTEM_CLK_24M, 115200, 0x10, 0x0, 0xd},
                                    {UART_SYSTEM_CLK_24M, 57600, 0x10, 0x0, 0x1a},
                                    {UART_SYSTEM_CLK_24M, 38400, 0x10, 0x0, 0x27},
                                    {UART_SYSTEM_CLK_24M, 19200, 0x000000a0, 0x00000000, 0x0000004e},
                                    {UART_SYSTEM_CLK_24M, 14400, 0x000000a0, 0x00000000, 0x00000068},
                                    {UART_SYSTEM_CLK_24M, 9600, 0x000000a0, 0x00000000, 0x0000009c},
                                    {UART_SYSTEM_CLK_24M, 4800, 0x000000a0, 0x00000001, 0x00000038},
                                    {UART_SYSTEM_CLK_24M, 2400, 0x000000a0, 0x00000002, 0x00000071},
                                    {UART_SYSTEM_CLK_24M, 1200, 0x000000a0, 0x00000004, 0x000000e2},
                                    {UART_SYSTEM_CLK_24M, 600, 0x000000a0, 0x00000009, 0x000000c4},
                                    {UART_SYSTEM_CLK_24M, 300, 0x000000a0, 0x00000013, 0x00000088},
                                    {UART_SYSTEM_CLK_24M, 110, 0x000000a0, 0x00000035, 0x00000044},
                                #endif
                                #if (0)
                                    {UART_SYSTEM_CLK_12M, 921600, 0x000001a6, 0x00000000, 0x00000001},
                                    {UART_SYSTEM_CLK_12M, 460800, 0x000000a6, 0x00000000, 0x00000001},
                                    {UART_SYSTEM_CLK_12M, 230400, 0x000000a1, 0x00000000, 0x00000003},
                                    {UART_SYSTEM_CLK_12M, 115200, 0x85, 0x0, 0x4},
                                    {UART_SYSTEM_CLK_12M, 57600, 0x10, 0x0, 0xd},
                                    {UART_SYSTEM_CLK_12M, 38400, 0xa3, 0x0, 0xf},
                                    {UART_SYSTEM_CLK_12M, 19200, 0x000000a0, 0x00000000, 0x00000027},
                                    {UART_SYSTEM_CLK_12M, 14400, 0x000000a0, 0x00000000, 0x00000034},
                                    {UART_SYSTEM_CLK_12M, 9600, 0x000000a0, 0x00000000, 0x0000004e},
                                    {UART_SYSTEM_CLK_12M, 4800, 0x000000a0, 0x00000000, 0x0000009c},
                                    {UART_SYSTEM_CLK_12M, 2400, 0x000000a0, 0x00000001, 0x00000038},
                                    {UART_SYSTEM_CLK_12M, 1200, 0x000000a0, 0x00000002, 0x00000071},
                                    {UART_SYSTEM_CLK_12M, 600, 0x000000a0, 0x00000004, 0x000000e2},
                                    {UART_SYSTEM_CLK_12M, 300, 0x000000a0, 0x00000009, 0x000000c4},
                                    {UART_SYSTEM_CLK_12M, 110, 0x000000a0, 0x0000001a, 0x000000a2},
                                #endif
                                #if 0//defined(FPGA_CASE)
                                    {UART_SYSTEM_CLK_40M, 921600, 0x1a1, 0, 0x5},
                                    {UART_SYSTEM_CLK_40M, 460800, 0xb1, 0x0, 0x5},
                                    {UART_SYSTEM_CLK_40M, 230400, 0x10, 0x0, 0xb},
                                    {UART_SYSTEM_CLK_40M, 115200, 0x110, 0x0, 0x2b},
                                    {UART_SYSTEM_CLK_40M, 57600, 0x110, 0x0, 0x57},
                                    {UART_SYSTEM_CLK_40M, 38400, 0x10, 0x0, 0x41},
                                    {UART_SYSTEM_CLK_40M, 19200, 0x10, 0x0, 0x82},
                                    {UART_SYSTEM_CLK_40M, 14400, 0x10, 0x0, 0xae},
                                    {UART_SYSTEM_CLK_40M, 9600, 0x10, 0x1, 0x4},
                                    {UART_SYSTEM_CLK_40M, 4800, 0x110, 0x4, 0x11},
                                    {UART_SYSTEM_CLK_40M, 2400, 0x10, 0x4, 0x11},
                                    {UART_SYSTEM_CLK_40M, 1200, 0x10, 0x8, 0x23},
                                    {UART_SYSTEM_CLK_40M, 600, 0x10, 0x10, 0x46},
                                    {UART_SYSTEM_CLK_40M, 300, 0x10, 0x20, 0x1},
                                    {UART_SYSTEM_CLK_40M, 110, 0x10, 0x58, 0xc7},

                                    {UART_SYSTEM_CLK_20M, 921600, 0x000000a4, 0x00000000, 0x00000001},
                                    {UART_SYSTEM_CLK_20M, 115200, 0xa1, 0x0, 0x0a},
                                    {UART_SYSTEM_CLK_20M, 57600, 0x1a0, 0x0, 0x2b},
                                    {UART_SYSTEM_CLK_20M, 38400, 0x1a0, 0x0, 0x41},
                                #endif
                                    {0, 0, 0, 0, 0},
                                };
//=============================================================================
//                  Private Function Definition
//=============================================================================
#define SERIAL_BUFFER_SIZE 16

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
};
	
struct ring_buffer rx_buffer = { { 0 }, 0, 0};														
struct ring_buffer rx_buffer1 = { { 0 }, 0, 0};														
																
																														
static sn_uart_t*
_uart_get_handle(
    uart_port_t     port_id)
{
    sn_uart_t   *pDev = 0;

    if( port_id == UART_PORT_0 )
        pDev = (sn_uart_t*)UART0_REG_BASE;
    else if( port_id == UART_PORT_1 )
        pDev = (sn_uart_t*)UART1_REG_BASE;

    return pDev;
}


static void
_uart_irq_handle(
    sn_uart_t   *pDev,
    uint32_t    port)
{
    uint32_t    irq_identify = pDev->II;

    // check interrupt status
    while( (irq_identify & MASK_UART_II_INTERRUPT_STATUS) == 0 )
    {
        // get interrupt and check it's type
        switch( (irq_identify & MASK_UART_II_RX_IRQ_ID) >> 1 )
        {
            case 0x1:   // THRE interrupt
                {
                    uint32_t    line_status = pDev->LS;
                #if 0
                    (void)line_status;
                #else
                    if( line_status & MASK_UART_LS_TX_HOLDING_EMPTY )
                    {
                    }
                #endif
                }
                break;
            case 0x2:   // Receive Data Available
            case 0x6:   // Character Time-out Indicator
                {
                    uint32_t    line_status = pDev->LS;
                    if( line_status & MASK_UART_LS_RX_DATA_READY )
                    {
                        g_recv_irq_byte[port] = (uint32_t)(pDev->RB & 0xFF);
                        g_recv_irq_byte[port] |= 0x80000000;
											  if(port==UART_PORT_0)
												{
													 int i = (unsigned int)(rx_buffer.head + 1) % SERIAL_BUFFER_SIZE;
													 if (i != rx_buffer.tail) {
														 rx_buffer.buffer[rx_buffer.head] = (uint32_t)(pDev->RB & 0xFF);
														 rx_buffer.head = i;
												   }
                         }else if(port==UART_PORT_1)
                         {												
													 int i = (unsigned int)(rx_buffer1.head + 1) % SERIAL_BUFFER_SIZE;
													 if (i != rx_buffer1.tail) {
															rx_buffer1.buffer[rx_buffer1.head] = (uint32_t)(pDev->RB & 0xFF);
															rx_buffer1.head = i;
						               }				  
												 }
                    } 
                }
                break;

            case 0x3:   // Receive Line Status
                {
                    uint32_t    line_status = pDev->LS;

                    if( line_status & MASK_UART_LS_OVER_RUN_ERR )
                    {
                        // Overrun Error
                    }

                    if( line_status & MASK_UART_LS_RX_FIFO_ERR )
                    {
                        #if 1
                        uint32_t    tmp = tmp = pDev->RB;  // read register for unlock it;
                        (void)tmp;
                        #else

                        uint32_t    tmp = 0;

                        /**
                         *  Parity Error amd clear interrupt
                         */
                        if( line_status & MASK_UART_LS_PARITY_ERR )
                            tmp = pDev->RB;  // read register for unlock it

                        /**
                         *  Framing Error amd clear interrupt
                         */
                        if( line_status & MASK_UART_LS_FRAMING_ERR )
                            tmp = pDev->RB; // read register for unlock it

                        /**
                         *  Break Interrupt and clear interrupt
                         */
                        if( line_status & MASK_UART_LS_BREAK_INTERRUPT )
                            tmp = pDev->RB; // read register for unlock it

                        #endif
                    }
                }
                break;

            case 0x7:   // TEMT interrupt
                {
                    uint32_t    line_status = pDev->LS;
                #if 0
                    (void)line_status;
                #else
                    if( line_status & MASK_UART_LS_TRANSMITTER_EMPTY )
                    {
                        // Clear interrupt
                    }
                #endif
                }
                break;

            case 0x0: // Modem status, spec delete
            default:    break;
        }

        irq_identify = pDev->II;
    }


    if( irq_identify & MASK_UART_II_AUTO_BAUD_END_IRQ )
    {
		//Auto Baud interrupt
        reg_write_mask_bits(&pDev->ABCTRL, (0x1 << 8), (0x1 << 8));
    }
    else if( irq_identify & MASK_UART_II_AUTO_BAUD_TIMEOUT_IRQ )
    {
        //Auto Baud time-out interrupt
        reg_write_mask_bits(&pDev->ABCTRL, (0x1 << 9), (0x1 << 9));
    }

    return;
}

static int
_uart_set_freq(
    sn_uart_t   *pDev,
    uint32_t    sys_clock,
    uint32_t    baud_rate)
{
    int                 rval = -1;
    uart_clk_info_t     *pCur = (uart_clk_info_t*)g_clk_table;

    while( pCur->sys_clock )
    {
        if( pCur->sys_clock == sys_clock &&
            pCur->baud_rate == baud_rate )
        {
            rval = 0;
            break;
        }
        pCur++;
    }

    if( pCur->sys_clock )
    {
        pDev->FD  = pCur->FD;
        pDev->DLM = pCur->DLM;
        pDev->DLL = pCur->DLL;
    }

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
UART_Init(
    uart_port_t     port_id,
    uint32_t        sys_clock,
    uint32_t        baud_rate)
{
    int         rval = -1;
    sn_uart_t   *pDev = 0;

    do {
        pDev = _uart_get_handle(port_id);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        //-----------------------------
        // Line Control
        reg_write_mask_bits(&pDev->LC, (0x3 << 0), (0x3 << 0));
        reg_write_mask_bits(&pDev->LC, (0x0 << 2), (0x1 << 2));
        reg_write_mask_bits(&pDev->LC, (0x0 << 3), (0x1 << 3));
        reg_write_mask_bits(&pDev->LC, (0x0 << 4), (0x3 << 4));
        reg_write_mask_bits(&pDev->LC, (0x0 << 6), (0x1 << 6));
        reg_write_mask_bits(&pDev->LC, (0x1 << 7), (0x1 << 7));

        if( _uart_set_freq(pDev, sys_clock, baud_rate) )
        {
            break;
        }

        reg_write_mask_bits(&pDev->LC, (0x0 << 7), (0x1 << 7));

        //-----------------------------
        // FIFO Control
        reg_write_mask_bits(&pDev->FIFOCTRL, (0x1 << 0), (0x1 << 0));
        reg_write_mask_bits(&pDev->FIFOCTRL, (0x1 << 1), (0x1 << 1));
        reg_write_mask_bits(&pDev->FIFOCTRL, (0x1 << 2), (0x1 << 2));
        reg_write_mask_bits(&pDev->FIFOCTRL, (0x2 << 6), (0x3 << 6));

        //-----------------------------
        // Interrupt Enable
        reg_write_mask_bits(&pDev->IE, (0x1 << 0), (0x1 << 0));

        //-----------------------------
        // USART Control
        reg_write_mask_bits(&pDev->CTRL, (0x1 << 0), (0x1 << 0));
        reg_write_mask_bits(&pDev->CTRL, (0x0 << 1), (0x7 << 1));
        reg_write_mask_bits(&pDev->CTRL, (0x1 << 6), (0x1 << 6));
        reg_write_mask_bits(&pDev->CTRL, (0x1 << 7), (0x1 << 7));

        //-----------------------------
        // NVIC
        NVIC_EnableIRQ((IRQn_Type)((uint32_t)UART0_IRQn + port_id));
        NVIC_SetPriority((IRQn_Type)((uint32_t)UART0_IRQn + port_id), 1);

        rval = 0;

    } while(0);

    return rval;
}

int
uart_deinit(
    uart_port_t     port_id)
{
    int     rval = 0;
    return rval;
}










uint32_t
UART_ReadByte(
    uart_port_t     port_id)
{
    uint32_t    value = 0;
    sn_uart_t   *pDev = 0;

    do {
        pDev = _uart_get_handle(port_id);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        // check uart input enable or not
        if( !reg_read_mask_bits(&pDev->CTRL, (0x1 << 6)) )
        {
            break;
        }

        while( ((pDev->II >> 1) & 0x7) != 0x6 &&
               (pDev->LS & 0x1) != 0x1 )
        {
        }

        value = (pDev->RB & 0xFF);

    } while(0);
    return value;
}

#if 1 // code size issue if care that
int
UART_WriteByte(
    uart_port_t     port_id,
    uint8_t         data)
{
    int         rval = -1;
    sn_uart_t   *pDev = 0;

    do {
        pDev = _uart_get_handle(port_id);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        // check uart output enable or not
        if( !reg_read_mask_bits(&pDev->CTRL, (0x1 << 7)) )
        {
            break;
        }

        pDev->TH = data;
        while( (pDev->LS & 0x40) == 0 ) {}

        #if 0 // uart not only for debug port
        if( data == '\n' )
        {
            pDev->TH = '\r';
            while( (pDev->LS & 0x40) == 0 ) {}
        }
        #endif

        rval = 0;
    } while(0);

    return rval;
}
#endif

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
    uint32_t        length)
{
    int         rval = -1;
    sn_uart_t   *pDev = 0;

    do {
        pDev = _uart_get_handle(port_id);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }

        // check uart output enable or not
        if( !reg_read_mask_bits(&pDev->CTRL, (0x1 << 7)) )
        {
            break;
        }

        while( length-- )
        {
            uint32_t    data = *pData++;

            pDev->TH = data & 0xFF;
            while( (pDev->LS & 0x40) == 0 ) {}

            #if 0 // uart not only for debug port
            if( data == '\n' )
            {
                pDev->TH = '\r';
                while( (pDev->LS & 0x40) == 0 ) {}
            }
            #endif
        }

        rval = 0;
    } while(0);

    return rval;
}



uint32_t
UART_ReadIrqByte(
    uart_port_t     port_id)
{
    uint32_t    value = g_recv_irq_byte[port_id];

    // clear
    g_recv_irq_byte[port_id] = 0UL;

    return (value & 0x80000000) ? (value & 0xFF) : 0;
}




#if 0
/**
 *  \brief  uart IRQ handle
 *
 *  \param [in] port_id     target uart port
 *  \return
 *
 *  \details
 */
uint32_t
uart_irq_handle(
    uart_port_t     port_id)
{
    int         rval = -1;
    sn_uart_t   *pDev = 0;

    do {
        pDev = _uart_get_handle(port_id);
        if( !pDev )     break;

        _uart_irq_handle(pDev, port_id);

        rval = 0;
    } while(0);

    return rval;
}
#else
__irq void UART0_IRQHandler (void)
{
    _uart_irq_handle((sn_uart_t*)UART0_REG_BASE, UART_PORT_0);
    return;
}

__irq void UART1_IRQHandler (void)
{
    _uart_irq_handle((sn_uart_t*)UART1_REG_BASE, UART_PORT_1);
}
#endif

uint32_t
UART_GetVersion(void)
{
    return UART_VERSION;
}

int UART_READ(uart_port_t     port_id) {
	 struct ring_buffer *_rx_buffer;
   if(port_id==UART_PORT_0)
		 _rx_buffer=&rx_buffer;
	 else if(port_id==UART_PORT_1)
		 _rx_buffer=&rx_buffer1;
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    return c;
  }
}

int UART_AVAILABLE(uart_port_t     port_id) {
	 struct ring_buffer *_rx_buffer;
   if(port_id==UART_PORT_0)
		 _rx_buffer=&rx_buffer;
	 else if(port_id==UART_PORT_1)
		 _rx_buffer=&rx_buffer1;	
   return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
}

int UART_PEEK(uart_port_t     port_id) {
	 struct ring_buffer *_rx_buffer;
   if(port_id==UART_PORT_0)
		 _rx_buffer=&rx_buffer;
	 else if(port_id==UART_PORT_1)
		 _rx_buffer=&rx_buffer1;	
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    return _rx_buffer->buffer[_rx_buffer->tail];
  }
}

int UART_END(uart_port_t     port_id) {
	 struct ring_buffer *_rx_buffer;
   if(port_id==UART_PORT_0)
		 _rx_buffer=&rx_buffer;
	 else if(port_id==UART_PORT_1)
		 _rx_buffer=&rx_buffer1;	
   _rx_buffer->head = _rx_buffer->tail=0 ;
   return 0;
}


int UART_WRITEBYTE(uart_port_t port_id, uint8_t data)
{
    int         rval = -1;
    sn_uart_t   *pDev = 0;

    do {
        pDev = _uart_get_handle(port_id);
        if( !pDev )
        {
            snc_assert(pDev != 0);
            break;
        }
        // check uart output enable or not
        if( !reg_read_mask_bits(&pDev->CTRL, (0x1 << 7)) )
        {
            break;
        }
        pDev->TH = data;
        while( (pDev->LS & 0x40) == 0 ) {}
        rval = 0;
    } while(0);
    return rval;
}
