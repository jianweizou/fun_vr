/**
 * @file
 * this is sd register control file
 * sdv2.c
 * @author CJ
 */

#include <stdint.h>
//#include <bsp.h>
#include "sdv2.h"

#define outl(addr, value)       (*((volatile unsigned int *)(addr)) = value)
#define inl(addr)                (*((volatile unsigned int *)(addr)))
	
/*SD CTL APIs	*/
// =========================================================================
//	MS mode selection
//	00: GPIO mode
//	01: SD card SPI mode
//	10: SD card SD mode
// =========================================================================
void sdSetMsMode(uint32_t mode)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_MODE_MASK); 
	outl((SD_CTL), data | mode);
}

// =========================================================================
//      MS R/W Select 1: read mode , 0: write mode
// =========================================================================
void sdMsRegRWSwitch(uint32_t value)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_REGRW_MASK);
	data |=(value << SD_CTL_REGRW_BIT);
	outl((SD_CTL), data);
}

// =========================================================================
//       1: enable DMA , 0: disable DMA
// =========================================================================
void sdMsDmaENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_DMAEN_MASK);
	data |=(isEnable << SD_CTL_DMAEN_BIT);
	outl((SD_CTL), data);
}

// =========================================================================
//       DMA Select R/W ,1: read mode , 0: write mode
// =========================================================================
void sdMsDmaRWSwitch(uint32_t value)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_DMARW_MASK);
	data |=(value << SD_CTL_DMARW_BIT);
	outl((SD_CTL), data);
}

// =========================================================================
//       1: enable Extra ECC DATA enable , 0: disable(nand flash)
// =========================================================================
void sdExtraENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_EXTRAEN_MASK);
	data |=(isEnable << SD_CTL_EXTRAEN_BIT);
	outl((SD_CTL), data);
}
/*[isEnable] 1: enable ECC or CRC, 0: disable*/
// =========================================================================
//         Set ECC/CRC write '0' will be clear
// =========================================================================
void sdEccENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_ECCEN_MASK);
	data |=(isEnable << SD_CTL_ECCEN_BIT);
	outl((SD_CTL), data);
}

/*return 1: ready*/
// =========================================================================
//         MS is ready for r/w
// Bug: delay 5 cycles of AHB2 clock for HW updating MS_RDY bit.
// =========================================================================
uint32_t sdCheckMsRdy(void)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= SD_CTL_MSRDY_MASK;
	data =(data >> SD_CTL_MSRDY_BIT);

	return data;
}

/*[isEnable] 1:trigger*/
// =========================================================================
//         Trigger to polling the and of busy for SD card
// =========================================================================
void sdSetSpiBusyTri(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_SPIBUSYTRI_MASK);
	data |=(isEnable << SD_CTL_SPIBUSYTRI_BIT);
	outl((SD_CTL), data);
}

/*[isEnable] 1:trigger*/
// =========================================================================
//         Trigger to send command 
// =========================================================================
void sdSetSpiCmdTri(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_SPICMDTRI_MASK);
	data |=(isEnable << SD_CTL_SPICMDTRI_BIT);
	outl((SD_CTL), data);
}

// =========================================================================
//         Read data Command
// =========================================================================
void sdReadDataCmd(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_RAEDDATACMD_MASK);
	data |=(isEnable << SD_CTL_RAEDDATACMD_BIT);
	//data = 0xfff001e5;
	outl((SD_CTL), data);
}

// =========================================================================
//         Set MS DMA Speed
// =========================================================================
void sdSetMsSpeed(uint32_t msspeed)
{
	uint32_t data;
	data = inl((SD_CTL));
	data &= ~(SD_CTL_MSSPEED_MASK);
	data |=(msspeed << SD_CTL_MSSPEED_BIT);
	outl((SD_CTL), data);
}

/*SD DMA_SIZE APIs*/
// =========================================================================
//         Set MS DMA Size
// =========================================================================
void sdSetBlkSize(uint32_t size)
{
	uint32_t data;
	data = inl((SD_DMA_SIZE));
	data &= ~(SD_DMASIZE_MSDMASIZE_MASK);
	data |=((size - 1) << SD_DMASIZE_MSDMASIZE_BIT);
	outl((SD_DMA_SIZE), data);
}

