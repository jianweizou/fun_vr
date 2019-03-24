/**
 * @file
 * this is sd control file
 * sd_ctrl.c
 * @author CJ
 */
 
#include <FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include "snc_types.h"
//#include <bsp.h>
#include "sd_ctrl.h"
#include "sdv2_sd.h"
#include "sdv2.h"

#if defined(CONFIG_SD_DETECT_SIMULATE)
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)	
#include "USBH.h"
#endif
#endif

#define RDY_RETRY_CNT						25000L
#define	SRAM_RW_ADDR						0x18000000
#define	DDR_RW_ADDR							0x30000000

uint8_t SRAM_DATA_BUF[MAX_SRAM_BUF_SIZE];

volatile  int sd_card_remove = 1;
volatile  int sd_card_error = 0; 	// 0:no problem , 1:sd in error 
static int sd_card_identify = 0; 	// 0:No identify , 1:Identify done
struct sd_info sd_card_info;
static uint32_t ms_rdy_flag = 0x0;
static uint32_t ms_err_flag = 0x0;

#if(OS_ON == 1)
// static xSemaphoreHandle		xSEM_SD_Rdy;
static xSemaphoreHandle		xSEM_SD_Intr;
#endif

#define	DBG_TIME 0

// =========================================================================
// SD Ready flag Detect 
// =========================================================================
static uint32_t sd_ready_flag_detect(void)
{
	uint32_t ret;	
#if(OS_ON == 0)	
	uint32_t msReady = 0;
	uint32_t retryCount = 0;
#endif	
	
#if(OS_ON == 1)
//	printf("sd_ready_flag_detect : take xSEM_SD_Intr start\r\n");
	xSemaphoreTake(xSEM_SD_Intr, 5000/portTICK_PERIOD_MS);
//	printf("sd_ready_flag_detect : take xSEM_SD_Intr!!\r\n");
#else
	while(1)
	{
		msReady = sdCheckMsRdyFlag();		
		if( msReady )
		{
			printf("sd_ready_flag_detect : msReady = %d\n", msReady);
			sdClearMsRdyFlag(0x1);
			ms_rdy_flag = 1;
			break;
		}
		else if( retryCount > RDY_RETRY_CNT )
		{
			printf("sd_ready_flag_detect : retryCount = %d\n", retryCount);
			break;
		}
		
		retryCount++;
	}
	
	if(sdCheckMsErrFlag()){
		sdClearMsErrFlag(0x1);
		ms_err_flag = 1;
	}
#endif
		
	if(sd_card_remove){
		ret = 1;
	}

	if((ms_rdy_flag == 1)&&(ms_err_flag == 0)){
		ret = 0;
	}
	else{
		sd_card_error = 1;
		ret = 1;
	}

	ms_rdy_flag = 0;
	ms_err_flag = 0;
	
	return ret;
}

// =========================================================================
// SD card detect interrupt ISR
// =========================================================================
#if 0
static void sd_card_detect_isr(int irq)
{
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	unsigned int data;
	unsigned int i;

	for (i=0; i<81920; i++) {
		data = *((volatile unsigned *)(SD_MS_IO_I));
		data = (data & (0x1 << SD_CD_PIN)) >> SD_CD_PIN;	// inser : 0, remove : 1
	}

	printf("sd isr detect %d\n", data);

	if(data != sd_card_remove){
		sd_card_remove = data;

		if (sd_card_remove == 0) {
			sd_card_error = 0;
		}
		else 
		{
			sd_card_identify=0;
			//printf("sd_card_identify=0\n");
		}
		
		printf("sd isr detect changes %d\n", sd_card_remove);

#if(OS_ON == 1)
	if (sd_card_remove == 1) {	
		xSemaphoreGiveFromISR(xSEM_SD_Intr, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken == pdTRUE){
			printf("release check ready\n");
		}
	}

		xSemaphoreGiveFromISR(sd_card_info.xSEM_SD_CardDetect, &xHigherPriorityTaskWoken);
		xSemaphoreGiveFromISR(sd_card_info.xSEM_SD_CardDetect_USBD, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken == pdTRUE){
			taskYIELD();	//portYIELD_FROM_ISR();
		}
#endif
	}

	sd_card_remove = data;

	sdClearCdIntrFlag(ENABLE);

	return;
}

// =========================================================================
// SD card detect ISR register and enable, for MS2 SD Card Detect
// =========================================================================
static __inline void sd_cdISR_enable(void)
{
	//pic_registerIrq(SD_CARD_DETECT_IRQ_NUM, sd_card_detect_isr, 1);
	//pic_enableInterrupt(SD_CARD_DETECT_IRQ_NUM);	
	//sdCdIntrENSwitch(ENABLE);
}
#endif //end of #if 0

#if !CONFIG_MODULE_WIFI_BCM43362
// =========================================================================
// SD dma complete ISR
// =========================================================================
__irq void SDIO_IRQHandler(void)
{
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
//	printf("SDIO_IRQHandler : start\r\n");
	if(sdCheckMsRdyFlag()){
		sdClearMsRdyFlag(0x1);
		ms_rdy_flag = 1;
	}

	if(sdCheckMsErrFlag()){
		sdClearMsErrFlag(0x1);
		ms_err_flag = 1;
	}

#if(OS_ON == 1)	
	xSemaphoreGiveFromISR(xSEM_SD_Intr, &xHigherPriorityTaskWoken);
	if(xHigherPriorityTaskWoken == pdTRUE){
		taskYIELD();	//portYIELD_FROM_ISR();
	}
#endif
}
#endif

