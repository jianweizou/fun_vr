/**
 * @file
 * this is sdio driver file
 * sdio.c
 * @author CJ
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdint.h>

#include <sdv2.h>
#include <sdv2_sd.h>
#include "sdv2_sdio.h"
#include "snc_types.h"
#include <stdio.h>

extern void wwd_thread_notify_irq( void );

static uint32_t cis_pointer = 0x0;
xSemaphoreHandle xSem_SDIOMutex;

#ifndef ENABLE
	#define	ENABLE		1
#endif

/**
* @brief Initial SD parameter.
* @param mode 1:SPI mode. 2:SD mode.
* @return SD initial is success or fail.
*/
int sdio_init(uint8_t mode)
{
//	printf("sd_init : start\r\n");
	
	sdReset();
	sdSetMsMode(mode);

	/*set related register*/
	sdExtraENSwitch(ENABLE);
	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);
	sdSetLba(0x3ff);			//auto
	sdSetTimeCount(TIME_CNT);
	sdioENSwitch(ENABLE);

	if(mode == MS_SD_MODE)
	{
		//SD mode
		sdSetMsSpeed(SD_CLK_IDENT);
	}
	else
	{
		//SPI mode
		sdSetMsSpeed(0xff);	
	}

	sd_card_detect();
	xSem_SDIOMutex = xSemaphoreCreateMutex();		
	
	
	return SD_RTN_PASS;
}


//------------------------------------------------------------------------------
snx_sdio_block_size_t find_optimal_block_size(uint32_t data_size)
{
    if ( data_size > (uint32_t) 256 )
        return SNX_SDIO_512B_BLOCK;
    if ( data_size > (uint32_t) 128 )
        return SNX_SDIO_256B_BLOCK;
    if ( data_size > (uint32_t) 64 )
        return SNX_SDIO_128B_BLOCK;
    if ( data_size > (uint32_t) 32 )
        return SNX_SDIO_64B_BLOCK;
    if ( data_size > (uint32_t) 16 )
        return SNX_SDIO_32B_BLOCK;
    if ( data_size > (uint32_t) 8 )
        return SNX_SDIO_16B_BLOCK;
    if ( data_size > (uint32_t) 4 )
        return SNX_SDIO_8B_BLOCK;
    if ( data_size > (uint32_t) 2 )
        return SNX_SDIO_4B_BLOCK;

    return SNX_SDIO_4B_BLOCK;
}


/**
* @brief Enable SDIO function.
* @param isEnable 0: disable, 1:enable.
*/
void sd_sdio_enable(uint32_t isEnable)
{
	sdioENSwitch(isEnable);
}

/**
* @brief Reset SDIO device.
*/
void sd_sdio_reset(void)
{
	struct sd_m2_command m2_cmd_info;
	struct sdio_command sdio_cmd_info;

	// Command State
	/*CMD52*/ // Card Reset
	sdio_cmd_info.function = 0x0;
	sdio_cmd_info.addr = 0x6;
	sdio_cmd_info.value = 0x08;
	sdio_cmd_info.write = 0x1;
	sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
	sd_resetm2cmd(&m2_cmd_info);
}

/**
* @brief SDIO device function select.
* @param fun_sel Set function to SDIO device.
* @return Success or fail.
*/
int sd_sdio_fn_select(uint32_t fun_sel)
{
	struct sd_m2_command m2_cmd_info;
	struct sdio_command sdio_cmd_info;

	sdio_cmd_info.function = 0x0;
	sdio_cmd_info.addr = 0xd;
	sdio_cmd_info.value = fun_sel;
	sdio_cmd_info.write = 0x1;
	sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
	sd_resetm2cmd(&m2_cmd_info);	

	// Check result 
	sdio_cmd_info.write = 0x0;
 	sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
	if(sdio_cmd_info.value == fun_sel){
 		printf("Function Select Pass\r\r\n");
		return SDIO_RTN_PASS;
 	}else{
	 	printf("Select Function: resp 5-2 (4 bytes) = 0x%x\r\r\n", m2_cmd_info.resp[0]);
		printf("Select Function: resp 1 (1 bytes) = 0x%x\r\r\n", m2_cmd_info.resp[1]);
		return SDIO_RTN_ERR_FN;
	}
}

