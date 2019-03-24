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
#define SN_JPEG_ENC_BASE                0x40027000UL
#define SN_IDMA_JPEG_ENC_IN_BASE        0x40027100UL
#define SN_IDMA_JPEG_ENC_OUT_BASE       0x40027110UL
#define SN_JPEG_DEC_BASE                0x40028000UL 
#define SN_IDMA_JPEG_DEC_IN_BASE        0x40028100UL
#define SN_IDMA_JPEG_DEC_OUT_BASE       0x40028110UL
#define SN_SCRIPT_BASE                  0x40039000UL
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
 * SN_JPEG_ENC Structure
 */
typedef struct {                   /*!< SN_JPEG_ENC Structure                                                 */ 

    union {
        __IO uint32_t  JPEG_ENC_CTRL;               /*!< Offset:0x00 JPEG Encoder Control Register                    */ 
        struct {         
            __IO uint32_t  Q_TABLE_GAIN     :  8;   /*!< Default:(001.00000) FINAL QP Value= QP TABLE*QP                */
            __IO uint32_t  QUT_RD_EN        :  1;   /*!< 0: QP TABLE Disable 1: QP TABLE Enable                    */
            __IO uint32_t  RESERVED0        :  1;   /*!< RESERVED                                    */
            __IO uint32_t  Input_Frame_End  :  1;   /*!< Input_Frame_End 0: Normal, 1: Input data frame end            */
            __IO uint32_t  Insert_Q_Table   :  1;   /*!< Insert_Q_Table  0: Normal, 1: Insert Q Table                */
            __IO uint32_t  YUV_Format       :  1;   /*!< YUV_Format 0: 420 format(4x1,1x1,1x1), 1: 422 format(2x1,1x1,1x1)     */
            __IO uint32_t  Bit_Stuff        :  1;   /*!< Fill data until 8bit at last byte in encode data 0:Fill 0 1:Fill 1                             */
            __IO uint32_t  RESERVED1        :  1;   /*!< RESERVED1                                  */
            __IO uint32_t  JPEG_ENC_EN      :  1;   /*!< JPEG Encode Enable 0: Disable 1: Enable                        */
        } JPEG_ENC_CTRL_b;               /*!< BitSize                                                               */
    };

    union {
        __IO uint32_t  JPEG_ENC_STAT;                       /*!< Offset:0x04 JPEG Encoder Status Register                          */
        struct {    
            __IO uint32_t  RESERVED0        :  3;   /*!< RESERVED                                    */
            __IO uint32_t  Q_Table_Full     :  1;   /*!< Q_Table_Full 0: Q_table full 1: Q_table not full                     */
            __IO uint32_t  RESERVED1        :  11;  /*!< RESERVED                                    */
            __IO uint32_t  OF_End           :  1;   /*!< Output Frame End  0: Normal 1:Input data frame end */
        } JPEG_ENC_STAT_b;                          /*!< BitSize                                                               */
    };

    __IO uint32_t  RESERVED1;            /*!< Offset:0x08  RESERVED                                                 */
    __IO uint32_t  RESERVED2;            /*!< Offset:0x0C  RESERVED                                                 */
    __IO uint32_t  JPEG_Write_Q_Table;   /*!< Offset:0x10  JPEG_Write_Q_Table Register[15:0]                        */
    __IO uint32_t  IDMA_Select;          /*!< Offset:0x14  IDMA_Select Register 0:CSC,1: RAM                        */                          
    __IO uint32_t  AC_REDUCE;            /*!< Use to reduce bitstream data[7:0] Default :8'd63              */
} SN_JPEG_ENC_Type_t;

/**
 * SN_JPEG_DEC Structure
 */
