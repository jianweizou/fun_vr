
#include "audio32codec_decoder.h"
#include "audio32codec_encoder.h"
#include "mp3enc_encoder.h"
#include "mp3dec_decoder.h"
#include "ima_codec.h"

int A32Enc_Init(aud32enc_t *encoder_control, int freq, int bitr, int wait_till_done);
int A32Enc_EncodeFrame(aud32enc_t *encoder_control, short *input, short *out_words, int wait_till_done);
int A32Dec_Init(aud32dec_t *decoder_control, int freq, int bitr, int wait_till_done);
int A32Dec_DecodeFrame(aud32dec_t *decoder_control, short *input, short *out_words, int wait_till_done);

int IMAEnc_Init(SonixIMAcodec *pSonixIMAcodec, short frmsizeT, int wait_till_done);
int IMAEnc_EncodeFrame(SonixIMAcodec *pSonixIMAcodec, short *frmbuf, short *packedbuf, int wait_till_done);
void IMADec_Init(SonixIMAcodec *pSonixIMAcodec, int packwords, int wait_till_done);
int IMADec_DecodeFrame(SonixIMAcodec *pSonixIMAcodec, short *packwordsbuf, short *outputbuf, int wait_till_done);

int MP3Enc_Init(SonixMP3Enc_encoder *s, int freq, int bitr, int stereo_mode, int emph, int copyright, int original, int interleave, int outbuf_size, int wait_till_done);
int MP3Enc_EncodeFrame(SonixMP3Enc_encoder *config, short *data, char *outbuf, int wait_till_done);
void MP3Enc_Flush(SonixMP3Enc_encoder *s, int wait_till_done);
void MP3Dec_Init(SonixMP3decoder *pSonixMP3decoder, short interleave, char *inbufh, char *inbuft, int recover, int sram, int wait_till_done);
int MP3Dec_DecodeFrame(SonixMP3decoder *pSonixMP3decoder, char *MP3InBuf, short *MP3OutBuf, int *used_bytes, int wait_till_done);

