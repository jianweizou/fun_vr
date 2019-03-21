#ifndef SNX_FFT_H
#define SNX_FFT_H

#include "snx_fft_table.h"

#ifdef __cplusplus 
extern "C" {
#endif

enum RDFTransformType
{
	DFT_R2C,
	IDFT_C2R,
	IDFT_R2C,
	DFT_C2R,
};

/* Q15 ---------------------------------------------------------------------------*/

typedef short FFTSampleQ15;
typedef struct
{
	FFTSampleQ15 re, im;

} FFTComplexQ15;

typedef struct
{
	short inverse;
	short nbits;
	const FFTSampleQ15 *tab_r4;	// radix 4 twiddle factor
	const FFTSampleQ15 *tab_r2;	// radix 2 twiddle factor
	short revfactor;
	const short *tab_rev;		// bit reversal table

	void (*R2_BF_0_Q15)(
		FFTSampleQ15 *pre, FFTSampleQ15 *pim, 
		FFTSampleQ15 *qre, FFTSampleQ15 *qim, 
		FFTSampleQ15 bx, FFTSampleQ15 by, 
		FFTSampleQ15 ax, FFTSampleQ15 ay);

	void (*R2_BF_N_Q15)(
		FFTSampleQ15 *pre, FFTSampleQ15 *pim, 
		FFTSampleQ15 *qre, FFTSampleQ15 *qim, 
		FFTSampleQ15 bx, FFTSampleQ15 by, 
		FFTSampleQ15 ax, FFTSampleQ15 ay);

	void (*R4_BF_0_Q15)(
		FFTComplexQ15 *b0, FFTComplexQ15 *b1, 
		FFTComplexQ15 *b2, FFTComplexQ15 *b3);

	void (*R4_BF_N_Q15)(
		FFTComplexQ15 *b0, FFTComplexQ15 *b1, 
		FFTComplexQ15 *b2, FFTComplexQ15 *b3);

} FFTContextQ15;

short snx_fft_init_q15(FFTContextQ15 *s, short nbits, short inverse);
void snx_fft_q15(FFTContextQ15 *s, FFTComplexQ15 *z);
void snx_fft_bit_reversal_q15(FFTContextQ15 *s, FFTSampleQ15 *src);

typedef struct
{
	short inverse;
	short nbits;
	short sign_convention;	
	short tab_step;
	const FFTSampleQ15 *tab_r2;	// radix 2 twiddle factor
	FFTContextQ15 fft;

} RDFTContextQ15;

short snx_rdft_init_q15(RDFTContextQ15 *s, short nbits, enum RDFTransformType trans);
void snx_rdft_q15(RDFTContextQ15 *s, FFTSampleQ15 *data);

/* Q31 ---------------------------------------------------------------------------*/

#define ENABLE_HIGH_QUALITY			0
#define ENABLE_DSP_FAST_OPERATION	1  /* only faster in DSP7001A/DSP7300 */

typedef int FFTSampleQ31;
typedef struct
{
	FFTSampleQ31 re, im;

} FFTComplexQ31;

typedef struct
{
	short inverse;
	short nbits;
	const FFTSampleQ15 *tab_r4;	// radix 4 twiddle factor
	const FFTSampleQ15 *tab_r2;	// radix 2 twiddle factor
	short revfactor;
	const short *tab_rev;

	void (*R4_BF_Q31)(
		FFTComplexQ31 *b0, FFTComplexQ31 *b1, 
		FFTComplexQ31 *b2, FFTComplexQ31 *b3);

} FFTContextQ31;

short snx_fft_init_q31(FFTContextQ31 *s, short nbits, short inverse);
void snx_fft_q31(FFTContextQ31 *s, FFTComplexQ31 *z);
void snx_fft_bit_reversal_q31(FFTContextQ31 *s, FFTSampleQ31 *src);

typedef struct
{
	short inverse;
	short nbits;
	short sign_convention;
	short tab_step;	
	const FFTSampleQ15 *tab_r2;	// radix 2 twiddle factor
	FFTContextQ31 fft;

} RDFTContextQ31;

short snx_rdft_init_q31(RDFTContextQ31 *s, short nbits, enum RDFTransformType trans);
void snx_rdft_q31(RDFTContextQ31 *s, FFTSampleQ31 *data);

#ifdef __cplusplus 
}
#endif

#endif

