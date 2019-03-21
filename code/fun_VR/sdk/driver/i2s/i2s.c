/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file i2s.c
 *
 * @author Kaishiang
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "snc_i2s.h"
#include "register_7320.h"

#define I2S_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================



//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================


/******************** (C) COPYRIGHT 2016 SONiX *******************************
* COMPANY:		  SONiX
* DATE:			    2016/04
* IC:			      SNC7312
* DESCRIPTION:	Audio functions.
*____________________________________________________________________________
* REVISION	     Date		       User		  Description
* 1.0		         2016/04/06	   SA2			First release
*
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/


/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
 
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/


//=============================================================================
//                  Private Function Definition
//=============================================================================
static SN_I2S_Type*
_i2s_ch_get_handle(
    i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *IP_SEL = 0;
    switch(i2s_ch)
    {
        case I2S_0:   IP_SEL = (SN_I2S_Type*)SN_I2S0_BASE; break;
        case I2S_1:   IP_SEL = (SN_I2S_Type*)SN_I2S1_BASE; break;
        case I2S_2:   IP_SEL = (SN_I2S_Type*)SN_I2S2_BASE; break;
        case I2S_3:   IP_SEL = (SN_I2S_Type*)SN_I2S3_BASE; break;
        

        default:    break;
    }

    return IP_SEL;
}
    
void I2S_Enable(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
    pDev->CTRL_b.I2SEN = 1;  
} 
 
void I2S_MODE(i2s_sel_t i2s_ch,uint8_t config)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
    pDev->CTRL_b.MS = config;   
}
 
void I2S_Tx_Enable(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
    pDev->CTRL_b.I2S_DIRECTION = 1;	/*SDIO Pin config as Output*/
    pDev->CTRL_b.TXEN = 1;	/*Transmit Enable*/
    pDev->CTRL_b.RXEN = 0;			   
}

void I2S_Rx_Enable(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
    pDev->CTRL_b.I2S_DIRECTION = 0;
    pDev->CTRL_b.RXEN = 1;
    pDev->CTRL_b.TXEN = 0;
}

void I2S_Disable(i2s_sel_t i2s_ch)
{
	SN_I2S_Type*	pDev = _i2s_ch_get_handle(i2s_ch);

	//Axl add 180829 {
	pDev->CTRL_b.RXEN = 0;
	pDev->CTRL_b.TXEN = 0;
	//}
	pDev->CTRL_b.I2SEN = 0;
}

void I2S_Set_Ch_Len(i2s_sel_t i2s_ch, uint8_t Ch_Len)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->CTRL_b.CHLENGTH = Ch_Len;
}

void I2S_Set_Data_Len(i2s_sel_t i2s_ch, uint8_t Data_Len)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->CTRL_b.DL = Data_Len;
}

void I2S_Set_Format(i2s_sel_t i2s_ch, uint8_t Format)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->CTRL_b.FORMAT = Format;
}

void I2S_Set_MONO_STEREO(i2s_sel_t i2s_ch, uint8_t Format)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->CTRL_b.MONO = Format;
}

void I2S_SEL_IMCLK(SN_I2S_Type *IP_SEL, uint8_t Config)
{
		
//		if(Config==IMCLK_I2S_4 || Config==IMCLK_IHRC || Config==IMCLK_XTAL)
//		{
			IP_SEL->CLK_b.IMCLKSEL = Config;
			if(Config==IMCLK_I2S_4){
			//SN_I2S4->CTRL_b.I2SEN = 1;
			}
		//}
//		else
//		{
//			IP_SEL->CLK_b.IMCLKSEL = 3;
//		if(Config==I2S_IMCLK_FROM_PLL_DIV9){
//			IP_SEL->CLK_b.FPLL_DIV = 0;
//		}
//		else if(Config==I2S_IMCLK_FROM_PLL_DIV10){
//			IP_SEL->CLK_b.FPLL_DIV = 1;
//		}
//		else if(Config==I2S_IMCLK_FROM_PLL_DIV14){
//			IP_SEL->CLK_b.FPLL_DIV = 2;
//		}
//		else /*if(Config==I2S_IMCLK_FROM_SYSCLK_DIV2)*/{
//			IP_SEL->CLK_b.FPLL_DIV = 3;
//		}
//	}
}


void I2S_SET_BCLKDIV(SN_I2S_Type *IP_SEL, uint8_t Config)
{
	/*Config = BCLKDIV[7:0], BCLK = MCLK/(2*n+2)*/		
	IP_SEL->CLK_b.BCLKDIV = Config;
}

