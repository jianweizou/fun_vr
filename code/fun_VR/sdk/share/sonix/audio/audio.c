#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "audio.h"

codec_channel_mode_t Convert_to_Codec_Channel(E_CHANNEL eChannel)
{
	switch (eChannel)
	{
		case eMONO:	return eCODEC_MONO;
		case eSTEREO:	return eCODEC_STEREO;
	}
	return eCODEC_MONO;
}

codec_sampling_rate_t Convert_to_Codec_Sampling_Rate(E_SAMPLING_RATE eSamplingRate)
{
	switch (eSamplingRate)
	{
		case e8kHz:		return eCODEC_8kHz;
		case e16kHz:		return eCODEC_16kHz;
		case e24kHz:		return eCODEC_24kHz;
		case e32kHz:		return eCODEC_32kHz;
		case e44p1kHz:	return eCODEC_44p1kHz;
		case e48kHz:		return eCODEC_48kHz;
		case e64kHz:		return eCODEC_64kHz;
		case e96kHz:		return eCODEC_96kHz;
	}
	return eCODEC_16kHz;
}

aud_samplerate_t Convert_to_Aud_Sampling_Rate(E_SAMPLING_RATE eSamplingRate)
{
	switch (eSamplingRate)
	{
		case e8kHz:		return AUD_SR8K;
		case e16kHz:		return AUD_SR16K;
		case e24kHz:		return AUD_SR24K;
		case e32kHz:		return AUD_SR32K;
		case e44p1kHz:	return AUD_SR44p1K;
		case e48kHz:		return AUD_SR48K;
		case e64kHz:		return AUD_SR64K;
		case e96kHz:		return AUD_SR96K;
	}
	return AUD_SR16K;
}

E_ERROR_CODE External_Audio_Codec_Init(E_EXTERNAL_AUDIO_CHIP eExternalAudioChip, sn_i2c_sw_ch_t eSN_I2C_CH, E_SAMPLING_RATE eSamplingRate)
{
         I2S4_Init();
         if (eExternalAudioChip == eEAC_AUD)
         {
//               I2C_ch_sel(eSN_I2C_CH);
//               I2C_SoftWare_Master_Init();
//               while(AUD_MasterMode_Init(Convert_to_Aud_Sampling_Rate(eSamplingRate), MCLK_PAD_TO_PLL, YES) == AUD_FAIL);
             //SNaudio01  driver       ZSC 20181031
              AUD_Driver_Init(eSN_I2C_CH);
              SNAUD01_MasterMode_Init(Convert_to_Aud_Sampling_Rate(eSamplingRate), MCLK_PAD_TO_PLL, 0,0,3,100,1);
              return eAUDIO_SUCCESS;
         }
         return eAUDIO_INITIAL_FAIL;
}

