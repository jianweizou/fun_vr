/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file nf.c
 *
 * @author Enter Hung
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */
 
#include "register_7320.h"
#include "snc_nf.h"
#include "snc_idma.h"

#define NF_CMD_TIMEOUT	0x6000000
#define NF_CMD_NIB_WRITE_DATA					0x0
#define NF_CMD_NIB_READ_DATA					0x8
#define NF_CMD_NIB_ERASE_DATA					0xE
#define NF_CMD_NIB_MULTI_PLANE_ERASE	0xF
#define NF_CMD_NIB_READ_PAGE_INFO			0xA
#define	NF_CMD_NIB_RESET_NF						0x9
#define NF_CLEAR_CMD_ERROR_STATUS			0xFFFA

#define NF_CMD_READ_ID			0x90
#define NF_ADDR_READ_ID			0x00

#define NF_VERSION		0x73200000

typedef enum nf_rw_idx
{
	NF_WRITE,
	NF_READ
}nf_rw_idx_t;

typedef enum nf_page_size_idx
{
	NF_PS_512,
	NF_PS_2048,
	NF_PS_4096
}nf_page_size_idx_t;

typedef enum nf_page_num_idx
{
	NF_PN_16,
	NF_PN_32,
	NF_PN_64,
	NF_PN_128
}nf_page_num_idx_t;

typedef enum nf_addr_cycle_idx
{
	NF_AC_3,
	NF_AC_4,
	NF_AC_5
}nf_addr_cycle_idx_t;

typedef enum nf_rw_check_select_idx
{
	NF_RW_CHK_DISABLE,
	NF_RW_CHK_NORMAL,
	NF_RW_CHK_CATCH,
	NF_RW_CHK_MULTI_PLANE
}nf_rw_check_select_idx_t;

typedef enum nf_cmd_address_select_idx
{
	NF_CMD_ADDR_DISABLE,
	NF_CMD_ADDR_NORMAL,
	NF_CMD_ADDR_MULTI_PLANE
}nf_cmd_address_select_idx_t;

uint16_t nfid[5];
uint16_t *nf_read_buffer;

uint32_t NF_GetVersion(void)
{
	return NF_VERSION;
}

static int NF_Cmd_Polling(void)
{
	uint32_t i = 0;
	while(!(SN_NF->NF_CTRL_b.MS_RDY))
	{
		i++;
		if(i == NF_CMD_TIMEOUT)
			return NF_RETRY_TIME_OUT;
	}
	
	return NF_NO_ERR;
}

static uint32_t NF_Check_Cmd_Done(void)
{
	return (SN_NF->NF_STAT & SET_BIT1);
}

static int NF_NIB_Cmd_Polling(void)
{
	uint32_t i = 0;
	while(!NF_Check_Cmd_Done())
	{
		i++;
		if(i > NF_CMD_TIMEOUT)
			return NF_RETRY_TIME_OUT;		
	}
	
	return NF_NO_ERR;
}

static uint32_t NF_Check_Cmd_Buffer_Ready(void)
{
	return (SN_NF->NF_CMD_BUF & SET_BIT0);
}

static int NF_Check_Cmd_Error_Status(void)
{
	if(SN_NF->NF_STAT & 0xFFF9)
	{
		if(SN_NF->NF_STAT_b.ECC_ERR)
			return NF_ECC_ERR;
		else if(SN_NF->NF_STAT_b.ECC_FAIL)
			return NF_ECC_FAIL;
		else if(SN_NF->NF_STAT_b.PROG_ERR)
			return NF_PROGRAM_FAIL;
		else if(SN_NF->NF_STAT_b.ERASE_ERR)
			return NF_ERASE_FAIL;
		else if(SN_NF->NF_STAT_b.CRC_ERR_CNT)
			return NF_CRC_ERR;
		else if(SN_NF->NF_STAT_b.TIME_OUT_ERR)
			return NF_RW_TIME_OUT;
		else if(SN_NF->NF_STAT_b.NF_INFO_ERR)
			return NF_INFO_ERR;
	}
	
	return NF_NO_ERR;
}

static int NF_READ_ID(uint16_t *nf_chip_id)
{
	int ret;
	
	SN_NF->NF_CTRL_b.MS_REG_RW = NF_WRITE;
	SN_NF->NF_REG1 = NF_CMD_READ_ID;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;
	SN_NF->NF_REG2 = NF_ADDR_READ_ID;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;	
	/*get id*/
	SN_NF->NF_CTRL_b.MS_REG_RW = NF_READ;
	SN_NF->NF_REG3 = 1;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;	
	*nf_chip_id = SN_NF->NF_REG3;
	
	SN_NF->NF_REG3 = 1;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;	
	*(nf_chip_id + 1) = SN_NF->NF_REG3;	
	
	SN_NF->NF_REG3 = 1;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;	
	*(nf_chip_id + 2) = SN_NF->NF_REG3;

	SN_NF->NF_REG3 = 1;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;	
	*(nf_chip_id + 3) = SN_NF->NF_REG3;

	SN_NF->NF_REG3 = 1;
	ret = NF_Cmd_Polling();
	if(ret)
		return ret;	
	*(nf_chip_id + 4) = SN_NF->NF_REG3;
	
	return NF_NO_ERR;
}

