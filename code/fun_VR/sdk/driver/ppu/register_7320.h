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
 * @date 2018/01/09
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
#define SN_PPU_BASE         0x40050000UL
#define SN_LINE_DMA_BASE    0x40050100UL
#define SN_SCALING_DMA_BASE 0x40050200UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sn_ppu
{                                    /*!< SN_PPU Structure for PPU*/
    union {
        __IO uint32_t  PPU_CTRL;                          /*!< Offset:0x00*/
    
        struct {
            __IO uint32_t  TX1EN                :  1;             
            __IO uint32_t  TX2EN                :  1;                
            __IO uint32_t  SPEN                 :  1;                           
            __IO uint32_t  TX3EN                :  1;
            __IO uint32_t  PPUEN                :  1;
            __IO uint32_t  TX1PalRAM_Sel        :  1;//201706 modify
            __IO uint32_t  TX2PalRAM_Sel        :  1; //201706 modify         
            __IO uint32_t  PIXEL_LINE_Sel       :  1; //add 1280x720 resolution 201705
            __IO uint32_t  RESERVED1            :  1; //201706 modify 
            __IO uint32_t  PIXEL_LINE_Sel_HD    :  1; //add 1280x720 resolution 201706         
                
        } PPU_CTRL_b;                                   /*!< BitSize*/
    };

    __IO uint32_t  TX1_X_POS;                           /*!< Offset:0x04*/
    __IO uint32_t  TX1_Y_POS;                           /*!< Offset:0x08*/

    union {
        __IO uint32_t  TX1_ATTR;                        /*!< Offset:0x0C*/

        struct {
            __IO uint32_t  Color      :  2;             
            __IO uint32_t  HFlip      :  1;             
            __IO uint32_t  VFlip      :  1;                           
            __IO uint32_t  HSize      :  2;
            __IO uint32_t  VSize      :  2;
            __IO uint32_t  Palette    :  4;
            __IO uint32_t  Depth      :  2;
        } TX1_ATTR_b;                                   /*!< BitSize*/
    };

    union {
        __IO uint32_t  TX1_CTRL;                        /*!< Offset:0x10*/

        struct {
            __IO uint32_t  BMP        :  1;             
            __IO uint32_t  AttrInReg  :  1;             
            __IO uint32_t  WAP        :  1;                           
            __IO uint32_t  RGB        :  1;
            __IO uint32_t  RESERVED0  :  2;
            __IO uint32_t  VCMP_EN    :  1;
            __IO uint32_t  BLD_EN     :  1;
            __IO uint32_t  VSCRN_SIZE :  2;
        } TX1_CTRL_b;                                   /*!< BitSize*/
    };

    __IO uint32_t  TX1_NUM_PTR;                         /*!< Offset:0x14 [15:0]*/
    __IO uint32_t  TX1_ATTR_PTR;                        /*!< Offset:0x18 [15:0]*/
    __IO uint32_t  TX1_DISP_CTRL;                       /*!< Offset:0x1C [14:0]*/
    __IO uint32_t  TX1_PATT_ADDRL;                      /*!< Offset:0x20 [15:0]*/
    __IO uint32_t  TX1_PATT_ADDRH;                      /*!< Offset:0x24 [15:0]*/

    __IO uint32_t  TX2_X_POS;                           /*!< Offset:0x28 [8:0]*/
    __IO uint32_t  TX2_Y_POS;                           /*!< Offset:0x2C [7:0]*/

    union { 
        __IO uint32_t  TX2_ATTR;                        /*!< Offset:0x30*/
        struct {
            __IO uint32_t  Color      :  2;             
            __IO uint32_t  HFlip      :  1;             
            __IO uint32_t  VFlip      :  1;                           
            __IO uint32_t  HSize      :  2;
            __IO uint32_t  VSize      :  2;
            __IO uint32_t  Palette    :  4;
            __IO uint32_t  Depth      :  2;
        } TX2_ATTR_b;                                   /*!< BitSize*/
    }; 

    union {
        __IO uint32_t  TX2_CTRL;                        /*!< Offset:0x34*/

        struct {
            __IO uint32_t  BMP        :  1;             
            __IO uint32_t  AttrInReg  :  1;             
            __IO uint32_t  WAP        :  1;                           
            __IO uint32_t  RGB        :  1;
            __IO uint32_t  RESERVED0  :  2;
            __IO uint32_t  VCMP_EN    :  1;
            __IO uint32_t  BLD_EN     :  1;
            __IO uint32_t  VSCRN_SIZE :  2;
        } TX2_CTRL_b;                                   /*!< BitSize*/
    };        

    __IO uint32_t  TX2_NUM_PTR;                         /*!< Offset:0x38 [12:0]*/
    __IO uint32_t  TX2_ATTR_PTR;                        /*!< Offset:0x3C [12:0]*/
    __IO uint32_t  TX2_DISP_CTRL;                       /*!< Offset:0x40 [14:0]*/
    __IO uint32_t  TX2_PATT_ADDRL;                      /*!< Offset:0x44 [15:0]*/
    __IO uint32_t  TX2_PATT_ADDRH;                      /*!< Offset:0x48 [15:0]*/
    
    __IO uint32_t  SPR_PATT_ADDRL;                      /*!< Offset:0x4C [15:0]*/
    __IO uint32_t  SPR_PATT_ADDRH;                      /*!< Offset:0x50 [15:0]*/

    union {
        __IO uint32_t  DMA_REQ;                         /*!< Offset:0x54*/

        struct {
            __IO uint32_t  BURST_LENGTH     :  1;
            __IO uint32_t  RESERVED0        :  14;
            __IO uint32_t  START            :  1;
        } DMA_REQ_b;                                    /*!< BitSize*/
    };

    __IO uint32_t  DMA_SRC_ADDRL;                       /*!< Offset:0x58  [15:0]*/
    __IO uint32_t  DMA_SRC_ADDRH;                       /*!< Offset:0x5C  [15:0]*/
    __IO uint32_t  DMA_DES_ADDRL;                       /*!< Offset:0x60  [15:0]*/
    __IO uint32_t  DMA_DES_ADDRH;                       /*!< Offset:0x64  [15:0]*/
    __IO uint32_t  DMA_LEN;                             /*!< Offset:0x68  [12:0]*/

    union { 
        __IO uint32_t  Blend_CTRL;                      /*!< Offset:0x6C  [4:0]*/

        struct {
            __IO uint32_t  Level      :  5;
        } Blend_CTRL_b;
    };

    union {
        __IO uint32_t  Backdrop_Color;                  /*!< Offset:0x70  RGB565*/

        struct {
            __IO uint32_t  B          :  5;
            __IO uint32_t  G          :  6;
            __IO uint32_t  R          :  5;
        } Backdrop_Color_b;                             /*!< BitSize*/
    };

    union {
        __IO uint32_t  Bright_CTRL;                     /*!< Offset:0x74  [7:0]*/

        struct {
            __IO uint32_t  Level      :  5;
            __IO uint32_t  BRM        :  1;             /*!< 0: Increase, 1: Decrease*/
            __IO uint32_t  BROUT      :  1;             /*!< 0: Inside, 1: Outside*/
            __IO uint32_t  BREN       :  1;             /*!< 0: Disable, 1: Enable*/
        } Bright_CTRL_b;                                /*!< BitSize*/
    };

    __IO uint32_t  Brightness_STA_X;                    /*!< Offset:0x78 [8:0]*/
    __IO uint32_t  Brightness_STA_Y;                    /*!< Offset:0x7C [7:0]*/
    __IO uint32_t  Brightness_END_X;                    /*!< Offset:0x80 [8:0]*/
    __IO uint32_t  Brightness_END_Y;                    /*!< Offset:0x84 [7:0]*/
    __IO uint32_t  VCMP_Scale;                          /*!< Offset:0x88 [2:0]*/
    __IO uint32_t  VCMP_Offset;                         /*!< Offset:0x8C [7:0]*/
    __IO uint32_t  TX_LINE_STA_ADDRH;                   /*!< Offset:0x90 [7:0]->TX1  [23:16]->TX2*/

    __IO uint32_t  RESERVED0[17];                       /*!< Offset:0x94~0xD4*/
    
    __IO uint32_t  OTHER_RES_DISP_START_X;              /*!< Offset:0xD8*/
    __IO uint32_t  OTHER_RES_DISP_START_Y;              /*!< Offset:0xDC*/
    
    /* line 240~639 */
    __IO uint32_t  TX1_DISP_CTRL2;                      /*!< Offset:0xE0*/
    __IO uint32_t  TX2_DISP_CTRL2;                      /*!< Offset:0xE4*/
    /* line 240~639 */

    __IO uint32_t  OTHER_RES_DISP_SIZE_X;               /*!< Offset:0xE8*/
    __IO uint32_t  OTHER_RES_DISP_SIZE_Y;               /*!< Offset:0xEC*/

} sn_ppu_t;

