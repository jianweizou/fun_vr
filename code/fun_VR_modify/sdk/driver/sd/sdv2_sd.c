/**
 * @file
 * this is sdv2 sd driver file
 * sdv2_sd.c
 * @author CJ
 */

#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
//#include <nonstdlib.h>
//#include <bsp.h>
#include "sdv2.h"
#include "sdv2_sd.h"
extern void flush_dcache_area(unsigned int start_addr, unsigned int size);

#define MSRDY_CNT	0x10000000

#define	sd_sd_resp3			sd_sd_resp1
#define sd_sd_resp4			sd_sd_resp1
#define sd_sd_resp5			sd_sd_resp1
#define sd_sd_resp6			sd_sd_resp1
#define sd_sd_resp7			sd_sd_resp1

#define	SRAM_DATA_SIZE	64

extern volatile int sd_card_remove;
extern volatile int sd_card_error;
extern void sd_dma_read(uint32_t sd_start_addr, uint32_t ddr_start_addr, uint32_t blk_len, uint32_t mode);

uint8_t sram_data[SRAM_DATA_SIZE];

// =========================================================================
// SD R1 Response check 
// =========================================================================
static __inline uint32_t sd_r1_error(struct sd_m2_command *cmd_info);
static __inline uint32_t sd_r1_app_cmd(struct sd_m2_command *cmd_info);
static __inline uint32_t sd_r1_ready_data(struct sd_m2_command *cmd_info);
static __inline uint32_t sd_r1_status(struct sd_m2_command *cmd_info );
static __inline uint32_t sd_r3_busy(uint32_t *OCRValue);
static __inline uint32_t sd_r3_32_33(uint32_t *OCRValue);

// =========================================================================
// SD Ready Detect 
// =========================================================================
uint32_t sd_ready_detect(void)
{
	unsigned int cnt = MSRDY_CNT;

	while(cnt--)
	{
		if(sdCheckMsRdy())
			return SD_MS_RDY;

		if(sd_card_remove){
			return SD_MS_NOT_RDY;
		}
	}

	sd_card_error = 1;
	return SD_MS_NOT_RDY;
}

// =========================================================================
// SD R1 Response check Card Status error bit 
// =========================================================================
static __inline uint32_t sd_r1_error(struct sd_m2_command *cmd_info)
{
	if((cmd_info->resp[0] & 0x00f9f980) || (cmd_info->resp[1] & 0x08)){
		printf("Error : SD cmd%d response error\r\n", (cmd_info->resp[0] >> 24 ) & 0xff);
		return FALSE;
	}else{
//		printf("SD cmd%d  response success\r\n",(cmd_info->resp[0] >> 24 )& 0xff);
		return TRUE;
	}
}

// =========================================================================
// SD R1 Response check Card Status app cmd bit 
// =========================================================================
static __inline uint32_t sd_r1_app_cmd(struct sd_m2_command *cmd_info)
{
	if(((cmd_info->resp[1] >> 5 ) & 0x01)){
//		printf("SD app cmd enable\r\n");
		return TRUE;
	}else{
		printf("Error : SD app cmd disable\r\n");
		return FALSE;
	}
}

// =========================================================================
// SD R1 Response check Card Status Ready for Data bit 
// =========================================================================
static __inline uint32_t sd_r1_ready_data(struct sd_m2_command *cmd_info)
{
	if((cmd_info->resp[0] & 0x01)){
		return TRUE;
	}else{
		printf("Error : SD cmd check reday for data fail\r\n");
		return FALSE;
	}
}

// =========================================================================
// SD R1 Response check Card Status Current State 
// =========================================================================
static __inline uint32_t sd_r1_status(struct sd_m2_command *cmd_info)
{
#if 0 // show status
		switch((cmd_info->resp[0] >> 1 )& 0x0f)
		{
			case SD_STATE_IDLE:
				printf("SD status == idle\n");
			break;
			case SD_STATE_READY:
				printf("SD status == ready\n");
			break;
			case SD_STATE_IDENT:
				printf("SD status == ident\n");
			break;
			case SD_STATE_STBY:
				printf("SD status == stby\n");
			break;
			case SD_STATE_TRAN:
				printf("SD status == tran\n");
			break;
			case SD_STATE_DATA:
				printf("SD status == data\n");
			break;
			case SD_STATE_RCV:
				printf("SD status == rcv\n");
			break;
			case SD_STATE_PRG:
				printf("SD status == prg\n");
			break;
			case SD_STATE_DIS:
				printf("SD status == dis\n");
			break;
			default:
			printf("SD status == 0x%x\n",(cmd_info->resp[0] >> 1 )& 0x0f);

			break;
		}
#endif
		
		if(cmd_info->next_state == ((cmd_info->resp[0] >> 1) & 0x0f)){
			//printf("SD cmd%d check status pass\n", (cmd_info->resp[0] >> 24 )& 0xff);
			//printf("SD status == 0x%x\n", (cmd_info->resp[0] >> 1 ) & 0x0f);
			return TRUE;
		}else{
			printf("Error : SD cmd %d check status fail\n", (cmd_info->resp[0] >> 1) & 0x0f);
			return FALSE;
		}
}