// =========================================================================
// SD card erase function
// =========================================================================
void sd_erase(uint32_t erase_start_addr, uint32_t erase_end_addr, uint32_t mode)
{
	struct sd_m2_command m2_cmd_info;

	if((sd_card_remove) || (sd_card_info.write_protect == 1)){
		return;
	}

	if(mode == MS_SD_MODE){		//for SD mode
		/*CMD32*/
		sd_tx_cmd(SD_CMD32_INDEX, erase_start_addr, SD_CMD32_SD_RESPTYPE, &m2_cmd_info);
		if((m2_cmd_info.resp[0] != 0x20000009) || (m2_cmd_info.resp[1] != 0x0)){
			printf("Error: CMD32 response error\n");
		}
		sd_resetm2cmd(&m2_cmd_info);

		/*CMD33*/
		sd_tx_cmd(SD_CMD33_INDEX, erase_end_addr, SD_CMD33_SD_RESPTYPE, &m2_cmd_info);
		if((m2_cmd_info.resp[0] != 0x21000009) || (m2_cmd_info.resp[1] != 0x0)){
			printf("Error: CMD33 response error\n");
		}
		sd_resetm2cmd(&m2_cmd_info);

		/*CMD38*/
 		sd_tx_cmd(SD_CMD38_INDEX, 0x0, SD_CMD38_SD_RESPTYPE, &m2_cmd_info);
		if((m2_cmd_info.resp[0] != 0x26000008) || (m2_cmd_info.resp[1] != 0x0)){
			printf("Error: CMD38 response error\n");
		}
		sd_resetm2cmd(&m2_cmd_info);
	}
	else
	{				//for SPI mode
		/*CMD32*/
		sd_spi_m2_cmd(SD_CMD32_INDEX, erase_start_addr, SD_CMD32_SD_RESPTYPE, &m2_cmd_info);
		if(m2_cmd_info.resp[0] != 0x0){
			printf("Error: CMD32 response error\n");
		}
		sd_resetm2cmd(&m2_cmd_info);

		/*CMD33*/
		sd_spi_m2_cmd(SD_CMD33_INDEX, erase_end_addr, SD_CMD33_SD_RESPTYPE, &m2_cmd_info);
		if(m2_cmd_info.resp[0] != 0x0){
			printf("Error: CMD33 response error\n");
		}
		sd_resetm2cmd(&m2_cmd_info);

		/*CMD38*/
		sd_spi_m2_cmd(SD_CMD38_INDEX, 0x0, SD_CMD38_SD_RESPTYPE, &m2_cmd_info);
		if(m2_cmd_info.resp[0] != 0x0){
			printf("Error: CMD38 response error\n");
		}
		sd_resetm2cmd(&m2_cmd_info);
	}
	
	sdSetSpiBusyTri(ENABLE);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	/*send 8 dummy clock*/
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}
}

// =========================================================================
// SD card CPU write function
// =========================================================================
#if 0
static void sd_cpu_write(uint32_t sd_start_addr, uint32_t blk_len, uint32_t sd_bus_width, int mode)
{
	struct sd_m2_command m2_cmd_info;
	uint32_t CRC16_0[2], CRC16_1[2], CRC16_2[2], CRC16_3[2]; //[1] = H; [0] = L
	uint32_t temp = 0x0;
	int32_t i = 0;

	if((sd_card_remove) || (sd_card_info.write_protect == 1)){
		return;
	}

	for(i = 0; i < 2; i++){
		CRC16_0[i] = 0x0;
		CRC16_1[i] = 0x0;
		CRC16_2[i] = 0x0;
		CRC16_3[i] = 0x0;
	}

	sdSetBlkSize(blk_len);
	if(mode == MS_SD_MODE){
		sd_sd_write_cmd(sd_start_addr,	blk_len, &m2_cmd_info);
	}else{
		sd_spi_write_cmd(sd_start_addr, blk_len, &m2_cmd_info);
	}

	/*start bit*/
	if(sd_bus_width == SD_BUS_WIDTH_1BIT){
		sdWriteData(0xfe);
	}else if(sd_bus_width == SD_BUS_WIDTH_4BIT){
		sdWriteData(0xf0);
	}

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	for(i = 0; i <= blk_len; i++){
		sdWriteData(i);

		if(sd_ready_detect()){
			printf("sd: sd_ready_detect fail\n");
			return;
		}
	}

	// Read CRC
	temp = sdReadCrc16_0();
	CRC16_0[1] = (temp & 0x0000ff00) >> 8;
	CRC16_0[0] = (temp & 0x000000ff) >> 0;

	temp = sdReadCrc16_1();
	CRC16_1[1] = (temp & 0x0000ff00) >> 8;
	CRC16_1[0] = (temp & 0x000000ff) >> 0;

	temp = sdReadCrc16_2and16_3();
	CRC16_3[1] = (temp & 0xff000000) >> 24;
	CRC16_3[0] = (temp & 0x00ff0000) >> 16;
	CRC16_2[1] = (temp & 0x0000ff00) >> 8;
	CRC16_2[0] = (temp & 0x000000ff) >> 0;

	// Write CRC
	sdMsRegRWSwitch(SD_WRITE_MODE);
	for(i = 1 ; i >= 0; i--){
		if(sd_bus_width == SD_BUS_WIDTH_1BIT){
			temp = CRC16_0[i];
			sdWriteData(temp);
		}
		if(sd_bus_width == SD_BUS_WIDTH_4BIT){
			temp = 0x0;
			temp = ((CRC16_3[i]&BIT7) >> 0) 
				| ((CRC16_2[i]&BIT7) >> 1) 
				| ((CRC16_1[i]&BIT7) >> 2)  
				| ((CRC16_0[i]&BIT7) >> 3);
			temp |=  ((CRC16_3[i]&BIT6) >> 3) 
				| ((CRC16_2[i]&BIT6) >> 4) 
				| ((CRC16_1[i]&BIT6) >> 5) 
				| ((CRC16_0[i]&BIT6) >> 6);
			sdWriteData(temp);

			if(sd_ready_detect()){
				printf("sd: sd_ready_detect fail\n");
				return;
			}

			temp = 0x0;
			temp = ((CRC16_3[i]&BIT5) << 2)
				| ((CRC16_2[i]&BIT5) << 1) 
				| ((CRC16_1[i]&BIT5) >> 0) 
				| ((CRC16_0[i]&BIT5) >> 1);
			temp |= ((CRC16_3[i]&BIT4) >> 1) 
				| ((CRC16_2[i]&BIT4) >> 2) 
				| ((CRC16_1[i]&BIT4) >> 3) 
				| ((CRC16_0[i]&BIT4) >> 4);
			sdWriteData(temp);

			if(sd_ready_detect()){
				printf("sd: sd_ready_detect fail\n");
				return;
			}

			temp = 0x0;
			temp = ((CRC16_3[i] & BIT3) << 4) 
				| ((CRC16_2[i] & BIT3) << 3) 
				| ((CRC16_1[i] & BIT3) << 2) 
				| ((CRC16_0[i] & BIT3) << 1);
			temp |=  ((CRC16_3[i] & BIT2) << 1) 
				| ((CRC16_2[i] & BIT2) >> 0) 
				| ((CRC16_1[i] & BIT2) >> 1) 
				| ((CRC16_0[i] & BIT2) >> 2);
			sdWriteData(temp);

			if(sd_ready_detect()){
				printf("sd: sd_ready_detect fail\n");
				return;
			}

			temp = 0x0;
			temp = ((CRC16_3[i] & BIT1) << 6) 
				| ((CRC16_2[i] & BIT1) << 5) 
				| ((CRC16_1[i] & BIT1) << 4) 
				| ((CRC16_0[i] & BIT1) << 3);
			temp |=  ((CRC16_3[i] & BIT0) << 3) 
				| ((CRC16_2[i] & BIT0) << 2) 
				| ((CRC16_1[i] & BIT0) << 1) 
				| ((CRC16_0[i] & BIT0) >> 0);
			sdWriteData(temp);

			if(sd_ready_detect()){
				printf("sd: sd_ready_detect fail\n");
				return;
			}
		}
	}

	/*send 8 dummy clock*/
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	/*if data write to sd card*/
	sdSetSpiBusyTri(ENABLE);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	/*send 8 dummy clock*/
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}
}
#endif