/**
* @brief Check Card Status error bit from SD R5 Response.
* @param state SDIO current state.
* @param cmd_info structure pointer of sd_m2_command.
* @return Message of error flag.
*/
int sd_sdio_r5_check(uint8_t state, struct sd_m2_command *cmd_info)
{
	if(state != ((cmd_info->resp[0] >> 4) & 0x00000003)){
		printf("Error: SDIO CMD%d resp state error %x\r\r\n", cmd_info->index, cmd_info->resp[0]);
		return SDIO_RTN_ERR_STATE;
	}
		
	if((cmd_info->resp[0] & 0x000000CF)){
		printf("Error: SDIO CMD%d resp error %x\r\r\n", cmd_info->index, cmd_info->resp[0]);
		return SDIO_RTN_ERR_FLAG;
	}else{
//		printf("SD cmd%d  response success\r\r\n",(cmd_info->resp[0] >> 24 )& 0xff);
		return SDIO_RTN_PASS;
	}
}

/**
* @brief Read SDIO device card common control register.
* @return Success or fail.
*/
int sd_sdio_cccr_read(void)
{
	uint32_t Data = 0, Resp = 0;
	struct sd_m2_command m2_cmd_info;	
	struct sdio_command sdio_cmd_info;	
	int err_num = 0;
	uint8_t sdio_cccr[0x17];

	//init struct sdio_command
	sdio_cmd_info.write = 0x0;
	sdio_cmd_info.function = 0x0;
	
  	// Read Card Common Control Register (CCCR)
	for(sdio_cmd_info.addr = 0x00; sdio_cmd_info.addr < 0x17; (sdio_cmd_info.addr)++){
		sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);

		Data = (m2_cmd_info.resp[1] & 0x000000ff);
		Resp = (m2_cmd_info.resp[0] & 0x000000ff);
		sd_resetm2cmd (&m2_cmd_info);	

		if(Resp != 0x10){
			printf("addr 0x%x =0x%x Resp. fail Resp = %x\r\r\n", sdio_cmd_info.addr, Data, Resp);		
			err_num++;
		}else{
			sdio_cccr[sdio_cmd_info.addr] = Data;
			//printf("0x%x \t= %x\t0x%02x\r\r\n",sdio_cmd_info.addr, sdio_cmd_info.function,sdio_cccr[sdio_cmd_info.addr]);		
		}
	 	Data = 0x0;
	}
	
	cis_pointer = (sdio_cccr[0x9] & 0xff ) | ((sdio_cccr[0xa] << 8) & 0xff00) | ((sdio_cccr[0xb] << 16) & 0xff0000);
									
	printf("CIS == 0x%x \r\r\n", cis_pointer);		

	if(err_num){
		return SDIO_RTN_ERR_FLAG;
	}else{
		return SDIO_RTN_PASS;
	}
}

#if 0
static __inline uint32_t little_endian_transfer(uint8_t Hi, uint8_t Low)
{
	uint32_t value;
	value =	(Low & 0xff) | (((uint32_t)Hi << 8) & 0xff00);
	return value;
}
#endif

