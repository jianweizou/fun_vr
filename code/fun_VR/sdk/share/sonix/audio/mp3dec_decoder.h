

#ifndef MP3DEC_DECODER_H
#define MP3DEC_DECODER_H


typedef struct{
	unsigned long int   MP3_nDuration;
	int   MP3_ABFrmCnt;
	unsigned long int   MP3_SampleID;
	unsigned long int   MP3_FileSize;
	long int MP3_SampleRate;
	long int MP3_nChannels;
	long int MP3_BitRate;
	long int MP3_ID3Size;
	long int MP3_StartOffset;
	long int MP3_StartOffsetBytes;
	long int MP3_OffsetBytes;
	long int MP3_OIDformat;
	int gPreSlots;
	short MP3Format;	
	short pxgv5;
	short pxgv6;
	short MP3_Decoded_Bits;
	short MP3_MainData_Decoded_Bits;
	short MP3_BitS_Flag;
	short MP3_LowSamp_Flag;
	short header_IDex;
	short SnxPassWd;
	short gHuffManBits;
	short gMaxHuffManBits;
	short gScaleBits;
	short gOnlyOneByte;
	short _MP3MainDataOnlyOneByteFlag;
	short gEndQueueFlag;
	short gEndQueuePos;
	short gHeadPos;
	short gTailPos;
	short gBitPackPos;
	short gBitPackBuff;
	short gMainDataVal;
	short gHeadPosByte;
	short gTailPosByte;
	short DiscardBytes;
	short gValidFreqLine;
	short gValidFreqLineb;
	short gValidCB;
	short gLeftBits;
	short hdr_version;
	short hdr_lay;
	short hdr_error_protection;
	short hdr_bitrate_index;
	short hdr_sampling_frequency;
	short hdr_padding;
	short hdr_extension;
	short hdr_mode;
	short hdr_mode_ext;
	short hdr_copyright;
	short hdr_original;
	short hdr_emphasis;
	short gPrehdr_version;
	short gPrehdr_lay;
	short gPrehdr_sampling_frequency;
	short gPrehdr_stereo; 
	short gPreMP3_LowSamp_Flag;
	short hdr_actual_mode;
	short hdr_stereo;
	short hdr_jsbound;
	short hdr_sblimit;
	short si_main_data_begin;
	short si_private_bits;
	short ms_stereo_flag;
	short is_stereo_flag;	
	short max_stereo_sb;
	short MP3EQEnableFlag;
	short MP2_SubBand_Limit;
	short MP2_Decoded_Bits;
	short Music96SubFrmCnt;
	short debug_MP3_MainData_Decoded_Bits;
	short debug_gTailPosByte;
	short debug_gLeftBits;
	short debug_gMainDataVal;
	short debug_gTailPos;
	short debug_gHeadPos;
	short debug_gOnlyOneByte;
	short debug__MP3MainDataOnlyOneByteFlag;
	short debug_gBitPackPos;
	short debug_gHeadPosByte;
	short MP3SeekCount;	
	short error_msg;
	short interleave;
	unsigned short bit_alloc_dsp[2][32];
	unsigned short scfsi_dsp[2][32];
	unsigned short scale_index_dsp[2][3][32];	
	short gValidCBCH[2];
	short gValidFreqLineCH[2];
	short III_scalefac_l[2][23];
	short III_scalefac_s[2][13][3];
	short SynthesisIn[2][32];
	short bufOffset[2];
	short si_ch_scfsi[2][4];
	short si_ch_gr_info[2][2][18];
	short ss_add_dsp[16];
	short ss_sub_dsp[16];
	short MSFlag[576];	
	short MP3EQBandMag[10];	
	short fast_pcm_sample_dsp[2][3][32];
	short sample_dsp[2][3][32];
	short fraction_dsp[2][3][32];	
	short gQueue[1024];	
	short gQueue2[2];	
	long Huffman_Cnt[34];
	short** III_is;
	short** IMDCT_BLK;
	short** V_Fast_dsp;
	short *ptr_isratio;
	short *ptrTabAlloc;
	short *ptrTabAlloc2;
	char* inbufp;
	char* inbufp_in;
	char* inbufh;
	char* inbuft;
	int used_bytes;
	int output_bytes;
	//int SnxEncFlag;
	int NEWHUFFFUNC_Flag;
	int (*NEWHUFFFUNC)(void*, short*, short, short, short);
	int Recover_flag;
}SonixMP3decoder;




#endif


void MP3Dec_initialise_struct(SonixMP3decoder* pSonixMP3decoder, short interleave, char* inbufh, char* inbuft, int recover, int sram);//ROM***
int MP3Dec_decode(SonixMP3decoder* pSonixMP3decoder, char* MP3InBuf, short* MP3OutBuf, int* used_bytes);//ROM




