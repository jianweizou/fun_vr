#ifndef SNX_FFT_TABLE_H
#define SNX_FFT_TABLE_H

#define FFT_TAB_SIN_SIZE_BITS		11
#define FFT_TAB_SIN_SIZE_FULL		(1 << FFT_TAB_SIN_SIZE_BITS)
#define FFT_TAB_SIN_SIZE_1_2		(FFT_TAB_SIN_SIZE_FULL * 1 / 2)
#define FFT_TAB_SIN_SIZE_1_4		(FFT_TAB_SIN_SIZE_FULL * 1 / 4)
#define FFT_TAB_BITREV_SIZE_BITS	10
#define FFT_TAB_BITREV_SIZE_FULL	(1 << FFT_TAB_BITREV_SIZE_BITS)

#ifdef __cplusplus 
extern "C" {
#endif

//long lrintf(double x);
//short fixed(float x, int bits);

//void gen_fft_tab_radix2_twiddle(void);
extern const short fft_tab_radix2_twiddle[FFT_TAB_SIN_SIZE_FULL];
extern const short fft_tab_radix2_twiddle_q15[FFT_TAB_SIN_SIZE_FULL];

//void gen_fft_tab_radix4_twiddle(void);
extern const short ifft_tab_radix4_twiddle[486];
extern const short fft_tab_radix4_twiddle_q31[486];
extern const short fft_tab_radix4_twiddle_q15[486];

extern const short fft_tab_bit_reversal[FFT_TAB_BITREV_SIZE_FULL];

//void gen_fft_tab_bit_reversal_dsp(short n);

#ifdef __cplusplus 
}
#endif

#endif
