/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_i2s4.h
 *
 * @author Kaishiang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */


#ifndef __snc_i2s4_H_wLW2Q5RY_l4pK_Hioz_sv3h_uoG353hjL9Bt__
#define __snc_i2s4_wLW2Q5RY_l4pK_Hioz_sv3h_uoG353hjL9Bt__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"

/*_____ D E F I N I T I O N S ______________________________________________*/
/*IP return define*/

typedef enum i2s4_return
{
    I2S4_PASS       = 0,
    I2S4_FAIL,    
} i2s4_return_t;
/*IP Master Slave Mode define*/

typedef enum i2s4_mode
{
	I2S4_MASTER_TX       = 0,
    I2S4_MASTER_RX,
	I2S4_SLAVE_TX,
	I2S4_SLAVE_RX,
} i2s4_mode_t;


/*IP Channel Length (I2S_Set_Ch_Len)*/
typedef enum i2s4_ch_length
{
	I2S4_CH_8BIT=7,
	I2S4_CH_9BIT,
	I2S4_CH_10BIT,
	I2S4_CH_11BIT,
	I2S4_CH_12BIT,
	I2S4_CH_13BIT,
	I2S4_CH_14BIT,
	I2S4_CH_15BIT,
	I2S4_CH_16BIT,
	I2S4_CH_17BIT,
	I2S4_CH_18BIT,
	I2S4_CH_19BIT,
	I2S4_CH_20BIT,
	I2S4_CH_21BIT,
	I2S4_CH_22BIT,
	I2S4_CH_23BIT,
	I2S4_CH_24BIT,
	I2S4_CH_25BIT,
	I2S4_CH_26BIT,
	I2S4_CH_27BIT,
	I2S4_CH_28BIT,
	I2S4_CH_29BIT,
	I2S4_CH_30BIT,
	I2S4_CH_31BIT,
	I2S4_CH_32BIT,
} i2s4_ch_length_t;
/*IP Data Length (I2S_Set_Data_Len)*/

typedef enum i2s4_data_length
{
    I2S4_DATA_8BIT       = 0,
    I2S4_DATA_16BIT,
	I2S4_DATA_24BIT,
	I2S4_DATA_32BIT,
} i2s4_data_length_t;

/*IP operation format (I2S_Set_Format)*/

typedef enum i2s4_format
{
    I2S4_STANDARD_FORMAT       = 0,
    I2S4_LEFT_FORMAT,
	I2S4_RIGHT_FORMAT,
} i2s4_format_t;

/*IP MONO STEREO selection (I2S_Set_MONO_STEREO)*/

typedef enum i2s4_mono_stereo
{
	I2S4_STEREO       = 0,
    I2S4_MONO,
	
} i2s4_mono_stereo_t;
/*IP Clock Source (I2S_SEL_IMCLK)*/

typedef enum i2s4_sel_imclk
{
    I2S4_IMCLK_FROM_I2S4       = 0,
    I2S4_IMCLK_FROM_IHRC,
	I2S4_IMCLK_FROM_XTAL,
	I2S4_IMCLK_FROM_SYSCLK_DIV2,
	I2S4_IMCLK_FROM_SYSCLK_DIV9,
	I2S4_IMCLK_FROM_SYSCLK_DIV10,
	I2S4_IMCLK_FROM_SYSCLK_DIV14,
} i2s4_sel_imclk_t;
/*IP Tx Rx INT (I2S4_Set_INT_Enable)*/

typedef enum i2s4_int
{
    I2S4_RX_FIFOTHD       = 0,  /*RX FIFO Threshold issue interrupt*/
    I2S4_RX_FIFOUD,             /*RX FIFO Underflow issue interrupt*/
	I2S4_TX_FIFOTHD,						/*TX FIFO Threshold issue interrupt*/
	I2S4_TX_FIFOOV,							/*TX FIFO Overflow issue interrupt*/
	
} i2s4_int_t;

/*IP MCLK Output Pad0 Pad1 Option*/

typedef enum i2s4_mclk_output
{
    MCLK_PAD_OUTPUT_IHRC_CLK       = 0, 
    MCLK_PAD_OUTPUT_I2S4_CLK,             
	MCLK_PAD_OUTPUT_XTAL_CLK,						
} i2s4_mclk_output_t;
/*IP Status(uint32_t I2S4_Read_Status(void))*/

