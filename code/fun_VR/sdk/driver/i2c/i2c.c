/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file i2c.c
 *
 * @author chia-hao Hsu
 * @version 0.1
 * @date 2018/04/20
 * @license
 * @description
 */
#include "snc_i2c.h"
#include "register_7320.h"
#define I2C_VERSION        0x73200000


//=============================================================================
//                  Constant Definition
//=============================================================================
/* Define Test for I2C */
#define Test_GPIO_EN    0
/* Driver Test case */
#define TIME_OUT_EN     0

// TODO: delete when ic verify is finish.
#if Test_GPIO_EN    
    #include "snc_gpio.h"
#endif

/*Define Case*/
#define NONE                                0
#define MASTER_TX                       1
#define MASTER_RX                       2
#define SLAVE_TX                        3   
#define SLAVE_RX                        4
#define SLAVE_TX_FIFO_MODE  5
#define SLAVE_RX_FIFO_MODE  6
#define MASTER_TX_10BIT     7
#define MASTER_RX_10BIT     8

/*FIFO Mode TEST EN*/
#define ENABLE          1
#define DISABLE         0
/*FIFO DRIVER ENABLE*/
#define SLAVE_FIFO_DRIVER                ENABLE
/*FIFO Mode Slave Rx FIFO Test option*/
#define SLAVE_RX_FIFO_TEST_EMPTY            DISABLE
#define SLAVE_RX_FIFO_TEST_FULL             DISABLE
#define SLAVE_RX_FIFO_TEST_STOP             DISABLE
#define SLAVE_RX_FIFO_STOP_CNT              5

#define SLAVE_RX_FIFO_TEST_OVERFLOW     DISABLE
#define SLAVE_RX_FIFO_ACT_SEL                   SLAVE_RX_AUTO_NACK
#define SLAVE_RX_FIFO_OVERFLOW_CNT      5
#define SLAVE_RX_FIFO_TEST_OVERFLOW_RD_BY_EMPTY     DISABLE
#define SLAVE_RX_FIFO_TEST_OVERFLOW_RD_BY_FULL      DISABLE

/*FIFO Mode Slave Tx FIFO Test option*/
#define SLAVE_TX_FIFO_TEST_EMPTY            DISABLE
#define SLAVE_TX_FIFO_TEST_FULL             DISABLE
#define SLAVE_TX_FIFO_TEST_OVERFLOW     DISABLE
#define SLAVE_TX_FIFO_ACT_SEL                   SLAVE_TX_SEND_NONE
#define SLAVE_TX_FIFO_OVERFLOW_CNT      5

/*I2C SlaveFIFO Mode (_I2C_SlaveFIFO_EN)*/
#define	I2C_RX_FIFO_MODE		0
#define	I2C_TX_FIFO_MODE		1

/*I2C Slave Rx FIFO Full & Slave Tx FIFO Empty Action define (_I2C_SlaveFIFO_EN)*/
#define	SLAVE_RX_AUTO_NACK	0
#define	SLAVE_TX_SEND_NONE	0
#define	WAIT_STATE					1

/*I2C Table*/
#define	I2C0		(sn_i2c_t*) 0x40010000UL
#define	I2C1		(sn_i2c_t*) 0x40011000UL
#define	I2C2		(sn_i2c_t*) 0x40012000UL

/**
 * support system clock
 */

/**
 * register mask
 */

/**
 * test option
 */

