	/******************** (C) COPYRIGHT 2016 SONiX *******************************
* COMPANY:		  SONiX
* DATE:			    2016/03
* IC:			      SNC7312
* DESCRIPTION:	SPI/SSP functions.
*____________________________________________________________________________
* REVISION	     Date		       User		  Description
* 1.0		         2016/03/31	   SA2			
*
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "snc_spi_dma.h"
#include "snc_idma.h"
#include "snc_spi_nand.h"
#include "register_7320.h"

#define SPI_NAND_VERSION        0x73200000
/*_____ M A C R O S ________________________________________________________*/

/******************** (C) COPYRIGHT 2016 SONiX *******************************
* COMPANY:	SONiX
* DATE:			2017/07
* IC:				SNC7320
*____________________________________________________________________________
* REVISION	Date		User		Description
* 1.0		2017/09/18	SA2			First release
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/****************************************************************************
* Code		: ALG sample code
* Description	: 
					
***************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/


/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/




/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
static SN_SPI_DMA*
_spi_dma_ch_get_handle(
    spi_dma_sel_t spi_dma_ch)
{
    SN_SPI_DMA   *IP_SEL = 0;
    switch(spi_dma_ch)
    {
        case SPI_DMA_0:   IP_SEL = (SN_SPI_DMA*)SN_SPI0_DMA_IP_BASE; break;
        case SPI_DMA_1:   IP_SEL = (SN_SPI_DMA*)SN_SPI1_DMA_IP_BASE; break;
      

        default:    break;
    }

    return IP_SEL;
}


void SPI_DMA_Nand_Reset(spi_dma_sel_t spi_dma_sel)
{
    uint8_t cmd = SPINAND_RESET_CMD;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &cmd, SPIDMA_8BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}	

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_WriteEnable(spi_dma_sel_t spi_dma_sel)
{
		
    uint8_t cmd = SPINAND_WRITE_ENABLE_CMD;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &cmd, SPIDMA_8BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_WriteDisable(spi_dma_sel_t spi_dma_sel)
{
		
    uint8_t cmd = SPINAND_WRITE_DISABLE_CMD;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &cmd, SPIDMA_8BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t SPI_DMA_Nand_GetFeature(spi_dma_sel_t spi_dma_sel, uint8_t RegAddr)
{
		
    uint8_t Buf[2];
    Buf[0] = SPINAND_GET_FEATURE_CMD;
    Buf[1] = RegAddr;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_16BIT_TRANSFER);
    SPI_DMA_FWReceive_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_8BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
    return Buf[0];
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_SetFeature(spi_dma_sel_t spi_dma_sel, uint8_t RegAddr, uint8_t Data)
{
		
    uint8_t Buf[3];
    Buf[0] = SPINAND_SET_FEATURE_CMD;
    Buf[1] = RegAddr;
    Buf[2] = Data;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_24BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_BlockErase(spi_dma_sel_t spi_dma_sel, uint32_t BlockAddr, uint32_t PageAddr)
{
    uint32_t buf_transfer=0;
    buf_transfer = ((SPINAND_BLOCK_ERASE&0x000000FF)| (BlockAddr&0x000007FF)<<8 | (PageAddr&0x0000003F) <<19);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, (uint8_t *)&buf_transfer, SPIDMA_32BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_PageRead(spi_dma_sel_t spi_dma_sel, uint32_t BlockAddr, uint32_t PageAddr)
{
		
    uint32_t buf_transfer=0;
    buf_transfer = ((SPINAND_PAGE_READ&0x000000FF)| (BlockAddr&0x000007FF)<<8 | (PageAddr&0x0000003F) <<19);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, (uint8_t *)&buf_transfer, SPIDMA_32BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}    




/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_ProgramExecute(spi_dma_sel_t spi_dma_sel, uint32_t BlockAddr, uint32_t PageAddr)
{
		

    uint32_t buf_transfer=0;   
    buf_transfer = ((SPINAND_PROGRAM_EXECUTE&0x000000FF)| (BlockAddr&0x000007FF)<<8 | (PageAddr&0x0000003F) <<19);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, (uint8_t *)&buf_transfer, SPIDMA_32BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}



/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_ProgramLoadX1(spi_dma_sel_t spi_dma_sel, uint32_t ByteAddr, uint8_t *Data, uint32_t Len)
{
		
    uint8_t Buf[3];
    Buf[0] = SPINAND_PROGRAM_LOAD_X1;
    Buf[1] = (ByteAddr&0x0000FF00)>>8;	/*Column Address*/
    Buf[2] = (ByteAddr&0x000000FF);			/*Column Address*/
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_24BIT_TRANSFER);
    SPI_DMA_Send_Data(spi_dma_sel, SPIDMA_1IO_MODE, Data, Len);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_ProgramLoadX2(spi_dma_sel_t spi_dma_sel, uint32_t ByteAddr, uint8_t *Data, uint32_t Len)
{
		
    uint8_t Buf[3];
    Buf[0] = SPINAND_PROGRAM_LOAD_X1;
    Buf[1] = (ByteAddr&0x0000FF00)>>8;	/*Column Address*/
    Buf[2] = (ByteAddr&0x000000FF);			/*Column Address*/
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_24BIT_TRANSFER);
    SPI_DMA_Send_Data(spi_dma_sel, SPIDMA_2IO_MODE, Data, Len);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t SPI_DMA_Nand_ProgramLoadX4(spi_dma_sel_t spi_dma_sel, uint32_t ByteAddr, uint8_t *Data, uint32_t Len)
{
		
    uint8_t Buf[3];
    uint8_t SPI_NAND_Status,SPI_NAND_Config;
    uint32_t retry=1000;
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    SPI_NAND_Config |= SET_BIT0;
    SPI_DMA_Nand_WriteEnable(spi_dma_sel);
    SPI_DMA_Nand_SetFeature(spi_dma_sel, SPINAND_CONF_REG, SPI_NAND_Config);
    do{
        /*Check SPI NAND Operation Progress Bit*/
        SPI_NAND_Status = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_STA_REG);
        if(retry == 0)
            return SPINAND_FAIL;
            --retry;
    }
    while( SPI_NAND_Status & SET_BIT0 );		
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    if( !(SPI_NAND_Config&SET_BIT0) ){
        /*Fail*/
        return SPINAND_FAIL;
    }
                
                
    Buf[0] = SPINAND_PROGRAM_LOAD_X4;
    Buf[1] = (ByteAddr&0x0000FF00)>>8;	/*Column Address*/
    Buf[2] = (ByteAddr&0x000000FF);			/*Column Address*/
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_24BIT_TRANSFER);
    SPI_DMA_Send_Data(spi_dma_sel, SPIDMA_4IO_MODE, Data, Len);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
    
    /*Disable SPINAND QUAD MODE*/
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    SPI_NAND_Config &= ~SET_BIT0;
    SPI_DMA_Nand_WriteEnable(spi_dma_sel);
    SPI_DMA_Nand_SetFeature(spi_dma_sel, SPINAND_CONF_REG, SPI_NAND_Config);
    retry=1000;
    do{
        /*Check SPI NAND Operation Progress Bit*/
        SPI_NAND_Status = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_STA_REG);
        if(retry == 0)
            return SPINAND_FAIL;
            --retry;
    }
    while( SPI_NAND_Status & SET_BIT0 );
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    if( SPI_NAND_Config&SET_BIT0 ){
        /*Fail*/
        return SPINAND_FAIL;
    }
    return 0;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t SPI_DMA_Nand_ReadFromCacheX1(spi_dma_sel_t spi_dma_sel, uint32_t ByteAddr, uint8_t *Data, uint32_t Len)
{
		
    uint8_t Buf[3],status=0;
    Buf[0] = SPINAND_READ_CACHE_X1;
    Buf[1] = (ByteAddr&0x0000FF00)>>8;	/*Column Address*/
    Buf[2] = (ByteAddr&0x000000FF);			/*Column Address*/
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    if(SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_32BIT_TRANSFER)==SPIDMA_FAIL)
            return SPIDMA_FAIL;
    
    if(SPI_DMA_Receive_Data(spi_dma_sel, SPIDMA_1IO_MODE, Data, Len)==SPIDMA_FAIL)
            return SPIDMA_FAIL;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
    return status;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t SPI_DMA_Nand_ReadFromCacheX2(spi_dma_sel_t spi_dma_sel, uint32_t ByteAddr, uint8_t *Data, uint32_t Len)
{
		
    uint8_t Buf[3],status=0;
    Buf[0] = SPINAND_READ_CACHE_X2;
    Buf[1] = (ByteAddr&0x0000FF00)>>8;	/*Column Address*/
    Buf[2] = (ByteAddr&0x000000FF);			/*Column Address*/
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_32BIT_TRANSFER);
    SPI_DMA_Receive_Data(spi_dma_sel, SPIDMA_2IO_MODE, Data, Len);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
    return status;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t SPI_DMA_Nand_ReadFromCacheX4(spi_dma_sel_t spi_dma_sel, uint32_t ByteAddr, uint8_t *Data, uint32_t Len)
{
		
    uint8_t Buf[3],status=0;
    uint8_t SPI_NAND_Status,SPI_NAND_Config;
    uint32_t retry=1000;
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    SPI_NAND_Config |= SET_BIT0;
    SPI_DMA_Nand_WriteEnable(spi_dma_sel);
    SPI_DMA_Nand_SetFeature(spi_dma_sel, SPINAND_CONF_REG, SPI_NAND_Config);
    do{
        /*Check SPI NAND Operation Progress Bit*/
            SPI_NAND_Status = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_STA_REG);
            if(retry == 0)
            return SPINAND_FAIL;
            --retry;
    }
    while( SPI_NAND_Status & SET_BIT0 );
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    if( !(SPI_NAND_Config&SET_BIT0) ){
        /*Fail*/
        return SPINAND_FAIL;
    }
                    
    Buf[0] = SPINAND_READ_CACHE_X4;
    Buf[1] = (ByteAddr&0x0000FF00)>>8;	/*Column Address*/
    Buf[2] = (ByteAddr&0x000000FF);			/*Column Address*/
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_32BIT_TRANSFER);
    SPI_DMA_Receive_Data(spi_dma_sel, SPIDMA_4IO_MODE, Data, Len);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
    
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    SPI_NAND_Config &= ~SET_BIT0;
    SPI_DMA_Nand_WriteEnable(spi_dma_sel);
    SPI_DMA_Nand_SetFeature(spi_dma_sel, SPINAND_CONF_REG, SPI_NAND_Config);
    retry=1000;
    do{
        /*Check SPI NAND Operation Progress Bit*/
        SPI_NAND_Status = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_STA_REG);
        if(retry == 0)
            return SPINAND_FAIL;
            --retry;
    }
    while( SPI_NAND_Status & SET_BIT0 );
    SPI_NAND_Config = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_CONF_REG);
    if( SPI_NAND_Config&SET_BIT0 ){
        /*Fail*/
        return SPINAND_FAIL;
    }
    return status;
}
uint32_t SPINand_Read_Page(spi_dma_sel_t spi_dma_sel,uint32_t BlockAddr, uint32_t PageAddr, uint32_t ByteAddr, uint8_t read_io,uint8_t *Data, uint32_t Len)
{
  uint8_t status=0;
    uint32_t retry=10000;

        SPI_DMA_Nand_PageRead(spi_dma_sel, BlockAddr, PageAddr);
        do{
            /*Check SPI NAND Operation Progress Bit*/
            status = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_STA_REG);
                if(retry == 0)
                    return SPINAND_FAIL;
                    --retry;
        }
        while( status & SET_BIT0 );
        
        if(read_io==SPIDMA_1IO_MODE)
            status=SPI_DMA_Nand_ReadFromCacheX1(spi_dma_sel, ByteAddr, Data, Len );
        else if(read_io==SPIDMA_2IO_MODE)
            status=SPI_DMA_Nand_ReadFromCacheX2(spi_dma_sel, ByteAddr, Data, Len );
        else if(read_io==SPIDMA_4IO_MODE)
            status=SPI_DMA_Nand_ReadFromCacheX4(spi_dma_sel, ByteAddr, Data, Len );
        
        return status;
}	

