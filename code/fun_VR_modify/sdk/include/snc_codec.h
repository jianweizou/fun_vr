/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_codec.h
 *
 * @author Axl Lee
 * @version 0.1
 * @date 2018/8/31
 * @license
 * @description
 */

#ifndef __snc_codec_H_wyGTxhNr_lUoN_HN2s_s9ax_uCc6IY9z2xmA__
#define __snc_codec_H_wyGTxhNr_lUoN_HN2s_s9ax_uCc6IY9z2xmA__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_i2s.h"

typedef enum {
	eCODEC_SUCCESS = 0,
	eCODEC_ERROR,
	eCODEC_1ST_PARAMETER_ERROR,
	eCODEC_2ND_PARAMETER_ERROR
} codec_error_code_t;

typedef enum {
	eCODEC_I2S_0 = 0,
	eCODEC_I2S_1,
	eCODEC_I2S_2,
	eCODEC_I2S_3
} codec_i2s_t;

typedef enum {
	eCODEC_STEREO	= 0,
	eCODEC_MONO	= 1
} codec_channel_mode_t;

typedef enum {
	eCODEC_RESUME	= 0,
	eCODEC_PAUSE	= 1
} codec_pause_switch_t;

typedef enum {
	eCODEC_UNMUTE	= 0,
	eCODEC_MUTE	= 1
} codec_mute_switch_t;

typedef enum {
	eCODEC_I2S_4 = 0,
	eCODEC_IHRC,
	eCODEC_XTAL,
	eCODEC_FPLL
} codec_clock_source_t;

typedef enum {
	eCODEC_8kHz		= 8000U,
	eCODEC_11p025kHz	= 11025U,
	eCODEC_12kHz		= 12000U,
	eCODEC_16kHz		= 16000U,
	eCODEC_22p05kHz	= 22050U,
	eCODEC_24kHz		= 24000U,
	eCODEC_32kHz		= 32000U,
	eCODEC_44p1kHz		= 44100U,
	eCODEC_48kHz		= 48000U,
	eCODEC_64kHz		= 64000U,
	eCODEC_96kHz		= 96000U
} codec_sampling_rate_t;


codec_error_code_t
	ADC_Disable (
		codec_i2s_t	eI2S
	);

codec_error_code_t
	ADC_Enable (
		codec_i2s_t			eI2S,
		short*				pBuffer,
		unsigned short			BufferLength,
		codec_sampling_rate_t	eSamplingRate,
		codec_channel_mode_t	eChannel
	);

unsigned int
	ADC_Get_Rx_Index (
		codec_i2s_t	eI2S
	);

unsigned int
	ADC_Get_Version(void);

codec_error_code_t
	ADC_Pause_Switch (
		codec_i2s_t			eI2S,
		codec_pause_switch_t	ePauseSwitch
	);

codec_error_code_t
	ADC_Set_Channel_Mode (
		codec_i2s_t			eI2S,
		codec_channel_mode_t	eChannelMode
	);

codec_error_code_t
	ADC_Set_DMA_Address_and_Length (
		codec_i2s_t	eI2S,
		short*		pBuffer,
		unsigned short	BufferLength,
		unsigned short	Buffer2Offset
	);

codec_error_code_t
	ADC_Turn_On (
		codec_i2s_t			eI2S,
		codec_clock_source_t	eClockSource,
		codec_sampling_rate_t	eSamplingRate
	);

codec_error_code_t
	DAC_Disable (
		codec_i2s_t	eI2S
	);

codec_error_code_t
	DAC_Enable (
		codec_i2s_t			eI2S,
		short*				pBuffer,
		unsigned short			BufferLength,
		codec_sampling_rate_t	eSamplingRate,
		codec_channel_mode_t	eChannel
	);

unsigned int
	DAC_Get_Tx_Index (
		codec_i2s_t	eI2S
	);

unsigned int
	DAC_Get_Version(void);

codec_error_code_t
	DAC_Mute_Switch (
		codec_i2s_t			eI2S,
		codec_mute_switch_t	eMuteSwitch
	);

codec_error_code_t
	DAC_Pause_Switch (
		codec_i2s_t			eI2S,
		codec_pause_switch_t	ePauseSwitch
	);

codec_error_code_t
	DAC_Set_Channel_Mode (
		codec_i2s_t			eI2S,
		codec_channel_mode_t	eChannelMode
	);

codec_error_code_t
	DAC_Set_DMA_Address_and_Length (
		codec_i2s_t	eI2S,
		short*		pBuffer,
		unsigned short	BufferLength,
		unsigned short	Buffer2Offset
	);

codec_error_code_t
	DAC_Turn_On (
		codec_i2s_t			eI2S,
		codec_clock_source_t	eClockSource,
		codec_sampling_rate_t	eSamplingRate
	);

#ifdef __cplusplus
}
#endif
#endif