//=============================================================================
//                  Macro Definition
//=============================================================================
static void _I2C_Enable(sn_i2c_t *I2C_SEL);
static void _I2C_Address_Setting(sn_i2c_t *I2C_SEL, uint8_t ucAddr_mode, uint8_t ucGeneral_En, uint16_t usSlv_Addr0, uint16_t usSlv_Addr1, uint16_t usSlv_Addr2, uint16_t usSlv_Addr3);
static void _I2C_SlaveRxFIFO_Start(i2c_id_t i2c_id);
static void _i2c_time_out_init(void);
static uint32_t _i2c_time_out(void);
//static void _delay_ms(uint32_t us);
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t g_timeout_count;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static sn_i2c_t *_I2C_Get_Handle(i2c_id_t i2c_id)
{
    sn_i2c_t *pDev = 0;

    if( i2c_id == I2C_0 )
    {
        pDev = (sn_i2c_t *)SN_I2C0_BASE;
    }
    else if( i2c_id == I2C_1 )
    {
        pDev = (sn_i2c_t *)SN_I2C1_BASE;
    }
    else if( i2c_id == I2C_2 )
    {
        pDev = (sn_i2c_t *)SN_I2C2_BASE;
    }

    return pDev;
}

#if 0
/*****************************************************************************
* Function      : _Delay_ms
* Description   :   
* Input           : uint32_t us
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
static void _delay_ms(uint32_t us)
{
	uint32_t i,j;
	
	for(j=0; j<10; j++){
		for(i=0; i<us*2; i++){
			__nop();
			__nop();
			__nop();
			
		}
	}

}
#endif
// time out

static void _i2c_time_out_init(void)
{
    g_timeout_count=0;
} 

static uint32_t _i2c_time_out(void)
{
    g_timeout_count++;
    
    if(g_timeout_count == 0xffffffffffffff)
    //if(g_timeout_count == 0xffff)
    {
       return 1; 
    }   
    
    return 0;
}    


/*****************************************************************************
* Function      : _I2C_Address_Setting
* Description   : Set I2C Slave Address 0~3
* Input           : Addr_mode:  0:  7-bit address mode
                            1: 10-bit address mode
                              General_En: 0: Disable General call
                                            1: Enable General call
                                Slv_Addr0:  I2C Slave Address 0
                                Slv_Addr1:  I2C Slave Address 1
                                Slv_Addr2:  I2C Slave Address 2
                                Slv_Addr3:  I2C Slave Address 3
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
static void _I2C_Address_Setting(sn_i2c_t *I2C_SEL, uint8_t ucAddr_mode, uint8_t ucGeneral_En, uint16_t usSlv_Addr0, uint16_t usSlv_Addr1, uint16_t usSlv_Addr2, uint16_t usSlv_Addr3)
{ 
    if(ucAddr_mode == 1)
    {
        I2C_SEL->SLVADDR0_b.ADD_MODE = 1;
        I2C_SEL->SLVADDR0_b.ADDR = usSlv_Addr0;
        I2C_SEL->SLVADDR1_b.ADDR = usSlv_Addr1;
        I2C_SEL->SLVADDR2_b.ADDR = usSlv_Addr2;
        I2C_SEL->SLVADDR3_b.ADDR = usSlv_Addr3;
    }
    else
    {
        I2C_SEL->SLVADDR0_b.ADD_MODE = 0;
        I2C_SEL->SLVADDR0_b.ADDR = usSlv_Addr0 << 1;      
        I2C_SEL->SLVADDR1_b.ADDR = usSlv_Addr1 << 1;
        I2C_SEL->SLVADDR2_b.ADDR = usSlv_Addr2 << 1;
        I2C_SEL->SLVADDR3_b.ADDR = usSlv_Addr3 << 1;
    }

    I2C_SEL->SLVADDR0_b.GCEN = ucGeneral_En & 0x01;

}

/*****************************************************************************
* Function      : I2C_Set_Start_Condition
* Description   : I2C Controller Send Start Condition
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
void I2C_Set_Start_Condition(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.STA = 1;
    //while((I2C_SEL->STAT&0x8030)!=0x8030);
}

/*****************************************************************************
* Function      : _I2C_Clear_Flag
* Description   : Clear Interrupt Flag
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
void I2C_Clear_Flag(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->STAT_b.I2CIF = 1;
}

/*****************************************************************************
* Function      : _I2C_Enable
* Description   : I2C Controller Enable
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
static void _I2C_Enable(sn_i2c_t *I2C_SEL)
{
    I2C_SEL->CTRL_b.I2CEN = 1;
}

void I2C_Enable(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.I2CEN = 1;
}

/*****************************************************************************
* Function      : _I2C_Disable
* Description   : I2C Controller Enable
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
#if 1
static void _I2C_Disable(sn_i2c_t *I2C_SEL)
{
    I2C_SEL->CTRL_b.I2CEN = 0;
}
#endif

void I2C_Disable(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.I2CEN = 0;
}

/*****************************************************************************
* Function      : I2C_Read_Status
* Description   : Read I2C Controller Status
* Input         : None
* Output        : None
* Return        : Status Register
* Note          : None
*****************************************************************************/
uint32_t I2C_Read_Status(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    return (uint32_t)(i2c->STAT);
}