/**
* @brief Read SDIO device cis.
* @return Success or fail.
*/
int sd_sdio_cis_read(void)
{
	uint32_t Data = 0, Resp = 0;
	struct sd_m2_command m2_cmd_info;	
	struct sdio_command  sdio_cmd_info;	
	uint32_t i, j;
	tuple_t tpl[6];
	uint8_t code, size;
	uint32_t offset;
	int err_num=0;

	// Read Card Common Control Register (CCCR)
	if(cis_pointer == 0x0){
		printf("fail CIS pointer  = 0x%x\r\r\n", cis_pointer);
		return SDIO_RTN_ERR_CIS_POINTER;
	}

	sdio_cmd_info.addr = cis_pointer;
	sdio_cmd_info.function = 0x0;
	sdio_cmd_info.write = 0x0;

	printf("=================CIS================\r\r\n");
	for(i = 0; i < 5; i++){
		// Read Tuple Code
		sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
		Data = (m2_cmd_info.resp[1] & 0x000000ff);
		Resp = (m2_cmd_info.resp[0] & 0x000000ff);
		sd_resetm2cmd (&m2_cmd_info);	

		if(Resp != 0x10){
			printf("addr 0x%x = 0x%x Resp. fail Resp = %x\r\r\n", sdio_cmd_info.addr, Data, Resp);		
			err_num++;
		}else{
			//printf("0x%x = \t0x%02x\r\r\n",sdio_cmd_info.addr,Data);		
		}
		
		code = Data;

		if(code == 0xff){
			break;
		}
 		Data = 0x0;
		(sdio_cmd_info.addr)++;
		
		// Read Tuple Body Size
		sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
		Data = (m2_cmd_info.resp[1] & 0x000000ff);
		Resp = (m2_cmd_info.resp[0] & 0x000000ff);
		sd_resetm2cmd (&m2_cmd_info);	

		if(Resp != 0x10){
			printf("addr 0x%x = 0x%x Resp. fail Resp = %x\r\r\n", sdio_cmd_info.addr, Data,Resp);		
			err_num++;
		}else{
			//printf("0x%x = \t0x%02x\r\r\n",sdio_cmd_info.addr,Data);
		}

		size = Data;
		tpl[i].code = code;
		tpl[i].body_size = size;
		//printf("  Tuple Body Size = 0x%02x\r\r\n",tpl[i].body_size);		
  		(sdio_cmd_info.addr)++;

 		Data = 0x0;
		offset = sdio_cmd_info.addr + tpl[i].body_size;

		// Read Tuple Body Size
		for(j = 0; sdio_cmd_info.addr < offset; sdio_cmd_info.addr++, j++){
			sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
			Data = (m2_cmd_info.resp[1] & 0x000000ff);
			Resp = (m2_cmd_info.resp[0] & 0x000000ff);
			sd_resetm2cmd (&m2_cmd_info);	

			if(Resp != 0x10){
				printf("addr 0x%x =0x%x Resp. fail Resp = %x\r\r\n", sdio_cmd_info.addr, Data,Resp);		
				err_num++;
			}else{
				//printf("Body 0x%x = \t0x%02x\r\r\n",sdio_cmd_info.addr,Data);		
				tpl[i].body[j] = Data;
			}
		 	Data = 0x0;
		}
		
		/*
		if(tpl[i].code == CISTPL_VERS_1){
			for(j = 0; j < tpl[i].body_size; j++){
				printf("Body 0x%x\r\r\n", tpl[i].body[j]);
			}
		}

		if(tpl[i].code == CISTPL_MANFID){
			uint32_t value;
			value = little_endian_transfer(tpl[i].body[1], tpl[i].body[0]);
			printf("TPLMID_MANF = 0x%04x\r\r\n", value);
			value = little_endian_transfer(tpl[i].body[3], tpl[i].body[2]);
			printf("TPLMID_CARD = 0x%04x\r\r\n", value);
		}

		if(tpl[i].code == CISTPL_FUNCID){
			printf("TPLFID_FUNCTION = 0x%04x\r\r\n", tpl[i].body[0]);
			printf("TPLFID_SYSINIT = 0x%04x\r\r\n", tpl[i].body[1]);
		}

		if(tpl[i].code == CISTPL_FUNCE){
			uint32_t value;
			if(tpl[i].body[0] == 0x0){// TPLFE_TYPE Extended Data 00h
				value = little_endian_transfer(tpl[i].body[2], tpl[i].body[1]);
				printf("TPLFE_FN0_BLK_SIZE = 0x%04x\r\r\n", value);
				printf("TPLFE_MAX_TRAN_SPEED = 0x%04x\r\r\n", tpl[i].body[3]);	
			}
			
			for( j = 4; j < (tpl[i].body_size - 4); j++){
				printf("TPLFE_TC = 0x%04x\r\r\n", tpl[i].body[j++]);
				printf("TPLFE_CP = 0x%04x\r\r\n", tpl[i].body[j]);
			}
		}
		*/
	}
	printf("=====================================\r\r\n");
	
	if(err_num){
		return SDIO_RTN_ERR_FLAG;
	}else{
		return SDIO_RTN_PASS;
	}
}