typedef enum i2s4_status
{
    I2S4_INT_OCCURS       = 0x1,
    I2S4_CUR_CH           = 0x2,
	I2S4_TXFIFO_THF       = 0x40,
	I2S4_RXFIFO_THF       = 0x80,
	I2S4_TXFIFO_FULL      = 0x100,
	I2S4_RXFIFO_FULL      = 0x200,
	I2S4_TXFIFO_EMPTY     = 0x400,
	I2S4_RXFIFO_EMPTY     = 0x800,
} i2s4_status_t;

/*IP Interrupt Status(uint32_t I2S4_Read_Int_Status(void))*/

typedef enum i2s4_int_status
{
    I2S4_RXFIFOTHIF       = 0x80,     /*Rx FIFO Threshold Triggered*/
    I2S4_TXFIFOTHIF       = 0x40,     /*Tx FIFO Threshold Triggered*/
	I2S4_RXFIFOTHUD       = 0x20,     /*Rx FIFO Underflow*/
	I2S4_TXFIFOTHOV       = 0x10,     /*Tx FIFO overflow*/
} i2s4_int_status_t;
/*8Bit Data LENGTH FIFO Struct*/
union I2S4_8BIT_FIFO_STC{
	uint32_t Data;
	struct{
		uint32_t	L0	:8;
		uint32_t	R0	:8;
		uint32_t	L1	:8;
		uint32_t	R1	:8;
	}BIT;
};

/*16Bit Data LENGTH FIFO Struct*/
union I2S4_16BIT_FIFO_STC{
	uint32_t Data;
	struct{
		uint32_t	L0	:16;
		uint32_t	R0	:16;
	}BIT;
};

/*24/32Bit Data LENGTH FIFO Struct*/
struct I2S4_32BIT_FIFO_STC{
	uint32_t L_Data;
	uint32_t R_Data;
};

/*_____ M A C R O S ________________________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/

	/*I2S4 Command*/
void I2S4_Enable(uint8_t I2S_Mode);
void I2S4_Disable(void);
void I2S4_Start(void);
void I2S4_Stop(void);
void I2S4_Mute(void);
void I2S4_UnMute(void);

	/*I2S4 Channel Configuration*/
void I2S4_Set_Ch_Len(uint8_t Ch_Len);
void I2S4_Set_Data_Len(uint8_t Data_Len);
void I2S4_Set_Format(uint8_t Format);
void I2S4_Set_MONO_STEREO(uint8_t Format);

	/*I2S4 Clock Configuration*/
void I2S4_SEL_IMCLK(uint8_t Config);
void I2S4_SET_BCLKDIV(uint8_t Config);
void I2S4_SET_IMCLKDIV(uint8_t Config);

	/*I2S4 FIFO & Interrupt Clear*/
extern void I2S4_Clr_Int_Flag(void);
extern void I2S4_Clr_Tx_FIFO(void);
extern void I2S4_Clr_Rx_FIFO(void);

	/*I2S4 FIFO Configuration*/
void I2S4_Set_INT_Enable(uint8_t config);
void I2S4_Set_INT_Disable(void);
void I2S4_Set_Rx_THD(uint8_t Value);
void I2S4_Set_Tx_THD(uint8_t Value);

	/*I2S4 IP & FIFO & INT Stauts */
uint32_t I2S4_Read_IP_Status(void);
uint32_t I2S4_Read_Int_Status(void);
uint8_t I2S4_Read_Tx_FIFO_Status(void);
uint8_t I2S4_Read_Rx_FIFO_Status(void);

	/*I2S4 FIFO Read Write*/
uint32_t I2S4_Read_Rx_FIFO(void);
void I2S4_Write_Tx_FIFO(uint32_t Value);

	/*I2S4 Tx Rx Process*/
uint8_t I2S4_8Bit_STD_Handler(uint8_t Mode, uint8_t *L_Data, uint8_t *R_Data, uint8_t u8_Len);
uint8_t I2S4_16Bit_STD_Handler(uint8_t Mode, uint16_t *L_Data, uint16_t *R_Data, uint8_t u16_Len);
uint8_t I2S4_32Bit_STD_Handler(uint8_t Mode, uint32_t *L_Data, uint32_t *R_Data, uint8_t u32_Len);
uint8_t I2S4_MONO_Handler(uint8_t Mode, uint32_t *Data, uint8_t u32_Len);

	/*Individual Pad MCLK PAD0 PAD1 Control*/
void I2S4_MCLK_OUT0_SEL(uint8_t Value);
void I2S4_MCLK_OUT1_SEL(uint8_t Value);
void I2S4_MCLK_OUT_ENABLE(uint8_t Value);
void I2S4_MCLK_OUT0_OUT1_SEL(void);
void I2S4_Init(void);

#endif	/* __SNC7320_I2S_inc_H */