// =========================================================================
// SD R1 Response check all
// =========================================================================
uint32_t sd_r1_check_all(struct sd_m2_command *cmd_info, uint32_t SDIO_SWITCH)
{
	// Check index 
	if(cmd_info->index != ((cmd_info->resp[0] >> 24 ) & 0xff)){
		printf("Cmd index fail: CMD %d != 0x%x\r\n", cmd_info->index, (cmd_info->resp[0] >> 24 ) & 0xff);		
		return FALSE;
	}

	if(!SDIO_SWITCH){ // For SD card.
		// Check app cmd bit
		if(cmd_info->app_cmd){
			if((sd_r1_app_cmd(cmd_info) == FALSE)){
				return FALSE;
			}
		}

		if(sd_r1_ready_data(cmd_info) & sd_r1_status(cmd_info) & sd_r1_error(cmd_info)){
			return TRUE;
		}else{
			return FALSE;
		}
	}else{			// For SDIO.
		if(((cmd_info->resp[0] & 0xffff0000) >> 16) & 0x80c8){
			printf("Error : SDIO cmd%d response error\n", ((cmd_info->resp[0] & 0xffff0000) >> 16));
			return FALSE;
		}

		if(((cmd_info->resp[0] >> 1) & 0x0f) == 15){// bit[9] ~ bit[12] are all 1.
			return TRUE;
		}else{
			printf("Error : SDIO cmd%d response error\n", ((cmd_info->resp[0] >> 1) & 0x0f));
			return FALSE;
		}
	}
}

// =========================================================================
// SD R3 Response check Card capacity status bit
// =========================================================================
static __inline uint32_t sd_r3_ccs(uint32_t *OCRValue)
{
	return ((*OCRValue & OCR_CCS_MASK) >> OCR_CCS_BIT);
}

// =========================================================================
// SD R3 Response check Card power up status bit (busy)
// =========================================================================
static __inline uint32_t sd_r3_busy(uint32_t *OCRValue)
{
	//1 : Power up finished (idle)
	return ((*OCRValue & OCR_BUSY_MASK) >> OCR_BUSY_BIT);
}

// =========================================================================
// SD R3 Response check voltage 3.2 ~ 3.3 V
// =========================================================================
static __inline uint32_t sd_r3_32_33(uint32_t *OCRValue)
{
	return ((*OCRValue & OCR_32_33_MASK) >> OCR_32_33_BIT);
}

// =========================================================================
// SD R3 Response check voltage 3.2 ~ 3.3 V
// =========================================================================
void sd_m2setcmd(uint32_t index, uint32_t arg, int resp_type, struct sd_m2_command *cmd_info)
{	
	cmd_info->index = index;
	cmd_info->arg = arg;
	cmd_info->resp_type = resp_type;
	
	if((index == SD_CMD55_INDEX) || (index == SD_ACMD6_INDEX) || (index == SD_ACMD13_INDEX)
		 || (index == SD_ACMD41_INDEX) || (index == SD_ACMD51_INDEX)){
		cmd_info->app_cmd = 1;
	}else{
		cmd_info->app_cmd = 0;
	}
}

//===============================================================
void sd_resetm2cmd(struct sd_m2_command *cmd_info)
{
	cmd_info->index = 0x0;
	cmd_info->arg = 0x0;
	cmd_info->app_cmd = 0;
	cmd_info->next_state = 0;

	cmd_info->resp_type = 0;
	cmd_info->resp[0] = 0x0;
	cmd_info->resp[1] = 0x0;
	cmd_info->resp[2] = 0x0;
	cmd_info->resp[3] = 0x0;
	cmd_info->resp[4] = 0x0;
}

