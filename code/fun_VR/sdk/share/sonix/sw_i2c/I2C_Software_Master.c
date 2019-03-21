
#include "I2C_Software_Master.h"

#define SN_GPIO0                        ((sn_gpio_t           *) SN_GPIO_0_BASE)
#define SN_GPIO1                        ((sn_gpio_t           *) SN_GPIO_1_BASE)
#define SN_GPIO2                        ((sn_gpio_t           *) SN_GPIO_2_BASE)
#define SN_GPIO3                        ((sn_gpio_t           *) SN_GPIO_3_BASE)
#define SN_GPIO4                        ((sn_gpio_t           *) SN_GPIO_4_BASE)



/***************************************************************/
static uint8_t sn_i2c_sw_ch = eSN_I2C_CH1;
void I2C_ch_sel(sn_i2c_sw_ch_t sel)
{
    sn_i2c_sw_ch = sel;
}

static uint32_t *I2C_ch_data_table[3]=
{
    (uint32_t*)&(SN_GPIO0->DATA),
    (uint32_t*)&(SN_GPIO3->DATA),
    (uint32_t*)&(SN_GPIO4->DATA),
};

static uint32_t *I2C_ch_mode_table[3]=
{
    (uint32_t*)&(SN_GPIO0->MODE),
    (uint32_t*)&(SN_GPIO3->MODE),
    (uint32_t*)&(SN_GPIO4->MODE),
};

static uint32_t I2C_scl_table[3]=
{
    SET_BIT0,
    SET_BIT2,
    SET_BIT8,
};

static uint32_t I2C_sda_table[3]=
{
    SET_BIT1,
    SET_BIT3,
    SET_BIT9,
};

#define SCL_H               *I2C_ch_data_table[sn_i2c_sw_ch] |= I2C_scl_table[sn_i2c_sw_ch]
#define SCL_L               *I2C_ch_data_table[sn_i2c_sw_ch] &= ~I2C_scl_table[sn_i2c_sw_ch]
#define SDA_H               *I2C_ch_data_table[sn_i2c_sw_ch] |= I2C_sda_table[sn_i2c_sw_ch]
#define SDA_L               *I2C_ch_data_table[sn_i2c_sw_ch] &= ~I2C_sda_table[sn_i2c_sw_ch]

#define ACK_READ_INIT       *I2C_ch_mode_table[sn_i2c_sw_ch] &= ~I2C_sda_table[sn_i2c_sw_ch]
#define ACK_READ_DEINIT     *I2C_ch_mode_table[sn_i2c_sw_ch] |= I2C_sda_table[sn_i2c_sw_ch]

#define SCL_READ            SCL_rd()
#define SDA_READ            SDA_rd()

#define I2C_SW_INIT         *I2C_ch_mode_table[sn_i2c_sw_ch] |= I2C_scl_table[sn_i2c_sw_ch];    \
                            *I2C_ch_mode_table[sn_i2c_sw_ch] |= I2C_sda_table[sn_i2c_sw_ch];    \

uint32_t SCL_rd(void)
{
    uint32_t i=0;
    i = *I2C_ch_data_table[sn_i2c_sw_ch] & I2C_scl_table[sn_i2c_sw_ch];
    if(i)
    {
        while(1)
        {
            if( i & SET_BIT0 )
            {
                return i;
            }
            i >>= 1;
        }
    }
    return i;
}

uint32_t SDA_rd(void)
{
    uint32_t i=0;
    i = *I2C_ch_data_table[sn_i2c_sw_ch] & I2C_sda_table[sn_i2c_sw_ch];
    if(i)
    {
        while(1)
        {
            if( i & SET_BIT0 )
            {
                return i;
            }
            i >>= 1;
        }
    }
    return i;
}

/***************************************************************/

//#define SCL_H         SN_GPIO0->DATA_b.DATA0=1
//#define SCL_H         SN_GPIO0->BSET_b.BSET0=1

//#define SCL_L         SN_GPIO0->DATA_b.DATA0=0
//#define SCL_L         SN_GPIO0->BCLR_b.BCLR0=1


//#define SDA_H         SN_GPIO0->DATA_b.DATA1=1
//#define SDA_H         SN_GPIO0->BSET_b.BSET1=1

//#define SDA_L         SN_GPIO0->DATA_b.DATA1=0
//#define SDA_L         SN_GPIO0->BCLR_b.BCLR1=0

#define I2C_DIRECTION_TRANSMITTER       ((uint8_t)0x00)
#define I2C_DIRECTION_RECEIVER          ((uint8_t)0x01)



void I2C_SoftWare_Master_Init(void)
{
    //GPIO_SetPin(GPIO_PORT_0_P00,GPIO_MODE_OUTPUT,GPIO_CONFIG_INACTIVE,1);
    //GPIO_SetPin(GPIO_PORT_0_P01,GPIO_MODE_OUTPUT,GPIO_CONFIG_INACTIVE,1);
    I2C_SW_INIT;
        
    
    SCL_L;
    SCL_H;
    
    SCL_H;
    SDA_H;
    
    SCL_L;
    SDA_L;
    
    SCL_H;
    SDA_H;
}

void ACK_read_init(void)
{
    //GPIO_SetPin(GPIO_PORT_0_P00,GPIO_MODE_INPUT,GPIO_CONFIG_PULL_UP_ON,1);
    //SN_GPIO0->MODE_b.MODE1=GPIO_MODE_INPUT;    
    ACK_READ_INIT;
}    

