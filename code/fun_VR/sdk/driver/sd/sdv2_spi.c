/**
 * @file
 * this is sdv2 spi driver file
 * sdv2_spi.c
 * @author CJ
 */
 
//#include <nonstdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "sdv2.h"
#include "sdv2_sd.h"

//===============================================================
void sd_spi_resp1(struct sd_m2_command *cmd_info)
{
	cmd_info->resp[0] = sdReadSpiIndex();
}

//===============================================================
void sd_spi_resp2(struct sd_m2_command *cmd_info)
{
	cmd_info->resp[0] = sdReadSpiIndex();
}

//===============================================================
void sd_spi_resp3(struct sd_m2_command *cmd_info)
{
	uint32_t resp1 = 0x0;
	uint32_t resp2 = 0x0;

	resp1 = sdReadSpiIndex();
	resp2 = sdReadSpiCmd();

	cmd_info->resp[0] = (resp1&0x000000ff)<<24;
	cmd_info->resp[0] |= (resp2&0xffffff00)>>8;
	cmd_info->resp[1] = (resp2&0x000000ff)>>0;
}

//===============================================================
void sd_spi_resp7(struct sd_m2_command *cmd_info)
{
	uint32_t resp1 = 0x0;
	uint32_t resp2 = 0x0;

	// 1rd byte (high->low)
	resp1 = sdReadSpiIndex();

	if((resp1&0x4)==0x1){
		cmd_info->resp[0] = (resp1&0x000000ff)<<24;	
	}else{
		cmd_info->resp[0] = (resp1&0x000000ff)<<24;		

		//Get respose 
		// 2rd  byte (high->low)
		sdMsRegRWSwitch(SD_READ_MODE); //read mode
		sdWriteCommand(0x1);
		while(1){
			if(sdCheckMsRdy()){
				break;
			}
		}
		resp2 = sdReadCommand();
		cmd_info->resp[0] |= (resp2&0xff)<<16 ;

		// 3rd  byte (high->low)
		sdMsRegRWSwitch(SD_READ_MODE); //read mode
		sdWriteCommand(0x1);
		while(1){
			if(sdCheckMsRdy()){
				break;
			}
		}
		resp2 = sdReadCommand();
		cmd_info->resp[0] |= (resp2&0xff)<<8 ;

		// 4rd  byte (high->low)
		sdMsRegRWSwitch(SD_READ_MODE); //read mode
		sdWriteCommand(0x1);
		while(1){
			if(sdCheckMsRdy()){
				break;
			}
		}
		resp2 = sdReadCommand();
		cmd_info->resp[0] |= (resp2&0xff)<<0 ;

		// 5rd  byte (high->low)
		sdMsRegRWSwitch(SD_READ_MODE); //read mode
		sdWriteCommand(0x1);
		while(1){
			if(sdCheckMsRdy()){
				break;
			}
		}
		resp2 = sdReadCommand();
		cmd_info->resp[1] = resp2;
	}
}

//========================================================================
void sd_spi_m2_cmd(uint32_t index, uint32_t arg, int resp_type,struct sd_m2_command *cmd_info)
{
	sd_m2setcmd(index, arg, resp_type, cmd_info);
	sd_spi_m2_WcmdRresp(cmd_info);

	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1); 
	while(1){
		if(sdCheckMsRdy()){
			break;
		}
	}
}

//========================================================================
int sd_spi_read_cmd(uint32_t start_addr, struct sd_m2_command *cmd_info)
{
	//CMD17
	sd_m2setcmd(SD_CMD17_INDEX, start_addr, SD_CMD17_SPI_RESPTYPE, cmd_info);
	sd_spi_m2_WcmdRresp(cmd_info);

	if(cmd_info->resp[0] != 0x0){
		printf("Error: CMD17 response error\n");
		return SD_RTN_ERR_RSP_CMD17;
	}
	sd_resetm2cmd(cmd_info);
	
	return SD_RTN_PASS;
}

//=======================================================================
int sd_spi_write_cmd(uint32_t start_addr, uint32_t dma_size, struct sd_m2_command *cmd_info)
{
	uint32_t block_size = 0x0;

	block_size = dma_size;

	//CMD16__set block size
	sd_spi_m2_cmd(SD_CMD16_INDEX, block_size, SD_CMD16_SPI_RESPTYPE, cmd_info);
	if(cmd_info->resp[0] != 0x0){
		printf("Error: CMD16 response error\n");
		return SD_RTN_ERR_RSP_CMD16;
	}
	sd_resetm2cmd(cmd_info);

	//CMD24__set sd card start address
	sd_m2setcmd(SD_CMD24_INDEX, start_addr, SD_CMD24_SPI_RESPTYPE, cmd_info);
	sd_spi_m2_WcmdRresp(cmd_info);
	if(cmd_info->resp[0] != 0x0){
		printf("Error: CMD24 response error\n");
		return SD_RTN_ERR_RSP_CMD24;
	}
	sd_resetm2cmd(cmd_info);

	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);

	while(1){
		if(sdCheckMsRdy()){
			break;
		}
	}
	
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1); 
	while(1) {
		if(sdCheckMsRdy()){
			break;
		}
	}
	
	return SD_RTN_PASS;
}
//=======================================================================
void sd_spi_m2_WcmdRresp(struct sd_m2_command *cmd_info)
{
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteSpiIndex(cmd_info->index);
	sdWriteSpiCmd(cmd_info->arg);
	
 	sdSetSpiCmdTri(ENABLE);
	while(1){
		if(sdCheckMsRdy()){
			break;
		}
	}

	switch(cmd_info->resp_type)
	{
		case 0:
			break;
		case 1:
			sd_spi_resp1(cmd_info);
			break;
		case 2:
			sd_spi_resp2(cmd_info);
			break;
		case 3:
			sd_spi_resp3(cmd_info);
			break;
		case 7:
			sd_spi_resp7(cmd_info);
			break;
	}
}