//===============================================================
void sd_sd_m2_WcmdRresp(struct sd_m2_command *cmd_info)
{
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteSpiIndex(cmd_info->index);
	sdWriteSpiCmd(cmd_info->arg);

 	sdSetSpiCmdTri(ENABLE);
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\r\n");
		return; //delezue add
	}

	if(cmd_info->index == SD_CMD7_INDEX){
		sdSetSpiBusyTri(ENABLE);
		if(sd_ready_detect()){
			printf("sd: sdCheckMsRdy fail\n");
			return; //delezue add
		}
	}

	switch(cmd_info->resp_type){
		case SD_RESP_NONE:
			break;
		case SD_RESP_SD_R1:
			sd_sd_resp1(cmd_info);
			break;
		case SD_RESP_SD_R2:
			sd_sd_resp2(cmd_info);
			break;
		case SD_RESP_SD_R3:
			sd_sd_resp3(cmd_info);
			break;
		case SD_RESP_SD_R4:
			sd_sd_resp4(cmd_info);
			break;
		case SD_RESP_SD_R5:
			sd_sd_resp5(cmd_info);
			break;
		case SD_RESP_SD_R6:
			sd_sd_resp6(cmd_info);
			break;
		case SD_RESP_SD_R7:
			sd_sd_resp7(cmd_info);
			break;
	}
}

//=======================================================================
void sd_tx_cmd(uint32_t index, uint32_t arg, int resp_type, struct sd_m2_command *cmd_info)
{
	sd_m2setcmd(index, arg, resp_type, cmd_info);
	sd_sd_m2_WcmdRresp(cmd_info);

	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1); 
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return; //delezue add
	}
}

//=======================================================================
int sd_sd_write_cmd(uint32_t start_addr, uint32_t dma_size, struct sd_m2_command *cmd_info)
{
	uint32_t block_size = 0x0;

	block_size = dma_size;

	//CMD16__set block size
	sd_tx_cmd(SD_CMD16_INDEX, block_size, SD_CMD16_SD_RESPTYPE, cmd_info);
	if((cmd_info->resp[0] != 0x10000009) || (cmd_info->resp[1] != 0x0)){
		if(cmd_info->resp[0] == 0x10004009){
			printf("cmd is illegal before cmd16\n");
			return SD_RTN_ERR_RSP_CMD16_ILL;
		}else{
			printf("Error : CMD16 response error\n");
			return SD_RTN_ERR_RSP_CMD16;
		}
	}
	sd_resetm2cmd(cmd_info);

	//CMD24__set sd card start address
	sd_m2setcmd(SD_CMD24_INDEX, start_addr, SD_CMD24_SD_RESPTYPE, cmd_info);
	sd_sd_m2_WcmdRresp(cmd_info);
	if((cmd_info->resp[0] != 0x18000009) || (cmd_info->resp[1] != 0x0)){
		printf("Error : CMD24 response error\n");
		return SD_RTN_ERR_RSP_CMD24;
	}
	sd_resetm2cmd(cmd_info);

	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);

	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return SD_RTN_ERR_MS_RDY;
	}

	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1); 
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return SD_RTN_ERR_MS_RDY;
	}

	return SD_RTN_PASS;
}

//=======================================================================
void sd_sd_read_cmd(uint32_t start_addr, struct sd_m2_command *cmd_info)
{
	//CMD17
	sd_m2setcmd(SD_CMD17_INDEX, start_addr, SD_CMD17_SD_RESPTYPE, cmd_info);
	sd_sd_m2_WcmdRresp(cmd_info);

	cmd_info->next_state = SD_STATE_TRAN;
	sd_r1_check_all(cmd_info, DISABLE);
	sd_resetm2cmd(cmd_info);
}

//========================================================================
void sd_sd_resp1(struct sd_m2_command *cmd_info)
{
	uint32_t resp1 = 0x0;
	uint32_t resp2 = 0x0;
	resp1 = sdReadSpiIndex();
	resp2 = sdReadSpiCmd();

	cmd_info->resp[0] = (resp1 & 0x000000ff) << 24;
	cmd_info->resp[0] |= (resp2 & 0xffffff00) >> 8;
	cmd_info->resp[1] = (resp2 & 0x000000ff) >> 0;
}

