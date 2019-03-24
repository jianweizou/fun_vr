/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_i2c.h
 *
 * @author chia-hao Hsu
 * @version 0.1
 * @date 2018/04/20
 * @license
 * @description
 */

#ifndef __snc_i2c_H_wyGTxhNr_lUoN_HN2s_s9ax_uCc6IY9z2xmA__
#define __snc_i2c_H_wyGTxhNr_lUoN_HN2s_s9ax_uCc6IY9z2xmA__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"	
	
//=============================================================================
//                  Constant Definition
//=============================================================================
/* I2C Status Define */
#define S_RX_DN       				0x00000001
#define S_ACK_STAT    				0x00000002
#define S_NACK_STAT   				0x00000004
#define S_STOP_DN     				0x00000008
#define S_START_DN    				0x00000010
#define S_I2C_MST     				0x00000020
#define S_SLVRX_HIT   				0x00000040
#define S_SLVTX_HIT   				0x00000080
#define S_LOST_ARB    				0x00000100
#define S_TIMEOUT     				0x000001618
#define S_I2CIF       				0x00008000
#define	S_I2C_TXFIFO_EMPTY		0x01000000
#define	S_I2C_TXFIFO_FULL			0x02000000
#define	S_I2C_RXFIFO_EMPTY		0x04000000
#define	S_I2C_RXFIFO_FULL			0x08000000
#define	S_I2C_FIFO_OVERFLOW		0x10000000
#define F_MST_STRDN   0x8030
#define F_STOPDN      0x8008 
#define F_MST_ACK     0x8022
#define F_MST_RESTR   0x8038
#define F_MST_RX_DN   0x8021
#define F_MST_NACK    0x8024
    
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef enum i2c_id
{
    I2C_0     = 0,
    I2C_1,
    I2C_2,
}i2c_id_t;

//typedef enum i2c_speed
//{
//	I2C_STANDARD_SPEED= 0, 		/* 100kHz */
//    I2C_FAST_SPEED,			/* 400kHz */
//    I2C_FAST_SPEED_PLUS,	/* 1MHz */
//}i2c_speed_t;

typedef struct i2c_speed
{
    uint32_t SCLH;
    uint32_t SCLL;
}i2c_speed_t;

typedef enum i2c_slave_addr_mode
{
	MODE_7BIT= 0, 		
    MODE_10BIT,			
}slave_addr_mode_t;

typedef struct slave_address
{
    uint32_t ADDR0;
	uint32_t ADDR1;
	uint32_t ADDR2;
	uint32_t ADDR3;
}slave_addr_t;

typedef enum i2c_fifo_txrx_mode
{	
	NONE=0,
	FIFO_TX_MODE,
	FIFO_RX_MODE,
}i2c_fifo_txrx_mode_t;

typedef enum i2c_slave_Mode
{
	NORMAL=0,
	FIFO,
}slave_mode_t;

typedef enum fifo_threshold
{
    FIFO_THRESHOLD_1=1,
    FIFO_THRESHOLD_2,
	FIFO_THRESHOLD_3,
	FIFO_THRESHOLD_4,	
}fifo_threshold_t;

typedef enum fifo_act_sel 
{
	REPLY_NACK=0,/* Rx reply ack, Tx: don't tx data  */
	N_TXDATA=0,
	WAIT_STA,				/* wait state */
}fifo_act_t;

typedef struct fifo_mode_para
{
	fifo_threshold_t 	FIFO_THRESHLOD;
	fifo_act_t 			FIFO_ACT;	
}fifo_mode_para_t;

typedef struct i2c_init
{
	i2c_speed_t                 speed;
	i2c_id_t 					i2c;
	uint16_t time_out;
	slave_addr_mode_t 		    addr_mode;
	slave_addr_t 			    slave_addr;
	bool 				        gcen;
	bool 				        i2c_irq;
	i2c_fifo_txrx_mode_t 	    i2c_id_txrx_mode;
	bool 		                i2c_fifo_mode_en;
	fifo_mode_para_t 		    fifo_mode_para;
}i2c_init_info_t;

typedef enum i2c_slave_hit
{
    NO_HIT     = 0,
    SLAVE_TX_HIT,
    SLAVE_RX_HIT,
}i2c_slave_hit_t;

typedef enum i2c_err_code
{
    I2C_SUCCESS     = 0,
	I2C_FAIL,
    I2C_PROCESS_INTERRUPT,
}i2c_err_ret_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uint32_t I2C_Initial(i2c_init_info_t* info);
void I2C_Default_Init(i2c_id_t i2c_id,slave_addr_t* slave_addr);
uint32_t I2C_Deinit(i2c_init_info_t *info);
i2c_err_ret_t I2C_Master_Send(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t length,bool auto_stop);
i2c_err_ret_t I2C_Master_Send_10bit(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t length,bool auto_stop);
i2c_err_ret_t I2C_Master_Recv(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t lentgh,bool auto_stop);
i2c_err_ret_t I2C_Master_Recv_10bit(i2c_id_t i2c_id,uint32_t slave_addr,uint8_t* data_addr,uint32_t lentgh,bool auto_stop);
i2c_err_ret_t I2C_Slave_Send(i2c_id_t i2c_id,uint8_t* data_addr,uint32_t lentgh);
i2c_err_ret_t I2C_Slave_Recv(i2c_id_t i2c_id,uint8_t* data_addr,uint32_t lentgh);
/* I2C Peripheral API*/
void I2C_Send_ACK(i2c_id_t i2c_id);
void I2C_Send_NACK(i2c_id_t i2c_id);
void I2C_Set_Start_Condition(i2c_id_t i2c_id);
void I2C_Set_Stop_Condition(i2c_id_t i2c_id);
void I2C_Send_Data(i2c_id_t i2c_id, uint8_t ucData);
void I2C_Clear_Flag(i2c_id_t i2c_id);
/* FIFO API */
void I2C_SlaveFIFO_EN(i2c_id_t i2c_id, uint8_t Mode, bool FIFO_ACT_SEL);
void I2C_SlaveFIFO_Disable(i2c_id_t i2c_id);
void I2C_SlaveRxFIFO_Clear(i2c_id_t i2c_id);
void I2C_SlaveTxFIFO_Clear(i2c_id_t i2c_id);
void I2C_SlaveFIFO_THD(i2c_id_t i2c_id, uint8_t Value);
uint32_t I2C_Read_Status(i2c_id_t i2c_id);
uint8_t I2C_Receive_Data(i2c_id_t i2c_id);
i2c_slave_hit_t I2C_Slave_Hit_Status(i2c_id_t i2c_id);
uint32_t I2C_GetVersion(void);

#ifdef __cplusplus

}

#endif

#endif
