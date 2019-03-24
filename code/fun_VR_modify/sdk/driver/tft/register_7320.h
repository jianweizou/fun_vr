/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file register_7320.h
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/02/23
 * @license
 * @description
 */

#ifndef __register_7320_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__
#define __register_7320_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define SN_TFT_BASE        0x40050094UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sn_tft
{
    /* TFT */
    __IO uint32_t  TFT_HSYNC_BACK_PORCH;                /*!< Offset:0x00 [15:0]*/
    __IO uint32_t  TFT_HSYNC_FRONT_PORCH;               /*!< Offset:0x04 [15:0]*/

    __IO uint32_t  TFT_VSYNC_BACK_PORCH;                /*!< Offset:0x08 [15:0]*/
    __IO uint32_t  TFT_VSYNC_FRONT_PORCH;               /*!< Offset:0x0C [15:0]*/
    
    __IO uint32_t  TFT_HSYNC_PULSE_WIDTH;               /*!< Offset:0x10 [15:0] 0:1, 1:2, ... */
    __IO uint32_t  TFT_VSYNC_PULSE_WIDTH;               /*!< Offset:0x14 [15:0]0:1, 1:2, ...  */
    __IO uint32_t  TFT_HSYNC_DISPLAY;                /*!< Offset:0x18 [7:0]0:320, 1:640, 2:960 */
    
    union { 
        __IO uint32_t  TFT_BLK_INT_CTRL;                /*!< Offset:0x1C [1:0]*/
        struct {
            __IO uint32_t  HBLK       :  1; 
            __IO uint32_t  VBLK       :  1; 
        } TFT_BLK_INT_CTRL_b;
    };

    union { 
        __IO uint32_t  TFT_BLK_INT_STAT;                /*!< Offset:0x20 [1:0]*/
        struct {
            __IO uint32_t  HBLK       :  1; 
            __IO uint32_t  VBLK       :  1; 
        } TFT_BLK_INT_STAT_b;
    };

    union {
        __IO uint32_t  TFT_CTRL;                        /*!< Offset:0x24 [16:0]*/

        struct {
            __IO uint32_t  Mode             :  2;       /*!< 00:UPS051  01:UPS052*/
            __IO uint32_t  RESERVED0        :  2;
            __IO uint32_t  RGB_SEQ_052      :  1;
            __IO uint32_t  RESERVED1        :  1;
            __IO uint32_t  EVEN_RGB_SEQ_051 :  2;
            __IO uint32_t  RESERVED2        :  1;
            __IO uint32_t  ODD_RGB_SEQ_051  :  2;
            __IO uint32_t  DCLK_Pol         :  1;       /*!< 0: Negative polarity, 1: Positive polarity*/
            __IO uint32_t  HSYNC_Pol        :  1;       /*!< 0: Negative polarity, 1: Positive polarity*/
            __IO uint32_t  VSYNC_Pol        :  1;       /*!< 0: Negative polarity, 1: Positive polarity*/
            __IO uint32_t  PAREL_OPT        :  1;
            __IO uint32_t  EN               :  1;
        } TFT_CTRL_b;
    };

    __IO uint32_t  TFT_DCLK_LEN;                        /*!< Offset:0x28*/
} sn_tft_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
