/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file sdc.c
 *
 * @author Enter Hung
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */
 
#include "register_7320.h"
#include "snc_sdcard.h"
#include "snc_idma.h"

#define SDC_CMD_SELECT_CARD				0x07
#define SDC_CMD_STOP_TRANSMISSION	0x0c
#define SDC_CMD_SET_WRITE_PORT		0x1c
#define SDC_CMD_CLR_WRITE_PORT		0x1d
#define SDC_CMD_ERASE							0x26

#define SDC_MIN_CMD_TIMEOUT			640
#define SDC_SD_MODE 						0x02
#define SDC_CRC_ENABLE					0x03
#define SDC_AUTO_RESPONSE_MODE	0x3FFF
	
#define SDC_BLOCK_LENGTH	0x1FF
#define	SD_4BIT_MODE    0x01
#define	SD_SPI_MODE     0X04
#define SDC_CMD_TIMEOUT	0xFFFFFFFF
#define SDC_VERSION			0x73200000

__packed struct sdcinfo
{
	uint8_t sd_speed;
	uint8_t sd_card_version;
	uint8_t support_hi_speed;
	uint64_t sd_card_total_block;
	uint8_t card_type;
	uint8_t scr[8];
	uint8_t ocr[64];
	uint16_t rca;
	uint16_t csd_4h;
	uint16_t csd_4l;
	uint16_t csd_3h;
	uint16_t csd_3l;
	uint16_t csd_2h;
	uint16_t csd_2l;
	uint16_t csd_1h;
	uint16_t csd_1l;
	uint16_t cid_4h;
	uint16_t cid_4l;
	uint16_t cid_3h;
	uint16_t cid_3l;
	uint16_t cid_2h;
	uint16_t cid_2l;
	uint16_t cid_1h;
	uint16_t cid_1l;
}sdcard_info;

uint32_t sdc_cmd_timeout;
uint8_t sdc_polling_type;
uint8_t sdc_polling_mode;

uint32_t SDC_GetVersion(void)
{
	return SDC_VERSION;
}

uint64_t SDC_Get_Total_Block(void)
{
	return sdcard_info.sd_card_total_block;
}

static int SDC_Check_Busy(void)
{
	int i = 0;
	while(1)
	{
		if(SN_SDC->SD_CTRL_b.SPI_DONE)
			break;
		else if(i >= sdc_cmd_timeout)
			return SDC_RETRY_TIMEOUT;
		else 
			i++;
	}
	
	return SDC_NO_ERR;
}

static int SDC_Write_Dummyclock(uint8_t mode)
{
	SN_SDC->SD_CTRL_b.SPI_RW = mode; /*Give Any value in order to send dummy clock (WR:8, RD:80)*/
	SN_SDC->SD_REG3 = 0;
	
	return (SDC_Check_Busy());
}

static int SDC_Next_Response_Trigger(void)
{
	SN_SDC->SD_CTRL_b.SPI_RW = SDC_READ_MODE; /*Give Any value in order to send dummy clock (WR:8, RD:80)*/
	SN_SDC->SD_REG4 = 0;
	
	return (SDC_Check_Busy());
}

static int SDC_Check_Status(void)
{
	uint16_t status = 0;
	int ret = SDC_NO_ERR;
	uint32_t i = 0;
	
	while((status & SET_BIT1) == 0)
	{
		status = SN_SDC->SD_STAT;
		if(i >= sdc_cmd_timeout)
			return SDC_RETRY_TIMEOUT;
		i++;
	}	
	
	if(status & SET_BIT4){ 
		ret = SDC_CRC_ERR;
	}else if(status & SET_BIT6){ 
		ret = SDC_WRITE_ERR;
	}else if(status & SET_BIT10){
		ret = SDC_RW_TIMEOUT;
	}
	
	SN_SDC->SD_STAT |= (SET_BIT1 | SET_BIT4 | SET_BIT6 | SET_BIT10);
	return ret;
}

/*static */int SDC_Cmd(uint32_t cmd_index, uint16_t cmd_arg_h, uint16_t cmd_arg_l)
{
	int ret;
	
	SN_SDC->SD_CTRL_b.SPI_RW = SDC_WRITE_MODE;
	SN_SDC->SPI_CMD_INDEX = cmd_index;
	SN_SDC->SPI_CMD_ARGH = cmd_arg_h;
	SN_SDC->SPI_CMD_ARGL = cmd_arg_l;
	
	SN_SDC->SD_CTRL_b.SPI_CMD_TRIG = 1;
	
	ret = SDC_Check_Busy();
	if(ret)
		return ret;

	if((cmd_index==SDC_CMD_SELECT_CARD)||(cmd_index==SDC_CMD_STOP_TRANSMISSION)||(cmd_index==SDC_CMD_SET_WRITE_PORT) \
		||(cmd_index==SDC_CMD_CLR_WRITE_PORT)||(cmd_index==SDC_CMD_ERASE))
	{
		SN_SDC->SD_CTRL_b.SPI_BUSY_TRIG = 1;
		ret = SDC_Check_Busy();
		if(ret)
			return ret;
	}

	return SDC_NO_ERR;
}

static int SDC_Reset(void)
{
	int ret;
	
	ret = SDC_Cmd(0, 0x0000, 0x0000); /*sw reset*/
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	
	return SDC_NO_ERR;
}

static int SDC_Check_Version(void)
{
	int ret;
	
	ret = SDC_Cmd(8, 0x0000, 0x01AA); /*check sd version*/
	if(ret)
		return ret;
	if(SN_SDC->SPI_CMD_INDEX == 0x08 && SN_SDC->SPI_CMD_ARGL == 0x01AA)
		sdcard_info.sd_card_version = V20_SDSC;
	else
		sdcard_info.sd_card_version = V1X_SDSC;
	
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	
	return SDC_NO_ERR;
}