static int NF_Reset(void)
{
	uint32_t i = 0;
	int ret;
	
	while(!NF_Check_Cmd_Buffer_Ready())
	{
		i++;
		if(i > NF_CMD_TIMEOUT)
			return NF_RETRY_TIME_OUT;
	}
	
	SN_NF->NF_CMD_b.NF_CMD_NIB = NF_CMD_NIB_RESET_NF;
	ret = NF_NIB_Cmd_Polling();
	if(ret)
		return ret;
	
	ret = NF_Check_Cmd_Error_Status();
	SN_NF->NF_STAT = NF_CLEAR_CMD_ERROR_STATUS;
	
	return ret;
}

static int NF_Enable(void)
{
	int ret;
	
	NVIC_EnableIRQ(NFECC_IRQn);	
	/*set nf mode*/
	SN_NF->NF_CTRL_b.NF_MODE = 1;
	/* Time out counter up limit is clk_cycle * (TIME_CNT * 4) */
	/* 0.02083 us * (TIME_CNT*4)=100ms --> TIME_CNT=0x125040 */
	SN_NF->TIME_CNT_L = 0x5040;
	SN_NF->TIME_CNT_H = 0x0012;

	ret = NF_Reset();
	
	return ret;
}

static void NF_RW_And_Check_Flow_Select(uint32_t select_mode)
{
	SN_NF->NF_CMD_b.WR_CHK_SEL = select_mode;
}

static void NF_Cmd_Address_Select(uint32_t select_mode)
{
	SN_NF->NF_CMD_b.CMD_ADDR_SEL = select_mode;
}

static void NF_Info_Set(uint16_t page_size, uint16_t page_num_of_block, uint16_t addr_cycle, uint16_t read_width, uint16_t write_width)
{
	if(page_size == 512)
		SN_NF->NF_INFO_b.PAGE_SIZE = NF_PS_512;
	else if(page_size == 2048)
		SN_NF->NF_INFO_b.PAGE_SIZE = NF_PS_2048;
	else if(page_size == 4096)
		SN_NF->NF_INFO_b.PAGE_SIZE = NF_PS_4096;
	
	if(page_num_of_block == 16)
		SN_NF->NF_INFO_b.BLOCK_SIZE = NF_PN_16;
	else if(page_num_of_block == 32)
		SN_NF->NF_INFO_b.BLOCK_SIZE = NF_PN_32;
	else if(page_num_of_block == 64)
		SN_NF->NF_INFO_b.BLOCK_SIZE = NF_PN_64;
	else if(page_num_of_block == 128)
		SN_NF->NF_INFO_b.BLOCK_SIZE = NF_PN_128;
	
	if(addr_cycle == 3)
		SN_NF->NF_INFO_b.ADDR_CYCLE = NF_AC_3;
	else if(addr_cycle == 4)
		SN_NF->NF_INFO_b.ADDR_CYCLE = NF_AC_4;
	else if(addr_cycle == 5)
		SN_NF->NF_INFO_b.ADDR_CYCLE = NF_AC_5;
	
	SN_NF->NF_ACC = (write_width << 2) + read_width;
	NF_RW_And_Check_Flow_Select(NF_RW_CHK_NORMAL);
	NF_Cmd_Address_Select(NF_CMD_ADDR_NORMAL);
}

int NF_Page_Read(uint32_t *Addr, uint32_t data_size, uint16_t *ram_addr, uint32_t ecc_enable)
{
	int ret;
	uint32_t	Phyaddress=0;
	if(ecc_enable)
	{
		SN_NF->NF_CTRL_b.EXTRA_EN = 1;
		SN_NF->NF_CTRL_b.ECC_EN = 1;
		nf_read_buffer = ram_addr;
	}
	else
	{
		SN_NF->NF_CTRL_b.EXTRA_EN = 0;
		SN_NF->NF_CTRL_b.ECC_EN = 0;
	}
	Phyaddress=*Addr;
	SN_NF->NF_ADDRL = Phyaddress & 0x0000FFFF;
	SN_NF->NF_ADDRH = Phyaddress >> 16;
	SN_NF->NF_RW_LENGTH = data_size - 1;
	/*idma need use api*/
//	SN_IDMA_SDNF->CH_RAM_ADDR = (uint32_t)ram_addr;
//	SN_IDMA_SDNF->CH_LEN = data_size;
//	SN_IDMA_SDNF->CH_CTRL_b.WR_RAM = 1;
//	SN_IDMA_SDNF->CH_CTRL_b.START = 1;
	{
		idma_setting_t set;
		set.dip_addr = 0;
		set.ram_addr = (uint32_t)ram_addr;
		set.length = data_size;
		set.direction = IDMA_DIRECTION_DIP_2_RAM;
		ret = IDMA_Start(IDMA_CH_SDNF, &set);
		if(ret)
			return ret;
		//while(IDMA_Get_Status(IDMA_CH_SDNF));
	}
	/*idma need use api*/
	SN_NF->NF_CMD_b.NF_CMD_NIB = NF_CMD_NIB_READ_DATA;
	ret = NF_NIB_Cmd_Polling();
	if(ret)
		return ret;
	
	ret = NF_Check_Cmd_Error_Status();
	if(ret == NF_ECC_FAIL)
	{
		if(SN_NF->NF_STAT_b.NF_ECC_FF)
		{
			if(SN_NF->NF_PG_WR_b.PAGE_EMPTY)
			{
				SN_NF->NF_STAT = NF_CLEAR_CMD_ERROR_STATUS;
				return NF_NO_ERR;
			}
		}
	}
	SN_NF->NF_STAT = NF_CLEAR_CMD_ERROR_STATUS;
	return ret;
}