//===============================================================
void sd_sd_resp2(struct sd_m2_command *cmd_info)
{
	uint32_t resp1 = 0x0;
	uint32_t resp2 = 0x0;

	//Get response high --> low
	resp1 = sdReadSpiIndex();
	resp2 = sdReadSpiCmd();
	//Get response 1-5 byte
	cmd_info->resp[0] = (resp1 & 0x000000ff) << 24;
	cmd_info->resp[0] |= (resp2 & 0xffffff00) >> 8;
	cmd_info->resp[1] = (resp2 & 0x000000ff) << 24;

	//Get response 6-10 byte
	sdMsRegRWSwitch(SD_READ_MODE);		//read mode
	sdWriteAutoResponse(0x1);		//write value can be any value
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return; //delezue add
	}

	resp1 = sdReadSpiIndex();
	resp2 = sdReadSpiCmd();

	cmd_info->resp[1] |= (resp1 & 0x000000ff) << 16;
	cmd_info->resp[1] |= (resp2 & 0xffff0000) >> 16;
	cmd_info->resp[2] = (resp2 & 0x0000ffff) << 16;

	//Get Response 11-15
	sdMsRegRWSwitch(SD_READ_MODE);		//read mode
	sdWriteAutoResponse(0x1);		//write value can be any value
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return; //delezue add
	}

	resp1 = sdReadSpiIndex();
	resp2 = sdReadSpiCmd();

	cmd_info->resp[2] |= (resp1 & 0x000000ff) << 8;
	cmd_info->resp[2] |= (resp2 & 0xff000000) >> 24;
	cmd_info->resp[3] = (resp2 & 0x00ffffff) << 8;

	//Get respose 16
	sdWriteCommand(0x1);
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return; //delezue add
	}

	resp1 = sdReadCommand();
	cmd_info->resp[3] = cmd_info->resp[3] | ((resp1 & 0x000000ff) >> 0);

	//Get respose 17
	sdWriteCommand(0x1);
	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return; //delezue add
	}

	resp1 = sdReadCommand();
	cmd_info->resp[4] = resp1;
}

// =========================================================================
// SD Command 13 Send status
// =========================================================================
uint32_t sd_check_status_cmd13(uint32_t RCAValue, struct sd_m2_command *cmd_info)
{
	uint32_t arg_temp;
 
	/*CMD13*/
	arg_temp = RCAValue<<16;
	sd_tx_cmd(SD_CMD13_INDEX,arg_temp, SD_CMD13_SD_RESPTYPE, cmd_info);

	//Check CMD13 != ACMD13 not app cmd
	cmd_info->app_cmd= 0;
	cmd_info->next_state = ((cmd_info->resp[0] >> 1 )& 0x0f);

	if(sd_r1_check_all(cmd_info, DISABLE)){
		return TRUE;
	}else{
		return FALSE;
	}
}

// =========================================================================
// SD APP Command current on trnsfer status
// =========================================================================
uint32_t sd_sd_app_cmd_tran(uint32_t index, uint32_t arg, uint32_t sd_bus_width)
{
	uint32_t arg_temp;
	struct sd_m2_command m2_cmd_info;
	uint32_t i;

	sd_resetm2cmd(&m2_cmd_info);

	// Check Card Status
	/*CMD13*/
	i = 0;
	while(m2_cmd_info.next_state != SD_STATE_TRAN){
		i++;
		if((i > 10) || (sd_check_status_cmd13(arg, &m2_cmd_info) == FALSE)){
			return FALSE;
		}
	}

	arg_temp = arg << 16;

	/*CMD55*/
	sd_tx_cmd(SD_CMD55_INDEX, arg_temp, SD_CMD55_SD_RESPTYPE, &m2_cmd_info);

	m2_cmd_info.next_state = SD_STATE_TRAN;
	// Check CMD55 response
	if(sd_r1_check_all(&m2_cmd_info, DISABLE)){
		sd_resetm2cmd(&m2_cmd_info);
	}else{
		printf("%s:%d CMD55 error\n",__func__,__LINE__);
		return FALSE;
	}

	switch(index)
	{
		case SD_ACMD6_INDEX:
			/*ACMD6*/
			arg_temp = sd_bus_width;
			sd_tx_cmd(index, arg_temp, SD_ACMD6_SD_RESPTYPE, &m2_cmd_info);

			m2_cmd_info.next_state = SD_STATE_TRAN;
			// Check ACMD6 response
			if(sd_r1_check_all(&m2_cmd_info, DISABLE)){
				sd_resetm2cmd(&m2_cmd_info);
			}else{
				return FALSE;
			}

			break;
		case SD_ACMD51_INDEX:
			//set sd_read_data_cmd ==1
			sdMsRegRWSwitch(SD_WRITE_MODE); //write mode
			sdReadDataCmd (ENABLE);
			/*ACMD51*/	
			sd_m2setcmd(SD_ACMD51_INDEX, 0x0, SD_ACMD51_SD_RESPTYPE, &m2_cmd_info);
			sd_sd_m2_WcmdRresp(&m2_cmd_info);

			// ACMD51 auto complete status = SD_STATE_TRAN
			m2_cmd_info.next_state = SD_STATE_TRAN;
			// Check ACMD51 response
			if(sd_r1_check_all(&m2_cmd_info, DISABLE)){
				sd_resetm2cmd(&m2_cmd_info);
			}else{
				return FALSE;
			}

			break;
		case SD_ACMD13_INDEX:
			//set sd_read_data_cmd ==1
			sdMsRegRWSwitch(SD_WRITE_MODE); //write mode
			sdReadDataCmd (ENABLE);
			/*ACMD13*/	
			sd_m2setcmd(SD_ACMD13_INDEX, 0x0, SD_ACMD13_SD_RESPTYPE, &m2_cmd_info);
			sd_sd_m2_WcmdRresp(&m2_cmd_info);

			// ACMD13 auto complete status = SD_STATE_TRAN
			m2_cmd_info.next_state = SD_STATE_TRAN;
			// Check ACMD13 response
			if(sd_r1_check_all(&m2_cmd_info, DISABLE)){
				sd_resetm2cmd(&m2_cmd_info);
			}else{
				return FALSE;
			}

			break;
		default:
			printf("Bad App command.\n");
			break;
	}
	return TRUE;
}

