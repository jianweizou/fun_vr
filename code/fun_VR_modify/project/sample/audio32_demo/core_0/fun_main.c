
#include "snc_gpio.h"
#include "key.h"
#include "motor.h"
#include "led.h"
#include "snc_sar_adc.h"



extern unsigned char is_5ms_Flag;
unsigned int dpdtime;
#define ADC_CNT	8
unsigned char adccnt;
unsigned int adc[ADC_CNT];
unsigned char batlevel;
unsigned int adcvalue;
extern unsigned char Motor_Level;
extern unsigned int led_display_time;
/********************************/
#define Stage_A		1
#define Stage_B		2
#define Stage_C		4
#define Stage_D		8

unsigned char system_stage;
bool isneedinitstage;
bool isneedinitbatled;
bool isneedinitsys = true;
bool ischarging;
bool isstartsystem;
unsigned char startADC_cnt;
unsigned char batlevel_led_value;
unsigned char led_type;
unsigned char adc_pre_cnt;
sar_adc_info sar_adc;
/*******************************/

void interrupt_service(void)
{
	dpdtime++;
	startADC_cnt ++;
	led_display_time++;
	if (isStartMotor)
		Motor_done_cnt++;
}

unsigned int getadcvalue(void)
{
	unsigned char i;
	unsigned int adcvalue;
	adcvalue = 0;
	for(i=0;i<ADC_CNT;i++)
	{
		adcvalue += adc[i];
	}
	adcvalue >>=3;
	return adcvalue;
}


unsigned char getbatlevel(void)
{
	unsigned char templevel;
	unsigned char i;
	unsigned char cur_pwm_level;
	if (startADC_cnt > 0)
	{
		startADC_cnt = 0;
//		while(SAR_ADC_DATA_valid());
		adcvalue=(*(uint32_t*)(0x4003A000UL + 0x08))>>6;//SAR_ADC_DATA(); 
		adc[adccnt] = adcvalue;
		adccnt++;
		if (adccnt >= ADC_CNT)
		{
			adccnt = 0;
			adcvalue = getadcvalue();
			i = get_motor_level();
			if (i == 1)
			{
				cur_pwm_level = cur_pwm();
//				if (cur_pwm_level == 0)
//					adcvalue = adcvalue + 0x30;
//				else
				if (cur_pwm_level == 1)
					adcvalue = adcvalue + 0x0C;
				else if (cur_pwm_level == 2)
					adcvalue = adcvalue + 0x14;
				else
					adcvalue = adcvalue + 0x1C;
			}
			else if (i == 2)
			{
				adcvalue = adcvalue + 0x10;
			}
			else if (i == 4)
			{
				adcvalue = adcvalue + 0x08;
			}
			if (ischarging)
			{
				if (batlevel < 3)
					adcvalue = adcvalue - 32;
				else if (batlevel < 5)
					adcvalue = adcvalue - 26;
				else
					adcvalue = adcvalue - 20;	//0x90 -> 0.3v
			}
			if (adcvalue > 840)		//100% 8.2
			{
				templevel = 6;
			}
			else if (adcvalue > 810)	//>75%	8v		
			{
				templevel = 5;
			}
			else if (adcvalue > 780)	//>50%	7.7v
			{
				templevel = 4;
			}
			else if (adcvalue > 750)	//>25%	7.5
			{
				templevel = 3;
			}
			else if (adcvalue > 730)	//10%	7.2v
			{
				templevel = 2;
			}
			else
			{
				templevel = 1;
			}
//			else if (adcvalue < 0xA00)
//			{
//				templevel = 0;
//			}
			if (batlevel != templevel)
				adc_pre_cnt++;
			else
				adc_pre_cnt = 0;
			if (adc_pre_cnt > 4)
			{
				adc_pre_cnt = 0;
				batlevel = templevel;
			}
//			batlevel_led_value = batlevel_to_led_value();
			return 1;
		}
	}	
	return 0;
}


void SysInit(void)
{
	Init_LED();
	startADC_cnt = 0;
	adccnt = 0;
	KeyInit();	
	InitPWM();
	system_stage = Stage_A;
	isneedinitstage = 1;
	led_type = 0; 
	isneedinitbatled = 0;
	batlevel_led_value = 0;
	ischarging = 0;
	adc_pre_cnt = 0;
	sar_adc.adc_2M_clk=SAR_ADC_CLK_16_2M;  
	sar_adc.adc_clock_sel=ADC_CLOCK_60K;  
	sar_adc.ad_mode=SAR_ADC_NORMAL;  
	sar_adc.channel_select=SAR_ADC_AIN1;  
	SAR_ADC_init(&sar_adc); 
 
	SAR_ADC_Enable(); 
 
	SAR_ADC_Start();
	dpdtime = 0;
	while(dpdtime<8)
	{
		getbatlevel();
	}
	isstartsystem = 1;

	dpdtime = 0;
	led_display_time = 0;
	startADC_cnt = 0;
}

void setting_pwm(unsigned char level)
{
	unsigned char i;
	if(level != 0xFF)
		Motor_Level = level;
	i  = Change_Motor_PWM();
	LED_RGB_Setting(i,0);
	if ((i == 0) && (ischarging == 0))
		LED_Setting(0,0);
	else
		LED_Setting(system_stage,batlevel);	
}

