#ifndef _VR_UART_H_
#define _VR_UART_H_

#define VR_UART	1

#define VR_MODE "VA_"


typedef enum _t_vr_pid
{
	VR_Setting=0x1,
	VR_Checking=0x2,
	VR_Responsing=0x0F,
}vr_pid_t;

typedef enum _t_vr_type
{
	vr_color=1,
	vr_pwm,
	vr_mode,
}vr_type_t;


typedef struct _t_vr_cmd
{
	vr_type_t vr_type;
	unsigned char value;
}vr_cmd_t;

typedef struct _t_vr_data
{
	unsigned char vr_data_head;
	unsigned char vr_data_addr;
	vr_pid_t vr_data_pid;
	unsigned char vr_data_len;
	vr_cmd_t vr_data_cmd;
	unsigned char vr_data_checksum;
}vr_data_t;
void send_vr_info_init(void);
void send_vr_info(unsigned char tir_cmd,unsigned char index);
void send_vr_data(vr_pid_t vr_pid,vr_type_t vr_type,unsigned char value);

#endif
