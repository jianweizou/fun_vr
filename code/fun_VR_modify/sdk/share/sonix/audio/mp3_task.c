#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "snc_codec.h"
#include "snc_ipc.h"
#include "mp3dec_parser.h"
#include "rom_code_api.h"


void MP3Rec_Task(void* pvParameters)
{
	PS_RECORDER	psRec = pvParameters;
	unsigned int	Index;
	E_PINGPONG_BUFFER	ePPB = ePPB0;

	while(1)
	{
		if (psRec->eStatus == eRECORDING)
		{
			Index = ADC_Get_Rx_Index(psRec->eADC_I2S);
			
			if ((Index<psRec->ChannelBL/2)&&(ePPB==ePPB1))
			{
				if (psRec->eMP3Mode != eMP3_MODE_MONO)
				{
					memcpy(psRec->pTempBuffer, psRec->pADCBuffer+psRec->ChannelBL/2, psRec->ChannelBL);
					memcpy(psRec->pTempBuffer+psRec->ChannelBL/2, psRec->pADCBuffer+psRec->ChannelBL+psRec->ChannelBL/2, psRec->ChannelBL);
					MP3Enc_EncodeFrame(psRec->psMP3Enc, psRec->pTempBuffer, psRec->pBitsBuffer, YES);
				}
				else
				{
					MP3Enc_EncodeFrame(psRec->psMP3Enc, psRec->pADCBuffer+psRec->ADC_BL/2, psRec->pBitsBuffer, YES);
				}
				
				psRec->eStatus = (E_STATUS)psRec->pfnMoveBitsAway(psRec->pBitsBuffer, psRec->psMP3Enc->outbytes);
				ePPB = ePPB0;
			}
			else if ((Index>=psRec->ChannelBL/2)&&(ePPB==ePPB0))
			{
				if (psRec->eMP3Mode != eMP3_MODE_MONO)
				{
					memcpy(psRec->pTempBuffer, psRec->pADCBuffer, psRec->ChannelBL);
					memcpy(psRec->pTempBuffer+psRec->ChannelBL/2, psRec->pADCBuffer+psRec->ChannelBL, psRec->ChannelBL);
					MP3Enc_EncodeFrame(psRec->psMP3Enc, psRec->pTempBuffer, psRec->pBitsBuffer, YES);
				}
				else
				{
					MP3Enc_EncodeFrame(psRec->psMP3Enc, psRec->pADCBuffer, psRec->pBitsBuffer, YES);
				}

				psRec->eStatus = (E_STATUS)psRec->pfnMoveBitsAway(psRec->pBitsBuffer, psRec->psMP3Enc->outbytes);
				ePPB = ePPB1;
			}
		}
		vTaskDelay(1);
	}
}

void MP3Play_Task(void* pvParameters)
{
	PS_PLAYER	psPlay = pvParameters;
	bool		bGotSyncword = NO;
	char*	pRead = psPlay->pBitsBuffer;
	int	RemainingBytes = 0;
	int	UsedBytes = 0;
	unsigned int	Index;
	E_PINGPONG_BUFFER	ePPB = ePPB1;
	
	psPlay->pfnRefillBuffer(psPlay->pBitsBuffer, psPlay->BitsBL, pRead, &RemainingBytes);

	while(1)
	{
		if (RemainingBytes > 0 && psPlay->eStatus == ePLAYING)
		{
			Index = DAC_Get_Tx_Index(psPlay->eDAC_I2S);
			
			////seek syncword if needed
			if (bGotSyncword == NO)
			{
				switch(MP3Parser_Seek_Syncword(psPlay->pBitsBuffer, psPlay->pBitsBuffer+psPlay->BitsBL, &pRead, &UsedBytes, &RemainingBytes, 0))
				{
					case 0:
						psPlay->pfnRefillBuffer(psPlay->pBitsBuffer, psPlay->BitsBL, pRead, &RemainingBytes);
						goto exit_while;
						//break;
					case -1:
						psPlay->psMP3Dec->MP3SeekCount = 2;
					case 1:
					default:
						bGotSyncword = YES;
						break;
				}
				////fill input buffer
				psPlay->pfnRefillBuffer(psPlay->pBitsBuffer, psPlay->BitsBL, pRead, &RemainingBytes);
			}

			if ((Index<psPlay->ChannelBL/2)&&(ePPB==ePPB1))
			{
				if (eMONO != psPlay->eChannel)
				{
					MP3Dec_DecodeFrame(psPlay->psMP3Dec, pRead, psPlay->pTempBuffer, &UsedBytes, YES);
					memcpy(psPlay->pDACBuffer+psPlay->ChannelBL/2, psPlay->pTempBuffer, psPlay->ChannelBL);
					memcpy(psPlay->pDACBuffer+psPlay->ChannelBL+psPlay->ChannelBL/2, psPlay->pTempBuffer+psPlay->ChannelBL/2, psPlay->ChannelBL);
				}
				else
				{
					MP3Dec_DecodeFrame(psPlay->psMP3Dec, pRead, psPlay->pDACBuffer+psPlay->ChannelBL/2, &UsedBytes, YES);
				}
				
				ePPB = ePPB0;
				bGotSyncword = NO;
				////update buffer status
				RemainingBytes -= UsedBytes;
				pRead += UsedBytes;
				if (pRead >= psPlay->pBitsBuffer+psPlay->BitsBL)
					pRead -= psPlay->BitsBL;
			}
			else if ((Index>=psPlay->ChannelBL/2)&&(ePPB==ePPB0))
			{
				if (eMONO != psPlay->eChannel)
				{
					MP3Dec_DecodeFrame(psPlay->psMP3Dec, pRead, psPlay->pTempBuffer, &UsedBytes, YES);
					memcpy(psPlay->pDACBuffer, psPlay->pTempBuffer, psPlay->ChannelBL);
					memcpy(psPlay->pDACBuffer+psPlay->ChannelBL, psPlay->pTempBuffer+psPlay->ChannelBL/2, psPlay->ChannelBL);
				}
				else
				{
					MP3Dec_DecodeFrame(psPlay->psMP3Dec, pRead, psPlay->pDACBuffer, &UsedBytes, YES);
				}

				ePPB = ePPB1;
				bGotSyncword = NO;
				////update buffer status
				RemainingBytes -= UsedBytes;
				pRead += UsedBytes;
				if(pRead>=psPlay->pBitsBuffer+psPlay->BitsBL)
					pRead -= psPlay->BitsBL;
			}
exit_while:;
		}
		else
		{
			psPlay->eStatus = eSTOP;
			psPlay->pfnPlayFinished();
			vTaskDelete(NULL);
		}
	}
}