void fun_main(void)
{
	unsigned char keystatus,i;
	isstartsystem = 0;

	{
		if (isneedinitsys)
		{
			SysInit();
			isneedinitsys = 0;
		}
		if (is_5ms_Flag)
		{
			keystatus = KeyScan();
			if (system_stage == Stage_A)
			{
				if (isneedinitstage == 1)
				{
					isneedinitstage = 0;
					//init stage A
					LED_Setting(0,0);
					LED_RGB_Setting(0,0);
					led_type = 0;
					dpdtime = 0;
					ischarging = 0;
				}
				if (keystatus & 0x01)//key
				{
					//display power as LED
					led_type = 1;
					isneedinitbatled = 1;
					LED_Setting(system_stage,batlevel);
				}
				if (keystatus & 0x02)
				{
					system_stage = Stage_B;
					isneedinitstage = 1;
				}
				if (keystatus & 0x04)
				{
					if (system_stage == Stage_B)
						system_stage = Stage_D;
					else
						system_stage = Stage_C;
					isneedinitstage = 1;
				}			
			}
			else if (system_stage == Stage_B)
			{
				if (isneedinitstage == 1)
				{
					isneedinitstage = 0;
					//init stage B
					led_type = 2;	
					isneedinitbatled = 1;
					dpdtime = 0;
					ischarging = 0;			
				}
				if (keystatus & 0x01)//key
				{
					//change pwm
					setting_pwm(0xFF);
				}
				if (keystatus & 0x02)//safety
				{
				}
				else
				{
					system_stage = Stage_A;
					isneedinitstage = 1;
					//turn off pwm
					TurnOffMotor();
					LED_RGB_Setting(0,0);
					LED_Setting(0,0);
				}
				
				if (keystatus & 0x04)//charging
				{
					if (system_stage == Stage_A)
					{
						system_stage = Stage_C;
					}
					else
						system_stage = Stage_D;
//					TurnOffMotor();
//					LED_RGB_Setting(0,0);
					isneedinitstage = 1;
				}
				
				if (keystatus & 0x08)
				{
					//turn off pwm
					TurnOffMotor();
					LED_RGB_Setting(0,0);
					LED_Setting(0,0);
				}
				
			}
			else if (system_stage == Stage_C)
			{
				if (isneedinitstage == 1)
				{
					isneedinitstage = 0;
					//init stage C
					led_type = 3;	
					isneedinitbatled = 1;
					dpdtime = 0;
					ischarging = 1;
					LED_Setting(system_stage,batlevel);
					//turn off pwm
//					TurnOffMotor();
//					LED_RGB_Setting(0,0);
				}
				if (keystatus & 0x01)//key
				{
					
				}
				if (keystatus & 0x02)//safety
				{
					//enter stage_B
					system_stage = Stage_D;
					isneedinitstage = 1;
				}
				
				if (keystatus & 0x04)//charging
				{
					dpdtime = 0;
				}
				else
				{
					if (system_stage == Stage_D)
					{
						system_stage = Stage_B;
					}
					else
					{
						system_stage = Stage_A;
						LED_Setting(0,0);
					}
					isneedinitstage = 1;
				}
			}
			else if (system_stage == Stage_D)
			{
				if (isneedinitstage == 1)
				{
					isneedinitstage = 0;
					isneedinitbatled = 1;
					//init stage D
					led_type = 3;
					dpdtime = 0;
					ischarging = 1;
					LED_Setting(system_stage,batlevel);
				}
				if (keystatus & 0x01)//key
				{
					//change pwm
//					i  = Change_Motor_PWM();
//					LED_RGB_Setting(i,0);
					setting_pwm(0xFF);
				}
				if (keystatus & 0x02)//safety
				{
				}
				else
				{
					system_stage = Stage_C;
					isneedinitstage = 1;
					TurnOffMotor();
					LED_RGB_Setting(0,0);
				}
				
				if (keystatus & 0x04)//charging
				{
					dpdtime = 0;
				}
				else
				{
					if (system_stage == Stage_C)
					{
						system_stage = Stage_A;
					}
					else
					{
						system_stage = Stage_B;
						i = get_motor_level();
						LED_RGB_Setting(i,0);
						if (i == 0)
							LED_Setting(0,0);
						else
							LED_Setting(system_stage,batlevel);
					}
					isneedinitstage = 1;	
//					TurnOffMotor();	
//					LED_RGB_Setting(0,0);
				}
				
				if (keystatus & 0x08)
				{
					//turn off pwm
					TurnOffMotor();
					LED_RGB_Setting(0,0);
				}
			}
			
			//ADC process
			getbatlevel();

			//pwm rate
			if (check_motor_done())
			{
				//turn off pwm
				TurnOffMotor();
				LED_RGB_Setting(0,0);
				LED_Setting(0,0);
				//goto stage A,B
				#warning "change stage to A or B"
			}
			if(get_motor_level())
			{
				dpdtime = 0;
			}
			//LED_Process		
			LED_Process(system_stage,batlevel_led_value);
			
			//dpd
//			if (dpdtime >= 2000)
//			{
//				dpdtime = 0;
//				//enter dpd
//				//
//				Set_All_GPIO_Quasi_Mode;
//				clr_ADCEN;
//				
//				TurnOffMotor();
//				LED_WHITE_Setting(0,0);
//				LED_RGB_Setting(0,0);
//				DeInit_LED();
//				
//				P17_Input_Mode;
//				set_P1S_7;
//				set_EX1;
//								
//				set_PD;

//				PICON  = 0;
//							
//				clr_EX0;
//				clr_EX1;
//				clr_EPI;
//				isneedinitsys = 1;
//			}
		}
	}
}

