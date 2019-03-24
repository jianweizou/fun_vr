/******************** (C) COPYRIGHT 2016 SONiX *******************************
* COMPANY:		  SONiX
* DATE:			    2018/04
* IC:			      SNC7320
* DESCRIPTION:	Audio functions.
*____________________________________________________________________________
* REVISION	     Date		       User		  Description
* 1.0		         2018/04/06	   SA2			First release
*
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "register_7320.h"
#include "snc_i2s4.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
 
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Enable(uint8_t I2S_Mode)
{
	switch (I2S_Mode){
		case I2S4_MASTER_TX:
			I2S4_Clr_Tx_FIFO();
			SN_I2S4->CTRL_b.TXEN = 1;	/*Transmit Enable*/
			SN_I2S4->CTRL_b.RXEN = 0;
			SN_I2S4->CTRL_b.MS = 0;	/*Act as Master,Send BCLK and WS signal*/
			break;
		case I2S4_MASTER_RX:
			I2S4_Clr_Rx_FIFO();
			SN_I2S4->CTRL_b.TXEN = 0;	
			SN_I2S4->CTRL_b.RXEN = 1;	/*Receiver Enable*/
			SN_I2S4->CTRL_b.MS = 0;	/*Act as Master,Send BCLK and WS signal*/
			break;
		case I2S4_SLAVE_TX:
			I2S4_Clr_Tx_FIFO();
			SN_I2S4->CTRL_b.TXEN = 1;	/*Transmit Enable*/
			SN_I2S4->CTRL_b.RXEN = 0;
			SN_I2S4->CTRL_b.MS = 1;	/*Act as Slave using externally Bclk and WS signals*/
			break;
		case I2S4_SLAVE_RX:
			I2S4_Clr_Rx_FIFO();
			SN_I2S4->CTRL_b.TXEN = 0;	
			SN_I2S4->CTRL_b.RXEN = 1;	/*Receiver Enable*/
			SN_I2S4->CTRL_b.MS = 1;	/*Act as Slave using externally Bclk and WS signals*/
			break;
	};
	SN_I2S4->CTRL_b.I2SEN = 1;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Disable(void)
{
		SN_I2S4->CTRL_b.I2SEN = 0;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_Ch_Len(uint8_t Ch_Len)
{
		SN_I2S4->CTRL_b.CHLENGTH = Ch_Len;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_Data_Len(uint8_t Data_Len)
{
		SN_I2S4->CTRL_b.DL = Data_Len;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_Format(uint8_t Format)
{
		SN_I2S4->CTRL_b.FORMAT = Format;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_MONO_STEREO(uint8_t Format)
{
		SN_I2S4->CTRL_b.MONO = Format;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Start(void)
{
		SN_I2S4->CTRL_b.START = 1;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Mute(void)
{
		SN_I2S4->CTRL_b.MUTE = 1;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_UnMute(void)
{
		SN_I2S4->CTRL_b.MUTE = 0;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Stop(void)
{
		SN_I2S4->CTRL_b.START = 0;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_SEL_IMCLK(uint8_t Config)
{
	if(Config==I2S4_IMCLK_FROM_I2S4 || Config==I2S4_IMCLK_FROM_IHRC || Config==I2S4_IMCLK_FROM_XTAL){
		SN_I2S4->CLK_b.IMCLKSEL = Config;
	}
	else{
		SN_I2S4->CLK_b.IMCLKSEL = 3;
		if(Config==I2S4_IMCLK_FROM_SYSCLK_DIV9){
			SN_I2S4->CLK_b.FPLL_DIV = 0;
		}
		else if(Config==I2S4_IMCLK_FROM_SYSCLK_DIV10){
			SN_I2S4->CLK_b.FPLL_DIV = 1;
		}
		else if(Config==I2S4_IMCLK_FROM_SYSCLK_DIV14){
			SN_I2S4->CLK_b.FPLL_DIV = 2;
		}
		else /*if(Config==I2S_IMCLK_FROM_SYSCLK_DIV2)*/{
			SN_I2S4->CLK_b.FPLL_DIV = 3;
		}
	}
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_SET_BCLKDIV(uint8_t Config)
{
	/*Config = BCLKDIV[7:0], BCLK = MCLK/(2*n+2)*/
		SN_I2S4->CLK_b.BCLKDIV = Config;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_SET_IMCLKDIV(uint8_t Config)
{
	/*Config = IMCLKDIV[2:0], IMCLK = IMCLKSEL/(2*n),n>0*/
		SN_I2S4->CLK_b.BCLKDIV = Config;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Clr_Tx_FIFO(void)
{
		SN_I2S4->CTRL_b.CLRTXFIFO = 1;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Clr_Rx_FIFO(void)
{
		SN_I2S4->CTRL_b.CLRRXFIFO = 1;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Clr_Int_Flag(void)
{
		uint8_t Int_Status;
		Int_Status = SN_I2S4->RIS;
		SN_I2S4->IC = Int_Status;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_Rx_THD(uint8_t Value)
{
		Value &= 0x07;
		SN_I2S4->CTRL_b.RXFIFOTH = Value;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_Tx_THD(uint8_t Value)
{
		Value &= 0x07;
		SN_I2S4->CTRL_b.TXFIFOTH = Value;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_INT_Enable(uint8_t config)
{
	switch (config){
		case I2S4_RX_FIFOTHD:
			SN_I2S4->IE_b.RXFIFOTHIEN = 1;
			SN_I2S4->IE_b.RXFIFOUDFIEN = 0;
			SN_I2S4->IE_b.TXFIFOTHIEN = 0;
			SN_I2S4->IE_b.TXFIFOOVFIEN = 0;
			break;
		case I2S4_RX_FIFOUD:
			SN_I2S4->IE_b.RXFIFOTHIEN = 0;
			SN_I2S4->IE_b.RXFIFOUDFIEN = 1;
			SN_I2S4->IE_b.TXFIFOTHIEN = 0;
			SN_I2S4->IE_b.TXFIFOOVFIEN = 0;
			break;
		case I2S4_TX_FIFOTHD:
			SN_I2S4->IE_b.RXFIFOTHIEN = 0;
			SN_I2S4->IE_b.RXFIFOUDFIEN = 0;
			SN_I2S4->IE_b.TXFIFOTHIEN = 1;
			SN_I2S4->IE_b.TXFIFOOVFIEN = 0;
			break;
		case I2S4_TX_FIFOOV:
		default:
			SN_I2S4->IE_b.RXFIFOTHIEN = 0;
			SN_I2S4->IE_b.RXFIFOUDFIEN = 0;
			SN_I2S4->IE_b.TXFIFOTHIEN = 0;
			SN_I2S4->IE_b.TXFIFOOVFIEN = 1;
			break;
	}
	NVIC_ClearPendingIRQ(I2S4_IRQn);
	NVIC_EnableIRQ(I2S4_IRQn);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Set_INT_Disable(void)
{
		SN_I2S4->IE_b.RXFIFOTHIEN = 0;
		SN_I2S4->IE_b.RXFIFOUDFIEN = 0;
		SN_I2S4->IE_b.TXFIFOTHIEN = 0;
		SN_I2S4->IE_b.TXFIFOOVFIEN = 0;
		NVIC_ClearPendingIRQ(I2S4_IRQn);
		NVIC_DisableIRQ(I2S4_IRQn);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t I2S4_Read_Rx_FIFO(void)
{
		return SN_I2S4->RXFIFO;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_Write_Tx_FIFO(uint32_t Value)
{
		SN_I2S4->TXFIFO=Value;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t I2S4_Read_IP_Status(void)
{
		return SN_I2S4->STATUS;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t I2S4_Read_Int_Status(void)
{
		return SN_I2S4->RIS;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t I2S4_Read_Tx_FIFO_Status(void)
{
	/*Tx FIFO Used level, Valid value = 0~8 ,0: Empty ,8: Full*/
		return SN_I2S4->STATUS_b.TXFIFOLV;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t I2S4_Read_Rx_FIFO_Status(void)
{
	/*Rx FIFO Used level, Valid value = 0~8 ,0: Empty ,8: Full*/
		return SN_I2S4->STATUS_b.RXFIFOLV;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_MCLK_OUT0_SEL(uint8_t Value)
{
		SN_I2S4->CLK_b.MCLK_OUT0_SEL = Value;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void I2S4_MCLK_OUT1_SEL(uint8_t Value)
{
		SN_I2S4->CLK_b.MCLK_OUT1_SEL = Value;
}
void I2S4_MCLK_OUT_ENABLE(uint8_t Value)
{
		SN_I2S4->CLK_b.MCLKOEN = Value;
}

static uint32_t temp_reg = 0;

void I2S4_MCLK_OUT0_OUT1_SEL(void)
{
	uint32_t temp_reg_mclk;
	/* Set MCLK1 = XTAL */
	temp_reg &= ~SET_BIT9;//temp_reg |=SET_BIT19;
	temp_reg &= ~SET_BIT18;
	/* Set MCLK2 = XTAL */
	temp_reg &= ~SET_BIT7;//|=SET_BIT7;
	temp_reg &= ~SET_BIT6;

	temp_reg_mclk = *(uint32_t*)(0x40034004);
	temp_reg_mclk |= temp_reg;
	*(uint32_t*)(0x40034004) = temp_reg_mclk;

}

void I2S4_Init(void)
{
	I2S4_MCLK_OUT0_OUT1_SEL();
	I2S4_Enable(0);
	I2S4_Start();
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: 
								uint8_t mode			:	Tx Rx Mode
								uint8_t *L_Data		:	L-Channel Data ptr
								uint8_t *R_Data		:gjo3dj4	R-Channel Data ptr
								uint8_t Len				:	Receive or Send byte data LENGTH  (Length unit = uint8_t)
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t I2S4_8Bit_STD_Handler(uint8_t Mode, uint8_t *L_Data, uint8_t *R_Data, uint8_t u8_Len)
{
	union I2S4_8BIT_FIFO_STC temp;
	uint8_t j;
	
	if(u8_Len<1){
		return I2S4_FAIL;
	}
	
	temp.Data = 0;
	
	if(Mode==I2S4_MASTER_TX || Mode==I2S4_SLAVE_TX){
		
		for(j=0; j<u8_Len; j++){
			if( !(j & 0x01) ){
				temp.BIT.L0 = L_Data[j];
				temp.BIT.R0 = R_Data[j];
				if((j+1)==u8_Len){
					I2S4_Write_Tx_FIFO(temp.Data);
					temp.Data = 0;
				}
			}
			else{
				temp.BIT.L1 = L_Data[j];
				temp.BIT.R1 = R_Data[j];
				I2S4_Write_Tx_FIFO(temp.Data);
				temp.Data = 0;
			}
		}
		return I2S4_PASS;
	}
	else if(Mode==I2S4_MASTER_RX || Mode==I2S4_SLAVE_RX){
		
		for(j=0; j<u8_Len; j++){
			if( !(j & 0x01) ){
				temp.Data = I2S4_Read_Rx_FIFO();
				L_Data[j] = temp.BIT.L0;
				R_Data[j] = temp.BIT.R0;
			}
			else{
				L_Data[j] = temp.BIT.L1;
				R_Data[j] = temp.BIT.R1;
			}
		}
		return I2S4_PASS;
	}
	return I2S4_FAIL;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: 
								uint8_t mode				:	Tx Rx Mode
								uint16_t *L_Data		:	L-Channel Data ptr
								uint16_t *R_Data		:	R-Channel Data ptr
								uint8_t Len					:	Receive or Send word data LENGTH (Length unit = uint16_t)
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t I2S4_16Bit_STD_Handler(uint8_t Mode, uint16_t *L_Data, uint16_t *R_Data, uint8_t u16_Len)
{
	union I2S4_16BIT_FIFO_STC temp;
	uint8_t j;
	
	if(u16_Len<1){
		return I2S4_FAIL;
	}
	
	temp.Data = 0;
	
	if(Mode==I2S4_MASTER_TX || Mode==I2S4_SLAVE_TX){

		for(j=0; j<u16_Len; j++){
			temp.BIT.L0 = L_Data[j];
			temp.BIT.R0 = R_Data[j];
			I2S4_Write_Tx_FIFO(temp.Data);
		}
		return I2S4_PASS;
	}
	else if(Mode==I2S4_MASTER_RX || Mode==I2S4_SLAVE_RX){
		
		for(j=0; j<u16_Len; j++){
			temp.Data = I2S4_Read_Rx_FIFO();
			L_Data[j] = temp.BIT.L0;
			R_Data[j] = temp.BIT.R0;
		}
		return I2S4_PASS;
	}
	return I2S4_FAIL;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: 
								uint8_t mode				:	Tx Rx Mode
								uint32_t *L_Data		:	L-Channel Data ptr
								uint32_t *R_Data		:	R-Channel Data ptr
								uint8_t Len					:	Receive or Send 32bit data LENGTH (Length unit = uint32_t)
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t I2S4_32Bit_STD_Handler(uint8_t Mode, uint32_t *L_Data, uint32_t *R_Data, uint8_t u32_Len)
{
	struct I2S4_32BIT_FIFO_STC temp;
	uint8_t j;
	
	if(u32_Len<1){
		return I2S4_FAIL;
	}
	
	temp.L_Data = 0;
	temp.R_Data = 0;
	
	if(Mode==I2S4_MASTER_TX || Mode==I2S4_SLAVE_TX){
		
		for(j=0; j<u32_Len; j++){
			temp.L_Data = L_Data[j];
			temp.R_Data = R_Data[j];
			I2S4_Write_Tx_FIFO(temp.L_Data);
			I2S4_Write_Tx_FIFO(temp.R_Data);
		}
		return I2S4_PASS;
	}
	else if(Mode==I2S4_MASTER_RX || Mode==I2S4_SLAVE_RX){
		
		for(j=0; j<u32_Len; j++){
			temp.L_Data = I2S4_Read_Rx_FIFO();
			temp.R_Data = I2S4_Read_Rx_FIFO();
			L_Data[j] = temp.L_Data;
			R_Data[j] = temp.R_Data;
		}
		return I2S4_PASS;
	}
	return I2S4_FAIL;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: 
								uint8_t mode				:	Tx Rx Mode
								uint32_t *L_Data		:	L-Channel Data ptr
								uint32_t *R_Data		:	R-Channel Data ptr
								uint8_t Len					:	Receive or Send data LENGTH (Length unit = sizeof(uint32_t)*FIFO_Cnt)
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t I2S4_MONO_Handler(uint8_t Mode, uint32_t *Data, uint8_t u32_Len)
{
	uint8_t j;
	
	if(u32_Len<1){
		return I2S4_FAIL;
	}
	if(Mode==I2S4_MASTER_TX || Mode==I2S4_SLAVE_TX){
		
		for(j=0; j<u32_Len; j++){
			I2S4_Write_Tx_FIFO(Data[j]);
		}
		return I2S4_PASS;
	}
	else if(Mode==I2S4_MASTER_RX || Mode==I2S4_SLAVE_RX){
		
		for(j=0; j<u32_Len; j++){
			Data[j] = I2S4_Read_Rx_FIFO();
		}
		return I2S4_PASS;
	}
	return I2S4_FAIL;
}
