/*SD CRC1 APIs*/
// =========================================================================
//         Read CRC16_0
// =========================================================================
uint32_t sdReadCrc16_0(void)
{
	uint32_t data;
	data = inl((SD_CRC1));
	data &=(SD_CRC1_CRC16_0_L_MASK | SD_CRC1_CRC16_0_H_MASK);
	data =(data >> SD_CRC1_CRC16_0_L_BIT);

	return data;
}

// =========================================================================
//         Read CRC7
// =========================================================================
uint32_t sdReadCrc7(void)
{
	uint32_t data;
	data = inl((SD_CRC1));
	data &= SD_CRC1_CRC7_MASK;
	data =(data >> SD_CRC1_CRC7_BIT);

	return data;
}

/*SD CRC APIs*/
// =========================================================================
//         Read CRC16_1
// =========================================================================
uint32_t sdReadCrc16_1(void)
{
	uint32_t data;
	data = inl((SD_CRC2));
	data &= SD_CRC2_CRC16_1_MASK;
	data =(data >> SD_CRC2_CRC16_1_BIT);

	return data;
}

// =========================================================================
//         Read CRC16_2 & CRC16_3
// =========================================================================
uint32_t sdReadCrc16_2and16_3(void)
{
	uint32_t data;
	data = inl((SD_CRC3));
	data &=(SD_CRC2_CRC16_3_MASK | SD_CRC2_CRC16_2_MASK);
	data =(data >> SD_CRC2_CRC16_2_BIT);

	return data;
}

/*SD MS_IO_I APIs*/
// =========================================================================
//         Read MS_IO Input Data
// =========================================================================
uint32_t sdMsIOI(void)
{
	uint32_t data = 0;

	data = inl((SD_MS_IO_I));
	data &=(SD_MS_IO_I_I_MASK);
	data =(data >> SD_MS_IO_I_I_BIT);

	return data;
}

void sdReadGpioIn(uint32_t num)
{
	uint32_t data = 0;

	data = inl((SD_MS_IO_I));
	data &=(1 << num);
	data >>= num;
//	printf("sdReadGpioIn : num = %d, data = 0x%x\r\n", num, data);
}

/*SD MS_IO_O APIs*/
// =========================================================================
//         set MS_IO8 Output Data
// =========================================================================
void sdMsIOO_8(uint32_t value)
{
	uint32_t data = 0;
	
	data = inl((SD_MS_IO_O));
	data &= ~(SD_MS_IO_O_O_MASK);
	data |=(value << SD_MS_IO_O_O_BIT);
	outl((SD_MS_IO_O), data);
}

void sdWriteGpioOut(uint32_t gpio_num, uint32_t sig)
{
	uint32_t data = 0;

	data = inl((SD_MS_IO_O));
	data &= ~SD_MS_IO_O_O_MASK;

	if(sig == 1)		// Siganl is high.
	{
		data |=(1 << gpio_num);
	}
	else if(sig == 0)	// Signal is low.
	{
		data |= SD_MS_IO_O_O_MASK;
		data &= ~(1 << gpio_num);
	}

	outl((SD_MS_IO_O), data);
}

/*SD MS_IO_OE APIs*/
// =========================================================================
//	set MS_IO8 Output Enable
//	[OE:0]: input; [OE:1]: output
// =========================================================================
void sdMsIOOE_8(uint32_t value)
{
	uint32_t data = 0;

	data = inl((SD_MS_IO_OE));
	data &= ~(SD_MS_IO_OE_OE_MASK);
	data |=(value << SD_MS_IO_OE_OE_BIT);
	outl((SD_MS_IO_OE), data);
}

/*SD SPI_CMD APIs*/
// =========================================================================
//         Write command for SPI/SD command token
// =========================================================================
void sdWriteSpiCmd(uint32_t cmd)
{
	outl((SD_SPI_CMD), cmd);	
}

// =========================================================================
//         Read response 1 ~ 4 for SPI command token
// =========================================================================
uint32_t sdReadSpiCmd(void)
{
	return inl((SD_SPI_CMD));
}

