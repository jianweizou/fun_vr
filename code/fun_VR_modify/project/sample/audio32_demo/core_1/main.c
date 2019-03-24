/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file main.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/17
 * @license
 * @description
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "snc_types.h"
#include "bsp.h"
#include "snc_gpio.h"
//#include "sonix_config.h"
#include "snc_ipc.h"
#include "Alg_settings.h"

void *g_speechenhance_control;



void Alg_Init(void * g_speechenhance_control_T, int Fs, int ALG_SW);
void Alg_Process(short *f_lbuf, short *f_rbuf, short *f_refbuf, short *outdata);
int Alg_Get_Wram_Size(int ALG_SW);
int Alg_Settings(int Alg_ID, int Ch_ID, int setting_ID, int* setting_val);

int add_func(int a, int b);
int FrmCnt;
int anw;
int raw_CurAdrr;
short ChkSum, sum_input;
short databuf_temp[512];
short databuf_MicR[256];
short databuf_MicL[256];
short databuf_Ref[256];
short databuf_Out[256];

pCMD_ENT	ptrQ;

//=============================================================================
//                  Constant Definition
//=============================================================================


//=============================================================================
//                  Macro Definition
//=============================================================================


//=============================================================================
//                  Structure Definition
//=============================================================================


//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================


//=============================================================================
//                  Public Function Definition
//=============================================================================

#define MIPS_CALC
volatile int EncCnt;
int EncCnt_max=0;

#define ALG_SWITCH 0x08


/**
 *  CORE 0 main enter pointer
 */
int main (void)
{
int ix;
int ALG_Switch=ALG_SWITCH;
	
    SysTick_Config(CONFIG_CPU_CLOCK_MHZ * 1000 );
    GPIO_SetPin(GPIO_PORT_0_P02, GPIO_MODE_OUTPUT,
                GPIO_CONFIG_INACTIVE, 1);
	
		//SysTick_Config(162000); //1ms
		SysTick_Config(20250); //0.125ms
	
		FrmCnt = 0;
		ChkSum = 0;
		sum_input=0;
	
		anw = Alg_Get_Wram_Size(ALG_Switch);		//size of wram for core 1 algorithms
		void *Wram_Addr=malloc(anw);				//allocate wram for core 1
		
	
		raw_CurAdrr = 0x60400000;		//location of pcm file
	
		Alg_Init(Wram_Addr,16000,ALG_Switch);		//(wram address, fs, switch of alg)
		
		int temp_var=0x3000;
		Alg_Settings(NR_ID, NR_MIC_L, NR_gain_floor, &temp_var);
		temp_var=118;
		Alg_Settings(NR_ID, NR_MIC_L, NR_bin_used, &temp_var);
		
		EncCnt = 0;

    while(1)
    {
        asm ("nop");
				anw = add_func(7, 3);
			
				if(CMD_DONE == CMD_SENT)
				{
					memcpy(databuf_temp,(void *)raw_CurAdrr,1024);		//stereo pcm*256 point=2*2*256=1024
					raw_CurAdrr = raw_CurAdrr + 1024;
				
					for(ix = 0 ; ix < 256 ; ix++)
					{
						databuf_MicL[ix] = databuf_temp[2*ix];
						databuf_MicR[ix] = databuf_temp[2*ix+1];
						databuf_Ref[ix] = databuf_temp[2*ix+1];
						sum_input+=databuf_MicL[ix];
						sum_input+=databuf_MicR[ix];
					}
					
					EncCnt = 0;
					Alg_Process(databuf_MicL,databuf_MicR,databuf_Ref,databuf_Out);		//(mic in L, mic in R, ref in, output)
#ifdef MIPS_CALC
					while(CMD_DONE != CMD_SENT);
					if(EncCnt_max<EncCnt)
						EncCnt_max=EncCnt;
#endif
					
					for(ix = 0 ; ix < 256 ; ix++)
						ChkSum = ChkSum + databuf_Out[ix];
					
					FrmCnt++;
					if(FrmCnt > 0x0a63)
						break;
				}
    }
}

__irq void SysTick_Handler()
{
//	static int i=0;
//	
//	if(++i >= 300){
//		i = 0;
//		GPIO_TogglePin(GPIO_PORT_0_P02);
//	}
		EncCnt++;
}

int add_func(int a, int b)
{
	return (a+b);
}

void Alg_Init(void * g_speechenhance_control_T, int Fs, int ALG_SW)
{
	ptrQ = GET_CMD_ENT_ADD();
	ptrQ->CMD = SPEECH_ENHANCE_INIT;
	ptrQ->PARA1 = (int)g_speechenhance_control_T;
	ptrQ->PARA2 = (int)Fs;
	ptrQ->PARA3 = (int)ALG_SW;
	SEND_CMD();
	//while(CMD_DONE != CMD_SENT);
}

void Alg_Process(short *f_lbuf, short *f_rbuf, short *f_refbuf, short *outdata)
{
	ptrQ = GET_CMD_ENT_ADD();
	ptrQ->CMD = SPEECH_ENHANCE_PROCESS;
	ptrQ->PARA1 = (int)f_lbuf;
	ptrQ->PARA2 = (int)f_rbuf;
	ptrQ->PARA3 = (int)f_refbuf;
	ptrQ->PARA4 = (int)outdata;
	SEND_CMD();
	while(CMD_DONE != CMD_SENT);
}

int Alg_Get_Wram_Size(int ALG_SW)
{
	ptrQ = GET_CMD_ENT_ADD();
	ptrQ->CMD = SPEECH_ENHANCE_GET_WRAM_SIZE;
	ptrQ->PARA1 = (int)ALG_SW;
	SEND_CMD();
	while(CMD_DONE != CMD_SENT);
	return ptrQ->RET;
}

int Alg_Settings(int ALG_ID, int channel_ID, int setting_ID, int *setting_val)
{
	ptrQ = GET_CMD_ENT_ADD();
	ptrQ->CMD = SPEECH_ENHANCE_SETTINGS;
	ptrQ->PARA1 = (int)ALG_ID;
	ptrQ->PARA2 = (int)channel_ID;
	ptrQ->PARA3 = (int)setting_ID;
	ptrQ->PARA4 = (int)setting_val;
	SEND_CMD();
	while(CMD_DONE != CMD_SENT);
	return ptrQ->RET;
}