// =========================================================================
// SD APP Command current on trnsfer status
// =========================================================================
uint32_t sd_sd_app_cmd_idle(uint32_t index, uint32_t arg, uint32_t *value)
{
	struct sd_m2_command m2_cmd_info;
 	uint32_t OCRValue;

	/*CMD55*/
	sd_tx_cmd(SD_CMD55_INDEX, SD_CMD55_SD_ARG_INIT, SD_CMD55_SD_RESPTYPE, &m2_cmd_info);

	m2_cmd_info.next_state = SD_STATE_IDLE;
	// Check CMD55 response

	if(sd_r1_check_all(&m2_cmd_info, DISABLE)){
		sd_resetm2cmd(&m2_cmd_info);
	}
	else{
		printf("Err index=%x,%x Unusable Card reTry !!!\r\n", m2_cmd_info.index,m2_cmd_info.resp[0]);
		return FALSE;
	}

	 /*ACMD41*/
	 sd_tx_cmd(SD_ACMD41_INDEX, arg, SD_ACMD41_SD_RESPTYPE, &m2_cmd_info);
	 OCRValue = (m2_cmd_info.resp[0] & 0x00ffffffff) << 8;
	 OCRValue = (m2_cmd_info.resp[1] & 0xff) | OCRValue;
	 sd_resetm2cmd(&m2_cmd_info);

	*value = OCRValue;
	return TRUE;
}

// =========================================================================
// SD card calculate capacity
// =========================================================================
void sd_cal_capacity(struct sd_info *info)
{
	uint8_t  sd_c_size_mult;
	uint32_t sd_c_size;	
	uint32_t mult;

	if((info->sd_ver & 0x0F) == SD_STD_CARD){
		// version 1.0
		info->sd_rd_blk_len = (info->sd_csd[1] & 0x00000F00) >> 8;
		sd_c_size = ((info->sd_csd[1] & 0x00000003) << 10) | ((info->sd_csd[2] & 0xFFC00000) >> 22);
		sd_c_size_mult = (info->sd_csd[2] & 0x00000380) >> 7;
		mult = 0x01 << (sd_c_size_mult + 2);
		info->sd_rd_blk_len = 0x01 << info->sd_rd_blk_len;
		info->sd_capacity = (sd_c_size + 1) * mult * info->sd_rd_blk_len;
		printf("SD 1.0 -> Capacity = %d MBytes\r\n", info->sd_capacity >> 20);
		printf("Warning! SD 1.0 is not supported\r\n");
		printf("Please Insert SDHC Card (SD 2.0)\r\n");
	}else{
		// version 2.0
		info->sd_rd_blk_len = 512;
		sd_c_size = ((info->sd_csd[2] & 0x3FFFFF00) >> 8);
		info->sd_capacity = (sd_c_size + 1) << 10;
		printf("SD 2.0 -> Capacity = %d MBytes\r\n", info->sd_capacity >> 11);
	}
}

// =========================================================================
// SD card read status data
// =========================================================================
void sd_read_status_data(uint32_t ddr_start_addr, uint32_t blk_len)
{
	uint32_t CRCErrFlag;

	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);

	/*set DMA of Mass_storage */
	sdMsDmaRWSwitch(SD_READ_MODE);
	sdSetBlkSize(blk_len);
	sdSetDmaAddr(ddr_start_addr);

#if defined( CONFIG_SN_GCC_SDK )	
	flush_dcache_area(ddr_start_addr, blk_len);	//write back cacheable data to memory
#endif
	
	/*start DMA transfer*/
	sdMsDmaENSwitch(ENABLE);

	if(sd_ready_detect()){
		printf("sdCheckMsRdy fail\n");
	}

	/*disable DMA*/
	sdMsDmaENSwitch(DISABLE);

	/*send 8 dummy clock*/
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sdCheckMsRdy fail\n");
	}

	/*check crc*/
	CRCErrFlag = sdCheckCrcErr();
	if(CRCErrFlag){
		printf("Error SD CRC Error : error flag = %d.\n", CRCErrFlag);
	}
}


