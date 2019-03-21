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
#include "snc_spi.h"
#include "register_7320.h"


/*_____ M A C R O S ________________________________________________________*/
#define SPI_VERSION        0x73200000

typedef enum MA_SLA_MODE
{
    MasterTX 	=0,
    SlaveRX,  	
	MasterRX,	
    SlaveTX,
} ma_sla_mode;


uint8_t SPI0_MODE;
uint8_t SPI1_MODE;
uint8_t tx_count=0;
uint8_t Count0=0;

uint8_t sp0_irq_flag;
uint8_t sp1_irq_flag;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t spi0_ma_sla_mode_pri;
static uint16_t *spi0_send_buf_pri;
static uint16_t spi0_send_size_pri;
static uint16_t *spi0_rec_buf_pri;
static uint8_t *spi0_irq_flag_pri;

static uint8_t spi1_ma_sla_mode_pri;
static uint16_t *spi1_send_buf_pri;
static uint16_t spi1_send_size_pri;
static uint16_t *spi1_rec_buf_pri;
static uint8_t *spi1_irq_flag_pri;

//=============================================================================
//                  Private Function Definition
//=============================================================================



 
static SN_SPI_Type*
spi_ch_get_handle(
    spi_ch_t  spi_ch)
{
    SN_SPI_Type   *pDev = 0;
    switch(spi_ch)
    {
        case SPI_0:   pDev = (SN_SPI_Type*)SN_SPI0_BASE;          
            break;
        case SPI_1:   pDev = (SN_SPI_Type*)SN_SPI1_BASE;          
            break;    
        default:    
            break;
    }

    return pDev;
}

static void _SPI_Enable(SN_SPI_Type *pdev)
{
    pdev->CTRL0_b.SPIEN=1;
}


void SPI_Disable(spi_ch_t spi_ch)
{
		SN_SPI_Type   *pdev = 0;
		pdev=spi_ch_get_handle(spi_ch);
	  pdev->CTRL0_b.SPIEN=0;	
}

