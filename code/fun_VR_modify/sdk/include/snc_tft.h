/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_tft.h
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_tft_H_wks5E6JD_l5H0_HIZX_s1mw_uspkTA5sgyZJ__
#define __snc_tft_H_wks5E6JD_l5H0_HIZX_s1mw_uspkTA5sgyZJ__

#ifdef __cplusplus
extern "C" {
#endif
#include "snc_types.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

/**
 *  tft display mode
 */
typedef enum tft_display_mode
{
    UPS_051_SERIAL = 0,
    UPS_052_SERIAL,
    UPS_051_PARALLEL,
    UPS_052_PARALLEL
} tft_display_mode_t;

/**
 *  tft clock polarity
 */
typedef enum tft_clk_polarity
{
    POSITIVE_POLARITY = 0,
    NEGATIVE_POLARITY
} tft_clk_polarity_t;

/**
 *  tft data seqence
 */
typedef enum tft_data_seqence
{
    SEQ_RGB = 0,
    SEQ_GBR,
    SEQ_BRG,
    SEQ_RGBX,
    SEQ_XRGB
} tft_data_seqence_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef __packed struct tft_init
{
    uint8_t             clk_div;
    uint16_t            lcd_width;
    uint16_t            lcd_height;
    uint32_t            vsync_front_porch;
    uint32_t            vsync_back_porch;
    uint32_t            vsync_pulse_width;
    uint32_t            hsync_front_porch;
    uint32_t            hsync_back_porch;
    uint32_t            hsync_pulse_width;
    tft_display_mode_t  display_mode;
    tft_clk_polarity_t  clk_polarity;
    tft_clk_polarity_t  vsync_polarity;
    tft_clk_polarity_t  hsync_polarity;
    tft_data_seqence_t  even_line_seq;
    tft_data_seqence_t  odd_line_seq;
    tft_data_seqence_t  dummy_rgb_seq;
} tft_init_t;


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
 *  \brief  tft initial, set display mode
 *
 *  \param [in] tft_init_info     structure of tft display parameter
 *  \return                       0: ok, other: fail
 *
 *  \details
 */
int
TFT_Init(
    tft_init_t *tft_init_info);

/**
 *  \brief  tft deinitial
 *
 *  \return              0: ok, other: fail
 *
 *  \details
 */
int
TFT_Deinit(void);

/**
 *  \brief  Enable horizontal interrupt.
 *
 *  \param [in] uint8_t     ucPolling   0: Disable IRQ, 1: Enable IRQ
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_Enable(
    uint8_t Polling);

/**
 *  \brief  Disable horizontal interrupt.
 *
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_Disable(void);

/**
 *  \brief  Check horizontal interrupt flag status.
 *
 *  \param 
 *  \return  Horizontal interrupt flag status.
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_Status(void);

/**
 *  \brief  Clear horizontal interrupt flag.
 *
 *  \param 
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_HBLK_Interrupt_ClearFlag(void);

/**
 *  \brief  Enable vertical interrupt.
 *
 *  \param [in] uint8_t     ucPolling   0: Disable IRQ, 1: Enable IRQ
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_Enable(uint8_t Polling);

/**
 *  \brief  Disable vertical interrupt.
 *
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_Disable(void);

/**
 *  \brief  Check vertical interrupt flag status.
 *
 *  \param 
 *  \return  Vertical interrupt flag status.
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_Status(void);

/**
 *  \brief  Clear vertical interrupt flag.
 *
 *  \param 
 *  \return  0: ok, other: fail
 *
 *  \details
 */
int
TFT_VBLK_Interrupt_ClearFlag(void);

/**
 *  \brief  Get the version of tft driver
 *
 *  \param 
 *  \return  Version
 *
 *  \details
 */
uint32_t
TFT_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
