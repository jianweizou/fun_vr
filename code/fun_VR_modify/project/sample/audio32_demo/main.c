#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snc_audio.h"
#include "snc_codec.h"
#include "snc_gpio.h"
#include "snc_i2s.h"
#include "snc_i2s4.h"
#include "snc_ipc.h"
#include "snc_types.h"
#include "audio32codec_encoder.h"
#include "audio32codec_decoder.h"
#include "I2C_Software_Master.h"
#include "../../../sdk/share/audio/aud_driver.h"

extern void A32_32K_1_START(void);
extern void A32_32K_1_END(void);
extern void A32_32K_2_START(void);
extern void A32_32K_2_END(void);
extern void A32_32K_3_START(void);
extern void A32_32K_3_END(void);
extern void A32_32K_4_START(void);
extern void A32_32K_4_END(void);
extern void A32_32K_5_START(void);
extern void A32_32K_5_END(void);
extern void A32_24K_1_START(void);
extern void A32_24K_1_END(void);
extern void A32_24K_2_START(void);
extern void A32_24K_2_END(void);
extern void A32_24K_3_START(void);
extern void A32_24K_3_END(void);
extern void A32_24K_4_START(void);
extern void A32_24K_4_END(void);
extern void A32_24K_5_START(void);
extern void A32_24K_5_END(void);
extern void A32_16K_1_START(void);
extern void A32_16K_1_END(void);
extern void A32_16K_2_START(void);
extern void A32_16K_2_END(void);
extern void A32_16K_3_START(void);
extern void A32_16K_3_END(void);
extern void A32_16K_4_START(void);
extern void A32_16K_4_END(void);
extern void A32_16K_5_START(void);
extern void A32_16K_5_END(void);

#define	CORE1_ROM
#define	SR16kHz	16000

#define	CODEC_BL	640	/* BL = Buffer Length */
short		codec_buffer[CODEC_BL] = {0};

#define	BITS_BL	6000		/* BL = Buffer Length */
short		bits_buffer[BITS_BL] = {0};

unsigned short get_a_frame_length(unsigned int BitRate)
{
	if (BitRate == 32000)
		return 40;
	else if (BitRate == 24000)
		return 30;
	else if (BitRate == 16000)
		return 20;
	else
		return 0;
}

int record_A32(short* fpCurrent, short* fpEnd, unsigned int BitRate, unsigned int* FrmCnt)
{
	unsigned int index;
	E_PINGPONG_BUFFER ePPB;
#ifdef	CORE1_ROM
	CMD_ENT *ptr;
#endif
	aud32enc_t *psA32enc;

	if ((psA32enc = (aud32enc_t*)calloc(1, sizeof(aud32enc_t))) == NULL)
		return 2;

#ifdef	CORE1_ROM
	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_AU32_ENC_INIT;
	ptr->PARA1 = (int)psA32enc;
	ptr->PARA2 = (int)SR16kHz;
	ptr->PARA3 = (int)BitRate;
	SEND_CMD();
	while(CMD_DONE != CMD_SENT);
	if (ptr->RET)
		return 4;
#endif
	
	ADC_Enable(CODEC_SN_0, CODEC_AUD, codec_buffer, CODEC_BL, CODEC_BL, CODEC_16kHz, CODEC_MONO);
	*FrmCnt = 0;
	ePPB = ePPB0;

	while(fpCurrent < fpEnd)
	{
		index = ADC_Get_Rx_Index(CODEC_SN_0);
		
		if ((index<CODEC_BL/2)&&(ePPB==ePPB1))
		{
#ifdef	CORE1_ROM
			ptr = GET_CMD_ENT_ADD();
			ptr->CMD = E_AU32_ENC_FRM;
			ptr->PARA1 = (int)psA32enc;
			ptr->PARA2 = (int)(codec_buffer+CODEC_BL/2);
			ptr->PARA3 = (int)fpCurrent;
			SEND_CMD();
			//while(CMD_DONE != CMD_SENT);
#endif
			fpCurrent += get_a_frame_length(BitRate);
			(*FrmCnt)++;
			ePPB = ePPB0;
		}
		else if ((index>=CODEC_BL/2)&&(ePPB==ePPB0))
		{
#ifdef	CORE1_ROM
			ptr = GET_CMD_ENT_ADD();
			ptr->CMD = E_AU32_ENC_FRM;
			ptr->PARA1 = (int)psA32enc;
			ptr->PARA2 = (int)codec_buffer;
			ptr->PARA3 = (int)fpCurrent;
			SEND_CMD();
			//while(CMD_DONE != CMD_SENT);
#endif
			fpCurrent += get_a_frame_length(BitRate);
			(*FrmCnt)++;
			ePPB = ePPB1;
		}
	}

	ADC_Disable(CODEC_SN_0);
	memset(codec_buffer, 0, CODEC_BL*2);
	free(psA32enc);
	
	return 0;
}