/*SD SPI_INDEX APIs*/
// =========================================================================
//         Write Index for SPI/SD command token
// =========================================================================
void sdWriteSpiIndex(uint32_t index)
{
	uint32_t data;
	data = inl((SD_SPI_INDEX));
	data &= ~(SD_SPI_INDEX_INDEX_MASK);
	data |=(index << SD_SPI_INDEX_INDEX_BIT);
	outl((SD_SPI_INDEX), index);	
}

// =========================================================================
//         Read response 0 for SPI/SD command token
// =========================================================================
uint32_t sdReadSpiIndex(void)
{
	uint32_t data;
	data = inl((SD_SPI_INDEX));
	data &=(SD_SPI_INDEX_INDEX_MASK);
	data =(data >> SD_SPI_INDEX_INDEX_BIT);

	return data;
}

/*SD DMA_BLKSU APIs*/
// =========================================================================
//         Succeed DMA block number at Multiple DMA
// =========================================================================
void sdDmaBlock(uint32_t dmaBlockNum)
{
	uint32_t data;
	data = inl((SD_DMA_BLKSU));
	data &= ~(SD_BLKSU_BLK_MASK);
	data |=((dmaBlockNum - 1) << SD_BLKSU_BLK_BIT);
	outl((SD_DMA_BLKSU), data);
}

// =========================================================================
//         Succeed DMA block transfer number at every DMA
// =========================================================================
uint32_t sdReadSUDmaBlock(void)
{
	uint32_t data;
	data = inl((SD_DMA_BLKSU));
	data &= SD_BLKSU_SUBLK_MASK;
	data =(data >> SD_BLKSU_SUBLK_BIT);

	return data;
}

/*SD DMA_TMCNT APIs*/
// =========================================================================
//         Set Time out counter
// =========================================================================
void sdSetTimeCount(uint32_t timeCnt)
{
	uint32_t data;
	data = inl((SD_TMCNT));
	data &= ~(SD_TMCNT_TMCNT_MASK);
	data |=(timeCnt << SD_TMCNT_TMCNT_BIT);
	outl((SD_TMCNT), data);
}

/*SD MDMAECC APIs*/
// =========================================================================
//         Enable sd M-DMA 
// =========================================================================
void sdMsMDmaENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_MS_M_DMA_EN_MASK);
	data |=(isEnable >> SD_MS_M_DMA_EN_BIT);
	outl((SD_MDMAECC), data);
}

// =========================================================================
//         check sd M-DMA finish and not timeout
// =========================================================================
uint32_t sdCheckMsMDmaOkOnTime(void)
{
	int				ret;
	uint32_t 	data, chkbit;
	
	data = inl((SD_MDMAECC));
	chkbit = data & (SD_MS_M_DMA_OK_MASK | SD_MS_M_DMA_TIME_OUT_MASK);
	if (chkbit == SD_MS_M_DMA_OK_MASK)
		return MDMA_OK_ON_TIME;
	else if (chkbit == SD_MS_M_DMA_TIME_OUT_MASK)
		ret = MDMA_FAIL_TIME_OUT;
	else if (chkbit == (SD_MS_M_DMA_OK_MASK | SD_MS_M_DMA_TIME_OUT_MASK))
		ret = MDMA_OK_TIME_OUT;
	else
		ret = MDMA_FAIL_ON_TIME;

	data &= ~(SD_CLR_MDMA_TIMEOUT_MASK);
	data |=(1 << SD_CLR_MDMA_TIMEOUT_BIT);
	outl((SD_MDMAECC), data);
	return ret;
}

// =========================================================================
//    Write data block response err. (For SD Card Write Data Block only)
// =========================================================================
uint32_t sdCheckWriteErr(void)
{
    uint32_t data;
    data = inl((SD_MDMAECC));
    data &= SD_MDMAECC_CRC_W_MASK;
    data =(data >> SD_MDMAECC_CRC_W_BIT);

    return data;
}

// =========================================================================
//    Clear CRC_W_ERR2 flag
// =========================================================================
void sdClearCRCWErrFlag(uint32_t value)
{
		uint32_t data;
		data = inl((SD_MDMAECC));
		data &= ~(SD_CLR_W_ERR_MASK);
		data |=(value << SD_CLR_CRC_W_BIT);
		outl((SD_MDMAECC), data);
}


