	/******************** (C) COPYRIGHT 2016 SONiX *******************************
* COMPANY:		  SONiX
* DATE:			    2016/03
* IC:			      SNC7312
* DESCRIPTION:	SPI/SSP functions.
*____________________________________________________________________________
* REVISION	     Date		       User		  Description
* 1.0		         2016/03/31	   SA2			
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
#include <stdio.h>
#include "snc_spi_dma.h"
#include "snc_idma.h"
#include "register_7320.h"

#define SPI_DMA_VERSION        0x73200000
/*_____ M A C R O S ________________________________________________________*/

/******************** (C) COPYRIGHT 2016 SONiX *******************************
* COMPANY:	SONiX
* DATE:			2017/07
* IC:				SNC7320
*____________________________________________________________________________
* REVISION	Date		User		Description
* 1.0		2017/09/18	SA2			First release
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/****************************************************************************
* Code		: ALG sample code
* Description	: 
					
***************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
//#include <stdio.h>
//#include "SNC7320.h"
//#include "sn_spi_dma.h"

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/


static SN_SPI_DMA*
_spi_dma_ch_get_handle(
    spi_dma_sel_t spi_dma_ch)
{
    SN_SPI_DMA   *IP_SEL = 0;
    switch(spi_dma_ch)
    {
        case SPI_DMA_0:   IP_SEL = (SN_SPI_DMA*)SN_SPI0_DMA_IP_BASE; break;
        case SPI_DMA_1:   IP_SEL = (SN_SPI_DMA*)SN_SPI1_DMA_IP_BASE; break;
      

        default:    break;
    }

    return IP_SEL;
}

/*****************************************************************************
* Function		: SPI_DMA_Enable
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Enable(spi_dma_sel_t spi_dma_sel)
{
	 SN_SPI_DMA   *pDev = 0;
    pDev = _spi_dma_ch_get_handle(spi_dma_sel);
	/*SPI DMA IP force Master Only, For large data transfer application*/
	pDev->SPICTRL_b.MSMODE = 0;
	/*SPI DMA IP Enable*/
	pDev->SPICTRL_b.SPIEN = 1;
}

/*****************************************************************************
* Function		: SPI_DMA_Disable
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Disable(spi_dma_sel_t spi_dma_sel)
{
		SN_SPI_DMA   *pDev = 0;
		pDev = _spi_dma_ch_get_handle(spi_dma_sel);
		/*SPI DMA IP Disable*/
		pDev->SPICTRL_b.SPIEN = 0;
}