typedef struct sn_line_dma{ 

  union {
    __IO uint32_t  LINE_CTRL;                       /*!< Offset:0x00 DMA LINE Control Register																*/ 
    struct {
      __IO uint32_t	 DMA_EN		 			:  1;
	  __IO uint32_t	 DMA_START			:  1;
    } LINE_CTRL_b;
  };

	__IO uint32_t  LINE_ADDR_0;                     /*!< Offset:0x04 Line Addr0 Register                                      */
	__IO uint32_t  LINE_ADDR_1;                     /*!< Offset:0x08 Line Addr1 Register                                      */
	__IO uint32_t  LINE_NUM;                        /*!< Offset:0x0C LINE NUM Register for TX3                                */
	__IO uint32_t  PIXEL_NUM;                       /*!< Offset:0x10 PIXEL NUM Register for TX3                               */

} sn_line_dma_t;


typedef struct sn_scaling_dma{ 

  union {
    __IO uint32_t  LINE_CTRL;                       /*!< Offset:0x00 DMA LINE Control Register																*/ 
    struct {
	__IO uint32_t	 DMA_EN		 			:  1;
	__O  uint32_t	 DMA_START			:  1; 
	__IO uint32_t	 SCALE_SEL 			:  1;           /*!< 0:x2, 1:x4																                            */ 
    } LINE_CTRL_b;
  };

	__IO uint32_t  LINE_ADDR;                       /*!< Offset:0x04 Line Addr Register                                       */
	__IO uint32_t  LINE_NUM;                        /*!< Offset:0x08 Line Addr Register                                       */
	__IO uint32_t  PIXEL_NUM;                       /*!< Offset:0x0C Line Addr Register                                       */

} sn_scaling_dma_t;


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
