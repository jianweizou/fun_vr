//#include <stdio.h>
//#include <math.h>
//#include <stdlib.h>
//#include "typedef.h"
//#include "basop32.h"
/*
typedef signed char Word8;
typedef short Word16;
typedef long Word32;
typedef int Flag;
*/

#define MAX_DCT_LENGTH      320
#define MAX_NUMBER_OF_REGIONS   16//32//28
#define MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES 16
#define MAX_BITS_PER_FRAME 960
/***************************************************************************/
/* Type definitions                                                        */
/***************************************************************************/
#ifndef AUDIO32CODEC_DECODER_H
#define AUDIO32CODEC_DECODER_H
typedef struct
{
    short code_bit_count;      /* bit count of the current word */
    short current_word;        /* current word in the bitstream being processed */
    short *code_word_ptr;      /* pointer to the bitstream */
    short number_of_bits_left; /* number of bits left in the current word */
    short next_bit;            /* next bit in the current word */
}Bit_Obj;

typedef struct
{
    short seed0;
    short seed1;
    short seed2;
    short seed3;
}Rand_Obj;

/* main structure for audio32 decoder */
typedef struct 
{
    long  bit_rate;
    short  samplerate;//bandwidth;
    short  number_of_bits_per_frame;
    short  number_of_regions;

	short  inbytes;//inwords;//輸入的bitstream size

    short old_samples[MAX_DCT_LENGTH>>1];

	short decoder_mlt_coefs[MAX_DCT_LENGTH];//看能不能省, 要改rmlt


    Bit_Obj bitobj;
    Rand_Obj randobj;

	//for common.c
	int MinPow_Idx_Categories;
	int MinPow_Val_Categories;
	int EnhanceLowFreqMode_Flag;
	int Acc_T;// = 0;
	int Min,MinIdx;
	int absolute_region_power_index_frmdiff[MAX_NUMBER_OF_REGIONS];// = {0};
	int absolute_region_power_index_pre[MAX_NUMBER_OF_REGIONS];// = {0};
	short max_rate_categories[MAX_NUMBER_OF_REGIONS];
	short min_rate_categories[MAX_NUMBER_OF_REGIONS];
	short temp_category_balances[2*MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES];	
	short power_cats[MAX_NUMBER_OF_REGIONS];

	//decoder()
    short  absolute_region_power_index[MAX_NUMBER_OF_REGIONS];
    short  decoder_power_categories[MAX_NUMBER_OF_REGIONS];
    short  decoder_category_balances[MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES-1];
    short  decoder_region_standard_deviation[MAX_NUMBER_OF_REGIONS];

	//decode_envelope()
	short differential_region_power_index[MAX_NUMBER_OF_REGIONS];

	//dct_type_iv_s()
	//short dct_buffer_temp[40];
	short	new_samples[320];
	//short buffer_a[320], buffer_b[320], buffer_c[320];
	//rmlt_coefs_to_samples()
	//Word16	new_samples[MAX_DCT_LENGTH];//之後看能不能省掉
} aud32dec_t;
#endif

int aud32dec_initialise(aud32dec_t* decoder_control, int freq, int bitr/*, int regions*/);
int aud32dec_decode(aud32dec_t* decoder_control, short* input, short* output);