int NF_Page_Write(uint32_t *Addr, uint32_t data_size, uint16_t *ram_addr, uint32_t ecc_enable)
{
	int ret;
	uint32_t	Phyaddress=0;
	if(ecc_enable)
	{
		SN_NF->NF_CTRL_b.EXTRA_EN = 1;
		SN_NF->NF_CTRL_b.ECC_EN = 1;
	}
	else
	{
		SN_NF->NF_CTRL_b.EXTRA_EN = 0;
		SN_NF->NF_CTRL_b.ECC_EN = 0;
	}
	Phyaddress=*Addr;
	SN_NF->NF_ADDRL = Phyaddress & 0x0000FFFF;
	SN_NF->NF_ADDRH = Phyaddress >> 16;
	SN_NF->NF_RW_LENGTH = data_size - 1;
	/*idma need use api*/
//	SN_IDMA_SDNF->CH_RAM_ADDR = (uint32_t)ram_addr;
//	SN_IDMA_SDNF->CH_LEN = data_size;
//	SN_IDMA_SDNF->CH_CTRL_b.WR_RAM = 0;
//	SN_IDMA_SDNF->CH_CTRL_b.START = 1;
	{
		idma_setting_t set;
		set.dip_addr = 0;
		set.ram_addr = (uint32_t)ram_addr;
		set.length = data_size;
		set.direction = IDMA_DIRECTION_RAM_2_DIP;
		ret = IDMA_Start(IDMA_CH_SDNF, &set);
		if(ret)
			return ret;
		//while(IDMA_Get_Status(IDMA_CH_SDNF));	
	}
	/*idma need use api*/	
	SN_NF->NF_CMD_b.NF_CMD_NIB = NF_CMD_NIB_WRITE_DATA;
	ret = NF_NIB_Cmd_Polling();
	if(ret)
		return ret;
	
	ret = NF_Check_Cmd_Error_Status();
	SN_NF->NF_STAT = NF_CLEAR_CMD_ERROR_STATUS;
	return ret;	
}

int NF_Block_Erase(uint32_t *Addr)
{
	int ret;
	uint32_t	Phyaddress;
    Phyaddress=*Addr;
	SN_NF->NF_ADDRL = Phyaddress & 0x0000FFFF;
	SN_NF->NF_ADDRH = Phyaddress >> 16;
	
	SN_NF->NF_CMD_b.NF_CMD_NIB = NF_CMD_NIB_ERASE_DATA;
	ret = NF_NIB_Cmd_Polling();
	if(ret)
		return ret;
	
	ret = NF_Check_Cmd_Error_Status();
	SN_NF->NF_STAT = NF_CLEAR_CMD_ERROR_STATUS;
	return ret;		
}

int NF_Init(NFTYPE *nf)
{
	uint32_t cur_id = 0;
	int ret;
	
	ret = NF_Enable();
	if(ret)
		return ret;
	
	ret = NF_READ_ID(nfid);
	if(ret)
		return ret;
	
	cur_id |= (nfid[0] << 8);
	cur_id |= nfid[1];

	if( nf->id == cur_id)
	{
		NF_Info_Set(nf->page_size, nf->pages_per_block, nf->address_cycle, \
			nf->read_width, nf->write_width);
		return NF_NO_ERR;
	}

	return NF_NOT_SUPPORT;		
}

void NF_Deinit(void)
{
	SN_NF->NF_CTRL_b.NF_MODE = 0;
}

static void NF_ECC_Process(void)
{
	uint16_t ecc_addr;
	uint16_t word_addr;
	uint16_t ecc_l;
	uint16_t ecc_h;
	
	while(!(SN_NF->NF_ECC_REQ_b.ECC_DN))
	{
		while(!(SN_NF->NF_ECC_REQ_b.ECC_REQ));
		ecc_addr = SN_NF->NF_ECC_ADDR & 0x3FFF;
		word_addr = ecc_addr >> 1;
		ecc_l = SN_NF->NF_ECC_MASK & 0x00FF;
		ecc_h = ecc_l << 8;
		if(ecc_addr & SET_BIT0)
			/*high byte correct*/
			nf_read_buffer[word_addr] ^=  ecc_h;
		else
			/*low byte correct*/
			nf_read_buffer[word_addr] ^=  ecc_l;
	}
}

__irq void NFECC_IRQHandler(void)
{
	NF_ECC_Process();
}
