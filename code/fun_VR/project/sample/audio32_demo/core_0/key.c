#include "snc_gpio.h"
#include "key.h"

#define Fun_KEY_INPUT	GPIO_SetPin(GPIO_PORT_4_P05,GPIO_MODE_INPUT,GPIO_CONFIG_PULL_UP_ON,0)
#define Fun_KEY_P17		GPIO_GetPin(GPIO_PORT_4_P05)
#define Safety_KEY_INPUT	GPIO_SetPin(GPIO_PORT_0_P01,GPIO_MODE_INPUT,GPIO_CONFIG_PULL_UP_ON,0)
#define Safety_KEY		GPIO_GetPin(GPIO_PORT_0_P01)
#define Charging_KEY_INPUT	GPIO_SetPin(GPIO_PORT_2_P05,GPIO_MODE_INPUT,GPIO_CONFIG_PULL_UP_ON,0)
#define Charging_KEY	GPIO_GetPin(GPIO_PORT_2_P05)

unsigned char curKey,preKey;
unsigned int debounce;
unsigned char Safety_key_debounce;
unsigned char Charging_key_debounce;
extern char is_5ms_Flag;

bool iswaitrelease;

void KeyInit(void)
{
	curKey = 0;
	preKey = 0;
	debounce = 0;
	iswaitrelease = 0;
	Safety_key_debounce = 0;
	Charging_key_debounce = 0;
	Fun_KEY_INPUT;
	Safety_KEY_INPUT;
	Charging_KEY_INPUT;
}

char KeyScan(void)
{
	unsigned char keystatus ;
	keystatus = 0;
	if (is_5ms_Flag)
	{
		is_5ms_Flag = 0;
		if (iswaitrelease == 0)
			curKey = Fun_KEY_P17;
		else
		{
			if (Fun_KEY_P17 == 1)
			{
				iswaitrelease = 0;
				debounce = 0;
			}
			curKey = preKey;
		}
		if (curKey != preKey)
		{
			preKey = curKey;/*
			if (debounce>=400)
			{
				keystatus = 0x08;
			}
			else*/ if (debounce > 10)
			{
				keystatus = 1;
			}	
			debounce = 0;
		}
		else if(curKey == 0)
		{
			debounce++;
			if (debounce >= 500)
			{
				debounce = 500;
			}
			else if (debounce >= 400)
			{
				debounce = 500;
				keystatus = 0x08;				
				iswaitrelease = 1;
			}
		}
		
		if (Safety_KEY)
		{
			Safety_key_debounce = 0;
		}
		else
		{
			Safety_key_debounce++;
			if (Safety_key_debounce > 20)
			{
				Safety_key_debounce = 20;
				keystatus |= 0x02;
			}
		}
		
		if (Charging_KEY)
		{
			Charging_key_debounce = 0;
		}
		else
		{
			Charging_key_debounce++;
			if (Charging_key_debounce > 20)
			{
				Charging_key_debounce = 20;
				keystatus |= 0x04;
			}
		}
	}
	return keystatus;
}