void ACK_read_deinit(void)
{    
    //SN_GPIO0->MODE_b.MODE1=GPIO_MODE_OUTPUT;  
    ACK_READ_DEINIT;
}

uint32_t SCL_read(void)
{
    //return GPIO_GetPin(GPIO_PORT_0_P00);
    return SCL_READ;
}

uint32_t SDA_read(void)
{
    //return GPIO_GetPin(GPIO_PORT_0_P01);
    return SDA_READ;
}    

/* ======================================================================================================== */

void I2C_delay(void)
{
    volatile int i = 10;		  
    while (i){
        i--;
        __asm("nop");
    }
}

void I2C_delay_C(void)
{
    volatile int i = 10;		  
    while (i){
        i--;
        __asm("nop");
    }
}

void I2C_delay_2(void)
{
    volatile int i = 10;		  
    while (i){
        i--;
        __asm("nop");
    }
}

void I2C_delay_3(void)
{
    volatile int i = 10;		  
    while (i){
        i--;
        __asm("nop");
    }
}

uint8_t I2C_Start(void)
{
    SDA_H;
    SCL_H;
    I2C_delay();
    if (!SDA_read())
        return 0;
    SDA_L;
    I2C_delay();
    if (SDA_read())
        return 0;
    SCL_L;
    I2C_delay();
    return 1;
}

void I2C_Stop(void)
{
    SCL_L;
    I2C_delay();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SDA_H;
    I2C_delay();
}


void I2C_Ack(void)
{
    SCL_L;
    I2C_delay();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}

void I2C_NoAck(void)
{
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}

uint8_t I2C_WaitAck(void)
{
    ACK_read_init();  
    
    SCL_L;
    I2C_delay();
    SDA_H;
    if (SDA_read())
    {
        SCL_H;
        I2C_delay();
        return 0;
    }
    SCL_H;
    I2C_delay();
    
    ACK_read_deinit(); 
    
    return 1;
}

void I2C_SendByte(uint8_t byte)
{
    uint8_t i = 8;
    while (i--)
    {
        SCL_L;
        I2C_delay();//
        if (byte & 0x80)
            SDA_H;
        else
            SDA_L;
        I2C_delay();
        byte <<= 1;
        I2C_delay();
        SCL_H;
        I2C_delay();
    }
    SCL_L;
}

void I2C_SendByte2(uint8_t byte)
{
    uint8_t i = 8;
    while (i--)
    {
        SCL_L;
        //I2C_delay();
        if (byte & 0x80)
            SDA_H;
        else
            SDA_L;
        I2C_delay();//add to test
        I2C_delay();
        byte <<= 1;
        I2C_delay();
        SCL_H;
        I2C_delay();
    }
    SCL_L;
}


uint8_t I2C_ReceiveByte(void)
{
    uint8_t i = 8;
    uint8_t byte = 0;

    ACK_read_init();/* set SDA to input */
    
    while (i--) 
    {
        byte <<= 1;
        SCL_L;
        I2C_delay();
        SCL_H;
        I2C_delay();
        if (SDA_read()) 
        {
            byte |= 0x01;
        }
    }
    SCL_L;
    
    ACK_read_deinit();/* set SDA to input */
        
    return byte;
}

//int I2C_SoftWare_Master_Write(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
int I2C_SoftWare_Master_Write(uint8_t DeviceAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    int i;
    if (!I2C_Start())
        return I2C_SoftWare_Master_ReInit();
    
    I2C_delay();
    I2C_SendByte((DeviceAddr<<1) | I2C_DIRECTION_TRANSMITTER);
    if (!I2C_WaitAck())
    {
        //I2C_Stop();
        return -1;
    }
    
    I2C_delay();
   
    for (i = 0; i < NumByteToWrite; i++) 
    {
        I2C_SendByte(pBuffer[i]);
        if (!I2C_WaitAck()) 
        {
            //I2C_Stop();
            //return I2C_SoftWare_Master_ReInit();
            return -1;
        }
    }

    return 0; 
}

int I2C_Start_Master_read(void)
{
        I2C_delay();
        
        SCL_L;
        
        I2C_delay();
        
        if(!SCL_read())
        {
            SDA_H;            
        }else
        {        
            return 1;    
        }
        
        I2C_delay();
           
        if(!(SCL_read()&SDA_read()))      
        {
                I2C_delay();
        }
        
        I2C_Start();
                
        return 0;
}    

int I2C_SoftWare_Master_Read(uint8_t DeviceAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    //uint8_t tmp_addr = DeviceAddr;
    
    I2C_delay();  
    
    I2C_Start_Master_read();

    I2C_SendByte((DeviceAddr<<1) | I2C_DIRECTION_RECEIVER);
    
    if (!I2C_WaitAck())
    {
        //I2C_Stop();
        return 1;
    }
    
    while (NumByteToRead) 
    {
        *pBuffer = I2C_ReceiveByte();
        if (NumByteToRead == 1)
            I2C_NoAck();
        else
            I2C_Ack();
        pBuffer++;
        NumByteToRead--;
    }
    
    //I2C_Stop();
    
    return 0;
}

int I2C_SoftWare_Master_ReInit(void)
{
    I2C_SoftWare_Master_Init();
  
    return -1;
}
  