typedef struct {                   /*!< SN_JPEG_DEC Structure                                                 */ 
    union {
        __IO uint32_t  JPEG_DEC_CTRL;                   /*!< Offset:0x00 JPEG Decoder Control Register                */ 
        struct {         
            __IO uint32_t  Q_TABLE_GAIN     :  8;   /*!< Default:(001.00000) FINAL QP Value= QP TABLE*QP                */
            __IO uint32_t  RESERVED0        :  2;   /*!< RESERVED0                                    */
            __IO uint32_t  Input_Frame_End  :  1;   /*!< Input_Frame_End 0: Normal, 1: Input data frame end            */
            __IO uint32_t  Insert_Q_Table   :  1;   /*!< Insert_Q_Table  0: Normal, 1: Insert Q Table                */
            __IO uint32_t  YUV_Format       :  1;   /*!< YUV_Format 0: 420 format(4x1,1x1,1x1), 1: 422 format(2x1,1x1,1x1)     */
            __IO uint32_t  RESERVED1        :  2;   /*!< RESERVED1                                  */
            __IO uint32_t  JPEG_DEC_EN      :  1;   /*!< JPEG Encode Enable 0: Disable 1: Enable                        */
        } JPEG_DEC_CTRL_b;               /*!< BitSize                                                               */
    };

    union {
        __IO uint32_t  JPEG_DEC_STAT;                       /*!< Offset:0x04 JPEG Decoder Status Register                          */
        struct {    
            __IO uint32_t  Error_signal     :  1;   /*!< JPEG Error signal 0: Normal 1: Error Occurrence                */
            __IO uint32_t  RESERVED0        :  2;   /*!< RESERVED                                    */
            __IO uint32_t  Q_Table_Full     :  1;   /*!< Q_Table_Full 0: Q_table full 1: Q_table not full                     */
            __IO uint32_t  RESERVED1        :  11;  /*!< RESERVED                                    */
            __IO uint32_t  OF_End           :  1;   /*!< Output Frame End  0: Normal 1:Input data frame end                     */
        } JPEG_DEC_STAT_b;                          /*!< BitSize                                    */
    };

    __IO uint32_t  RESERVED1;                       /*!< Offset:0x08  RESERVED                                                 */
    __IO uint32_t  RESERVED2;                       /*!< Offset:0x0C  RESERVED                                                 */
    __IO uint32_t  JPEG_Write_Q_Table;              /*!< Offset:0x10  JPEG_Write_Q_Table Register[15:0]                        */
 
    union {
        __IO uint32_t  Skip_Byte;                   /*!< Offset:0x14 JPEG Decoder Status Register                          */
        struct {    
            __IO uint32_t  RESERVED0        :  4;   /*!< RESERVED                                 */
            __IO uint32_t  SKIP_BYTE        :  2;   /*!< decoder input bitstream align 4 bytes, range 0~3                                    */
        } SKIP_BYTE_b;                 /*!< BitSize                                                               */
    };
} SN_JPEG_DEC_Type_t;

/**
 * SN_SCRIPT Structure
 */
typedef struct{                              /*!< SN_SCRIPT Structure                                                 */ 
    union {
    __IO uint32_t  SCRIPT_STEP;              /*!< Offset:0x00 Script Mode Initial Register                */ 
        struct {         
            __IO uint32_t  wait_int_en :  1; /*!< Wait IRQ & Clr #IRQ                            */
            __IO uint32_t  CPU_w_en    :  1; /*!< Access CPU W & DATA                              */
            __IO uint32_t  J_IFN_int   :  1; /*!< If #int !=1 Jmp to Jmp_step ? repeat form step 1            */
            __IO uint32_t  Loop_en     :  1; /*!< if Loop_count_reg !=0 Jmp to Jmp_step ? repeat form step 1 else finish*/
            __IO uint32_t  IRQ         :  6; /*!< Waiting for which IRQ                          */
            __IO uint32_t  Jmp_step    :  4; /*!< Jump to which step                              */
        } SCRIPT_STEP_b;                     /*!< BitSize                                                               */
    };
    __IO uint32_t  RESERVED0;                /*!< Offset:0x04  RESERVED                                                  */
    __IO uint32_t  SET_DATA;                 /*!< Set Data Buffer Addr                                                 */
    __IO uint32_t  SET_ADDR;                 /*!< Set CSC or JPEG Register                                                */
} SCRIPT_STEP_Type;

