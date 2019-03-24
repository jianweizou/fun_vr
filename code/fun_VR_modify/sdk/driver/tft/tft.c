/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file tft.c
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/02/23
 * @license
 * @description
 */

#include "snc_tft.h"
#include "reg_util.h"
#include "register_7320.h"
#define TFT_VERSION        0x73200000

//=============================================================================
//                  Constant Definition
//=============================================================================

#define BIT_TFT_MODE        0
#define BIT_SEQ_052         4
#define BIT_EVEN_SEQ        6
#define BIT_ODD_SEQ         9
#define BIT_DCLK_POL        11
#define BIT_HS_POL          12
#define BIT_VS_POL          13
#define BIT_PAR_OPT         14
#define BIT_TFT_EN          15

#define BIT_HBLK            0
#define BIT_VBLK            1

#define BIT_HBLK_STA        0
#define BIT_VBLK_STA        1

#define H_DISP_Paralle      0
#define H_DISP_Serial       2
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

/**
 *  \brief  tft display mode setting
 *
 *  \param [in] mode     enum tft_display_mode_t
 *  \return              0: ok, other: fail
 *
 *  \details
 */
static int
_TFT_Display_Mode_Setting(
    sn_tft_t *pDev,
    uint8_t mode)
{
    int rval = 0;
    
    //TODO : Check if these mode is necessary
    // Set display mode   
    if(mode == UPS_051_SERIAL)
    {
        /* Default : Serial(UPS051) */
        reg_clear_bit(&pDev->TFT_CTRL, BIT_PAR_OPT);    // Serial Mode
        reg_clear_bit(&pDev->TFT_CTRL, BIT_TFT_MODE);   // UPS051
        reg_write_bits(&pDev->TFT_HSYNC_DISPLAY, H_DISP_Serial);

    }
    else if(mode == UPS_052_SERIAL)
    {
        /* Serial(UPS052) */
        reg_clear_bit(&pDev->TFT_CTRL, BIT_PAR_OPT);    // Serial Mode
        reg_set_bit(&pDev->TFT_CTRL, BIT_TFT_MODE);     // UPS052
        reg_write_bits(&pDev->TFT_HSYNC_DISPLAY, H_DISP_Serial);
    }
    else if (mode == UPS_051_PARALLEL)
    {
        /* Parallel(UPS051) */
        reg_set_bit(&pDev->TFT_CTRL, BIT_PAR_OPT);      // Parallel Mode
        reg_clear_bit(&pDev->TFT_CTRL, BIT_TFT_MODE);   // UPS051
        reg_write_bits(&pDev->TFT_HSYNC_DISPLAY, H_DISP_Paralle);
    }
    
    else if(mode == UPS_052_PARALLEL)
    {
        /* Parallel(UPS052) */
        reg_set_bit(&pDev->TFT_CTRL, BIT_PAR_OPT);      // Parallel Mode
        reg_set_bit(&pDev->TFT_CTRL, BIT_TFT_MODE);     // UPS052 
        reg_write_bits(&pDev->TFT_HSYNC_DISPLAY, H_DISP_Paralle);
    }
    else
    {
        /* Wrong Mode */
        rval = -1;
        return rval;
    }

    return rval;
}

/**
 *  \brief  tft dclk setting, set display mode
 *
 *  \param [in] polarity     enum tft_clk_polarity
 *  \param [in] ui_value     tft data pre-scaler
 *  \return                  0: ok, other: fail
 *
 *  \details
 */
static int
_TFT_DCLK_Setting(
    sn_tft_t *pDev,
    uint8_t polarity, 
    uint16_t ui_value)
{
    int rval = 0;

    if(polarity)
    {
        reg_set_bit(&pDev->TFT_CTRL, BIT_DCLK_POL);     //Positive polarity
    }
    else
    {
        reg_clear_bit(&pDev->TFT_CTRL, BIT_DCLK_POL);   //Negative polarity
    }

    reg_write_bits(&pDev->TFT_DCLK_LEN, ui_value & 0xFFFF);
    
    return rval;
}

/**
 *  \brief  tft data seqence
 *
 *  \param [in] Odd_line     enum tft_data_seqence_t
 *  \param [in] Even_line    enum tft_data_seqence_t
 *  \return                  0: ok, other: fail
 *
 *  \details
 */
static int
_TFT_Data_Seq_Setting(
    sn_tft_t *pDev,
    uint8_t Odd_line,
    uint8_t Even_line)
{
    int rval = 0;

    reg_write_mask_bits(&pDev->TFT_CTRL, Odd_line<<9, 0x03<<9);
    reg_write_mask_bits(&pDev->TFT_CTRL, Even_line<<6, 0x03<<6);

    return rval;
}