static int SDC_Check_Card_Type(void)
{
	int ret;
	int i;

	while(!(SN_SDC->SPI_CMD_ARGH & SET_BIT15))
	{
		ret = SDC_Cmd(55, 0x0000, 0x0000); /*next cmd is an app cmd*/
		if(ret)
			return ret;
		ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
		if(ret)
			return ret;
	
		if(SN_SDC->SPI_CMD_INDEX == 55)		//Detect this card is SD or MMC
			sdcard_info.card_type = SD_CARD;
		else
			sdcard_info.card_type = MMC_CARD;		
		
		if(sdcard_info.sd_card_version == V1X_SDSC)
		{
			ret = SDC_Cmd(41, 0x00FF, 0x8000); /*identify the VDD range*/
			if(ret)
				return ret;
		}
		else
		{
			ret = SDC_Cmd(41, 0x40FF, 0x8000); /*identify the VDD range*/
			if(ret)
				return ret;
		}
		ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
		if(ret)
			return ret;
			
		i++; /*ACMD41 return busy*/
		if(i > 1000)
			return SDC_RETRY_TIMEOUT;
	}

	if(sdcard_info.sd_card_version == V20_SDSC)
	{
		if((SN_SDC->SPI_CMD_INDEX == 0x003F) && (SN_SDC->SPI_CMD_ARGH == 0xC0FF) && (SN_SDC->SPI_CMD_ARGL == 0x8000))
			sdcard_info.sd_card_version = V20_SDHC;
	}
	
	if((SN_SDC->SPI_CMD_INDEX == 0x003F) && (SN_SDC->SPI_CMD_ARGH == 0xFFFF) && (SN_SDC->SPI_CMD_ARGL == 0xFFFF))
		return SDC_NO_SUPPLY_CARD;
	
	return SDC_NO_ERR;
}

static int SDC_Get_CID_Cmd(void)
{
	int ret;
	
	ret = SDC_Cmd(2, 0x0000, 0x0000); /*ask sdcard send cid*/
	if(ret)
		return ret;
	sdcard_info.cid_4h = SN_SDC->SPI_CMD_ARGH;
	sdcard_info.cid_4l = SN_SDC->SPI_CMD_ARGL;
	ret = SDC_Next_Response_Trigger();
	if(ret)
		return ret;
	sdcard_info.cid_3h = (SN_SDC->SPI_CMD_INDEX<<8) + (SN_SDC->SPI_CMD_ARGH>>8);
	sdcard_info.cid_3l = ((SN_SDC->SPI_CMD_ARGH&0x00FF)<<8) + (SN_SDC->SPI_CMD_ARGL>>8);
	sdcard_info.cid_2h = ((SN_SDC->SPI_CMD_ARGL&0x00FF)<<8);
	ret = SDC_Next_Response_Trigger();
	if(ret)
		return ret;
	sdcard_info.cid_2h += SN_SDC->SPI_CMD_INDEX;
	sdcard_info.cid_2l = SN_SDC->SPI_CMD_ARGH;
	sdcard_info.cid_1h = SN_SDC->SPI_CMD_ARGL;
	ret = SDC_Next_Response_Trigger();
	if(ret)
		return ret;
	sdcard_info.cid_1l = (SN_SDC->SPI_CMD_INDEX<<8) + (SN_SDC->SPI_CMD_ARGH>>8);
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;	
	
	return SDC_NO_ERR;
}

static int SDC_Get_RCA(void)
{
	int ret;
	
	ret = SDC_Cmd(3, 0x0000, 0x0000); /*ask sdcard send rca*/
	if(ret)
		return ret;
	
	sdcard_info.rca = SN_SDC->SPI_CMD_ARGH;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	
	return SDC_NO_ERR;
}

static int SDC_Get_CSD_Cmd(void)
{
	int ret;
	
	ret = SDC_Cmd(9, sdcard_info.rca, 0x0000);  /*ask sdcard send csd*/
	if(ret)
		return ret;
	
	sdcard_info.csd_4h = SN_SDC->SPI_CMD_ARGH;
	sdcard_info.csd_4l = SN_SDC->SPI_CMD_ARGL;
	ret = SDC_Next_Response_Trigger();
	if(ret)
		return ret;
	sdcard_info.csd_3h = (SN_SDC->SPI_CMD_INDEX<<8) + (SN_SDC->SPI_CMD_ARGH>>8);
	sdcard_info.csd_3l = ((SN_SDC->SPI_CMD_ARGH & 0x00FF)<<8) + (SN_SDC->SPI_CMD_ARGL>>8);
	sdcard_info.csd_2h = ((SN_SDC->SPI_CMD_ARGL & 0x00FF)<<8);
	ret = SDC_Next_Response_Trigger();
	if(ret)
		return ret;
	sdcard_info.csd_2h += SN_SDC->SPI_CMD_INDEX;
	sdcard_info.csd_2l = SN_SDC->SPI_CMD_ARGH;
	sdcard_info.csd_1h = SN_SDC->SPI_CMD_ARGL;	
	ret = SDC_Next_Response_Trigger();
	if(ret)
		return ret;
	sdcard_info.csd_1l = (SN_SDC->SPI_CMD_INDEX<<8) + (SN_SDC->SPI_CMD_ARGH>>8);
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	
	return SDC_NO_ERR;
}

static int SDC_Select_Card(void)
{
	int ret;
	
	ret = SDC_Cmd(7, sdcard_info.rca, 0x0000); /*card select*/
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;	
	if((SN_SDC->SPI_CMD_INDEX != 0x0007) || (SN_SDC->SPI_CMD_ARGH != 0x0000) || (SN_SDC->SPI_CMD_ARGL != 0x0700))
		return SDC_CMD_ERR;
	
	return SDC_NO_ERR;
}

