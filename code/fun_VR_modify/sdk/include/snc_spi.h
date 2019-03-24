/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_spi.h
 *
 * @author kaishiang
 * @version 0.1
 * @date 2018/03/20
 * @license
 * @description
 */

#ifndef __snc_spi_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__
#define __snc_spi_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__

#ifdef __cplusplus
extern "C" {
#endif
#include "snc_types.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

typedef enum spi_interrupt_status
{
    SPI_IRQ_RXOVF       = 0,
    SPI_IRQ_RXTO,
    SPI_IRQ_RXFIFOTH,
    SPI_IRQ_TXFIFOTH,
    
} spi_interrupt_status_t;

/*spi status include fifo,TX/RX,spi busy status*/
typedef enum spi_status
{
    SPI_STATUS_TXEMPTY       = 0,
    SPI_STATUS_TXFULL,
    SPI_STATUS_RXEMPTY,
    SPI_STATUS_RXFULL,
    SPI_STATUS_BUSY,
    SPI_STATUS_TXFIFOTH,
    SPI_STATUS_RXFIFOTH,
    
} spi_status_t;


typedef enum spi_ch
{
    SPI_0       = 0,
    SPI_1,   
} spi_ch_t;

typedef enum spi_mode_t
{
    SPI_MASTER       = 0,
    SPI_SLAVE,   
} spi_mode_t;


typedef enum spi_clock_phase
{
    SPI_CLOCK_PHASE_LOW       = 0,
    SPI_CLOCK_PHASE_HIGH,   
} spi_clock_phase_t;

typedef enum spi_clock_polarity
{
   SPI_CLOCK_POLARITY_FALLING       = 0,
   SPI_CLOCK_POLARITY_RISING,   
} spi_clock_polarity_t;

typedef enum spi_data_mlsb
{
    SPI_MSB_FIRST       = 0,
    SPI_LSB_FIRST,   
} spi_data_mlsb_t;


/*SPI TX/RX FIFO threshold, range is [0-7] */
typedef enum spi_rx_fifo_th
{
    SPI_RX_THRESHOLD0       = 0,
    SPI_RX_THRESHOLD1,
    SPI_RX_THRESHOLD2,
    SPI_RX_THRESHOLD3,
    SPI_RX_THRESHOLD4,
    SPI_RX_THRESHOLD5,
    SPI_RX_THRESHOLD6,
    SPI_RX_THRESHOLD7,
    
} spi_rx_fifo_th_t;

typedef enum spi_tx_fifo_th
{
   
    SPI_TX_THRESHOLD0       = 0,
    SPI_TX_THRESHOLD1,
    SPI_TX_THRESHOLD2,
    SPI_TX_THRESHOLD3,
    SPI_TX_THRESHOLD4,
    SPI_TX_THRESHOLD5,
    SPI_TX_THRESHOLD6,
    SPI_TX_THRESHOLD7,
    
} spi_tx_fifo_th_t;

/*spi data length,range is [3-16] bits*/
typedef enum spi_data_length
{
    SPI_DATA_LENGTH3       = 3,
    SPI_DATA_LENGTH4,
    SPI_DATA_LENGTH5,
    SPI_DATA_LENGTH6,
    SPI_DATA_LENGTH7,
    SPI_DATA_LENGTH8,
    SPI_DATA_LENGTH9,
    SPI_DATA_LENGTH10,
    SPI_DATA_LENGTH11,   
    SPI_DATA_LENGTH12,
    SPI_DATA_LENGTH13,
    SPI_DATA_LENGTH14,
    SPI_DATA_LENGTH15,
    SPI_DATA_LENGTH16,  
    
} spi_data_length_t;


/*spi read/write data method by IRQ or polling*/
typedef enum spi_irq_method
{
     
    SPI_DISABLE_IRQ     				=0,  // polling flag
    SPI_ENABLE_IRQ,	  							 // interrupt read/write flag
    
}spi_irq_method_t;

typedef enum spi_error_code
{
    SPI_INIT_SUCCESS      = 0,
    SPI_READ_DATA_SUCCESS,
		SPI_WRITE_DATA_SUCESS,
    SPI_WRITE_DATA_FAIL, 
    SPI_READ_DATA_FAIL,
    SPI_READ_DATA_TIMEOUT,
    SPI_WRITE_DATA_TIMEOUT,
    SPI_INFO_ERROR,
}spi_error_code_t;



//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct spi_init
{    
 
    uint8_t                 clock_div;       // clock divider
    spi_clock_phase_t       phase_sel;
    spi_clock_polarity_t    polarity_sel;
    spi_data_length_t       spi_data_length; //3-16 bits
    spi_data_mlsb_t         mlsb_sel;       // msb/lsb
    spi_rx_fifo_th_t        rx_fifo_sel;    // rx fifo threshold 0-7
    spi_tx_fifo_th_t        tx_fifo_sel;    // tx fifo threshold 0-7

} spi_init_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

spi_error_code_t SPI_Init(spi_ch_t spi_ch,spi_mode_t spi_mode,spi_init_t *spi_init_info);
spi_error_code_t SPI_Read(spi_ch_t spi_ch,uint16_t *buf,uint32_t data_length,spi_irq_method_t enable_irq);
spi_error_code_t SPI_Write(spi_ch_t spi_ch,uint16_t *buf,uint32_t data_length,spi_irq_method_t enable_irq);
void SPI_Disable(spi_ch_t spi_ch);
void SPI1_IRQ_Handler(void);
void SPI0_IRQ_Handler(void);

uint32_t
SPI_GetVersion(void);
      
#ifdef __cplusplus
}
#endif

#endif
