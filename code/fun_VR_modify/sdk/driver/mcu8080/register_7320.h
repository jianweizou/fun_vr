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
 * @date 2018/04/27
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
#define SN_MCU_8080_BASE        0x400500C0UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct sn_mcu8080{
    
    union {
        __IO uint32_t  MPU_8080_CTRL;                       /*!< Offset:0x00*/

        struct {
            __IO uint32_t  MPUS         :  1;               /*!< 0:Idle, 1:Start*/
            __IO uint32_t  MPURW        :  2;               /*!< 0:Read Command, 1:Read Data, 2:Write Command, 3:Write Data*/
            __IO uint32_t  ASRM         :  1;               /*!< 0:D7 ~ D0 is valid, 1:D15 ~ D0 is valid(2 bytes)*/
            __IO uint32_t  HLF          :  1;               /*!< 0:High byte first, 1:Low byte first*/
            __IO uint32_t  RESERVED0    :  3;
            __IO uint32_t  PPU_8080_EN  :  1;               /*!< 0:No Action, 1:PPU display via 8080*/
            __IO uint32_t  RESERVED1    :  3;
            __IO uint32_t  DMODE        :  1;               /*!< 0:8-bit mode, 1:16-bit mode*/
            __IO uint32_t  RESERVED2    :  2;
            __IO uint32_t  MPUEN        :  1;               /*!< 0:Disable, 1:Enable*/
        } MPU_8080_CTRL_b;
    };

    union {
        __IO uint32_t  MPU_8080_WIDTH_CTRL;                 /*!< Offset:0x04*/

        struct {
            __IO uint32_t  RW_LP      :  8;
            __IO uint32_t  RW_HP      :  8;
        } MPU_8080_WIDTH_CTRL_b;
    };

    __IO uint32_t  MPU_8080_DATA;                        /*!< Offset:0x08*/
    __IO uint32_t  MPU_8080_DMA_RAM_ADDR;                   /*!< Offset:0x0C*/

    union {
        __IO uint32_t  MPU_8080_DMA_CTRL;                   /*!< Offset:0x10*/
        struct {
            __IO uint32_t  START      :  1;                 /*!< 0:Stop, 1:Start*/
            __IO uint32_t  MODE_SEL   :  1;                 /*!< 0:8080 -> AHB RAM (Read)*/
                                                            /*!< 1:AHB RAM -> 8080 (Write)*/
        } MPU_8080_DMA_CTRL_b;
    };

    __IO uint32_t  MPU_8080_DMA_LEN;                        /*!< Offset:0x14*/
} sn_mcu8080_t;

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
