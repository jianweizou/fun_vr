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

#define MAX_FRAMESIZE   320
#define MAX_NUMBER_OF_REGIONS   16//32//28
#define MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES 16
#define MAX_BITS_PER_FRAME 960
/***************************************************************************/
/* Type definitions                                                        */
/***************************************************************************/
#ifndef AUDIO32CODEC_ENCODER_H
#define AUDIO32CODEC_ENCODER_H
/* main structure for audio32 encoder */
typedef struct 
{
    long  bit_rate;
    short  samplerate;//bandwidth;
    short  number_of_bits_per_frame;
    short  number_of_regions;
	short  outbytes;//outwords;//輸出的bitstream size
	short  history[MAX_FRAMESIZE];//塞到struct裡面
	//short dct_buffer_temp[40];
    short  absolute_region_power_index[MAX_NUMBER_OF_REGIONS];
    short  power_categories[MAX_NUMBER_OF_REGIONS];
    short  category_balances[MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES-1];
    short  drp_num_bits[MAX_NUMBER_OF_REGIONS+1];
    unsigned short drp_code_bits[MAX_NUMBER_OF_REGIONS+1];
    short  region_mlt_bit_counts[MAX_NUMBER_OF_REGIONS];
    unsigned int region_mlt_bits[4*MAX_NUMBER_OF_REGIONS];
	short differential_region_power_index[MAX_NUMBER_OF_REGIONS];
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
	short mlt_coefs[320];
	//short buffer_a[320], buffer_b[320], buffer_c[320]; //SAVE WORKING RAM
} aud32enc_t;
#endif

int aud32enc_initialise(aud32enc_t* encoder_control, int freq, int bitr/*, int regions*/);
int aud32enc_encode(aud32enc_t* encoder_control, short* input, short* output);

