#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "snc_codec.h"
#include "snc_gpio.h"
#include "snc_ipc.h"
#include "snc_types.h"
#include "snc_spifc.h"
#include "audio32codec_decoder.h"
#include "aud_driver.h"

#include "CSpotterSDKApi.h"
#include "Alg_settings.h"

#include "PLD_NR\PLDNR.h"
#include "Speech_Enhance.h"
#include "snc_log_util.h"
#include "snc_system.h"
#include "uart_send_data.h"
//#include "snc_ipc_vr.h"
/*_____ D E F I N I T I O N S ______________________________________________*/
#define TRIGGER_WORD 1
#ifndef TRIGGER_WORD
#define TRIGGER_WORD 0
#endif

#define ALG_FUNCTION				1 //Add alg. funtion. 0:disable; 1:enable
#define RESULT_HANDLER			1 //recognition result handle
#define AGC_FUNCTION				1 //AGC control, 0:disable, 1:enable


#define CYB_CMD_LENGTH 60*1024	//50*1024 // This value is depended on *.bin size.
																// It should be dynamically calculated in rd_wr_flash.s.																
#define MEM_SIZE (10 * 1024)
#define MAX_TIME 300
#define AD_GAIN_TAB_LEN 64

#define	OFF	0
#define	ON	1

#define P_NONE	0
#define P_PLAY	1
#define P_PAUSE	2

/*_____ M A C R O S ________________________________________________________*/
#if ALG_FUNCTION == 1
enum Alg_ID dsp_alg_status = 0x08;//0x80:NLMS_ID//0x08:NR_ID
//ALG_switch: (LSB) BF, PLD, NLP, NR, MDF, EQ, IVA, NLMS, NA*8 (MSB) 
void NR_Init(void);
int Alg_Settings(int ALG_ID, int channel_ID, int setting_ID, int *setting_val);
#endif

#if AGC_FUNCTION == 1
short AD_Gain_Tab[AD_GAIN_TAB_LEN]={
		0x00,	//-12dB
		0x01,	//-11.25dB
		0x02,	//-10.5dB
		0x03,	//-9.75dB
		0x04,	//-9dB
		0x05,	//-8.25dB
		0x06,	//-7.5dB
		0x07,	//-6.75dB
		0x08,	//-6dB
		0x09,	//-5.25dB
		0x0a,	//-4.5dB
		0x0b,	//-3.75dB
		0x0c,	//-3dB
		0x0d,	//-2.25dB
		0x0e,	//-1.5dB
		0x0f,	//-0.75dB
		0x10,	//0dB
		0x11,	//0.75dB
		0x12,	//1.5dB
		0x13,	//2.25dB
		0x14,	//3dB
		0x15,	//3.75dB
		0x16,	//4.5dB
		0x17,	//5.25dB
		0x18,	//6dB
		0x19,	//6.75dB
		0x1a,	//7.5dB
		0x1b,	//8.25dB 
		0x1c,	//9dB 
		0x1d,	//9.75dB
		0x1e,	//10.5dB
		0x1f,	//11.25dB
		0x20,	//12dB
		0x21,	//12.75dB
		0x22,	//13.5dB
		0x23,	//14.25dB
		0x24,	//15dB
		0x25,	//15.75dB
		0x26,	//16.5dB
		0x27,	//17.25dB
		0x28,	//18dB
		0x29,	//18.75dB
		0x2a,	//19.5dB
		0x2b,	//20.25dB
		0x2c,	//21dB
		0x2d,	//21.75dB
		0x2e,	//22.5dB
		0x2f,	//23.25dB
		0x30,	//24dB
		0x31,	//24.75dB
		0x32,	//25.5dB
		0x33,	//26.25dB
		0x34,	//27dB
		0x35,	//27.75dB
		0x36,	//28.5dB
		0x37,	//29.25dB
		0x38,	//30dB
		0x39,	//30.75dB
		0x3a,	//31.5dB
		0x3b,	//32.25dB
		0x3c,	//33dB
		0x3d,	//33.75dB
		0x3e,	//34.5dB
		0x3f		//35.25dB
};
#endif


/*_____ D E C L A R A T I O N S ____________________________________________*/
uint32_t cmd_idx;
uint32_t stop_flag = 0;
uint32_t trigger_flag = 0;
uint32_t trigger_timer_count=0;
uint32_t cmd_timer_count=0;
uint32_t ALG_FLAG=1;
uint32_t recognition_result;