/*****************************************************************************
* Function      : I2C_Receive_Data
* Description   : I2C Controller Receive_Data
* Input           : None
* Output          : None
* Return          : Received Data
* Note            : None
*****************************************************************************/
uint8_t I2C_Receive_Data(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    return (i2c->RXDATA);
}

/*****************************************************************************
* Function      : I2C_Send_ACK
* Description   : I2C Controller Send ACK
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
void I2C_Send_ACK(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.ACK = 1;
}

/*****************************************************************************
* Function      : I2C_Send_Data
* Description   : I2C Transmit
* Input           : Data: Send Data Value
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_Send_Data(i2c_id_t i2c_id, uint8_t ucData)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->TXDATA = ucData;
}

/*****************************************************************************
* Function      : I2C_Send_NACK
* Description   : I2C Controller Send NACK
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
void I2C_Send_NACK(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.NACK = 1;
}

/*****************************************************************************
* Function      : I2C_Set_Stop_Condition
* Description   : I2C Controller Send Stop Condition
* Input         : None
* Output        : None
* Return        : None
* Note          : None
*****************************************************************************/
void I2C_Set_Stop_Condition(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.STO = 1;
    //while((I2C_SEL->STAT&0x8008)!=0x8008);
    //I2C_SEL->STAT_b.I2CIF=1;
}

/*****************************************************************************
* Function      :   I2C_SlaveRxFIFO_Clear
* Description   :
* Input           :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_SlaveRxFIFO_Clear(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.RXFIFOCLR = 1;
}

/*****************************************************************************
* Function      :   I2C_SlaveTxFIFO_Clear
* Description   :
* Input           :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_SlaveTxFIFO_Clear(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.TXFIFOCLR = 1;
}

/*****************************************************************************
* Function      :
* Description   :
* Input           :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
//static void _I2C_SlaveRxFIFO_Stop(sn_i2c_t *I2C_SEL)
//{
//    I2C_SEL->CTRL_b.FIFO_FILL_STOP = 1;
//}

/*****************************************************************************
* Function      :
* Description   :
* Input           :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
static void _I2C_SlaveRxFIFO_Start(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;
    i2c = _I2C_Get_Handle(i2c_id);
    
    i2c->CTRL_b.FIFO_FILL_STOP = 0;
}

/*****************************************************************************
* Function      :   _I2C_SlaveFIFO_EN
* Description   :
* Input           : i2c_id_t
    FIFO_ACT_SEL-> Slave Rx 0: reply nack
                            1: wait state 
                   Slave Tx 0: Do not transmit data
                            1: wait state
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_SlaveFIFO_EN(i2c_id_t i2c_id, uint8_t Mode, bool FIFO_ACT_SEL)
{
    sn_i2c_t *i2c;
    i2c = _I2C_Get_Handle(i2c_id);
    
    if(Mode == I2C_RX_FIFO_MODE)
    {
        i2c->CTRL_b.FIFO_ACT_SEL = FIFO_ACT_SEL;
        i2c->CTRL_b.RX_FIFO_EN = 1;
    }
    else  /*if(Mode == I2C_TX_FIFO_MODE)*/
    {
        i2c->CTRL_b.FIFO_ACT_SEL = FIFO_ACT_SEL;
        i2c->CTRL_b.TX_FIFO_EN = 1;
    }
}

