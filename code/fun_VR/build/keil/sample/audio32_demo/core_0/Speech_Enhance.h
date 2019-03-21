
#define FRAME_SIZE 512
#define FRAME_STEP 256
#define FFT_SIZE 512
#define HALF_FFT_SIZE FFT_SIZE/2

#include "snx_fft.h"


#ifndef SPEECHENHANCE_H
#define SPEECHENHANCE_H
typedef struct
{
	RDFTContextQ31 rdft_q31_ctx;
	RDFTContextQ31 irdft_q31_ctx;

	FFTSampleQ31 g_fft_buf_q31[1024];
	FFTSampleQ31 g_fft_buf_q31_1[1024];
	FFTSampleQ31 g_fft_buf_q31_2[1024];
	FFTSampleQ31 g_fft_buf_q31_3[1024];
	FFTSampleQ31 g_fft_buf_q31_4[1024];
	FFTSampleQ31 g_fft_buf_q31_5[1024];

	short saved_output[HALF_FFT_SIZE];
	int Slr[FFT_SIZE+2];
	//float gHistogram[257][181]={0};
	short H_out[256];
	short H_NR[256];
	short H_PLDNR[256];
	//short *EQ_addr;
	
	short L_old[HALF_FFT_SIZE];
	short R_old[HALF_FFT_SIZE];
	short f_refbuf_old[HALF_FFT_SIZE];
	short MDF_out[HALF_FFT_SIZE];
	short MDF_out_old[HALF_FFT_SIZE];

	


}speech_enhance_t;
#endif


