
extern bool isStartMotor;
extern unsigned char Motor_done_cnt;
extern void InitPWM(void);
extern void TurnOffMotor(void);
extern unsigned char Change_Motor_PWM(void);
extern void MOTOR_FG_PinInterrupt_ISR (void);
extern unsigned char check_motor_done(void);
extern unsigned char get_motor_level(void);
extern unsigned char cur_pwm(void);

