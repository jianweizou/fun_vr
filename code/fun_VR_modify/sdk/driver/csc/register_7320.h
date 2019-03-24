/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/09
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
 *  7320 definition
 */
#define SN_CSC_BASE                     0x40026000UL
#define SN_IDMA_CSC_IN_BASE             0x40026100UL
#define SN_IDMA_CSC_OUT_BASE            0x40026110UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 * SN_CSC Structure
 */
typedef struct {
    union {
        __IO uint32_t  CSC_CTRL;                /*!< Offset:0x00 CSC Control Register                          */                          
        struct {
            __O  uint32_t  Output_Valid      :  1;   /*!< Output_Valid                                        */
            __O  uint32_t  Input_Request     :  1;   /*!< Input_Request                                        */
            __IO uint32_t  RESERVED1         :  1;   /*!< RESERVED                                            */
            __IO uint32_t  SCAL_VSYNC        :  1;   /*!< Before doing scalar should set this bit from 1 to 0           */
            __IO uint32_t  SCAL_FIR_EN       :  1;   /*!< 0:Disable 1:Enable                            */
            __IO uint32_t  SCAL_FS_EN        :  1;   /*!< 0:Disable 1:Enable                            */
            __IO uint32_t  SCAL_EN           :  1;   /*!< 0:Disable 1:Enable,must enable with FS and filter together        */
            __IO uint32_t  RGB_DI_EN         :  1;   /*!< 0:Disable 1:Enable                            */
            __IO uint32_t  YCbCr_Mode        :  1;   /*!< YCbCr_Mode 0: 420 mode, 1: 422 mode                   */
            __IO uint32_t  RESERVED2         :  3;   /*!< RESERVED                                            */
              __IO uint32_t  CSC_Mode        :  2;   /*!< 10: YCbCr to RGB (Data in/Data out), 11: RGB to YCbCr(Data in/Data out)*/
            __IO uint32_t  DMA_Mode          :  1;   /*!< DMA_Mode 0: By MPU, 1: By DMA                                         */
            __IO uint32_t  CSC_EN            :  1;   /*!< CSC Enable                                                            */
        } CSC_CTRL_b;                           /*!< BitSize                                                               */
    };

  __I  uint32_t  RESERVED3;                 /*!< Offset:0x04  CSC Data In Register[15:0]                               */
  __O  uint32_t  RESERVED4;                 /*!< Offset:0x08  CSC Data Out Register[15:0]                              */
  __IO uint32_t  RESERVED5;                 /*!< Offset:0x0C                                                           */
  __IO uint32_t  RESERVED6;                 /*!< Offset:0x10                                                           */
  __IO uint32_t  RESERVED7;                 /*!< Offset:0x14  Tr Table Address Register 12:0]                          */
  __IO uint32_t  CSC_DMA_DATA_IN_Addr_L;    /*!< Offset:0x18  Data In Addr_L[15:0]                                     */
  __IO uint32_t  CSC_DMA_DATA_IN_Addr_H;    /*!< Offset:0x1C  Data In Addr_H[15:0]                                     */
  __IO uint32_t  CSC_DMA_DATA_OUT_Addr_L;   /*!< Offset:0x20 //for jpeg encoder                                        */
  __IO uint32_t  CSC_DMA_DATA_OUT_Addr_H;   /*!< Offset:0x24                                                           */
  __IO uint32_t  JPEG_LINE_SIZE;            /*!< Offset:0x28                                                           */
  __IO uint32_t  DI_HSize;                  /*!< Offset:0x2C                                                           */
  __IO uint32_t  DI_VSize;                  /*!< Offset:0x30                                                           */
  __IO uint32_t  SCAL_FIR_0;                /*!< Offset:0x34                                                           */
  __IO uint32_t  SCAL_FIR_1;                /*!< Offset:0x38                                                           */
  __IO uint32_t  SCAL_FIR_2;                /*!< Offset:0x3C                                                           */
  __IO uint32_t  SCAL_FIR_3;                /*!< Offset:0x40                                                           */
  __IO uint32_t  SCAL_FIR_4;                /*!< Offset:0x44                                                           */
  __IO uint32_t  SCAL_H_RATIO;              /*!< Offset:0x48                                                           */
  __IO uint32_t  SCAL_V_RATIO;              /*!< Offset:0x4C                                                           */
  __IO uint32_t  SCAL_HISIZE;               /*!< Offset:0x50                                                           */
  __IO uint32_t  SCAL_VISIZE;               /*!< Offset:0x54                                                           */
  __IO uint32_t  SCAL_HOSIZE;               /*!< Offset:0x58                                                           */
  __IO uint32_t  SCAL_VOSIZE;               /*!< Offset:0x5C                                                          */
} SN_CSC_Type_t;

/**
 * SN_IDMA Structure
 */

typedef struct {                                    /*!< SN_IDMA Structure                                                     */

    __IO uint32_t  CH_RAM_ADDR;                     /*!< Offset:0x00 Ram address                                               */
    __IO uint32_t  CH_LEN;  

    union {
        __IO uint32_t  CH_CTRL;                        /*!< Offset:0x08 Comparator Capture Output Match register                  */

        struct {
            __IO uint32_t  START              :  1;
            __IO uint32_t  WR_RAM             :  1;
            __IO uint32_t  PRAMM              :  1;
            __IO uint32_t  AD_PASS            :  1;
            __IO uint32_t  SCALAR             :  2;
            __IO uint32_t  INC                :  1;
            __IO uint32_t  RESERVED0          :  9;
            __IO uint32_t  REG_BURST_LEN_D    :  5;
            __IO uint32_t  RESERVED1          :  11;  
        } CH_CTRL_b;                                    /*!< BitSize                                                               */
    };
    __IO uint32_t  CH_DIP_ADDR;
} SN_IDMA_Type;

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
