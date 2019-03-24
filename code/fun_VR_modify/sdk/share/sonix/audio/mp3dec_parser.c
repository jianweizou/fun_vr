#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp3dec_parser.h"

#define ALIGNING		8
#define MPG_MD_MONO		3


#ifdef _MSC_VER
extern const short putmask[17];//defined in mp3dec_tables.c
#else
#if 0
extern const short putmask[17];//defined in mp3dec_tables.c
#else
const short putmask[17]=
{
	0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
#endif
#endif

enum MP3_SeekBegPosStatus{
	inbufemptystatus = 1,
	endbitstreamstatus = -1,
	syncstatus = -2,
	id3status = -3,
	riffstatus = -4,
	bmpstatus = -5
};

enum MP3_SeekSyncStatus{
	validsyncword = 1,
	invalidsyncwordskipped = -1,
	endbitstream = 0,
	cleanup = 2
};

enum MP3_Layer{
	reserved = 0,
	layer1 = 3,
	layer2 = 2,
	layer3 = 1
};

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
const short Mp3FrameSizeTable_[3][3][15] =
{
	//11.025kHz, 12kHz, 8kHz (MPEG 2.5 NON-ISO)
	0,  52, 104, 156, 208, 261, 313, 365, 417, 522, 626, 731, 835, 940, 1044,
	0,  48,  96, 144, 192, 240, 288, 336, 384, 480, 576, 672, 768, 864, 960,
	0,  72, 144, 216, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1296, 1440,
	//22.05,24,16
	0,  26,  52,  78, 104, 130, 156, 182, 208, 261, 313, 365, 417, 470, 522,
	0,  24,  48,  72,  96, 120, 144, 168, 192, 240, 288, 336, 384, 432, 480,
	0,  36,  72, 108, 144, 180, 216, 252, 288, 360, 432, 504, 576, 648, 720,
	//44.1,48,32
	0, 104, 130, 156, 182, 208, 261, 313, 365, 417, 522, 626, 731, 835, 1044,
	0,  96, 120, 144, 168, 192, 240, 288, 336, 384, 480, 576, 672, 768, 960,
	0, 144, 180, 216, 252, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1440
};

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
const short MP3_BITRATE_TABLE[2][3][15] =
{
	//for 22.05,24,16kHz,layer1,layer2,layer3
	0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256,
	0, 8,  16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160,
	0, 8,  16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160,
	//for 44.1,48,32kHz,layer1,layer2,layer3
	0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,
	0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384,
	0, 32, 40, 48, 56, 64, 80,  96, 112, 128, 160, 192, 224, 256, 320
};

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
const short MP2_FRMSIZE_TABLE[9] =
{
	26, 24, 36,
	13, 12, 18,
	6, 24, 9,
};

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
const short MP3_MASKS[17] =
{
	0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

/*_____ F U N C T I O N S ______________________________________________*/


/*
return  1: valid syncword found
return -1: valid syncword found after invalid syncword skipped
return  0: end of bitstream
*/

int MP3Parser_Seek_Syncword(char* inbufh, char* inbuft, char** inbufp_in, int* used_bytes, int* valid_bytes_inbuf_in, short clean)//RAM
{
	static short first_in = 1;
	short MP3_BitS_Flag = 0;
	static short gPrehdr_lay;
	static short gPrehdr_version;
	static short gPrehdr_sampling_frequency;
	static short gPrehdr_stereo;
	static short gPreMP3_LowSamp_Flag;
	short temp;
	short tempval;
	short val = 0;
	short flag = 0;
	short synckey = (short)0xffe0;
	short hdr_lay, header_IDex, hdr_version;
	short hdr_bitrate_index,hdr_sampling_frequency;
	short hdr_mode;
	short MP3_LowSamp_Flag;
	short hdr_stereo;
	short ret_value = 1;
	short INBUF_SIZE = inbuft - inbufh;
	int valid_bytes_inbuf;
	char* inbufp;
	//Axl	char* inbufp_in = inbufp;
	//*used_bytes = 0;

	if(clean)
	{
		first_in = 1;
		MP3_BitS_Flag = 0;
		return cleanup;
	}
	else
	{
		valid_bytes_inbuf = *valid_bytes_inbuf_in;
		inbufp =  *inbufp_in;	
	}

	//Axl	char* inbufp_in = inbufp;
	if (used_bytes != NULL)
		*used_bytes = 0;

	do{
		if((val & synckey)==synckey){//0xffex
/*
/| 12bit syncword     | 1bit ID               | 2bit layer   | 1bit protection bit    | 
/| 4bit bitrate index | 2bit samplerate index | 1bit padding | 1bit private bit       |
/| 2bit mode(channel) | 2bit mode ext | 1bit copyright | 1bit og/copy | 2bit emphasis |
AAAAAAAA AAAABCCD EEEEFFGH IIJJKLMM AAAAAAAA AAAABCCD EEEEFFGH IIJJKLMM
	A = sync word = 11111111 1111
	B = ID. 0 = MEPG-2, 1 = MPEG-1
	C = layer ID. 10 = Layer 2. (DAB uses it only)
	D = protection bit. Use 0 (with CRC) for DAB.
	E = bitrate index. From 32 kbps ~ 384 kbps.
	F = Sampling frequency. 01 = 48/24 kHz.
	G = padding = 0. No padding required
	H = private bit. Not used.	
*/
			tempval=val>>1;
			tempval=tempval & (short)0x0003;//get layer
			if(tempval != layer2 && tempval != layer3){
				goto CHK_SEEK_CORRECT_SYNC;
			}
			hdr_lay=4-tempval;

			tempval=val>>3;
			tempval=tempval & (short)0x0003;
			if(tempval==1){
				goto CHK_SEEK_CORRECT_SYNC;
			}
			tempval=val>>4;
			header_IDex=tempval & (short)0x0001;
			tempval=val>>3;
			hdr_version=tempval & (short)0x0001;
			
			if(hdr_version==0) {
				MP3_LowSamp_Flag=1;
				if(header_IDex==1) hdr_version=1;
			}
			else{
				MP3_LowSamp_Flag=0;
				hdr_version=2;
			}
			
			//unpack 8bits
			if(valid_bytes_inbuf)
			{
				temp = *inbufp++;
				if(inbufp==inbuft)
					inbufp=inbufh;
				valid_bytes_inbuf--;
			}
			else
			{
				//set up
				*valid_bytes_inbuf_in -= *used_bytes;
				*inbufp_in += *used_bytes;
				if(*inbufp_in >= inbufh+INBUF_SIZE)
					*inbufp_in -= INBUF_SIZE;
				return 0;
			}
			(*used_bytes)++;
			//unpack done
			hdr_bitrate_index=(temp>>4)&putmask[4];
			hdr_sampling_frequency=(temp>>2)&putmask[2];

			if(hdr_bitrate_index==15){//最大為14
				ret_value = -1;//沒通過, 再找
				goto CHK_SEEK_CORRECT_SYNC;
			}			

			//unpack 8bits
			if(valid_bytes_inbuf)
			{
				temp = *inbufp++;
				if(inbufp==inbuft)
					inbufp=inbufh;
				valid_bytes_inbuf--;
			}
			else
			{
				//set up
				*valid_bytes_inbuf_in -= *used_bytes;
				*inbufp_in += *used_bytes;
				if(*inbufp_in >= inbufh+INBUF_SIZE)
					*inbufp_in -= INBUF_SIZE;
				return 0;
			}
			(*used_bytes)++;
			//unpack done
			
			hdr_mode=(temp>>6)&putmask[2];
			
			if(hdr_bitrate_index==15){
				goto CHK_SEEK_CORRECT_SYNC;
			}
			if(hdr_sampling_frequency==3){
				goto CHK_SEEK_CORRECT_SYNC;
			}
			hdr_stereo=(hdr_mode == MPG_MD_MONO) ? 1 : 2;
			if(first_in){//第一次進來, 必定是對的
				first_in = 0;
				
				gPrehdr_lay=hdr_lay;
				gPrehdr_version=hdr_version;
				gPrehdr_sampling_frequency=hdr_sampling_frequency;
				//gPrehdr_mode=hdr_mode;
				gPrehdr_stereo=hdr_stereo;
				gPreMP3_LowSamp_Flag=MP3_LowSamp_Flag;
				//找到了
				flag=-1;
				//MP3_bufseek(pSonixMP3inbuf,-4);//總共檢查過16bits, 全部退回去
				(*used_bytes)-=4;

				//set up
				*valid_bytes_inbuf_in -= *used_bytes;
				*inbufp_in += *used_bytes;
				if(*inbufp_in >= inbufh+INBUF_SIZE)
					*inbufp_in -= INBUF_SIZE;

				return ret_value;
			}
			else{
				if(
				gPrehdr_lay!=hdr_lay ||
				gPrehdr_version!=hdr_version ||
				gPrehdr_sampling_frequency!=hdr_sampling_frequency ||
				//gPrehdr_mode!=hdr_mode ||
				gPrehdr_stereo!=hdr_stereo ||
				gPreMP3_LowSamp_Flag!=MP3_LowSamp_Flag
				){//沒通過, 再找
					ret_value = -1;
					goto CHK_SEEK_CORRECT_SYNC;
				}
				else{//找到了
					flag=-1;
					//MP3_bufseek(pSonixMP3inbuf,-4);//總共檢查過16bits, 全部退回去
					(*used_bytes)-=4;

					//set up
					*valid_bytes_inbuf_in -= *used_bytes;
					*inbufp_in += *used_bytes;
					if(*inbufp_in >= inbufh+INBUF_SIZE)
						*inbufp_in -= INBUF_SIZE;


					return ret_value;
				}


			}

		}


CHK_SEEK_CORRECT_SYNC:
		if(flag==0)
		{
			val <<= ALIGNING;
			//unpack 8bits
			if(valid_bytes_inbuf)
			{
				temp = *inbufp++;
				if(inbufp==inbuft)
					inbufp=inbufh;
				valid_bytes_inbuf--;
			}
			else
			{
				//set up
				*valid_bytes_inbuf_in -= *used_bytes;
				*inbufp_in += *used_bytes;
				if(*inbufp_in >= inbufh+INBUF_SIZE)
					*inbufp_in -= INBUF_SIZE;
				return 0;
			}
			(*used_bytes)++;
			//unpack done
			
			
			temp &= 0x00FF;
			val |= temp;
		}
	}
	while(flag!=-1 && (!MP3_BitS_Flag));


	return 0;
}




short GetWord(DSP_MP3_Content *s, short *out)
{
	short word;
	short ret = 1;

	if(s->remaining_bytes > 0)
	{
		word = *(short*)(s->pread);
		
		s->pread += 2;
		s->remaining_bytes -= 2;
		
		ret = 1;
		goto EXIT_SYS_GET_WORD;
	}

	ret = 0;
	word = 0;

EXIT_SYS_GET_WORD:
	*out = word;
	return ret;
}



#ifdef _MSC_VER
int MP3_set_source_SD(DSP_MP3_Content *s, FILE *pfile, char* inbuf, int buffer_length)
#else
int MP3_set_source_SD(DSP_MP3_Content *s, char* inbuf, int buffer_length, PFN_MP3_RESET_BUFFER pfn_mp3_reset_buffer)
#endif
{
/*
	//bitstream related
	short bitstream_mode;//0 for sd card
	char* inbuf;
	unsigned short buffer_length;
	FIL *pfile;
*/
	s->bitstream_mode = 0;
	s->inbuf = inbuf;
	s->pread = inbuf;
	s->buffer_length = buffer_length;
	s->remaining_bytes = 0;
#ifdef _MSC_VER
	s->pfile = pfile;
#else
	//s->pfn_refill_buffer = pfn_refill_buffer;
	s->pfn_mp3_reset_buffer = pfn_mp3_reset_buffer;
#endif
	
	return 0;
}

short GetByteCnt(DSP_MP3_Content *s)
{
/*
	從circular buffer instance撈資訊, 一樣不應該直接放在global, 而是要透過function argument存取
*/
	return s->remaining_bytes;
}
short ResetInBuf(DSP_MP3_Content *s, long pos)
{

	//fill input buffer
#ifdef _MSC_VER
	s->pread = s->inbuf;
	if(fseek(s->pfile, pos, SEEK_SET)==0){//If successful, the function returns zero.
		s->remaining_bytes = fread(s->inbuf, sizeof(char), s->buffer_length, s->pfile);
	}
	else{
		s->remaining_bytes = 0;
	}
#else
	s->pread = s->inbuf;//reset pread
	//function callback, reset_buffer(fseek + fread)
	if (s->bitstream_mode == 0)//sd fat w/ freertos
		s->remaining_bytes = s->pfn_mp3_reset_buffer(s->buffer_length, s->inbuf, pos);
#endif

	s->DSP_MP3pxgv5 = 0;
	MP3_Clr_Decoded_Bits(s);


	return 0;
}
//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
void MP3_SetPxgv5Pxgv6(DSP_MP3_Content *s, short pxgv5, short pxgv6)
{
	s->DSP_MP3pxgv5 = pxgv5;
	s->DSP_MP3pxgv6 = pxgv6;
	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
void MP3_Clr_Decoded_Bits(DSP_MP3_Content *s)
{
	s->DSP_MP3_Decoded_Bits = 0;
	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
void MP3_End(DSP_MP3_Content *s)
{
	s->DSP_MP3_BitstreamEndFlag = 1;
	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
__inline short ExchangeHiLowBytes(short x)
{
	short y;
	y = x >> 8 & 0x00ff;
	y = y | x << 8;
	return y;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static short UnpackBitstream(DSP_MP3_Content *s, short y1)
{
	const short SnxPassWd = 0;
	short x0, x1, y0, r1;
	short high_word, low_word;

	s->DSP_MP3BitstreamValid = 1;

	x0 = s->DSP_MP3_Decoded_Bits;
	x0 = x0 + y1;
	x0 = x0 & 0x0007;
	s->DSP_MP3_Decoded_Bits = x0;
	x0 = s->DSP_MP3pxgv5;
	r1 = x0 - y1;

	if(r1 >= 0)
	{
		x1 = s->DSP_MP3pxgv6;
		s->DSP_MP3pxgv5 = r1;
		x1 = (unsigned short)x1 >> r1; //logic right shift
		y0 = MP3_MASKS[y1];
		x1 = x1 & y0;
	}
	else
	{
		y0 = MP3_MASKS[s->DSP_MP3pxgv5];
		x1 = s->DSP_MP3pxgv6;
		high_word = x1 & y0; //high byte
		s->DSP_MP3pxgv5 = 16 + r1;
		r1 = -r1;

		s->DSP_MP3BitstreamValid = GetWord(s, &s->DSP_MP3pxgv6);

		if(!s->DSP_MP3BitstreamValid)
		{
			s->DSP_MP3_BitS_Flag = 1;
			return 0;
		}

		s->DSP_MP3pxgv6 = ExchangeHiLowBytes(s->DSP_MP3pxgv6);
		s->DSP_MP3pxgv6 = s->DSP_MP3pxgv6 ^ SnxPassWd;

		y0 = MP3_MASKS[r1];
		high_word = high_word << r1;
		r1 = s->DSP_MP3pxgv5;
		x1 = s->DSP_MP3pxgv6;
		x1 = (unsigned short)x1 >> r1;
		low_word = x1 & y0; //low byte
		x1 = high_word | low_word;
	}

	return x1;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
//static short MP3_CalL2Bytes(DSP_MP3_Content *s)
short MP3_CalL2Bytes(DSP_MP3_Content *s)
{
	short offset;

	offset = 0;

	if(s->DSP_MP3_LowSamp_Flag == 1)
	{
		offset = (long int)MP3_BITRATE_TABLE[0][1][s->DSP_MP3hdr_bitrate_index];
	}
	else
	{
		offset = MP3_BITRATE_TABLE[1][1][s->DSP_MP3hdr_bitrate_index];
	}

	if(s->DSP_MP3hdr_version <= 2 || s->DSP_MP3hdr_sampling_frequency <= 2)	// && ?
	{
		short v = MP2_FRMSIZE_TABLE[3 * s->DSP_MP3hdr_version + s->DSP_MP3hdr_sampling_frequency];
		offset = (offset * v) >> 1;
		offset += (s->DSP_MP3hdr_padding - 4);
	}

	return offset;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static short MP3_CalL3Bytes(DSP_MP3_Content *s)
{
	short offset = 0;

	if(s->DSP_MP3_LowSamp_Flag)
	{
		offset = 9;

		if(s->DSP_MP3hdr_stereo != 1)
		{
			offset = 17;
		}
	}
	else
	{
		offset = 17;

		if(s->DSP_MP3hdr_stereo != 1)
		{
			offset = 32;
		}
	}

	offset += s->DSP_MP3PreSlots;

	return offset;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void Main_Data_Slots_dsp(DSP_MP3_Content *s)
{
	short nSlots;

	if(s->DSP_MP3hdr_bitrate_index < 15)
	{
		nSlots = Mp3FrameSizeTable_[s->DSP_MP3hdr_version][s->DSP_MP3hdr_sampling_frequency][s->DSP_MP3hdr_bitrate_index];
	}
	else
	{
		nSlots = -100;
	}

	if(s->DSP_MP3hdr_padding)
	{
		nSlots++;
	}

	nSlots -= 4;

	if(s->DSP_MP3hdr_error_protection)
	{
		nSlots -= 2;
	}

	if(s->DSP_MP3_LowSamp_Flag)
	{
		if(s->DSP_MP3hdr_stereo == 1)
		{
			nSlots -= 9;
		}
		else
		{
			nSlots -= 17;
		}
	}
	else
	{
		if(s->DSP_MP3hdr_stereo == 1)
		{
			nSlots -= 17;
		}
		else
		{
			nSlots -= 32;
		}
	}

	s->DSP_MP3Slots = nSlots;

	if(s->DSP_MP3Slots > 0)
	{
		s->DSP_MP3PreSlots = s->DSP_MP3Slots;
	}
	else
	{
		s->DSP_MP3Slots = s->DSP_MP3PreSlots;
	}

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static long MP3_CalBitstreamSize(DSP_MP3_Content *s)
{
	s->DSP_MP3QueueData2 = -1;

	if(s->DSP_MP3hdr_lay == 3)
	{
		if(s->DSP_MP3hdr_error_protection == 1)
		{
			UnpackBitstream(s, 16);
			s->DSP_MP3QueueData2 = 2;
		}

		Main_Data_Slots_dsp(s);

		s->DSP_MP3QueueData2 += MP3_CalL3Bytes(s);
	}
	else if(s->DSP_MP3hdr_lay == 2)
	{
		s->DSP_MP3QueueData2 = MP3_CalL2Bytes(s);
	}

	return s->DSP_MP3QueueData2;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void MP3_UpdateHeader(DSP_MP3_Content *s)
{
	s->DSP_MP3Prehdr_version = s->DSP_MP3hdr_version;
	s->DSP_MP3Prehdr_lay = s->DSP_MP3hdr_lay;
	s->DSP_MP3Prehdr_sampling_frequency = s->DSP_MP3hdr_sampling_frequency;
	s->DSP_MP3Prehdr_stereo = s->DSP_MP3hdr_stereo;
	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void Decode_Info_dsp(DSP_MP3_Content *s)
{
	s->DSP_MP3hdr_version = UnpackBitstream(s, 1);

	if(s->DSP_MP3hdr_version == 0)
	{
		s->DSP_MP3_LowSamp_Flag = 1;

		if(s->DSP_MP3hdr_IDex == 1)
		{
			s->DSP_MP3hdr_version = 1;
		}
	}
	else
	{
		s->DSP_MP3_LowSamp_Flag = 0;
		s->DSP_MP3hdr_version = 2;
	}

	s->DSP_MP3hdr_lay = 4 - UnpackBitstream(s, 2);
	s->DSP_MP3hdr_error_protection = !UnpackBitstream(s, 1);
	s->DSP_MP3hdr_bitrate_index = UnpackBitstream(s, 4);
	s->DSP_MP3hdr_sampling_frequency = UnpackBitstream(s, 2);
	s->DSP_MP3hdr_padding = UnpackBitstream(s, 1);
	s->DSP_MP3hdr_extension = UnpackBitstream(s, 1);
	s->DSP_MP3hdr_mode = UnpackBitstream(s, 2);
	s->DSP_MP3hdr_mode_ext = UnpackBitstream(s, 2);
	s->DSP_MP3hdr_copyright = UnpackBitstream(s, 1);
	s->DSP_MP3hdr_original = UnpackBitstream(s, 1);
	s->DSP_MP3hdr_emphasis = UnpackBitstream(s, 2);

	s->DSP_MP3HeaderFlag = 1;

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static short MP3_CurHeaderisValid(DSP_MP3_Content *s)
{
	short ret;

	if(s->DSP_MP3Prehdr_version != s->DSP_MP3hdr_version ||
			s->DSP_MP3Prehdr_lay != s->DSP_MP3hdr_lay ||
			s->DSP_MP3Prehdr_sampling_frequency != s->DSP_MP3hdr_sampling_frequency ||
			s->DSP_MP3Prehdr_stereo != s->DSP_MP3hdr_stereo)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void Seek_Sync_dsp(DSP_MP3_Content *s)
{
	short val;
	const short maxi = 4094;

	if(s->DSP_MP3SubFrmCnt >= 1)
	{
		goto SEEK_SYNC_DSP_LOWS_NEXT3;
	}

	if(s->DSP_MP3_BitstreamEndFlag == 1)
	{
		if(MP3_INBUF_MIN_CNT*2 >= GetByteCnt(s))
		{
			s->DSP_MP3_BitS_Flag = 1;
			goto SEEK_SYNC_DSP_LOWS_END;
		}
	}

	if(s->DSP_MP3_Decoded_Bits)
	{
		UnpackBitstream(s, (short)(8 - s->DSP_MP3_Decoded_Bits));
	}

	val = UnpackBitstream(s, 12);

	while(((val & maxi) != maxi))
	{
		if(s->DSP_MP3_BitS_Flag)
		{
			goto SEEK_SYNC_DSP_LOWS_NEXT3;
		}

		val <<= 8;
		val |= UnpackBitstream(s, 8);

		if(s->DSP_MP3BitstreamValid != 1)
		{
			s->DSP_Flag_MP3 = 0;
			goto SEEK_SYNC_DSP_LOWS_END;
		}
	}

	s->DSP_MP3hdr_IDex = val & 0x01;

SEEK_SYNC_DSP_LOWS_NEXT3:

	s->DSP_Flag_MP3 = 2;

SEEK_SYNC_DSP_LOWS_END:

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void MP3_GetHeader(DSP_MP3_Content *s)
{
	Seek_Sync_dsp(s);

	if(s->DSP_Flag_MP3 == 2)
	{
		Decode_Info_dsp(s);
	}

	if(s->DSP_MP3hdr_mode == 3)
	{
		s->DSP_MP3hdr_stereo = 1;
	}
	else
	{
		s->DSP_MP3hdr_stereo = 2;
	}

	s->DSP_MP3_Format = s->DSP_MP3hdr_lay;

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
__inline static long MP3_GetBegOffset(DSP_MP3_Content *s)
{
	return s->DSP_MP3QueueData2;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void MP3_CalBmpHSize(DSP_MP3_Content *s)
{
	short val1;
	short val2;
	short val3;
	short val4;
	long bitmapsize;

	val1 = UnpackBitstream(s, 16);
	val3 = val1 << 8;
	val4 = (val1 >> 8) & 0x00ff;
	val1 = val3 | val4;
	val2 = UnpackBitstream(s, 16);
	val3 = val2 << 8;
	val4 = (val2 >> 8) & 0x00ff;
	val2 = val3 | val4;
	bitmapsize = (long)val2 << 16 | (val1 & 0xffff);
	//bitmapsize -= 6;

	s->DSP_MP3ID3Size = bitmapsize;

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void MP3_CalRiffHSize(DSP_MP3_Content *s)
{
	short val;
	short temp = 0;

	s->DSP_MP3ID3Size = 0;

	while(1)
	{
		if(s->DSP_MP3ID3Size >= 50)
		{
			s->DSP_MP3ID3Size = 0;
			break;
		}

		temp = temp << 8;
		val = UnpackBitstream(s, 8);
		temp = temp | val;
		s->DSP_MP3ID3Size += 1;

		if(temp == 0x6461)
		{
			val = UnpackBitstream(s, 16);
			s->DSP_MP3ID3Size += 2;

			if(val == 0x7461)
			{
				break;
			}
		}
	}

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void MP3_CalID3HSize(DSP_MP3_Content *s)
{
	short tag[4];
	unsigned long int id3size;

	UnpackBitstream(s, 16);
	UnpackBitstream(s, 8);
	tag[0] = UnpackBitstream(s, 8);
	tag[1] = UnpackBitstream(s, 8);
	tag[2] = UnpackBitstream(s, 8);
	tag[3] = UnpackBitstream(s, 8);

	id3size = 0;
	id3size = (unsigned int)tag[3];
	id3size |= ((unsigned int)tag[2] << 7);
	id3size |= ((unsigned int)tag[1] << 14);
	id3size |= ((unsigned int)tag[0] << 21); //id3size�]�t�e10bytes

	id3size -= 10;
	s->DSP_MP3ID3Size = id3size;

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static short MP3_SeekBegPos(DSP_MP3_Content *s)
{
	const short synckey = 0xffe0;
	const short id3key1 = 0x4944;
	const short id3key2 = 0x4433;
	const short riffkey1 = 0x5249;
	const short riffkey2 = 0x4646;
	const short bmpkey1 = 0x424d;
	/*
	const short endbitstreamstatus = -1;
	const short syncstatus = -2;
	const short id3status = -3;
	const short riffstatus = -4;
	const short bmpstatus = -5;
	*/
	short val = 0;
	short flag = 0;
	short tempval = 0;
	long bytescnt = 0;
	short searchcnt = 0;


	val = UnpackBitstream(s, 16);
	bytescnt = 2; 
	flag = 1;
	searchcnt = 0;


	while(flag >= 0)
	{
#if 1

		if(GetByteCnt(s) < 128*2)
		{
			flag = inbufemptystatus;
			goto EXIT_MP3_SEEKBEGPOS;
		}

#endif

		searchcnt++;

		if((val & synckey) == synckey) //0xffex
		{
			tempval = val >> 1;
			tempval = tempval & (short)0x0003;

			if(tempval == 0 || tempval == 3)
			{
				goto END_CHK_MP3_HEADER_TYPE;
			}

			tempval = val >> 3;
			tempval = tempval & (short)0x0003;

			if(tempval == 1)
			{
				goto END_CHK_MP3_HEADER_TYPE;
			}

			val = UnpackBitstream(s, 16);
			bytescnt += 2;
			tempval = val >> 12;
			tempval = tempval & (short)0x000f;

			if(tempval == 15)
			{
				goto END_CHK_MP3_HEADER_TYPE;
			}

			tempval = val >> 10;
			tempval = tempval & (short)0x0003;

			if(tempval == 3)
			{
				goto END_CHK_MP3_HEADER_TYPE;
			}

			flag = syncstatus;
		}
		else if(val == id3key1)	//0x494433(ID3)
		{
			val <<= 8;
			val |= UnpackBitstream(s, 8);

			if(val == id3key2)
			{
				flag = id3status;
			}

			bytescnt += 1;
		}
		else if(val == riffkey1)	//2011/02/25,0x52494646(RIFF)
		{
			val = UnpackBitstream(s, 16);

			if(val == riffkey2)
			{
				flag = riffstatus;
			}

			bytescnt += 2;
		}
		else if(val == bmpkey1) //2012/02/08// for bitmap header ,0x424d(BM)
		{
			flag = bmpstatus;
		}

END_CHK_MP3_HEADER_TYPE:

		if(s->DSP_MP3_BitS_Flag)
		{
			flag = endbitstreamstatus;
		}

		if(flag == id3status)
		{
			MP3_CalID3HSize(s);
			bytescnt += s->DSP_MP3ID3Size;
		}
		else if(flag == riffstatus)
		{
			MP3_CalRiffHSize(s);//2011/02/25
			bytescnt += s->DSP_MP3ID3Size;
		}
		else if(flag == bmpstatus)
		{
			MP3_CalBmpHSize(s);
			bytescnt += s->DSP_MP3ID3Size;
		}
		else if(flag == 1)
		{
			val <<= 8;
			val |= UnpackBitstream(s, 8);
			bytescnt += 1;
		}

		if(searchcnt >= 32767)
		{
			
			flag = endbitstreamstatus;
		}

	}

	if(flag == syncstatus)
	{
		//only mp3
		bytescnt = bytescnt - 4;
	}

EXIT_MP3_SEEKBEGPOS:
	s->DSP_MP3QueueData2 = bytescnt;
	return flag;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
static void DSP_MP3_Content_initial(DSP_MP3_Content *s)
{
	s->DSP_MP3ID3Size = 0;
	s->DSP_MP3QueueData2 = 0;
	s->DSP_MP3pxgv5 = 0;
	s->DSP_MP3pxgv6 = 0;
	s->DSP_MP3_Decoded_Bits = 0;
	s->DSP_MP3_BitstreamEndFlag = 0;
	s->DSP_Flag_MP3 = 0;
	s->DSP_MP3_BitS_Flag = 0;
	s->DSP_MP3_LowSamp_Flag = 0;
	s->DSP_MP3Pre_LowSamp_Flag = 0;
	s->DSP_MP3hdr_bitrate_index = 0;
	s->DSP_MP3hdr_copyright = 0;
	s->DSP_MP3hdr_emphasis = 0;
	s->DSP_MP3hdr_error_protection = 0;
	s->DSP_MP3hdr_extension = 0;
	s->DSP_MP3hdr_IDex = 0;
	s->DSP_MP3hdr_lay = 0;
	s->DSP_MP3hdr_mode = 0;
	s->DSP_MP3hdr_mode_ext = 0;
	s->DSP_MP3hdr_original = 0;
	s->DSP_MP3hdr_padding = 0;
	s->DSP_MP3hdr_sampling_frequency = 0;
	s->DSP_MP3hdr_version = 0;
	s->DSP_MP3BitstreamValid = 0;
	s->DSP_MP3SeekCount = 0;
	s->DSP_MP3SubFrmCnt = 0;
	s->DSP_MP3hdr_stereo = 0;
	s->DSP_MP3_Format = 0;
	s->DSP_MP3Prehdr_lay = 0;
	s->DSP_MP3Prehdr_sampling_frequency = 0;
	s->DSP_MP3Prehdr_stereo = 0;
	s->DSP_MP3Prehdr_version = 0;
	s->DSP_MP3HeaderFlag = 0;
	s->DSP_MP3PreSlots = 0;
	s->DSP_MP3Slots = 0;

	return;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
long MP3_Get_StartOffset(DSP_MP3_Content *s)
{
	int flag;
	int validHeaderFound;
	int CurBitstreamPos;
	int accbytes, mp3offset, firstvalidoffset;
	int cnt32;
	DSP_MP3_Content_initial(s);

	validHeaderFound = 0;
	CurBitstreamPos = 0;
	accbytes = 0;
	flag = 0;

	while(validHeaderFound < 3)
	{
		CurBitstreamPos = CurBitstreamPos + accbytes;
		accbytes = 0;
		if(ResetInBuf(s, CurBitstreamPos))
		{
			MP3_End(s);
			firstvalidoffset = -1;
			goto exit;
		}
		
		if (GetByteCnt(s) == 0)
		{
			firstvalidoffset = -1;
			goto exit;
		}	

		flag = MP3_SeekBegPos(s);
		mp3offset = MP3_GetBegOffset(s);

		if (mp3offset < 0)
		{
			flag = 0;
			mp3offset = 10;
		}

		switch( flag )
		{
			case syncstatus:
				accbytes = mp3offset;
				CurBitstreamPos = CurBitstreamPos + accbytes;
				accbytes = 0;
				if(validHeaderFound == 0)
				{
					firstvalidoffset = CurBitstreamPos;
				}

				if(ResetInBuf(s, CurBitstreamPos))
				{
					MP3_End(s);
					firstvalidoffset = -1;
					goto exit;
				}

				MP3_GetHeader(s);
				accbytes = accbytes + 4;

				if(validHeaderFound > 0)
				{
					if(!MP3_CurHeaderisValid(s))
					{
						validHeaderFound = -1;
					}
				}

				MP3_UpdateHeader(s);
				cnt32 = MP3_CalBitstreamSize(s);
				accbytes = accbytes + cnt32;
				validHeaderFound = validHeaderFound + 1;

				if(validHeaderFound == 0)
				{
					CurBitstreamPos = firstvalidoffset;
					accbytes = 4;
				}
				break;
		
			case endbitstreamstatus:
				firstvalidoffset = -1;
				goto exit;
				//break;
		
			case inbufemptystatus:
			case id3status:
			case riffstatus:
			case bmpstatus:
			default:
				accbytes = mp3offset;
				break;
		}

	}

exit:
	return firstvalidoffset;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
short MP3_Get_Channel(DSP_MP3_Content *s)
{
	short ch = -1;

	if(s->DSP_MP3HeaderFlag == 1)
	{
		if((s->DSP_MP3hdr_mode - 3) == 0)
		{
			ch = 1;
		}
		else
		{
			ch = 2;
		}
	}

	return ch;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
int MP3_Get_Samples(DSP_MP3_Content *s)
{
	int index = -1;
	const int SR[9] = {576, 576, 576, 576, 576, 576, 1152, 1152, 1152};
	if(s->DSP_MP3HeaderFlag == 1)
	{
		index = 3 * s->DSP_MP3hdr_version + 
			s->DSP_MP3hdr_sampling_frequency;
	}

	if(index < 0 || index >= 9)
		return -1;

	return SR[index];
}

int MP3_Get_SampleRate(DSP_MP3_Content *s)
{
	int index = -1;
	const int SR[9] = {11025, 12000, 8000, 22050, 24000, 16000, 44100, 48000, 32000};
	if(s->DSP_MP3HeaderFlag == 1)
	{
		index = 3 * s->DSP_MP3hdr_version + 
			s->DSP_MP3hdr_sampling_frequency;
	}

	return SR[index];
}
//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
short MP3_Get_BitRate(DSP_MP3_Content *s)
{
	short offset;
  if(s->DSP_MP3hdr_lay ==2)
	{
		if(s->DSP_MP3_LowSamp_Flag == 1)
			offset = (long int)MP3_BITRATE_TABLE[0][1][s->DSP_MP3hdr_bitrate_index];
		else
			offset = MP3_BITRATE_TABLE[1][1][s->DSP_MP3hdr_bitrate_index];
	}
	else if(s->DSP_MP3hdr_lay ==3)
	{
		if(s->DSP_MP3_LowSamp_Flag == 1)
			offset = (long int)MP3_BITRATE_TABLE[0][2][s->DSP_MP3hdr_bitrate_index];
		else
			offset = MP3_BITRATE_TABLE[1][2][s->DSP_MP3hdr_bitrate_index];
	}
	return offset;
}
