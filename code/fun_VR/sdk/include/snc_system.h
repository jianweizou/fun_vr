/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file snc_system.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/14
 * @license
 * @description
 */

#ifndef __snc_system_H_wrTKUQfS_lMlX_Hdwt_sNOc_uH6ULAirnWAx__
#define __snc_system_H_wrTKUQfS_lMlX_Hdwt_sNOc_uH6ULAirnWAx__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  System PLL ctrl
 */
typedef enum system_pll_ctrl
{
    SYSTEM_PLL_LPF_OFF,
    SYSTEM_PLL_EN,
    SYSTEM_PLL_CLK_SEL,
    SYSTEM_PLL_TCK_SEL,
    SYSTEM_PLL_HXTALEN,
    SYSTEM_PLL_ICP_SEL,
    SYSTEM_PLL_DLY_SEL,
    SYSTEM_PLL_IHRCEN,
    SYSTEM_PLL_FRA_L,
    SYSTEM_PLL_IVCO_SEL,
    SYSTEM_PLL_SDM_EN,
    SYSTEM_PLL_VLD_IN,
    SYSTEM_PLL_INT,
    SYSTEM_PLL_FRA_H,

} system_pll_ctrl_t;

/**
 *  system clock type
 */
typedef enum system_clock
{
    SYSTEM_CLOCK_IHRC   = 0,
    SYSTEM_CLOCK_ILRC,
    SYSTEM_CLOCK_HXTAL,
    SYSTEM_CLOCK_LXTAL,
    SYSTEM_CLOCK_SPLL,

} system_clock_t;

/**
 *  system clock dividing
 */
typedef enum system_clock_div
{
    SYSTEM_CLOCK_DIV_1      = 0,
    SYSTEM_CLOCK_DIV_2,
    SYSTEM_CLOCK_DIV_4,
    SYSTEM_CLOCK_DIV_8,
    SYSTEM_CLOCK_DIV_16,
    SYSTEM_CLOCK_DIV_32,
    SYSTEM_CLOCK_DIV_64,
    SYSTEM_CLOCK_DIV_128,
} system_clock_div_t;

/**
 *  system reset type
 */
typedef enum system_reset_type
{
    SYSTEM_RESET_SW   = 0,
    SYSTEM_RESET_WDT,
    SYSTEM_RESET_LVR,
    SYSTEM_RESET_DPD,
    SYSTEM_RESET_POR,

} system_reset_type_t;

typedef enum system_pin_set
{
    SYSTEM_PIN_SET_USB_TSQU_SEL,
    SYSTEM_PIN_SET_USB_DBG_MODE,
    SYSTEM_PIN_SET_HOST_DEV_SEL,
    SYSTEM_PIN_SET_AUDIO_IF2,
    SYSTEM_PIN_SET_I2C_SYNC_SW,
    SYSTEM_PIN_SET_PWMIO,
    SYSTEM_PIN_SET_OID_SENSOR,
    SYSTEM_PIN_SET_SPIDMA0,
    SYSTEM_PIN_SET_SWDV_1,
    SYSTEM_PIN_SET_SWDV_0,
    SYSTEM_PIN_SET_I2S4,
    SYSTEM_PIN_SET_HOST_PPC,
    SYSTEM_PIN_SET_TFT_8080,
    SYSTEM_PIN_SET_SDNF0,

} system_pin_set_t;


typedef enum system_ndt_time
{
    SYSTEM_NDT_250US    = 0,
    SYSTEM_NDT_500US,
    SYSTEM_NDT_1000US,
    SYSTEM_NDT_2000US,
} system_ndt_time_t;

/**
 *  system re-map start address
 */
typedef enum system_remap
{
    SYSTEM_REMAP_TO_ROM     = 0,
    SYSTEM_REMAP_TO_PRAM

} system_remap_t;

/**
 *  module support ddr
 */
typedef enum system_ddr_user
{
    SYSTEM_DDR_USER_DMA1_OUT    = 0,
    SYSTEM_DDR_USER_DMA1_IN,
    SYSTEM_DDR_USER_JPEG_D_IN,
    SYSTEM_DDR_USER_JPEG_E_OUT,
    SYSTEM_DDR_USER_DMA0_OUT,
    SYSTEM_DDR_USER_DMA0_IN,
    SYSTEM_DDR_USER_CIS,
    SYSTEM_DDR_USER_PPU,

} system_ddr_user_t;

/**
 *  ddr channel select
 */
typedef enum system_ddr_channel
{
    SYSTEM_DDR_CH_NONE      = 0,
    SYSTEM_DDR_CH_0,
    SYSTEM_DDR_CH_1,
    SYSTEM_DDR_CH_2,

} system_ddr_channel_t;


/**
 *  system APB (Advanced Peripheral Bus) bus divide
 */
typedef enum system_apb_div
{
    SYSTEM_APB_DIV_1    = 0,
    SYSTEM_APB_DIV_2,
    SYSTEM_APB_DIV_4,
    SYSTEM_APB_DIV_8,
    SYSTEM_APB_DIV_16,

} system_apb_div_t;

/**
 *  system SD/Nand divide
 */
typedef enum system_sd_nf_div
{
    SYSTEM_SD_NF_DIV_1  = 0,
    SYSTEM_SD_NF_DIV_2,
    SYSTEM_SD_NF_DIV_4,
    SYSTEM_SD_NF_DIV_8,

} system_sd_nf_div_t;


typedef enum system_dev
{
    SYSTEM_DEV_NONE       = 0,

    SYSTEM_DEV_NUM
} system_dev_t;

/**
 *  system LVD settings
 */

