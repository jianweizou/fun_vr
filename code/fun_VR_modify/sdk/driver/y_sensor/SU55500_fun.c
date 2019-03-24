#include <stdio.h>
#include "snc_y_sensor.h"
#include "snc_gpio.h"

#define SHIFR_BIT 15
#define SHIFR_TRAN_BIT 7

#define CLK_HIGH    GPIO_SetPin(GPIO_PORT_1_P08, GPIO_MODE_IGNORE, GPIO_CONFIG_IGNORE, 1)
#define CLK_LOW     GPIO_SetPin(GPIO_PORT_1_P08, GPIO_MODE_IGNORE, GPIO_CONFIG_IGNORE, 0)
#define CLK_OUTPUT  GPIO_SetPin(GPIO_PORT_1_P08, GPIO_MODE_OUTPUT, GPIO_CONFIG_IGNORE, GPIO_PIN_DATA_IGNORE)

#define SDIO_HIGH   GPIO_SetPin(GPIO_PORT_1_P09, GPIO_MODE_IGNORE, GPIO_CONFIG_IGNORE, 1)
#define SDIO_LOW    GPIO_SetPin(GPIO_PORT_1_P09, GPIO_MODE_IGNORE, GPIO_CONFIG_IGNORE, 0)
#define SDIO_OUTPUT GPIO_SetPin(GPIO_PORT_1_P09, GPIO_MODE_OUTPUT, GPIO_CONFIG_IGNORE, GPIO_PIN_DATA_IGNORE)
#define SDIO_INPUT  GPIO_SetPin(GPIO_PORT_1_P09, GPIO_MODE_INPUT, GPIO_CONFIG_IGNORE, GPIO_PIN_DATA_IGNORE)
#define SDIO_PIN    GPIO_GetPin(GPIO_PORT_1_P09)

void wait_ms(uint32_t ms)
{
    uint32_t i;
    for(i=0;i<27000*ms;i++); // 6t+2
}

void wait_2us(void)
{
    uint16_t i;
    for(i=0;i<(24+4)*2;i++);
}

void wait_4us(void)
{
    uint16_t i;
    for(i=0;i<(48+12)*2;i++);
}

uint16_t OID_CheckReadData(void)
{
    if(SDIO_PIN == 1)
    {
        return 1;
    }
    wait_2us();
    if(SDIO_PIN == 1)
    {
        return 1;
    }
    return 0;
}

void Receive_Data(uint16_t * ack_buf)
{
    uint32_t i;
    CLK_HIGH;
    wait_4us();
    SDIO_OUTPUT;
    SDIO_LOW;
    wait_2us();
    CLK_LOW;
    wait_2us();
    CLK_HIGH;
    SDIO_INPUT;
    wait_2us();

    for(i=0;i<16;i++)
    {
        CLK_HIGH;
        wait_4us();
        CLK_LOW;
        ack_buf[0] += SDIO_PIN << (SHIFR_BIT - i);
    }
    for(i=0;i<16;i++)
    {
        CLK_HIGH;
        wait_4us();
        CLK_LOW;
        ack_buf[1] += SDIO_PIN << (SHIFR_BIT - i);
    }
    for(i=0;i<16;i++)
    {
        CLK_HIGH;
        wait_4us();
        CLK_LOW;
        ack_buf[2] += SDIO_PIN << (SHIFR_BIT - i);
    }
    for(i=0;i<16;i++)
    {
        CLK_HIGH;
        wait_4us();
        CLK_LOW;
        ack_buf[3] += SDIO_PIN << (SHIFR_BIT - i);
    }
}

void Transmit_Data(uint8_t transmit_data)
{
    uint32_t i;
    SDIO_HIGH;
    SDIO_OUTPUT;//output
    CLK_HIGH;
    wait_4us();	
    CLK_LOW;

    for(i = 0; i< 8; i++)
    {
        CLK_HIGH;
        if((transmit_data>>(SHIFR_TRAN_BIT-i) & 0x1) == 0)
        {
            SDIO_LOW;
        }
        else
        {
            SDIO_HIGH;
        }
//      SDIO_PIN = (transmit_data>>(SHIFR_TRAN_BIT-i));
        wait_4us();
        CLK_LOW;
    }
    SDIO_INPUT;//input
}

uint8_t Y_Sensor_TransmitCmd(uint8_t cmd, uint16_t * ack_buf)
{
    uint32_t i;
    for(i=0;i<25;i++)
    {
        wait_4us();
    }
    if(OID_CheckReadData() == 0)
    {
        Receive_Data(ack_buf);
    }
    Transmit_Data(cmd);

    for(i=0;i<4;i++)
    {
        ack_buf[i]=0x0000;
    }
    while(OID_CheckReadData());
    Receive_Data(ack_buf);

    if((ack_buf[0] & 0xFF) == cmd+1 && ack_buf[3] == 0xFFF8)
    {
        return 0;
    }
    return 1;
}

uint8_t Write_Multi_Cmd(uint8_t cmd, uint16_t data1, uint16_t data2, uint16_t * ack_buf)
{
    uint8_t multi_cmd_buf[6];
    uint16_t cmd_num=0;
    uint16_t error;

    multi_cmd_buf[0]=5;
    multi_cmd_buf[1]=data1>>8;
    multi_cmd_buf[2]=data1;
    multi_cmd_buf[3]=data2>>8;
    multi_cmd_buf[4]=data2;
    multi_cmd_buf[5]=(multi_cmd_buf[0]+multi_cmd_buf[1]+
                                        multi_cmd_buf[2]+multi_cmd_buf[3]+multi_cmd_buf[4])&0xFF;

    error = Y_Sensor_TransmitCmd(cmd, ack_buf);

    for(cmd_num = 0;cmd_num<6;cmd_num++)
    {
        error = Y_Sensor_TransmitCmd(multi_cmd_buf[cmd_num], ack_buf);
    }
    return error;
}

uint8_t Y_Sensor_PowerOn(uint16_t * ack_buf)
{
    uint32_t i = 0;
    CLK_OUTPUT; //output
    CLK_HIGH;
    wait_ms(55);
    CLK_LOW;
    /* time out */
    while(i<0x40000)
    {
        i++;
        if(OID_CheckReadData()==0)
        {
            break;
        }
    };
    Receive_Data(ack_buf);
    if(ack_buf[0] == 0xA000 && ack_buf[3] == 0xFFF8)
    {
        return 0;
    }
    return 1;
}