//========================================================================
int sd_spi_identify(struct sd_info *info)
{
	uint32_t OCRValue;
	struct sd_m2_command m2_cmd_info;	

	sdSetMsSpeed(0xff);

	/*set CE signal=1*/
	sdMsIOOE_8(ENABLE);
	sdMsIOO_8(ENABLE);

	/*80 dummy clock*/
	sdMsRegRWSwitch(SD_READ_MODE);
	sdWriteDummyClock(0x1);
	while(1){
		if(sdCheckMsRdy()){
			break;
		}
	}

	sdMsIOO_8(DISABLE);

	/*CMD0*/
	sd_spi_m2_cmd(SD_CMD0_INDEX, SD_CMD0_SPI_ARG_INIT, SD_CMD0_SPI_RESPTYPE, &m2_cmd_info);
	if(m2_cmd_info.resp[0] != 0x1){
		printf("Error: CMD0 response error\r\n");
	}
	sd_resetm2cmd(&m2_cmd_info);

	/*CMD8*/
	sd_spi_m2_cmd(SD_CMD8_INDEX, SD_CMD8_SPI_ARG_INIT, SD_CMD8_SPI_RESPTYPE, &m2_cmd_info);
	if(((m2_cmd_info.resp[0] & 0x04000000) > 26) == 0x1){		//illegal command
		printf("SD 1.0 init->\n");

		info->sd_ver = (SD_VER_11 << 4) | SD_STD_CARD;
		sd_resetm2cmd(&m2_cmd_info);

		/*CMD58*/
		sd_spi_m2_cmd(SD_CMD58_INDEX, SD_CMD58_SPI_ARG_INIT, SD_CMD58_SPI_RESPTYPE, &m2_cmd_info);

		OCRValue = (m2_cmd_info.resp[0] & 0x00ffffffff) << 8;
		OCRValue = (m2_cmd_info.resp[1] & 0xff) | OCRValue;
		//printf("OCRValue = 0x%x\n",OCRValue);
		sd_resetm2cmd(&m2_cmd_info);

		/*check 3.3v*/
		if(((OCRValue & 0x00100000) >> 20) == 1){
			//printf("Support  votage	 3.2~3.3\n");
		}else{
			printf("Error: Not support votage 3.2~3.3\n");
			return SD_RTN_ERR_RSP_CMD58;
		}

		do{
			/*CMD55*/	
			sd_spi_m2_cmd(SD_CMD55_INDEX, SD_CMD55_SPI_ARG_INIT, SD_CMD55_SPI_RESPTYPE, &m2_cmd_info);
			sd_resetm2cmd(&m2_cmd_info);

			/*ACMD41*/
			sd_spi_m2_cmd(SD_ACMD41_INDEX, SD_ACMD41_SPI_ARG_INIT, SD_ACMD41_SPI_RESPTYPE, &m2_cmd_info);
		} while((m2_cmd_info.resp[0] & 0x1) == 0x1);
		sd_resetm2cmd(&m2_cmd_info);
	}else{
		printf("SD 2.0 init->\r\n");

		info->sd_ver = (info->sd_ver & 0x0F) | (SD_VER_20 << 4);

		//check response
		if(m2_cmd_info.resp[0] != 0x01000001){
			printf("Error: CMD8 response error\n");
		}

		//check pattern
		if(m2_cmd_info.resp[1] != (SD_CMD8_SPI_ARG_INIT & 0xff)){
			printf("Error: CMD8 check pattern error\n");
			printf("send pattern: %x, readback pattern: %x\n", (SD_CMD8_SPI_ARG_INIT & 0xff), m2_cmd_info.resp[1]);
		}

		sd_resetm2cmd(&m2_cmd_info);

		do{
			/*CMD55*/	
			sd_spi_m2_cmd(SD_CMD55_INDEX, SD_CMD55_SPI_ARG_INIT, SD_CMD55_SPI_RESPTYPE, &m2_cmd_info);
			sd_resetm2cmd(&m2_cmd_info);

			/*ACMD41*/
			sd_spi_m2_cmd(SD_ACMD41_INDEX, 0x40000000, SD_ACMD41_SPI_RESPTYPE, &m2_cmd_info);
		} while((m2_cmd_info.resp[0] & 0x1) == 0x1);
		sd_resetm2cmd(&m2_cmd_info);

		/*CMD58*/
		sd_spi_m2_cmd(SD_CMD58_INDEX, SD_CMD58_SPI_ARG_INIT, SD_CMD58_SPI_RESPTYPE, &m2_cmd_info);

		OCRValue = (m2_cmd_info.resp[0] & 0x00ffffffff) << 8;
		OCRValue = (m2_cmd_info.resp[1] & 0xff) | OCRValue;
		//printf("OCRValue = 0x%x\n",OCRValue);
		sd_resetm2cmd(&m2_cmd_info);

		/*check ccs*/
		if(((OCRValue & 0x40000000) >> 30) == 1){
			info->sd_ver = (info->sd_ver & 0xF0) | SD_HIGH_CARD;	
		}else{
			info->sd_ver = (info->sd_ver & 0xF0) | SD_STD_CARD;
		}		

		/*check 3.3v*/
		if(((OCRValue & 0x00100000) >> 20) == 1){
			//printf("Support votage 3.2~3.3\n");
		}else{
			printf("Error: Not support votage 3.2~3.3\n");
			return SD_RTN_ERR_RSP_CMD58;
		}
	}

	/*set clock to max*/
	sdSetMsSpeed(0x0);

	return SD_RTN_PASS;
}
