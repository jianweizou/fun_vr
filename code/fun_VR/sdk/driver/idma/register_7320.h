/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/02/09
 * @license
 * @description
 */

#ifndef __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__
#define __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 * 7320 definition
 */

#define SN_IDMA_0_BASE                  0x40020000UL
#define SN_IDMA_1_BASE                  0x40020010UL
#define SN_IDMA_SPIFC_BASE              0x40022100UL
#define SN_IDMA_SDNF_BASE               0x40023110UL
#define SN_IDMA_SDIO_BASE               0x40024100UL
#define SN_IDMA_CIS_BASE                0x40025110UL
#define SN_IDMA_CSC_IN_BASE             0x40026100UL
#define SN_IDMA_CSC_OUT_BASE            0x40026110UL
#define SN_IDMA_JPEG_ENC_IN_BASE        0x40027100UL
#define SN_IDMA_JPEG_ENC_OUT_BASE       0x40027110UL
#define SN_IDMA_JPEG_DEC_IN_BASE        0x40028100UL
#define SN_IDMA_JPEG_DEC_OUT_BASE       0x40028110UL

#define SN_IDMA_SPI0_BASE               0x4002B100UL
#define SN_IDMA_SPI1_BASE               0x4002C100UL

#define SN_IDMA_AES_BASE                0x40029100UL
#define SN_IDMA_I2S0_BASE               0x40030100UL
#define SN_IDMA_I2S1_BASE               0x40031100UL
#define SN_IDMA_I2S2_BASE               0x40032100UL
#define SN_IDMA_I2S3_BASE               0x40033100UL

#define SN_DRAM_REG_BASE                0x40040000UL

#define SN_SYS0_DRAM_CHL_SEL            0x45000054UL

#define IDMA_CTRL_START_Pos     (0x00)
#define IDMA_CTRL_START_Msk     (0x01 << IDMA_CTRL_START_Pos)

#define IDMA_CTRL_RDIP_Pos      (0x01)
#define IDMA_CTRL_RDIP_Msk      (0x01 << IDMA_CTRL_RDIP_Pos)

#define IDMA_CTRL_CONTINUE_MSK  (0x40)

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  SN_IDMA Structure
 */
typedef struct sn_idma
{
    __IO uint32_t  CH_RAM_ADDR;                     /*!< Offset:0x00 Ram address */

    __IO uint32_t  CH_LEN;

    union {
        __IO uint32_t  CH_CTRL;                         /*!< Offset:0x08 Comparator Capture Output Match register */

        struct {
            __IO uint32_t  START        :  1;           /* Activate DMA transfer by setting 1, auto cleared by HW */
            __IO uint32_t  DIP_TO_RAM   :  1;           /* 1- DIP->RAM, 0- RAM->DIP */
            __IO uint32_t  RESERVED1    :  1;
            __IO uint32_t  AD_PASS      :  1;           /* CIS IDMA bit */
            __IO uint32_t  SCALAR       :  2;           /* 0-Normal, 1-Keep LSB 1 Word only with every 2 Words, 2- ...every 4Words. */
            __IO uint32_t  CONTINUE     :  1;           /* If set, DIP_AD and RAM_AD will be incresed by DMA_LEN after DMA finnished.  */
            __IO uint32_t  RESERVED7    :  9;
            __IO uint32_t  BURST_NUM    :  9;           /* Burst number for DDR in IDMA0/1 only */
            
        } CH_CTRL_b;                                    /*!< BitSize */
    };

    __IO uint32_t  CH_DIP_ADDR;
} sn_idma_t;

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