/*****************************************************************************
* Function		: SPI_DMA_ClockRate
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_ClockRate(spi_dma_sel_t spi_dma_sel, uint8_t PreScaler, uint8_t Div)
{
		SN_SPI_DMA   *pDev = 0;
		pDev = _spi_dma_ch_get_handle(spi_dma_sel);
		pDev->SPIBAUD_b.SPIDIV = Div;
		pDev->SPIBAUD_b.SPIPRS = PreScaler;
 
}

/*****************************************************************************
* Function		: SPI_DMA_ClockPhasePolarity
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_ClockPhasePolarity(spi_dma_sel_t spi_dma_sel, uint8_t MODE)
{
		SN_SPI_DMA   *pDev = 0;
			pDev = _spi_dma_ch_get_handle(spi_dma_sel);
		switch(MODE){
			case SPIDMA_PHASE_POLARITY_MODE0:
				pDev->SPICTRL_b.CPHA = 0;
				pDev->SPICTRL_b.CPOL = 0;
				break;
			case SPIDMA_PHASE_POLARITY_MODE1:
				pDev->SPICTRL_b.CPHA = 1;
				pDev->SPICTRL_b.CPOL = 0;
				break;
			case SPIDMA_PHASE_POLARITY_MODE2:
				pDev->SPICTRL_b.CPHA = 0;
				pDev->SPICTRL_b.CPOL = 1;
				break;
			case SPIDMA_PHASE_POLARITY_MODE3:
				pDev->SPICTRL_b.CPHA = 1;
				pDev->SPICTRL_b.CPOL = 1;
				break;
		};
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_IRQ_Enable(spi_dma_sel_t spi_dma_sel)
{
		
			/*Select IP IDMA*/
		if(spi_dma_sel == SPI_DMA_0){
			NVIC_ClearPendingIRQ(SPI0_DMA_IRQn);	
			NVIC_EnableIRQ(SPI0_DMA_IRQn);
		}
		else /*if(IP_SEL == SEL_SPIDMA1)*/{
			NVIC_ClearPendingIRQ(SPI1_DMA_IRQn);	
			NVIC_EnableIRQ(SPI1_DMA_IRQn);
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
void SPI_DMA_IRQ_Disable(spi_dma_sel_t spi_dma_sel)
{
			/*Select IP IDMA*/
		if(spi_dma_sel == SPI_DMA_0){
			NVIC_ClearPendingIRQ(SPI0_DMA_IRQn);	
			NVIC_DisableIRQ(SPI0_DMA_IRQn);
		}
		else /*if(IP_SEL == SEL_SPIDMA1)*/{
			NVIC_ClearPendingIRQ(SPI1_DMA_IRQn);	
			NVIC_DisableIRQ(SPI1_DMA_IRQn);
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
void SPI_DMA_Endial_Set(spi_dma_sel_t spi_dma_sel, uint8_t endian_set)
{
	    SN_SPI_DMA   *pDev = 0;
		pDev = _spi_dma_ch_get_handle(spi_dma_sel); 
		pDev->SPICTRL_b.ENDIAN_SEL = endian_set;
}


/*****************************************************************************
* Function		: SPI_DMA_FWSet_CS
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_FWSet_CS(spi_dma_sel_t spi_dma_sel, spi_dma_cs_t CS_Set)
{
		SN_SPI_DMA   *pDev = 0;
		pDev = _spi_dma_ch_get_handle(spi_dma_sel); 
		pDev->SPICS_b.CS = 0;
		pDev->SPICS_b.SW_CS = CS_Set;
}

/*****************************************************************************
* Function		: SPI_DMA_FWSend_Data
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t SPI_DMA_FWSend_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint8_t TransferLenth)
{
	uint8_t Len;
    uint8_t *Ptr;
    SN_SPI_DMA   *pDev = 0;
	pDev = _spi_dma_ch_get_handle(spi_dma_sel);  
		/*Check valid mode*/
		if( TransferLenth != SPIDMA_8BIT_TRANSFER && TransferLenth != SPIDMA_16BIT_TRANSFER && TransferLenth != SPIDMA_24BIT_TRANSFER 
			&& TransferLenth != SPIDMA_32BIT_TRANSFER && TransferLenth != SPIDMA_40BIT_TRANSFER && TransferLenth != SPIDMA_48BIT_TRANSFER ){
				return SPIDMA_FAIL;
		}

		/*Setup 1xIO 2xIO 4xIO 3Wire*/
		switch(TransferMode){
			case SPIDMA_1IO_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_2IO_MODE:
				pDev->SPICTRL_b.DUAL = 1;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_4IO_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 1;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_3WIRE_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 1;
				pDev->SPICTRL_b.SPI_MOSI_EN = 0;
				break;
		};		
		
		/*Setup Transfer Lenth*/
		pDev->SPITRANSFER = TransferLenth;
		
		/*Reset SPI TC*/
		pDev->SPITCW |= SET_BIT0;
		
		/*Setup Transfer Data*/
		Len = TransferLenth;      
		Ptr = (uint8_t *)DataPtr; 
		while(Len!=0){
			switch(Len){
				case SPIDMA_8BIT_TRANSFER:
					pDev->SPIDATABUF0 = *Ptr;
					break;
				case SPIDMA_16BIT_TRANSFER:
					pDev->SPIDATABUF1 = *Ptr;
					break;
				case SPIDMA_24BIT_TRANSFER:
					pDev->SPIDATABUF2 = *Ptr;
					break;
				case SPIDMA_32BIT_TRANSFER:
					pDev->SPIDATABUF3 = *Ptr;
					break;
				case SPIDMA_40BIT_TRANSFER:
					pDev->SPIDATABUF4 = *Ptr;
					break;
				case SPIDMA_48BIT_TRANSFER:
					pDev->SPIDATABUF5 = *Ptr;
					break;
			}
			Len -= 8;
			Ptr++;
		}
	
		/*Trigger to Transfer*/
		pDev->SPICTRL_b.START = 1;
		while(pDev->SPICTRL_b.START);
		return SPIDMA_PASS;
}

/*****************************************************************************
* Function		: SPI_DMA_FWReceive_Data
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t SPI_DMA_FWReceive_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint8_t TransferLenth)
{
		uint8_t Len,*Ptr;
		SN_SPI_DMA   *pDev = 0;
		pDev = _spi_dma_ch_get_handle(spi_dma_sel);
		/*Check valid mode*/
		if( TransferLenth != SPIDMA_8BIT_TRANSFER && TransferLenth != SPIDMA_16BIT_TRANSFER && TransferLenth != SPIDMA_24BIT_TRANSFER 
			&& TransferLenth != SPIDMA_32BIT_TRANSFER && TransferLenth != SPIDMA_40BIT_TRANSFER && TransferLenth != SPIDMA_48BIT_TRANSFER ){
				return SPIDMA_FAIL;
		}
		
		/*Setup 1xIO 2xIO 4xIO 3Wire*/
		switch(TransferMode){
			case SPIDMA_1IO_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_2IO_MODE:
				pDev->SPICTRL_b.DUAL = 1;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 1;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_4IO_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 1;
				pDev->SPICTRL_b.RWMODE = 1;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_3WIRE_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 0;
				break;
		};
	
		/*Setup Transfer Lenth*/
		pDev->SPITRANSFER = TransferLenth;
		
		/*Reset SPI TC*/
		pDev->SPITCW |= SET_BIT0;
		
		/*Trigger to Transfer*/
		pDev->SPICTRL_b.START = 1;
		while(pDev->SPICTRL_b.START);
		
		/*Receive Data*/
		Len = TransferLenth;
		Ptr = DataPtr;
		while(Len!=0){
			switch(Len){
				case SPIDMA_8BIT_TRANSFER:
					*Ptr = pDev->SPIDATABUF0;
					break;
				case SPIDMA_16BIT_TRANSFER:
					*Ptr = pDev->SPIDATABUF1;
					break;
				case SPIDMA_24BIT_TRANSFER:
					*Ptr = pDev->SPIDATABUF2;
					break;
				case SPIDMA_32BIT_TRANSFER:
					*Ptr = pDev->SPIDATABUF3;
					break;
				case SPIDMA_40BIT_TRANSFER:
					*Ptr = pDev->SPIDATABUF4;
					break;
				case SPIDMA_48BIT_TRANSFER:
					*Ptr = pDev->SPIDATABUF5;
					break;
			}
			Len -= 8;
			Ptr++;
		}
	
		return SPIDMA_PASS;
}
	

/*****************************************************************************
* Function		: SPI_DMA_Send_Data
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t SPI_DMA_Send_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint32_t TransferLenth)
{
		idma_ch_t IDMA;
		SN_SPI_DMA   *pDev = 0;
		idma_setting_t  spi_idma_setting = {0};
		pDev = _spi_dma_ch_get_handle(spi_dma_sel);
		/*Select IP IDMA*/
		if(spi_dma_sel == SPI_DMA_0){
			IDMA = IDMA_CH_SPI0;
		}
		else /*if(IP_SEL == SEL_SPIDMA1)*/{
			IDMA = IDMA_CH_SPI1;
		}
		
		/*Setup 1xIO 2xIO 4xIO 3Wire*/
		switch(TransferMode){
			case SPIDMA_1IO_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
					pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_2IO_MODE:
				pDev->SPICTRL_b.DUAL = 1;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
				pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_4IO_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 1;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 0;
					pDev->SPICTRL_b.SPI_MOSI_EN = 1;
				break;
			case SPIDMA_3WIRE_MODE:
				pDev->SPICTRL_b.DUAL = 0;
				pDev->SPICTRL_b.QUAD = 0;
				pDev->SPICTRL_b.RWMODE = 0;
				pDev->SPICTRL_b.SPI_3W = 1;
				pDev->SPICTRL_b.SPI_MOSI_EN = 0;
				break;
		};	
		
		/*Setup IDMA*/
		spi_idma_setting.length=TransferLenth;
		spi_idma_setting.ram_addr=(uint32_t)DataPtr;
		spi_idma_setting.direction=IDMA_DIRECTION_RAM_2_DIP;
		IDMA_Start(IDMA,&spi_idma_setting);
		

		while(IDMA_Get_Status(IDMA));
		
		return SPIDMA_PASS;
}