static int SDC_Set_Bus_Width(uint8_t bus_width)
{
	int ret;
	
	ret = SDC_Cmd(55, sdcard_info.rca, 0x0000); /*next cmd is an app cmd*/
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;	
	if((SN_SDC->SPI_CMD_INDEX != 0x0037) || (SN_SDC->SPI_CMD_ARGH != 0x0000) || (SN_SDC->SPI_CMD_ARGL != 0x0920))
		return SDC_CMD_ERR;
	if(bus_width == SD_BUS_WIDTH_4BIT)
		ret = SDC_Cmd(6, 0x0000, 0x0002); /*set 4-bits bus width*/
	else if(bus_width == SD_BUS_WIDTH_1BIT)
		ret = SDC_Cmd(6, 0x0000, 0x0000); /*set 4-bits bus width*/
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	if((SN_SDC->SPI_CMD_INDEX != 0x0006) || (SN_SDC->SPI_CMD_ARGH != 0x0000) || (SN_SDC->SPI_CMD_ARGL != 0x0920))
		return SDC_CMD_ERR;	
	
	return SDC_NO_ERR;
}

static int SDC_Get_SCR(void)
{
	int ret;
	
	ret = SDC_Cmd(55, sdcard_info.rca, 0x0000); /*next cmd is an app cmd*/
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;	
	if((SN_SDC->SPI_CMD_INDEX != 0x0037) || (SN_SDC->SPI_CMD_ARGH != 0x0000) || (SN_SDC->SPI_CMD_ARGL != 0x0920))
		return SDC_CMD_ERR;
	ret = SDC_Cmd(51, 0x0000, 0x0000); /*read scr sd configuration register*/
	if(ret)
		return ret;
	SN_SDC->SD_CTRL_b.CMD_DATA_R = 1; /*trigger transfer cmd by data line*/
	SN_SDC->SD_RW_LEN = 0x0007;
	{
//		idma_setting_t set;
//		set.dip_addr = 0;
//		set.ram_addr = (volatile uint32_t)sdcard_info.scr;
//		set.length = 8;
//		set.direction = IDMA_DIRECTION_DIP_2_RAM;
//		ret = IDMA_Start(IDMA_CH_SDNF, &set);
//		if(ret)
//			return ret;
//		//while(IDMA_Get_Status(IDMA_CH_SDNF));
		SN_IDMA_SDC->CH_LEN = 8;
		SN_IDMA_SDC->CH_RAM_ADDR = (uint32_t)sdcard_info.scr;
		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 1;
		SN_IDMA_SDC->CH_CTRL_b.START = 1;
	}
	SN_SDC->SD_CMD = 0x0B00; /*DMA read data*/
	SN_SDC->SD_CTRL_b.CMD_DATA_R = 0;
	ret = SDC_Check_Status();
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;	
	
	return SDC_NO_ERR;
}

static int SDC_Check_Support_Highspeed(void)
{
	int cmd6_support;
	int ret;
	
	if((sdcard_info.scr[0] & 0x000F) != 0)
		cmd6_support = 1;
	else
		cmd6_support = 0;
	
	if(cmd6_support)
	{
		ret = SDC_Cmd(6, 0x0000, 0x00FF); /*check support high-speed 25MB/s*/
		if(ret)
			return ret;		
		if((SN_SDC->SPI_CMD_INDEX != 0x0006) || (SN_SDC->SPI_CMD_ARGH != 0x0000) || (SN_SDC->SPI_CMD_ARGL != 0x0900))
			return SDC_CMD_ERR;
		SN_SDC->SD_RW_LEN = 0x003F;
		/*get ocr: store VDD profile of the card*/
		{
//			idma_setting_t set;
//			set.dip_addr = 0;
//			set.ram_addr = (volatile uint32_t)sdcard_info.ocr;
//			set.length = 0x40;
//			set.direction = IDMA_DIRECTION_DIP_2_RAM;
//			ret = IDMA_Start(IDMA_CH_SDNF, &set);
//			if(ret)
//				return ret;
//			//while(IDMA_Get_Status(IDMA_CH_SDNF));
			SN_IDMA_SDC->CH_RAM_ADDR = (uint32_t)sdcard_info.ocr;
			SN_IDMA_SDC->CH_LEN = 0x40;
			SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 1;
			SN_IDMA_SDC->CH_CTRL_b.START = 1;
		}
		SN_SDC->SD_CMD = 0x0B00; /*DMA read data*/
		ret = SDC_Check_Status();
		if(ret)
			return ret;
		ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
		if(ret)
			return ret;			
		
		if((sdcard_info.ocr[6] & SET_BIT9))
			sdcard_info.support_hi_speed = 1;
		else
			sdcard_info.support_hi_speed = 0;
	}
	else
		sdcard_info.support_hi_speed = 0;
	
	return SDC_NO_ERR;
}

static void SDC_Set_Speed(uint8_t speed)
{
	sdcard_info.sd_speed = speed;
	SN_SDC->SD_SPEED = (speed<<8);
}

static void SDC_Count_Total_Block(void)
{
	uint32_t tmp;
	
	if((sdcard_info.csd_4h & 0xC000) == 0x4000)	/*CSD Ver2.0*/
	{
		sdcard_info.sd_card_total_block = (uint32_t)(sdcard_info.csd_2h + 1)<<10;
	}
	else if((sdcard_info.csd_4h & 0xC000) == 0x0000)	/*CSD Ver1.0*/
	{
		sdcard_info.sd_card_total_block = (((sdcard_info.csd_3l & 0x03FF)<<2) + ((sdcard_info.csd_2h & 0xC000)>>14) + 1);
		tmp = (uint32_t)(((sdcard_info.csd_2h & 0x0003)<<1) + ((sdcard_info.csd_2l & 0x8000)>>15) + 2) + (sdcard_info.csd_3h & 0x000F);
		sdcard_info.sd_card_total_block <<= tmp;
		sdcard_info.sd_card_total_block >>= 9;
	}
}

