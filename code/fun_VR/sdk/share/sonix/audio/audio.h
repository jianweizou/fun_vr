/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_audio.h
 *
 * @author Axl Lee
 * @version 0.1
 * @date 2018/9/4
 * @license
 * @description
 */
 
#ifndef	__snc_audio_h__
#define	__snc_audio_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "audio32codec_decoder.h"
#include "audio32codec_encoder.h"
#include "mp3enc_encoder.h"
#include "mp3dec_decoder.h"
#include "ima_codec.h"

#include "snc_codec.h"
#include "snc_i2s4.h"
#include "aud_driver.h"
#include "ff.h"

#define	NO	0
#define	YES	1
#define	FALSE	0
#define	TRUE	1
#define	OFF	0
#define	ON	1
#define	LOW		0
#define	HIGH	1

#define	S_FILE	FIL

typedef enum {
	ePPB0 = 0,
	ePPB1
} E_PINGPONG_BUFFER;

typedef enum {
	ePARSER = 0,
	eDECODER
} E_STAGE;

typedef enum {
	eAUDIO_SUCCESS = 0,
	eAUDIO_NOT_ENOUGH_MEMORY,
	eAUDIO_DAC_AMP_TURN_ON_FAIL,
	eAUDIO_INITIAL_FAIL,
	eAUDIO_SAMPLING_RATE_AUDIO,
	eAUDIO_BITRATE_AUDIO,
	eAUDIO_VALID_SYNCWORD_FOUND,
	eAUDIO_VALID_SYNCWORD_FOUND_AFTER_INVALID_SYNCWORD_SKIPPED,
	eAUDIO_END_OF_BITSTREAM
} E_ERROR_CODE;

typedef enum {
	eSD_CARD = 0,
	eSPI_FLASH,
	eSTREAMING
} E_IO_STORAGE;

typedef enum {
	eIDLE = 0,
	eINITIAL,
	eRECORDING,
	ePLAYING,
	eSTOP,
	ePAUSE,
	eRESUME,
	eMUTE
} E_STATUS;

typedef enum {
	eAUDIO32	= 0x5541,
	eIMA_ADPCM,
	eMP3		= 0xFFE0,
	eWAV		= 0x52494646
} E_AUDIO_FORMAT;

typedef enum {
	e8kHz		= 8000U,
	e16kHz		= 16000U,
	e24kHz		= 24000U,
	e32kHz		= 32000U,
	e44p1kHz	= 44100U,
	e48kHz		= 48000U,
	e64kHz		= 64000U,
	e96kHz		= 96000U
} E_SAMPLING_RATE;

typedef enum {
	e16kbps		= 16000U,
	e24kbps		= 24000U,
	e32kbps		= 32000U,
	e40kbps		= 40000U,
	e48kbps		= 48000U,
	e56kbps		= 56000U,
	e64kbps		= 64000U,
	e80kbps		= 80000U,
	e96kbps		= 96000U,
	e112kbps	= 112000U,
	e128kbps	= 128000U,
	e160kbps	= 160000U,
	e192kbps	= 192000U,
	e224kbps	= 224000U,
	e256kbps	= 256000U,
	e288kbps	= 288000U,
	e320kbps	= 320000U
} E_BITRATE;

typedef enum {
	eMONO	= 1,
	eSTEREO	= 2
} E_CHANNEL;

typedef enum {
	eMP3_MODE_STEREO			= 0,
	eMP3_MODE_JOINT_STEREO	= 1,
	eMP3_MODE_DUAL_CHANNEL	= 2,
	eMP3_MODE_MONO			= 3
} E_MP3_MODE;

typedef enum {
	eEAC_AUD = 0
} E_EXTERNAL_AUDIO_CHIP;


typedef	int (*PFN_MOVE_BITS_AWAY)(char* pStart, unsigned int Length);
typedef	int (*PFN_REFILL_BUFFER)(char* pStart, int Length, char *pRead, int *pRemainingBytes);
typedef	int (*PFN_MP3_RESET_BUFFER)(int Length, char* pRead, long int Position);
typedef	void (*PFN_PLAY_FINISHED)(void);

typedef struct {
	char			ChunkID[4];		//"RIFF"
	unsigned int	ChunkSize;		//overall size of file in bytes
	char			Format[4];		//"WAVE"
	char			SubChunk1ID[4];	//"fmt "
	unsigned int	SubChunk1Size;	//length of the format data
	unsigned short	AudioFormat;		//format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned short	NumOfChannels;	//0x1 for mono, 0x2 for stereo
	unsigned int	SampleRate;		//sampling rate (blocks per second)
	unsigned int	ByteRate;		//SampleRate * NumOfChannels * BitsPerSample/8
	unsigned short	BlockAlign;		//NumOfChannels * BitsPerSample/8
	unsigned short	BitsPerSample;	//bits per sample, 8- 8bits, 16- 16 bits etc
	char			SubChunk2ID[4];	//DATA string or FLLR string
	unsigned int	SubChunk2Size;	//NumSamples * NumOfChannels * BitsPerSample/8 - size of the next chunk that will be read
} S_WAV_HEADER, *PS_WAV_HEADER;