// =========================================================================
// SD card use CMD6 to check high speed mode
// =========================================================================
uint32_t sd_check_highspeed(void)
{
	struct sd_m2_command m2_cmd_info;
	uint32_t SD_CMD6_ARG;
	int32_t test_dma_size = 64;
	int32_t ret = FALSE;
//	uint8_t data_temp[64];
	
	memset(sram_data, 0x00, SRAM_DATA_SIZE);
	
	//write cmd6
	//check function -- check high speed
	SD_CMD6_ARG = 0x00ffff01;		//mode 0; command system: default; access mode: High-Speed
	sd_m2setcmd(SD_CMD6_INDEX, SD_CMD6_ARG, SD_CMD6_SD_RESPTYPE, &m2_cmd_info);
	sd_sd_m2_WcmdRresp(&m2_cmd_info);

	if((m2_cmd_info.resp[0] != 0x6000009) | (m2_cmd_info.resp[1] != 0x0)){
		if((m2_cmd_info.resp[0] == 0xffffffff) && (m2_cmd_info.resp[1] == 0xff)){
			printf("----> SD1.0 cmd6 is reserved\r\n");
		}
		else{
			printf("Error : CMD6 response error\r\n");
			printf("CMD6 response 5-2 (4 bytes) = 0x%x\n", m2_cmd_info.resp[0]);
			printf("CMD6 response 1 (1 bytes) = 0x%x\n", m2_cmd_info.resp[1]);
		}
	}
	else{
		sd_read_status_data((uint32_t)sram_data, test_dma_size);
		if ((sram_data[16] & 0xFF) == 0x01){
			printf("SD card support high speed mode.\r\n");
		}
		else if((sram_data[16] & 0xFF) == 0x0F){
			printf("SD card not support high speed mode.\r\n");
		}
		else{
			printf("sram_data[16] = 0x%x\r\n", sram_data[16]);
		}
	}
	sd_resetm2cmd(&m2_cmd_info);

	if((sram_data[16] & 0xFF) == 0x01)
	{
		//switch function
		SD_CMD6_ARG = 0x80ffff01;	//mode 1; command system: eC; access mode: High-Speed
		sd_m2setcmd(SD_CMD6_INDEX, SD_CMD6_ARG, SD_CMD6_SD_RESPTYPE, &m2_cmd_info);
		sd_sd_m2_WcmdRresp(&m2_cmd_info);

		if((m2_cmd_info.resp[0] != 0x6000009) | (m2_cmd_info.resp[1] != 0x0)){
			if((m2_cmd_info.resp[0] == 0xffffffff) && (m2_cmd_info.resp[1] == 0xff)){
				printf("SD1.0 cmd6 is reserved.\n");
			}
			else{
				printf("Error : CMD6 response error\n");
				printf("CMD6 response 5-2 (4 bytes) = 0x%x\n", m2_cmd_info.resp[0]);
				printf("CMD6 response 1 (1 bytes) = 0x%x\n", m2_cmd_info.resp[1]);
			}
		}
		else{
			sd_read_status_data((uint32_t)sram_data, test_dma_size);
			ret = TRUE;
		}
		sd_resetm2cmd(&m2_cmd_info);
	}
	
//	vPortFree(data_temp);
	
	return ret;
}

