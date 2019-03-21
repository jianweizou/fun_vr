
#include <stdio.h>
#include <string.h>

#include "snc_codec.h"
#include "snc_i2c.h"
#include "snc_i2s4.h"
#include "aud_driver.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define AUD_MS_DELAY(x)     Delay(x);//vTaskDelay(x);//Delay(x);

typedef enum aud_driver_modde
{
    AUD_NU = 0,
    AUD_01,
    AUD_NU_SW,
    AUD_01_SW,
}aud_driver_modde_t;

typedef enum aud01_ldo18_bandgap
{
    AUD01_LDO18_BANDGAP_ON = 0,
    AUD01_LDO18_BANDGAP_OFF,
}aud01_ldo18_bandgap_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
//static i2c_init_info_t I2C;
static AUD_RegBit_s AUD_Reg;
static aud_driver_modde_t aud_driver_mode =AUD_01_SW; //AUD_01;
static i2c_id_t aud_driver_i2c;
//=============================================================================
//                  Private Function Definition
//=============================================================================
//static void Delay(uint16_t TIME)
//{
//	#define m_1mS_COUNT	6000
//	//SYSCLK=96MHz	
//	uint16_t i,j;
//	uint16_t temp;			

//	temp=SN_SYS0->SYS_CLKCFG_b.STAT;
//	
//	//SYS_CLK Source
//	switch(temp){
//		case 0: //0:IHRC=12M
//		case 2: //2:HXTAL=12M
//			temp=m_1mS_COUNT>>3;						
//			break;
//		case 1: //1:ILRC=32K
//		case 3: //3:LXTAL=32K
//			temp=m_1mS_COUNT/3000;						
//			break;
//		case 4: //4:SPLL=96M
//			temp=m_1mS_COUNT;			
//			break;
//		default:
//			temp=m_1mS_COUNT>>3;
//			break;
//	}
//	
//	//SYS_CLK DIV
//	i=SN_SYS0->SYS_CLKDIV;
//	while(i){
//		i--;
//		temp=temp>>1;
//	}		
//	
//	while(TIME){
//		for(i=0;i<temp;i++){ //6000 *16=96Mhz
//				//++: 2T
//				//for:8T
//				j++;
//				j++;
//				j++;
//				j++;
//				j++;
//		}	
//		TIME--;
//	}
//}
void Delay(u16 t)
{
    unsigned int i,j;
    for (j = 0; j < t; j++){
        for (i = 0; i <= 0x12C0; i++){
        }
    }
}