void SDC_Get_Clock(uint8_t sd_port, uint32_t sys_clock, uint32_t *sd_clock)
{
	if(sd_port == SD_CARD_PORT_0)
		*sd_clock = sys_clock/(sdcard_info.sd_speed + 2);
}

static int SDC_Read_Check_Transfer_Mode(void)
{
	int ret = SDC_RETRY_TIMEOUT;
	
	if(sdc_polling_mode != SD_4BIT_MODE)
	{
		if(SN_SDC->SPI_CMD_INDEX == 0x0000)
			ret = SDC_NO_ERR;
	}
	else
	{
		if(sdc_polling_type == 0)
		{
			if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0B00))
				ret = SDC_NO_ERR;
			else if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x8000) && (SN_SDC->SPI_CMD_ARGL == 0x0B00))
				ret = SDC_NO_ERR;
		}
		else if(sdc_polling_type == TAIL)
		{
			if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0B00))
				ret = SDC_NO_ERR;
		}
		else
		{
			if((SN_SDC->SPI_CMD_INDEX == 0x0012) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0900))
				ret = SDC_NO_ERR;
		}
	}	
	
	sdc_polling_mode = 0;
	sdc_polling_type = 0;
	
	return ret;
}

static int SDC_Write_Check_Transfer_Mode(void)
{
	int ret = SDC_RETRY_TIMEOUT;
	
	if(sdc_polling_mode != SD_4BIT_MODE)
	{
		if(SN_SDC->SPI_CMD_INDEX == 0x0000)
			ret = SDC_NO_ERR;
	}
	else
	{
		if(sdc_polling_type == 0)
		{
			if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0C00))
				ret = SDC_NO_ERR;
		}
		else if(sdc_polling_type == TAIL)
		{
			if((SN_SDC->SPI_CMD_INDEX != 0x00FF) && (SN_SDC->SPI_CMD_ARGH == 0x0000) &&  ((SN_SDC->SPI_CMD_ARGL &(0xFFFFFEFF)) == 0x0C00))
				ret = SDC_NO_ERR;
		}
		else
		{
			if((SN_SDC->SPI_CMD_INDEX != 0x00FF) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0900))
				ret = SDC_NO_ERR;
		}
	}
	
	sdc_polling_mode = 0;
	sdc_polling_type = 0;
	
	return ret;
}

int SDC_Polling_Flag(void)
{
	uint16_t status;
	int ret = SDC_NO_ERR;
	
	if(!sdc_polling_mode)/*no r/w cmd*/
		return ret;
	
	status = SN_SDC->SD_STAT;
	if(status & SET_BIT1)
	{
		if(status & SET_BIT4)
			ret = SDC_CRC_ERR;
		else if(status & SET_BIT6)
			ret = SDC_WRITE_ERR;
		else if(status & SET_BIT10)
			ret = SDC_RW_TIMEOUT;
		
		SN_SDC->SD_STAT |= (SET_BIT1 | SET_BIT4 | SET_BIT6 | SET_BIT10);
	}
	else
	{
		ret = SDC_BUSY;
		return ret;
	}
	
	if(ret)
	{
		sdc_polling_type = 0;
		sdc_polling_mode = 0;
		return ret;
	}
	
	if(sdc_polling_mode & SET_BIT7)
	{
		sdc_polling_mode &= ~SET_BIT7;
		ret = SDC_Write_Check_Transfer_Mode();
	}
	else
	{
		ret = SDC_Read_Check_Transfer_Mode();
	}
	
	return ret;
}


int SDC_RW_Polling(uint8_t sd_port)
{
	int ret;
	int i;
	
	if(sd_port == SD_CARD_PORT_0){
		while(1)
		{
			ret = SDC_Polling_Flag();
			if(ret != SDC_BUSY)
				return ret;
			i++;
			if(i>SDC_CMD_TIMEOUT)
				return SDC_RW_TIMEOUT;
		}
	}
	
	return SDC_NO_ERR;
}

/*static*/ int SDC_Read_Single_Block(uint32_t ram_addr, uint32_t lba, uint8_t sd_mode)
{
	int ret;
	
	if(lba <= sdcard_info.sd_card_total_block)
	{
		SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
		{
			idma_setting_t set;
			set.dip_addr = 0;
			set.ram_addr = ram_addr;
			set.length = 0x200;
			set.direction = IDMA_DIRECTION_DIP_2_RAM;
			ret = IDMA_Start(IDMA_CH_SDNF, &set);
			if(ret)
				return ret;
			//while(IDMA_Get_Status(IDMA_CH_SDNF));
	//		SN_IDMA_SDC->CH_LEN = 0x200;
	//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
	//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 1;
	//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
		}
		if(sdcard_info.sd_card_version == V20_SDHC)
		{
			SN_SDC->SPI_CMD_ARGH = (lba>>16);
			SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
		}
		else
		{
			SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
			SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);
		}
		SN_SDC->SD_CMD = 0x080A;

		sdc_polling_type = 0;
		sdc_polling_mode = sd_mode;
		return SDC_NO_ERR;
	}
	return SDC_FUNC_PARAMETER_ERR;
}

/*static*/ int SDC_Read_Multi_Block(uint32_t ram_addr, uint32_t lba, uint8_t block_header, uint8_t sd_mode)
{
	int ret;

	if(lba <= sdcard_info.sd_card_total_block)
	{
		SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
		{
			idma_setting_t set;
			set.dip_addr = 0;
			set.ram_addr = ram_addr;
			set.length = 0x200;
			set.direction = IDMA_DIRECTION_DIP_2_RAM;
			ret = IDMA_Start(IDMA_CH_SDNF, &set);
			if(ret)
				return ret;
			//while(IDMA_Get_Status(IDMA_CH_SDNF));
	//		SN_IDMA_SDC->CH_LEN = 0x200;
	//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
	//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 1;
	//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
		}
		if(block_header == HEAD)
		{
			if(sdcard_info.sd_card_version == V20_SDHC)
			{
				SN_SDC->SPI_CMD_ARGH = (lba>>16);
				SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
			}
			else
			{
				SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
				SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);
			}
			SN_SDC->SD_CMD = 0x0808;
		}
		else if(block_header == MIDDLE)
		{
			SN_SDC->SD_CMD = 0x0800;
		}
		else if(block_header == TAIL)
		{
			SN_SDC->SD_CMD = 0x0802;
		}
		
		sdc_polling_type = block_header;
		sdc_polling_mode = sd_mode;
		return SDC_NO_ERR;
	}
	return SDC_FUNC_PARAMETER_ERR;
}