/**
* @brief Identify SDIO device.
* @return Success or fail.
*/
int sd_sdio_sd_identify(void)
{
	uint32_t OCRValue;
	uint32_t RCAValue;
	uint32_t NF, MP, IOREADY;
	uint32_t arg_temp;
	struct sd_m2_command m2_cmd_info;	
	//struct sdio_command sdio_cmd_info;	

	/*fast step*/
	// Card identification mode
	/*CMD5*/
	
	sdSetMsSpeed(SD_CLK_IDENT);
	
	sd_tx_cmd(SD_CMD5_INDEX, 0x0, SD_CMD5_SD_RESPTYPE, &m2_cmd_info);
	//printf("CMD5: resp 5-2 (4 bytes) = 0x%x\r\r\n", m2_cmd_info.resp[0]);
	//printf("CMD5: resp 1 (1bytes) = 0x%x\r\r\n", m2_cmd_info.resp[1]);
	OCRValue = (m2_cmd_info.resp[0] & 0x0000ffff) << 8;
	OCRValue = (m2_cmd_info.resp[1] & 0xff) | OCRValue;

	NF = (m2_cmd_info.resp[0] & 0x00700000) >> 20;
	MP = (m2_cmd_info.resp[0] & 0x00080000) >> 19;
	IOREADY = (m2_cmd_info.resp[0] & 0x00800000) >> 23;
	printf("Number Functions = 0x%x\r\n", NF);
	printf("Memory Present = 0x%x\r\n", MP);
	printf("OCRValue = 0x%x\r\n", OCRValue);
	printf("IOREADY = 0x%x\r\n", IOREADY);

	sd_resetm2cmd(&m2_cmd_info);

	if(((OCRValue & 0x00100000) >> 20) == 1){     
		printf("OCR bit20 = 1: 3.2 ~ 3.3\r\n");
	}

	if(NF > 0x0){     
		sd_tx_cmd(SD_CMD5_INDEX, 0x00ff0000, SD_CMD5_SD_RESPTYPE, &m2_cmd_info);
		//printf("CMD5: resp 5-2 (4 bytes) = 0x%x\r\n", m2_cmd_info.resp[0]);
		//printf("CMD5: resp 1 (1 bytes) = 0x%x\r\n", m2_cmd_info.resp[1]);
		OCRValue = (m2_cmd_info.resp[0] & 0x0000ffff) << 8;
		OCRValue = (m2_cmd_info.resp[1] & 0xff) | OCRValue;
		NF = (m2_cmd_info.resp[0] & 0x00700000) >> 20;
		MP = (m2_cmd_info.resp[0] & 0x00080000) >> 19;
		IOREADY = (m2_cmd_info.resp[0] & 0x00800000) >> 23;
		
		printf("Memory Present = 0x%x\r\n", MP);	
		printf("Number Functions = 0x%x\r\n", NF);	
		printf("OCRValue = 0x%x\r\n", OCRValue);
		printf("IOREADY = 0x%x\r\n", IOREADY);
	}

	// Initialization State
	/*CMD3*/
	sd_tx_cmd(SD_CMD3_INDEX, SD_CMD3_SD_ARG_INIT, SD_CMD3_SD_RESPTYPE, &m2_cmd_info);
	//printf("CMD3: resp 5-2 (4 bytes) = 0x%x\r\n", m2_cmd_info.resp[0]);
	//printf("CMD3: resp 1 (1bytes) = 0x%x\r\n", m2_cmd_info.resp[1]);
	RCAValue = (m2_cmd_info.resp[0] & 0x00ffff00) >> 8;
	printf("New RCA = 0x%x\r\n", RCAValue);
	sd_resetm2cmd(&m2_cmd_info);

	// Standby State
	/*CMD7*/
	arg_temp = 0x0;
	arg_temp = RCAValue << 16;
	sd_tx_cmd(SD_CMD7_INDEX, arg_temp, SD_CMD7_SD_RESPTYPE, &m2_cmd_info);
	printf("CMD7: resp 5-2 (4 bytes) = 0x%x\r\n", m2_cmd_info.resp[0]);
	printf("CMD7: resp 1 (1 bytes) = 0x%x\r\n", m2_cmd_info.resp[1]);
	
	if(sd_r1_check_all(&m2_cmd_info, ENABLE)){
		sd_resetm2cmd(&m2_cmd_info);
	}else{
		return SDIO_RTN_ERR_RSP_CMD7;
	}

	sd_resetm2cmd(&m2_cmd_info);
	sdSetMsSpeed(0x1);

	return SDIO_RTN_PASS;
}