typedef enum system_lvd_ctrl
{
    SYSTEM_LVD_11EN       = 0,
    SYSTEM_LVD_33EN,
    SYSTEM_LVD_POW_DOWN,
    SYSTEM_LVD_33_VAL,
    SYSTEM_LDO_ILRC_SEL,
    SYSTEM_LVD_33_FLAG,
    SYSTEM_LVD_EFUSE_ERR,
    SYSTEM_LVD_NUM
} system_lvd_ctrl_t;


/**
 *  system LVD settings
 */

typedef enum system_ram_pd
{
    SYSTEM_PD_SRAM0,
    SYSTEM_PD_SRAM1,
    SYSTEM_PD_SRAM2,
    SYSTEM_PD_SRAM3,
    SYSTEM_PD_SRAM4,
    SYSTEM_PD_SRAM5,
    SYSTEM_PD_SRAM6,
    SYSTEM_PD_SRAM7,
    
    SYSTEM_PD_MAIL_BOX,
    SYSTEM_PD_CACHE,
    SYSTEM_PD_USB_H1,
    SYSTEM_PD_USB_H2,
    SYSTEM_PD_USB_DEV,
    SYSTEM_PD_USB_PPU,
    SYSTEM_PD_USB_CSC,
    SYSTEM_PD_USB_JPEG,
} system_ram_pd_t;

//=============================================================================
//                  Macro Definition
//=============================================================================
#define System_HTXL_PLL_162MHZ()                                                                \
{                                                                                               \
    *(uint32_t*)0x45000000 |= (1<<4) | (1<<14);     /* Enable HTXL, PLL from HTXL */            \
    while( ! (*(uint32_t*)0x45000008 & (1<<4)) );   /* Wait HTXL stable */                      \
    *(uint32_t*)0x45000000 |= (1<<15);               /* Enable PLL */                            \
    while( ! (*(uint32_t*)0x45000008 & (1<<6)) );   /* Wait PLL stable */                       \
    *(uint32_t*)0x4500000C = 4;                     /* Sys clk form PLL */                      \
    while( *(uint32_t*)0x4500000C != 0x44 );        /* Wait switch source of Sys clk  */        \
}


#define System_IHRC_PLL_162MHZ()                                                                  \
{                                                                                               \
    *(uint32_t*)0x45000000 |= 1;                    /* Enable IHRC */                           \
    while( ! (*(uint32_t*)0x45000008 & 1) );        /* Wait IHRC stable */                      \
    *(uint32_t*)0x45000000 |= (1<<15);               /* Enable PLL */                            \
    while( ! (*(uint32_t*)0x45000008 & (1<<6)) );   /* Wait PLL stable */                       \
    *(uint32_t*)0x4500000C = 4;                     /* Sys clk form PLL */                      \
    while( *(uint32_t*)0x4500000C != 0x44 );        /* Wait switch source of Sys clk  */        \
}



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

/**
 *  \brief: System_Clock_Enable
 *  
 *  \param [in] clk: clock source.
 *  \return:   0- Success, else- fail.
 *  
 *  \details: Enables clock sources, should go with 
 *            function "System_Get_Clock_Status" to check if the clock stable.
 *  
 */
int 
System_Clock_Enable(system_clock_t clk);


/**
 *  \brief: System_Clock_Disable
 *  
 *  \param [in] clk: clock_type: types defined in system_clock_t.
 *  \return:   0- Success, else- fail.
 *  
 *  \details: Do not turn off the system source clock.
 *  
 */
int 
System_Clock_Disable(system_clock_t clk);


/**
 *  \brief: System_Get_Clock_Status
 *  
 *  \param [in] clock_type: types defined in system_clock_t.
 *  \return:    1- clock stable, else clock unstable.
 *  
 *  \details:
 *  
 */
int
System_Get_Clock_Status(system_clock_t  clock_type);


/**
 *  \brief: System_PLL_Sel
 *  
 *  \param [in] clk:  SYSTEM_CLOCK_IHRC/SYSTEM_CLOCK_HXTAL.
 *  \return:    0- Success, else- fail.
 *  
 *  \details:
 *  
 */
int
System_PLL_Sel(system_clock_t clk);


/**
 *  \brief      Set the pll-system clock to given clock frequency.
 *
 *  \return     -1: Fail, else System clk in hz.
 *
 *  \details
 */
int
System_Set_SysClk(uint32_t Clk);


/**
 *  \brief      Return current system clock.
 *
 *  \return     0: Fail, Else System clk in hz.
 *
 *  \details
 */
uint32_t
System_Get_SysClk(void);



 /**
 *  \brief      switch system clock
 *
 *  \param [in] clock_type      target clock type
 *  \param [in] clock_div       clock divider (pre-scaler)
 *  \return     0: ok, other: fail
 *
 *  \details
 */
int
System_Clock_Switch(
    system_clock_t          clock_type,
    system_clock_div_t      clock_div);
 
 
int
System_LVD_Ctrl(
    system_lvd_ctrl_t   ctrl_type,
    uint32_t            value);

/**
 *  \brief  Remap system start memory address
 *
 *  \param [in] remap_type      enum system_remap
 *  \return
 *
 *  \details
 */

int
System_Pin_Ctrl(
    system_pin_set_t    pin_type,
    uint32_t            value);


int
System_Set_NDT_Time(
    system_ndt_time_t   ndt_time);



int
System_Ram_Power_Down(uint32_t ram);


int
System_DRAM_Channel_Sel(
    system_ddr_user_t       user,
    system_ddr_channel_t    channel);

int
System_Clock_Peripheral(uint32_t setting);


int
System_Clock_APB_Divide(
    system_apb_div_t    clock_div);


int
System_Clock_SD_NF_Divide(
    system_sd_nf_div_t    clock_div);


#ifdef __cplusplus
}
#endif

#endif
