
#include "snc_gpio.h"
#include "led.h"
#include "motor.h"

#define LED1_OUTPUT		GPIO_SetPin(GPIO_PORT_3_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)
#define LED2_OUTPUT		GPIO_SetPin(GPIO_PORT_3_P05,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)
#define LED3_OUTPUT		GPIO_SetPin(GPIO_PORT_3_P06,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)
#define LED4_OUTPUT		GPIO_SetPin(GPIO_PORT_3_P07,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)
#define LED_R_OUTPUT	GPIO_SetPin(GPIO_PORT_3_P09,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)
#define LED_G_OUTPUT	GPIO_SetPin(GPIO_PORT_2_P08,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)
#define LED_B_OUTPUT	GPIO_SetPin(GPIO_PORT_2_P07,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0)

#define LED1_SETTING(x)		GPIO_SetPin(GPIO_PORT_3_P04,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)
#define LED2_SETTING(x)		GPIO_SetPin(GPIO_PORT_3_P05,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)
#define LED3_SETTING(x)		GPIO_SetPin(GPIO_PORT_3_P06,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)
#define LED4_SETTING(x)		GPIO_SetPin(GPIO_PORT_3_P07,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)
#define LED_R_SETTING(x)	GPIO_SetPin(GPIO_PORT_3_P09,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)
#define LED_G_SETTING(x)	GPIO_SetPin(GPIO_PORT_2_P08,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)
#define LED_B_SETTING(x)	GPIO_SetPin(GPIO_PORT_2_P07,GPIO_MODE_IGNORE,GPIO_CONFIG_IGNORE,x)

extern unsigned char is_5ms_Flag;
unsigned char led_stage=0;
unsigned char led_white_mode;
bool led_rgb_mode;
bool led_white_tog;
unsigned int led_display_time = 0;
extern unsigned char batlevel;
void Init_LED(void)
{
	LED1_OUTPUT;
	LED2_OUTPUT;
	LED3_OUTPUT;
	LED4_OUTPUT;
	LED1_SETTING(0);
	LED2_SETTING(0);
	LED3_SETTING(0);
	LED4_SETTING(0);	
	
	LED_R_OUTPUT;
	LED_G_OUTPUT;
	LED_B_OUTPUT;
	
	LED_R_SETTING(0);
	LED_G_SETTING(0);
	LED_B_SETTING(0);
	
	led_white_mode = 0;
	led_rgb_mode = 0;
}

void DeInit_LED(void)
{
//	P13_OpenDrain_Mode;
//	P00_OpenDrain_Mode;
//	P01_OpenDrain_Mode;
//	P03_OpenDrain_Mode;
//	P10_OpenDrain_Mode;
//	P11_OpenDrain_Mode;
//	P15_OpenDrain_Mode;
}
/*
led_type:	LED Or RGB(0:1)
led_value: led value
led_mode:	Normal or Flash(0:1)
*/
void LED_WHITE_Set_value(unsigned char led_value)
{
	LED1_SETTING(led_value&LED1);
	led_value = led_value >> 1;
	LED2_SETTING(led_value&LED1);
	led_value = led_value >> 1;
	LED3_SETTING(led_value&LED1);
	led_value = led_value >> 1;
	LED4_SETTING(led_value&LED1);	
}
void LED_WHITE_Setting(unsigned char led_value,unsigned char led_mode)
{
//	led_white_mode = led_mode;
	LED_WHITE_Set_value(led_value);
	led_display_time = 0;
	led_white_tog = 0;
}

void LED_RGB_Set_value(unsigned char led_value)
{
//	LED_R_SETTING(led_value&LEDR);
//	#warning "need update RGB-LED "
//	if (led_value&LEDR)
//	{
//		LED_G_SETTING(1);
//		LED_B_SETTING(1);
//	}
//	else
//	{
//		led_value = led_value >> 1;
//		LED_B_SETTING(led_value&LEDR);
//		led_value = led_value >> 1;
//		LED_G_SETTING(led_value&LEDR);	
//	}
		
		LED_R_SETTING(led_value&LEDR);
		led_value = led_value >> 1;
		LED_B_SETTING(led_value&LEDR);	
		led_value = led_value >> 1;	
		LED_G_SETTING(led_value&LEDR);
}
void LED_RGB_Setting(unsigned char led_value,unsigned char led_mode)
{
	led_rgb_mode = led_mode;
	LED_RGB_Set_value(led_value);
}