static void _I2S_SET_IMCLKDIV(SN_I2S_Type *IP_SEL, uint8_t Config)
{ 
	/*Config = IMCLKDIV[2:0], IMCLK = IMCLKSEL/(2*n),n>0*/
	IP_SEL->CLK_b.IMCLKDIV = Config;
}

static void	_IHRC_case(SN_I2S_Type *IP_SEL,i2s_sample_rate_t SR)
{
   switch (SR){
        case SAMPLERATE_8K:
            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_11_025K:
            
        break;
        case SAMPLERATE_12K:
//            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
//            _I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_16K:
            _I2S_SET_IMCLKDIV(IP_SEL,0);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,1);//20M/20=1M
        break;
        case SAMPLERATE_22_05K:
            
        break;
        case SAMPLERATE_24K:
//            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
//            _I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_32K:
            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,4);//20M/10=1M
        break;
        case SAMPLERATE_44_1K:
            
        break;
        case SAMPLERATE_48K:
            _I2S_SET_IMCLKDIV(IP_SEL,0);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,2);//20M/20=1M
        break;
        case SAMPLERATE_96K:
            
        break;
   };       
}    
static void	_XTAL_case(SN_I2S_Type *IP_SEL,i2s_sample_rate_t SR)
{
   switch (SR){
        case SAMPLERATE_8K:
            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_11_025K:
            
        break;
        case SAMPLERATE_12K:
//            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
//            _I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_16K:
            _I2S_SET_IMCLKDIV(IP_SEL,2);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,2);//20M/20=1M
        break;
        case SAMPLERATE_22_05K:
            
        break;
        case SAMPLERATE_24K:
//            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
//            _I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_32K:
            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,4);//20M/10=1M
        break;
        case SAMPLERATE_44_1K:
            
        break;
        case SAMPLERATE_48K:
            _I2S_SET_IMCLKDIV(IP_SEL,0);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,3);//20M/20=1M
        break;
        case SAMPLERATE_96K:
            
        break;
   };       
}
static void  _IMCLK_I2S_4_case(SN_I2S_Type *IP_SEL,i2s_sample_rate_t SR)
{
    // to do
    switch (SR){
        case SAMPLERATE_8K:
           
        break;
        case SAMPLERATE_11_025K:
            
        break;
        case SAMPLERATE_12K:
//        
        break;
        case SAMPLERATE_16K:
      
        break;
        case SAMPLERATE_22_05K:
            
        break;
        case SAMPLERATE_24K:

        break;
        case SAMPLERATE_32K:

        break;
        case SAMPLERATE_44_1K:
            
        break;
        case SAMPLERATE_48K:

        break;
        case SAMPLERATE_96K:
            
        break;
   };       
}  

static void	_FPLL_case(SN_I2S_Type *IP_SEL,i2s_sample_rate_t SR)
{
   switch (SR){
        case SAMPLERATE_8K:
            _I2S_SET_IMCLKDIV(IP_SEL,5);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,9);//20M/20=1M
        break;
        case SAMPLERATE_11_025K:
            
        break;
        case SAMPLERATE_12K:
//            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
//            _I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_16K:
            _I2S_SET_IMCLKDIV(IP_SEL,4);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,4);//20M/20=1M
        break;
        case SAMPLERATE_22_05K:
            
        break;
        case SAMPLERATE_24K:
//            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
//            _I2S_SET_BCLKDIV(IP_SEL,18);//20M/20=1M
        break;
        case SAMPLERATE_32K:
            _I2S_SET_IMCLKDIV(IP_SEL,1);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,4);//20M/10=1M
        break;
        case SAMPLERATE_44_1K:
            
        break;
        case SAMPLERATE_48K:
            _I2S_SET_IMCLKDIV(IP_SEL,0);//40M/2=20
            I2S_SET_BCLKDIV(IP_SEL,2);//20M/20=1M
        break;
        case SAMPLERATE_96K:
            
        break;
   };       
}    
void I2S_Set_BCLK(i2s_sel_t i2s_ch,i2s_clocksource_t clock_source,i2s_sample_rate_t samplerate)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
    I2S_SEL_IMCLK(pDev,clock_source);
    if(clock_source==IMCLK_I2S_4) //12.288
    {
		_IMCLK_I2S_4_case(pDev,samplerate);    
    }
    else if (clock_source==IMCLK_IHRC) // FPGA 40MHZ temporariry 
    {
		_IHRC_case(pDev,samplerate);     
    }   
    else if(clock_source==IMCLK_XTAL)    
    {
		_XTAL_case(pDev,samplerate);
    }
    else if(clock_source==IMCLK_FPLL)
    {
        _FPLL_case(pDev,samplerate);
    }           
}    