// =========================================================================
// SD card DMA write function
// =========================================================================
#if 0
static void sd_dma_write(uint32_t sd_start_addr , uint32_t ddr_start_addr , uint32_t blk_len, uint32_t sd_bus_width, uint32_t mode)
{
	uint32_t resp, resp1, resp2;	
	struct sd_m2_command m2_cmd_info;

	if((sd_card_remove) || (sd_card_info.write_protect == 1)){
		return;
	}

	/*DMA_Write*/
	if(mode == MS_SD_MODE){
		sd_sd_write_cmd(sd_start_addr, blk_len, &m2_cmd_info);
	}else{
		sd_spi_write_cmd(sd_start_addr, blk_len, &m2_cmd_info);
	}

	/*set DMA of Mass_storage */
	sdMsDmaRWSwitch(SD_WRITE_MODE);
	sdSetBlkSize(blk_len);
	sdSetDmaAddr(ddr_start_addr);

	/*send 8 dummy clock*/
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	/*DMA write start*/
	if(sd_bus_width == SD_BUS_WIDTH_1BIT){
		sdWriteData(0xfe);
	}	
	if(sd_bus_width == SD_BUS_WIDTH_4BIT){
		sdWriteData(0xf0);
	}

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	sdMsErrIntrENSwitch(ENABLE);
	sdMsRdyIntrENSwitch(ENABLE);

	/*start DMA transfer*/
	sdMsDmaENSwitch(ENABLE);

	if(mode == MS_SD_MODE){
		if(sd_ready_flag_detect()){
			printf("sd: sd_ready_flag_detect fail\n");
			return;
		}
	}else{
		if(sd_ready_detect()){
			printf("sd: sd_ready_detect fail\n");
			return;
		}		
	}

	sdMsRdyIntrENSwitch(DISABLE);
	sdMsErrIntrENSwitch(DISABLE);

	/*disable DMA*/
	sdMsDmaENSwitch(DISABLE);

	/*read response*/
	resp1 = sdReadSpiIndex();
	resp2 = sdReadSpiCmd();
	resp = ((resp1 & 0xff) << 24) | ((resp2 & 0xffffff) >> 8);

	if(mode == MS_SD_MODE){
		if(resp != 0x05000009){
			printf("Error: SD mode dma write CRC error\n");
		}
	}else{
		if(resp != 0x0){
			printf("Error: SPI mode dma write CRC error\n");
		}
	}

	resp = (resp & 0xe000000) >> 25;
	if(resp == 0x5){
		printf("Error: Write CRC error at DMA WRITE MODE\n");
	}

	/*if data write to sd card*/
	sdSetSpiBusyTri(ENABLE);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	/*send 8 dummy clock*/
	sdMsRegRWSwitch(SD_WRITE_MODE);
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}
}
#endif

// =========================================================================
// SD card DMA read function
// =========================================================================
void sd_dma_read(uint32_t sd_start_addr, uint32_t ddr_start_addr, uint32_t blk_len, uint32_t mode)
{	
	int32_t CRCErrFlag;	
	struct sd_m2_command m2_cmd_info;

	if(sd_card_remove){
		return;
	}

	if(mode == MS_SD_MODE){
		sd_sd_read_cmd(sd_start_addr, &m2_cmd_info);
	}else{
		sd_spi_read_cmd(sd_start_addr, &m2_cmd_info);
	}

	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);

	/*set DMA of Mass_storage */
	sdMsDmaRWSwitch(SD_READ_MODE);
	sdSetBlkSize(blk_len);
	sdSetDmaAddr(ddr_start_addr);

	sdMsErrIntrENSwitch(ENABLE);
	sdMsRdyIntrENSwitch(ENABLE);

	/*start DMA transfer*/
	sdMsDmaENSwitch(ENABLE);

	if(mode == MS_SD_MODE){
		if(sd_ready_flag_detect()){
			printf("sd_dma_read : sd_ready_flag_detect fail\n");
			return;
		}
	}else{
		if(sd_ready_detect()){
			printf("sd: sd_ready_detect fail\n");
			return;
		}		
	}

	sdMsRdyIntrENSwitch(DISABLE);
	sdMsErrIntrENSwitch(DISABLE);

	/*disable DMA*/
	sdMsDmaENSwitch(DISABLE);

	/*send 8 dummy clock*/
	sdWriteDummyClock(0x1);

	if(sd_ready_detect()){
		printf("sd: sd_ready_detect fail\n");
		return;
	}

	/*check crc*/
	CRCErrFlag = sdCheckCrcErr();
	if(CRCErrFlag){
		printf("Error: SD CRC Error: error number = test\n");
	}
}