/*****************************************************************************
* Function		: SPI_DMA_Receive_Data
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t SPI_DMA_Receive_Data(spi_dma_sel_t spi_dma_sel, uint8_t TransferMode, uint8_t *DataPtr, uint32_t TransferLenth)
{
	idma_ch_t IDMA;
	SN_SPI_DMA   *pDev = 0;
	idma_setting_t  spi_idma_setting = {0};
	pDev = _spi_dma_ch_get_handle(spi_dma_sel);
	/*Select IP IDMA*/
	if(spi_dma_sel == SPI_DMA_0){
		IDMA = IDMA_CH_SPI0;
	}
	else /*if(IP_SEL == SEL_SPIDMA1)*/{
		IDMA = IDMA_CH_SPI1;
	}
	
	/*Setup 1xIO 2xIO 4xIO 3Wire*/
	switch(TransferMode){
		case SPIDMA_1IO_MODE:
			pDev->SPICTRL_b.DUAL = 0;
			pDev->SPICTRL_b.QUAD = 0;
			pDev->SPICTRL_b.RWMODE = 0;
			pDev->SPICTRL_b.SPI_3W = 0;
			pDev->SPICTRL_b.SPI_MOSI_EN = 1;
			break;
		case SPIDMA_2IO_MODE:
			pDev->SPICTRL_b.DUAL = 1;
			pDev->SPICTRL_b.QUAD = 0;
			pDev->SPICTRL_b.RWMODE = 1;
			pDev->SPICTRL_b.SPI_3W = 0;
			pDev->SPICTRL_b.SPI_MOSI_EN = 1;
			break;
		case SPIDMA_4IO_MODE:
			pDev->SPICTRL_b.DUAL = 0;
			pDev->SPICTRL_b.QUAD = 1;
			pDev->SPICTRL_b.RWMODE = 1;
			pDev->SPICTRL_b.SPI_3W = 0;
			pDev->SPICTRL_b.SPI_MOSI_EN = 1;
			break;
		case SPIDMA_3WIRE_MODE:
			pDev->SPICTRL_b.DUAL = 0;
			pDev->SPICTRL_b.QUAD = 0;
			pDev->SPICTRL_b.RWMODE = 0;
			pDev->SPICTRL_b.SPI_3W = 0;
			pDev->SPICTRL_b.SPI_MOSI_EN = 0;
			break;
	};	
	
	/*Setup IDMA*/