#if TRIGGER_WORD == 0
	int32_t pnWordIdx[1] = {0};
	uint8_t *lppbyModel[2];
#else
	int32_t pnWordIdx[2] = {0};
	uint8_t *lppbyModel[3];	
#endif


INT nErr;
uint8_t lpbyState[96];
INT nStateSize = 96;
int32_t tmp;
HANDLE hCSpotter;
short AEC_OutBuf[256];
int nNumSample=0;
int16_t *audio_buf_L;//mic buffer
int16_t *audio_buf_R;//ref. buffer
	

#if AGC_FUNCTION == 1
int AGC_GAIN_temp;
short last_gainidx = 0;
extern int SNX_AGC_Process(short *inputbuf, short *outputbuf, short sample_num);
void SNX_AGC_Process_init(int AGC_gain_max,\
													int AGC_gain_min,	\
													int AGC_gain_default,\
													int AGC_Dynamic_gain_max,\
													int AGC_Dynamic_gain_min,\
													int AGC_Dynamic_Target_Level_Low,\
													int AGC_Dynamic_Target_Level_High,\
													int AGC_Dynamic_updateSpeed ,\
													int AGC_bufsize,\
													int sample_rate,\
													int peakamp_thres,\
													int peakcnt_thres,\
													int response_up_step,\
													int response_down_step);
#endif


int16_t ALG_Status;
int16_t * ALGSequegInputBuf;
int16_t * ALGSequegInputBuf1;

extern const unsigned char CYB_CMD1_INFLASH[];


__align(4) BYTE memory_pool[MEM_SIZE] __attribute__((align(0x100)));//0x18023E00
uint8_t model_buf[CYB_CMD_LENGTH] __attribute__((align(0x100)));//0x18026600


int ADC_IDX;
int DAC_IDX;


short *fpA32Current;
short *fpA32End;

bool bDAC_Switch;


#if ALG_FUNCTION == 1
int anw;
pld_nr_t *g_pld_control;
speech_enhance_t *g_speechenhance_control;
pCMD_ENT	ptrQ;
#endif

E_PINGPONG_BUFFER eADC_PPB = ePPB0;
E_PINGPONG_BUFFER eDAC_PPB = ePPB1;

E_BITRATE eDAC_BR = e32kbps;
aud32dec_t *psA32dec;


#define	ADC_BL	512//2048	/* BL = Buffer Length */
short		ADC_Buffer[ADC_BL*2] = {0};

#define	MIC_BL	512//2048	/* BL = Buffer Length */
short		MIC_Buffer[MIC_BL*2] = {0};

#define	DAC_BL	640	/* BL = Buffer Length */
short		DAC_Buffer[DAC_BL] = {0};


void AGC_Init(void);
void REF_AGC_Init(void);
void ALG_Init(void);
void Alg_Process_PLDInit(void * g_speechenhance_control_T, int Fs, int ALG_SW);
void Alg_Process_PLDCore(short *f_lbuf, short *f_rbuf, short *f_refbuf, short *outdata);
int Alg_Get_Wram_Size(int ALG_SW);
void Record_Start(void);
void Record_Progress(void);
void Audio32_Play_Start(void);
void Audio32_Play_Select(uint8_t n);
void Audio_Playback_Process(void);
void Voice_Recognition_Init(void);
void Voice_Recognition_Progress(void);

extern int g_AGC_bufsize;
int *AGC_ptr;
//extern int *AGC_ptr[41];
//int *AGC_ptr[41];
int AGC_ptr_Mic[41];
int AGC_ptr_Ref[41];

int temp_var;


unsigned char Language_index = 0,isneedsetlanguage =1;

unsigned char pwmvalue=70;

