
#ifndef _AUD_DRIVER_H
#define _AUD_DRIVER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "snc_types.h"
#include "snc_i2c.h"
#include "I2C_Software_Master.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum AUD_ret
{
    AUD_PASS = 0,
    AUD_FAIL,
}AUD_ret_t;

typedef enum AUD_ID
{
    AUD01_ID        = 0x19,
    AUDL_ID         = 0x1A,
    AUDL_REVISION   = 0x07F,
}AUD_ID_t;

typedef enum aud_driver_modde
{
    AUD_NU = 0,
    AUD_01,
    AUD_NU_SW,
    AUD_01_SW,
}aud_driver_modde_t;

/* AUD Register */
typedef enum AUD_RegTable 
{                                                   
    /*=== Basic ===*/
    Software_Reset=0,                   /* Offset 0 */                                                            
    Power_Management1,                  /* Offset 1 */                                                                     
    Power_Management2,                  /* Offset 2 */  
    Power_Management3,                  /* Offset 3 */  
    
    /*=== General Audio Controls ===*/
    Audio_Interface,                    /* Offset 4 */
    Companding,                         /* Offset 5 */
    Clock_Control1,                     /* Offset 6 */
    Clock_Control2,                     /* Offset 7 */
    GPIO,                               /* Offset 8 */
    Jack_Detect1,                       /* Offset 9 */
    
    DAC_Control,                        /* Offset 10 */
    Left_Dac_Volume,                    /* Offset 11 */
    Right_DAC_Volume,                   /* Offset 12 */
    Jack_Detect2,                       /* Offset 13 */
    ADC_Control,                        /* Offset 14 */
    Left_ADC_Volume,                    /* Offset 15 */
    Right_ADC_Volume,                   /* Offset 16 */
    Reserved00,                         /* Offset 17 */
    
    /*=== Equalizer ===*/
    EQ1_low_cutoff,                     /* Offset 18 */
    EQ2_peak1,                          /* Offset 19 */
    EQ3_peak2,                          /* Offset 20 */
    EQ4_peak3,                          /* Offset 21 */
    EQ5_high_cutoff,                    /* Offset 22 */
    Reserved01,                         /* Offset 23 */
    
    /*=== DAC Limiter ===*/
    DAC_Limiter1,                       /* Offset 24 */
    DAC_Limiter2,                       /* Offset 25 */
    Reserved02,                         /* Offset 26 */
    
    /*=== Notch Filter ===*/
    Notch_Filter1,                      /* Offset 27 */
    Notch_Filter2,                      /* Offset 28 */
    Notch_Filter3,                      /* Offset 29 */
    Notch_Filter4,                      /* Offset 30 */
    Reserved03,                         /* Offset 31 */
    
    /*=== ALC and Noise Gate Control ===*/
    ALC_Control1,                       /* Offset 32 */
    ALC_Control2,                       /* Offset 33 */
    ALC_Control3,                       /* Offset 34 */
    Noise_Gate,                         /* Offset 35 */
    
    /*=== Phase Locked Loop ===*/
    PLL_N,                              /* Offset 36 */
    PLL_K1,                             /* Offset 37 */
    PLL_K2,                             /* Offset 38 */
    PLL_K3,                             /* Offset 39 */
    Reserved04,                         /* Offset 40 */
    
    /*=== Miscellaneous ===*/
    _3D_Control,                        /* Offset 41 */
    Reserved05,                         /* Offset 42 */
    Right_Speaker_Submix,               /* Offset 43 */
    Input_Control,                      /* Offset 44 */
    Left_Input_PGA_Gain,                /* Offset 45 */
    Right_Input_PGA_Gain,               /* Offset 46 */
    Left_ADC_Boost,                     /* Offset 47 */
    Right_ADC_Boost,                    /* Offset 48 */
    Output_Control,                     /* Offset 49 */
    Left_Mixer,                         /* Offset 50 */
    Right_Mixer,                        /* Offset 51 */
    LHP_Volume,                         /* Offset 52 */
    RHP_Volume,                         /* Offset 53 */
    LSPKOUT_Volume,                     /* Offset 54 */
    RSPKOUT_Volume,                     /* Offset 55 */
    AUX2_Mixer,                         /* Offset 56 */
    AUX1_Mixer,                         /* Offset 57 */
    
    /*=== Begin AUDL Proprietary Register Space ===*/
    Power_Management4,                  /* Offset 58 */
    
    /*=== PCM Time Slot and ADCOUT Impedance Option Control ===*/
    Left_Time_Slot,                     /* Offset 59 */
    Misc,                               /* Offset 60 */
    Right_Time_Slot,                    /* Offset 61 */
    
    /*=== Silicon Revision and Device ID ===*/
    Device_Revision,                    /* Offset 62 */
    Device_ID,                          /* Offset 63 */
    ALC_Enhancements1=70,               /* Offset 70 */
    ALC_Enhancements2,                  /* Offset 71 */
    _192kHz_Sampling,                   /* Offset 72 */
    Misc_Control,                       /* Offset 73 */
    Tie_Off_Overrides,                  /* Offset 74 */
    Power_Tie_off_Ctrl,                 /* Offset 75 */
    P2P_Detector_Read,                  /* Offset 76 */
    Peak_Detector_Read,                 /* Offset 77 */
    Control_And_Status,                 /* Offset 78 */
    Output_tie_off_control,             /* Offset 79 */
    SPI1=87,                            /* Offset 87 */
    SPI2=108,                           /* Offset 108 */
    SPI3=115,                           /* Offset 115 */
    
} AUD_RegTable_t;