// =========================================================================
// SD card 
// =========================================================================
static int sd_mdma_write(uint32_t sd_start_addr, uint32_t ddr_start_addr, uint32_t blk_len, uint32_t blk_num, uint32_t mode)
{
	//struct sd_m2_command m2_cmd_info;
	int status;
	int ret = SD_RTN_ERR_MDMA_FAIL;

//	printf("sd_mdma_write : sd_start_addr = 0x%x, ddr_start_addr = 0x%x, blk_num = %d\r\n", sd_start_addr, ddr_start_addr, blk_num);
	
	if ((sd_card_remove) || (sd_card_error)) {
		return SD_RTN_ERR_CARD_REMOVED;
	}

	if(sd_card_info.write_protect == 1){
		return SD_RTN_SD_CARD_WRITE_PROTECT;
	}

	/*DMA_Write*/
#if 0
	/*CMD16*/
	if(mode == MS_SD_MODE){
		sd_tx_cmd(SD_CMD16_INDEX, blk_len, SD_CMD16_SD_RESPTYPE, &m2_cmd_info);
		if((m2_cmd_info.resp[0] != 0x10000009) || (m2_cmd_info.resp[1] != 0x0)){
			printf("Error: CMD16 response error\n");
			sd_card_error = 1;
			return SD_RTN_ERR_RSP_CMD16;
		}
	}else{
		sd_spi_m2_cmd(SD_CMD16_INDEX, blk_len, SD_CMD16_SPI_RESPTYPE, &m2_cmd_info);
		if(m2_cmd_info.resp[0] != 0x0){
			printf("Error: CMD16 response error\n");
			return SD_RTN_ERR_RSP_CMD16;
		}
	}
	sd_resetm2cmd(&m2_cmd_info);
#endif

	// Set sd start addr.
	// sdMsRegRWSwitch(SD_WRITE_MODE);
	// sdWriteSpiIndex(SD_CMD25_INDEX);	
	sdWriteSpiCmd(sd_start_addr);
	
	// Set DMA of Mass_storage.
	sdSetBlkSize(blk_len);
	sdSetTimeCount(TIME_CNT);
		
	if(ddr_start_addr >= (uint32_t)DDR_RW_ADDR)
	{
		memcpy(SRAM_DATA_BUF, (void *)ddr_start_addr, blk_num*512);
		sdSetDmaAddr((uint32_t)SRAM_DATA_BUF);
	}
	else
		sdSetDmaAddr(ddr_start_addr);
	
	sdMsMDmaENSwitch(ENABLE);
	sdDmaBlock(blk_num);
	sdMsDmaRWSwitch(SD_WRITE_MODE);

	sdMsErrIntrENSwitch(ENABLE);
	sdMsRdyIntrENSwitch(ENABLE);
	sdMsDmaENSwitch(ENABLE);
	
#if 0
while(1){
	if(sdCheckMsRdyFlag()){
		sdClearMsRdyFlag(0x1);
		ms_rdy_flag = 1;
		break;
	}
}

	if(sdCheckMsErrFlag()){
		sdClearMsErrFlag(0x1);
		ms_err_flag = 1;
	}
#else
	if(mode == MS_SD_MODE){
		if(sd_ready_flag_detect()){
			printf("sd: sd_ready_flag_detect fail\n");
			ret = SD_RTN_ERR_MDMA_COMPLETE_FLAG;
			goto sd_mdma_write_err;
		}
	}else{
		if(sd_ready_detect()){
			printf("sd: sd_ready_detect fail\n");
			ret = SD_RTN_ERR_MS_RDY;
			goto sd_mdma_write_err;
		}
	}
#endif
	status = sdCheckMsMDmaOkOnTime();

	if(status == MDMA_OK_ON_TIME){
		ret = SD_RTN_PASS;
	}else{
		sd_card_error = 1;

		if(sd_card_detect()){
			sd_card_remove = 1;
			ret = SD_RTN_ERR_CARD_REMOVED;
			printf("Error: SD card is removed\n");
		}else if(status == MDMA_OK_TIME_OUT){
			ret = SD_RTN_ERR_MDMA_TIMEOUT;
			printf("Error: M DMA complete with Time out flag\n");
		}else if(status == MDMA_FAIL_TIME_OUT){
			ret = SD_RTN_ERR_MDMA_TIMEOUT;
			printf("Error: M DMA not complete with Time out flag\n");
		}else{
			ret = SD_RTN_ERR_MDMA_FAIL;
			printf("sd_mdma_write : Error!! M DMA not complete\r\n");
		}
	}

sd_mdma_write_err:
	sdMsRdyIntrENSwitch(DISABLE);
	sdMsErrIntrENSwitch(DISABLE);
	sdMsDmaENSwitch(DISABLE);
	sdMsMDmaENSwitch(DISABLE);

	return ret;
}