bool Is_ADC_Time_Up(void);
uint32_t Voice_Recognition_Result(uint8_t n);
unsigned int LED_status;
int led_vr_time;
/********************************************fun ************************/
unsigned char is_5ms_Flag;
unsigned char cnt_5ms = 0;
extern void fun_main(void);
/*_____ F U N C T I O N S ______________________________________________*/
/*********************************
* VR + AGC + NR
*********************************/
int main (void)
{
	SysTick_Config(162000);
	NVIC_EnableIRQ(SysTick_IRQn);

	I2S4_Init();

	Record_Start();
	
	AGC_ptr = &g_AGC_bufsize;

#if AGC_FUNCTION == 1
	AGC_Init();
	for(int k=0;k<41;k++){
		AGC_ptr_Mic[k] = AGC_ptr[k];
	}
#endif

#if AEC_AGC_FUNCTION == 1
	REF_AGC_Init();
	for(int k=0;k<41;k++){
		AGC_ptr_Ref[k] = AGC_ptr[k];
	}
#endif
	
#if ALG_FUNCTION == 1
	ALG_Init();

	if((0x01&(dsp_alg_status>>3)) ==1)
		NR_Init();
#endif

	Voice_Recognition_Init();
    log_driver_init_t  init_log = {0};
	init_log.uart.port = UART_PORT_0;
    init_log.uart.sys_clock = CONFIG_CPU_CLOCK_MHZ * 1000000;
    init_log.uart.baud_rate = 115200;

    LOG_Driver_Init(&init_log);
	while(1)
	{
		fun_main();
		if (Is_ADC_Time_Up())
			Voice_Recognition_Progress();
	}
}

/*********************************
* Record Start
*********************************/
void Record_Start(void)
{
	if (External_Audio_Codec_Init(eEAC_AUD, eSN_I2C_CH2, e16kHz))
		return ;
	AUD01_ADC_Gain_Ctrl(0x01, 0x3f, 0x0000, 0x010);
	ADC_Enable(eCODEC_I2S_0, ADC_Buffer, ADC_BL, Convert_to_Codec_Sampling_Rate(e16kHz), Convert_to_Codec_Channel(eSTEREO));
	return ;		
}

/*********************************
* Record Progress
*********************************/
bool Is_ADC_Time_Up(void)
{
	unsigned int index;

	index = ADC_Get_Rx_Index(eCODEC_I2S_0);

	{
		if ((index<ADC_BL/2)&&(eADC_PPB==ePPB1))
		{
			audio_buf_R =(int16_t *)(ADC_Buffer+ADC_BL/2);
			audio_buf_L =(int16_t *)(ADC_Buffer+ADC_BL/2)+ADC_BL;		
			nNumSample = 0;
			eADC_PPB = ePPB0;
			return YES;
		}
		else if ((index>=ADC_BL/2)&&(eADC_PPB==ePPB0))
		{
			audio_buf_R =(int16_t *)(ADC_Buffer);
			audio_buf_L =(int16_t *)(ADC_Buffer)+ADC_BL;		
			nNumSample = ADC_BL/2;
			eADC_PPB = ePPB1;
			return YES;
		}
	}
	
	return NO;
}

/*********************************
* AGC Function Initial
*********************************/
void AGC_Init(void)
{
	int sample_rate_temp, buf_size_temp, sat_thr_temp;	
	buf_size_temp = 256;
	sample_rate_temp = 16000;
	sat_thr_temp=20;	//150
	SNX_AGC_Process_init(37,	//37,AGC_gain_max, 24
			15,		//AGC_gain_min, 27	
			32,		//32//AGC_gain_default,
			37,		//AGC_Dynamic_gain_max, 37
			15,		//AGC_Dynamic_gain_min, 27
			1500,	//AGC_Dynamic_Target_Level_Low,
			3000,	//AGC_Dynamic_Target_Level_High,
			32,		//8,AGC_Dynamic_updateSpeed , 10
			buf_size_temp,	//AGC_bufsize,
			sample_rate_temp,	//sample_rate,
			1000,	//16500peakamp_thres, 15000
			(int)sat_thr_temp,//(g_HALF_LENGTH>>1),	//peakcnt_thres ,
			14,		//response_up_step,
			1);		//response_down_step);
}