typedef struct {
	char			ChunkID[4];		//"RIFF"
	unsigned int	ChunkSize;		//F0 F1 F2 F3
	char			Format[4];		//"WAVE"
	char			SubChunk1ID[4];	//"fmt "
	unsigned int	SubChunk1Size;	//0x14 0x0 0x0 0x0
	unsigned short	AudioFormat;		//0x11 for IMA-ADPCM
	unsigned short	NumOfChannels1;	//0x1 for mono, 0x2 for stereo
	unsigned int	SampleRate;		//0x1F40 for 8kHz
	unsigned int	ByteRate;		//0xFD7 for 8kHz mono
	unsigned short	BlockAlign;		//256 for mono, 512 for stereo
	unsigned short	BitsPerSample;	//4-bit ADPCM
	unsigned short	ByteExtraData;	//0x2
	unsigned short	ExtraData;		//samples per block (505)
	char			SubChunk2ID[4];	//"fact"
	unsigned int	SubChunk2Size;	//0x4
	unsigned int	NumOfChannels2;	//S0 S1 S2 S3
	char			SubChunk3ID[4];	//"data"
	unsigned int	SubChunk3Size;	//data size (file size-60)
} S_IMA_ADPCM_HEADER, *PS_IMA_ADPCM_HEADER;

typedef struct {
	unsigned short	CType;
	unsigned short	SR;
	unsigned short	BR;
	unsigned short	Ch;
	unsigned int	FrmLen;
	unsigned int	FileLen;
	unsigned short	MF;
	unsigned short	SF;
	unsigned short	MBF;
	unsigned short	PCS;
	unsigned short	Rec;
	unsigned short	HeaderLen;
	unsigned short	Audio32Type;
	unsigned short	StopCode;
	unsigned short	SHeader;
} S_SONIX_HEADER, *PS_SONIX_HEADER;

typedef struct {
	char*				pBitsBuffer;
	short*				pADCBuffer;
	short*				pTempBuffer;
	unsigned short			ADC_BL;		/*BL=Buffer Length*/
	unsigned short			ChannelBL;	/*BL=Buffer Length*/
	unsigned int 			ErasedFlashAdd;
	codec_i2s_t			eADC_I2S;
	E_BITRATE			eBitrate;
	E_CHANNEL			eChannel;
	E_MP3_MODE			eMP3Mode;
	E_SAMPLING_RATE		eSamplingRate;
	E_STATUS				eStatus;
	E_PINGPONG_BUFFER 	ePPB;
	PFN_MOVE_BITS_AWAY	pfnMoveBitsAway;
	aud32enc_t*			psA32Enc;
	SonixMP3Enc_encoder*	psMP3Enc;
} S_RECORDER, *PS_RECORDER;

typedef struct {
	char*				pBitsBuffer;
	short*				pDACBuffer;
	short*				pTempBuffer;
	unsigned short			BitsBL;
	unsigned short			DAC_BL;		/*BL=Buffer Length*/
	unsigned short			ChannelBL;	/*BL=Buffer Length*/
	codec_i2s_t			eDAC_I2S;
	E_BITRATE			eBitrate;
	E_CHANNEL			eChannel;
	E_MP3_MODE			eMP3Mode;
	E_SAMPLING_RATE		eSamplingRate;
	E_STATUS				eStatus;
	E_PINGPONG_BUFFER	ePPB;
	PFN_PLAY_FINISHED		pfnPlayFinished;
	PFN_REFILL_BUFFER		pfnRefillBuffer;
	aud32dec_t*			psA32Dec;
	SonixMP3decoder*		psMP3Dec;
} S_PLAYER, *PS_PLAYER;

codec_channel_mode_t Convert_to_Codec_Channel(E_CHANNEL eChannel);
codec_sampling_rate_t Convert_to_Codec_Sampling_Rate(E_SAMPLING_RATE eSamplingRate);
aud_samplerate_t Convert_to_Aud_Sampling_Rate(E_SAMPLING_RATE eSamplingRate);
E_ERROR_CODE External_Audio_Codec_Init(E_EXTERNAL_AUDIO_CHIP eExternalAudioChip, sn_i2c_sw_ch_t eSN_I2C_CH, E_SAMPLING_RATE eSamplingRate);


void MP3Rec_Task(void* pvParameters);
void MP3Play_Task(void* pvParameters);


#ifdef __cplusplus
}
#endif

#endif