/**
* @brief Set command 52 to SDIO device.
* @param sdio_cmd_info structure pointer of sdio_command.
* @param cmd_info structure pointer of sd_m2_command.
*/
int sd_sdio_cmd52(struct sdio_command *sdio_cmd_info, struct sd_m2_command *cmd_info)
{
	uint32_t arg_temp;
	int Status = SDIO_RTN_PASS;
	
	xSemaphoreTake(xSem_SDIOMutex, portMAX_DELAY);	
	
	arg_temp = 0x0;
	// R/W flag   
	arg_temp |= sdio_cmd_info->write << 31;	
	// RAW Flag
	arg_temp |= (uint32_t)(0 << 27);	
	// Function Number Function 0 = CIA
	arg_temp |= sdio_cmd_info->function << 28;	
	// address
	arg_temp |= sdio_cmd_info->addr << 9;	
	// data, read mode not use, set = 0
	if(!sdio_cmd_info->write){
		sdio_cmd_info->value = 0x0;
	}
	arg_temp |= sdio_cmd_info->value << 0;	

	sd_tx_cmd(SD_CMD52_INDEX, arg_temp, SD_CMD52_SD_RESPTYPE, cmd_info);

	//if cmd52 reset, don't check R5 response
	if(sdio_cmd_info->write){
		if(sdio_cmd_info->addr != 0x6 && sdio_cmd_info->value != 0x8 && sdio_cmd_info->function != 0){	// reset command
	  		sd_sdio_r5_check(STATE_CMD, cmd_info);
		}
	}else{
		if(sd_sdio_r5_check(STATE_CMD, cmd_info) == SDIO_RTN_PASS){
			sdio_cmd_info->value = (cmd_info->resp[1] & 0x000000ff);
		}
	}
	
	xSemaphoreGive(xSem_SDIOMutex);
	return Status;
}

/**
* @brief Set command 53 to SDIO device.
* @param sdio_cmd_info structure pointer of sdio_command.
* @param cmd_info structure pointer of sd_m2_command.
*/

