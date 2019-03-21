/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_spi_nand.h
 *
 * @author Kaishiang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_spi_nand_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__
#define __snc_spi_nand_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
#include "snc_spi_dma.h"
	
/*NAND Address Struct*/
#define	NAND_BLOCK_ADDR_LEN		11
#define	NAND_PAGE_ADDR_LEN		6
#define	NAND_BYTE_ADDR_LEN		12
#define	NAND_DUMMY_ADDR			3
#define	SEL_SPIDMA0				SN_SPI0_DMA_IP	
typedef enum spi_nand
{
    SPINAND_PASS       = 0,
    SPINAND_FAIL,   
} spi_nand_t;


typedef enum spi_nand_command
{
	SPINAND_WRITE_DISABLE_CMD       = 0x04,
	SPINAND_WRITE_ENABLE_CMD		= 0x06,
	SPINAND_RESET_CMD      			= 0xFF,
	SPINAND_BLOCK_ERASE      		= 0xD8,
	SPINAND_GET_FEATURE_CMD      	= 0x0F,   
	SPINAND_SET_FEATURE_CMD      	= 0x1F,
	SPINAND_READ_ID                 = 0x9F,
	SPINAND_PAGE_READ               = 0x13,
	SPINAND_PROGRAM_EXECUTE         = 0x10,
	SPINAND_PROGRAM_LOAD_X1         = 0x02,
	SPINAND_PROGRAM_LOAD_X4         = 0x32,
	SPINAND_READ_CACHE_X1           = 0x03,
	SPINAND_READ_CACHE_X2           = 0x3B,
	SPINAND_READ_CACHE_X4           = 0x6B,		
} spi_nand_command_t;


typedef enum spi_nand_feature
{
    SPINAND_BP_REG       		= 0xA0,
    SPINAND_CONF_REG			= 0xB0,
	SPINAND_STA_REG     		= 0xC0,
		
} spi_nand_feature_t;

/*SPI NAND Flash Type Select*/

typedef enum spi_nand_type
{
    SPINAND_512BYTE	= 0,
    SPINAND_2048BYTE,
	SPINAND_4096BYTE,		
} spi_nand_type_t;

union st_NAND_ADDR
{
	uint32_t Addr;
	struct{
		uint32_t Byte_Num		:NAND_BLOCK_ADDR_LEN;
		uint32_t Page_Num		:NAND_PAGE_ADDR_LEN;
		uint32_t Block_Num		:NAND_BYTE_ADDR_LEN;
        uint32_t RESERVED		:NAND_DUMMY_ADDR;
	}Bit;
};


/*_____ M A C R O S ________________________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*IP Driver*/


/*Control SPI Nand Common Func*/
void SPI_DMA_Nand_Reset(spi_dma_sel_t spi_dma_sel);

void SPI_DMA_Nand_WriteEnable(spi_dma_sel_t spi_dma_sel);
void SPI_DMA_Nand_WriteDisable(spi_dma_sel_t spi_dma_sel);
uint8_t SPI_DMA_Nand_GetFeature(spi_dma_sel_t spi_dma_sel, uint8_t RegAddr);
void SPI_DMA_Nand_SetFeature(spi_dma_sel_t spi_dma_sel, uint8_t RegAddr, uint8_t Data);

void SPI_DMA_Nand_BlockErase(spi_dma_sel_t spi_dma_sel, uint32_t BlockAddr, uint32_t PageAddr);
uint32_t SPI_DMA_Nand_ReadID(spi_dma_sel_t spi_dma_sel);

uint32_t SPINand_Read_Page(spi_dma_sel_t spi_dma_sel,uint32_t BlockAddr, uint32_t PageAddr, uint32_t ByteAddr, uint8_t read_io,uint8_t *Data, uint32_t Len);
uint32_t SPINand_Program_Page(spi_dma_sel_t spi_dma_sel,uint32_t BlockAddr, uint32_t PageAddr, uint32_t ByteAddr, uint8_t write_io,uint8_t *Data, uint32_t Len);


/*Control SPI Nand GD Func*/

/*ECC Function*/
void SPI_DMA_Nand_ECC_Enable(spi_dma_sel_t spi_dma_sel, uint8_t NandType);
void SPI_DMA_Nand_ECC_Disable(spi_dma_sel_t spi_dma_sel);

void spinand_irq(uint16_t *DataPtr);
uint32_t SPI_NAND_GetVersion(void);
#ifdef __cplusplus
}
#endif

#endif