// =========================================================================
// SD card M-DMA Read function
// =========================================================================
static int sd_mdma_read(uint32_t sd_start_addr, uint32_t ddr_start_addr, uint32_t blk_len, int32_t blk_num, int32_t mode)
{
	//struct sd_m2_command m2_cmd_info;
	int status;
	int ret = SD_RTN_ERR_MDMA_FAIL;
	
//	printf("sd_mdma_read : sd_start_addr = 0x%x, ddr_start_addr = 0x%x, blk_num = %d\r\n", sd_start_addr, ddr_start_addr, blk_num);
	
	if ((sd_card_remove) || (sd_card_error)) {
		return SD_RTN_ERR_CARD_REMOVED;
	}

#if 0
	/*CMD16*/
	if(mode == MS_SD_MODE){
		sd_tx_cmd(SD_CMD16_INDEX, blk_len, SD_CMD16_SD_RESPTYPE, &m2_cmd_info);
		if((m2_cmd_info.resp[0] != 0x10000009) || (m2_cmd_info.resp[1] != 0x0)){
			printf("Error: CMD16 response error\n");
			sd_card_error = 1;
			return SD_RTN_ERR_RSP_CMD16;
		}
	}else{
		sd_spi_m2_cmd(SD_CMD16_INDEX, blk_len, SD_CMD16_SPI_RESPTYPE, &m2_cmd_info);
		if(m2_cmd_info.resp[0] != 0x0){
			printf("Error: CMD16 response error\n");
			return SD_RTN_ERR_RSP_CMD16;
		}
	}
	sd_resetm2cmd(&m2_cmd_info);
#endif

	// Set sd start addr.
	// sdMsRegRWSwitch(SD_WRITE_MODE);
	// sdWriteSpiIndex(SD_CMD18_INDEX);
	sdWriteSpiCmd(sd_start_addr);

	/*set DMA of Mass_storage */
	sdSetBlkSize(blk_len);
	sdSetTimeCount(TIME_CNT);

	if(ddr_start_addr >= DDR_RW_ADDR)
		sdSetDmaAddr((uint32_t)SRAM_DATA_BUF);
	else
		sdSetDmaAddr(ddr_start_addr);
	
	sdMsMDmaENSwitch(ENABLE);
	
	
	sdDmaBlock(blk_num);
	sdMsDmaRWSwitch(SD_READ_MODE);

	sdMsErrIntrENSwitch(ENABLE);
	sdMsRdyIntrENSwitch(ENABLE);
	sdMsDmaENSwitch(ENABLE);

	if(mode == MS_SD_MODE){
		if(sd_ready_flag_detect()){
			printf("sd: sd_ready_flag_detect fail\n");
			ret = SD_RTN_ERR_MDMA_COMPLETE_FLAG;
			goto sd_mdma_read_err;
		}
	}else{
		if(sd_ready_detect()){
			printf("sd: sd_ready_detect fail\n");
			ret = SD_RTN_ERR_MS_RDY;
			goto sd_mdma_read_err;
		}
	}

	status = sdCheckMsMDmaOkOnTime();

	if(status == MDMA_OK_ON_TIME){
		ret = SD_RTN_PASS;
		if(ddr_start_addr >= DDR_RW_ADDR)		
			memcpy((void *)ddr_start_addr, SRAM_DATA_BUF, blk_num*512);
	}else{
		sd_card_error = 1;

		if(sd_card_detect()){
			sd_card_remove = 1;
			ret = SD_RTN_ERR_CARD_REMOVED;
			printf("Error: SD card is removed\n");
		}else if(status == MDMA_OK_TIME_OUT){
			ret = SD_RTN_ERR_MDMA_TIMEOUT;
			printf("Error: M DMA complete with Time out flag\n");
		}else if(status == MDMA_FAIL_TIME_OUT){
			ret = SD_RTN_ERR_MDMA_TIMEOUT;
			printf("Error: M DMA not complete with Time out flag\n");
		}else{
			ret = SD_RTN_ERR_MDMA_FAIL;
			printf("sd_mdma_read : Error !! M DMA not complete\r\n");
		}
	}

sd_mdma_read_err:
	sdMsRdyIntrENSwitch(DISABLE);
	sdMsErrIntrENSwitch(DISABLE);
	sdMsDmaENSwitch(DISABLE);
	sdMsMDmaENSwitch(DISABLE);

	return ret;
}

int snx_sd_initial_for_remove (void)
{
	// vSemaphoreDelete(xSEM_SD_Rdy);
#if(OS_ON == 1)	
	vSemaphoreDelete(xSEM_SD_Intr);
	vSemaphoreDelete(sd_card_info.xSEM_SD_CardDetect);
	vSemaphoreDelete(sd_card_info.xSEM_SD_CardDetect_USBD);
#endif
	
	ms_rdy_flag = 0;
	ms_err_flag = 0;

	sd_card_remove = 1;
	sd_card_error = 0;
	sd_card_info.ms_mode = MS_SD_MODE;
	sd_card_info.write_protect = 0;
	sd_card_info.bus_width = SD_BUS_WIDTH_4BIT;
	sd_card_info.sd_ver = (SD_VER_11 << 4) | SD_STD_CARD;
	sd_card_info.sd_capacity = 0;
	sd_card_info.sd_rd_blk_len = 0;
	sd_card_identify = 0;

	sdReset();
	sdSetMsMode(MS_SD_MODE);

	/*set related register*/
	sdExtraENSwitch(ENABLE);
	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);
	sdSetLba(0x3ff);			//auto
	sdSetTimeCount(TIME_CNT);
	sdSetMsSpeed(SD_CLK_IDENT);


	// xSEM_SD_Rdy = xSemaphoreCreateMutex();
	// if(xSEM_SD_Rdy == NULL){
	// 	return	SD_RTN_ERR_SEMAPHORE_CREATE;
	// }

#if(OS_ON == 1)
	xSEM_SD_Intr = xSemaphoreCreateBinary();
	if(xSEM_SD_Intr == NULL){
		// vSemaphoreDelete(xSEM_SD_Rdy);
		return	SD_RTN_ERR_SEMAPHORE_CREATE;
	}	

	sd_card_info.xSEM_SD_CardDetect = xSemaphoreCreateBinary();	
	if(sd_card_info.xSEM_SD_CardDetect == NULL){
		// vSemaphoreDelete(xSEM_SD_Rdy);
		vSemaphoreDelete(xSEM_SD_Intr);
		return	SD_RTN_ERR_SEMAPHORE_CREATE;
	}

	sd_card_info.xSEM_SD_CardDetect_USBD= xSemaphoreCreateBinary();	
	if(sd_card_info.xSEM_SD_CardDetect_USBD == NULL){
		// vSemaphoreDelete(xSEM_SD_Rdy);
		vSemaphoreDelete(xSEM_SD_Intr);
		return	SD_RTN_ERR_SEMAPHORE_CREATE;
	}