/*****************************************************************************
* Function      :
* Description   :
* Input           :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_SlaveFIFO_Disable(i2c_id_t i2c_id)
{
    sn_i2c_t *i2c;
    i2c = _I2C_Get_Handle(i2c_id);  
    i2c->CTRL_b.RX_FIFO_EN = 0;
}

/*****************************************************************************
* Function      :
* Description   :
* Input           :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_SlaveFIFO_THD(i2c_id_t i2c_id, uint8_t Value)
{
    sn_i2c_t *i2c;
    i2c = _I2C_Get_Handle(i2c_id);  
    i2c->CTRL_b.FIFO_THD = Value;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

/*****************************************************************************
* Function      : I2C_Initial
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
uint32_t I2C_Initial(i2c_init_info_t *info)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(info->i2c);

    _I2C_Address_Setting(i2c, info->addr_mode, info->gcen, info->slave_addr.ADDR0, info->slave_addr.ADDR1, info->slave_addr.ADDR2, info->slave_addr.ADDR3);

    /* i2c speed setting */
    i2c->SCLHT_b.SCLH =info->speed.SCLH;

    i2c->SCLLT_b.SCLL=info->speed.SCLL;

    /* enable i2c */
    _I2C_Enable(i2c);

    /* I2C TIME OUT Setting */
    i2c->TOCTRL_b.TO = info->time_out;

    if(info->i2c_id_txrx_mode == FIFO_TX_MODE)
    {
        I2C_SlaveTxFIFO_Clear(info->i2c);
        I2C_SlaveFIFO_THD(info->i2c, info->fifo_mode_para.FIFO_THRESHLOD);
        if(info->i2c_fifo_mode_en == true)
        {
            I2C_SlaveFIFO_EN(info->i2c, I2C_TX_FIFO_MODE, info->fifo_mode_para.FIFO_ACT);
        }
        else if(info->i2c_fifo_mode_en == false)
        {
            I2C_SlaveFIFO_Disable(info->i2c);
        }
    }

    if(info->i2c_id_txrx_mode == FIFO_RX_MODE)
    {
        _I2C_SlaveRxFIFO_Start(info->i2c);
        
        I2C_SlaveRxFIFO_Clear(info->i2c);
        
        I2C_SlaveFIFO_THD(info->i2c, info->fifo_mode_para.FIFO_THRESHLOD);
        
        if(info->i2c_fifo_mode_en == true)
        {
            I2C_SlaveFIFO_EN(info->i2c, I2C_RX_FIFO_MODE, info->fifo_mode_para.FIFO_ACT);
        }
        else if(info->i2c_fifo_mode_en == false)
        {
            I2C_SlaveFIFO_Disable(info->i2c);
        }
    }

    /* Enalbe IRQ */
    if(info->i2c_irq == ENABLE)
    {
        if(info->i2c == I2C_0)
        {
            NVIC_ClearPendingIRQ(I2C0_IRQn);

            NVIC_EnableIRQ(I2C0_IRQn);
        }
        else if(info->i2c == I2C_1)
        {
            NVIC_ClearPendingIRQ(I2C1_IRQn);

            NVIC_EnableIRQ(I2C1_IRQn);
        }
        else if(info->i2c == I2C_2)
        {
            NVIC_ClearPendingIRQ(I2C2_IRQn);

            NVIC_EnableIRQ(I2C2_IRQn);
        }

    }
    
    return 0; /* I2C_SUCCESS */
}