/*static*/ int SDC0_Read_Block(uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	uint32_t tmp_block_num = block_num;
	int ret;
	
	if(block_num > 1)
	{
		while(tmp_block_num > 0)
		{
			if(tmp_block_num == block_num)
			{
				ret = SDC_Read_Multi_Block(ram_addr, lba, HEAD, SD_4BIT_MODE);
			}
			else if(tmp_block_num == 1)
			{
				ret = SDC_Read_Multi_Block(ram_addr, lba, TAIL, SD_4BIT_MODE);
			}
			else
			{
				ret = SDC_Read_Multi_Block(ram_addr, lba, MIDDLE, SD_4BIT_MODE);
			}
			if(ret)
				return ret;
			tmp_block_num--;
			lba++;
		}
	}
	else
	{
		ret = SDC_Read_Single_Block(ram_addr, lba, SD_4BIT_MODE);
		if(ret)
			return ret;
	}
	
	return SDC_NO_ERR;
}

/*static*/ int SDC_Write_Single_Block	(uint32_t ram_addr, uint32_t lba, uint8_t sd_mode)
{
	int ret;

	if(lba <= sdcard_info.sd_card_total_block)
	{	
		SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
		{
			idma_setting_t set;
			set.dip_addr = 0;
			set.ram_addr = ram_addr;
			set.length = 0x200;
			set.direction = IDMA_DIRECTION_RAM_2_DIP;
			ret = IDMA_Start(IDMA_CH_SDNF, &set);
			if(ret)
				return ret;
			//while(IDMA_Get_Status(IDMA_CH_SDNF));
	//		SN_IDMA_SDC->CH_LEN = 0x200;
	//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
	//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 0;
	//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
		}
		if(sdcard_info.sd_card_version == V20_SDHC)
		{
			SN_SDC->SPI_CMD_ARGH = (lba>>16);
			SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
		}
		else
		{
			SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
			SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);	
		}
		SN_SDC->SD_CMD = 0x000A;

		sdc_polling_type = 0;
		sdc_polling_mode = sd_mode | SET_BIT7;
		return SDC_NO_ERR;
	}		
	return SDC_FUNC_PARAMETER_ERR;
}
	
/*static*/ int SDC_Write_Multi_Block(uint32_t ram_addr, uint32_t lba, uint8_t block_header, uint8_t sd_mode)
{	
	int ret;

	if(lba <= sdcard_info.sd_card_total_block)
	{		
		SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
		{
			idma_setting_t set;
			set.dip_addr = 0;
			set.ram_addr = ram_addr;
			set.length = 0x200;
			set.direction = IDMA_DIRECTION_RAM_2_DIP;
			ret = IDMA_Start(IDMA_CH_SDNF, &set);
			if(ret)
				return ret;
			//while(IDMA_Get_Status(IDMA_CH_SDNF));
	//		SN_IDMA_SDC->CH_LEN = 0x200;
	//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
	//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 0;
	//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
		}
		if(block_header == HEAD)
		{
			if(sdcard_info.sd_card_version == V20_SDHC)
			{
				SN_SDC->SPI_CMD_ARGH = (lba>>16);
				SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
			}
			else
			{
				SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
				SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);
			}
			
			SN_SDC->SD_CMD = 0x0008;
		}
		else if(block_header == MIDDLE)
		{
			SN_SDC->SD_CMD = 0x0000;
		}
		else if(block_header == TAIL)
		{
			SN_SDC->SD_CMD = 0x0002;
		}
		sdc_polling_type = block_header;
		sdc_polling_mode = sd_mode | SET_BIT7;	
		return SDC_NO_ERR;
	}	
	return SDC_FUNC_PARAMETER_ERR;
}
	
/*static*/ int SDC0_Write_Block(uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	uint32_t tmp_block_num = block_num;
	int ret;
	
	if(block_num > 1)
	{
		while(tmp_block_num > 0)
		{
			if(tmp_block_num == block_num)
			{
				ret = SDC_Write_Multi_Block(ram_addr, lba, HEAD, SD_4BIT_MODE);
			}
			else if(tmp_block_num == 1)
			{
				ret = SDC_Write_Multi_Block(ram_addr, lba, TAIL, SD_4BIT_MODE);
			}
			else
			{
				ret = SDC_Write_Multi_Block(ram_addr, lba, MIDDLE, SD_4BIT_MODE);
			}
			if(ret)
				return ret;
			tmp_block_num--;
			lba++;
		}
	}
	else
	{
		ret = SDC_Write_Single_Block(ram_addr, lba, SD_4BIT_MODE);;
		if(ret)
			return ret;
	}
	
	return SDC_NO_ERR;
}

int SDC_Read_Block(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	int ret;
	
	if(sd_port == SD_CARD_PORT_0)
		ret = SDC0_Read_Block(ram_addr, lba, block_num);
	
	return ret;
}

int SDC_Write_Block(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	int ret;
	
	if(sd_port == SD_CARD_PORT_0)
		SDC0_Write_Block(ram_addr, lba, block_num);
	
	return ret;
}