#endif
	
	sd_card_detect();
	sd_write_protect_detect();

	sdCdIntrENSwitch(ENABLE);
	return SD_RTN_PASS;
}



/**
* @brief Initial SD parameter.
* @param mode 1:SPI mode. 2:SD mode.
* @return SD initial is success or fail.
*/
int sd_init(uint8_t mode)
{
	ms_rdy_flag = 0;
	ms_err_flag = 0;

	sd_card_remove = 1;
	sd_card_error = 0;
	sd_card_info.ms_mode = mode;
	sd_card_info.write_protect = 0;
	sd_card_info.bus_width = SD_BUS_WIDTH_4BIT;
	sd_card_info.sd_ver = (SD_VER_11 << 4) | SD_STD_CARD;
	sd_card_info.sd_capacity = 0;
	sd_card_info.sd_rd_blk_len = 0;
	sd_card_identify = 0;

//	printf("sd_init : start\r\n");
	
	sdReset();
	sdSetMsMode(mode);

	/*set related register*/
	sdExtraENSwitch(ENABLE);
	sdEccENSwitch(DISABLE);
	sdEccENSwitch(ENABLE);
	sdSetLba(0x3ff);			//auto
	sdSetTimeCount(TIME_CNT);

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

	//sd_cdISR_enable();

#if(OS_ON == 1)	
#if defined(CONFIG_PLATFORM_ST53510)
	pic_registerIrq(SD1_IRQ_NUM, sd_dma_complete_isr, 1);
	pic_enableInterrupt(SD1_IRQ_NUM);
#else	
//	pic_registerIrq(MS2_IRQ_NUM, sd_dma_complete_isr, 1);
//	pic_registerIrq(MS2_IRQ_NUM, SDIO_IRQHandler, 1);	
//	pic_enableInterrupt(MS2_IRQ_NUM);		
	NVIC_EnableIRQ(SDIO_IRQn);
#endif
#endif	//end of #if(OS_ON == 1)

#if(OS_ON == 1)
	// xSEM_SD_Rdy = xSemaphoreCreateMutex();
	// if(xSEM_SD_Rdy == NULL){
	// 	return	SD_RTN_ERR_SEMAPHORE_CREATE;
	// }

	xSEM_SD_Intr = xSemaphoreCreateBinary();
	if(xSEM_SD_Intr == NULL){
		// vSemaphoreDelete(xSEM_SD_Rdy);
		printf("sd_init : SD_RTN_ERR_SEMAPHORE_CREATE\n");
		return	SD_RTN_ERR_SEMAPHORE_CREATE;
	}

	sd_card_info.xSEM_SD_CardDetect = xSemaphoreCreateBinary();	
	if(sd_card_info.xSEM_SD_CardDetect == NULL){
		// vSemaphoreDelete(xSEM_SD_Rdy);
		vSemaphoreDelete(xSEM_SD_Intr);
		printf("sd_init : SD_RTN_ERR_SEMAPHORE_CREATE\n");
		return	SD_RTN_ERR_SEMAPHORE_CREATE;
	}

	sd_card_info.xSEM_SD_CardDetect_USBD= xSemaphoreCreateBinary(); 
	if(sd_card_info.xSEM_SD_CardDetect_USBD== NULL){
		// vSemaphoreDelete(xSEM_SD_Rdy);
		vSemaphoreDelete(xSEM_SD_Intr);
		printf("sd_init : SD_RTN_ERR_SEMAPHORE_CREATE\n");
		return	SD_RTN_ERR_SEMAPHORE_CREATE;
	}
#endif

	sd_card_detect();
	sd_write_protect_detect();	

//	printf("sd_init : pass\r\n");
	return SD_RTN_PASS;
}

/**
* @brief Uninitial SD parameter.
* @return SD uninitial is success or fail.
*/
int sd_uninit(void)
{
#if defined(CONFIG_PLATFORM_ST53510)
	//pic_disableInterrupt(SD1_IRQ_NUM);
#else
	//pic_disableInterrupt(MS2_IRQ_NUM);
#endif
	
	//pic_disableInterrupt(SD_CARD_DETECT_IRQ_NUM);
	
#if defined(CONFIG_PLATFORM_ST53510)
	//pic_unregisterIrq(SD1_IRQ_NUM);
#else	
	//pic_unregisterIrq(MS2_IRQ_NUM);
#endif
	//pic_unregisterIrq(SD_CARD_DETECT_IRQ_NUM);

#if(OS_ON == 1)
	// vSemaphoreDelete(xSEM_SD_Rdy);
	vSemaphoreDelete(xSEM_SD_Intr);
	vSemaphoreDelete(sd_card_info.xSEM_SD_CardDetect);
	vSemaphoreDelete(sd_card_info.xSEM_SD_CardDetect_USBD);
#endif

	sd_card_remove = 1;

	printf("sd driver unloaded\n");

	return SD_RTN_PASS;	
}

/**
* @brief Identify SD card version and capacity.
* @return SD card identify is success or fail.
*/
int sd_identify(void)
{
	uint32_t ret = 0;

#if(OS_ON == 1)
	// if(xSemaphoreTake(xSEM_SD_Rdy, SD_TAKE_SEMAPHORE_WAIT_TIME) == pdFAIL){
	// 	//printf("xSEM_SD_Rdy Err\r\n");
	// 	return SD_RTN_ERR_SEMAPHORE_TAKE;
	// }
#endif
		
	sd_card_detect();
	sd_write_protect_detect();	
	
	if(!sd_card_identify)
	{
		if(sd_card_info.ms_mode == MS_SPI_MODE){
			ret = sd_spi_identify(&sd_card_info);	
		}else{
			ret = sd_sd_identify(&sd_card_info);
		}
		
		if( ret != SD_RTN_PASS )
			printf("sd_identify FAIL !!\r\n");
		
		sd_card_identify = ret;
	}
	else
		ret=TRUE;
	

#if(OS_ON == 1)
	// xSemaphoreGive(xSEM_SD_Rdy);
#endif

	return ret;
}