// =========================================================================
//     ECC error correct has error after DMA transfer or CRC16 error in SD
// =========================================================================
uint32_t sdCheckCrcErr(void)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= SD_MDMAECC_CRCERR_MASK;
	data =(data >> SD_MDMAECC_CRCERR_BIT);

	return data;
}

// =========================================================================
//         Enable Ready interrupt
// =========================================================================
void sdMsRdyIntrENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_MS_RDY_INTR_EN_MASK);
	data |=(isEnable << SD_MS_RDY_INTR_EN_BIT);
	outl((SD_MDMAECC), data);
}

// =========================================================================
//         Read SD DMA is complet flag
// =========================================================================
uint32_t sdCheckMsRdyFlag(void)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= SD_MS_RDY_FLAG_MASK;
	data =(data >> SD_MS_RDY_FLAG_BIT);
	
	return data;
}

// =========================================================================
//         Clear sd ready flag
// =========================================================================
void sdClearMsRdyFlag(uint32_t value)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_CLR_MS_RDY_MASK);
	data |=(value << SD_CLR_MS_RDY_BIT);
	outl((SD_MDMAECC), data);
} 

// =========================================================================
//         Enable MS error interrupt(for AHB)
// =========================================================================
void sdMsErrIntrENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_MS_ERR_INTR_EN_MASK);
	data |=(isEnable << SD_MS_ERR_INTR_EN_BIT);
	outl((SD_MDMAECC), data);
}

// =========================================================================
//         Read error occurs on AHB
// =========================================================================
uint32_t sdCheckMsErrFlag(void)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= SD_MS_ERR_FLAG_MASK;
	data =(data >> SD_MS_ERR_FLAG_BIT);

	return data;
}

// =========================================================================
//         Clear sd error flag
// =========================================================================
void sdClearMsErrFlag(uint32_t value)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_CLR_MS_ERR_MASK);
	data |=(value << SD_CLR_MS_ERR_BIT);
	outl((SD_MDMAECC), data);
}

// =========================================================================
//         Enable SD Detect interrupt
// =========================================================================
void sdCdIntrENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_DETECT_INTR_EN_MASK);
	data |=(isEnable << SD_DETECT_INTR_EN_BIT);
	outl((SD_MDMAECC), data);
}

// =========================================================================
//         Read SD card Detect flag
// =========================================================================
uint32_t sdCdIntrFlag(void)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= SD_DETECT_FLAG_MASK;
	data =(data >> SD_DETECT_FLAG_BIT);

	return data;
}

// =========================================================================
//         Clear SD_DETECT flag
// =========================================================================
void sdClearCdIntrFlag(uint32_t value)
{
	uint32_t data;
	data = inl((SD_MDMAECC));
	data &= ~(SD_CLR_DETECT_MASK);
	data |=(value << SD_CLR_DETECT_BIT);
	outl((SD_MDMAECC), data);
} 

/*SD LBA APIs*/
// =========================================================================
//         Set Response byte number
// =========================================================================
void sdSetLba(uint32_t value)
{
	uint32_t data;
	data = inl((SD_LBA));
	data &= ~(SD_LBA_LBAW_MASK);
	data |=(value << SD_LBA_LBAW_BIT);
	outl((SD_LBA), data);
}

/*SD DMA_ADDR APIs*/
// =========================================================================
//         Set DMA Address
// =========================================================================
void sdSetDmaAddr(uint32_t addr)
{
	outl((SD_DMA_ADDR), addr);
}

/*SD 0x70~0x7c APIs*/
// =========================================================================
//         Mass Stoarge(SD mode) write command to trigger	0x70
// =========================================================================
void sdWriteCommand(uint32_t cmd)
{
	outl((SD_REG_CMD), cmd);
}

// =========================================================================
//         Mass Stoarge(SD mode) read response 		0x70
// =========================================================================
uint32_t sdReadCommand(void)
{
	return inl((SD_REG_CMD));
}

// =========================================================================
//         Mass Stoarge(SD mode) write Data 			0x74
// =========================================================================
void sdWriteData(uint32_t value)
{
	outl((SD_REG_DATA), value);
}