/*static*/ int SDC_Read_Single_Block_Blocking(uint32_t ram_addr, uint32_t lba, uint8_t sd_mode)
{
	int ret;
	
	SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
	{
		idma_setting_t set;
		set.dip_addr = 0;
		set.ram_addr = ram_addr;
		set.length = 0x200;
		set.direction = IDMA_DIRECTION_DIP_2_RAM;
		ret = IDMA_Start(IDMA_CH_SDNF, &set);
		if(ret)
			return ret;
		//while(IDMA_Get_Status(IDMA_CH_SDNF));
//		SN_IDMA_SDC->CH_LEN = 0x200;
//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 1;
//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
	}
	if(sdcard_info.sd_card_version == V20_SDHC)
	{
		SN_SDC->SPI_CMD_ARGH = (lba>>16);
		SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
	}
	else
	{
		SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
		SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);
	}
	SN_SDC->SD_CMD = 0x080A;

	ret = SDC_Check_Status();
	if(ret)
		return ret;

	if(sd_mode == SD_4BIT_MODE)
	{
		if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0B00))
			return SDC_NO_ERR;
		else if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x8000) && (SN_SDC->SPI_CMD_ARGL == 0x0B00))
			return SDC_NO_ERR;
		else
			return SDC_CMD_ERR;
	}
	else
	{
		if(SN_SDC->SPI_CMD_INDEX == 0x0000)
			return SDC_NO_ERR;
		else
			return SDC_CMD_ERR;
	}
}

/*static*/ int SDC_Read_Multi_Block_Blocking(uint32_t ram_addr, uint32_t lba, uint8_t block_type, uint8_t sd_mode)
{
	int ret;
	
	SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
	{
		idma_setting_t set;
		set.dip_addr = 0;
		set.ram_addr = ram_addr;
		set.length = 0x200;
		set.direction = IDMA_DIRECTION_DIP_2_RAM;
		ret = IDMA_Start(IDMA_CH_SDNF, &set);
		if(ret)
			return ret;
		//while(IDMA_Get_Status(IDMA_CH_SDNF));
//		SN_IDMA_SDC->CH_LEN = 0x200;
//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 1;
//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
	}	
	if(block_type == HEAD)
	{
		if(sdcard_info.sd_card_version == V20_SDHC)
		{
			SN_SDC->SPI_CMD_ARGH = (lba>>16);
			SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
		}
		else
		{
			SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
			SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);
		}
		SN_SDC->SD_CMD = 0x0808;
	}
	else if(block_type == MIDDLE)
	{
		SN_SDC->SD_CMD = 0x0800;
	}
	else if(block_type == TAIL)
	{
		SN_SDC->SD_CMD = 0x0802;
	}
	
	ret = SDC_Check_Status();
	if(ret)
		return ret;
	
	if(block_type != TAIL)
	{
		if(sd_mode == SD_4BIT_MODE)
		{
			if((SN_SDC->SPI_CMD_INDEX == 0x0012) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0900))
				return SDC_NO_ERR;
			else
				return SDC_CMD_ERR;
		}
		else
		{
			if(SN_SDC->SPI_CMD_INDEX == 0x0000)
				return SDC_NO_ERR;
		}
	}
	else
	{
		if(sd_mode == SD_4BIT_MODE)
		{
			if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0B00))
				return SDC_NO_ERR;
			else
				return SDC_CMD_ERR;
		}
		else
		{
			if(SN_SDC->SPI_CMD_INDEX == 0x0000)
				return SDC_NO_ERR;	
		}
	}
	
	return SDC_NO_ERR;
}

static int SDC0_Read_Block_Blocking(uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	uint32_t tmp_block_num = block_num;
	int ret;
	
	if(block_num > 1)
	{
		while(tmp_block_num > 0)
		{
			if(tmp_block_num == block_num)
			{
				ret = SDC_Read_Multi_Block_Blocking(ram_addr, lba, HEAD, SD_4BIT_MODE);
				if(ret)
					return ret;	
			}
			else if(tmp_block_num == 1)
			{
				ret = SDC_Read_Multi_Block_Blocking(ram_addr, lba, TAIL, SD_4BIT_MODE);
				if(ret)
					return ret;
			}
			else
			{
				ret = SDC_Read_Multi_Block_Blocking(ram_addr, lba, MIDDLE, SD_4BIT_MODE);
				if(ret)
					return ret;
			}			
			tmp_block_num--;
			lba++;
		}
	}
	else
	{
		ret = SDC_Read_Single_Block_Blocking(ram_addr, lba, SD_4BIT_MODE);;
		if(ret)
			return ret;
	}
	
	return SDC_NO_ERR;
}

/*static*/ int SDC_Write_Single_Block_Blocking(uint32_t ram_addr, uint32_t lba, uint8_t sd_mode)
{
	int ret;
	
	if(sdcard_info.sd_card_version == V20_SDHC)
	{
		SN_SDC->SPI_CMD_ARGH = (lba>>16);
		SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
	}
	else
	{
		SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
		SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);	
	}
	
	SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
	{
		idma_setting_t set;
		set.dip_addr = 0;
		set.ram_addr = ram_addr;
		set.length = 0x200;
		set.direction = IDMA_DIRECTION_RAM_2_DIP;
		ret = IDMA_Start(IDMA_CH_SDNF, &set);
		if(ret)
			return ret;
		//while(IDMA_Get_Status(IDMA_CH_SDNF));
//		SN_IDMA_SDC->CH_LEN = 0x200;
//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 0;
//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
	}
	SN_SDC->SD_CMD = 0x000A;
	
	ret = SDC_Check_Status();
	if(ret)
		return ret;

	if(sd_mode == SD_4BIT_MODE)
	{
		if((SN_SDC->SPI_CMD_INDEX == 0x000C) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && ((SN_SDC->SPI_CMD_ARGL&(0xFFFFFEFF)) == 0x0C00)) 
			return SDC_NO_ERR;
		else
			return SDC_CMD_ERR;
	}
	else
	{
		if(SN_SDC->SPI_CMD_INDEX == 0x0000)
			return SDC_NO_ERR;
	}
	
	return SDC_NO_ERR;
}

