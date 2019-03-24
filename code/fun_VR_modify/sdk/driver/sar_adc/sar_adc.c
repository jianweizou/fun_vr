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
#include "snc_sar_adc.h"
#include "register_7320.h"


/*_____ M A C R O S ________________________________________________________*/




//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================




void _SAR_ADC_Enable()
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
    pDev->ADM_b.ADC_EN=1;
}
void _SAR_ADC_Disable()
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		pDev->ADM_b.ADC_EN=0;
		
}
void _SAR_ADC_Start()
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		pDev->ADM_b.START=1;
}
void _SAR_ADC_Stop()
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		pDev->ADM_b.START=0;
}
uint32_t SAR_ADC_status()
{
	  SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		return pDev->ADM_b.START;
}	
void _SAR_ADC_Channel_Sel(channel_select_t ch)
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
    pDev->ADP_b.CHS=ch;
}
void _SAR_ADC_ClockSource(uint32_t adc_cks)
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		pDev->ADM_b.ADCKS=adc_cks;
}	
void _SAR_ADC_ClockRate(ad_2m_clk_t adc_ckr)
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		pDev->ADM_b.AD_2M_CLK=adc_ckr;
}	

void _SAR_ADC_S_MODE(uint8_t adc_mode)
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		pDev->ADM_b.S_MODE=adc_mode;
}
uint32_t SAR_ADC_DATA()
{
		SN_SAR_ADC_Type *pDev = (SN_SAR_ADC_Type*) SN_SAR_ADC_BASE;
		return (pDev->ADR_b.DATA);
}

uint32_t SAR_ADC_DATA_valid()
{
	volatile uint32_t temp;
	temp = *( uint32_t volatile*)(0x4003A000 + 0x04);
	
	if((temp&0x02) == 0x2 )
		return 0;
	else
		return 1;

}
void SAR_ADC_Enable()
{
		_SAR_ADC_Enable();
}
void SAR_ADC_Disable()
{
		_SAR_ADC_Disable();
		
}
void SAR_ADC_Start()
{
		_SAR_ADC_Start();
		
}
void SAR_ADC_Stop()
{
		_SAR_ADC_Stop();
		
}
void SAR_ADC_init(sar_adc_info *sar_adc_init)
{
		_SAR_ADC_Channel_Sel(sar_adc_init->channel_select);
		_SAR_ADC_ClockRate(sar_adc_init->adc_2M_clk);
		_SAR_ADC_S_MODE(sar_adc_init->ad_mode);
		_SAR_ADC_ClockSource(sar_adc_init->adc_clock_sel);

}	