/* AUD01 reg table*/
typedef enum AUD01_RegTable 
{
    /*=== System ===*/                               
    SYS_CLK_CTRL = 0x00,            /* Offset 0x00 */
    FPLL_CTRL,                      /* Offset 0x01 */
    LDO_CTRL,                       /* Offset 0x02 */
    PMU_CTRL,                       /* Offset 0x03 */
    
    /*=== I2S Interface ===*/       
    I2S_ADC_CTRL0 = 0x08,           /* Offset 0x08 */
    I2S_ADC_CTRL2,                  /* Offset 0x09 */
    I2S_DAC_CLKSRC,                 /* Offset 0x0A */
    I2S_DAC_CTRL2,                  /* Offset 0x0B */

    /*=== I2C Interface ===*/      
    I2C_ALT_ID_EN = 0x0F,           /* Offset 0x0F */

    /*=== ADC ===*/ 
    ADC_MIC_CTRL = 0x10,            /* Offset 0x10 */
    ADC_CTRL0,                      /* Offset 0x11 */
    ADC_CTRL1,                      /* Offset 0x12 */
    ADC_CTRL2,                      /* Offset 0x13 */
    ADC_ZCU_CTRL,                   /* Offset 0x14 */
    ADC_PGA_R,                      /* Offset 0x15 */
    ADC_FILTER_R,                   /* Offset 0x16 */
    ADC_PGA_L,                      /* Offset 0x17 */
    ADC_FILTER_L,                   /* Offset 0x18 */
    ADC_Test,                       /* Offset 0x19 */

    /*=== DAC ===*/ 
    DAC_CTRL0 = 0x30,               /* Offset 0x30 */
    DAC_CTRL1,                      /* Offset 0x31 */
    DAC_PD,                         /* Offset 0x32 */
    DAC_GAIN,                       /* Offset 0x33 */
    DAC_DEEMP,                      /* Offset 0x34 */
    DAC_MUTE,                       /* Offset 0x35 */
    
    /*=== Audio AMP ===*/ 
    AMP_CTRL = 0x40,                /* Offset 0x40 */

    /*=== Test Mode ===*/       
    DEBUG_MODE0 = 0x50,             /* Offset 0x50 */
    DEBUG_MODE1,                    /* Offset 0x51 */
    GPIO_IO_MODE,                   /* Offset 0x52 */
    GPIO_IO_CHG,                    /* Offset 0x53 */
    GPIO_DAT,                       /* Offset 0x54 */
    
    /*=== ISO ===*/ 
    DAC_ISO1 = 0x60,                /* Offset 0x60 */
    DAC_ISO2,                       /* Offset 0x61 */
    DAC_ISO3,                       /* Offset 0x62 */

} AUD01_RegTable_t;