/**
 *  \brief  tft vsync & hsync setting
 *
 *  \param [in] BP           Backporch
 *  \param [in] FP           Frontporch
 *  \param [in] polarity     enum tft_clk_polarity
 *  \param [in] PW           Pulse width
 *  \return                  0: ok, other: fail
 *
 *  \details
 */
static int
_TFT_Vsync_Setting(
    sn_tft_t *pDev,
    uint16_t BP,
    uint16_t FP,
    uint8_t polarity,
    uint16_t PW)
{
    int rval = 0;

    reg_write_bits(&pDev->TFT_VSYNC_BACK_PORCH, BP);
    reg_write_bits(&pDev->TFT_VSYNC_FRONT_PORCH, FP);
    reg_write_bits(&pDev->TFT_VSYNC_PULSE_WIDTH, PW);
    if(polarity == NEGATIVE_POLARITY)
    {
        reg_set_bit(&pDev->TFT_CTRL, BIT_VS_POL);
    }
    
    else if(polarity == POSITIVE_POLARITY)
    {
        reg_clear_bit(&pDev->TFT_CTRL, BIT_VS_POL);
    }
    
    else
    {
        /* Wrong polarity */
        rval = -1;
        return rval;
    }
    
    return rval;
}

static int
_TFT_Hsync_Setting(
    sn_tft_t *pDev,
    uint16_t BP,
    uint16_t FP,
    uint8_t polarity,
    uint16_t PW)
{
    int rval = 0;

    reg_write_bits(&pDev->TFT_HSYNC_BACK_PORCH, BP);
    reg_write_bits(&pDev->TFT_HSYNC_FRONT_PORCH, FP);
    reg_write_bits(&pDev->TFT_HSYNC_PULSE_WIDTH, PW);
    
    if(polarity == NEGATIVE_POLARITY)
    {
        reg_set_bit(&pDev->TFT_CTRL, BIT_HS_POL);
    }
    
    else if(polarity == POSITIVE_POLARITY)
    {
        reg_clear_bit(&pDev->TFT_CTRL, BIT_HS_POL);
    }
    
    else
    {
        /* Wrong polarity */
        rval = -1;
        return rval;
    }
    
    return rval;
}

/**
 *  \brief  tft dummy RGB for UPS052 mode
 *
 *  \param [in] ui_mode     0: RGBX , 1: XRGB
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
static int
_TFT_Dummy_RGB(
    sn_tft_t *pDev,
    uint8_t ui_mode
    )
{
    int rval = 0;

    /* XRGB */
    if(ui_mode == SEQ_XRGB)
    {
        reg_set_bit(&pDev->TFT_CTRL, BIT_SEQ_052);
    }
    
    /* RGBX */
    else if(ui_mode == SEQ_RGBX)
    {
        reg_clear_bit(&pDev->TFT_CTRL, BIT_SEQ_052);
    }

    return rval;
}

/**
 *  \brief  tft initial, set display mode
 *
 *  \param [in] tft_init_info     structure of tft display parameter
 *  \return                       0: ok, other: fail
 *
 *  \details
 */
int 
TFT_Init(
    tft_init_t *tft_init_info
    )
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    if(!tft_init_info)
    {
        rval = -1;
        return rval;
    }

    _TFT_Display_Mode_Setting(pDev, tft_init_info->display_mode);
    _TFT_DCLK_Setting(pDev, tft_init_info->clk_polarity, tft_init_info->clk_div);
    _TFT_Data_Seq_Setting(pDev, tft_init_info->odd_line_seq, tft_init_info->even_line_seq);
    _TFT_Hsync_Setting(pDev, tft_init_info->hsync_back_porch, tft_init_info->hsync_front_porch, tft_init_info->hsync_polarity, tft_init_info->hsync_pulse_width);

    if(tft_init_info->display_mode == UPS_051_SERIAL)
    {
        tft_init_info->vsync_pulse_width = (tft_init_info->vsync_pulse_width)*((tft_init_info->hsync_back_porch + tft_init_info->hsync_front_porch) + ((tft_init_info->lcd_width)*3));
        _TFT_Vsync_Setting(pDev, tft_init_info->vsync_back_porch, tft_init_info->vsync_front_porch, tft_init_info->vsync_polarity, tft_init_info->vsync_pulse_width);
    }
    else if(tft_init_info->display_mode == UPS_051_PARALLEL)
    {
        tft_init_info->vsync_pulse_width = (tft_init_info->vsync_pulse_width)*((tft_init_info->hsync_back_porch + tft_init_info->hsync_front_porch) + (tft_init_info->lcd_width));
        _TFT_Vsync_Setting(pDev, tft_init_info->vsync_back_porch, tft_init_info->vsync_front_porch, tft_init_info->vsync_polarity, tft_init_info->vsync_pulse_width);
    }
    
    else if(tft_init_info->display_mode == UPS_052_SERIAL)
    {
        _TFT_Dummy_RGB(pDev, tft_init_info->dummy_rgb_seq);
        tft_init_info->vsync_pulse_width = (tft_init_info->vsync_pulse_width)*((tft_init_info->hsync_back_porch + tft_init_info->hsync_front_porch) + ((tft_init_info->lcd_width)*3));
        _TFT_Vsync_Setting(pDev, tft_init_info->vsync_back_porch, tft_init_info->vsync_front_porch, tft_init_info->vsync_polarity, tft_init_info->vsync_pulse_width);
    }
    
    else if(tft_init_info->display_mode == UPS_052_PARALLEL)
    {
        _TFT_Dummy_RGB(pDev, tft_init_info->dummy_rgb_seq);
        tft_init_info->vsync_pulse_width = (tft_init_info->vsync_pulse_width)*((tft_init_info->hsync_back_porch + tft_init_info->hsync_front_porch) + (tft_init_info->lcd_width));
        _TFT_Vsync_Setting(pDev, tft_init_info->vsync_back_porch, tft_init_info->vsync_front_porch, tft_init_info->vsync_polarity, tft_init_info->vsync_pulse_width);
    }

    reg_set_bit(&pDev->TFT_CTRL, BIT_TFT_EN);   // Enable tft

    return rval;
}