static uint16_t bswap_16(uint16_t __x)
{
	return ((__x<<8) | (__x>>8));
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
AUD_ret_t AUD_RegWrite(uint8_t RegAddr, uint16_t Data)
{
    uint8_t ret, ByteWrite[2];

    switch (aud_driver_mode)
    {
        case AUD_NU:
            ByteWrite[0] = ((RegAddr<<1)&0xFE)|((Data)>>8);
            ByteWrite[1] = ((uint8_t)Data);
            ret = I2C_Master_Send( aud_driver_i2c, AUDL_ID, ByteWrite, 2, false ); 
            I2C_Set_Stop_Condition( aud_driver_i2c );
            break;
        case AUD_01:
            ByteWrite[0] = RegAddr;
            ByteWrite[1] = ((uint8_t)Data);
            ret = I2C_Master_Send( aud_driver_i2c, AUD01_ID, ByteWrite, 2, false );    
            I2C_Set_Stop_Condition(aud_driver_i2c);
            break;
        case AUD_NU_SW:
            ByteWrite[0] = ((RegAddr<<1)&0xFE)|((Data)>>8);
            ByteWrite[1] = ((uint8_t)Data);
            ret = I2C_SoftWare_Master_Write(AUDL_ID, ByteWrite, 2);
            I2C_Stop();
            break;
        case AUD_01_SW:
            ByteWrite[0] = RegAddr;
            ByteWrite[1] = ((uint8_t)Data);
            ret = I2C_SoftWare_Master_Write( AUD01_ID, ByteWrite, 2 );    
            I2C_Stop();
            break;
    }
    
    if( ret == I2C_SUCCESS)
    {
        return AUD_PASS;
    }
    else
    {
        return AUD_FAIL;
    }
}


AUD_ret_t AUD_RegRead(uint8_t RegAddr, uint16_t *Data)
{
    uint8_t ret, ByteWrite[2];
    uint8_t temp;

    switch (aud_driver_mode)
    {
        case AUD_NU:
            ret = I2C_Master_Send( aud_driver_i2c, AUDL_ID, &ByteWrite[0], 1, false); 
            ret = I2C_Master_Recv( aud_driver_i2c, AUDL_ID, (uint8_t*)Data, 2, false);
            I2C_Set_Stop_Condition( aud_driver_i2c );
            break;
        case AUD_01:
            temp = *(uint8_t*)Data;
            ByteWrite[0] =  RegAddr;
            ret = I2C_Master_Send( aud_driver_i2c, AUD01_ID, &ByteWrite[0], 1, false );
            ret = I2C_Master_Recv( aud_driver_i2c, AUD01_ID, (uint8_t*)&temp, 1, false );
            I2C_Set_Stop_Condition(aud_driver_i2c);
            *Data = (uint16_t)temp;
            break;
        case AUD_NU_SW:
            ByteWrite[0] = (RegAddr<<1)&0xFE;
            ret = I2C_SoftWare_Master_Write(AUDL_ID, ByteWrite, 1);
            ret = I2C_SoftWare_Master_Read(AUDL_ID, (uint8_t*)Data, 2);
            I2C_Stop();
            *Data = bswap_16(*Data);
            break;
        case AUD_01_SW:
            temp = *(uint8_t*)Data;
            ByteWrite[0] =  RegAddr;
            ret = I2C_SoftWare_Master_Write( AUD01_ID, &ByteWrite[0], 1);
            ret = I2C_SoftWare_Master_Read( AUD01_ID, (uint8_t*)&temp, 1);
            I2C_Stop();
            *Data = (uint16_t)temp;
            break;
    }
    
    if( ret == I2C_SUCCESS)
    {
        return AUD_PASS;
   }
    else
    {
        return AUD_FAIL;
    }
}

void AUD_mclk_enable(mclk_pad_output_t sel)
{
//    *(uint32_t*)(0x45000100) |= SET_BIT14;
//    *(uint32_t*)(0x45000100) |= SET_BIT13;
//    *(uint32_t*)(0x40034000) = 0x8000; /* I2S4 */
//    //*(uint32_t*)(0x40034004) = 0x8080; /* force xtal as mclk */
//    I2S4_MCLK_OUT0_OUT1_SEL();
    
	uint32_t temp_reg_mclk;
	uint32_t temp_reg=0;

    if(sel == MCLK_OUTPUT_I2S4){
        temp_reg = 0x0000|SET_BIT18|SET_BIT6;
    }
    else if(sel == MCLK_OUTPUT_XTAL){
        temp_reg = 0x0000|SET_BIT19|SET_BIT7;
    }
    else /*if(sel == MCLK_OUTPUT_IHRC)*/{
        temp_reg = 0x0000;
    }
        
//	/* Set MCLK1 = XTAL */
//	temp_reg |=SET_BIT19;
//	temp_reg &= ~SET_BIT18;
//	/* Set MCLK2 = XTAL */
//	temp_reg |=SET_BIT7;
//	temp_reg &= ~SET_BIT6;

	temp_reg_mclk = *(uint32_t*)(0x40034004);
	temp_reg_mclk |= temp_reg;
	*(uint32_t*)(0x40034004) = temp_reg_mclk;
}

void AUD_mclk_disable(void)
{
    *(uint32_t*)(0x45000100) &= ~SET_BIT14;
    *(uint32_t*)(0x45000100) &= ~SET_BIT13;
}

AUD_ret_t AUD_Driver_Init(i2c_id_t i2c_id)
{
    volatile uint32_t ret=0;
    i2c_init_info_t I2C;

/* ============Disable 7320 mclk pad clk to aud============ */
//    AUD_mclk_disable();

    switch (aud_driver_mode)
    {
        case AUD_NU:
        case AUD_01:
        /* ============USE HW I2C INIT============ */
         //   slave_addr_t* slave_addr;
            /* I2C IP Initial */
            aud_driver_i2c = i2c_id;
            
            I2C.speed.SCLH = 200;
            I2C.speed.SCLL = 200;
            
            I2C.i2c = i2c_id;
            I2C.time_out = 0;
            
            I2C.addr_mode = MODE_7BIT;
            I2C.slave_addr.ADDR0 = 0x01;
            I2C.slave_addr.ADDR1 = 0x01;
            I2C.slave_addr.ADDR2 = 0x01;
            I2C.slave_addr.ADDR3 = 0x01;
            I2C.gcen = false;
            I2C.i2c_irq = false;
            I2C.i2c_id_txrx_mode = NONE;
            I2C.i2c_fifo_mode_en = false;
            I2C.fifo_mode_para.FIFO_THRESHLOD = FIFO_THRESHOLD_2;
            I2C.fifo_mode_para.FIFO_ACT = REPLY_NACK;

            ret = I2C_Initial(&I2C);
        //    slave_addr_setting(slave_addr);
        //		I2C_Default_Init(I2C_0,slave_addr);
        //		I2C_Default_Init(I2C_0,slave_addr);

            break;
        
        case AUD_NU_SW:
        case AUD_01_SW:
        /* ============USE SW I2C INIT============ */
            I2C_ch_sel((sn_i2c_sw_ch_t)i2c_id);
            I2C_SoftWare_Master_Init();
            break;
    }

    if(ret==I2C_SUCCESS)
    {
        return AUD_PASS;
    }
    else
    {
        return AUD_FAIL;
    }
}


AUD_ret_t AUD_SlaveMode_Init(void) /*More detail register bit description please refer to AUD spec*/
{
    uint8_t ret;
    uint16_t rdata,wdata;
    
    /* Register Map Initial */
    memset( (uint8_t*)&AUD_Reg, 0, sizeof(AUD_Reg) );
    
    /* Software Reset */
    AUD_Reg.Software_Reset.Word = 0x00; /* Any write operation to this register resets all registers to default values */
    ret = AUD_RegWrite( Software_Reset, AUD_Reg.Software_Reset.Word );  
    
    /* Spec provide power up */
     Delay(1);// cc
    ret = AUD_RegWrite( Output_Control, SET_BIT2|SET_BIT3|SET_BIT4 );
    
    Delay(1);// cc
    AUD_Reg.Power_Management1.Word = 0x0104;
    ret = AUD_RegWrite( Power_Management1, AUD_Reg.Power_Management1.Word );
    
    AUD_Reg.Power_Management1.Word = 0x010D;
    ret = AUD_RegWrite( Power_Management1, AUD_Reg.Power_Management1.Word );
    ret = AUD_RegRead( Power_Management1, &rdata );
    Delay(1);
    
    /* Power Management */
    AUD_Reg.Power_Management1.Word = 0x01FF;    /*turn on all*/
    ret = AUD_RegWrite( Power_Management1, AUD_Reg.Power_Management1.Word );
    ret = AUD_RegRead( Power_Management1, &rdata );
    if(rdata!=AUD_Reg.Power_Management1.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Power_Management2.Word = 0x01BF;    /*turn on all*/
    ret = AUD_RegWrite( Power_Management2, AUD_Reg.Power_Management2.Word );
    ret = AUD_RegRead( Power_Management2, &rdata );
    if(rdata!=AUD_Reg.Power_Management2.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
  
    AUD_Reg.Power_Management3.Word = 0x01EF;    /*turn on all*/
    ret = AUD_RegWrite( Power_Management3, AUD_Reg.Power_Management3.Word );
    ret = AUD_RegRead( Power_Management3, &rdata );
    if(rdata!=AUD_Reg.Power_Management3.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    /* General Audio Control */
    AUD_Reg.Audio_Interface.Word = 0x0050;      /*default*/
    AUD_Reg.Audio_Interface.Bit.BCLKP = 0;      /*0= Normal Phase*/
    AUD_Reg.Audio_Interface.Bit.LRP = 0;        /*0 = Normal Phase Operation*/
    AUD_Reg.Audio_Interface.Bit.WLEN = 0;       /*0 = 16bit, 1 = 20bit, 2=24bit, 3=32bit*/
    AUD_Reg.Audio_Interface.Bit.AIFMT = 2;      /*0 = right, 1 = left, 2 = STD, 3 = PCMA or PCMB*/
    AUD_Reg.Audio_Interface.Bit.DACPHS = 0; 
    AUD_Reg.Audio_Interface.Bit.ADCPHS = 0; 
    AUD_Reg.Audio_Interface.Bit.MONO = 0;   
    ret = AUD_RegWrite( Audio_Interface, AUD_Reg.Audio_Interface.Word );
    ret = AUD_RegRead( Audio_Interface, &rdata );
    if(rdata!=AUD_Reg.Audio_Interface.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

    AUD_Reg.Companding.Word = 0x0000;           /*default*/
    ret = AUD_RegWrite( Companding, AUD_Reg.Companding.Word );
    ret = AUD_RegRead( Companding, &rdata );
    if(rdata!=AUD_Reg.Companding.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Clock_Control1.Word = 0x140;        /*default*/
    
    /*master clock source selection control, 
    0=MCLK, pin used as master clock, 
    1=internal PLL oscillator output used as master clock*/    
    AUD_Reg.Clock_Control1.Bit.CLKM = 0x0;     
    
    /*Scaling of Master clock source for internal 256fs rate Axl*/
		AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0x0;//48kHz
		//AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0x1;//32kHz
		//AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0x2;//24kHz
		//AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0x3;//16kHz
		//AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0x4;//12kHz
		//AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0x5;//8kHz
    
    /*Scaling of output frequency at BCLK pin when chip is in master mode*/
    AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x0;
    
    /*Enable chip master mode to driver FS and BCLK outputs, 
    0=FS and BCLK are inputs, 
    1=FS and BCLK are driven as outputs by internally generated clocks*/
    AUD_Reg.Clock_Control1.Bit.CLKIOEN = 0x0;   
    ret = AUD_RegWrite( Clock_Control1, AUD_Reg.Clock_Control1.Word );
    Delay(1);    
    ret = AUD_RegRead( Clock_Control1, &rdata );
    if(rdata!=AUD_Reg.Clock_Control1.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

    AUD_Reg.Clock_Control2.Word = 0x0000;       /*default*/
    ret = AUD_RegWrite( Clock_Control2, AUD_Reg.Clock_Control2.Word );
    Delay(1);
    ret = AUD_RegRead( Clock_Control2, &rdata );
    if(rdata!=AUD_Reg.Clock_Control2.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.GPIO.Word = 0x0000;                 /*default*/
    ret = AUD_RegWrite( GPIO, AUD_Reg.GPIO.Word );
    ret = AUD_RegRead( GPIO, &rdata );
    if(rdata!=AUD_Reg.GPIO.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Jack_Detect1.Word = 0x0000;         /*default*/
    ret = AUD_RegWrite( Jack_Detect1, AUD_Reg.Jack_Detect1.Word );
    ret = AUD_RegRead( Jack_Detect1, &rdata );
    if(rdata!=AUD_Reg.Jack_Detect1.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

    AUD_Reg.DAC_Control.Word = 0x000;           /*Default*/
    AUD_Reg.DAC_Control.Bit.SOFTMT = 0;         /*Softmute feature control for DACs*/
    AUD_Reg.DAC_Control.Bit.DACOS = 1;          /*DAC oversampleing rate selection, 0=64x, 1=128x*/
    AUD_Reg.DAC_Control.Bit.AUTOMT = 0;         /*DAC automute function enable*/
    AUD_Reg.DAC_Control.Bit.RDACPL = 0;         /*DAC right ch output polarity control*/
    AUD_Reg.DAC_Control.Bit.LDACPL = 0;         /*DAC left ch output polarity control*/
    ret = AUD_RegWrite( DAC_Control, AUD_Reg.DAC_Control.Word );
    ret = AUD_RegRead( DAC_Control, &rdata );
    if(rdata!=AUD_Reg.DAC_Control.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Left_Dac_Volume.Word = 0x00FF;      /*Default*/
    ret = AUD_RegWrite( Left_Dac_Volume, AUD_Reg.Left_Dac_Volume.Word );
    ret = AUD_RegRead( Left_Dac_Volume, &rdata );
    if(rdata!=AUD_Reg.Left_Dac_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Right_DAC_Volume.Word = 0x00FF;      /*Default*/
    ret = AUD_RegWrite( Right_DAC_Volume, AUD_Reg.Right_DAC_Volume.Word );
    ret = AUD_RegRead( Right_DAC_Volume, &rdata );
    if(rdata!=AUD_Reg.Right_DAC_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Jack_Detect2.Word = 0x0000;
    ret = AUD_RegWrite( Jack_Detect2, AUD_Reg.Jack_Detect2.Word );
    ret = AUD_RegRead( Jack_Detect2, &rdata );
    if(rdata!=AUD_Reg.Jack_Detect2.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.ADC_Control.Word = 0x0100;
    AUD_Reg.ADC_Control.Bit.HPFEN = 1;          /*High pass filter enable control for filter of ADC output data stream, default 1= high pass filter enable*/
    AUD_Reg.ADC_Control.Bit.HPFAM = 0;          /*High pass filter mode selection , default 0*/
    AUD_Reg.ADC_Control.Bit.HPF = 0;            /*Application specific mode cutoff frequency selection, default 0*/
    AUD_Reg.ADC_Control.Bit.ADCOS = 0;          /*ADC oversampling rate selection, 0=64x, 1=128x*/
    AUD_Reg.ADC_Control.Bit.RADCPL = 0;         /*ADC right ch polarity control, 0=normal phase*/
    AUD_Reg.ADC_Control.Bit.LADCPL = 0;         /*ADC left ch polarity control, 0=Normal phase*/
    ret = AUD_RegWrite( ADC_Control, AUD_Reg.ADC_Control.Word );
    ret = AUD_RegRead( ADC_Control, &rdata );
    if(rdata!=AUD_Reg.ADC_Control.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Left_ADC_Volume.Word = 0x00FF;      /*default*/
    ret = AUD_RegWrite( Left_ADC_Volume, AUD_Reg.Left_ADC_Volume.Word );
    ret = AUD_RegRead( Left_ADC_Volume, &rdata );
    if(rdata!=AUD_Reg.Left_ADC_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Right_ADC_Volume.Word = 0x00FF;     /*default*/
    ret = AUD_RegWrite( Right_ADC_Volume, AUD_Reg.Right_ADC_Volume.Word );
    ret = AUD_RegRead( Right_ADC_Volume, &rdata );
    if(rdata!=AUD_Reg.Right_ADC_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

//    /* EQ */
//    ret = AUD_RegWrite( EQ1_low_cutoff, wdata=0x012C );     /*default*/
//    ret = AUD_RegWrite( EQ2_peak1, wdata=0x002C );          /*default*/
//    ret = AUD_RegWrite( EQ3_peak2, wdata=0x002C );          /*default*/
//    ret = AUD_RegWrite( EQ4_peak3, wdata=0x002C );          /*default*/
//    ret = AUD_RegWrite( EQ5_high_cutoff, wdata=0x002C );    /*default*/
//    ret = AUD_RegWrite( Reserved01, wdata=0x0000 );         /*default*/

//    /* DAC Limiter */
//    ret = AUD_RegWrite( DAC_Limiter1, wdata=0x0032 );       /*default*/
//    ret = AUD_RegWrite( DAC_Limiter2, wdata=0x0000 );       /*default*/

//    /* Notch Filter */
//    ret = AUD_RegWrite( Notch_Filter1, wdata=0x0000 );      /*default*/
//    ret = AUD_RegWrite( Notch_Filter2, wdata=0x0000 );      /*default*/
//    ret = AUD_RegWrite( Notch_Filter3, wdata=0x0000 );      /*default*/
//    ret = AUD_RegWrite( Notch_Filter4, wdata=0x0000 );      /*default*/

//    /* ALC and Noise Gate Control */
//    ret = AUD_RegWrite( ALC_Control1, wdata=0x0038 );       /*default*/
//    ret = AUD_RegWrite( ALC_Control2, wdata=0x000B );       /*default*/
//    ret = AUD_RegWrite( ALC_Control3, wdata=0x0032 );       /*default*/
//    ret = AUD_RegWrite( Noise_Gate, wdata=0x0010 );         /*default*/

//    /* Phase Locked Loop (PLL) */
//    ret = AUD_RegWrite( PLL_N, wdata=0x0008 );              /*default*/
//    ret = AUD_RegWrite( PLL_K1, wdata=0x000C );             /*default*/
//    ret = AUD_RegWrite( PLL_K2, wdata=0x0093 );             /*default*/
//    ret = AUD_RegWrite( PLL_K3, wdata=0x00E9 );             /*default*/
    
    /* Miscellaneous */
//    ret = AUD_RegWrite( _3D_Control, wdata=0x0000 );    /*default*/
//    ret = AUD_RegWrite( Right_Speaker_Submix, wdata=0x0000 );   /*default*/

//    ret = AUD_RegWrite( Input_Control, wdata=0x033 );     /*default*/
    //ret = AUD_RegWrite( Left_Input_PGA_Gain, wdata=0x0010 );    /*default*/
    //ret = AUD_RegWrite( Right_Input_PGA_Gain, wdata=0x0010 );   /*default*/
//    ret = AUD_RegWrite( Left_ADC_Boost, wdata=0x0100 );         /*default*/
//    ret = AUD_RegWrite( Right_ADC_Boost, wdata=0x0100 );        /*default*/        
    
    ret = AUD_RegWrite( Output_Control, wdata=SET_BIT1|SET_BIT2 );          
    ret = AUD_RegRead( Output_Control, &rdata );
    if(rdata!=wdata)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    ret = AUD_RegWrite( Left_Mixer, wdata=0x0001 );             /*default*/  
    ret = AUD_RegWrite( Right_Mixer, wdata=0x0001 );            /*default*/  
    ret = AUD_RegWrite( LHP_Volume, wdata=0x0039 );             /*default*/  
    ret = AUD_RegWrite( RHP_Volume, wdata=0x0039 );             /*default*/  

    ret = AUD_RegWrite( LSPKOUT_Volume, wdata=0x0039 );         /*default*/  
    ret = AUD_RegWrite( RSPKOUT_Volume, wdata=0x0039 );         /*default*/  
    ret = AUD_RegWrite( AUX2_Mixer, wdata=0x0001 );             /*default*/  
    ret = AUD_RegWrite( AUX1_Mixer, wdata=0x0001 );             /*default*/  

    /* Device ID */
    ret = AUD_RegRead( Device_Revision, &rdata );     
    if(rdata!=AUDL_REVISION)    /* REV = 0x07F for REV-A */
    {
        return AUD_FAIL;
    }
    
    ret = AUD_RegRead( Device_ID, &rdata );
    if(rdata!=AUDL_ID)  /* ID = 0x01A */
    {
        return AUD_FAIL;
    }

    /* Check pass & fail */
    if(ret!=AUD_PASS)
    {
        return AUD_FAIL;
    }

    return AUD_PASS;
}

AUD_ret_t AUD_MasterMode_Init(aud_samplerate_t eSamplingRate, aud_mclk_path_t mclk_path, bool gp1_clk) /*More detail register bit description please refer to AUD spec*/
{
    uint8_t ret;
    uint16_t rdata,wdata;
    
    /* Register Map Initial */
    memset( (uint8_t*)&AUD_Reg, 0, sizeof(AUD_Reg) );
    
    /* Software Reset */
    AUD_Reg.Software_Reset.Word = 0x00; /* Any write operation to this register resets all registers to default values */
    ret = AUD_RegWrite( Software_Reset, AUD_Reg.Software_Reset.Word );  
    Delay(1);
    
    /* Spec provide power up */
    ret = AUD_RegWrite( Output_Control, SET_BIT2|SET_BIT3|SET_BIT4 );
    
    AUD_Reg.Power_Management1.Word = 0x0104;
    ret = AUD_RegWrite( Power_Management1, AUD_Reg.Power_Management1.Word );
    
    AUD_Reg.Power_Management1.Word = 0x010D;
    ret = AUD_RegWrite( Power_Management1, AUD_Reg.Power_Management1.Word );
    ret = AUD_RegRead( Power_Management1, &rdata );
    Delay(1);
    
    /* Power Management */
    AUD_Reg.Power_Management1.Word = 0x01FF;    /*turn on all*/
    ret = AUD_RegWrite( Power_Management1, AUD_Reg.Power_Management1.Word );
    ret = AUD_RegRead( Power_Management1, &rdata );
    if(rdata!=AUD_Reg.Power_Management1.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Power_Management2.Word = 0x01BF;    /*turn on all*/
    ret = AUD_RegWrite( Power_Management2, AUD_Reg.Power_Management2.Word );
    ret = AUD_RegRead( Power_Management2, &rdata );
    if(rdata!=AUD_Reg.Power_Management2.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
  
    AUD_Reg.Power_Management3.Word = 0x01EF;    /*turn on all*/
    ret = AUD_RegWrite( Power_Management3, AUD_Reg.Power_Management3.Word );
    ret = AUD_RegRead( Power_Management3, &rdata );
    if(rdata!=AUD_Reg.Power_Management3.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    /* General Audio Control */
    AUD_Reg.Audio_Interface.Word = 0x0050;      /*default*/
    AUD_Reg.Audio_Interface.Bit.BCLKP = 0;      /*0= Normal Phase*/
    AUD_Reg.Audio_Interface.Bit.LRP = 0;        /*0 = Normal Phase Operation*/
    AUD_Reg.Audio_Interface.Bit.WLEN = 0;       /*0 = 16bit, 1 = 20bit, 2=24bit, 3=32bit*/
    AUD_Reg.Audio_Interface.Bit.WLEN = 3;       /*0 = 16bit, 1 = 20bit, 2=24bit, 3=32bit*/
    AUD_Reg.Audio_Interface.Bit.AIFMT = 2;      /*0 = right, 1 = left, 2 = STD, 3 = PCMA or PCMB*/
    AUD_Reg.Audio_Interface.Bit.DACPHS = 0; 
    AUD_Reg.Audio_Interface.Bit.ADCPHS = 0; 
    AUD_Reg.Audio_Interface.Bit.MONO = 0;   
    ret = AUD_RegWrite( Audio_Interface, AUD_Reg.Audio_Interface.Word );
    ret = AUD_RegRead( Audio_Interface, &rdata );
    if(rdata!=AUD_Reg.Audio_Interface.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Companding.Word = 0x0000;           /*default*/
    ret = AUD_RegWrite( Companding, AUD_Reg.Companding.Word );
    ret = AUD_RegRead( Companding, &rdata );
    if(rdata!=AUD_Reg.Companding.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Clock_Control1.Word = 0x140;        /*default*/
    
    /*master clock source selection control, 
    0=MCLK, pin used as master clock, 
    1=internal PLL oscillator output used as master clock*/  
    AUD_Reg.Clock_Control1.Bit.CLKM = mclk_path;     
    
	if( mclk_path == MCLK_PAD_TO_INTER )
	{
		if(eSamplingRate==AUD_SR48K)
		{
			/* 48k */
			AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0;	    
			AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;
		}
		else if(eSamplingRate==AUD_SR24K)
		{
			/* 24k */
			AUD_Reg.Clock_Control1.Bit.MCLKSEL = 2;	    
			AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;
		}
		else if(eSamplingRate==AUD_SR12K)
		{
			/* 12k */
			AUD_Reg.Clock_Control1.Bit.MCLKSEL = 4;	    
			AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;
		}
		else if(eSamplingRate==AUD_SR32K)
		{
			/* 32 */
			AUD_Reg.Clock_Control1.Bit.MCLKSEL = 1;	    
			AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;
		}
		else if(eSamplingRate==AUD_SR16K)
		{
			/* 16 */
			AUD_Reg.Clock_Control1.Bit.MCLKSEL = 3;	    
			AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;
		}
		else /*if(eSamplingRate==AUD_SR8K)*/
		{
			/* 8 */
			AUD_Reg.Clock_Control1.Bit.MCLKSEL = 5;	    
			AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;
		}
	}
	else if( mclk_path == MCLK_PAD_TO_PLL )
	{
		switch(eSamplingRate)
		{
			case AUD_SR96K:
			case AUD_SR88p2K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 0;//divide by 1
				break;
			case AUD_SR64K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 1;//divide by 1.5
				break;
			case AUD_SR48K:
			case AUD_SR44p1K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 2;//divide by 2
				break;
			case AUD_SR32K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 3;//divide by 3
				break;
			case AUD_SR24K:
			case AUD_SR22p05K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 4;//divide by 4
				break;
			case AUD_SR16K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 5;//divide by 6
				break;
			case AUD_SR12K:
			case AUD_SR11p025K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 6;//divide by 8
				break;
			case AUD_SR8K:
				AUD_Reg.Clock_Control1.Bit.MCLKSEL = 7;//divide by 12
				break;
		}

		AUD_Reg.Clock_Control1.Bit.BCLKSEL = 0x2;

	}

    /*Enable chip master mode to driver FS and BCLK outputs, 
    0=FS and BCLK are inputs, 
    1=FS and BCLK are driven as outputs by internally generated clocks*/
    AUD_Reg.Clock_Control1.Bit.CLKIOEN = 0x1;   
    ret = AUD_RegWrite( Clock_Control1, AUD_Reg.Clock_Control1.Word );  
    ret = AUD_RegRead( Clock_Control1, &rdata );
    if(rdata!=AUD_Reg.Clock_Control1.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

    AUD_Reg.Clock_Control2.Word = 0x0000;       /*default*/
    ret = AUD_RegWrite( Clock_Control2, AUD_Reg.Clock_Control2.Word );
    ret = AUD_RegRead( Clock_Control2, &rdata );
    if(rdata!=AUD_Reg.Clock_Control2.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.GPIO.Word = 0x0000;                 /*default*/
    if(gp1_clk)
    {
        AUD_Reg.GPIO.Bit.GPIO1PLL = 1;              /* output */
        AUD_Reg.GPIO.Bit.GPIO1SEL = 4;              /* output divided PLL clock */
    }
    ret = AUD_RegWrite( GPIO, AUD_Reg.GPIO.Word );
    ret = AUD_RegRead( GPIO, &rdata );
    if(rdata!=AUD_Reg.GPIO.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Jack_Detect1.Word = 0x0000;         /*default*/
    ret = AUD_RegWrite( Jack_Detect1, AUD_Reg.Jack_Detect1.Word );
    ret = AUD_RegRead( Jack_Detect1, &rdata );
    if(rdata!=AUD_Reg.Jack_Detect1.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

    AUD_Reg.DAC_Control.Word = 0x000;           /*Default*/
    AUD_Reg.DAC_Control.Bit.SOFTMT = 0;         /*Softmute feature control for DACs*/
    AUD_Reg.DAC_Control.Bit.DACOS = 1;          /*DAC oversampleing rate selection, 0=64x, 1=128x*/
    AUD_Reg.DAC_Control.Bit.AUTOMT = 0;         /*DAC automute function enable*/
    AUD_Reg.DAC_Control.Bit.RDACPL = 0;         /*DAC right ch output polarity control*/
    AUD_Reg.DAC_Control.Bit.LDACPL = 0;         /*DAC left ch output polarity control*/
    ret = AUD_RegWrite( DAC_Control, AUD_Reg.DAC_Control.Word );
    ret = AUD_RegRead( DAC_Control, &rdata );
    if(rdata!=AUD_Reg.DAC_Control.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Left_Dac_Volume.Word = 0x00FF;      /*Default*/
    ret = AUD_RegWrite( Left_Dac_Volume, AUD_Reg.Left_Dac_Volume.Word );
    ret = AUD_RegRead( Left_Dac_Volume, &rdata );
    if(rdata!=AUD_Reg.Left_Dac_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Right_DAC_Volume.Word = 0x00FF;      /*Default*/
    ret = AUD_RegWrite( Right_DAC_Volume, AUD_Reg.Right_DAC_Volume.Word );
    ret = AUD_RegRead( Right_DAC_Volume, &rdata );
    if(rdata!=AUD_Reg.Right_DAC_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Jack_Detect2.Word = 0x0000;
    ret = AUD_RegWrite( Jack_Detect2, AUD_Reg.Jack_Detect2.Word );
    ret = AUD_RegRead( Jack_Detect2, &rdata );
    if(rdata!=AUD_Reg.Jack_Detect2.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.ADC_Control.Word = 0x0100;
    AUD_Reg.ADC_Control.Bit.HPFEN = 1;          /*High pass filter enable control for filter of ADC output data stream, default 1= high pass filter enable*/
    AUD_Reg.ADC_Control.Bit.HPFAM = 0;          /*High pass filter mode selection , default 0*/
    AUD_Reg.ADC_Control.Bit.HPF = 0;            /*Application specific mode cutoff frequency selection, default 0*/
    AUD_Reg.ADC_Control.Bit.ADCOS = 0;          /*ADC oversampling rate selection, 0=64x, 1=128x*/
    AUD_Reg.ADC_Control.Bit.RADCPL = 0;         /*ADC right ch polarity control, 0=normal phase*/
    AUD_Reg.ADC_Control.Bit.LADCPL = 0;         /*ADC left ch polarity control, 0=Normal phase*/
    ret = AUD_RegWrite( ADC_Control, AUD_Reg.ADC_Control.Word );
    ret = AUD_RegRead( ADC_Control, &rdata );
    if(rdata!=AUD_Reg.ADC_Control.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Left_ADC_Volume.Word = 0x00FF;      /*default*/
    ret = AUD_RegWrite( Left_ADC_Volume, AUD_Reg.Left_ADC_Volume.Word );
    ret = AUD_RegRead( Left_ADC_Volume, &rdata );
    if(rdata!=AUD_Reg.Left_ADC_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);
    
    AUD_Reg.Right_ADC_Volume.Word = 0x00FF;     /*default*/
    ret = AUD_RegWrite( Right_ADC_Volume, AUD_Reg.Right_ADC_Volume.Word );
    ret = AUD_RegRead( Right_ADC_Volume, &rdata );
    if(rdata!=AUD_Reg.Right_ADC_Volume.Word)
    {
        return AUD_FAIL;
    }
    Delay(1);

//    /* EQ */
//    ret = AUD_RegWrite( EQ1_low_cutoff, wdata=0x012C );     /*default*/
//    ret = AUD_RegWrite( EQ2_peak1, wdata=0x002C );          /*default*/
//    ret = AUD_RegWrite( EQ3_peak2, wdata=0x002C );          /*default*/
//    ret = AUD_RegWrite( EQ4_peak3, wdata=0x002C );          /*default*/
//    ret = AUD_RegWrite( EQ5_high_cutoff, wdata=0x002C );    /*default*/
//    ret = AUD_RegWrite( Reserved01, wdata=0x0000 );         /*default*/

//    /* DAC Limiter */
//    ret = AUD_RegWrite( DAC_Limiter1, wdata=0x0032 );       /*default*/
//    ret = AUD_RegWrite( DAC_Limiter2, wdata=0x0000 );       /*default*/

//    /* Notch Filter */
//    ret = AUD_RegWrite( Notch_Filter1, wdata=0x0000 );      /*default*/
//    ret = AUD_RegWrite( Notch_Filter2, wdata=0x0000 );      /*default*/
//    ret = AUD_RegWrite( Notch_Filter3, wdata=0x0000 );      /*default*/
//    ret = AUD_RegWrite( Notch_Filter4, wdata=0x0000 );      /*default*/

//    /* ALC and Noise Gate Control */
//    ret = AUD_RegWrite( ALC_Control1, wdata=0x0038 );       /*default*/
//    ret = AUD_RegWrite( ALC_Control2, wdata=0x000B );       /*default*/
//    ret = AUD_RegWrite( ALC_Control3, wdata=0x0032 );       /*default*/
//    ret = AUD_RegWrite( Noise_Gate, wdata=0x0010 );         /*default*/

	if (eSamplingRate==AUD_SR88p2K || eSamplingRate==AUD_SR44p1K || eSamplingRate==AUD_SR22p05K || eSamplingRate==AUD_SR11p025K)
	{
		/* Phase Locked Loop (PLL) */
		ret = AUD_RegWrite( PLL_N, wdata=0x0007 );              /*default*/    
		ret = AUD_RegRead( PLL_N, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);

		ret = AUD_RegWrite( PLL_K1, wdata=0x0021 );             /*default*/ 
		ret = AUD_RegRead( PLL_K1, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);

		ret = AUD_RegWrite( PLL_K2, wdata=0x15C );             /*default*/ 
		ret = AUD_RegRead( PLL_K2, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);

		ret = AUD_RegWrite( PLL_K3, wdata=0x000 );             /*default*/ 
		ret = AUD_RegRead( PLL_K3, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);
	}
	else
	{
		/* Phase Locked Loop (PLL) */
		ret = AUD_RegWrite( PLL_N, wdata=0x0008 );              /*default*/    
		ret = AUD_RegRead( PLL_N, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);

		ret = AUD_RegWrite( PLL_K1, wdata=0x000C );             /*default*/ 
		ret = AUD_RegRead( PLL_K1, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);

		ret = AUD_RegWrite( PLL_K2, wdata=0x0093 );             /*default*/ 
		ret = AUD_RegRead( PLL_K2, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);

		ret = AUD_RegWrite( PLL_K3, wdata=0x00E9 );             /*default*/ 
		ret = AUD_RegRead( PLL_K3, &rdata );
		if(rdata!=wdata)
		{
			return AUD_FAIL;
		}
		Delay(1);
	}
    
    /* Miscellaneous */
//    ret = AUD_RegWrite( _3D_Control, wdata=0x0000 );    /*default*/
//    ret = AUD_RegWrite( Right_Speaker_Submix, wdata=0x0000 );   /*default*/

//    ret = AUD_RegWrite( Input_Control, wdata=0x033 );     /*default*/
//    ret = AUD_RegWrite( Left_Input_PGA_Gain, wdata=0x0010 );    /*default*/
//    ret = AUD_RegWrite( Right_Input_PGA_Gain, wdata=0x0010 );   /*default*/
//    ret = AUD_RegWrite( Left_ADC_Boost, wdata=0x0100 );         /*default*/
//    ret = AUD_RegWrite( Right_ADC_Boost, wdata=0x0100 );        /*default*/      
    
    ret = AUD_RegWrite( Output_Control, wdata=SET_BIT1|SET_BIT2 );          
    ret = AUD_RegRead( Output_Control, &rdata );
//    if(*rdata!=wdata)
//    {
//        return AUD_FAIL;
//    }
    Delay(1);
    
//    ret = AUD_RegWrite( Left_Mixer, wdata=0x0001 );             /*default*/  
//    ret = AUD_RegWrite( Right_Mixer, wdata=0x0001 );            /*default*/  
//    ret = AUD_RegWrite( LHP_Volume, wdata=0x0039 );             /*default*/  
//    ret = AUD_RegWrite( RHP_Volume, wdata=0x0039 );             /*default*/  

//    ret = AUD_RegWrite( LSPKOUT_Volume, wdata=0x0039 );         /*default*/  
//    ret = AUD_RegWrite( RSPKOUT_Volume, wdata=0x0039 );         /*default*/  
//    ret = AUD_RegWrite( AUX2_Mixer, wdata=0x0001 );             /*default*/  
//    ret = AUD_RegWrite( AUX1_Mixer, wdata=0x0001 );             /*default*/  

    /* Device ID */
    ret = AUD_RegRead( Device_Revision, &rdata );     
    if(rdata!=AUDL_REVISION)    /* REV = 0x07F for REV-A */
    {
        return AUD_FAIL;
    }
    
    ret = AUD_RegRead( Device_ID, &rdata );
    if(rdata!=AUDL_ID)  /* ID = 0x01A */
    {
        return AUD_FAIL;
    }

    /* Check pass & fail */
    if(ret!=AUD_PASS)
    {
        return AUD_FAIL;
    }

    return AUD_PASS;
}


AUD_ret_t AUD_DAC_Volume(uint16_t L_DAC_Volume, uint16_t R_DAC_Volume)
{
		uint8_t ret;
    uint16_t rdata;


		ret = AUD_RegWrite( LSPKOUT_Volume, L_DAC_Volume ); //default =0x039
    ret = AUD_RegRead( LSPKOUT_Volume, &rdata );
	
		if((L_DAC_Volume >> 8) & 0x1){
			if((rdata|0x100)!=L_DAC_Volume)
				return AUD_FAIL;
		}
		else
			if(rdata!=L_DAC_Volume)
				return AUD_FAIL;
    Delay(1);


		ret = AUD_RegWrite( RSPKOUT_Volume, R_DAC_Volume );	//default =0x039
    ret = AUD_RegRead( RSPKOUT_Volume, &rdata );
	
		if((R_DAC_Volume >> 8) & 0x1){
			if((rdata|0x100)!=R_DAC_Volume)
				return AUD_FAIL;
		}
		else
			if(rdata!=R_DAC_Volume)
				return AUD_FAIL;
    Delay(1);

    /* Check pass & fail */
    if(ret!=AUD_PASS)
    {
        return AUD_FAIL;
    }

    return AUD_PASS;			
}


AUD_ret_t AUD_ADC_Gain_Setting(uint16_t L_ADC_Boost_s, uint16_t L_ADC_Gain, uint16_t R_ADC_Boost_s, uint16_t R_ADC_Gain)
{
		uint8_t ret;
    uint16_t rdata;
	
    ret = AUD_RegWrite( Left_Input_PGA_Gain, L_ADC_Gain ); //default =0x010
    ret = AUD_RegRead( Left_Input_PGA_Gain, &rdata );
	
		if((L_ADC_Gain >> 8) & 0x1){
			if((rdata|0x100)!=L_ADC_Gain)
				return AUD_FAIL;
		}
		else
			if(rdata!=L_ADC_Gain)
				return AUD_FAIL;
    Delay(1);

		R_ADC_Gain = 0x010;	
		ret = AUD_RegWrite( Right_Input_PGA_Gain, R_ADC_Gain ); //default =0x010
    ret = AUD_RegRead( Right_Input_PGA_Gain, &rdata );
	
		if((R_ADC_Gain >> 8) & 0x1){
			if((rdata|0x100)!=R_ADC_Gain)
				return AUD_FAIL;
		}
		else
			if(rdata!=R_ADC_Gain)
				return AUD_FAIL;
    Delay(1);	

		L_ADC_Boost_s =0x100;
		ret = AUD_RegWrite( Left_ADC_Boost, L_ADC_Boost_s ); //default =0x100
    ret = AUD_RegRead( Left_ADC_Boost, &rdata );
    if(rdata!=L_ADC_Boost_s)
    {
        return AUD_FAIL;
    }
    Delay(1);

	
    ret = AUD_RegWrite( Right_ADC_Boost, R_ADC_Boost_s ); //default =0x100
    ret = AUD_RegRead( Right_ADC_Boost, &rdata );
    if(rdata!=R_ADC_Boost_s)
    {
        return AUD_FAIL;
    }
    Delay(1);
		
    /* Check pass & fail */
    if(ret!=AUD_PASS)
    {
        return AUD_FAIL;
    }

    return AUD_PASS;		
}

AUD_ret_t AUD_Get_DAC_Volume(uint16_t* Volume)
{
	return AUD_RegRead(LSPKOUT_Volume, Volume);
}

#define SNAUD_PRINTF_REG(str,reg,rdata)             printf("[SNAUD01] [REG:%s] Reg_Addr=0x%x Rdata=0x%x \r\n",str, reg, rdata);
#define SNAUD_DEBUG_REG(str,reg,wdata,rdata)        printf("[SNAUD01] [REG:%s] Reg_Addr=0x%x Wdata=0x%x Rdata=0x%x \r\n",str, reg, wdata, rdata);
#define SNAUD_REG_WRITE_FAIL(str)                   printf("[SNAUD01] [REG:%s] FAIL!!!\n\r",str);
#define SNAUD_REG_WRITE_PASS(str)                   //printf("[SNAUD01] [REG:%s] PASS\n\r",str);
#define SNAUD_REG_WAIT(str)                         printf("Wait %s \r\n",str);
#define SNAUD_REG_WAIT_PASS(str)                    //printf("Wait %s PASS\r\n",str);
static AUD_ret_t SNAUD01_sys_power_up(void)
{
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;
   
    /* SYS_CLK_CTRL */
    reg=SYS_CLK_CTRL;
    wdata=0x88;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);  
    SNAUD_DEBUG_REG("SYS_CLK_CTRL", reg, wdata, rdata);
    if(wdata!=(rdata&~(SET_BIT0)))
    {
        SNAUD_REG_WRITE_FAIL("SYS_CLK_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("SYS_CLK_CTRL");

    
    /* check  FPLL_RDY is ready*/
    SNAUD_REG_WAIT("FPLL_RDY");
    while((rdata&0x01)!=0x01)
    {    
        AUD_RegRead( reg, &rdata);    
    }
    SNAUD_REG_WAIT_PASS("FPLL_RDY");

    
    /* FPLL_CTRL */
    reg=FPLL_CTRL;
    wdata=0x46;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("FPLL_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("FPLL_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("FPLL_CTRL");
    
    
    /* LDO_CTRL */
    reg=LDO_CTRL;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("LDO_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("LDO_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("LDO_CTRL");

    
    /* PMU_CTRL */
    reg=PMU_CTRL;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("PMU_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("PMU_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("PMU_CTRL");
    
 
    /* I2S_ADC_CTRL0 - ADC master or slave*/
    reg=I2S_ADC_CTRL0;
    //wdata=0x03;    //48K  ZSC
		wdata=0x23;      //16K  ZSC
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("I2S_ADC_CTRL0", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("I2S_ADC_CTRL0");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("I2S_ADC_CTRL0");
    
    
    /* I2S_ADC_CTRL2 - BCLKDIV */
    reg=I2S_ADC_CTRL2;
    //wdata=0x01;    //48K  ZSC
		wdata=0x02;      //16K   ZSC
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("I2S_ADC_CTRL2", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("I2S_ADC_CTRL2");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("I2S_ADC_CTRL2");
    

    /* I2S_DAC_CLKSRC  */    
    reg=I2S_DAC_CLKSRC;
//    wdata=0x03;
		wdata=0x23;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("I2S_DAC_CLKSRC", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("I2S_DAC_CLKSRC");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("I2S_DAC_CLKSRC");
    
    
    /* I2S_DAC_CTRL2 master only*/ 
    reg=I2S_DAC_CTRL2;
//    wdata=0x01;
		wdata=0x02;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("I2S_DAC_CTRL2", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("I2S_DAC_CTRL2");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("I2S_DAC_CTRL2");
    
    
    /* I2C_ALT_ID_EN */
//    reg=I2C_ALT_ID_EN;
//    wdata=0xD8;
//    AUD_RegWrite( reg, wdata);
//    AUD_RegRead( reg, &rdata);
//    SNAUD_DEBUG_REG("I2C_ALT_ID_EN", reg, wdata, rdata);
//    if(wdata!=rdata)
//    {
//        SNAUD_REG_WRITE_FAIL("I2C_ALT_ID_EN");
//       return AUD_FAIL; 
//    }
//    SNAUD_REG_WRITE_PASS("I2C_ALT_ID_EN");


    /* DEBUG_MODE0 */
//    reg=DEBUG_MODE0;
//    wdata=0x00;
//    AUD_RegWrite( reg, wdata);
//    AUD_RegRead( reg, &rdata);
//    SNAUD_DEBUG_REG("DEBUG_MODE0", reg, wdata, rdata);
//    if(wdata!=rdata)
//    {
//        SNAUD_REG_WRITE_FAIL("DEBUG_MODE0");
//       return AUD_FAIL; 
//    }
//    SNAUD_REG_WRITE_PASS("DEBUG_MODE0");
    
    
    /* DEBUG_MODE1 */
//    reg=DEBUG_MODE0;
//    wdata=0x00;
//    AUD_RegWrite( reg, wdata);
//    AUD_RegRead( reg, &rdata);
//    SNAUD_DEBUG_REG("DEBUG_MODE1", reg, wdata, rdata);
//    if(wdata!=rdata)
//    {
//        SNAUD_REG_WRITE_FAIL("DEBUG_MODE1");
//       return AUD_FAIL; 
//    }
//    SNAUD_REG_WRITE_PASS("DEBUG_MODE1");


    /* GPIO_IO_MODE */
//    reg=GPIO_IO_MODE;
//    wdata=0x00;
//    AUD_RegWrite( reg, wdata);
//    AUD_RegRead( reg, &rdata);
//    SNAUD_DEBUG_REG("GPIO_IO_MODE", reg, wdata, rdata);
//    if(wdata!=rdata)
//    {
//        SNAUD_REG_WRITE_FAIL("GPIO_IO_MODE");
//       return AUD_FAIL; 
//    }
//    SNAUD_REG_WRITE_PASS("GPIO_IO_MODE");


    /* GPIO_IO_CHG */
//    reg=GPIO_IO_CHG;
//    wdata=0x00;
//    AUD_RegWrite( reg, wdata);
//    AUD_RegRead( reg, &rdata);
//    SNAUD_DEBUG_REG("GPIO_IO_CHG", reg, wdata, rdata);
//    if(wdata!=rdata)
//    {
//        SNAUD_REG_WRITE_FAIL("GPIO_IO_CHG");
//       return AUD_FAIL; 
//    }
//    SNAUD_REG_WRITE_PASS("GPIO_IO_CHG");


    /* GPIO_DAT */
//    reg=GPIO_DAT;
//    wdata=0x00;
//    AUD_RegWrite( reg, wdata);
//    AUD_RegRead( reg, &rdata);
//    SNAUD_DEBUG_REG("GPIO_DAT", reg, wdata, rdata);
//    if(wdata!=rdata)
//    {
//        SNAUD_REG_WRITE_FAIL("GPIO_DAT");
//       return AUD_FAIL; 
//    }
//    SNAUD_REG_WRITE_PASS("GPIO_DAT");

    return AUD_PASS; 
}

static AUD_ret_t 
SNAUD01_dac_power_up(
    aud_dac_type_t      dac_type, 
    aud_dac_depop_t     dac_depop, 
    uint32_t            dac_charge_delay)
{
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;
    
    switch (dac_type)
    {
        case AC_COUPLED:    /* AC Coupled power up flow , do not modify */

        /* DAC_ISO1->ISO_IN_SELB=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_IN_SELB=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO2->ISO_PDB_IREF_L/R=1 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT6|SET_BIT7) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->ISO_PDB_IREF_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        

        /* DAC_ISO1->ISO_PDB_VREFSPK=1 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT2) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_VREFSPK=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO2->PDB_DRV_L/R=1 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT4|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->PDB_DRV_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        
        
        /* DAC_ISO1->ISO_NOPOP_HP_SEL[2:0]=dac_depop value */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= (rdata&~(SET_BIT5|SET_BIT6|SET_BIT7));
        if( dac_depop == AUD_NOPOP_8  )
        {
            wdata |= (AUD_NOPOP_4<<5);  /* turn off weakly drive */
        }
        else
        {
            wdata |= (dac_depop<<5);    /* turn on weakly drive */
        }
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_NOPOP_HP_SEL[2:0]=dac_depop", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");

        /* DAC_ISO2->NOPOP_VREFSPK_SEL=1 */
        if( dac_depop==AUD_NOPOP_8 )
        {
            reg=DAC_ISO2;
            AUD_RegRead( reg, &rdata);
            wdata= rdata|(SET_BIT3) ;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("DAC_ISO2->NOPOP_VREFSPK_SEL=1", reg, wdata, rdata);
            if(wdata!=rdata)
            {
                SNAUD_REG_WRITE_FAIL("DAC_ISO2");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("DAC_ISO2");
        }
        
        
        AUD_MS_DELAY(dac_charge_delay);

        
        /* DAC_PD->PD_DAC_L/R=0 , DAC_PD->PD_VMIDBUF_L/R=0 , DAC_PD->PD_CLK_L/R=0 */
        reg=DAC_PD;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT3|SET_BIT7|SET_BIT2|SET_BIT6|SET_BIT1|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_PD->PD_DAC_L/R=0 , DAC_PD->PD_VMIDBUF_L/R=0 , DAC_PD->PD_CLK_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_PD");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_PD");

        
        /* DAC_MUTE->I_MUTEB_L/R=1 */
        reg=DAC_MUTE;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT0|SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_MUTE->I_MUTEB_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_MUTE");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_MUTE");
        

        /* DAC_ISO3->ISO_PDB_DFO_16/17=1 */
        reg=DAC_ISO3;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT0|SET_BIT1) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO3->ISO_PDB_DFO_16/17=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO3");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO3");

        
        /* DAC_CTRL1 */
        reg=DAC_CTRL1;
        wdata=0x03;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_CTRL1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_CTRL1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_CTRL1");

        
        /* DAC_ISO2->NOPOP_VREFSPK_SEL=0 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT3) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->NOPOP_VREFSPK_SEL=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        

        /* DAC_ISO1->ISO_IN_SELB=1 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_IN_SELB=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");

        
        break;
    
        case CAPLESS:
        default :

        /* DAC_ISO1->ISO_IN_SELB=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_IN_SELB=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO2->ISO_PDB_IREF_L/R=1 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT6|SET_BIT7) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->ISO_PDB_IREF_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        

        /* DAC_ISO1->ISO_PDB_IREF_VCOM=1 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT0) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_IREF_VCOM=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO1->ISO_PDB_VREFSPK=1 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT2) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_VREFSPK=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO2->PDB_DRV_L/R=1 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT4|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->PDB_DRV_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        
        
        /* DAC_ISO1->ISO_PDB_VCOM_BUF=1 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT1) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_VCOM_BUF=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");

        
        /* DAC_ISO1->ISO_NOPOP_HP_SEL[2:0]=dac_depop value */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= (rdata&~(SET_BIT5|SET_BIT6|SET_BIT7));
        if( dac_depop == AUD_NOPOP_8  )
        {
            wdata |= (AUD_NOPOP_4<<5);  /* turn off weakly drive */
        }
        else
        {
            wdata |= (dac_depop<<5);    /* turn on weakly drive */
        }
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_NOPOP_HP_SEL[2:0]=dac_depop", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");

        /* DAC_ISO2->NOPOP_VREFSPK_SEL=1 */
        if( dac_depop==AUD_NOPOP_8 )    /* turn on HP output weakly drive */
        {
            reg=DAC_ISO2;
            AUD_RegRead( reg, &rdata);
            wdata= rdata|(SET_BIT3) ;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("DAC_ISO2->NOPOP_VREFSPK_SEL=1", reg, wdata, rdata);
            if(wdata!=rdata)
            {
                SNAUD_REG_WRITE_FAIL("DAC_ISO2");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("DAC_ISO2");
        }
        
        
        /* DAC_PD->PD_DAC_L/R=0 , DAC_PD->PD_VMIDBUF_L/R=0 , DAC_PD->PD_CLK_L/R=0 */
        reg=DAC_PD;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT3|SET_BIT7|SET_BIT2|SET_BIT6|SET_BIT1|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_PD->PD_DAC_L/R=0 , DAC_PD->PD_VMIDBUF_L/R=0 , DAC_PD->PD_CLK_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_PD");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_PD");
        
        
        /* DAC_MUTE->I_MUTEB_L/R=1 */
        reg=DAC_MUTE;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT0|SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_MUTE->I_MUTEB_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_MUTE");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_MUTE");
        

        /* DAC_ISO3->ISO_PDB_DFO_16/17=1 */
        reg=DAC_ISO3;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT0|SET_BIT1) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO3->ISO_PDB_DFO_16/17=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO3");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO3");

        
        /* DAC_CTRL1 */
        reg=DAC_CTRL1;
        wdata=0x03;
//				wdata=0x13;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_CTRL1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_CTRL1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_CTRL1");

        
        /* DAC_ISO1->ISO_IN_SELB=1 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_IN_SELB=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");        

        break;

    }

    /* AMP_CTRL */
    reg=AMP_CTRL;
    wdata=0x9D;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("AMP_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("AMP_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("AMP_CTRL");
    
    
    /* OPA_RDY*/
    SNAUD_REG_WAIT("OPA_RDY");
    while((rdata&0x1) != 0x1)
    {
        AUD_RegRead( reg, &rdata);
    }    
    SNAUD_REG_WAIT_PASS("OPA_RDY");    
    
    return AUD_PASS; 
}

static AUD_ret_t SNAUD01_adc_power_up(void)
{
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;
    
    /* ADC_MIC_CTRL */
    reg=ADC_MIC_CTRL;
    wdata=0xD9;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_MIC_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_MIC_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_MIC_CTRL");
    
    
    /* ADC_CTRL0 */
    reg=ADC_CTRL0;
    wdata=0xEE;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_CTRL0", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_CTRL0");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_CTRL0");
    
    
    /* ADC_CTRL1 */
    reg=ADC_CTRL1;
    wdata=0xEE;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_CTRL1", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_CTRL1");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_CTRL1");
    
    
    /* ADC_CTRL2 */
    reg=ADC_CTRL2;
    wdata=0x31;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_CTRL2", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_CTRL2");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_CTRL2");
        
    
    /* ADC_ZCU_CTRL */
    reg=ADC_ZCU_CTRL;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_ZCU_CTRL", reg, wdata, rdata);
    if(wdata!=(rdata&~(SET_BIT5|SET_BIT1)))
    {
        SNAUD_REG_WRITE_FAIL("ADC_ZCU_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_ZCU_CTRL");
    
    
    /* ADC_PGA_R  */
    reg=ADC_PGA_R;
    wdata=0x10;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_PGA_R", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_PGA_R");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_PGA_R");
    
    
    /* ADC_FILTER_R */
    reg=ADC_FILTER_R;
    wdata=0xB0;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_FILTER_R", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_FILTER_R");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_FILTER_R");
    
    
    /* ADC_PGA_L */
    reg=ADC_PGA_L;
    wdata=0x10;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_PGA_L", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_PGA_L");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_PGA_L");
    
    
    /* ADC_FILTER_L */
    reg=ADC_FILTER_L;
    wdata=0xB0;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_FILTER_L", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_FILTER_L");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_FILTER_L");
    
    
    /* ADC_Test */
    reg=ADC_Test;
    wdata=0x11;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_Test", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_Test");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_Test");
    
    return AUD_PASS; 
}


static AUD_ret_t SNAUD01_sys_power_down(void)
{
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;
    
    /* SYS_CLK_CTRL */
    reg=SYS_CLK_CTRL;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("SYS_CLK_CTRL", reg, wdata, rdata);
    if(wdata!=(rdata&~(SET_BIT0)))
    {
        SNAUD_REG_WRITE_FAIL("SYS_CLK_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("SYS_CLK_CTRL");
    
    
    /* PMU_CTRL */
    reg=PMU_CTRL;
    wdata=(0x29<<1)|SET_BIT0;
    AUD_RegWrite( reg, wdata);
    SNAUD_DEBUG_REG("PMU_CTRL", reg, wdata, rdata);
    SNAUD_REG_WRITE_PASS("PMU_CTRL");
    
    return AUD_PASS; 
}

static AUD_ret_t 
SNAUD01_dac_power_down(
    aud_dac_type_t          dac_type,
    aud01_ldo18_bandgap_t   ldo18_bandgap,
    bool weakly_drive)
{
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;

    switch (dac_type)
    {
        case AC_COUPLED:    /* AC Coupled power down flow , do not modify */
            
        /* DAC_ISO1->ISO_IN_SELB=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_IN_SELB=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");


        /* DAC_ISO3->ISO_PDB_DFO_16/17=0 */
        reg=DAC_ISO3;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT0|SET_BIT1) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO3->ISO_PDB_DFO_16/17=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO3");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO3");
        

        /* DAC_MUTE->I_MUTEB_L/R=0 */
        reg=DAC_MUTE;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT0|SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_MUTE->I_MUTEB_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_MUTE");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_MUTE");
        
        
        /* DAC_PD->PD_DAC_L/R=1 , DAC_PD->PD_VMIDBUF_L/R=1 , DAC_PD->PD_CLK_L/R=1 */
        reg=DAC_PD;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT3|SET_BIT7|SET_BIT2|SET_BIT6|SET_BIT1|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_PD->PD_DAC_L/R=1 , DAC_PD->PD_VMIDBUF_L/R=1 , DAC_PD->PD_CLK_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_PD");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_PD");
        
        
        /* DAC_ISO2->PDB_DRV_L/R=0 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT4|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->PDB_DRV_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        

        /* DAC_ISO1->ISO_PDB_VREFSPK=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT2) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_VREFSPK=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        

        /* DAC_ISO2->ISO_PDB_IREF_L/R=0 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT6|SET_BIT7) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->ISO_PDB_IREF_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");

        
        /* DAC_CTRL1 */
        reg=DAC_CTRL1;
        wdata=0x13;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_CTRL1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_CTRL1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_CTRL1");
    
        break;
        
        case CAPLESS:       /* Capless power down flow , do not modify */
        default :

        /* DAC_ISO1->ISO_IN_SELB=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_IN_SELB=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO3->ISO_PDB_DFO_16/17=0 */
        reg=DAC_ISO3;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT0|SET_BIT1) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO3->ISO_PDB_DFO_16/17=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO3");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO3");
        
        
        /* DAC_MUTE->I_MUTEB_L/R=0 */
        reg=DAC_MUTE;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT0|SET_BIT4) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_MUTE->I_MUTEB_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_MUTE");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_MUTE");
        
        
        /* DAC_PD->PD_DAC_L/R=1 , DAC_PD->PD_VMIDBUF_L/R=1 , DAC_PD->PD_CLK_L/R=1 */
        reg=DAC_PD;
        AUD_RegRead( reg, &rdata);
        wdata= rdata|(SET_BIT3|SET_BIT7|SET_BIT2|SET_BIT6|SET_BIT1|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_PD->PD_DAC_L/R=1 , DAC_PD->PD_VMIDBUF_L/R=1 , DAC_PD->PD_CLK_L/R=1", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_PD");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_PD");
        
        
        /* DAC_ISO2->PDB_DRV_L/R=0 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT4|SET_BIT5) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->PDB_DRV_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");

        
        /* DAC_ISO1->ISO_PDB_VCOM_BUF=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT1) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_VCOM_BUF=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO1->ISO_PDB_VREFSPK=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT2) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_VREFSPK=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");
        
        
        /* DAC_ISO2->ISO_PDB_IREF_L/R=0 */
        reg=DAC_ISO2;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT6|SET_BIT7) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO2->ISO_PDB_IREF_L/R=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO2");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO2");
        
        
        /* DAC_ISO1->ISO_PDB_IREF_VCOM=0 */
        reg=DAC_ISO1;
        AUD_RegRead( reg, &rdata);
        wdata= rdata&~(SET_BIT0) ;
        AUD_RegWrite( reg, wdata);
        AUD_RegRead( reg, &rdata);
        SNAUD_DEBUG_REG("DAC_ISO1->ISO_PDB_IREF_VCOM=0", reg, wdata, rdata);
        if(wdata!=rdata)
        {
            SNAUD_REG_WRITE_FAIL("DAC_ISO1");
           return AUD_FAIL; 
        }
        SNAUD_REG_WRITE_PASS("DAC_ISO1");

        break;
    }

    /* DAC_ISO2 */
    reg=DAC_ISO2;
    if( ldo18_bandgap == AUD01_LDO18_BANDGAP_OFF )
    {
        wdata=0x04;
    }
    if( ldo18_bandgap == AUD01_LDO18_BANDGAP_ON )
    {
        wdata=0x00;
    }
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("DAC_ISO2", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("DAC_ISO2");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("DAC_ISO2");

    
    /* AMP_CTRL */
    reg=AMP_CTRL;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("AMP_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("AMP_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("AMP_CTRL");

    return AUD_PASS; 
}

static AUD_ret_t SNAUD01_adc_power_down(void)
{
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;
    
    /* ADC_MIC_CTRL */
    reg=ADC_MIC_CTRL;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_MIC_CTRL", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_MIC_CTRL");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_MIC_CTRL");
    
    
    /* ADC_CTRL0 */
    reg=ADC_CTRL0;
    wdata=0x11;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_CTRL0", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_CTRL0");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_CTRL0");
    
    
    /* ADC_CTRL1 */
    reg=ADC_CTRL1;
    wdata=0x11;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_CTRL1", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_CTRL1");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_CTRL1");


    /* ADC_PGA_R  */
    reg=ADC_PGA_R;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_PGA_R", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_PGA_R");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_PGA_R");  
    
    
    /* ADC_PGA_L */
    reg=ADC_PGA_L;
    wdata=0x00;
    AUD_RegWrite( reg, wdata);
    AUD_RegRead( reg, &rdata);
    SNAUD_DEBUG_REG("ADC_PGA_L", reg, wdata, rdata);
    if(wdata!=rdata)
    {
        SNAUD_REG_WRITE_FAIL("ADC_PGA_L");
       return AUD_FAIL; 
    }
    SNAUD_REG_WRITE_PASS("ADC_PGA_L");
    
    return AUD_PASS; 
}

AUD_ret_t
SNAUD01_MasterMode_Init( 
    aud_samplerate_t        eSamplingRate, 
    aud_mclk_path_t         mclk_path, 
    bool                    gpio_clk,
    aud_dac_type_t          dac_type,
    aud_dac_depop_t         dac_depop, 
    uint32_t                dac_charge_delay)
{
    AUD_ret_t ret1,ret2,ret3;

    /* system power up */
    ret1 = SNAUD01_sys_power_up();
    
    /* adc power up */
    ret2 = SNAUD01_adc_power_up();
    
    /* dac power up */
    ret3 = SNAUD01_dac_power_up(dac_type,dac_depop,dac_charge_delay);

    return (AUD_ret_t)(ret1|ret2|ret3);
}

AUD_ret_t SNAUD01_power_down(aud_dac_type_t dac_type)
{
    AUD_ret_t ret1,ret2,ret3;

    /* adc power down */
    ret1 = SNAUD01_adc_power_down();
    
    /* dac power down */
    ret2 = SNAUD01_dac_power_down(dac_type, AUD01_LDO18_BANDGAP_OFF, false);

    /* system power down */
    ret3 = SNAUD01_sys_power_down();
    
    return (AUD_ret_t)(ret1|ret2|ret3);
}

AUD_ret_t SNAUD01_sleep(aud_dac_type_t dac_type)
{
    AUD_ret_t ret1,ret2,ret3;

    /* adc power down */
    ret1 = SNAUD01_adc_power_down();
    
    /* dac power down */
    ret2 = SNAUD01_dac_power_down(dac_type, AUD01_LDO18_BANDGAP_ON, true);

    /* system power down */
    ret3 = SNAUD01_sys_power_down();
    
    return (AUD_ret_t)(ret1|ret2|ret3);
}

AUD_ret_t SNAUD01_list_all_reg(void)
{
    AUD_ret_t ret = AUD_PASS;
    uint16_t rdata;
    AUD01_RegTable_t reg;
      
    /* SYS_CLK_CTRL */
    reg=SYS_CLK_CTRL;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("SYS_CLK_CTRL",reg, rdata);
    
    
    /* FPLL_CTRL */
    reg=FPLL_CTRL; 
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("FPLL_CTRL",reg, rdata);
    
    
    /* LDO_CTRL */
    reg=LDO_CTRL;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("LDO_CTRL",reg, rdata);

    
    /* PMU_CTRL */
    reg=PMU_CTRL;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("PMU_CTRL",reg, rdata);
 
 
    /* I2S_ADC_CTRL0 - ADC master or slave*/
    reg=I2S_ADC_CTRL0;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("I2S_ADC_CTRL0",reg, rdata);
    
    
    /* I2S_ADC_CTRL2 - BCLKDIV */
    reg=I2S_ADC_CTRL2;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("I2S_ADC_CTRL2",reg, rdata);
    
    
    /* I2S_DAC_CLKSRC  */    
    reg=I2S_DAC_CLKSRC;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("I2S_DAC_CLKSRC",reg, rdata);
    
    
    /* I2S_DAC_CTRL2 master only*/ 
    reg=I2S_DAC_CTRL2;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("I2S_DAC_CTRL2",reg, rdata);
    
    
    /* I2C_ALT_ID_EN */
    reg=I2C_ALT_ID_EN;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("I2C_ALT_ID_EN",reg, rdata);
    
    
    /* ADC_MIC_CTRL */
    reg=ADC_MIC_CTRL;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_MIC_CTRL",reg, rdata);
    
    
    /* ADC_CTRL0 */
    reg=ADC_CTRL0;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_CTRL0",reg, rdata);
    
    
    /* ADC_CTRL1 */
    reg=ADC_CTRL1;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_CTRL1",reg, rdata);
    
    
    /* ADC_CTRL2 */
    reg=ADC_CTRL2;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_CTRL2",reg, rdata);
        
    
    /* ADC_ZCU_CTRL */
    reg=ADC_ZCU_CTRL;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_ZCU_CTRL",reg, rdata);
    
    
    /* ADC_PGA_R  */
    reg=ADC_PGA_R;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_PGA_R",reg, rdata);
    
    
    /* ADC_FILTER_R */
    reg=ADC_FILTER_R;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_FILTER_R",reg, rdata);
    
    
    /* ADC_PGA_L */
    reg=ADC_PGA_L;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_PGA_L",reg, rdata);
    
    
    /* ADC_FILTER_L */
    reg=ADC_FILTER_L;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_FILTER_L",reg, rdata);
    
    
    /* ADC_Test */
    reg=ADC_Test;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("ADC_Test",reg, rdata);
    
    
    /* DAC_CTRL0 */
    reg=DAC_CTRL0;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_CTRL0",reg, rdata);
    
    
    /* DAC_CTRL1 */
    reg=DAC_CTRL1;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_CTRL1",reg, rdata);
    
    
    /* DAC_PD */
    reg=DAC_PD;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_PD",reg, rdata);
    
    
    /* DAC_GAIN */
    reg=DAC_GAIN;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_GAIN",reg, rdata);
    
    
    /* DAC_DEEMP */
    reg=DAC_DEEMP;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_DEEMP",reg, rdata);
    
    
    /* DAC_MUTE */
    reg=DAC_MUTE;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_MUTE",reg, rdata);

    
    /* AMP_CTRL */
    reg=AMP_CTRL;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("AMP_CTRL",reg, rdata);
    
    /* DEBUG_MODE0 */
    reg=DEBUG_MODE0;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DEBUG_MODE0",reg, rdata);
    
    
    /* DEBUG_MODE1 */
    reg=DEBUG_MODE0;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DEBUG_MODE1",reg, rdata);


    /* GPIO_IO_MODE */
    reg=GPIO_IO_MODE;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("GPIO_IO_MODE",reg, rdata);


    /* GPIO_IO_CHG */
    reg=GPIO_IO_CHG;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("GPIO_IO_CHG",reg, rdata);


    /* GPIO_DAT */
    reg=GPIO_DAT;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("GPIO_DAT",reg, rdata);


    /* DAC_ISO1 */
    reg=DAC_ISO1;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_ISO1",reg, rdata);
    
    
    /* DAC_ISO2 */
    reg=DAC_ISO2;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_ISO2",reg, rdata);
    
    
    /* DAC_ISO3 */
    reg=DAC_ISO3;
    ret = AUD_RegRead( reg, &rdata);
    SNAUD_PRINTF_REG("DAC_ISO3",reg, rdata);

    if( ret != AUD_PASS )
    {
        SNAUD_REG_WRITE_FAIL("Access Fail")
    }

    return ret;
}


AUD_ret_t SNAUD01_debug_mode(AUD01_DEBUG_MODE_t aud_debug_mode)
{
    uint8_t mode;
    uint16_t wdata,rdata;
    AUD01_RegTable_t reg;
    
    switch (mode)
    {
        case DEBUG_MODE0_NONE:

            break;
        
        case DEBUG_MODE0_DAC_L_ANALOG:
        case DEBUG_MODE0_DAC_R_ANALOG:
        case DEBUG_MODE0_ADC_L_ANALOG:
        case DEBUG_MODE0_ADC_R_ANALOG:
        case DEBUG_MODE0_ADC_DAC_LOOPBACK:
        case DEBUG_MODE0_HP_L:
        case DEBUG_MODE0_HP_R:
        case DEBUG_MODE0_GPIO:
        case DEBUG_MODE0_I2C_TEST1:
        case DEBUG_MODE0_I2C_TEST2:
        case DEBUG_MODE0_LDO18_CHOPPER:


        
            break;
        
        case DEBUG_MODE1_DIGITAL:

            /* SYS_CLK_CTRL */
            reg=SYS_CLK_CTRL;
            wdata=0x08;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);  
            SNAUD_DEBUG_REG("SYS_CLK_CTRL", reg, wdata, rdata);
            if(wdata!=(rdata&~(SET_BIT0)))
            {
                SNAUD_REG_WRITE_FAIL("SYS_CLK_CTRL");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("SYS_CLK_CTRL");

            /* I2S_ADC_CTRL0 - ADC master or slave*/
            reg=I2S_ADC_CTRL0;
            wdata=0x00;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("I2S_ADC_CTRL0", reg, wdata, rdata);
            if(wdata!=rdata)
            {
                SNAUD_REG_WRITE_FAIL("I2S_ADC_CTRL0");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("I2S_ADC_CTRL0");
            
            
            /* I2S_ADC_CTRL2 - BCLKDIV */
            reg=I2S_ADC_CTRL2;
            wdata=0x00;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("I2S_ADC_CTRL2", reg, wdata, rdata);
            if(wdata!=rdata)
            {
                SNAUD_REG_WRITE_FAIL("I2S_ADC_CTRL2");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("I2S_ADC_CTRL2");
            

            /* I2S_DAC_CLKSRC  */    
            reg=I2S_DAC_CLKSRC;
            wdata=0x00;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("I2S_DAC_CLKSRC", reg, wdata, rdata);
            if(wdata!=rdata)
            {
                SNAUD_REG_WRITE_FAIL("I2S_DAC_CLKSRC");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("I2S_DAC_CLKSRC");
            
            
            /* I2S_DAC_CTRL2 master only*/ 
            reg=I2S_DAC_CTRL2;
            wdata=0x00;
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("I2S_DAC_CTRL2", reg, wdata, rdata);
            if(wdata!=rdata)
            {
                SNAUD_REG_WRITE_FAIL("I2S_DAC_CTRL2");
               return AUD_FAIL; 
            }
            SNAUD_REG_WRITE_PASS("I2S_DAC_CTRL2");
    

            /* DEBUG_MODE0 */
            reg=DEBUG_MODE0;
            wdata=(0xAC)|(aud_debug_mode);   /* Bit7~Bit4 key = 0xA */
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("DEBUG_MODE0", reg, wdata, rdata);
            SNAUD_REG_WRITE_PASS("DEBUG_MODE0");
            
            
            /* DEBUG_MODE1 */
            reg=DEBUG_MODE1;
            wdata=(0xB9);                    /* Bit7~Bit4 key = 0xA */
            AUD_RegWrite( reg, wdata);
            AUD_RegRead( reg, &rdata);
            SNAUD_DEBUG_REG("DEBUG_MODE1", reg, wdata, rdata);
            SNAUD_REG_WRITE_PASS("DEBUG_MODE1");
        
            break;
        default :
            printf("DEBUG_MODE_NULL\n\r");
            break;
    }

    return AUD_PASS;
}
void AUD_Set_DACL_Volume(unsigned short index)
{
  uint8_t ret;
  uint16_t rdata;
  rew1:
  Delay(100);
  AUD_Reg.Left_Dac_Volume.Word = index | 0x100;
   // ret = AUD_RegWrite( Left_Dac_Volume, AUD_Reg.Left_Dac_Volume.Word );
   // ret = AUD_RegRead( Left_Dac_Volume, &rdata );
   ret = AUD_RegWrite(DAC_GAIN,AUD_Reg.Left_Dac_Volume.Word );
   ret = AUD_RegRead(DAC_GAIN, &rdata);
    if(rdata!=index)
    {
        goto rew1;
    } 
}

void AUD_Set_DACR_Volume(unsigned short index)
{
  uint8_t ret;
  uint16_t rdata;
  rew:
  Delay(100);
  AUD_Reg.Right_DAC_Volume.Word = index | 0x100;
    //ret = AUD_RegWrite( Right_DAC_Volume, AUD_Reg.Right_DAC_Volume.Word );
   // ret = AUD_RegRead( Right_DAC_Volume, &rdata );
	   ret = AUD_RegWrite(AMP_CTRL,0xFD );
   ret = AUD_RegRead(AMP_CTRL, &rdata);
   ret = AUD_RegWrite(DAC_GAIN,AUD_Reg.Right_DAC_Volume.Word );
   ret = AUD_RegRead(DAC_GAIN, &rdata);

    if(rdata!=index)
    {
        goto rew;
    } 
}
uint16_t  set_vloume(uint16_t lv,uint16_t rv)
{
	uint8_t ret;
    uint16_t rdata;
	rew1:
	Delay(100);
	AUD_Reg.Left_Dac_Volume.Word = lv | 0x100;
    ret = AUD_RegWrite( Left_Dac_Volume, AUD_Reg.Left_Dac_Volume.Word );
    ret = AUD_RegRead( Left_Dac_Volume, &rdata );
    if(rdata!=lv)
    {
        goto rew1;
    }		
	rew:
	Delay(100);
	AUD_Reg.Right_DAC_Volume.Word = rv | 0x100;
    ret = AUD_RegWrite( Right_DAC_Volume, AUD_Reg.Right_DAC_Volume.Word );
    ret = AUD_RegRead( Right_DAC_Volume, &rdata );
    if(rdata!=rv)
    {
        goto rew;
    }	
}














