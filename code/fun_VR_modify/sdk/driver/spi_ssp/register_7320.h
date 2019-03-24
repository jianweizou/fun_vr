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
#define SN_SPI0_BASE                    0x4000E000UL
#define SN_SPI1_BASE                    0x4000F000UL
#define SN_SPI0                         ((SN_SPI_Type             *) SN_SPI0_BASE)
#define SN_SPI1                         ((SN_SPI_Type             *) SN_SPI1_BASE)
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

typedef struct {                                    /*!< SN_SSP0 Structure                                                     */
  
  union {
    __IO uint32_t  CTRL0;                           /*!< Offset:0x00 SSPn Control Register 0                                   */
    
    struct {
      __IO uint32_t  SPIEN      :  1;               /*!< SSP enable                                                            */
      __IO uint32_t  LOOPBACK   :  1;               /*!< Loopback mode enable                                                  */
      __IO uint32_t  SDODIS     :  1;               /*!< Slave data out disable                                                */
      __IO uint32_t  MS         :  1;               /*!< Master/Slave selection                                                */
      __IO uint32_t  FORMAT     :  1;               /*!< Interface format                                                      */
           uint32_t             :  1;
      __O  uint32_t  FRESET     :  2;               /*!< SSP FSM and FIFO Reset                                                */
      __IO uint32_t  DL         :  4;               /*!< Data length = DL[3:0]+1                                               */
      __IO uint32_t  TXFIFOTH   :  3;               /*!< TX FIFO Threshold level                                               */
      __IO uint32_t  RXFIFOTH   :  3;               /*!< Rx FIFO Threshold level                                               */
      __IO uint32_t  SELDIS     :  1;               /*!< Auto-SEL disable bit                                                  */
      __IO uint32_t  SELCTRL    :  1;               /*!< Source for SEL pin                                                    */
    } CTRL0_b;                                      /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  CTRL1;                           /*!< Offset:0x04 SSPn Control Register 1                                   */
    
    struct {
      __IO uint32_t  MLSB       :  1;               /*!< MSB/LSB seletion                                                      */
      __IO uint32_t  CPOL       :  1;               /*!< Clock priority selection                                              */
      __IO uint32_t  CPHA       :  1;               /*!< Clock phase of edge sampling                                          */
    } CTRL1_b;                                      /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  CLKDIV;                          /*!< Offset:0x08 SSPn Clock Divider Register                               */
    
    struct {
      __IO uint32_t  DIV        :  8;               /*!< SSPn SCK=SSPn_PCLK/(2*DIV+2)                                          */
    } CLKDIV_b;                                     /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  STAT;                            /*!< Offset:0x0C SSPn Status Register                                      */
    
    struct {
      __I  uint32_t  TX_EMPTY   :  1;               /*!< TX FIFO empty flag                                                    */
      __I  uint32_t  TX_FULL    :  1;               /*!< TX FIFO full flag                                                     */
      __I  uint32_t  RX_EMPTY   :  1;               /*!< RX FIFO empty flag                                                    */
      __I  uint32_t  RX_FULL    :  1;               /*!< RX FIFO full flag                                                     */
      __I  uint32_t  BUSY       :  1;               /*!< Busy flag                                                             */
      __I  uint32_t  TXFIFOTHF  :  1;               /*!< TX FIFO threshold flag                                                */
      __I  uint32_t  RXFIFOTHF  :  1;               /*!< RX FIFO threshold flag                                                */
    } STAT_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IE;                              /*!< Offset:0x10 SSPn Interrupt Enable Register                            */
    
    struct {
      __IO uint32_t  RXOVFIE    :  1;               /*!< RX FIFO overflow interrupt enable                                     */
      __IO uint32_t  RXTOIE     :  1;               /*!< RX time-out interrupt enable                                          */
      __IO uint32_t  RXFIFOTHIE :  1;               /*!< RX FIFO threshold interrupt enable                                    */
      __IO uint32_t  TXFIFOTHIE :  1;               /*!< TX FIFO threshold interrupt enable                                    */
    } IE_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  RIS;                             /*!< Offset:0x14 SSPn Raw Interrupt Status Register                        */
    
    struct {
      __I  uint32_t  RXOVFIF    :  1;               /*!< RX FIFO overflow interrupt flag                                       */
      __I  uint32_t  RXTOIF     :  1;               /*!< RX time-out interrupt flag                                            */
      __I  uint32_t  RXFIFOTHIF :  1;               /*!< RX FIFO threshold interrupt flag                                      */
      __I  uint32_t  TXFIFOTHIF :  1;               /*!< TX FIFO threshold interrupt flag                                      */
    } RIS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  IC;                              /*!< Offset:0x18 SSPn Interrupt Clear Register                             */
    
    struct {
      __O  uint32_t  RXOVFIC    :  1;               /*!< RX FIFO overflow flag clear                                           */
      __O  uint32_t  RXTOIC     :  1;               /*!< RX time-out interrupt flag clear                                      */
      __O  uint32_t  RXFIFOTHIC :  1;               /*!< RX Interrupt flag Clear                                               */
      __O  uint32_t  TXFIFOTHIC :  1;               /*!< TX Interrupt flag Clear                                               */
    } IC_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  DATA;                            /*!< Offset:0x1C SSPn Data Register                                        */
    
    struct {
      __IO uint32_t  Data       : 16;               /*!< Data                                                                  */
    } DATA_b;                                       /*!< BitSize                                                               */
  };
} SN_SPI_Type;

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