//	IDMA->CH_LEN = TransferLenth;
//	IDMA->CH_RAM_ADDR = (uint32_t)DataPtr;
//	IDMA->CH_CTRL_b.WR_RAM = 1;	//0:DIP->RAM
//	IDMA->CH_CTRL_b.START = 1;
//	while(IDMA->CH_CTRL_b.START);
		spi_idma_setting.length=TransferLenth;
		spi_idma_setting.ram_addr=(uint32_t)DataPtr;
		spi_idma_setting.direction=IDMA_DIRECTION_DIP_2_RAM;
		IDMA_Start(IDMA,&spi_idma_setting);
		while(IDMA_Get_Status(IDMA));
		/*When DMA Done , Wait Ecc code read finish*/
		if(pDev->SPI_ECC_CTRL_b.ECC_En){
			while(!pDev->SPI_ECC_STATUS_b.ECC_DMA_DN);
		}
		return SPIDMA_PASS;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/



void SPI_DMA_Init(spi_dma_sel_t spi_dma_ch,spi_dma_init_t *spi_dma_init_info )     
{

	SPI_DMA_ClockRate(spi_dma_ch, spi_dma_init_info->prescale, spi_dma_init_info->clock_div);	
//    SPI_DMA_ClockRate(spi_dma_ch, 4, 1);	/*SCK = (SysClk)/(SPIPRS*(SPIDIV+1))*/ /*SCK = 40Mhz/(32*(1+1)) = 0.625Mhz = 625Khz*/
//	SPI_DMA_ClockRate(spi_dma_ch, 10, 10);	
//	SPI_DMA_ClockRate(spi_dma_ch, 10, 20);	
	SPI_DMA_ClockPhasePolarity(spi_dma_ch, spi_dma_init_info->polarity_sel);
	SPI_DMA_FWSet_CS(SPI_DMA_0, SPIDMA_CS_HIGH);

	//SPI_DMA_Nand_ECC_Disable(spi_dma_ch);
	/*SPI DMA IP Enable*/

	SPI_DMA_Enable(spi_dma_ch);

}

uint32_t
SPI_DMA_GetVersion(void)
{
    return SPI_DMA_VERSION;
}











