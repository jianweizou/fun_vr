#include "snc_gpio.h"
#include "motor.h"
#include "snc_timer_pwm.h"

unsigned char Motor_Level;
bool isStartMotor;
bool isMaxPWM;
unsigned int cur_Motor_PWM;
unsigned char Motor_Wakeup_cnt;
unsigned char Motor_done_cnt;
extern bool issafety;
timer_pwm_init_t    init_info = {0};
__irq void GPIO3_IRQHandler(void)
{
	if (GPIO_Irq_GetPinIrq(GPIO_PORT_3_P08))
	{
		GPIO_Irq_ClearPinIrq(GPIO_PORT_3_P08);
		Motor_done_cnt = 0;
	}
}

void InitPWM(void)
{
	Motor_Level = 0;
//    set_PWMRUN;	
	GPIO_SetPin(GPIO_PORT_0_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
	isStartMotor = 0;
	Motor_done_cnt = 0;
	
	GPIO_SetPin(GPIO_PORT_2_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
	GPIO_SetPin(GPIO_PORT_0_P02,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
}

void TurnOffMotor(void)
{
	GPIO_Irq_Deinit(GPIO_PORT_3);
//	clr_EPI;
	//turn off
	Motor_Level = 0;
	init_info.target_pwm[0].target_pwm_id   = (timer_pwm_id_t)0;
	init_info.target_pwm[1].target_pwm_id   = (timer_pwm_id_t)TIMER_PWM_IGNORE;
	init_info.target_pwm[2].target_pwm_id   = (timer_pwm_id_t)TIMER_PWM_IGNORE;
	init_info.target_pwm[0].duty_cycle      = 99;
	TimerPwm_Reset(TIMER_CT32BITS_7, &init_info);
	Timer_Launch(TIMER_CT32BITS_7);
	//dispower
	isStartMotor = 0;
	Motor_done_cnt = 0;

	GPIO_SetPin(GPIO_PORT_2_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
	GPIO_SetPin(GPIO_PORT_0_P02,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
	GPIO_SetPin(GPIO_PORT_0_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
}

unsigned char Change_Motor_PWM(void)
{
	if (Motor_Level == 0)
	{
		GPIO_SetPin(GPIO_PORT_2_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
		isStartMotor = 1;
		
		GPIO_Irq_Init(GPIO_PORT_3);
		GPIO_Irq_SetPin(GPIO_PORT_3_P08, true, GPIO_EDGE_SENSE_RISING);
		
		Motor_Level = 1;
		GPIO_SetPin(GPIO_PORT_0_P02,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,1);
		
		 *(uint32_t*)0x45000020 |= SET_BIT19;
		init_info.pclk                = 162000000;
		init_info.is_irq_one_periodic = false;
		init_info.periodic_us         = 1000/4;
		init_info.target_pwm[0].target_pwm_id   = (timer_pwm_id_t)0;
        init_info.target_pwm[0].is_irq_duty_end = false;
        init_info.target_pwm[0].duty_cycle      = 40;
		init_info.target_pwm[1].target_pwm_id   = (timer_pwm_id_t)TIMER_PWM_IGNORE;
		init_info.target_pwm[2].target_pwm_id   = (timer_pwm_id_t)TIMER_PWM_IGNORE;
		TimerPwm_Reset(TIMER_CT32BITS_7, &init_info);
		Timer_Launch(TIMER_CT32BITS_7);
		
		cur_Motor_PWM = 40;
		Motor_Wakeup_cnt = 0;
		isMaxPWM = 0;
	}
	else if (Motor_Level == 1)
	{
		Motor_Level = 2;
		//pwm mid
		init_info.target_pwm[0].duty_cycle      = 24;
		TimerPwm_Reset(TIMER_CT32BITS_7, &init_info);
		Timer_Launch(TIMER_CT32BITS_7);
	}
	else if (Motor_Level == 2)
	{
		Motor_Level = 4;
		//pwm low		
		init_info.target_pwm[0].duty_cycle      = 40;
		TimerPwm_Reset(TIMER_CT32BITS_7, &init_info);
		Timer_Launch(TIMER_CT32BITS_7);
	}
	else if (Motor_Level == 4)
	{
		TurnOffMotor();
	}
	return Motor_Level;
}
//unsigned char set_motor_level(unsigned char level)
//{
//	if (issafety == true)
//	{
//		Motor_Level = level;
//		Change_Motor_PWM();
//	}
//}
unsigned char get_motor_level(void)
{
	return Motor_Level;
}

unsigned char is_start_motor(void)
{
	return isStartMotor;
}

unsigned char cur_pwm(void)
{
	unsigned int curpwm;
	if (isMaxPWM == 1)
		return 0;
	curpwm = init_info.target_pwm[0].duty_cycle;
	if (curpwm > 30)
	{
		return 1;
	}
	else if (curpwm > 12)
	{
		return 2;
	}
	return 3;
}

unsigned char check_motor_done(void)
{
	if (isStartMotor)
	{
		if (Motor_Level == 1)
		{
			if (cur_Motor_PWM > 24)
			{
				if (Motor_done_cnt > 10)
					return 1;
			}
			else if (Motor_done_cnt > 1)
				return 1;
		}
		else if (Motor_done_cnt > 1)
			return 1;
	}
	if ((Motor_Level == 1)&&(isMaxPWM == 0))
	{
		Motor_Wakeup_cnt++;
		if (Motor_Wakeup_cnt >= 30)
		{
			Motor_Wakeup_cnt = 0;
			cur_Motor_PWM-=2;
			if (cur_Motor_PWM >= 4)
			{
				init_info.target_pwm[0].duty_cycle-=2;
				TimerPwm_Reset(TIMER_CT32BITS_7, &init_info);
				Timer_Launch(TIMER_CT32BITS_7);
			}
			else
			{
				isMaxPWM = 1;
				init_info.target_pwm[0].duty_cycle = 0;
				TimerPwm_Reset(TIMER_CT32BITS_7, &init_info);
				Timer_Launch(TIMER_CT32BITS_7);
				
				//High
//				GPIO_SetPin(GPIO_PORT_0_P04,GPIO_MODE_OUTPUT,GPIO_CONFIG_IGNORE,0);
			}
		
		}
	}
	return 0;
}