uint32_t I2S_Get_Rx_Toggle_IRQ(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	return pDev->STATUS_b.RX_TOG;
}


uint32_t I2S_Get_Tx_Toggle_IRQ(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	return pDev->STATUS_b.TX_TOG;
}


void I2S_Set_Rx_Toggle_IRQ(i2s_sel_t i2s_ch, bool status)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
    
	if(status==true){
		pDev->IE_b.RXINTEN = 1;	
		pDev->IE_b.TXINTEN = 0;
		if(pDev == I2S_0){
			NVIC_ClearPendingIRQ(I2S0_DMA_IRQn);
			NVIC_EnableIRQ(I2S0_DMA_IRQn);
		}
		else if(i2s_ch == I2S_1){
			NVIC_ClearPendingIRQ(I2S1_DMA_IRQn);
			NVIC_EnableIRQ(I2S1_DMA_IRQn);
		}
		else if(i2s_ch == I2S_2){
			NVIC_ClearPendingIRQ(I2S2_DMA_IRQn);
			NVIC_EnableIRQ(I2S2_DMA_IRQn);
		}
		else if(i2s_ch == I2S_3){
			NVIC_ClearPendingIRQ(I2S3_DMA_IRQn);
			NVIC_EnableIRQ(I2S3_DMA_IRQn);
		}
	}
	else{
		pDev->IE_b.RXINTEN = 0;	
		pDev->IE_b.TXINTEN = 0;
		if(pDev == I2S_0){
			NVIC_ClearPendingIRQ(I2S0_DMA_IRQn);
			NVIC_DisableIRQ(I2S0_DMA_IRQn);
		}
		else if(i2s_ch == I2S_1){
			NVIC_ClearPendingIRQ(I2S1_DMA_IRQn);
			NVIC_DisableIRQ(I2S1_DMA_IRQn);
		}
		else if(i2s_ch == I2S_2){
			NVIC_ClearPendingIRQ(I2S2_DMA_IRQn);
			NVIC_DisableIRQ(I2S2_DMA_IRQn);
		}
		else if(i2s_ch == I2S_3){
			NVIC_ClearPendingIRQ(I2S3_DMA_IRQn);
			NVIC_DisableIRQ(I2S3_DMA_IRQn);
		}
	}
}


void I2S_Set_Tx_Toggle_IRQ(i2s_sel_t i2s_ch, bool status)
{  
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	if(status==true){
		pDev->IE_b.RXINTEN = 0;	
		pDev->IE_b.TXINTEN = 1;
		if(i2s_ch == I2S_0){
			NVIC_ClearPendingIRQ(I2S0_DMA_IRQn);
			NVIC_EnableIRQ(I2S0_DMA_IRQn);
		}
		else if(i2s_ch == I2S_1){
			NVIC_ClearPendingIRQ(I2S1_DMA_IRQn);
			NVIC_EnableIRQ(I2S1_DMA_IRQn);
		}
		else if(i2s_ch == I2S_2){
			NVIC_ClearPendingIRQ(I2S2_DMA_IRQn);
			NVIC_EnableIRQ(I2S2_DMA_IRQn);
		}
		else if(i2s_ch == I2S_3){
			NVIC_ClearPendingIRQ(I2S3_DMA_IRQn);
			NVIC_EnableIRQ(I2S3_DMA_IRQn);
		}
	}
	else{
		pDev->IE_b.RXINTEN = 0;	
		pDev->IE_b.TXINTEN = 0;
		if(i2s_ch == I2S_0){
			NVIC_ClearPendingIRQ(I2S0_DMA_IRQn);
			NVIC_DisableIRQ(I2S0_DMA_IRQn);
		}
		else if(i2s_ch == I2S_1){
			NVIC_ClearPendingIRQ(I2S1_DMA_IRQn);
			NVIC_DisableIRQ(I2S1_DMA_IRQn);
		}
		else if(i2s_ch == I2S_2){
			NVIC_ClearPendingIRQ(I2S2_DMA_IRQn);
			NVIC_DisableIRQ(I2S2_DMA_IRQn);
		}
		else if(i2s_ch == I2S_3){
			NVIC_ClearPendingIRQ(I2S3_DMA_IRQn);
			NVIC_DisableIRQ(I2S3_DMA_IRQn);
		}
	}
}