int sd_sdio_cmd53(struct sdio_command *sdio_cmd_info, struct sd_m2_command *cmd_info)
{
	uint32_t ulBlkNum = 0;
	int ubRegWrieStatus = SDIO_RTN_PASS;
	int status;

	xSemaphoreTake(xSem_SDIOMutex, portMAX_DELAY);

	sdMsMDmaENSwitch(DISABLE);
	sdMsDmaENSwitch(DISABLE);
	sdClearMsRdyFlag(0x1);
	sdSetTimeCount(0x08000000);

	sdioENSwitch(0x1);
	sdioFuncSet(sdio_cmd_info->function);
	sdioOPCodeSet(0x1);
	sdioBLKmodeSet(sdio_cmd_info->block);
	sdioRegAddrSet(sdio_cmd_info->addr);


	if(sdio_cmd_info->block == SNX_SDIO_BLOCK_MODE)			//Block mode
	{
	
		sdSetBlkSize(sdio_cmd_info->blk_size);
		sdSetDmaAddr(sdio_cmd_info->dma_addr);
		sdMsMDmaENSwitch(ENABLE);
		
		ulBlkNum =  (sdio_cmd_info->data_len / sdio_cmd_info->blk_size) & 0x1ff;
		if((ulBlkNum * sdio_cmd_info->blk_size) < sdio_cmd_info->data_len)
			++ulBlkNum;
		sdDmaBlock(ulBlkNum);

		sdMsDmaRWSwitch(sdio_cmd_info->write);
		sdMsDmaENSwitch(ENABLE);
	}
	else													//Byte mode
	{

		sdio_cmd_info->data_len = find_optimal_block_size(sdio_cmd_info->data_len);
		if(sdio_cmd_info->data_len > 512)
			printf("SDIO_CMD53: data_size > 512 for byte mode");
		sdSetBlkSize(sdio_cmd_info->data_len);
		sdSetDmaAddr(sdio_cmd_info->dma_addr);
		sdMsMDmaENSwitch(ENABLE);
		sdDmaBlock(1); //SDHC_SF->MS_DMA_BLOCK2 = 0;
		sdMsDmaRWSwitch(sdio_cmd_info->write);	
		sdMsDmaENSwitch(ENABLE);
	}
	
	while(!sdCheckMsRdy());
	sdMsMDmaENSwitch(DISABLE);
	sdMsDmaENSwitch(DISABLE);
	sdClearMsRdyFlag(0x1);
	
	 status = sdCheckMsMDmaOkOnTime();
	
	if(status == MDMA_OK_ON_TIME){
			ubRegWrieStatus = SDIO_RTN_PASS;
		 }else{
			if(status == MDMA_OK_TIME_OUT){
				ubRegWrieStatus = SD_RTN_ERR_MDMA_TIMEOUT;
				printf("Error: M DMA complete with Time out flag\r\n");
			}else if(status == MDMA_FAIL_TIME_OUT){
				ubRegWrieStatus = SD_RTN_ERR_MDMA_TIMEOUT;
				printf("Error: M DMA not complete with Time out flag\r\n");
			}else{
				if(sdio_cmd_info->block == SNX_SDIO_BLOCK_MODE)			//Block mode
				{
					if(sdReadSUDmaBlock()!= ulBlkNum)
						printf("(B:%d != SU:%d)", ulBlkNum, sdReadSUDmaBlock());
					
					if(sdCheckWriteErr()){
						printf("(CRC_W Error)");
						sdClearCRCWErrFlag(0x01);
					}
				}
				ubRegWrieStatus = SD_RTN_ERR_MDMA_FAIL;
				printf("Error: M DMA not complete\r\n");
			}
  }

	xSemaphoreGive(xSem_SDIOMutex);

	return ubRegWrieStatus;
}

//------------------------------------------------------------------------------
#if CONFIG_MODULE_WIFI_BCM43362
void SDIO_IRQHandler(int irq)
{
	if(sdCheckIntrFlag())
	{
		sdClearIntrFlag(0x01);
		wwd_thread_notify_irq();
	}
}
#endif

//------------------------------------------------------------------------------
void sdio_enable_irq(void)
{
	sdioIntrENSwitch(ENABLE);
	NVIC_EnableIRQ(SDIO_IRQn);
}
