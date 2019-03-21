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
#define SN_CIS_BASE                     0x40025000UL
#define SN_IDMA0_CIS_BASE               0x40025100UL
#define SN_IDMA1_CIS_BASE               0x40025110UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  SN_CIS Structure
 */
typedef struct sn_cis
{
    union {
        __IO uint32_t  CIS_CTRL;                   /*!< Offset:0x00 CIS register */

        struct {
            __IO uint32_t  CIS_CLK_SEL      :  2;  //0 1
            __IO uint32_t  CIS_PIXEL_CLK    :  1;  //2
            __IO uint32_t  CIS_HERF         :  1;  //3
            __IO uint32_t  CIS_VSYNC        :  1;  //4
            __IO uint32_t  L0_FULL_FLG      :  1;  //5
            __IO uint32_t  L1_FULL_FLG      :  1;  //6
            __IO uint32_t  L2_FULL_FLG      :  1;  //7
            __IO uint32_t  PIXEL_FMT        :  1;  //8
            __IO uint32_t  L2B_FMT          :  1;  //9
            __IO uint32_t  WINDOWS_EN       :  1;  //10
            __IO uint32_t  TRANSPARENT_EN   :  1;  //11
            __IO uint32_t  L2B_EN           :  1;  //12
            __IO uint32_t  CIS_DATA         :  1;  //13
            __IO uint32_t  RESERVED1        :  1;  //14
            __IO uint32_t  CIS_EN           :  1;  //15
            __IO uint32_t  OID_EN           :  1;  //16
            __IO uint32_t  OID_SEL          :  2;  //17 18
            __IO uint32_t  PCLK_SEL         :  1;  //19 0:PCLK 1 :PCLK delay 1T(under /4)
            __IO uint32_t  PCLK_LATCH       :  1;  //20 0:PCLK LATCH DATA to CIS 1:PCLK not latch data (the same with7312) 
        } CIS_CTRL_b;
    };
  
    __IO uint32_t  LINE0_ADDR;                      /*!< Offset:0x04                                                          */
    __IO uint32_t  LINE1_ADDR;                      /*!< Offset:0x08                                                          */
    __IO uint32_t  LINE2_ADDR;                      /*!< Offset:0x0C                                                          */
    __IO uint32_t  CIS_COL_N;                       /*!< Offset:0x10                                                          */
    __IO uint32_t  RESERVED0;                       /*!< Offset:0x14                                                          */
    __IO uint32_t  CIS_VOFFSET;                     /*!< Offset:0x18                                                          */
    __IO uint32_t  CIS_HOFFSET;                     /*!< Offset:0x1C                                                          */

    union {
        __IO uint32_t  CIS_SCALE_SIZE;              /*!< Offset:0x20 CIS_SCALE_SIZE                                           */

        struct {
            __IO uint32_t  PIXEL_SKIP       :  2;  
            __IO uint32_t  PIXEL_SEL        :  1;
            __IO uint32_t  RESERVED         :  5;
            __IO uint32_t  LINE_SKIP        :  2;
            __IO uint32_t  LINE_SEL         :  1;         
            __IO uint32_t  SCLAE_EN         :  1;         
        } CIS_SCALE_SIZE_b;                                                                                             
    };

    __IO uint32_t L2B_LINESKIP;                     /*!< Offset:0x24    */
    __IO uint32_t RESERVED1;                        /*!< Offset:0x28     */
    union {
        __IO uint32_t  CIS_AE_SIZE_SKIP;            /*!< Offset:0x2C CIS_AE_SIZE_SKIP                                           */

        struct {
            __IO uint32_t  SIZE_X           :  7;  
            __IO uint32_t  RESERVED0        :  1;
            __IO uint32_t  SIZE_Y           :  7;
            __IO uint32_t  RESERVED         :  1;
            __IO uint32_t  SKIP_X           :  4;
            __IO uint32_t  SKIP_Y           :  4;                

        } CIS_AE_SIZE_SKIP_b;                                                                                             
    };
    
    union {
    __IO uint32_t  CIS_AE_START_XY;                 /*!< Offset:0x30 CIS_AE_START_XY                                           */

        struct {
            __IO uint32_t  STARTX           :  12;  
            __IO uint32_t  RESERVED0        :  4;
            __IO uint32_t  STARTY           :  11;              

        } CIS_AE_START_XY_b;                                                                                             
    };
    
    union {
    __IO uint32_t  CIS_AE_YHIST;                    /*!< Offset:0x34 CIS_AE_YHIST                                           */
        
        struct {
            __IO uint32_t  YHIST_CNT_R      :  16;  
            __IO uint32_t  YHIST_IDX        :  5;           

        } CIS_AE_YHIST_b;                                                                                             
    };
    
    union {
    __IO uint32_t  CIS_AE_YHIST_MODE;               /*!< Offset:0x38 CIS_AE_YHIST_MODE                                          */

        struct {
            __IO uint32_t  RESERVED0        :  8;
            __IO uint32_t  YHIST_MODE       :  4;  
            __IO uint32_t  RESERVED1        :  5;
            __IO uint32_t  RDY              :  1;   
            __IO uint32_t  RESERVED2        :  1; 
            __IO uint32_t  AE_RPT_EN        :  1;     

        } CIS_AE_YHIST_MODE_b;                                                                                             
    };
    
    union {
    __IO uint32_t  CIS_AE_REPORT;                   /*!< Offset:0x3C CIS_AE_REPORT                                          */
        
        struct {
            __IO uint32_t  YREPORT          :  21;
            __IO uint32_t  RESERVED0        :  3;  
            __IO uint32_t  YREPORT_IDX      :  4;       
        } CIS_AE_REPORT_b;                                                                                             
    };
    
    union {
    __IO uint32_t  CIS_AE_MAX_MIN;                  /*!< Offset:0x40 CIS_AE_REPORT                                          */
        
        struct {
            __IO uint32_t  MAX_Y            :  8;
            __IO uint32_t  RESERVED0        :  8;  
            __IO uint32_t  MIN_Y            :  8;       
        } CIS_AE_MAX_MIN_b;                                                                                             
    };
} sn_cis_t;
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
