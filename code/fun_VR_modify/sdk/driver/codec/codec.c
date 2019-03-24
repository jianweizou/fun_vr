/******************** (C) COPYRIGHT 2018 SONiX *******************************
* COMPANY:		SONiX
* DATE:			2018/06
* IC:				SNC7320
* DESCRIPTION:	Audio codec drivers.
*____________________________________________________________________________
* REVISION	Date			User		Description
* 1.0		2018/7/4	SA2		First release
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
#include "snc_codec.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
 
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
extern void I2S_Set_BCLK(i2s_sel_t i2s_ch, i2s_clocksource_t clock_source, i2s_sample_rate_t samplerate);
extern void I2S_UnMute(i2s_sel_t i2s_ch);
extern void I2S_Resume(i2s_sel_t i2s_ch);

static codec_error_code_t Check_Serial_Number_is_Correct(codec_i2s_t eI2S)
{
	if ((eI2S!=eCODEC_I2S_0)&&(eI2S!=eCODEC_I2S_1)&&(eI2S!=eCODEC_I2S_2)&&(eI2S!=eCODEC_I2S_3))
		return eCODEC_1ST_PARAMETER_ERROR;
	return eCODEC_SUCCESS;
}

/*****************************************************************************
* Function		: ADC_Set_Channel_Mode
* Description	: set ADC channel mode
* Arguments	: eI2S
				eCODEC_I2S_0
				eCODEC_I2S_1
				eCODEC_I2S_2
				eCODEC_I2S_3
			: eChannelMode
				eCODEC_MONO
				eCODEC_STEREO
* Return		: error code
* Note		: None
*****************************************************************************/
codec_error_code_t ADC_Set_Channel_Mode(codec_i2s_t eI2S, codec_channel_mode_t eChannelMode)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	if ((eChannelMode!=eCODEC_MONO)&&(eChannelMode!=eCODEC_STEREO))
		return eCODEC_2ND_PARAMETER_ERROR;
	
	I2S_Set_MONO_STEREO((i2s_sel_t)eI2S, eChannelMode);

	return eCODEC_SUCCESS;
}

codec_error_code_t ADC_Turn_On(codec_i2s_t eI2S, codec_clock_source_t eClockSource, codec_sampling_rate_t eSamplingRate)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	I2S_Set_Ch_Len((i2s_sel_t)eI2S, 15);
	I2S_Set_Data_Len((i2s_sel_t)eI2S, I2S_DATA_LENGTH_16BIT);
	I2S_Set_Format((i2s_sel_t)eI2S, I2S_STANDARD_FORMAT);

	//clock setup
	I2S_Set_BCLK((i2s_sel_t)eI2S, (i2s_clocksource_t)eClockSource, (i2s_sample_rate_t)eSamplingRate);

	//command I2S
	I2S_MODE((i2s_sel_t)eI2S, I2S_SLAVE);
	I2S_Rx_Enable((i2s_sel_t)eI2S);
	I2S_Enable((i2s_sel_t)eI2S);
	I2S_UnMute((i2s_sel_t)eI2S);
	I2S_Start((i2s_sel_t)eI2S);

	return eCODEC_SUCCESS;
}

codec_error_code_t ADC_Pause_Switch(codec_i2s_t eI2S, codec_pause_switch_t ePauseSwitch)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	if (ePauseSwitch == eCODEC_PAUSE)
		I2S_Pause((i2s_sel_t)eI2S);
	else if (ePauseSwitch == eCODEC_RESUME)
		I2S_Resume((i2s_sel_t)eI2S);
	else
		return eCODEC_2ND_PARAMETER_ERROR;
	
	return eCODEC_SUCCESS;
}

codec_error_code_t ADC_Set_DMA_Address_and_Length(codec_i2s_t eI2S, short* pBuffer, unsigned short BufferLength, unsigned short Buffer2Offset)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	I2S_Set_RxBuf1_Addr((i2s_sel_t)eI2S, (unsigned int*)pBuffer);
	I2S_Set_RxBuf2_Offset((i2s_sel_t)eI2S, Buffer2Offset<<1);
	I2S_Set_DMA_Len((i2s_sel_t)eI2S, BufferLength<<1);
	return eCODEC_SUCCESS;
}

unsigned int ADC_Get_Rx_Index(codec_i2s_t eI2S)
{
	return ((I2S_Get_RxBuf_Idx((i2s_sel_t)eI2S)-I2S_Get_RxBuf1_Addr((i2s_sel_t)eI2S))>>1);
}

codec_error_code_t
	ADC_Enable (
		codec_i2s_t			eI2S,
		short*				pBuffer,
		unsigned short			BufferLength,
		codec_sampling_rate_t	eSamplingRate,
		codec_channel_mode_t	eChannel)
{
	if (ADC_Set_Channel_Mode(eI2S, eChannel))
		return eCODEC_ERROR;
	if (ADC_Set_DMA_Address_and_Length(eI2S, pBuffer, BufferLength, BufferLength))
		return eCODEC_ERROR;
	if (ADC_Turn_On(eI2S, eCODEC_XTAL, eSamplingRate))
		return eCODEC_ERROR;
	return eCODEC_SUCCESS;
}

