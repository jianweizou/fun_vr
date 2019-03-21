/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file spi_dma.h
 *
 * @author Kaishiang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_spi_dma_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__
#define __snc_spi_dma_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"

	
typedef enum spi_dma_result
{
    SPIDMA_PASS       = 0,
    SPIDMA_FAIL,   
} spi_dma_result_t;




/*IP SEL*/
#define	SEL_SPIDMA0				SN_SPI0_DMA_IP
#define	SEL_SPIDMA1				SN_SPI1_DMA_IP


typedef enum spi_dma_sel
{
    SPI_DMA_0       = 0,
    SPI_DMA_1,   
} spi_dma_sel_t;	

typedef enum spi_idma_ch
{    
	SPI_IDMA_0       = 12,
	SPI_IDMA_1,          
} spi_idma_ch_t;

typedef enum spi_dma_prescale
{
	prescale_2       =0,
	prescale_4       =1,
	prescale_8       =2,
	prescale_16       =3,
	prescale_32       =4,
	prescale_64       =5,
	prescale_128       =6,
	prescale_256       =7,
} spi_dma_prescale_t;	


typedef enum spi_dma_io_mode
{
	SPIDMA_1IO_MODE       = 0,
	SPIDMA_2IO_MODE,  
	SPIDMA_4IO_MODE,
	SPIDMA_3WIRE_MODE,	
} spi_dma_io_mode_t;


typedef enum spi_dma_clock_polarity
{
	SPIDMA_PHASE_POLARITY_MODE0       = 0,
	SPIDMA_PHASE_POLARITY_MODE1,  
	SPIDMA_PHASE_POLARITY_MODE2,
	SPIDMA_PHASE_POLARITY_MODE3,	
} spi_dma_clock_polarity_t;

/*IP DMA Endian Select*/
//#define	SPIDMA_LITTLE_ENDIAN		0
//#define	SPIDMA_BIG_ENDIAN				1

typedef enum spi_dma_data_mlsb
{
    SPIDMA_LITTLE_ENDIAN       = 0,
    SPIDMA_BIG_ENDIAN,   
} spi_dma_data_mlsb_t;


/*IP FW Set CS pin*/
//#define	SPIDMA_CS_LOW						0
//#define	SPIDMA_CS_HIGH					1


typedef enum spi_dma_cs
{
    SPIDMA_CS_LOW       = 0,
    SPIDMA_CS_HIGH,   
}spi_dma_cs_t;


/*IP FW Read Write Define*/
#define	SPIDMA_FW_READ					0
#define	SPIDMA_FW_WRITE					1


typedef enum spi_dma_transfer
{
	SPIDMA_8BIT_TRANSFER       = 8,
	SPIDMA_16BIT_TRANSFER	   	 =16,
	SPIDMA_24BIT_TRANSFER      =24,
	SPIDMA_32BIT_TRANSFER      =32,
	SPIDMA_40BIT_TRANSFER      =40,   
	SPIDMA_48BIT_TRANSFER      =48,
}spi_dma_transfer_t;


typedef struct spi_dma_init
{    
    uint8_t                 		clock_div;       // clock divider
		spi_dma_prescale_t          prescale;
    spi_dma_clock_polarity_t    polarity_sel;
    spi_dma_io_mode_t      		 	io_mode; //3-16 bits
    spi_dma_transfer_t         	transfer_mode;         
} spi_dma_init_t;

/*_____ M A C R O S ________________________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*IP Driver*/
void SPI_DMA_Endial_Set(spi_dma_sel_t spi_dma_sel, uint8_t endian_set);
void SPI_DMA_Disable(spi_dma_sel_t spi_dma_sel);
void SPI_DMA_Enable(spi_dma_sel_t spi_dma_sel);
void SPI_DMA_ClockRate(spi_dma_sel_t spi_dma_sel, uint8_t PreScaler, uint8_t Div);
void SPI_DMA_ClockPhasePolarity(spi_dma_sel_t spi_dma_sel, uint8_t MODE);
void SPI_DMA_FWSet_CS(spi_dma_sel_t spi_dma_sel, spi_dma_cs_t CS_Set);
uint32_t SPI_DMA_FWSend_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint8_t TransferLenth);
uint32_t SPI_DMA_FWReceive_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint8_t TransferLenth);
uint32_t SPI_DMA_Send_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint32_t TransferLenth);
uint32_t SPI_DMA_Receive_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint32_t TransferLenth);
void SPI_DMA_IRQ_Enable(spi_dma_sel_t spi_dma_sel);
void SPI_DMA_IRQ_Disable(spi_dma_sel_t spi_dma_sel);
void SPI_DMA_Init(spi_dma_sel_t spi_dma_ch,spi_dma_init_t *spi_dma_init_info );   

uint32_t SPI_DMA_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