// =========================================================================
// SD card M2 initial 
// =========================================================================
int sd_sd_identify(struct sd_info *info)
{
	uint32_t OCRValue;
	uint32_t RCAValue;
	uint32_t arg_temp;
	uint32_t i;
	int32_t ret = 0;
	struct sd_m2_command m2_cmd_info;

	// uint32_t sd_no_resp_c = 0;


//re_init:

	sdSetMsSpeed(SD_CLK_IDENT);

	/*let sd card ready to accept cmd*/
	sdMsRegRWSwitch(SD_READ_MODE);
	sdWriteDummyClock(0x1); // write value can be any value

	if(sd_ready_detect()){
		printf("sd: sdCheckMsRdy fail\n");
		return SD_RTN_ERR_MS_RDY;
	}

	/*fast step*/
	// Card identification mode
	/*CMD0 (softreset)*/
	sd_tx_cmd(SD_CMD0_INDEX, SD_CMD0_SD_ARG_INIT, SD_CMD0_SD_RESPTYPE, &m2_cmd_info);
	sd_resetm2cmd(&m2_cmd_info);

	/*CMD8 (ver2.0 sd memory card)*/
	sd_tx_cmd(SD_CMD8_INDEX, SD_CMD8_SD_ARG_INIT, SD_CMD8_SD_RESPTYPE, &m2_cmd_info);
	if((m2_cmd_info.resp[0] == 0xffffffff) && (m2_cmd_info.resp[1] == 0xff)){
 	 	sd_resetm2cmd(&m2_cmd_info);

		info->sd_ver = (SD_VER_11 << 4) | SD_STD_CARD;

	 	ret = sd_sd_app_cmd_idle(SD_ACMD41_INDEX, SD_ACMD41_SD_ARG_INIT, &OCRValue);

	 	if (ret == FALSE) {
			printf("sd_sd_identify : ACMD41 resp error\r\n");
			return -1;
		}

	 	/*check 3.3v*/
	 	if((sd_r3_32_33(&OCRValue) == 1) && (ret == TRUE)){
	  		do{
	  			if (sd_card_detect() == 1) {
	  				return SD_RTN_ERR_RSP_ACMD41;
	  			}

				ret = sd_sd_app_cmd_idle(SD_ACMD41_INDEX, 0x100000, &OCRValue);

				if (ret == FALSE) {
					printf("%s:%d ACMD41 resp error\n",__func__,__LINE__);
					return -1;
				}

				if((sd_r3_32_33(&OCRValue) == 1) && (ret == TRUE)){
					//printf("OCR bit20=1: 3.2~3.3\n");
				}else{
					return SD_RTN_ERR_RSP_ACMD41;
				}
	  		}while(sd_r3_busy(&OCRValue) == 0); // idle = 1; busy = 0
	 	}else{			
			// if ((ret == FALSE)&&(sd_no_resp_c < 64)) {
			// 	sd_no_resp_c++;
			// 	goto re_init;
			// }
	  		printf("Error : Not support votage 3.2~3.3\n");
	  		return SD_RTN_ERR_RSP_ACMD41;
	 	}

		sd_resetm2cmd(&m2_cmd_info);

		printf("SD 1.0 init->\n");
	}
	else
	{
		info->sd_ver = (info->sd_ver & 0x0F) | (SD_VER_20 << 4);

	 	//check response
	 	if(m2_cmd_info.resp[0] != 0x8000001){
			printf("Error : CMD8 response error\n");
		}

	 	//check pattern
	 	if(m2_cmd_info.resp[1] != (SD_CMD8_SD_ARG_INIT & 0xff)){
			printf("Error : CMD8 check pattern error\n");
			printf("send pattern: %x, readback pattern: %x\n", (SD_CMD8_SD_ARG_INIT & 0xff), m2_cmd_info.resp[1]);
	 	}


		sd_resetm2cmd(&m2_cmd_info);
		ret = sd_sd_app_cmd_idle(SD_ACMD41_INDEX, SD_ACMD41_SD_ARG_INIT, &OCRValue);

		if (ret == FALSE) {
			printf("%s : %d ACMD41 resp error\n", __func__, __LINE__);
			return -1;
		}


	 	/*check 3.3v*/
	 	if((sd_r3_32_33(&OCRValue) == 1) && (ret == TRUE)){
	 		//printf("Support  votage	3.2~3.3\n");
	 		// sd_no_resp_c = 0;
	  		do{
	  			if (sd_card_detect() == 1) {
	  				return SD_RTN_ERR_RSP_ACMD41;
	  			}

				ret = sd_sd_app_cmd_idle(SD_ACMD41_INDEX, 0x40100000, &OCRValue);

				if (ret == FALSE) {
					printf("%s:%d ACMD41 resp error\n",__func__,__LINE__);
					return -1;
				}
#if 1
				if((sd_r3_32_33(&OCRValue) == 1) && (ret == TRUE)){
					//printf("OCR bit20 = 1: 3.2~3.3\n");
				}else{
					return SD_RTN_ERR_RSP_ACMD41;
				}
#else
				if (ret == FALSE) {
					sd_no_resp_c++;
				}

				if (sd_no_resp_c > 128) {
					printf("Unusable Card has polled %d times!!!\n", sd_no_resp_c);
					break;
				}
#endif
	 		}while(sd_r3_busy(&OCRValue) == 0); // idle = 1; busy = 0
	 	}else{	  		
				printf("Error : Not support votage 3.2~3.2\n");
	  		return SD_RTN_ERR_RSP_ACMD41;
	 	}

		if(sd_r3_ccs(&OCRValue) == 1){
			info->sd_ver = (info->sd_ver & 0xF0) | SD_HIGH_CARD;	
		}else{
			info->sd_ver = (info->sd_ver & 0xF0) | SD_STD_CARD;
		}

		sd_resetm2cmd(&m2_cmd_info);

//		printf("SD 2.0 init->\r\n");
	}

	/*CMD2 ask card to send CID number*/
	sd_tx_cmd(SD_CMD2_INDEX, SD_CMD2_SD_ARG_INIT, SD_CMD2_SD_RESPTYPE, &m2_cmd_info);
	// 1-127 bits is CID
	//printf("CID = %x  %x  %x  %x  %x\n", (m2_cmd_info.resp[0] & 0x00ffffff), m2_cmd_info.resp[1], m2_cmd_info.resp[2], m2_cmd_info.resp[3], m2_cmd_info.resp[4]);
	for (i = 0; i < 5; i++) {
		info->sd_cid[i] = m2_cmd_info.resp[i];
	}	
	sd_resetm2cmd(&m2_cmd_info);

	/*CMD3 ask the card to publish new relative address(RCA)*/
	sd_tx_cmd(SD_CMD3_INDEX, SD_CMD3_SD_ARG_INIT, SD_CMD3_SD_RESPTYPE, &m2_cmd_info);
	//printf("CMD3 resp 5-2 (4 bytes) = 0x%x\n", m2_cmd_info.resp[0]);
	//printf("CMD3 resp 1 (1bytes) = 0x%x\n", m2_cmd_info.resp[1]);
	RCAValue = (m2_cmd_info.resp[0] & 0x00ffff00) >> 8;
	//printf("New RCA = 0x%x\n",RCAValue);
	sd_resetm2cmd(&m2_cmd_info);

	// Data Transfer mode
	/*CMD9 address card send its card specific data(CSD)*/
	arg_temp = RCAValue << 16;
	sd_tx_cmd(SD_CMD9_INDEX, arg_temp, SD_CMD9_SD_RESPTYPE, &m2_cmd_info);
	// 1-127 bits is CSD
	//printf("CSD = %x  %x  %x  %x  %x\n", (m2_cmd_info.resp[0] & 0x00ffffff), m2_cmd_info.resp[1], m2_cmd_info.resp[2], m2_cmd_info.resp[3], m2_cmd_info.resp[4]);
	for (i = 0; i < 5; i++) {
		info->sd_csd[i] = m2_cmd_info.resp[i];
	}
	sd_resetm2cmd(&m2_cmd_info);

	sd_cal_capacity(info);

	// Check Card Status
	/*CMD13*/
	i = 0;
	while(m2_cmd_info.next_state != SD_STATE_STBY){
		if (sd_card_detect() == 1) {
	  		return SD_RTN_ERR_RSP_CMD13;
	  	}

		i++;
		if((i > 10) || (sd_check_status_cmd13(RCAValue, &m2_cmd_info) == FALSE)){
			printf("SD CMD13 resp error\n");
			return SD_RTN_ERR_RSP_CMD13;
		}
	}
	sdSetMsSpeed(SD_CLK_LOW);

	/*CMD7 toggles a card to transfer state*/
	arg_temp = RCAValue << 16;
	sd_tx_cmd(SD_CMD7_INDEX, arg_temp, SD_CMD7_SD_RESPTYPE, &m2_cmd_info);

	m2_cmd_info.next_state = SD_STATE_STBY;
	if(sd_r1_check_all(&m2_cmd_info, DISABLE)){
		sd_resetm2cmd(&m2_cmd_info);
	}else{
		printf("SD CMD7 resp error\n");
		return SD_RTN_ERR_RSP_CMD7;
	}
	sd_resetm2cmd(&m2_cmd_info);

	ret = sd_sd_app_cmd_tran(SD_ACMD6_INDEX, RCAValue, info->bus_width);

	if (ret == FALSE) {
		printf("SD ACMD6 resp error\n");
		return -1;
	}

	/*Fine tune transfer mode clock: AHB_CLK / (SPEED + 2)*/	
	if(sd_check_highspeed() == TRUE){
//		printf("SD : high speed\r\n");
		sdSetMsSpeed(SD_CLK_HIGH);
	}
	else{
		printf("SD : normal speed\r\n");		
		//sdSetMsSpeed(SD_CLK_LOW);
	}

	sdMsDmaENSwitch(DISABLE);
	sdMsMDmaENSwitch(DISABLE);

	if((info->sd_ver & 0x0F) == SD_STD_CARD){
		return SD_RTN_ERR_VER_NO_SUPPORT;
	}

	if( ret != SD_RTN_PASS )
		printf("sd_sd_identify : FAIL (%d)\r\n", ret);
	
	return ret;
}
