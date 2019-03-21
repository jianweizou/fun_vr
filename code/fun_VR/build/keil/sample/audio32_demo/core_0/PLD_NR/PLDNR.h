
#define FRAME_SIZE 512
#define FRAME_STEP 256
#define FFT_SIZE 512
#define HALF_FFT_SIZE FFT_SIZE/2


#ifndef PLDNOISEREDUCE_H
#define PLDNOISEREDUCE_H
typedef struct
{
	short  PSNR_16, CM_Mean_16, SpeechProb_16;
	int	   Mic1TotalPower_32;
	short  FNLP_16;
	int	   extendcnt;

	short  fftMic1_16[(HALF_FFT_SIZE + 1) * 2], fftMic2_16[(HALF_FFT_SIZE + 1) * 2]; // 514*2
	int    psdMic1_32[(HALF_FFT_SIZE + 1)], psdMic2_32[(HALF_FFT_SIZE + 1)]; // 257*2*2
	int	   phi11_32[HALF_FFT_SIZE + 1], phi22_32[HALF_FFT_SIZE + 1], phinn_32[HALF_FFT_SIZE + 1];//257*3*2
	int	   phi12_32[2][HALF_FFT_SIZE + 1];//257*2*2

	short  phi12_16[2][HALF_FFT_SIZE + 1];//257*2

	int	   phiPLD_32[HALF_FFT_SIZE + 1];//257*2
	short  AUSF_16[HALF_FFT_SIZE + 1];//257
	short  G_CM_16[HALF_FFT_SIZE + 1];//257
	short  G_PLD_16[HALF_FFT_SIZE + 1];//257
	short  G_MAX_16[HALF_FFT_SIZE + 1];//257
	short  G_MIN_16[HALF_FFT_SIZE + 1];//257
	short  GAINF_16[HALF_FFT_SIZE + 1];//257


	//int gFFTBuf32_1[FFT_SIZE * 2];
	//int fft_time1_PLD[FFT_SIZE * 2];
	//int gFFTBuf32_2[FFT_SIZE * 2];
	//int fft_time2_PLD[FFT_SIZE * 2];
	//int fft_time0_Out[FFT_SIZE];
	


}pld_nr_t;
#endif

void PLDNoiseInit(pld_nr_t* pldnr_control, int sampleRate);
int PLDNoiseReduce(pld_nr_t* pldnr_control, int *g_fft_buf_q31_1, int *g_fft_buf_q31_2, short *GAINF_16_T);
//void PLDNoiseInit(short sampleRate);
//int PLDNoiseReduce(double inBuffer[],double outBuffer[]);
//int PLDNoiseReduce(short inBuffer[], short outBuffer[]);
//void PLDNoiseInit(pld_nr_t* pldnr_control, short sampleRate);
//int PLDNoiseReduce(pld_nr_t* pldnr_control, short inBuffer[], short outBuffer[]);

