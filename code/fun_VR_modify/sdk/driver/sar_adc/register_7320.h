/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author kaishiang
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
//      Constant Definition
//=============================================================================
/**
 *  7320 definition
 */
#define SN_SAR_ADC_BASE                 0x4003A000UL //SNC7320 NEW
#define SN_SAR_ADC                      ((SN_SAR_ADC_Type       *) SN_SAR_ADC_BASE)

//=============================================================================
//      Macro Definition
//=============================================================================

//=============================================================================
//      Structure Definition
//=============================================================================
/* =========================================================================================================================== */
/* ====================================                    SN_SPI                      ======================================= */
/* =========================================================================================================================== */
/**
  * @brief SPI (SN_SPI)
  */

/* =========================================================================================================================== */
/* ====================================                   SN_ADC                       ======================================= */
/* =========================================================================================================================== */
/**
  * @brief
  */

typedef struct {                                    /*!< SN_SAR_ADC Structure for SAR_ADC                                     */

  union {
  __IO uint32_t  ADP;                           /*!< Offset:0x00                                       */
  struct {
   __IO uint32_t  RESERVED0    :  13;   
   __IO uint32_t  CHS       :  3;         /*!< CHS ? analog input channel select bin                        */
                              /*!< 000/111 : all analog input disable                           */
                              /*!< 001 : AIN0 is analog input                             */
                              /*!< 010 : AIN1 is analog input                              */
                              /*!< 011 : AIN2 is analog input                             */
                              /*!< 100 : AIN3 is analog input                             */
                          /*!< 101 : AIN4 is analog input                             */
                          /*!< 111 : AIN5 is analog input                             */
  __IO uint32_t  RESERVED1    :  16;
  } ADP_b;                                        /*!< BitSize                                                             */
  
 }; 
 
 union {
  __IO uint32_t  ADM;                           /*!< Offset:0x04                                       */
  struct {
   __IO uint32_t  START     :  1;           /*!< 0: stop, 1: start                                                   */
   __IO uint32_t  VALID     :  1;           /*!< 1:data is valid (Read only)                                         */
   __IO uint32_t  S_MODE     :  1;      /*!< 0: Normal SAR ADC mode, 1: Single mode                              */
   __IO uint32_t  RESERVED3   :  6;
   __IO uint32_t  ADCKS     :  3;           /*!< ADC’s clock source select bit                                       */
   __IO uint32_t  AD_2M_CLK   :  1;           /*!< ADC’s clock source group select bit                                 */   
   __IO uint32_t  RESERVED4   :  2;
   __IO uint32_t  ADC_EN     :  1;           /*!< 1:ADC enable                                                         */
   __IO uint32_t  RESERVED5   :  16;   
  } ADM_b;                                        /*!< BitSize                                                             */
 };
 
 union {
  __IO uint32_t  ADR;                           /*!< Offset:0x04                                       */
  struct {
   __IO uint32_t  RESERVED6   :  6;
   __IO uint32_t  DATA      :  10;
   __IO uint32_t  RESERVED7   :  16;
  } ADR_b;                                       /*!< Offset:0x08 12-bit ADC output data at MSB, and LSB 6 bit is ??    */
 };
 
}SN_SAR_ADC_Type;


//=============================================================================
//      Global Data Definition
//=============================================================================

//=============================================================================
//      Private Function Definition
//=============================================================================

//=============================================================================
//      Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