/*****************************************************************************
* Function      : I2C_deinit
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : release I2C port
*****************************************************************************/
uint32_t I2C_Deinit(i2c_init_info_t *info)
{
    sn_i2c_t *i2c;

    i2c = _I2C_Get_Handle(info->i2c);
    
    _I2C_Disable(i2c);
    
    return 0; /* I2C_SUCCESS */
}

/*****************************************************************************
* Function      : I2C_Default_Init
* Description   : Default initial
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
void I2C_Default_Init(i2c_id_t i2c_id, slave_addr_t *slave_addr)
{
    sn_i2c_t *i2c;
    
    i2c = _I2C_Get_Handle(i2c_id);
    /* enable i2c */
    _I2C_Enable(i2c);
    /* i2c speed: standard mode */
    /* i2c->SCLHT_b.SCLH=119 */
    i2c->SCLHT_b.SCLH = 200; //SCL_temp;
    i2c->SCLLT_b.SCLL =  200; //SCL_temp;
    /* I2C TIME OUT Setting */
    i2c->TOCTRL_b.TO = 0;
    _I2C_Address_Setting(i2c, MODE_7BIT, 0, slave_addr->ADDR0, slave_addr->ADDR1, slave_addr->ADDR2, slave_addr->ADDR3);
}

/*****************************************************************************
* Function      : I2C_Master_Send
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
i2c_err_ret_t I2C_Master_Send(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t length,bool auto_stop)
{
    uint32_t bufcount_tx;  
    
    _i2c_time_out_init();
    
    if( !length )
    {
        return I2C_FAIL; /* error handle */
    }
    
    I2C_Set_Start_Condition(i2c_id);
    
    /*send target address*/
    //while(((I2C_Read_Status(i2c) & 0xFFF) != (S_START_DN | S_I2C_MST)));
    while(((I2C_Read_Status(i2c_id) & (S_START_DN | S_I2C_MST)) != (S_START_DN | S_I2C_MST)))
    {
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }        
    }        
    
    I2C_Send_Data(i2c_id, slave_addr << 1);
    
    /*received an ACK*/
    for(bufcount_tx = 0; bufcount_tx < length; bufcount_tx++)
    {
        
        while(((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) != (S_ACK_STAT | S_I2C_MST)))
        {
            if( (I2C_Read_Status(i2c_id) & (S_NACK_STAT | S_I2C_MST)) == (S_NACK_STAT | S_I2C_MST) )
            {
                return I2C_FAIL;
            }
            
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }    
        }
        
        I2C_Send_Data(i2c_id,*data_addr);//send data

        data_addr++;
    }
    while(((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) != (S_ACK_STAT | S_I2C_MST)))
    {
        if( (I2C_Read_Status(i2c_id) & (S_NACK_STAT | S_I2C_MST)) == (S_NACK_STAT | S_I2C_MST) )
        {
                if(auto_stop==1)
                {        
                    I2C_Set_Stop_Condition(i2c_id);
                }          
            return I2C_FAIL;
        }
    
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        } 
    }
    
    /* auto stop condition */
    if(auto_stop==1)
    {        
        I2C_Set_Stop_Condition(i2c_id);
    }
          
    return I2C_SUCCESS;
}
/*****************************************************************************
* Function      : I2C_Master_Send_10bit
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
i2c_err_ret_t I2C_Master_Send_10bit(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t length,bool auto_stop)
{
    uint32_t bufcount_tx;
    uint8_t slave_addr_H,slave_addr_L;
    
    slave_addr=slave_addr&0x3FF;
    
    _i2c_time_out_init();
    
    if( !length )
    {
        return I2C_FAIL; /* error handle */
    }

    I2C_Set_Start_Condition(i2c_id);
    
    /*send target address*/
    //while(((I2C_Read_Status(i2c) & 0xFFF) != (S_START_DN | S_I2C_MST)));
    while(((I2C_Read_Status(i2c_id) & (S_START_DN | S_I2C_MST)) != (S_START_DN | S_I2C_MST)))
    {
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }        
    }        
    
    slave_addr_H = (((slave_addr)>>8)+0x78 );
    
    slave_addr_L = (slave_addr&0xFF);
    
    I2C_Send_Data(i2c_id, slave_addr_H << 1);
    
    while(((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) != (S_ACK_STAT | S_I2C_MST)))
    {
    }    
    
    I2C_Send_Data(i2c_id,slave_addr_L);
    
    /*received an ACK*/
    for(bufcount_tx = 0; bufcount_tx < length; bufcount_tx++)
    {
        
        while(((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) != (S_ACK_STAT | S_I2C_MST)))
        {
            if( (I2C_Read_Status(i2c_id) & (S_NACK_STAT | S_I2C_MST)) == (S_NACK_STAT | S_I2C_MST) )
            {
                return I2C_FAIL;
            }
            
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }    
        }
        
        I2C_Send_Data(i2c_id,*data_addr);//send data

        data_addr++;
    }
    
    while(((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) != (S_ACK_STAT | S_I2C_MST)))
    {
        if( (I2C_Read_Status(i2c_id) & (S_NACK_STAT | S_I2C_MST)) == (S_NACK_STAT | S_I2C_MST) )
        {
            return I2C_FAIL;
        }
    
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        } 
    }
    
    /* auto stop condition */
    if(auto_stop==1)
    {        
        I2C_Set_Stop_Condition(i2c_id);
    }
    
    return I2C_SUCCESS;
}

