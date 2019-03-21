/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/30
 * @license
 * @description
 */

#ifndef __register_crc_H_
#define __register_crc_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
    
//=============================================================================
//                  Constant Definition
//=============================================================================

#define SN_CRC_BASE         0x40038000
#define SN_CRC              ((SN_CRC_TYPE   *) SN_CRC_BASE)
    
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct {                /*!< SN_CRC Structure                           */
  
  union {
    __IO uint32_t	CTRL;       /*!< Offset:0x00 CRC Control Register           */
    
    struct {        
      __IO uint32_t	Mode                :  1;   /*!<    */
      __O  uint32_t Reserved0           :  3;   /*!<    */
      __IO uint32_t CRC_EN              :  1;   /*!<    */
      __I  uint32_t Reserved1           :  3;   /*!<    */
      __O  uint32_t CRC_EN_RDY          :  1;   /*!<    */
      __O  uint32_t CRC_OK              :  1;   /*!<    */
      __O  uint32_t Reserved2           :  22;  /*!<    */
    } CRC_CTRL_b;
  };
  __IO uint32_t CRC_ADDR;               /*!< Offset:0x04 CRC Start Address Register  */
  __IO uint32_t R_LINE_SIZE;            /*!< Offset:0x08 */
  __IO uint32_t R_LINE_NUM;             /*!< Offset:0x0C */
  __IO uint32_t GAP;                    /*!< Offset:0x10 */
 
  
  union {
    __IO uint32_t	CRC_SET;            /*!< Offset:0x14 CRC Start Address Register  */   
    
    struct{
      __IO uint32_t Init                :   1;  /* CRC Init value: 0- Init with zero, 1- Init witf 0xFFFF FFFF */
      __IO uint32_t XOR                 :   1;  /* CRC output XOR value: 0- Init with zero, 1- Init witf 0xFFFF FFFF */
      __IO uint32_t Reserved            :   2;  
      __IO uint32_t Order               :   5;  /* Order number of CRC Polynomialm - 1  */
    }CRC_SET_b;
  };
  
  __IO uint32_t CRC_POLY;               /*!< Offset:0x18 CRC Start Address Register  */
  __IO uint32_t CHECKSUM;               /*!< Offset:0x1C CRC Start Address Register  */
  
}SN_CRC_TYPE;
    
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