/* AUD Register Bit Group */
typedef struct AUD_RegBit
{
    union   /* Offset 0 */        
    {
        uint16_t Word;
        struct
        {
            uint16_t Reserved       ;
        }Bit;
    }Software_Reset;
    
    union   /* Offset 1 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t REFIMP     :2;
            uint16_t IOBUFEN    :1;
            uint16_t ABIASEN    :1;
            uint16_t MICBIASEN  :1;
            uint16_t PLLEN      :1;
            uint16_t AUX2MXEN   :1;
            uint16_t AUX1MXEN   :1;
            uint16_t DCBUFEN    :1;
        }Bit;
    }Power_Management1;
    
    union   /* Offset 2 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t LADCEN     :1;
            uint16_t RADCEN     :1;
            uint16_t LPGAEN     :1;
            uint16_t RPGAEN     :1;
            uint16_t LBSTEN     :1;
            uint16_t RBSTEN     :1;
            uint16_t SLEEP      :1;
            uint16_t LHPEN      :1;
            uint16_t RHPEN      :1;
        }Bit;
    }Power_Management2;

    union   /* Offset 3 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t LDACEN     :1;
            uint16_t RDACEN     :1;
            uint16_t LMIXEN     :1;
            uint16_t RMIXEN     :1;
            uint16_t RESERVED   :1;
            uint16_t RSPKEN     :1;
            uint16_t LSPKEN     :1;
            uint16_t AUXOUT2EN  :1;
            uint16_t AUXOUT1EN  :1;
        }Bit;
    }Power_Management3;
    
    union   /* Offset 4 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t MONO       :1;
            uint16_t ADCPHS     :1;
            uint16_t DACPHS     :1;
            uint16_t AIFMT      :2;
            uint16_t WLEN       :2;
            uint16_t LRP        :1;
            uint16_t BCLKP      :1;
        }Bit;
    }Audio_Interface;
    
    union   /* Offset 5 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t ADDAP      :1;
            uint16_t ADCCM      :2;
            uint16_t DACCM      :2;
            uint16_t CMB8       :1;
        }Bit;
    }Companding;
    
    union   /* Offset 6 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t CLKIOEN    :1;
            uint16_t RESERVED   :1;
            uint16_t BCLKSEL    :3;
            uint16_t MCLKSEL    :3;
            uint16_t CLKM       :1;
        }Bit;
    }Clock_Control1;
    
    union   /* Offset 7 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t SCLKEN     :1;
            uint16_t SMPLR      :3;
            uint16_t RESERVED   :4;
            uint16_t _4WSPIEN   :1;
        }Bit;
    }Clock_Control2;
    
    union   /* Offset 8 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t GPIO1SEL   :3;
            uint16_t GPIO1PL    :1;
            uint16_t GPIO1PLL   :2;
            uint16_t RESERVED   :3;
        }Bit;
    }GPIO;
    
    union   /* Offset 9 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t RESERVED   :4;
            uint16_t JCKDIO     :1;
            uint16_t JACDEN     :1;
            uint16_t JCKMIDEN   :2;
        }Bit;
    }Jack_Detect1;
    
    union   /* Offset 10 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t LDACPL     :1;
            uint16_t RDACPL     :1;
            uint16_t AUTOMT     :1;
            uint16_t DACOS      :2;
            uint16_t RESERVED00 :2;
            uint16_t SOFTMT     :1;
            uint16_t RESERVED01 :2;
        }Bit;
    }DAC_Control;
    
    union   /* Offset 11 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t LDACGAIN   :8;
            uint16_t LDACVU     :1;
        }Bit;
    }Left_Dac_Volume;
    
    union   /* Offset 12 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t RDACGAIN   :8;
            uint16_t RDACVU     :1;
        }Bit;
    }Right_DAC_Volume;
    
    union   /* Offset 13 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t JCKDONE0   :4;
            uint16_t JCKDONE1   :4;
            uint16_t RESERVED   :1;
        }Bit;
    }Jack_Detect2;
    
    union   /* Offset 14 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t LADCPL     :1;
            uint16_t RADCPL     :1;
            uint16_t RESERVED   :1;
            uint16_t ADCOS      :1;
            uint16_t HPF        :3;
            uint16_t HPFAM      :1;
            uint16_t HPFEN      :1;
        }Bit;
    }ADC_Control;
    
    union   /* Offset 15 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t LADCGAIN   :8;
            uint16_t LADCVU     :1;
        }Bit;
    }Left_ADC_Volume;
    
    union   /* Offset 16 */   
    {
        uint16_t Word;
        struct
        {
            uint16_t RADCGAIN   :8;
            uint16_t RADCVU     :1;
        }Bit;
    }Right_ADC_Volume;
    
    /* Offset 17 ~ 115 Pending */
//    union   /* Offset AAA */   
//    {
//        uint16_t WWW;
//        struct
//        {
//            uint16_t AA        :1;
//            uint16_t BB        :1;
//            uint16_t CC        :1;
//            uint16_t DD        :2;
//            uint16_t EE        :2;
//            uint16_t FF        :1;
//            uint16_t GG        :1;
//        }Bit;
//    }AAAAAAAA;
    
} AUD_RegBit_s;