uint32_t SPINand_Program_Page(spi_dma_sel_t spi_dma_sel,uint32_t BlockAddr, uint32_t PageAddr, uint32_t ByteAddr, uint8_t write_io,uint8_t *Data, uint32_t Len)
{
    uint8_t status=0;
    uint32_t retry=1000;
    SPI_DMA_Nand_WriteEnable(spi_dma_sel);
    if(write_io==SPIDMA_1IO_MODE)
        SPI_DMA_Nand_ProgramLoadX1(spi_dma_sel, ByteAddr, Data, Len );
    if(write_io==SPIDMA_2IO_MODE)
        SPI_DMA_Nand_ProgramLoadX2(spi_dma_sel, ByteAddr, Data, Len );
    if(write_io==SPIDMA_4IO_MODE)	
        status=SPI_DMA_Nand_ProgramLoadX4(spi_dma_sel, ByteAddr, Data, Len );
    
    SPI_DMA_Nand_ProgramExecute(spi_dma_sel, BlockAddr, PageAddr);
    do{
    /*Check SPI NAND Operation Progress Bit*/
        status = SPI_DMA_Nand_GetFeature(spi_dma_sel, SPINAND_STA_REG);
        if(retry == 0)
                    return SPINAND_FAIL;
                    --retry;
    }
    while( status & SET_BIT0 );
    return status;
}	
/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_ECC_Enable(spi_dma_sel_t spi_dma_sel, uint8_t NandType)
{
    SN_SPI_DMA   *pDev = 0;
    pDev = _spi_dma_ch_get_handle(spi_dma_sel);
    /*If use SPI NF Type with Ecc function , only support BIG ENDIAN*/
    SPI_DMA_Endial_Set(spi_dma_sel,SPIDMA_BIG_ENDIAN);
    
    if(spi_dma_sel == SPI_DMA_0){
        NVIC_EnableIRQ(SPI0_ECC_IRQn);
    }
    else /*if(IP_SEL == SEL_SPIDMA1)*/{
        NVIC_EnableIRQ(SPI1_ECC_IRQn);
    }
    pDev->SPI_ECC_CTRL_b.NAND_FLASH_SEL = NandType;
    pDev->SPI_ECC_CTRL_b.ECC_En = 1;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
void SPI_DMA_Nand_ECC_Disable(spi_dma_sel_t spi_dma_sel)
{
    SN_SPI_DMA   *pDev = 0;
    pDev = _spi_dma_ch_get_handle(spi_dma_sel);
    if(spi_dma_sel == SPI_DMA_0){
        NVIC_DisableIRQ(SPI0_ECC_IRQn);
    }
    else /*if(IP_SEL == SEL_SPIDMA1)*/{
        NVIC_DisableIRQ(SPI1_ECC_IRQn);
    }	
    pDev->SPI_ECC_CTRL_b.ECC_En = 0;
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint8_t SPI_DMA_Nand_ECC_IRQHandler(SN_SPI_DMA *IP_SEL, uint16_t *DataPtr, uint32_t Timeout)
{
		
    uint8_t Ecc_Blk_Cnt;
    uint16_t Ecc_Addr, Ecc_Bit;
    uint32_t Retry=0;
    //SN_SPI_DMA   *pDev = 0;
 
    //pDev = _spi_dma_ch_get_handle(spi_dma_sel);
        
    if( !IP_SEL->SPI_ECC_STATUS_b.ECC_ERROR ){
        
        /*Check Ecc Module Done*/
        while(!IP_SEL->SPI_ECC_STATUS_b.ECC_DN){
        Retry++;
        if(Retry>Timeout)
                return SPINAND_FAIL;
        }
        
        while(IP_SEL->SPI_ECC_STATUS_b.ECC_REQ){

            /*Read Ecc Addr First*/
            Ecc_Blk_Cnt = IP_SEL->SPI_ECC_ADDR_b.SPI_ECC_BLK_CNT;/*Block Cnt*/
            Ecc_Addr = IP_SEL->SPI_ECC_ADDR_b.SPI_ECC_ADDR;	/*Word Addr*/
            
            /*Read Ecc Data*/
            Ecc_Bit = IP_SEL->SPI_ECC_DATA_b.SPI_ECC_DATA;/*Errorr Bit*/
            
            /*ECC Correct*/
            DataPtr[Ecc_Addr+(256*Ecc_Blk_Cnt)] ^= Ecc_Bit;/*Error Correct*/
            
        }
        
        return 0;
    }
    else{
        return 1;
    }
}

/*****************************************************************************
* Function		: 
* Description	: 
* Input			: None
* Output		: 
* Return		: None
* Note			: None
*****************************************************************************/
uint32_t SPI_DMA_Nand_ReadID(spi_dma_sel_t spi_dma_sel)
{
		
    uint8_t Buf[3];
    uint32_t ID;
    Buf[0] = SPINAND_READ_ID;
    Buf[1] = 0x00;
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_LOW);
    SPI_DMA_FWSend_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_16BIT_TRANSFER);	/*1Byte Cmd+1Byte Dummy*/
    SPI_DMA_FWReceive_Data(spi_dma_sel, SPIDMA_1IO_MODE, &Buf[0], SPIDMA_16BIT_TRANSFER);
    SPI_DMA_FWSet_CS(spi_dma_sel, SPIDMA_CS_HIGH);
    ID = (Buf[0]<<8)+Buf[1];
    return ID;
}


uint32_t
SPI_NAND_GetVersion(void)
{
    return SPI_NAND_VERSION;
}


void spinand_irq(uint16_t *DataPtr)
{
    SN_GPIO0->DATA_b.DATA2 ^= 1;
    SN_GPIO0->DATA_b.DATA2 ^= 1;
	if( SPI_DMA_Nand_ECC_IRQHandler(SEL_SPIDMA0, DataPtr, 0xFFFF ) != 0 ){
		/*Correct Fail*/
		while(1);
	}
}    