int play_A32(short* fpCurrent, short* fpEnd, unsigned int BitRate, unsigned int* FrmCnt)
{
	unsigned int index;
	E_PINGPONG_BUFFER ePPB;
#ifdef	CORE1_ROM
	CMD_ENT *ptr;
#endif
	aud32dec_t *psA32dec;
	
	if ((psA32dec = (aud32dec_t*)calloc(1, sizeof(aud32dec_t))) == NULL)
		return 2;

#ifdef	CORE1_ROM
	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_AU32_DEC_INIT;
	ptr->PARA1 = (int)psA32dec;
	ptr->PARA2 = (int)SR16kHz;
	ptr->PARA3 = (int)BitRate;
	SEND_CMD();
	while(CMD_DONE != CMD_SENT);
	if (ptr->RET)
		return 4;
#endif

	DAC_Enable(CODEC_SN_2, CODEC_AUD, codec_buffer, CODEC_BL, CODEC_BL, CODEC_16kHz, CODEC_MONO);
	*FrmCnt = 0;
	ePPB = ePPB1;
	
	while(fpCurrent < fpEnd)
	{
		index = DAC_Get_Tx_Index(CODEC_SN_2);
		
		if ((index<CODEC_BL/2)&&(ePPB==ePPB1))
		{
#ifdef	CORE1_ROM
			ptr = GET_CMD_ENT_ADD();
			ptr->CMD = E_AU32_DEC_FRM;
			ptr->PARA1 = (int)psA32dec;
			ptr->PARA2 = (int)fpCurrent;
			ptr->PARA3 = (int)(codec_buffer+CODEC_BL/2);
			SEND_CMD();
			//while(CMD_DONE != CMD_SENT);
#endif
			fpCurrent += get_a_frame_length(BitRate);
			(*FrmCnt)++;
			ePPB = ePPB0;
		}
		else if ((index>=CODEC_BL/2)&&(ePPB==ePPB0))
		{
#ifdef	CORE1_ROM
			ptr = GET_CMD_ENT_ADD();
			ptr->CMD = E_AU32_DEC_FRM;
			ptr->PARA1 = (int)psA32dec;
			ptr->PARA2 = (int)fpCurrent;
			ptr->PARA3 = (int)codec_buffer;
			SEND_CMD();
			//while(CMD_DONE != CMD_SENT);
#endif
			fpCurrent += get_a_frame_length(BitRate);
			(*FrmCnt)++;
			ePPB = ePPB1;
		}
	}

	DAC_Disable(CODEC_SN_2);
	memset(codec_buffer, 0, CODEC_BL*2);
	free(psA32dec);
	
	return 0;
}

int main (void)
{
	unsigned int FrmCnt = 0;
	
	I2C_SoftWare_Master_Init();
	I2S4_Init();

	while(1)
	{
#if 1
		if (play_A32((short*)A32_32K_1_START, (short*)A32_32K_1_END, 32000, &FrmCnt))
			return 1;//0x143
		if (play_A32((short*)A32_32K_2_START, (short*)A32_32K_2_END, 32000, &FrmCnt))
			return 2;//0x735
		if (play_A32((short*)A32_32K_3_START, (short*)A32_32K_3_END, 32000, &FrmCnt))
			return 3;//0x70
		if (play_A32((short*)A32_32K_4_START, (short*)A32_32K_4_END, 32000, &FrmCnt))
			return 4;//0xC7
		if (play_A32((short*)A32_32K_5_START, (short*)A32_32K_5_END, 32000, &FrmCnt))
			return 5;//0xAE4

		if (play_A32((short*)A32_24K_1_START, (short*)A32_24K_1_END, 24000, &FrmCnt))
			return 1;//0x143
		if (play_A32((short*)A32_24K_2_START, (short*)A32_24K_2_END, 24000, &FrmCnt))
			return 2;//0x735
		if (play_A32((short*)A32_24K_3_START, (short*)A32_24K_3_END, 24000, &FrmCnt))
			return 3;//0x70
		if (play_A32((short*)A32_24K_4_START, (short*)A32_24K_4_END, 24000, &FrmCnt))
			return 4;//0xC7
		if (play_A32((short*)A32_24K_5_START, (short*)A32_24K_5_END, 24000, &FrmCnt))
			return 5;//0xAE4

		if (play_A32((short*)A32_16K_1_START, (short*)A32_16K_1_END, 16000, &FrmCnt))
			return 1;//0x143
		if (play_A32((short*)A32_16K_2_START, (short*)A32_16K_2_END, 16000, &FrmCnt))
			return 2;//0x735
		if (play_A32((short*)A32_16K_3_START, (short*)A32_16K_3_END, 16000, &FrmCnt))
			return 3;//0x70
		if (play_A32((short*)A32_16K_4_START, (short*)A32_16K_4_END, 16000, &FrmCnt))
			return 4;//0xC7
		if (play_A32((short*)A32_16K_5_START, (short*)A32_16K_5_END, 16000, &FrmCnt))
			return 5;//0xAE4
#else
		record_A32(bits_buffer, &bits_buffer[BITS_BL], 32000, &FrmCnt);
		play_A32(bits_buffer, &bits_buffer[BITS_BL], 32000, &FrmCnt);
#endif
	}
}