typedef enum {
	AUD_SR8K		= 8000U,
	AUD_SR11p025K	= 11025U,
	AUD_SR12K		= 12000U,
	AUD_SR16K		= 16000U,
	AUD_SR22p05K	= 22050U,
	AUD_SR24K		= 24000U,
	AUD_SR32K		= 32000U,
	AUD_SR44p1K	    = 44100U,
	AUD_SR48K		= 48000U,
	AUD_SR64K		= 64000U,
	AUD_SR88p2K	    = 88200U,
	AUD_SR96K		= 96000U
} aud_samplerate_t;

typedef enum aud_mclk_path
{
	MCLK_PAD_TO_SYS = 0,
	MCLK_PAD_TO_PLL,
} aud_mclk_path_t;

typedef enum aud_ms
{
	SLAVE_MODE = 0,
	MASTER_MODE,
} aud_ms_t;

typedef enum aud_ch
{
	AUD0 = 0,   /* mapping to i2c0 */
	AUD1,       /* mapping to i2c2 */
	AUD2,       /* mapping to i2c1 */
} aud_ch_t;

typedef enum AUD01_DEBUG_MODE
{
    DEBUG_MODE0_NONE=0,                     /* 0 */
    DEBUG_MODE0_DAC_L_ANALOG,               /* 1 */
    DEBUG_MODE0_DAC_R_ANALOG,               /* 2 */
    DEBUG_MODE0_ADC_L_ANALOG,               /* 3 */
    DEBUG_MODE0_ADC_R_ANALOG,               /* 4 */
    DEBUG_MODE0_ADC_DAC_LOOPBACK,           /* 5 */
    DEBUG_MODE0_HP_L,                       /* 6 */
    DEBUG_MODE0_HP_R,                       /* 7 */
    DEBUG_MODE0_GPIO,                       /* 8 */
    DEBUG_MODE0_I2C_TEST1,                  /* 9 */
    DEBUG_MODE0_I2C_TEST2,                  /* 10 */
    DEBUG_MODE0_LDO18_CHOPPER,              /* 11 */
    DEBUG_MODE1_DIGITAL,                    /* 12 */
}AUD01_DEBUG_MODE_t;

typedef enum aud_dac_type
{
    AC_COUPLED = 0,
    CAPLESS,
}aud_dac_type_t;

typedef enum aud_dac_depop
{
    AUD_NOPOP_0 = 0,
    AUD_NOPOP_1,
    AUD_NOPOP_2,
    AUD_NOPOP_3,
    AUD_NOPOP_4,
    AUD_NOPOP_5,
    AUD_NOPOP_6,
    AUD_NOPOP_7,
    AUD_NOPOP_8,    /* NOPOP_VREFSPK */
}aud_dac_depop_t;