void I2S_Set_DMA_Len(i2s_sel_t i2s_ch, uint16_t Length)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->RAM1_Len_b.RAM1_Len = Length;
}

void I2S_Set_TxBuf1_Addr(i2s_sel_t i2s_ch, uint32_t *Ptr)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->TxRAM1_Address = (uint32_t)Ptr;
}

uint32_t I2S_Get_TxBuf1_Addr(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
	pDev = _i2s_ch_get_handle(i2s_ch);
	return pDev->TxRAM1_Address;
}

void I2S_Set_TxBuf2_Offset(i2s_sel_t i2s_ch, uint16_t Offset)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->TxRAM2_Offset = Offset;
}

uint32_t I2S_Get_TxBuf_Idx(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	return pDev->TxIndex;
}

void I2S_Set_RxBuf1_Addr(i2s_sel_t i2s_ch, uint32_t *Ptr)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->RxRAM1_Address = (uint32_t)Ptr;
}

uint32_t I2S_Get_RxBuf1_Addr(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
	pDev = _i2s_ch_get_handle(i2s_ch);
	return pDev->RxRAM1_Address;
}

void I2S_Set_RxBuf2_Offset(i2s_sel_t i2s_ch, uint16_t Offset)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->RxRAM2_Offset = Offset;
}

uint32_t I2S_Get_RxBuf_Idx(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	return pDev->RxIndex;
}
   
void I2S_Pause(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
   
	pDev->CTRL_b.PAUSE = 1;
}

void I2S_Resume(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
   
	pDev->CTRL_b.PAUSE = 0;
}

void I2S_Start(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
  
	pDev->CTRL_b.START = 1;
}

void I2S_Mute(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
   
	pDev->CTRL_b.MUTE = 1;
}

void I2S_UnMute(i2s_sel_t i2s_ch)
{
	SN_I2S_Type   *pDev = 0;
	pDev = _i2s_ch_get_handle(i2s_ch);

	pDev->CTRL_b.MUTE = 0;
}

void I2S_Stop(i2s_sel_t i2s_ch)
{
    SN_I2S_Type   *pDev = 0;
    pDev = _i2s_ch_get_handle(i2s_ch);
	pDev->CTRL_b.START = 0;
}

int I2S_Init(i2s_info_t *i2s_info)
{
    
    SN_I2S_Type   *pDev = 0;
    int rval=0;
    pDev = _i2s_ch_get_handle(i2s_info->sel);
    if( !pDev )
	{   
		// sn_log(LOG_ERR, "err: wrong idma channel (%d) !!\n", channel);
		rval = -1;
	}
    /*Format Setup*/
     
    I2S_Set_Ch_Len(i2s_info->sel,31); // channel length default set 32bit
    
    
    I2S_Set_Data_Len(i2s_info->sel,I2S_DATA_LENGTH_32BIT);
    I2S_Set_Format(i2s_info->sel,i2s_info->format);
    I2S_Set_MONO_STEREO(i2s_info->sel,i2s_info->channel);
    
    //clock setup 
    I2S_Set_BCLK(i2s_info->sel,i2s_info->clock_source,i2s_info->samplerate);

    //buffer setup
    if(i2s_info->trans_mode==I2S_TX_MODE){
		I2S_Set_TxBuf1_Addr(i2s_info->sel,i2s_info->TXBufAddr1);
		I2S_Set_TxBuf2_Offset(i2s_info->sel,i2s_info->TXBufAddr2Offset);
		I2S_Set_DMA_Len(i2s_info->sel,i2s_info->TXBufLen); 
		I2S_Tx_Enable(i2s_info->sel);
	}	
	else if(i2s_info->trans_mode==I2S_RX_MODE){
		I2S_Set_RxBuf1_Addr(i2s_info->sel,i2s_info->RXBufAddr1);
		I2S_Set_RxBuf2_Offset(i2s_info->sel,i2s_info->RXBufAddr2Offset);
		I2S_Set_DMA_Len(i2s_info->sel,i2s_info->RXBufLen); 
		I2S_Rx_Enable(i2s_info->sel);
	}	
    
    //command I2S
    I2S_MODE(i2s_info->sel,i2s_info->mode);
    
    I2S_Enable(i2s_info->sel);
         
	I2S_UnMute(i2s_info->sel);
	I2S_Start(i2s_info->sel);	
    
    return rval;
}   

uint32_t I2S_GetVersion(void)
{
    return I2S_VERSION;
} 