/**
* @brief Write data to SD card.
* @param dst_addr destination address pointer of SD card(unit 512Byte).
* @param src_addr source address pointer of DRAM.
* @param blk_num write numbers of data blocks(1 block = 512Byte).
* @return Message of write data to SD card is success or fail.
*/
int sd_write(uint32_t dst_addr, uint32_t src_addr, uint32_t blk_num)
{
	int rtn;
	uint32_t blk_len;

#if(DBG_TIME == 1)
	TickType_t cur_tick, spent_tick;
#endif	
#if(OS_ON == 1)
	// if(xSemaphoreTake(xSEM_SD_Rdy, SD_TAKE_SEMAPHORE_WAIT_TIME) == pdFAIL){
	// 	//printf("xSEM_SD_Rdy Err\r\n");
	// 	return SD_RTN_ERR_SEMAPHORE_TAKE;
	// }
#endif
#if(DBG_TIME == 1)
	cur_tick = xTaskGetTickCount();
#endif

	//dst_addr unit: 512Byte
	//blk_num unit: 512Byte
	// ----- error check ----- // 
	if(dst_addr > (sd_card_info.sd_capacity - 1)){
		return SD_RTN_ERR_MDMA_SIZE;
	}else if(((dst_addr + blk_num) > sd_card_info.sd_capacity) || ((sd_card_info.sd_rd_blk_len == 0)&&(sd_card_info.ms_mode == MS_SD_MODE))
			||(blk_num == 0)){
		return SD_RTN_ERR_MDMA_SIZE;
	}

	/*
	if(sd_card_info.ms_mode == MS_SPI_MODE){
		blk_len = 512;
	}else{
		if(sd_card_info.sd_rd_blk_len <= 512){
			blk_len = sd_card_info.sd_rd_blk_len;
		}else{
			blk_len = 512;
		}
	}
	*/

	blk_len = sd_card_info.sd_rd_blk_len;
	rtn = sd_mdma_write(dst_addr, src_addr, blk_len, blk_num, sd_card_info.ms_mode);

#if(DBG_TIME == 1)
	spent_tick = xTaskGetTickCount() - cur_tick;
	if(spent_tick>5){
		printf("sd_write:2 = %d\n", spent_tick);
	}
#endif
#if(OS_ON == 1)
	// xSemaphoreGive(xSEM_SD_Rdy);
#endif

	return rtn;
}

/**
* @brief Read data from SD card.
* @param dst_addr destination address pointer of DRAM.
* @param src_addr source address pointer of SD card(unit 512Byte).
* @param blk_num read numbers of data blocks(1 block = 512Byte).
* @return Message of read data from SD card is success or fail.
*/
int sd_read(uint32_t dst_addr, uint32_t src_addr, uint32_t blk_num)
{
	int rtn;
	uint32_t blk_len;

#if(DBG_TIME == 1)
	TickType_t cur_tick, spent_tick;
#endif	
#if(OS_ON == 1)
	// if(xSemaphoreTake(xSEM_SD_Rdy, SD_TAKE_SEMAPHORE_WAIT_TIME) == pdFAIL){
	// 	//printf("xSEM_SD_Rdy Err\r\n");
	// 	return SD_RTN_ERR_SEMAPHORE_TAKE;
	// }
#endif
#if(DBG_TIME == 1)
	cur_tick = xTaskGetTickCount();
#endif

	//src_addr unit: 512Byte
	//blk_num unit: 512Byte
	// ----- error check ----- // 
	if(src_addr > (sd_card_info.sd_capacity - 1)){
		return SD_RTN_ERR_MDMA_SIZE;
	}else if(((src_addr + blk_num) > sd_card_info.sd_capacity) || ((sd_card_info.sd_rd_blk_len == 0)&&(sd_card_info.ms_mode == MS_SD_MODE))
			||(blk_num == 0)){
		return SD_RTN_ERR_MDMA_SIZE;
	}

	/*
	if(sd_card_info.ms_mode == MS_SPI_MODE){
		blk_len = 512;
	}else{
		if(sd_card_info.sd_rd_blk_len <= 512){
			blk_len = sd_card_info.sd_rd_blk_len;
		}else{
			blk_len = 512;
		}
	}
	*/

	blk_len = sd_card_info.sd_rd_blk_len;

	rtn = sd_mdma_read(src_addr, dst_addr, blk_len, blk_num, sd_card_info.ms_mode);

#if(DBG_TIME == 1)
	spent_tick = xTaskGetTickCount() - cur_tick;
	if(spent_tick>5){
		printf("sd_read:2 = %d\n", spent_tick);
	}
#endif
#if(OS_ON == 1)
	// xSemaphoreGive(xSEM_SD_Rdy);
#endif

	return rtn;
}

/**
* @brief Get SD card capacity.
* @return Size of SD card capacity.
*/
uint32_t sd_get_capacity(void)
{
	//unit: 512Byte
	if((sd_card_info.sd_ver & 0x0F) == SD_STD_CARD){
		return sd_card_info.sd_capacity >> 9;
	}else{
		printf("sd_get_capacity : sd_capacity = %d\n", sd_card_info.sd_capacity);
		printf("sd_get_capacity : sd_rd_blk_len = %d\n", sd_card_info.sd_rd_blk_len);
		return sd_card_info.sd_capacity;
	}	
}

/**
* @brief Detect SD card write protect pin.
* @return SD card is write protect or not.
*/
int sd_write_protect_detect(void)
{ 
	uint32_t data;

	//set msio input mode
	data = *((volatile unsigned *)(SD_MS_IO_OE));
	data &= ~(0x1 << SD_CD_PIN);
	*((volatile unsigned *)(SD_MS_IO_OE)) = data;

	//detect write protect pin
	data = *((volatile unsigned *)(SD_MS_IO_I));
	data = (data & (0x1 << SD_WP_PIN)) >> SD_WP_PIN;

	sd_card_info.write_protect = data;

	if (sd_card_info.write_protect){
		//printf("SD: write protect\n");
		return SD_RTN_SD_CARD_WRITE_PROTECT;
	}
	else{
		//printf("SD: not write protect\n");
		return SD_RTN_SD_CARD_NO_WRITE_PROTECT;
	}
}