/*****************************************************************************
* Function      : I2C_Master_Recv
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
i2c_err_ret_t I2C_Master_Recv(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t lentgh,bool auto_stop)
{
    uint32_t bufcount_rx=0;
    
    _i2c_time_out_init();
    
    if( !lentgh )
    {
        return I2C_FAIL; /* error handle */
    }

    I2C_Set_Start_Condition(i2c_id);

    /*send target address*/
    while(!((I2C_Read_Status(i2c_id) & (S_START_DN | S_I2C_MST)) == (S_START_DN | S_I2C_MST)))
    {
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        }                     
    }
    
    I2C_Send_Data(i2c_id, slave_addr << 1 | 1);
    
    /*received an ACK*/
    while(!((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) == (S_ACK_STAT | S_I2C_MST)))
    {
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        }      
    }
    /*a 8-bit RX with ACK/NACK transfer is done*/
    for(bufcount_rx = 0; bufcount_rx < lentgh; bufcount_rx++)
    {        
        if( bufcount_rx == (lentgh-1) )
        {   /* the last byte need to send nck */
            I2C_Send_NACK(i2c_id);
        }
        else if(bufcount_rx < lentgh)
        {   /* Master Rx buffer no empty keep read data from slave, send ACK */
            I2C_Send_ACK(i2c_id);
        }
        
        //I2C_Send_ACK(i2c);
        
        while((I2C_Read_Status(i2c_id) & (S_RX_DN | S_I2C_MST)) != (S_RX_DN | S_I2C_MST))
        {
            if((I2C_Read_Status(i2c_id) & (S_NACK_STAT | S_I2C_MST)) == (S_NACK_STAT | S_I2C_MST))
            {
                I2C_Set_Stop_Condition(i2c_id);
                
                return I2C_FAIL;
            }
            
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }   
        }
        
        *data_addr = I2C_Receive_Data(i2c_id);//receive data to RXBuf

        data_addr++;     
    }
           
    /* auto set stop condition */
    if(auto_stop==1)
    {        
        I2C_Set_Stop_Condition(i2c_id);
    }

    return I2C_SUCCESS;
}

