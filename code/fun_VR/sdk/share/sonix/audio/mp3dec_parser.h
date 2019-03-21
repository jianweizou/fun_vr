#ifdef _MSC_VER
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <tchar.h>
#else
#include "ff.h"
#include "audio.h"
#endif


//#include "mp3dec_decoder.h" //for MP3Dec_malloc_mem() and MP3Dec_free_mem() only


#ifndef MP3DEC_PARSER_NEW_H
#define MP3DEC_PARSER_NEW_H

#define MP3_INBUF_MIN_CNT				72

    /* 7.18.1.1 */

    /* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;
typedef   signed       __int64 int64_t;

    /* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;
typedef unsigned       __int64 uint64_t;

typedef struct
{
	long DSP_MP3ID3Size;
	long DSP_MP3QueueData2;
	short DSP_MP3pxgv5;
	short DSP_MP3pxgv6;
	short DSP_MP3_BitstreamEndFlag;
	short DSP_Flag_MP3;
	short DSP_MP3_BitS_Flag;
	short DSP_MP3_Decoded_Bits;
	short DSP_MP3_LowSamp_Flag;
	short DSP_MP3Pre_LowSamp_Flag;	
	short DSP_MP3hdr_bitrate_index;
	short DSP_MP3hdr_copyright;
	short DSP_MP3hdr_emphasis;
	short DSP_MP3hdr_error_protection;
	short DSP_MP3hdr_extension;
	short DSP_MP3hdr_IDex;
	short DSP_MP3hdr_lay;
	short DSP_MP3hdr_mode;
	short DSP_MP3hdr_mode_ext;
	short DSP_MP3hdr_original;
	short DSP_MP3hdr_padding;
	short DSP_MP3hdr_sampling_frequency;
	short DSP_MP3hdr_version;
	short DSP_MP3BitstreamValid;
	short DSP_MP3SeekCount;
	short DSP_MP3SubFrmCnt;
	short DSP_MP3hdr_stereo;
	short DSP_MP3_Format;
	short DSP_MP3Prehdr_lay;
	short DSP_MP3Prehdr_sampling_frequency;
	short DSP_MP3Prehdr_stereo;
	short DSP_MP3Prehdr_version;
	short DSP_MP3HeaderFlag;
	short DSP_MP3PreSlots;
	short DSP_MP3Slots;

	//bitstream related
	short bitstream_mode;//0 for sd card
	char* inbuf;
	char* pread;
	int buffer_length;
	int remaining_bytes;
#ifdef _MSC_VER
	FILE *pfile;
#else
	S_FILE *pfile;
	//PFN_REFILL_BUFFER	pfn_refill_buffer;
	PFN_MP3_RESET_BUFFER	pfn_mp3_reset_buffer;
#endif


} DSP_MP3_Content;
void MP3_SetPxgv5Pxgv6(DSP_MP3_Content *s, short pxgv5, short pxgv6);
void MP3_Clr_Decoded_Bits(DSP_MP3_Content *s);
void MP3_End(DSP_MP3_Content *s);
__inline short ExchangeHiLowBytes(short x);
static short UnpackBitstream(DSP_MP3_Content *s, short y1);
short MP3_CalL2Bytes(DSP_MP3_Content *s);
static short MP3_CalL3Bytes(DSP_MP3_Content *s);
static void Main_Data_Slots_dsp(DSP_MP3_Content *s);
static long MP3_CalBitstreamSize(DSP_MP3_Content *s);
static void MP3_UpdateHeader(DSP_MP3_Content *s);
static void Decode_Info_dsp(DSP_MP3_Content *s);
static short MP3_CurHeaderisValid(DSP_MP3_Content *s);
static void Seek_Sync_dsp(DSP_MP3_Content *s);
static void MP3_GetHeader(DSP_MP3_Content *s);
static long MP3_GetBegOffset(DSP_MP3_Content *s);
static void MP3_CalBmpHSize(DSP_MP3_Content *s);
static void MP3_CalRiffHSize(DSP_MP3_Content *s);
static void MP3_CalID3HSize(DSP_MP3_Content *s);
static short MP3_SeekBegPos(DSP_MP3_Content *s);
static void DSP_MP3_Content_initial(DSP_MP3_Content *s);


long MP3_Get_StartOffset(DSP_MP3_Content *s);
#ifdef _MSC_VER
int MP3_set_source_SD(DSP_MP3_Content *s, FILE *pfile, char* inbuf, int buffer_length);
#else
int MP3_set_source_SD(DSP_MP3_Content *s, char* inbuf, int buffer_length, PFN_MP3_RESET_BUFFER pfn_mp3_reset_buffer);
#endif

short MP3_Get_Channel(DSP_MP3_Content *s);
int MP3_Get_SampleRate(DSP_MP3_Content *s);
int MP3_Get_Samples(DSP_MP3_Content *s);
short MP3_Get_BitRate(DSP_MP3_Content *s);

int MP3Parser_Seek_Syncword(char* inbufh, char* inbuft, char** inbufp, int* used_bytes, int* valid_bytes_inbuf, short clean);//RAM

#endif