/*****************************************************************************
* Function		: SPI0_Auto_SEL_Setting
* Description	: 
* Input			: ucEn: 0/1 Enable/Disable Auto_SEL flow control
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _Init_IRQ_Info_TX(
    spi_ch_t spi_mode,
    ma_sla_mode spi_ma_sla_mode,
    uint16_t *spi_send_data_addr,
    uint16_t spi_send_size,
    uint8_t *irq_flag)
{
    if(spi_mode==SPI_0)
    {    
        spi0_ma_sla_mode_pri = spi_ma_sla_mode;
        spi0_send_buf_pri = spi_send_data_addr;
        spi0_send_size_pri = spi_send_size;     
        spi0_irq_flag_pri=irq_flag;
    }
    else
    {
        spi1_ma_sla_mode_pri = spi_ma_sla_mode;
        spi1_send_buf_pri = spi_send_data_addr;
        spi1_send_size_pri = spi_send_size;
        spi1_irq_flag_pri=irq_flag;
    }        
    
}
static void _Init_IRQ_Info_RX(
    spi_ch_t spi_mode,
    ma_sla_mode spi_ma_sla_mode,
    uint16_t * spi_receive_data_addr,
    uint16_t spi_receive_size,
    uint8_t *irq_flag)
{
    if(spi_mode==SPI_0)
    {    
        spi0_ma_sla_mode_pri = spi_ma_sla_mode;
        spi0_rec_buf_pri = spi_receive_data_addr;      
        spi0_irq_flag_pri=  irq_flag;
    }
    else
    {
        spi1_ma_sla_mode_pri = spi_ma_sla_mode;
        spi1_rec_buf_pri = spi_receive_data_addr;    
        spi1_irq_flag_pri=  irq_flag;
    }        
    
}
/*****************************************************************************
* Function		: SPI0_Auto_SEL_Setting
* Description	: 
* Input			: ucEn: 0/1 Enable/Disable Auto_SEL flow control
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Auto_SEL_Setting(SN_SPI_Type *pdev,uint8_t ucEn)
{
 
    pdev->CTRL0_b.SELDIS=ucEn&0x01;
  
}

/*****************************************************************************
* Function		: SPI0_Bus_Mode_Setting
* Description	: Setting SPI Commication Type
* Input			: ucMSSL : 0/1  Master/Slave
*             ucucTsmtMode:
*                       0x00  SPI MODE0 & MSB transmit first( CPOL:0 CPHA:0 MLSB:0 )
*                       0x01  SPI MODE1 & MSB transmit first( CPOL:0 CPHA:1 MLSB:0 )
*                       0x02  SPI MODE2 & MSB transmit first( CPOL:1 CPHA:0 MLSB:0 )
*                       0x03  SPI MODE3 & MSB transmit first( CPOL:1 CPHA:1 MLSB:0 )
*                       0x10  SPI MODE0 & LSB transmit first( CPOL:0 CPHA:0 MLSB:1 )
*                       0x11  SPI MODE1 & LSB transmit first( CPOL:0 CPHA:1 MLSB:1 )
*                       0x12  SPI MODE2 & LSB transmit first( CPOL:1 CPHA:0 MLSB:1 )
*                       0x13  SPI MODE3 & LSB transmit first( CPOL:1 CPHA:1 MLSB:1 )
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/

static void _SPI_Phase_Setting(SN_SPI_Type *pdev,spi_clock_phase_t clock_phase_Mode)
{
    pdev->CTRL1_b.CPHA=clock_phase_Mode;
}  


static void _SPI_Polarity_Setting(SN_SPI_Type *pdev,uint8_t clock_polarity_Mode)
{
    pdev->CTRL1_b.CPOL=clock_polarity_Mode;
}


static void _SPI_Bus_Mode_Setting(SN_SPI_Type *pdev,uint8_t MLSB_Mode)
{

//    if(ucTsmtMode&0x10)
    pdev->CTRL1_b.MLSB=MLSB_Mode;
//	else
//		pdev->CTRL1_b.MLSB=0;
	 
//        pdev->CTRL1_b.CPHA=ucTsmtMode&0x01;
//        pdev->CTRL1_b.CPOL=(ucTsmtMode&0x02)>>1; 
}
 
 /*****************************************************************************
* Function		: SPI0_Check_Flag
* Description	: 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static uint8_t _SPI_Check_Flag(SN_SPI_Type *pdev)
{
    return (pdev->RIS);	
}

 /*****************************************************************************
* Function		: SPI0_Clear_Flag
* Description	: 
* Input			: Clear the SPI Interrupt Flag
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Clear_Flag(SN_SPI_Type *pdev,uint8_t ucClrFlag)
 {

    pdev->IC=ucClrFlag;
  
 }
 
 /*****************************************************************************
* Function		: SPI0_Clock_Setting
* Description	: 
* Input			: ucDIV = SSPn_PCLK/SCK
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Clock_Setting(SN_SPI_Type *pdev,uint8_t ucDIV)
{

    if (ucDIV<2)
        pdev->CLKDIV=0;
    else
        pdev->CLKDIV=(ucDIV>>1)-1;

}
/*****************************************************************************
* Function		: SPI0_Data_Length_Setting
* Description	: 
* Input			: ucDL: (0~2)Reversed, (3~16)Vaild
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Data_Length_Setting(SN_SPI_Type *pdev,uint8_t ucDL)
{
    if(ucDL>2)
        pdev->CTRL0_b.DL=(ucDL-1)&0x0F;

}
/*****************************************************************************
* Function		: SPI0_Interrupt_Enable
* Description	: 
* Input			: Enable SPI Interrupt
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Interrupt_Enable(SN_SPI_Type *pdev,uint8_t ucINTEn)
{

	pdev->IE=ucINTEn;
}
/*****************************************************************************
* Function		: SPI0_LoopBack_Enable
* Description	: 
* Input			: ucLoopBack: 0/1 Disable/Enable Loop Back
* Output		: None
* Return		: None* Note			: None
*****************************************************************************/
#if 0
static void _SPI_LoopBack_Enable(SN_SPI_Type *pdev,uint8_t ucLoopBack) 
{

		pdev->CTRL0_b.LOOPBACK=ucLoopBack&0x01;
  
}
#endif 
 /*****************************************************************************
* Function		: SPI0_Master_Slave_Setting
* Description	: 
* Input			: ucMS:0/1  Master/Slave
*           : ucFIFOTH: TX/RX FIFO Threshold level(0~7) 
*           : ucSDODIS: 0/1 Disable/Enable Slave data out disable(SLAVE ONLY)
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/


static void _SPI_Master_Slave_Setting(SN_SPI_Type *pdev,uint8_t ucMS,uint8_t ucSDODIS)
{

		pdev->CTRL0_b.MS=ucMS&0x01;
		if(ucMS&0x01)
		{
			pdev->CTRL0_b.SDODIS=ucSDODIS&0x01;
		}

}
 
static void _SPI_RX_threshold_Setting(SN_SPI_Type *pdev,uint8_t ucFIFOTH)
{
    	
    pdev->CTRL0_b.RXFIFOTH=ucFIFOTH;		
  
}

static void _SPI_TX_threshold_Setting(SN_SPI_Type *pdev,uint8_t ucFIFOTH)
{
    	  		
    pdev->CTRL0_b.TXFIFOTH=ucFIFOTH;
  
}
 /*****************************************************************************
* Function		: SPI0_Read_IE
* Description	: 
* Input			: 
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static uint8_t _SPI_Read_IE(SN_SPI_Type *pdev)
{

	return pdev->IE;
  
}
/*****************************************************************************
* Function		: SPI0_Read_Status
* Description	: 
* Input			: None
* Output		: None
* Return		: Status value of SPI.
* Note			: None
*****************************************************************************/
static uint8_t _SPI_Read_Status(SN_SPI_Type *pdev)
{
	return (pdev->STAT);
}
uint8_t SPI_Read_Status(spi_ch_t spi_ch)
{
		SN_SPI_Type   *pdev = 0;
		pdev=spi_ch_get_handle(spi_ch);   
		return (pdev->STAT);
}
 /*****************************************************************************
* Function		: SPI0_Receive_Data
* Description	: 
* Input			: 
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static uint16_t _SPI_Receive_Data(SN_SPI_Type *pdev)
{
	return (pdev->DATA);
}
/*****************************************************************************
* Function		: SPI0_Reset_State
* Description	: 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Reset_State(SN_SPI_Type *pdev)
{
	pdev->CTRL0_b.FRESET=3;
  
}
/*****************************************************************************
* Function		: SPI0_SEL_Ctrl
* Description	: 
* Input			: ucSel: 0/1 Low/High Level of sel pin
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
#if 0
static void _SPI_SEL_Ctrl(SN_SPI_Type *pdev,uint8_t ucSel)
{
 
    pdev->CTRL0_b.SELCTRL=ucSel&0x01;
  
}
#endif
 /*****************************************************************************
* Function		: SPI0_Send_Data
* Description	: 
* Input			: 
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void _SPI_Send_Data(SN_SPI_Type *pdev,uint16_t usData)
{

	pdev->DATA=usData;
  
}

static void _SPI0_IRQHandler()
{
    uint16_t Rx_Data;
    uint16_t Flag_Temp;

    Flag_Temp = _SPI_Check_Flag(SN_SPI0);//check IRQ flag
    /*If Tx Fifo threshold int enabel + Tx fifo int flag = 1*/
    //TXFIFOTH interrupt
   
    if(Flag_Temp & ((1<<SPI_IRQ_TXFIFOTH) & _SPI_Read_IE(SN_SPI0)))
    {
		
			/*Master Tx*/
        if(spi0_ma_sla_mode_pri == MasterTX)
        {
                     
            if(spi0_send_size_pri<=0)
            {   
                                               
                while( !(_SPI_Read_Status(SN_SPI0) & (1<<SPI_STATUS_TXEMPTY)) );
                SN_SPI0->IE_b.TXFIFOTHIE = 0;	/*Disable Tx Fifo Int*/
                *spi0_irq_flag_pri=0;
                
            }
            else
            {          
                _SPI_Send_Data(SN_SPI0 ,*spi0_send_buf_pri);//send data                   
                spi0_send_buf_pri++;
                spi0_send_size_pri--;
                   
            
            }
             

        }
		
			/*Slave Tx*/
        if(spi0_ma_sla_mode_pri == SlaveTX)
        {
				
            while((_SPI_Read_Status(SN_SPI0) & (1<<SPI_STATUS_TXFULL)) != (1<<SPI_STATUS_TXFULL)){
				
                 if(spi0_send_size_pri<=0)
                 {
											 while( !(_SPI_Read_Status(SN_SPI0) & (1<<SPI_STATUS_TXEMPTY)) );
											 SN_SPI0->IE_b.TXFIFOTHIE = 0;	/*Disable Tx Fifo Int*/                    
											 break;
                }
                else
                {
											_SPI_Send_Data(SN_SPI0 ,*(spi0_send_buf_pri));//send data					
											spi0_send_buf_pri++;
											spi0_send_size_pri--;
                }
            }
        }

        _SPI_Clear_Flag(SN_SPI0,(1<<SPI_IRQ_TXFIFOTH));//clear TXFIFOTH IRQ flag 
	}
	/*RXFIFOTH interrupt*/
	else if(Flag_Temp & ((1<<SPI_IRQ_RXFIFOTH) & _SPI_Read_IE(SN_SPI0)))
    {
		while((_SPI_Read_Status(SN_SPI0) & (1<<SPI_STATUS_RXEMPTY)) == 0){

			/*Slave Rx Master Rx*/
			Rx_Data = _SPI_Receive_Data(SN_SPI0);
            
			if( (spi0_ma_sla_mode_pri == SlaveRX || spi0_ma_sla_mode_pri == MasterRX))
			{
                
				*(spi0_rec_buf_pri)=Rx_Data;
        spi0_rec_buf_pri++;    
					

			}
		}
		_SPI_Clear_Flag(SN_SPI0,(1<<SPI_IRQ_RXFIFOTH));//clear RXFIFOTH IRQ flag 
	}
	//RXTO interrupt
	else if(Flag_Temp & ((1<<SPI_IRQ_RXTO) & _SPI_Read_IE(SN_SPI0)))
    {
		_SPI_Clear_Flag(SN_SPI0,(1<<SPI_IRQ_RXTO));//clear RXTO IRQ flag 
	}
	//RXOVF interrupt
	else if(Flag_Temp & ((1<<SPI_IRQ_RXOVF) & _SPI_Read_IE(SN_SPI0)))
    {
		_SPI_Clear_Flag(SN_SPI0,(1<<SPI_IRQ_RXOVF));//clear RXOVF IRQ flag 
	}	
	
}