/*****************************************************************************
* Function      : I2C_Master_Recv_10bit
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
i2c_err_ret_t I2C_Master_Recv_10bit(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t lentgh,bool auto_stop)
{
    uint32_t bufcount_rx=0;
    slave_addr=slave_addr&0x3FF;
    uint8_t slave_addr_H,slave_addr_L;
    
    _i2c_time_out_init();
    
    if( !lentgh )
    {
        return I2C_FAIL; /* error handle */
    }
    
    I2C_Set_Start_Condition(i2c_id);

    /*send target address*/
    while(!((I2C_Read_Status(i2c_id) & (S_START_DN | S_I2C_MST)) == (S_START_DN | S_I2C_MST)))
    {
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        }                     
    }
    
    slave_addr_H = (((slave_addr)>>8)+0x78);    
    slave_addr_L = (slave_addr&0xFF);
    I2C_Send_Data(i2c_id, slave_addr_H << 1);
    
    /*received an ACK*/
    while(!((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) == (S_ACK_STAT | S_I2C_MST)))
    {
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        }      
    }
    
    I2C_Send_Data(i2c_id,slave_addr_L);
    
    while(!((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) == (S_ACK_STAT | S_I2C_MST)))
    {
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        }      
    }
    
    I2C_Set_Start_Condition(i2c_id);
    
    while(!((I2C_Read_Status(i2c_id) & (S_START_DN | S_I2C_MST)) == (S_START_DN | S_I2C_MST)))
    {        
    }    
    
    I2C_Send_Data(i2c_id, slave_addr_H << 1|1);
    
    while(!((I2C_Read_Status(i2c_id) & (S_ACK_STAT | S_I2C_MST)) == (S_ACK_STAT | S_I2C_MST)))
    {
        if(_i2c_time_out()==1)
        {
            return I2C_FAIL;
        }      
    }
    
    /*a 8-bit RX with ACK/NACK transfer is done*/
    for(bufcount_rx = 0; bufcount_rx < lentgh; bufcount_rx++)
    {        
        if( bufcount_rx == (lentgh-1) )
        {   /* the last byte need to send nck */
            I2C_Send_NACK(i2c_id);
        }
        else if(bufcount_rx < lentgh)
        {   /* Master Rx buffer no empty keep read data from slave, send ACK */
            I2C_Send_ACK(i2c_id);
        }
        
        //I2C_Send_ACK(i2c);
        
        while((I2C_Read_Status(i2c_id) & (S_RX_DN | S_I2C_MST)) != (S_RX_DN | S_I2C_MST))
        {
            if((I2C_Read_Status(i2c_id) & (S_NACK_STAT | S_I2C_MST)) == (S_NACK_STAT | S_I2C_MST))
            {
                I2C_Set_Stop_Condition(i2c_id);
                
                return I2C_FAIL;
            }
            
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }   
        }
        
        *data_addr = I2C_Receive_Data(i2c_id);//receive data to RXBuf

        data_addr++;
        
    }
           
    /* auto set stop condition */
    if(auto_stop==1)
    {        
        I2C_Set_Stop_Condition(i2c_id);
    }
        
    return I2C_SUCCESS;

}