/**
* @brief Detect SD card exist.
* @return SD card is exist or not.
*/
int sd_card_detect(void)
{
#if defined(CONFIG_SD_DETECT_SIMULATE)	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)	
	automount_info_usbh   *automt_info_usbh;
	
//	printf("sd_card_detect : CONFIG_SD_DETECT_SIMULATE \r\n");
#if defined(CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined(CONFIG_USBH_PHY0_DEV_PHY1_HOST)
	automt_info_usbh = get_automount_info_usbh(USBH2);
#endif
#if defined(CONFIG_USBH_PHY0_HOST_PHY1_HOST) || defined(CONFIG_USBH_PHY0_HOST_PHY1_DEV)
	automt_info_usbh = get_automount_info_usbh(USBH1);
#endif
	
	if( xSemaphoreTake(automt_info_usbh->xSEM_USBH_PLUG_IN, 300/portTICK_PERIOD_MS ) == pdTRUE ) //portMAX_DELAY
	{
//		printf("sd_card_detect : SD Card IN !!\r\n");
		sd_card_remove = 0;
	}		
	
	if( xSemaphoreTake(automt_info_usbh->xSEM_USBH_PLUG_OUT, 300/portTICK_PERIOD_MS ) == pdTRUE ) 
	{
//		printf("sd_card_detect : SD Card OUT !!\r\n");
		sd_card_remove = 1;
	}
	
	return sd_card_remove;
#endif
	
#else
	
	uint32_t data = 0;

	//set msio input mode
	// data = *((volatile unsigned *)(SD_MS_IO_OE));
	// data &= ~(0x1 << SD_CD_PIN);
	// *((volatile unsigned *)(SD_MS_IO_OE)) = data;

	//detect card remove pin
	data = *((volatile unsigned *)(SD_MS_IO_I));
	sd_card_remove = (data & (0x1 << SD_CD_PIN)) >> SD_CD_PIN;
	
	return sd_card_remove;

	// if(sd_card_remove){
	// 	return SD_DETECT_NONE;
	// }
	// else{
	// 	return SD_DETECT_CD_PIN;
	// }
#endif	//end of #if defined(CONFIG_SD_DETECT_SIMULATE)	
}

/**
* @brief Get SD info structure pointer.
* @return SD information structure pointer.
*/
struct sd_info* sd_get_info(void)
{
	return &sd_card_info;
}

int get_sd_error(void)
{
	return sd_card_error;
}

int	sd_get_memory_present(void)
{
	int				i = 0;
	int				memory_present = SD_MEMORY_UNKNOWN;
	uint8_t		number_fun = 0;
	uint32_t 	resp0 = 0;
	uint32_t 	resp1 = 0;
	struct 		sd_m2_command m2_cmd_info;
	
	for( i=0; i<2; i++ )
	{
		sd_tx_cmd(SD_CMD5_INDEX, SD_CMD5_SD_ARG_INIT, SD_CMD5_SD_RESPTYPE, &m2_cmd_info);
		resp0 = m2_cmd_info.resp[0];
		resp1 = m2_cmd_info.resp[1];		
		memory_present = (int)(((resp0>>16)&0x0008)>>3);
		number_fun = (uint8_t)(((resp0>>16)&0x0070)>>4);
//		printf("sd_get_memory_present : resp0 = 0x%x, resp1 = 0x%x\r\n", resp0, resp1);
//		printf("sd_get_memory_present : memory_present = 0x%x, number_fun = 0x%x\r\n", memory_present, number_fun);
		
		if( (resp0 == 0xffffffff) && (resp1 == 0xff) && (memory_present == 0x01) )
		{
			memory_present = SD_MEMORY_CARD;
		}
		else if(memory_present == 0x00)
		{
			if( number_fun <= 0 )
				memory_present = SD_MEMORY_UNKNOWN;
			else
				memory_present = SD_MEMORY_IO;
			break;			
		}
		else
			memory_present = SD_MEMORY_UNKNOWN;
	}
	
	return memory_present;
}

#if 0
void sd_test(void)
{
	uint32_t sd_size=0;
	uint8_t *sd_buf1, *sd_buf2;
	int i, check_len = 512;	//byte
	sd_buf1 = pvPortMalloc(check_len, GFP_DMA, MODULE_DRI_SD);
	sd_buf2 = pvPortMalloc(check_len, GFP_DMA, MODULE_DRI_SD);
	
	if(!sd_buf1 || !sd_buf2)
	{	
		printf("alloc error\n");
		goto fail;
	}
	if(sd_identify()!=SD_RTN_PASS)
	{
		printf("sd identify fail\n");
		goto fail;
	}
	sd_size = sd_get_capacity();
	printf("sd_size = %ld\n", sd_size);
	printf("data = %lx, %lx, %lx, %lx, %lx, %lx\n", *((uint32_t*)sd_buf1), *((uint32_t*)(sd_buf1+4)), *((uint32_t*)(sd_buf1+8))
													, *((uint32_t*)(sd_buf1+12)), *((uint32_t*)(sd_buf1+16)), *((uint32_t*)(sd_buf1+20)));
	sd_write(0,(uint32_t)sd_buf1,check_len>>9);
	memset((void*)sd_buf2, 0, check_len);
	sd_read((uint32_t)sd_buf2,0,check_len>>9);
	
	for(i = 0; i < check_len; i++)
	{
		if(sd_buf1[i]!=sd_buf2[i])
		{
			printf("sd rw data checking error(i=%d)\n",i);
			break;
		}
		//if((i%10)==0)
		//	printf("sd check to %d\n", i);
	}	
	if(i==check_len)
		printf("sd checking ok\n");
	//return;
	
fail:
	if(sd_buf1)
		vPortFree(sd_buf1);
	if(sd_buf2)	
		vPortFree(sd_buf2);
}

#endif