/*********************************
* AGC Setting
*********************************/
#if AGC_FUNCTION == 1
uint16_t realgainL, realgainR;
void Gain_Setting(short *gain_table,short gain_table_size,short gainidx, short channel)
{
	uint16_t realgain;
	uint16_t wdata;
	if(gainidx!=last_gainidx)
	{
		realgain = gain_table[gainidx];

		if(channel == 0)
		{
			realgainL = realgain ;
			realgain = realgain + 0x40;
			AUD_RegWrite( ADC_PGA_L, realgain);   //0x003F /*default*/
			AUD_RegRead( ADC_PGA_L, &wdata);
		}
    else if(channel ==1)
		{
		  realgainR = realgain ;
			realgain = realgain | 0x40;
			AUD_RegWrite( ADC_PGA_R, realgain );  //0x003F /*default*/
			AUD_RegRead( ADC_PGA_R, &wdata);
		}

		last_gainidx = gainidx;
	}
}
#endif

/*********************************
* Alg. Function Initial
*********************************/
#if ALG_FUNCTION == 1
void ALG_Init(void)
{
	int i;
	i=Alg_Get_Wram_Size(dsp_alg_status);//NR=0x0008
	g_speechenhance_control = (speech_enhance_t*)malloc(i);	// PLD_NR App malloc	, speech_enhance_t= 0x7A50
	ALGSequegInputBuf = (int16_t *)malloc(1024);
	ALGSequegInputBuf1 = (int16_t *)malloc(1024);
	Alg_Process_PLDInit(g_speechenhance_control,16000,dsp_alg_status);
}