typedef struct {                             /*!< SN_SCRIPT Structure                                                 */ 
    union {
        __IO uint32_t  SCRIPT_INIT;                  /*!< Offset:0x00 Script Mode Initial Register                */ 
        struct {         
            __IO uint32_t  Active            :  1;   /*!< Initial 0, Write 1 to start, read 1 when going, read 0 means done     */
            __O  uint32_t  Reset             :  1;   /*!< reset the Script FSM                             */
            __IO uint32_t  RESERVED0         :  6;   /*!< RESERVED                                 */
            __I  uint32_t  Current_Step      :  5;   /*!< Current Step Read Only                            */
            __IO uint32_t  RESERVED1         :  3;   /*!< RESERVED                                  */
            __I  uint32_t  Current_loop      :  6;   /*!< Current loop Read Only                            */
            __IO uint32_t  RESERVED2         :  2;   /*!< RESERVED                                  */
            __IO uint32_t  Total_loop        :  6;   /*!< the total loop count, should >=2                           */
        } SCRIPT_INIT_b;                /*!< BitSize                                                               */
    };
    __IO uint32_t  RESERVED0;            /*!< Offset:0x04  RESERVED                                                 */
    __IO uint32_t  RESERVED1;            /*!< Offset:0x08  RESERVED                                                 */
    __IO uint32_t  RESERVED2;            /*!< Offset:0x0C  RESERVED                                                 */

    __IO SCRIPT_STEP_Type STEP1;        /*!< Offset:0x10                                                        */
    __IO SCRIPT_STEP_Type STEP2;        /*!< Offset:0x20                                                        */
    __IO SCRIPT_STEP_Type STEP3;        /*!< Offset:0x30                                                        */
    __IO SCRIPT_STEP_Type STEP4;        /*!< Offset:0x40                                                        */
    __IO SCRIPT_STEP_Type STEP5;        /*!< Offset:0x50                                                        */
    __IO SCRIPT_STEP_Type STEP6;        /*!< Offset:0x60                                                        */
    __IO SCRIPT_STEP_Type STEP7;        /*!< Offset:0x70                                                        */
    __IO SCRIPT_STEP_Type STEP8;        /*!< Offset:0x80                                                        */
    __IO SCRIPT_STEP_Type STEP9;        /*!< Offset:0x90                                                        */
    __IO SCRIPT_STEP_Type STEPA;        /*!< Offset:0xA0                                                        */
    __IO SCRIPT_STEP_Type STEPB;        /*!< Offset:0xB0                                                        */
    __IO SCRIPT_STEP_Type STEPC;        /*!< Offset:0xC0                                                        */
    __IO SCRIPT_STEP_Type STEPD;        /*!< Offset:0xD0                                                        */
    __IO SCRIPT_STEP_Type STEPE;        /*!< Offset:0xE0                                                        */
    __IO SCRIPT_STEP_Type STEPF;        /*!< Offset:0xF0                                                        */
    __IO SCRIPT_STEP_Type STEP10;        /*!< Offset:0x100                                                        */
    __IO SCRIPT_STEP_Type STEP11;        /*!< Offset:0x110                                                        */
    __IO SCRIPT_STEP_Type STEP12;        /*!< Offset:0x120                                                        */
    __IO SCRIPT_STEP_Type STEP13;        /*!< Offset:0x130                                                        */
    __IO SCRIPT_STEP_Type STEP14;        /*!< Offset:0x140                                                        */
    __IO SCRIPT_STEP_Type STEP15;        /*!< Offset:0x150                                                        */
    __IO SCRIPT_STEP_Type STEP16;        /*!< Offset:0x160                                                        */
    __IO SCRIPT_STEP_Type STEP17;        /*!< Offset:0x170                                                        */
    __IO SCRIPT_STEP_Type STEP18;        /*!< Offset:0x180                                                        */
    __IO SCRIPT_STEP_Type STEP19;        /*!< Offset:0x190                                                        */
    __IO SCRIPT_STEP_Type STEP1A;        /*!< Offset:0x1A0                                                        */
    __IO SCRIPT_STEP_Type STEP1B;        /*!< Offset:0x1B0                                                        */
    __IO SCRIPT_STEP_Type STEP1C;        /*!< Offset:0x1C0                                                        */
    __IO SCRIPT_STEP_Type STEP1D;        /*!< Offset:0x1D0                                                        */
    __IO SCRIPT_STEP_Type STEP1E;        /*!< Offset:0x1E0                                                        */
    __IO SCRIPT_STEP_Type STEP1F;        /*!< Offset:0x1F0                                                        */
} SN_SCRIPT_Type_t;

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