void _SPI1_IRQHandler()
{
	uint16_t Rx_Data,Flag_Temp;
	Flag_Temp = _SPI_Check_Flag(SN_SPI1);//check IRQ flag
	
	/*If Tx Fifo threshold int enabel + Tx fifo int flag = 1*/
    //TXFIFOTH interrupt
    if(Flag_Temp & ((1<<SPI_IRQ_TXFIFOTH) & _SPI_Read_IE(SN_SPI1))){
			/*Master Tx*/
        if(spi1_ma_sla_mode_pri == MasterTX)
        {
                			
            if(spi1_send_size_pri<=0)
            {   
                                               
                 while( !(_SPI_Read_Status(SN_SPI1) & (1<<SPI_STATUS_TXEMPTY)) );
                 SN_SPI1->IE_b.TXFIFOTHIE = 0;	/*Disable Tx Fifo Int*/
                          
                 *spi1_irq_flag_pri=0;
                     
            }
            else
            {
                _SPI_Send_Data(SN_SPI1 ,*spi1_send_buf_pri);//send data
           
                spi1_send_buf_pri++;
                spi1_send_size_pri--;						                       
            }
			
        }
		
			/*Slave Tx*/
        if(spi1_ma_sla_mode_pri == SlaveTX)
        {
            
            while((_SPI_Read_Status(SN_SPI1) & (1<<SPI_STATUS_TXFULL)) != (1<<SPI_STATUS_TXFULL))
            {
            
                if(spi1_send_size_pri<=0)
                {
                     while( !(_SPI_Read_Status(SN_SPI1) & (1<<SPI_STATUS_TXEMPTY)) );
                     SN_SPI1->IE_b.TXFIFOTHIE = 0;	/*Disable Tx Fifo Int*/
                    
                     break;
                }
                else
                {
                    //serial_printf("SPI1 send\n");
                    _SPI_Send_Data(SN_SPI1 ,*(spi1_send_buf_pri));//send data
                    spi1_send_buf_pri++;
                    spi1_send_size_pri--;
                }
            }
        }

            _SPI_Clear_Flag(SN_SPI1,(1<<SPI_IRQ_TXFIFOTH));//clear TXFIFOTH IRQ flag 
	}
    //RXFIFOTH interrupt
	else if(Flag_Temp & ((1<<SPI_IRQ_RXFIFOTH) & _SPI_Read_IE(SN_SPI1)))
    {
        
		while((_SPI_Read_Status(SN_SPI1) & (1<<SPI_STATUS_RXEMPTY)) == 0)
		{
                
			/*Slave Rx Master Rx*/
			 Rx_Data = _SPI_Receive_Data(SN_SPI1);
            
			if( (spi1_ma_sla_mode_pri == SlaveRX || spi1_ma_sla_mode_pri == MasterRX))
			{
                
					*(spi1_rec_buf_pri)=Rx_Data;
					spi1_rec_buf_pri++;               

			}
		}
		_SPI_Clear_Flag(SN_SPI1,(1<<SPI_IRQ_RXFIFOTH));//clear RXFIFOTH IRQ flag 
	}
	//RXTO interrupt
	else if(Flag_Temp & ((1<<SPI_IRQ_RXTO) & _SPI_Read_IE(SN_SPI1)))
    {
		_SPI_Clear_Flag(SN_SPI1,(1<<SPI_IRQ_RXTO));//clear RXTO IRQ flag 
	}
	//RXOVF interrupt
	else if(Flag_Temp & ((1<<SPI_IRQ_RXOVF) & _SPI_Read_IE(SN_SPI1)))
    {
		_SPI_Clear_Flag(SN_SPI1,(1<<SPI_IRQ_RXOVF));//clear RXOVF IRQ flag 
	}	
	
	
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 */
spi_error_code_t    
SPI_Init(spi_ch_t spi_ch,spi_mode_t spi_mode,spi_init_t *spi_init_info )     
{
 
    SN_SPI_Type   *pDev = 0;
    pDev=spi_ch_get_handle(spi_ch); 
     if( !spi_init_info )
    {
        //sn_log(LOG_ERR, "%s", "err: spi_init_info null input \n");
        return SPI_INFO_ERROR;
    }
    if(spi_ch==SPI_0)               
        SPI0_MODE=spi_mode;
    else if(spi_ch==SPI_1)
        SPI1_MODE=spi_mode;
		else
			  return SPI_INFO_ERROR;
    
    _SPI_Clock_Setting(pDev,spi_init_info->clock_div);//SSPn_PCLK / 8	
    _SPI_Auto_SEL_Setting(pDev,0);//enable Auto_SEL flow control   
    _SPI_Phase_Setting(pDev,spi_init_info->phase_sel);
    _SPI_Polarity_Setting(pDev,spi_init_info->polarity_sel);
    _SPI_Bus_Mode_Setting(pDev,spi_init_info->mlsb_sel);
    _SPI_Master_Slave_Setting(pDev,spi_mode,0);
  
    _SPI_Data_Length_Setting(pDev,spi_init_info->spi_data_length);//data length 
    _SPI_RX_threshold_Setting(pDev,spi_init_info->rx_fifo_sel);
    _SPI_TX_threshold_Setting(pDev,spi_init_info->tx_fifo_sel);
    _SPI_Enable(pDev);//enable SPI
    _SPI_Reset_State(pDev);//reset SPI
    	
    return SPI_INIT_SUCCESS;
}
    

uint32_t SPI_Write_IRQ(spi_ch_t spi_ch)
{
    if(spi_ch==SPI_0)
    {
        NVIC_ClearPendingIRQ(SPI0_IRQn);
        NVIC_EnableIRQ(SPI0_IRQn);
    } 
    else
    {
        NVIC_ClearPendingIRQ(SPI1_IRQn);
        NVIC_EnableIRQ(SPI1_IRQn);
    } 
    return 0;
}    

spi_error_code_t SPI_Write(spi_ch_t spi_ch,uint16_t *buf,uint32_t data_length,spi_irq_method_t enable_irq){
    int i;
    SN_SPI_Type   *pDev = 0;
		pDev=spi_ch_get_handle(spi_ch);
    
    if(enable_irq == 1){
       
        if(spi_ch==SPI_0) 
        {
             NVIC_SetPriority(SPI0_IRQn, 10);
            if(SPI0_MODE==SPI_MASTER)
            {
                _Init_IRQ_Info_TX(spi_ch,MasterTX,buf,data_length,&sp0_irq_flag);
            }    
            else
            {              
                _Init_IRQ_Info_TX(spi_ch,SlaveTX,buf,data_length,&sp0_irq_flag);            
            }  
                NVIC_ClearPendingIRQ(SPI0_IRQn);
                NVIC_EnableIRQ(SPI0_IRQn);            
        }    
        else
        {
           NVIC_SetPriority(SPI1_IRQn, 10);
            if(SPI1_MODE==SPI_MASTER)
            {
                _Init_IRQ_Info_TX(spi_ch,MasterTX,buf,data_length,&sp1_irq_flag);
            }    
            else
            {              
                _Init_IRQ_Info_TX(spi_ch,SlaveTX,buf,data_length,&sp1_irq_flag);            
            }  
                NVIC_ClearPendingIRQ(SPI1_IRQn);
                NVIC_EnableIRQ(SPI1_IRQn);            
        }    
        _SPI_Interrupt_Enable(pDev,(1<<SPI_IRQ_TXFIFOTH));
				return SPI_WRITE_DATA_SUCESS;
    }    
    else if(enable_irq == 0){
        for(i=0;i<data_length;i++)
        {
                 
            while(!pDev->RIS_b.TXFIFOTHIF);

            _SPI_Send_Data(pDev,buf[i]);
            _SPI_Clear_Flag(pDev,(1<<SPI_IRQ_TXFIFOTH));
         
        } 
				return SPI_WRITE_DATA_SUCESS;
    }
    return SPI_INFO_ERROR;
}

uint32_t SPI_Read_IRQ(spi_ch_t spi_ch){
    
    if(spi_ch==SPI_0){
        NVIC_ClearPendingIRQ(SPI0_IRQn);
        NVIC_EnableIRQ(SPI0_IRQn);
    } 
    else{
        NVIC_ClearPendingIRQ(SPI1_IRQn);
        NVIC_EnableIRQ(SPI1_IRQn);
    }
    return 0;    
}    


spi_error_code_t SPI_Read(spi_ch_t spi_ch,uint16_t *buf,uint32_t data_length,spi_irq_method_t enable_irq){
               
    uint32_t i;
    SN_SPI_Type   *pdev = 0;
		pdev=spi_ch_get_handle(spi_ch);   
            
    if(enable_irq == 1){
      
        if(spi_ch==SPI_0) 
        {
            NVIC_SetPriority(SPI0_IRQn, 9);
            if(SPI0_MODE==SPI_MASTER)
            {
                _Init_IRQ_Info_RX(spi_ch,MasterRX,buf,data_length,&sp0_irq_flag);
            }    
            else
            {              
                _Init_IRQ_Info_RX(spi_ch,SlaveRX,buf,data_length,&sp0_irq_flag);            
            }  
                NVIC_ClearPendingIRQ(SPI0_IRQn);
                NVIC_EnableIRQ(SPI0_IRQn);            
        }    
        else
        {
            NVIC_SetPriority(SPI1_IRQn, 9);
            if(SPI1_MODE==SPI_MASTER)
            {
                _Init_IRQ_Info_RX(spi_ch,MasterRX,buf,data_length,&sp1_irq_flag);
            }    
            else
            {              
                _Init_IRQ_Info_RX(spi_ch,SlaveRX,buf,data_length,&sp1_irq_flag);            
            }  
                NVIC_ClearPendingIRQ(SPI1_IRQn);
                NVIC_EnableIRQ(SPI1_IRQn);            
        }    
        _SPI_Interrupt_Enable(pdev,(1<<SPI_IRQ_RXFIFOTH));
				return SPI_READ_DATA_SUCCESS;
    }    
    else if(enable_irq == 0){
            /*to do by polling method*/
        
            for(i=0;i<data_length;i++){
							while((_SPI_Read_Status(pdev) & (1<<SPI_STATUS_RXEMPTY)) == 0){
								*buf=_SPI_Receive_Data(pdev); 
								buf++;
							}	
						}
						return SPI_READ_DATA_SUCCESS;
    }
    return SPI_INFO_ERROR;
           
}    
uint32_t
SPI_GetVersion(void)
{
    return SPI_VERSION;
}




void SPI0_IRQ_Handler(){
    _SPI0_IRQHandler();
}

void SPI1_IRQ_Handler(){
    _SPI1_IRQHandler();
}