/*static*/ int SDC_Write_Multi_Block_Blocking(uint32_t ram_addr, uint32_t lba, uint8_t block_type, uint8_t sd_mode)
{
	int ret;
	
	SN_SDC->SD_RW_LEN = SDC_BLOCK_LENGTH;
	{
		idma_setting_t set;
		set.dip_addr = 0;
		set.ram_addr = ram_addr;
		set.length = 0x200;
		set.direction = IDMA_DIRECTION_RAM_2_DIP;
		ret = IDMA_Start(IDMA_CH_SDNF, &set);
		if(ret)
			return ret;
		//while(IDMA_Get_Status(IDMA_CH_SDNF));
//		SN_IDMA_SDC->CH_LEN = 0x200;
//		SN_IDMA_SDC->CH_RAM_ADDR = ram_addr;
//		SN_IDMA_SDC->CH_CTRL_b.WR_RAM = 0;
//		SN_IDMA_SDC->CH_CTRL_b.START = 1;
	}
	if(block_type == HEAD)
	{
		if(sdcard_info.sd_card_version == V20_SDHC)
		{
			SN_SDC->SPI_CMD_ARGH = (lba>>16);
			SN_SDC->SPI_CMD_ARGL = (lba & 0x0000FFFF);
		}
		else
		{
			SN_SDC->SPI_CMD_ARGH = ((lba<<9)>>16);
			SN_SDC->SPI_CMD_ARGL = ((lba<<9)&0x0000FFFF);
		}
		
		SN_SDC->SD_CMD = 0x0008;
	}
	else if(block_type == MIDDLE)
	{
		SN_SDC->SD_CMD = 0x0000;
	}
	else if(block_type == TAIL)
	{
		SN_SDC->SD_CMD = 0x0002;
	}

	ret = SDC_Check_Status();
	if(ret)
		return ret;

	if(block_type != TAIL)
	{
		if(sd_mode == SD_4BIT_MODE)
		{
			if((SN_SDC->SPI_CMD_INDEX != 0x00FF) && (SN_SDC->SPI_CMD_ARGH == 0x0000) && (SN_SDC->SPI_CMD_ARGL == 0x0900))
				return SDC_NO_ERR;
			else
				return SDC_CMD_ERR;
		}
		else
		{
			if(SN_SDC->SPI_CMD_INDEX == 0x0000)
				return SDC_NO_ERR;
		}
	}
	else
	{
		if(sd_mode == SD_4BIT_MODE)
		{
			if((SN_SDC->SPI_CMD_INDEX != 0x00FF) && (SN_SDC->SPI_CMD_ARGH == 0x0000) &&  ((SN_SDC->SPI_CMD_ARGL &(0xFFFFFEFF)) == 0x0C00))
				return SDC_NO_ERR;
			else
				return SDC_CMD_ERR;
		}
		else
		{
			if(SN_SDC->SPI_CMD_INDEX == 0x0000)
				return SDC_NO_ERR;
		}
	}
	return SDC_NO_ERR;
}

static int SDC0_Write_Block_Blocking(uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	uint32_t tmp_block_num = block_num;
	int ret;
	
	if(block_num > 1)
	{
		while(tmp_block_num > 0)
		{
			if(tmp_block_num == block_num)
			{
				ret = SDC_Write_Multi_Block_Blocking(ram_addr, lba, HEAD, SD_4BIT_MODE);
				if(ret)
					return ret;	
			}
			else if(tmp_block_num == 1)
			{
				ret = SDC_Write_Multi_Block_Blocking(ram_addr, lba, TAIL, SD_4BIT_MODE);
				if(ret)
					return ret;
			}
			else
			{
				ret = SDC_Write_Multi_Block_Blocking(ram_addr, lba, MIDDLE, SD_4BIT_MODE);
				if(ret)
					return ret;
			}			
			tmp_block_num--;
			lba++;
		}
	}
	else
	{
		ret = SDC_Write_Single_Block_Blocking(ram_addr, lba, SD_4BIT_MODE);;
		if(ret)
			return ret;
	}
	
	return SDC_NO_ERR;
}

int SDC_Read_Block_Blocking(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	int ret;
	
	if(sd_port == SD_CARD_PORT_0)
		ret = SDC0_Read_Block_Blocking(ram_addr, lba, block_num);
	
	return ret;
}

int SDC_Write_Block_Blocking(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num)
{
	int ret;
	
	if(sd_port == SD_CARD_PORT_0)
		ret = SDC0_Write_Block_Blocking(ram_addr, lba, block_num);
	
	return ret;
}

static int SDC0_Erase(uint32_t start_lba, uint32_t end_lba)
{
	uint16_t erase_block_addr_l;
	uint16_t erase_block_addr_h;
	int ret;
	
	if(sdcard_info.sd_card_version != V20_SDHC)
	{
		start_lba <<= 9;
		end_lba <<= 9;
	}
	
	erase_block_addr_l = start_lba & 0xffff;
	erase_block_addr_h = start_lba >> 16;
	erase_block_addr_h &= 0xffff;
	
	ret = SDC_Cmd(32, erase_block_addr_h, erase_block_addr_l);
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	
	erase_block_addr_l = end_lba & 0xffff;
	erase_block_addr_h = end_lba >> 16;
	erase_block_addr_h &= 0xffff;	
	
	ret = SDC_Cmd(33, erase_block_addr_h, erase_block_addr_l);
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;	
	
	ret = SDC_Cmd(38, 0x0000, 0x0000);
	if(ret)
		return ret;
	ret = SDC_Write_Dummyclock(SDC_WRITE_MODE);
	if(ret)
		return ret;
	
	return SDC_NO_ERR;
}