void Alg_Process_PLDInit(void * g_speechenhance_control_T, int Fs, int ALG_SW)
{
	ptrQ = GET_CMD_ENT_ADD();
	ptrQ->CMD = SPEECH_ENHANCE_INIT;
	ptrQ->PARA1 = (int)g_speechenhance_control_T;
	ptrQ->PARA2 = (int)Fs;
	ptrQ->PARA3 = (int)ALG_SW;
	SEND_CMD();
	//while(CMD_DONE != CMD_SENT);
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

void NR_Init(void)
{
	temp_var=0x4500; //set NR gain floor
	Alg_Settings(NR_ID, NR_MIC_L, NR_gain_floor, &temp_var);
	temp_var=118; //set NR 作用頻帶上限
	Alg_Settings(NR_ID, NR_MIC_L, NR_bin_used, &temp_var);
}

/*********************************
* NR function Setting
*********************************/
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
#endif

/*********************************
* Alg. function Progress
*********************************/
#if ALG_FUNCTION == 1
void Alg_Process_PLDCore(short *f_lbuf, short *f_rbuf, short *f_refbuf, short *outdata)
{
	while(CMD_DONE != CMD_SENT);
	ptrQ = GET_CMD_ENT_ADD();
	ptrQ->CMD = SPEECH_ENHANCE_PROCESS;
	ptrQ->PARA1 = (int)f_lbuf;
	ptrQ->PARA2 = (int)f_rbuf;
	ptrQ->PARA3 = (int)f_refbuf;
	ptrQ->PARA4 = (int)outdata;
	SEND_CMD();
	//while(CMD_DONE != CMD_SENT);
}
#endif

unsigned short get_a_frame_length(E_BITRATE eBitrate)
{
	if (eBitrate == e32kbps)
		return 40;
	else if (eBitrate == e24kbps)
		return 30;
	else if (eBitrate == e16kbps)
		return 20;
	else
		return 0;
}


uint32_t UnpackBin(uint32_t lpbyFlashAddr, uint8_t *lppbyModel[],
                   int32_t nMaxNumModel, int32_t *lpnFlashSize)
{
	int32_t *lpnBin = (int32_t *) lpbyFlashAddr;
	int32_t nNumBin = lpnBin[0];
	int32_t *lpnBinSize = lpnBin + 1;
	int32_t i;

	lppbyModel[0] = (uint8_t *) (lpnBinSize + nNumBin);
	for (i = 1; i < nNumBin; i++) {
		if (i >= nMaxNumModel)
			break;
		lppbyModel[i] = lppbyModel[i-1] + lpnBinSize[i-1];
	}

	*lpnFlashSize = (uint32_t) (lppbyModel[i-1] + lpnBinSize[i-1]) 
	                           - lpbyFlashAddr;
	// 4KB alignment for SPI
	*lpnFlashSize = ((*lpnFlashSize + 0xFFF) >> 12) << 12;

	return i;
}

/*********************************
* Voice Recognition Initial
*********************************/
void Voice_Recognition_Init(void)
{
	if (isneedsetlanguage == 0)
		return;
	isneedsetlanguage = 0;
	if (Language_index == 0)
		SPIFC_ReadWithDma( SPIFC_RD_MODE_FAST, (uint32_t)&CYB_CMD1_INFLASH , CYB_CMD_LENGTH , (uint32_t)model_buf ,(uint32_t)true);
	lppbyModel[0] = model_buf;

	#if TRIGGER_WORD == 0
		if (UnpackBin((uint32_t)&model_buf, lppbyModel, 2, &tmp) < 2) 
			while(1);
	#else
		if (UnpackBin((uint32_t)&model_buf, lppbyModel, 3, &tmp) < 3)
			while(1);
	#endif


	#if TRIGGER_WORD == 0
		int32_t n;
		n = CSpotter_GetMemoryUsage_Sep((BYTE *)lppbyModel[0], lppbyModel[1], 200);
		if (n > 1000000)
			while(1);
		hCSpotter = CSpotter_Init_Sep(lppbyModel[0], lppbyModel[1], MAX_TIME,
																	memory_pool, MEM_SIZE, lpbyState, nStateSize, &nErr);
	#else
		int32_t n;
		n = CSpotter_GetMemoryUsage_Multi((BYTE *)lppbyModel[0], (BYTE **)&lppbyModel[1], 2, 200);
		if (n > 1000000)
			while(1);
		hCSpotter = CSpotter_Init_Multi(lppbyModel[0], (BYTE **)&lppbyModel[1], 2, MAX_TIME,
																	memory_pool, MEM_SIZE, lpbyState, nStateSize, &nErr);
	#endif
		if (hCSpotter == NULL)
			while(1);
		CSpotter_Reset(hCSpotter);

	#if TRIGGER_WORD == 0
		pnWordIdx[0] = CSpotter_GetNumWord((BYTE *)lppbyModel[1]);
	#else
		pnWordIdx[0] = CSpotter_GetNumWord((BYTE *)lppbyModel[1]);
		pnWordIdx[1] = CSpotter_GetNumWord((BYTE *)lppbyModel[2]);
	#endif
		
		CSpotter_SetRejectionLevel(hCSpotter,10);	//14
}

/*********************************
* Voice Recognition Progress
*********************************/
void Voice_Recognition_Progress(void)
{
//	if (eADC_PPB == ePPB1)
		{
		for (int i = 0; i < 256; i+=256) {
			ALGSequegInputBuf =(int16_t *)(audio_buf_R + i);
			ALGSequegInputBuf1 =(int16_t *)(audio_buf_L + i);

			for(int j = 0 ; j < 2 ; j++)
			{
				/*********************************
				* Voice recognition get result
				*********************************/				
				recognition_result = Voice_Recognition_Result(j);

				if (recognition_result < pnWordIdx[0]) { // Detect trigger word.
					CSpotter_SetRejectionLevel(hCSpotter,14);
				#if RESULT_HANDLER == 1
					trigger_flag = 1;
					trigger_timer_count =1;
					LED_status = GPIO_GetPin(GPIO_PORT_3_P04);
					LED_status |= GPIO_GetPin(GPIO_PORT_3_P05)<<1;
					LED_status |= GPIO_GetPin(GPIO_PORT_3_P06)<<2;
					LED_status |= GPIO_GetPin(GPIO_PORT_3_P07)<<3;
					led_vr_time = 0;
					GPIO_SetPin(GPIO_PORT_3_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
					GPIO_SetPin(GPIO_PORT_3_P05,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
					GPIO_SetPin(GPIO_PORT_3_P06,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
					GPIO_SetPin(GPIO_PORT_3_P07,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
					while(led_vr_time < 100);
					led_vr_time = 0;
					GPIO_SetPin(GPIO_PORT_3_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
					GPIO_SetPin(GPIO_PORT_3_P05,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
					GPIO_SetPin(GPIO_PORT_3_P06,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
					GPIO_SetPin(GPIO_PORT_3_P07,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
					while(led_vr_time < 500);
					GPIO_SetPin(GPIO_PORT_3_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,LED_status&0x01);
					LED_status >>= 1;
					GPIO_SetPin(GPIO_PORT_3_P05,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,LED_status&0x01);
					LED_status >>= 1;
					GPIO_SetPin(GPIO_PORT_3_P06,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,LED_status&0x01);
					LED_status >>= 1;
					GPIO_SetPin(GPIO_PORT_3_P07,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,LED_status&0x01);
					cmd_timer_count = 0;
				#endif
				}
				else if (recognition_result >= pnWordIdx[0] && recognition_result < pnWordIdx[0]+pnWordIdx[1] && trigger_flag == 1) { // Detect command word.
				#if RESULT_HANDLER == 1		
					cmd_idx = cmd_idx-pnWordIdx[0];
					cmd_timer_count=1;			
					unsigned char pwm_level;
					switch(cmd_idx){
						case 0://open
							setting_pwm(0);
							break;
						case 1://close
							setting_pwm(4);
							break;
						case 2://fun level up
							if (is_start_motor())
							{
								pwm_level = get_motor_level();
								if (pwm_level == 1)
									break;
								else if (pwm_level == 2)
									pwm_level = 0;
								else if (pwm_level == 4)
									pwm_level = 1;
								setting_pwm(pwm_level);
							}
							break;
						case 3://fun level down
							if (is_start_motor())
							{
								pwm_level = get_motor_level();
								setting_pwm(pwm_level);
							}
							break;
						default:
							
							break;
					}
				#endif
					trigger_timer_count = 0;
				}
			}
		}
		
		// Speech Enhancement St.
	#if ALG_FUNCTION == 1
		if(ALG_FLAG ==1)
		{
			Alg_Process_PLDCore(ALGSequegInputBuf1,ALGSequegInputBuf,ALGSequegInputBuf,AEC_OutBuf);
		}
		else{ //no NR
			for(int j = 0 ; j < 256 ; j++)
				AEC_OutBuf[j] = audio_buf_L[j];
		}
	#else
		for(int j = 0 ; j < 256 ; j++)
			AEC_OutBuf[j] = audio_buf_L[j];
	#endif

	#if AGC_FUNCTION == 1
		for(int k=0;k<41;k++){
			AGC_ptr[k] = AGC_ptr_Mic[k];
		}
		AGC_GAIN_temp = SNX_AGC_Process(AEC_OutBuf, AEC_OutBuf, 256 );
		Gain_Setting(&AD_Gain_Tab[0],AD_GAIN_TAB_LEN,AGC_GAIN_temp,0);
		for(int k=0;k<41;k++){
			AGC_ptr_Mic[k] = AGC_ptr[k];
		}
	#endif
		// Speech Enhancement End
	}

#if RESULT_HANDLER == 1
	if (trigger_timer_count == 0 && trigger_flag == 1){ //trigger word LED disable
		
		trigger_flag =0;
		stop_flag=0;
		CSpotter_SetRejectionLevel(hCSpotter,10);
	}

	if(cmd_idx <= pnWordIdx[1] && cmd_timer_count == 0){ //command word LED disable
		stop_flag=0;
		switch(cmd_idx){
			case 0:
				
				break;
			case 1:
				
				break;
			case 2:
				
				break;
			case 3:
				
				break;
			case 4:
				
				break;
			case 5:
				
				break;
			default:
				
				break;
		}
	}

	if (trigger_flag == 0){ // if trigger word stop and command word not stop
	}
#endif
}

/*********************************
* Get Voice Recognition Result
*********************************/
uint32_t Voice_Recognition_Result(uint8_t n)
{
	if (CSpotter_AddSample(hCSpotter, (SHORT*)(AEC_OutBuf +(n*128)), 128) == CSPOTTER_SUCCESS) {
		int32_t nID;
		nID = CSpotter_GetResult((HANDLE)hCSpotter);
		cmd_idx = nID;
		return cmd_idx;
	}
	return 0xFF; //don't detect anything
}

__irq void SysTick_Handler(void)
{
	NVIC_ClearPendingIRQ(SysTick_IRQn);
	if (trigger_timer_count )
	{
		trigger_timer_count++;
		if (trigger_timer_count > 5000)
		{
			trigger_timer_count = 0;
		}
	}
	
	if (cmd_timer_count)
	{
		cmd_timer_count++;
		if (cmd_timer_count > 1000)
		{
			cmd_timer_count = 0;
		}			
	}	
	cnt_5ms++;
	if (cnt_5ms>=5)
	{
		is_5ms_Flag = 1;
		cnt_5ms = 0;
		interrupt_service();
	}
	led_vr_time++;
}
