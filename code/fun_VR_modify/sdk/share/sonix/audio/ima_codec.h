#ifndef IMA_CODEC_H
#define IMA_CODEC_H

typedef struct{
	short StepIndex;
	short PredictedValue;
	short PackBitCnt;
	short EncodeDataResdu;
	short PackResidueBitFlag;
	short ReadEncodeDataFlag;
	short frmcnts;
	short frmsizeT;
	int packwords;
}SonixIMAcodec;



#endif

int IMA_Encode_Init(SonixIMAcodec* pSonixIMAcodec,short frmsizeT);
void IMA_Decode_Init(SonixIMAcodec* pSonixIMAcodec,int packwords);

short IMA_EncodeProcess(SonixIMAcodec* pSonixIMAcodec, short *frmbuf, short *packedbuf);
int IMA_DecodeProcess(SonixIMAcodec* pSonixIMAcodec, short *packwordsbuf, short *outputbuf);