/*****************************************************************************
* Function      : I2C_Slave_Send
* Description   :
* Input         :
* Output          : None
* Return          : None
* Note            : None
*****************************************************************************/
i2c_err_ret_t I2C_Slave_Send(i2c_id_t i2c_id,uint8_t* data_addr,uint32_t lentgh)
{
    sn_i2c_t *i2c;

    uint32_t bufcount_tx;
    
    _i2c_time_out_init();
    
    if( !lentgh )
    {
        return I2C_FAIL; /* error handle */
    }

    i2c = _I2C_Get_Handle(i2c_id);
      
    /* for Addr mode set 10-bit */
    if( i2c->SLVADDR0_b.ADD_MODE == 1 )
    {
            while(!((I2C_Read_Status(i2c_id)& S_SLVRX_HIT) == S_SLVRX_HIT))
            {
                if(_i2c_time_out()==1)
                {
                    return I2C_FAIL;
                }  
            }

            I2C_Send_ACK(i2c_id);
                          
    }
    
    /*slave address hit*/
    while(!((I2C_Read_Status(i2c_id)& S_SLVTX_HIT) == S_SLVTX_HIT))
    {
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }  
    }
    
    I2C_Send_Data(i2c_id, *data_addr); //send data

    data_addr++;

    /*received an ACK*/
    for(bufcount_tx = 0; bufcount_tx < lentgh; bufcount_tx++)
    {
        while(!((I2C_Read_Status(i2c_id) & S_ACK_STAT) == (S_ACK_STAT)))
        {               
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }
            
            if( ((I2C_Read_Status(i2c_id) & S_NACK_STAT) == (S_NACK_STAT)) )
            {
                I2C_Send_ACK(i2c_id);
                
                return I2C_SUCCESS;
            }    
            
        }

        I2C_Send_Data(i2c_id, *data_addr); //send data

        data_addr++;
            
    }
  
    return I2C_SUCCESS;
    
}

/*****************************************************************************
* Function      : I2C_Slave_Recv
* Description   :
* Input         :
* Output          : None
* Return          : i2c_err_ret_t
* Note            : None
*****************************************************************************/
i2c_err_ret_t I2C_Slave_Recv(i2c_id_t i2c_id,uint8_t* data_addr,uint32_t lentgh)
{ 
    uint32_t bufcount_rx;
    
    _i2c_time_out_init();
    
    if( !lentgh )
    {
        return I2C_FAIL; /* error handle */
    }
    
    /*slave address hit*/
    while(!((I2C_Read_Status(i2c_id)&S_SLVRX_HIT ) == (S_SLVRX_HIT )))
    {
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }  
    }
    /*send ACK*/        
    I2C_Send_ACK(i2c_id);

    for(bufcount_rx = 0; bufcount_rx < lentgh; bufcount_rx++)
    {
        while(!((I2C_Read_Status(i2c_id) & S_RX_DN ) == S_RX_DN ))
        {
            if(_i2c_time_out()==1)
            {
                return I2C_FAIL;
            }  
        }
        
        *data_addr = I2C_Receive_Data(i2c_id);//receive data to RXBuf

        data_addr++;
        /* Master Rx buffer no empty keep read data from slave, send ACK */
        if(bufcount_rx < lentgh)
        {
            I2C_Send_ACK(i2c_id);//send ACK , Rx buffer not full , tell master tx ack keep send data
        }
        else if(bufcount_rx == (lentgh-1))
        {
            I2C_Send_NACK(i2c_id);//send NACK , Last Byte Data that slave rx need to reveice , tell master tx nack
        }
        else if(bufcount_rx >= lentgh)
        {
            //Finish Receive data from master tx after last byte fill in and finish nack.
            I2C_Send_NACK(i2c_id);//send NACK for slave rx release bus
            /* length diff with Master */
            return I2C_FAIL;
        }

    }

    return I2C_SUCCESS;
}

/*****************************************************************************
* Function      : I2C_Slave_Hit_Status
* Description   :
* Input         :  i2c_id_t
* Output          : None
* Return          : i2c_slave_hit_t
* Note            : None
*****************************************************************************/
i2c_slave_hit_t I2C_Slave_Hit_Status(i2c_id_t i2c_id)
{      
    if(I2C_Read_Status(i2c_id)&S_SLVRX_HIT)
    {
        return SLAVE_RX_HIT;
    }
    else if(I2C_Read_Status(i2c_id)&S_SLVTX_HIT)
    {
        return SLAVE_TX_HIT;
    }else
    {
        return NO_HIT;
    }
    
}

/*****************************************************************************
* Function      : I2C_GetVersion
* Description   :
* Input         :  none
* Output          : None
* Return          : uint32_t
* Note            : None
*****************************************************************************/
uint32_t I2C_GetVersion(void)
{
    return I2C_VERSION;
}

