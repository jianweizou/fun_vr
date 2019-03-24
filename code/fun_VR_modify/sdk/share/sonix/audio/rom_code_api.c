#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "snc_ipc.h"
#include "audio32codec_decoder.h"
#include "audio32codec_encoder.h"
#include "mp3enc_encoder.h"
#include "mp3dec_decoder.h"
#include "ima_codec.h"
#include "rom_code_api.h"

#define	CORE1_ROM_A32_ENC
#define	CORE1_ROM_A32_DEC
#define	CORE1_ROM_IMA_ENC
#define	CORE1_ROM_IMA_DEC
#define	CORE1_ROM_MP3_ENC
#define	CORE1_ROM_MP3_DEC

static void Waiting(void)
{
	while(CMD_DONE != CMD_SENT);
}

int A32Enc_Init(aud32enc_t *encoder_control, int freq, int bitr, int wait_till_done)
{
#ifdef	CORE1_ROM_A32_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_AU32_ENC_INIT;
	ptr->PARA1 = (int)encoder_control;
	ptr->PARA2 = (int)freq;
	ptr->PARA3 = (int)bitr;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return aud32enc_initialise(encoder_control, freq, bitr);
#endif
}

int A32Enc_EncodeFrame(aud32enc_t *encoder_control, short *input, short *out_words, int wait_till_done)
{
#ifdef	CORE1_ROM_A32_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_AU32_ENC_FRM;
	ptr->PARA1 = (int)encoder_control;
	ptr->PARA2 = (int)input;
	ptr->PARA3 = (int)out_words;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return aud32enc_encode(encoder_control, input, out_words);
#endif
}

int A32Dec_Init(aud32dec_t *decoder_control, int freq, int bitr, int wait_till_done)
{
#ifdef	CORE1_ROM_A32_DEC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_AU32_DEC_INIT;
	ptr->PARA1 = (int)decoder_control;
	ptr->PARA2 = (int)freq;
	ptr->PARA3 = (int)bitr;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return aud32dec_initialise(decoder_control, freq, bitr);
#endif
}

int A32Dec_DecodeFrame(aud32dec_t *decoder_control, short *input, short *out_words, int wait_till_done)
{
#ifdef	CORE1_ROM_A32_DEC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_AU32_DEC_FRM;
	ptr->PARA1 = (int)decoder_control;
	ptr->PARA2 = (int)input;
	ptr->PARA3 = (int)out_words;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return aud32dec_decode(decoder_control, input, out_words);
#endif
}

int IMAEnc_Init(SonixIMAcodec *pSonixIMAcodec, short frmsizeT, int wait_till_done)
{
#ifdef	CORE1_ROM_IMA_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_IMA_ENC_INIT;
	ptr->PARA1 = (int)pSonixIMAcodec;
	ptr->PARA2 = (int)frmsizeT;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return IMA_Encode_Init(pSonixIMAcodec, frmsizeT);
#endif
}

int IMAEnc_EncodeFrame(SonixIMAcodec *pSonixIMAcodec, short *frmbuf, short *packedbuf, int wait_till_done)
{
#ifdef	CORE1_ROM_IMA_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_IMA_ENC_FRM;
	ptr->PARA1 = (int)pSonixIMAcodec;
	ptr->PARA2 = (int)frmbuf;
	ptr->PARA3 = (int)packedbuf;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return IMA_EncodeProcess(pSonixIMAcodec, frmbuf, packedbuf);
#endif
}

void IMADec_Init(SonixIMAcodec *pSonixIMAcodec, int packwords, int wait_till_done)
{
#ifdef	CORE1_ROM_IMA_DEC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_IMA_DEC_INIT;
	ptr->PARA1 = (int)pSonixIMAcodec;
	ptr->PARA2 = (int)packwords;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
#else
	IMA_Decode_Init(pSonixIMAcodec, packwords);
#endif
}

int IMADec_DecodeFrame(SonixIMAcodec *pSonixIMAcodec, short *packwordsbuf, short *outputbuf, int wait_till_done)
{
#ifdef	CORE1_ROM_IMA_DEC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_IMA_DEC_FRM;
	ptr->PARA1 = (int)pSonixIMAcodec;
	ptr->PARA2 = (int)packwordsbuf;
	ptr->PARA3 = (int)outputbuf;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return IMA_DecodeProcess(pSonixIMAcodec, packwordsbuf, outputbuf);
#endif
}

int MP3Enc_Init(SonixMP3Enc_encoder *s, int freq, int bitr, int stereo_mode, int emph, int copyright, int original, int interleave, int outbuf_size, int wait_till_done)
{
#ifdef	CORE1_ROM_MP3_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_MP3_ENC_INIT;
	ptr->PARA1 = (int)s;
	ptr->PARA2 = (int)freq;
	ptr->PARA3 = (int)bitr;
	ptr->PARA4 = (int)stereo_mode;
	ptr->PARA5 = (int)emph;
	ptr->PARA6 = (int)copyright;
	ptr->PARA7 = (int)original;
	ptr->PARA8 = (int)interleave;
	ptr->PARA9 = (int)outbuf_size;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return MP3Enc_initialise_all(s, freq, bitr, stereo_mode, emph, copyright, original, interleave, outbuf_size);
#endif
}

int MP3Enc_EncodeFrame(SonixMP3Enc_encoder *config, short *data, char *outbuf, int wait_till_done)
{
#ifdef	CORE1_ROM_MP3_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_MP3_ENC_FRM;
	ptr->PARA1 = (int)config;
	ptr->PARA2 = (int)data;
	ptr->PARA3 = (int)outbuf;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return MP3Enc_encode(config, data, (unsigned char*)outbuf);
#endif
}

void MP3Enc_Flush(SonixMP3Enc_encoder *s, int wait_till_done)
{
#ifdef	CORE1_ROM_MP3_ENC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_MP3_ENC_FLUSH;
	ptr->PARA1 = (int)s;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
#else
	MP3Enc_flush(s);
#endif
}

void MP3Dec_Init(SonixMP3decoder *pSonixMP3decoder, short interleave, char *inbufh, char *inbuft, int recover, int sram, int wait_till_done)
{
#ifdef	CORE1_ROM_MP3_DEC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_MP3_DEC_INIT;
	ptr->PARA1 = (int)pSonixMP3decoder;
	ptr->PARA2 = (int)interleave;
	ptr->PARA3 = (int)inbufh;
	ptr->PARA4 = (int)inbuft;
	ptr->PARA5 = (int)recover;
	ptr->PARA6 = (int)sram;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
#else
	MP3Dec_initialise_struct(pSonixMP3decoder, interleave, inbufh, inbuft, recover, sram);
#endif
}

int MP3Dec_DecodeFrame(SonixMP3decoder *pSonixMP3decoder, char *MP3InBuf, short *MP3OutBuf, int *used_bytes, int wait_till_done)
{
#ifdef	CORE1_ROM_MP3_DEC
	CMD_ENT *ptr;

	ptr = GET_CMD_ENT_ADD();
	ptr->CMD = E_MP3_DEC_FRM;
	ptr->PARA1 = (int)pSonixMP3decoder;
	ptr->PARA2 = (int)MP3InBuf;
	ptr->PARA3 = (int)MP3OutBuf;
	ptr->PARA4 = (int)used_bytes;
	SEND_CMD();
	if (wait_till_done)
		Waiting();
	return ptr->RET;
#else
	return MP3Dec_decode(pSonixMP3decoder, MP3InBuf, MP3OutBuf, used_bytes);
#endif
}