typedef enum mclk_pad_output
{
    MCLK_OUTPUT_IHRC = 0,
    MCLK_OUTPUT_XTAL,
    MCLK_OUTPUT_I2S4,
}mclk_pad_output_t;

typedef enum ADC_Boost
{
    p0dB = 0,
    p20dB,
    p40dB,
    p50dB,
}ADC_Boost_t;

typedef enum  ADC_Gain
{
    m12dB=0,
    m11p25dB,
    m10p5dB,
    m9p75dB,
    m8p25dB,
    m7p5dB,
    m6p75dB,
    m6pdB,
    m5p25dB,
    m4p5dB,
    m3p75dB,
    m3dB,
    m2p25dB,
    m1p5dB,
    m0p75dB,
    mp0dB,
    p0p75dB,
    p1p5dB,
    p2p25dB,
    p3dB,
    p3p75dB,
    p4p5dB,
    p5p25dB,
    p6p00dB,
    p6p75dB,
    p7p5dB,
    p8p25dB,
    p9p00dB,
    p9p75dB,
    p10p5dB,
    p11p25dB,
    p12dB,
    p12p75dB,
    p13p5dB,
    p14p25dB,
    p15dB,
    p15p75dB,
    p16p5dB,
    p17p25dB,
    p18dB,
    p18p75dB,
    p19p5dB,
    p20p25dB,
    p21pdB,
    p21p75dB,
    p22p5dB,
    p23p25dB,
    p24pdB,
    p24p75dB,
    p25p5dB,
    p26p25dB,
    p27pdB,
    p27p75dB,
    p28p5dB,
    p29p25dB,
    p30pdB,
    p30p75dB,
    p31p5dB,
    p32p25dB,
    p33pdB,
    p33p75dB,
    p34p5dB,
    p35p25dB,
}ADC_Gain_t;    

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
extern AUD_ret_t AUD_Driver_Init(i2c_id_t i2c_id);
extern AUD_ret_t AUD_SlaveMode_Init(void);
extern AUD_ret_t AUD_MasterMode_Init(aud_samplerate_t sr, aud_mclk_path_t mclk_path, bool gp1_clk);
extern AUD_ret_t AUD_RegWrite(uint8_t RegAddr, uint16_t Data);
extern AUD_ret_t AUD_RegRead(uint8_t RegAddr, uint16_t *Data);
extern void AUD_mclk_enable(mclk_pad_output_t sel);
extern void AUD_mclk_disable(void);
extern AUD_ret_t AUD_DAC_Volume(uint16_t L_DAC_Volume, uint16_t R_DAC_Volume);
extern AUD_ret_t AUD_ADC_Gain_Setting(uint16_t L_ADC_Boost_s, uint16_t L_ADC_Gain, uint16_t R_ADC_Boost_s, uint16_t R_ADC_Gain);
extern AUD_ret_t AUD_Get_DAC_Volume(uint16_t* Volume);

//=============================================================================
//                  AUD01 TEST API (don't touch)
//=============================================================================
/**
 *  \brief  set shaud01 init as master mode , dac & adc will turn on
 *
 *  \param [in] eSamplingRate   enum eSamplingRate
 *  \param [in] mclk_path       mclk path for aud01
 *  \param [in] gpio_clk        sel output aud01 sys clk or not
 *  \param [in] dac_type        dac circuit definition , there will appear pop noise if dac_type and pcb circuit incompatible 
 *  \return                     AUD_PASS: ok, other: fail
 *
 *  \details
 */
AUD_ret_t
SNAUD01_MasterMode_Init( 
    aud_samplerate_t        eSamplingRate, 
    aud_mclk_path_t         mclk_path, 
    bool                    gpio_clk,
    aud_dac_type_t          dac_type,
    aud_dac_depop_t         dac_depop, 
    uint32_t                dac_charge_delay,
    bool                    amp_on);

    
/**
 *  \brief  list all snaud01 register value
 *
 *  \return                     AUD_PASS: ok, other: fail
 *
 *  \details
 */