/**
 *  \brief  tft deinitial
 *
 *  \return              0: ok, other: fail
 *
 *  \details
 */
int
TFT_Deinit(void)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_clear_bit(&pDev->TFT_CTRL, BIT_TFT_EN);

    return rval;
}

/**
 *  \brief  Enable horizontal interrupt.
 *
 *  \param [in] uint8_t     ucPolling   0: Disable IRQ, 1: Enable IRQ
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int 
TFT_HBLK_Interrupt_Enable(uint8_t Polling)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_set_bit(&pDev->TFT_BLK_INT_CTRL, BIT_HBLK);
    if(Polling != 0)
    {
        NVIC_ClearPendingIRQ(PPU_HBLK_IRQn);
        NVIC_EnableIRQ(PPU_HBLK_IRQn);
    }
    
    return rval;
}

/**
 *  \brief  Disable horizontal interrupt.
 *
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_Disable(void)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_clear_bit(&pDev->TFT_BLK_INT_CTRL, BIT_HBLK);
    NVIC_DisableIRQ(PPU_HBLK_IRQn);
    
    return rval;
}

/**
 *  \brief  Check horizontal interrupt flag status.
 *
 *  \param 
 *  \return  Horizontal interrupt flag status.
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_Status(void)
{
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    return pDev->TFT_BLK_INT_STAT_b.HBLK;
}

/**
 *  \brief  Clear horizontal interrupt flag.
 *
 *  \param 
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_ClearFlag(void)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_set_bit(&pDev->TFT_BLK_INT_STAT, BIT_HBLK_STA);
    
    return rval;
}

/**
 *  \brief  Enable vertical interrupt.
 *
 *  \param [in] uint8_t     ucPolling   0: Disable IRQ, 1: Enable IRQ
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int 
TFT_VBLK_Interrupt_Enable(uint8_t Polling)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_set_bit(&pDev->TFT_BLK_INT_CTRL, BIT_VBLK);
    if(Polling != 0)
    {
        NVIC_ClearPendingIRQ(PPU_VBLK_IRQn);
        NVIC_EnableIRQ(PPU_VBLK_IRQn);
    }
    
    return rval;
}

/**
 *  \brief  Disable vertical interrupt.
 *
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_Disable(void)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_clear_bit(&pDev->TFT_BLK_INT_CTRL, BIT_VBLK);
    NVIC_DisableIRQ(PPU_VBLK_IRQn);
    
    return rval;
}

/**
 *  \brief  Check vertical interrupt flag status.
 *
 *  \param 
 *  \return  Vertical interrupt flag status.
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_Status(void)
{
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    return pDev->TFT_BLK_INT_STAT_b.VBLK;
}

/**
 *  \brief  Clear vertical interrupt flag.
 *
 *  \param 
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_ClearFlag(void)
{
    int rval = 0;
    sn_tft_t *pDev = (sn_tft_t*) SN_TFT_BASE;
    
    reg_set_bit(&pDev->TFT_BLK_INT_STAT, BIT_VBLK_STA);
    
    return rval;
}

/**
 *  \brief  Get the version of tft driver
 *
 *  \param 
 *  \return  Version
 *
 *  \details
 */
uint32_t
TFT_GetVersion(void)
{
    return TFT_VERSION;
}