int SDC_Erase(uint8_t sd_port, uint32_t start_lba, uint32_t end_lba)
{
	int ret;
	
	if(sd_port == SD_CARD_PORT_0)
		ret = SDC0_Erase(start_lba, end_lba);
	
	return ret;
}

void SDC_Get_CSD(uint8_t sd_port, uint32_t *csd)
{

	if(sd_port == SD_CARD_PORT_0)
	{
		*(csd + 0) = 0x0;
		*(csd + 0) |= sdcard_info.csd_1h;
		*(csd + 0) = (*(csd + 0) << 16) + sdcard_info.csd_1l;
		*(csd + 1) = 0x0;
		*(csd + 1) |= sdcard_info.csd_2h;
		*(csd + 1)  = (*(csd + 1)  << 16) + sdcard_info.csd_2l;
		*(csd + 2) = 0x0;
		*(csd + 2) |= sdcard_info.csd_3h;
		*(csd + 2) = (*(csd + 2) << 16) + sdcard_info.csd_3l;
		*(csd + 3) = 0x0;
		*(csd + 3) |= sdcard_info.csd_4h;
		*(csd + 3) = (*(csd + 3) << 16) + sdcard_info.csd_4l;
	}
}

void SDC_Get_CID(uint8_t sd_port, uint32_t *cid)
{
	if(sd_port == SD_CARD_PORT_0)
	{
		*(cid + 0) = 0x0;
		*(cid + 0) |= sdcard_info.cid_1h;
		*(cid + 0) = (*(cid + 0) << 16) + sdcard_info.cid_1l;
		*(cid + 1) = 0x0;
		*(cid + 1) |= sdcard_info.cid_2h;
		*(cid + 1)  = (*(cid + 1)  << 16) + sdcard_info.cid_2l;
		*(cid + 2) = 0x0;
		*(cid + 2) |= sdcard_info.cid_3h;
		*(cid + 2) = (*(cid + 2) << 16) + sdcard_info.cid_3l;
		*(cid + 3) = 0x0;
		*(cid + 3) |= sdcard_info.cid_4h;
		*(cid + 3) = (*(cid + 3) << 16) + sdcard_info.cid_4l;
	}
}

void SDC_Get_Card_Type(uint8_t sd_port, uint8_t *card_type)
{
	if(sd_port == SD_CARD_PORT_0)
		*card_type = sdcard_info.card_type;
}

void SDC_Get_OCR(uint8_t sd_port, uint8_t *ocr)
{
	if(sd_port == SD_CARD_PORT_0)
	{
		*(ocr + 0) = sdcard_info.ocr[0];
		*(ocr + 1) = sdcard_info.ocr[1];
		*(ocr + 2) = sdcard_info.ocr[2];
		*(ocr + 3) = sdcard_info.ocr[3];
		*(ocr + 4) = sdcard_info.ocr[4];
		*(ocr + 5) = sdcard_info.ocr[5];
		*(ocr + 6) = sdcard_info.ocr[6];
		*(ocr + 7) = sdcard_info.ocr[7];
	}
}

void SDC_Get_Capacity(uint8_t sd_port, uint64_t *capacity)
{
	if(sd_port == SD_CARD_PORT_0)
	{	
		*capacity = sdcard_info.sd_card_total_block * 512;
	}
}

static uint8_t SDC0_Init(void)
{
	int ret;
	
	sdc_polling_type = 0;
	sdc_polling_mode = 0;
	
//	if(SDC_CMD_TIMEOUT < SDC_MIN_CMD_TIMEOUT)
//		sdc_cmd_timeout = SDC_MIN_CMD_TIMEOUT;
//	else
//		sdc_cmd_timeout = SDC_CMD_TIMEOUT;	
	
	sdc_cmd_timeout = SDC_CMD_TIMEOUT;
	
	SN_SDC->SD_CTRL_b.SD_MODE = SDC_SD_MODE;
	SN_SDC->SD_CTRL_b.CRC_EN = SDC_CRC_ENABLE;
	SN_SDC->SD_SPEED = 0x9E00; /*Set SDC Init speed < 400KHz*/
	SN_SDC->RESPONSE_MODE = SDC_AUTO_RESPONSE_MODE;
	SN_SDC->TIME_CNT_L = 0xFFFF;
	SN_SDC->TIME_CNT_H = 0xFFFF;
	SN_SDC->SD_STAT_b.TO_DONE = 1; /*clear timeout flag*/
	ret = SDC_Write_Dummyclock(SDC_READ_MODE);
	if(ret)
		return ret;
	ret = SDC_Reset();
	if(ret)
		return ret;
	ret = SDC_Check_Version();
	if(ret)
		return ret;
	ret = SDC_Check_Card_Type();
	if(ret)
		return ret;
	ret = SDC_Get_CID_Cmd();
	if(ret)
		return ret;
	ret = SDC_Get_RCA();
	if(ret)
		return ret;
	ret = SDC_Get_CSD_Cmd();
	if(ret)
		return ret;
	
	SDC_Set_Speed(0);
	
	ret = SDC_Select_Card();
	if(ret)
		return ret;
	ret = SDC_Set_Bus_Width(SD_BUS_WIDTH_4BIT);
	if(ret)
		return ret;
	ret = SDC_Get_SCR();
	if(ret)
		return ret;
	ret = SDC_Check_Support_Highspeed();
	if(ret)
		return ret;
	SDC_Count_Total_Block();
	
	return SDC_NO_ERR;
}

int SDC_Init(uint8_t sd_port)
{
	int ret;

	if(sd_port == SD_CARD_PORT_0)
		ret = SDC0_Init();
	
	return ret;
}

static void SDC0_Deinit(void)
{
	SN_SDC->SD_CTRL_b.SD_MODE= 0;
	sdcard_info.sd_card_total_block = 0;
}

void SDC_Deinit(uint8_t sd_port)
{
	if(sd_port == SD_CARD_PORT_0)
		SDC0_Deinit();
}