AUD_ret_t 
SNAUD01_list_all_reg(void);

    
/**
 *  \brief  set shaud01 enter test mode
 *
 *  \param [in] aud_debug_mode  enum aud_debug_mode
 *  \return                     AUD_PASS: ok, other: fail
 *
 *  \details
 */
AUD_ret_t 
SNAUD01_debug_mode(
AUD01_DEBUG_MODE_t aud_debug_mode);
    
    
/**
 *  \brief  set snaud01 enter deep power down mode
 *
 *  \return                     AUD_PASS: ok, other: fail
 *
 *  \details
 */                              
AUD_ret_t 
SNAUD01_power_down(aud_dac_type_t dac_type);


/**
 *  \brief  set snaud01 enter sleep mode
 *
 *  \return                     AUD_PASS: ok, other: fail
 *
 *  \details
 */  
AUD_ret_t 
SNAUD01_sleep(aud_dac_type_t dac_type);


//=============================================================================
//                  AUD01 SYS API
//=============================================================================
/**
 *  \brief      Set aud01 enter deep power down mode
 *
 *  \return     AUD_PASS: ok, other: fail
 *
 *  \details
 */      
AUD_ret_t 
AUD01_dpd(void);


//=============================================================================
//                  AUD01 ADC API
//=============================================================================
extern void AUD_Set_Driver_Mode(aud_driver_modde_t eDriverMode);
extern uint32_t AUD01_ADC_Turn_On(aud_ms_t ms_mode,aud_mclk_path_t mclk_path ,aud_samplerate_t sample_rate);
extern AUD_ret_t AUD01_ADC_Turn_Off(void);
extern AUD_ret_t AUD01_ADC_Gain_Ctrl(ADC_Boost_t L_ADC_Boost_s, ADC_Gain_t L_ADC_Gain, ADC_Boost_t R_ADC_Boost_s, ADC_Gain_t R_ADC_Gain);


//=============================================================================
//                  AUD01 DAC API
//=============================================================================
/**
 *  \brief      Turn on DAC
 *
 *  \param      [in] aud_mode       set aud01 as master or slave mode
 *  \param      [in] mclk_def       define aud01 mclk is sys clk or pll input clk
 *  \param      [in] dac_sr         DAC sample Rate
 *  \param      [in] dac_type       DAC headphone circuit type
 *  \param      [in] dac_depop      DAC headphone depop configuration
 *  \param      [in] dac_delay      DAC headphone charge delay , Long delay is good for depop
 *  \return     AUD_PASS: ok, other: fail
 *
 *  \details    
 */
AUD_ret_t 
AUD01_DAC_Turn_On(
    aud_ms_t            aud_mode,
    aud_mclk_path_t     mclk_def,
    aud_samplerate_t    dac_sr,
    aud_dac_type_t      dac_type, 
    aud_dac_depop_t     dac_depop, 
    uint32_t            dac_delay);


/**
 *  \brief      Turn dac off
 *
 *  \param      
 *  \return     AUD_PASS: ok, other: fail
 *
 *  \details    
 */
AUD_ret_t 
AUD01_DAC_Turn_Off(
    aud_dac_type_t      dac_type, 
    bool                weakly_drv);


/**
 *  \brief      Set dac volmue
 *
 *  \param      [in] vol       dac vol range is 0 ~ 255
 *  \return     AUD_PASS: ok, other: fail
 *
 *  \details    
 */
AUD_ret_t 
AUD01_DAC_Volume_Control(uint8_t vol);


/**
 *  \brief      Turn on amp
 *
 *  \param      [in] vol       amp vol range is 0 ~ 31
 *  \return     AUD_PASS: ok, other: fail
 *
 *  \details    
 */
AUD_ret_t 
AUD01_AMP_Turn_On(uint8_t vol);


/**
 *  \brief      Turn off amp
 *
 *  \param      
 *  \return     AUD_PASS: ok, other: fail
 *
 *  \details    
 */
AUD_ret_t 
AUD01_AMP_Turn_Off(void);


#endif














