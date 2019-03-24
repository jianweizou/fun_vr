/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_i2s.h
 *
 * @author Kaishiang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_i2s_H_wyGTxhNr_lUoN_HN2s_s9ax_uCc6IY9z2xmA__
#define __snc_i2s_H_wyGTxhNr_lUoN_HN2s_s9ax_uCc6IY9z2xmA__

#ifdef __cplusplus
extern "C" {
#endif
#include "snc_types.h"

//=============================================================================
//                  Constant Definition
//=============================================================================    
  
  
typedef enum i2s_sel
{
    I2S_0       = 0,
    I2S_1,   
    I2S_2,
    I2S_3,
} i2s_sel_t;


typedef enum i2s_mode
{
    I2S_MASTER       = 0,
    I2S_SLAVE,   
} i2s_mode_t;

typedef enum i2s_trans_mode
{
    I2S_RX_MODE       = 0,
    I2S_TX_MODE,   
} i2s_trans_mode_t;

typedef enum i2s_channel
{
    I2S_STEREO    =0,
    I2S_MONO,       
} i2s_channel_t;



typedef enum i2s_data_length
{
    I2S_DATA_LENGTH_8BIT       = 0,
    I2S_DATA_LENGTH_16BIT, 
    I2S_DATA_LENGTH_24BIT,
    I2S_DATA_LENGTH_32BIT,   
    
} i2s_data_length_t;

typedef enum i2s_data_format
{
    I2S_STANDARD_FORMAT       = 0,
    I2S_LEFT_FORMAT, 
    I2S_RIGHT_FORMAT,   
    
} i2s_data_format_t;



typedef enum i2s_clocksource
{
    IMCLK_I2S_4       = 0,
    IMCLK_IHRC,
    IMCLK_XTAL,
    IMCLK_FPLL,   
} i2s_clocksource_t;

typedef enum i2s_pll_div
{
    I2S_IMCLK_FROM_PLL_DIV2       = 0,
    I2S_IMCLK_FROM_PLL_DIV9,
    I2S_IMCLK_FROM_PLL_DIV10,
    I2S_IMCLK_FROM_PLL_DIV14,   
} i2s_pll_div_t;


typedef enum i2s_sample_rate
{
    SAMPLERATE_8K       = 0, 
    SAMPLERATE_11_025K,
    SAMPLERATE_12K,
    SAMPLERATE_16K,
    SAMPLERATE_22_05K,
    SAMPLERATE_24K,
    SAMPLERATE_32K,
    SAMPLERATE_44_1K,
    SAMPLERATE_48K,
    SAMPLERATE_96K,
    
} i2s_sample_rate_t;


//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================


 typedef struct
 {
	i2s_sel_t                   sel;
	i2s_mode_t                  mode;    
	i2s_data_length_t           data_length;
	i2s_channel_t               channel;
	i2s_data_format_t           format;
	i2s_clocksource_t           clock_source;
	i2s_sample_rate_t           samplerate; 
	i2s_trans_mode_t            trans_mode;
	uint32_t 					*TXBufAddr1;			/*buffer addr*/		
	uint32_t 					TXBufAddr2Offset;		/*buffer2 offset*/
	uint32_t 					TXBufLen;		        /*Buffer Length*/ 

	uint32_t 					*RXBufAddr1;			/*buffer addr*/		
	uint32_t 					RXBufAddr2Offset;		/*buffer2 offset*/
	uint32_t 					RXBufLen;		        /*Buffer Length*/  
	bool                        i2s_enable;
	bool                        pauce_status;
	bool                        start_status;
	bool                        mute_status;
 }i2s_info_t; 
 
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================


	/*I2S Channel Configuration*/


/*I2S init/clock*/
int I2S_Init(i2s_info_t *i2s_info);

/*I2S command*/
void I2S_Enable(i2s_sel_t i2s_sel);
void I2S_Disable(i2s_sel_t i2s_sel);
void I2S_Pause(i2s_sel_t i2s_sel);
void I2S_resume(i2s_sel_t i2s_sel);
void I2S_Start(i2s_sel_t i2s_sel);
void I2S_Stop(i2s_sel_t i2s_sel);
void I2S_Mute(i2s_sel_t i2s_sel);
void I2S_unMute(i2s_sel_t i2s_sel);
void I2S_MODE(i2s_sel_t i2s_sel,uint8_t config);
void I2S_Tx_Enable(i2s_sel_t i2s_sel);
void I2S_Rx_Enable(i2s_sel_t i2s_sel);
void I2S_Set_DMA_Len(i2s_sel_t i2s_sel, uint16_t Length);
void I2S_Set_TxBuf1_Addr(i2s_sel_t i2s_sel, uint32_t *Ptr);
void I2S_Set_TxBuf2_Offset(i2s_sel_t i2s_sel, uint16_t Offset);
void I2S_Set_RxBuf1_Addr(i2s_sel_t i2s_sel, uint32_t *Ptr);
void I2S_Set_RxBuf2_Offset(i2s_sel_t i2s_sel, uint16_t Offset);
void I2S_Set_MONO_STEREO(i2s_sel_t i2s_sel, uint8_t Format);
void I2S_Set_Format(i2s_sel_t i2s_sel, uint8_t Format);
void I2S_Disable(i2s_sel_t i2s_ch);
void I2S_Set_Ch_Len(i2s_sel_t i2s_sel, uint8_t Ch_Len);
void I2S_Set_Data_Len(i2s_sel_t i2s_sel, uint8_t Data_Len);

uint32_t I2S_Get_TxBuf1_Addr(i2s_sel_t i2s_ch);
uint32_t I2S_Get_RxBuf1_Addr(i2s_sel_t i2s_ch);

uint32_t I2S_Get_RxBuf_Idx(i2s_sel_t i2s_sel);
uint32_t I2S_Get_TxBuf_Idx(i2s_sel_t i2s_sel);
/*I2S DMA Buffer Index & Toggle*/


void I2S_Set_Tx_Toggle_IRQ(i2s_sel_t i2s_ch, bool config);
void I2S_Set_Rx_Toggle_IRQ(i2s_sel_t i2s_ch, bool config);

uint32_t I2S_Get_Tx_Toggle_IRQ(i2s_sel_t i2s_ch);
uint32_t I2S_Get_Rx_Toggle_IRQ(i2s_sel_t i2s_ch);

uint32_t I2S_GetVersion(void);
#ifdef __cplusplus
}
#endif

#endif