codec_error_code_t ADC_Disable(codec_i2s_t eI2S)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	I2S_Disable((i2s_sel_t)eI2S);

	return eCODEC_SUCCESS;
}

codec_error_code_t DAC_Set_Channel_Mode(codec_i2s_t eI2S, codec_channel_mode_t eChannelMode)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	if ((eChannelMode!=eCODEC_MONO)&&(eChannelMode!=eCODEC_STEREO))
		return eCODEC_2ND_PARAMETER_ERROR;
	
	I2S_Set_MONO_STEREO((i2s_sel_t)eI2S, eChannelMode);

	return eCODEC_SUCCESS;
}

codec_error_code_t DAC_Mute_Switch(codec_i2s_t eI2S, codec_mute_switch_t eMuteSwitch)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	if (eMuteSwitch == eCODEC_MUTE)
		I2S_Mute((i2s_sel_t)eI2S);
	else if (eMuteSwitch == eCODEC_UNMUTE)
		I2S_UnMute((i2s_sel_t)eI2S);
	else
		return eCODEC_2ND_PARAMETER_ERROR;
	
	return eCODEC_SUCCESS;
}

codec_error_code_t DAC_Turn_On(codec_i2s_t eI2S, codec_clock_source_t eClockSource, codec_sampling_rate_t eSamplingRate)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	I2S_Set_Data_Len((i2s_sel_t)eI2S, I2S_DATA_LENGTH_16BIT);
	I2S_Set_Ch_Len((i2s_sel_t)eI2S, 31);
	I2S_Set_Format((i2s_sel_t)eI2S, I2S_STANDARD_FORMAT);

	//clock setup 
	I2S_Set_BCLK((i2s_sel_t)eI2S, (i2s_clocksource_t)eClockSource, (i2s_sample_rate_t)eSamplingRate);

	//command I2S
	I2S_MODE((i2s_sel_t)eI2S, I2S_SLAVE);
	I2S_Tx_Enable((i2s_sel_t)eI2S);
	I2S_Enable((i2s_sel_t)eI2S);
	I2S_UnMute((i2s_sel_t)eI2S);
	I2S_Start((i2s_sel_t)eI2S);

	return eCODEC_SUCCESS;
}

codec_error_code_t DAC_Set_DMA_Address_and_Length(codec_i2s_t eI2S, short* pBuffer, unsigned short BufferLength, unsigned short Buffer2Offset)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	I2S_Set_TxBuf1_Addr((i2s_sel_t)eI2S, (unsigned int*)pBuffer);
	I2S_Set_TxBuf2_Offset((i2s_sel_t)eI2S, Buffer2Offset<<1);
	I2S_Set_DMA_Len((i2s_sel_t)eI2S, BufferLength<<1);
	return eCODEC_SUCCESS;
}

unsigned int DAC_Get_Tx_Index(codec_i2s_t eI2S)
{
	return ((I2S_Get_TxBuf_Idx((i2s_sel_t)eI2S)-I2S_Get_TxBuf1_Addr((i2s_sel_t)eI2S))>>1);
}

codec_error_code_t DAC_Pause_Switch(codec_i2s_t eI2S, codec_pause_switch_t ePauseSwitch)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	if (ePauseSwitch == eCODEC_PAUSE)
		I2S_Pause((i2s_sel_t)eI2S);
	else if (ePauseSwitch == eCODEC_RESUME)
		I2S_Resume((i2s_sel_t)eI2S);
	else
		return eCODEC_2ND_PARAMETER_ERROR;
	
	return eCODEC_SUCCESS;
}

codec_error_code_t
	DAC_Enable (
		codec_i2s_t			eI2S,
		short*				pBuffer,
		unsigned short			BufferLength,
		codec_sampling_rate_t	eSamplingRate,
		codec_channel_mode_t	eChannel)
{
	if (DAC_Set_Channel_Mode(eI2S, eChannel))
		return eCODEC_ERROR;
	if (DAC_Set_DMA_Address_and_Length(eI2S, pBuffer, BufferLength, BufferLength))
		return eCODEC_ERROR;
	if (DAC_Turn_On(eI2S, eCODEC_XTAL, eSamplingRate))
		return eCODEC_ERROR;
	return eCODEC_SUCCESS;
}

codec_error_code_t DAC_Disable(codec_i2s_t eI2S)
{
	if (Check_Serial_Number_is_Correct(eI2S))
		return eCODEC_1ST_PARAMETER_ERROR;
	
	I2S_Disable((i2s_sel_t)eI2S);

	return eCODEC_SUCCESS;
}