unsigned char batlevel_to_led_value(unsigned char stage)
{
	unsigned char val;
	if (stage == 1)//stage A
	{
		if (batlevel >= 5)
		{
			val = 0x0F;
		}
		else if (batlevel == 4)
		{
			val = 0x07;
		}
		else if (batlevel == 3)
		{
			val = 0x03;
		}
		else if (batlevel == 2)
		{
			val = 0x01;
		}			
	}
	else if (stage == 4)//stage C
	{
		if (batlevel == 6)	//100%
			val = 0x0F;
		else if (batlevel == 5)	//>75%
		{
			val = 0x07;
		}
		else if (batlevel == 4)	//50%
		{
			val = 0x03;
		}
		else if (batlevel == 3)	//25
		{
			val = 0x01;
		}
		else
			val = 0;
	}
	else if (stage == 2)//stage B
	{
		if (batlevel >= 5)	//>75%
		{
			val = 0x0F;
		}
		else if (batlevel == 4)//>50%
		{
			val = 0x07;
		}
		else if (batlevel == 3)//>25%
		{
			val = 0x03;
		}
		else if (batlevel == 2)//>10%
		{
			val = 0x01;
		}
		else if (batlevel == 1)//<10%
		{
			val = 0x00;
		}
	}
	else if (stage == 8)//stage D
	{
		if (batlevel == 6)
		{
			val = 0x0F;
		}
		else if (batlevel == 5)	//>75%
		{
			val = 0x07;
		}
		else if (batlevel == 4)//>50%
		{
			val = 0x03;
		}
		else if (batlevel == 3)//>25%
		{
			val = 0x01;
		}
		else	//<25%
		{
			val = 0;
		}
	}
	return val;
}

void LED_Setting(unsigned char stage,unsigned char batlevel)
{
	unsigned char temp;
	led_stage = stage;
	led_display_time = 0;
	if (led_stage == 0)
	{
		LED_WHITE_Setting(0,0);
	}
	else if (led_stage == 1)	//stage_A
	{
		led_white_mode = 0;
		temp = batlevel_to_led_value(led_stage);
		LED_WHITE_Setting(temp,0);
	}
	else if (led_stage == 4)//stage C
	{
		led_white_mode = 0;
		temp = batlevel_to_led_value(led_stage);
		LED_WHITE_Setting(temp,0);
	}
	else if (led_stage == 2)//stage B
	{
		led_white_mode = 0;
		temp = batlevel_to_led_value(led_stage);
		if (temp == 0)
			LED_WHITE_Setting(1,0);
		else
			LED_WHITE_Setting(temp,0);
	}
	else if (led_stage == 8)//stage D
	{
		led_white_mode = 0;
		temp = batlevel_to_led_value(led_stage);
		LED_WHITE_Setting(temp,0);
	}
}

void LED_Process(unsigned char stage,unsigned char led_value)
{
	unsigned char temp;
	if (led_stage == 1)	//stage
	{
		if (led_display_time >= 600)
		{
			led_stage = 0;
			LED_WHITE_Setting(0,0);
		}
	}
	else if (led_stage == 4)//stage C
	{
		if (led_display_time >= 100)
		{
			led_display_time = 0;
			if (led_white_tog == 0)
			{
				led_white_tog = 1;
				led_display_time = 1;
				temp = batlevel_to_led_value(led_stage);
				temp = (temp<<1) + 1;
				LED_WHITE_Set_value(temp);
			}
			else
			{
				led_white_tog = 0;
				led_display_time = 0;
				temp = batlevel_to_led_value(led_stage);
				LED_WHITE_Set_value(temp);
			}
		}		
	}
	else if (led_stage == 2)//stage B
	{
		if (led_display_time >= 50)
		{
			led_display_time = 0;
			if (led_white_tog == 0)
			{
				led_white_tog = 1;
				led_display_time = 1;
				temp = batlevel_to_led_value(led_stage);
				if (temp == 0)
				{
					temp = (temp<<1) + 1;
				}				
				LED_WHITE_Set_value(temp);
			}
			else
			{
				led_white_tog = 0;
				led_display_time = 0;
				temp = batlevel_to_led_value(led_stage);
				LED_WHITE_Set_value(temp);
			}
		}		
	}
	else if (led_stage == 8)//stage D
	{
		if (led_display_time >= 100)
		{
			led_display_time = 0;
			if (led_white_tog == 0)
			{
				led_white_tog = 1;
				led_display_time = 1;
				temp = batlevel_to_led_value(led_stage);
				temp = (temp<<1) + 1;
				LED_WHITE_Set_value(temp);
			}
			else
			{
				led_white_tog = 0;
				led_display_time = 0;
				temp = batlevel_to_led_value(led_stage);
				LED_WHITE_Set_value(temp);
			}
		}		
	}
}
