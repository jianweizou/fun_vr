#include "uart_send_data.h"
#include "snc_uart.h"
#include "snc_log_util.h"

#define VR_AEC_SYS_VERSION "181226A"

vr_data_t vr_data={
	.vr_data_head = 0xA8,
	.vr_data_addr = 0x3D,
	.vr_data_pid = VR_Setting,
	.vr_data_len = 3,
	.vr_data_cmd.vr_type = vr_color,
	.vr_data_cmd.value = 0x00,
	.vr_data_checksum = 0x00,
};


log_driver_init_t  init_log = {0};
void send_vr_info_init(void)
{	
	init_log.uart.sys_clock = 162000000;
	init_log.uart.baud_rate = 9600;	
	if (LOG_Driver_Init(&init_log) != 0)
		while(1);
}
/*
void send_vr_info(unsigned char tir_cmd,unsigned char index)
{
	unsigned char buf[20];
	unsigned char i;
	sprintf((char*)buf,"%s","VA_");
	sprintf((char*)(buf+3),"%s",tir_cmd?"C":"T");
	buf[4] = '_';
	buf[5] = index;
	buf[6] = '_';
	sprintf((char*)(buf+7),"%s",VR_AEC_SYS_VERSION);
	buf[14] = '_';
	buf[15] = 0;
	for(i=0;i<15;i++)
		buf[15]+=buf[i];
	buf[16] = 0;
	UART_WriteMultiBytes(init_log.uart.port,buf,16);
}
*/

void send_vr_data(vr_pid_t vr_pid,vr_type_t vr_type,unsigned char value)
{
	vr_data.vr_data_pid = vr_pid;
	vr_data.vr_data_cmd.vr_type = vr_type;
	vr_data.vr_data_cmd.value = value;
	vr_data.vr_data_checksum = vr_data.vr_data_pid + vr_data.vr_data_len + vr_data.vr_data_cmd.value + vr_data.vr_data_cmd.vr_type;
	vr_data.vr_data_checksum = ~vr_data.vr_data_checksum;
	UART_WriteMultiBytes(init_log.uart.port,(unsigned char*)&vr_data,7);
}	