// =========================================================================
//         Mass Stoarge(SD mode) Read Data 			0x74
// =========================================================================
uint32_t sdReadData(void)
{
	return inl((SD_REG_DATA));
}

// =========================================================================
//         Mass Stoarge(SD mode) send dummy clock		0x78
// =========================================================================
void sdWriteDummyClock(uint32_t value)
{
	outl((SD_REG_DUMMYCLOCK), value);
}

// =========================================================================
//         Mass Stoarge(SD mode) read response to SPI_CMD	0x7c
// =========================================================================
void sdWriteAutoResponse(uint32_t value)
{
	outl((SD_AUTO_RESPONSE), value);
}

// =========================================================================
//         Mass Stoarge(SD mode) reset function
// =========================================================================
void sdReset(void)
{
	outl((SD_CTL), 0);
	outl((SD_CTL), SD_CTL_MODE_GPIO);
	outl((SD_CTL), SD_CTL_MODE_SD);
	outl((SD_MDMAECC), \
	(SD_CLR_MS_RDY_MASK | SD_CLR_MS_ERR_MASK \
	| SD_CLR_ECC_ERR_MASK | SD_CLR_DETECT_MASK \
	| SD_CLR_MDMA_TIMEOUT_MASK | SD_CLR_W_ERR_MASK));
}

// =========================================================================
//       SDIO Enable 1: enable SDIO, 0: disable SDIO
// =========================================================================
void sdioENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_SDIO));
	data &= ~(SDIO_EN_MASK);
	data |=(isEnable << SDIO_EN_BIT);
	outl((SD_SDIO), data);
}

// =========================================================================
//       SDIO Interrupt Enable 1: enable SDIO, 0: disable SDIO
// =========================================================================
void sdioIntrENSwitch(uint32_t isEnable)
{
	uint32_t data;
	data = inl((SD_SDIO));
	data &= ~(SDIO_INTR_EN_MASK);
	data |=(isEnable << SDIO_INTR_EN_BIT);
	outl((SD_SDIO), data);
}

// =========================================================================
//         Read SD DMA is complet flag
// =========================================================================
uint32_t sdCheckIntrFlag(void)
{
    uint32_t data;
    data = inl((SD_SDIO));
    data &= SDIO_INTR_FLAG_MASK;
    data =(data >> SDIO_INTR_FLAG_BIT);

    return data;
}

// =========================================================================
//         Clear sd ready flag
// =========================================================================
void sdClearIntrFlag(uint32_t value)
{
    uint32_t data;
    data = inl((SD_SDIO));
    data &= ~(SDIO_CLR_INTR_FLAG_MASK);
    data |=(value << SDIO_CLR_INTR_FLAG_BIT);
    outl((SD_SDIO), data);
}


// =========================================================================
//       SDIO FUNC Set
// =========================================================================
void sdioFuncSet(uint32_t Value)
{
	uint32_t data;
	data = inl((SD_SDIO));
	data &= ~(SDIO_FUNC_MASK);
	data |=(Value << SDIO_FUNC_BIT);
	outl((SD_SDIO), data);
}

// =========================================================================
//       SDIO OP Code Set
// =========================================================================
void sdioOPCodeSet(uint32_t Value)
{
	uint32_t data;
	data = inl((SD_SDIO));
	data &= ~(SDIO_OP_CODE_MASK);
	data |=(Value << SDIO_OP_CODE_BIT);
	outl((SD_SDIO), data);
}

// =========================================================================
//       SDIO Block mode Set
// =========================================================================
void sdioBLKmodeSet(uint32_t Value)
{
	uint32_t data;
	data = inl((SD_SDIO));
	data &= ~(SDIO_BLK_MODE_MASK);
	data |=(Value << SDIO_BLK_MODE_BIT);
	outl((SD_SDIO), data);
}

// =========================================================================
//       SDIO reg address Set
// =========================================================================
void sdioRegAddrSet(uint32_t Value)
{
	uint32_t data;
	data = inl((SD_SDIO_ADDR));
	data &= ~(SDIO_REG_ADDR_MASK);
	data |=(Value << SDIO_REG_ADDR_BIT);
	outl((SD_SDIO_ADDR), data);
}
