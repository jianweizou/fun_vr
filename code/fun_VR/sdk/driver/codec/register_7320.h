/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Kaishiang
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
#define SN_I2S0_BASE                    0x40030000UL
#define SN_I2S1_BASE                    0x40031000UL
#define SN_I2S2_BASE                    0x40032000UL
#define SN_I2S3_BASE                    0x40033000UL	//SNC7320 NEW
#define SN_I2S4_BASE                    0x40034000UL	//SNC7320 NEW
#define SN_SPIFC_BASE					0x40022000UL	//SNC7320 NEW SN_FLASH_BASE 
#define SN_GPIO0_BASE                   0x40018000UL    

#define SN_OPM_SYS1_BASE								0x45000100UL    
#define SN_FLASH                        ((SN_FLASH_Type           *) SN_SPIFC_BASE)
#define SN_SYS1                        	((SN_PERI_Type            *) SN_OPM_SYS1_BASE) 
#define SN_GPIO0                        ((SN_GPIO0_Type           *) SN_GPIO0_BASE)    
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/* =========================================================================================================================== */
/* ====================================                    SN_I2S#0~#3                      ======================================= */
/* =========================================================================================================================== */
/**
  * @brief I2S DMA for aud interface (SN_I2S)
  */

typedef struct {                                    /*!< SN_I2S Structure																											*/
  
  union {
    __IO uint32_t  CTRL;                            /*!< Offset:0x00 I2S Control Register																			*/
    struct {
      __IO uint32_t  START      			:  1;         		/*!< Start Transmit/Receive																						*/
      __IO uint32_t  MUTE       			:  1;         		/*!< Mute enable																											*/
      __IO uint32_t  MONO      				:  1;         		/*!< Mono/stereo selection																						*/
      __IO uint32_t  MS         			:  1;         		/*!< Master/Slave selection bit																				*/
      __IO uint32_t  FORMAT    				:  2;         		/*!< I2S operation format																							*/
      __IO uint32_t  TXEN      			 	:  1;         		/*!< Transmit enable bit																							*/
      __IO uint32_t  RXEN       			:  1;         		/*!< Receiver enable bit																							*/
      __O  uint32_t  CLRTXFIFO  			:  1;         		/*!< Clear I2S TX FIFO																								*/
      __O  uint32_t  CLRRXFIFO  			:  1;         		/*!< Clear I2S RX FIFO																								*/
      __IO uint32_t  DL         			:  2;         		/*!< Data length																											*/
      __IO uint32_t  PAUSE						:  1;          		/*!< PAUSE Mode																												*/
      __I	 uint32_t  RESERVED00  			:  7;
      __IO uint32_t  CHLENGTH   			:  5;         		/*!< Bit number of single channel																			*/
      __I  uint32_t  RESERVED01  			:  3;
			__IO uint32_t  I2S_DIRECTION    :  1;          		/*!< SDIO input output selection																			*/
      __I  uint32_t  RESERVED02  			:  2;
      __IO uint32_t  I2SEN      			:  1;          		/*!< I2S enable																												*/
    } CTRL_b;                                       /*!< BitSize																															*/
  };
  
  union {
    __IO uint32_t  CLK;                             /*!< Offset:0x04 I2S Clock Register																				*/
    struct {
      __IO uint32_t  IMCLKDIV    			:  3;         		/*!< IMCLK -> IMCLKDIV -> BCLK, For BCLK PreScaler										*/
			__IO uint32_t  RESERVED00				:  1;							/*!< 12M MCLK output enable										*/
			__IO uint32_t  IMCLKSEL					:  2;							/*!< I2S IP Clock Source Select																				*/
      __I  uint32_t  RESERVED01  			:  2;
      __IO uint32_t  BCLKDIV    			:  8;         		/*!< BCLK -> BCLKDIV -> BCLK PAD, BCLK divider												*/
			__IO uint32_t  FPLL_DIV    			:  2;         		/*!< 00: FPLL VCO/9, 01:FPLL VCO/10 10:FPLL VCO/14										*/
      __I  uint32_t  RESERVED02  			:  14;
    } CLK_b;                                        /*!< BitSize																															*/
  };
  
  union {
    __I  uint32_t  STATUS;                          /*!< Offset:0x08 I2S Status Register																			*/
    struct {
      __I  uint32_t  TX_TOG    			 	:  1;             /*!< TX Toggle																												*/
      __I  uint32_t  RX_TOG     			:  1;             /*!< Rx Toggle																												*/
      __I  uint32_t  RESERVED00  			:  30;
    } STATUS_b;                                     /*!< BitSize																															*/
  };
  
  union {
    __IO uint32_t  IE;                              /*!< Offset:0x0C I2S Interrupt Enable Register														*/
    struct {
      __IO uint32_t  TXINTEN					:  1;             /*!< TX interrupt enable																							*/
      __IO uint32_t  RXINTEN					:  1;             /*!< RX interrupt enable																							*/
      __I  uint32_t  RESERVED00  			:  30;
    } IE_b;                                         /*!< BitSize																															*/
  };
  
  union {
    __IO uint32_t  RAM1_Len;                        /*!< Offset:0x10 I2S RAM Length Register																	*/
    struct {
      __IO uint32_t  RAM1_Len					:  16;            /*!< Transfer ram size																								*/
      __I  uint32_t  RESERVED00  			:  16;
    } RAM1_Len_b;                                   /*!< BitSize																															*/
  };
  
	__I  uint32_t  RESERVED00[3];											/*!< RESERVED 0x14~0x1C*/ 
	__IO  uint32_t  TxRAM1_Address;											/*!< Offset:0x20 , Transfer ram start address															*/ 
	__IO  uint32_t  TxRAM2_Offset;											/*!< Offset:0x24 , Ram2 offset from Ram1																	*/ 
	__IO  uint32_t  TxIndex;														/*!< Offset:0x28 , Ram transfer index																			*/ 
	
	__I  uint32_t  RESERVED01[1];											/*!< RESERVED 0x2C*/ 
	__IO  uint32_t  RxRAM1_Address;											/*!< Offset:0x30 , Transfer ram start address															*/ 
	__IO  uint32_t  RxRAM2_Offset;											/*!< Offset:0x34 , Ram2 offset from Ram1																	*/ 
	__IO  uint32_t  RxIndex;														/*!< Offset:0x38 , Ram transfer index																			*/ 
	

} SN_I2S_Type;

/* =========================================================================================================================== */
/* ====================================                    SN_I2S4                     ======================================= */
/* =========================================================================================================================== */
/**
  * @brief I2S_2 (SN_I2S_2)
  */

typedef struct {                                    /*!< SN_I2S_2 Structure                                                    */
  
  union {
    __IO uint32_t  CTRL;                            /*!< Offset:0x00 I2S Control Register                                      */
    
    struct {
      __IO uint32_t  START      :  1;               /*!< Start Transmit/Receive                                                */
      __IO uint32_t  MUTE       :  1;               /*!< Mute enable                                                           */
      __IO uint32_t  MONO       :  1;               /*!< Mono/stereo selection                                                 */
      __IO uint32_t  MS         :  1;               /*!< Master/Slave selection bit                                            */
      __IO uint32_t  FORMAT     :  2;               /*!< I2S operation format                                                  */
      __IO uint32_t  TXEN       :  1;               /*!< Transmit enable bit                                                   */
      __O  uint32_t  RXEN       :  1;               /*!< Receiver enable bit                                                   */
      __O  uint32_t  CLRTXFIFO  :  1;               /*!< Clear I2S TX FIFO                                                     */
      __O  uint32_t  CLRRXFIFO  :  1;               /*!< Clear I2S RX FIFO                                                     */
      __IO uint32_t  DL         :  2;               /*!< Data length                                                           */
      __IO uint32_t  TXFIFOTH   :  3;               /*!< Tx FIFO threshold level                                               */
           uint32_t             :  1;
      __IO uint32_t  RXFIFOTH   :  3;               /*!< Rx FIFO threshold level                                               */
           uint32_t             :  1;
      __IO uint32_t  CHLENGTH   :  5;               /*!< Bit number of single channel                                          */
           uint32_t             :  6;
      __IO uint32_t  I2SEN      :  1;               /*!< I2S enable                                                            */
    } CTRL_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  CLK;                             /*!< Offset:0x04 I2S Clock Register                                        */
    
    struct {
      __IO uint32_t  IMCLKDIV    			:  3;         		/*!< IMCLK -> IMCLKDIV -> BCLK, For BCLK PreScaler										*/
			__IO uint32_t  MCLKOEN					:  1;							/*!< 12M MCLK output enable										*/
			__IO uint32_t  IMCLKSEL					:  2;							/*!< I2S IP Clock Source Select																				*/
      __IO uint32_t  MCLK_OUT0_SEL		:  2;							/*!< Select MCLK Output Pad 0 clock source, For Aud Chip Clock				*/
      __IO uint32_t  BCLKDIV    			:  8;         		/*!< BCLK -> BCLKDIV -> BCLK PAD, BCLK divider												*/
			__IO uint32_t  FPLL_DIV    			:  2;         		/*!< 00: FPLL VCO/9, 01:FPLL VCO/10 10:FPLL VCO/14										*/
      __IO uint32_t  MCLK_OUT1_SEL		:  2;							/*!< Select MCLK Output Pad 1 clock source, For Aud Chip Clock				*/
      __I  uint32_t  RESERVED01  			:  12;
    } CLK_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  STATUS;                          /*!< Offset:0x08 I2S Status Register                                       */
    
    struct {
      __I  uint32_t  I2SINT     :  1;               /*!< I2S interrupt flag                                                    */
      __I  uint32_t  RIGHTCH    :  1;               /*!< Current channel status                                                */
           uint32_t             :  4;
      __I  uint32_t  TXFIFOTHF  :  1;               /*!< TX FIFO threshold flag                                                */
      __I  uint32_t  RXFIFOTHF  :  1;               /*!< RX FIFO threshold flag                                                */
      __I  uint32_t  TXFIFOFULL :  1;               /*!< TX FIFO full flag                                                     */
      __I  uint32_t  RXFIFOFULL :  1;               /*!< RX FIFO full flag                                                     */
      __I  uint32_t  TXFIFOEMPTY:  1;               /*!< TX FIFO empty flag                                                    */
      __I  uint32_t  RXFIFOEMPTY:  1;               /*!< RX FIFO empty flag                                                    */
      __I  uint32_t  TXFIFOLV   :  4;               /*!< TX FIFO used level                                                    */
           uint32_t             :  1;
      __I  uint32_t  RXFIFOLV   :  4;               /*!< RX FIFO used level                                                    */
    } STATUS_b;                                     /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IE;                              /*!< Offset:0x0C I2S Interrupt Enable Register                             */
    
    struct {
           uint32_t             :  4;
      __IO uint32_t  TXFIFOOVFIEN:  1;              /*!< TX FIFO overflow interrupt enable                                     */
      __IO uint32_t  RXFIFOUDFIEN:  1;              /*!< RX FIFO underflow interrupt enable                                    */
      __IO uint32_t  TXFIFOTHIEN:  1;               /*!< TX FIFO threshold interrupt enable                                    */
      __IO uint32_t  RXFIFOTHIEN:  1;               /*!< RX FIFO threshold interrupt enable                                    */
    } IE_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  RIS;                             /*!< Offset:0x10 I2S Raw Interrupt Status Register                         */
    
    struct {
           uint32_t             :  4;
      __I  uint32_t  TXFIFOOVIF :  1;               /*!< TX FIFO overflow interrupt flag                                       */
      __I  uint32_t  RXFIFOUDIF :  1;               /*!< RX FIFO underflow interrupt flag                                      */
      __I  uint32_t  TXFIFOTHIF :  1;               /*!< TX FIFO threshold interrupt flag                                      */
      __I  uint32_t  RXFIFOTHIF :  1;               /*!< RX FIFO threshold interrupt flag                                      */
    } RIS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  IC;                              /*!< Offset:0x14 I2S Interrupt Clear Register                              */
    
    struct {
           uint32_t             :  4;
      __O  uint32_t  TXFIFOOVIC :  1;               /*!< TX FIFO overflow interrupt clear                                      */
      __O  uint32_t  RXFIFOUDIC :  1;               /*!< RX FIFO underflow interrupt clear                                     */
      __O  uint32_t  TXFIFOTHIC :  1;               /*!< TX FIFO threshold interrupt clear                                     */
      __O  uint32_t  RXFIFOTHIC :  1;               /*!< RX FIFO threshold interrupt clear                                     */
    } IC_b;                                         /*!< BitSize                                                               */
  };
	
	__I  uint32_t  RXFIFO;                          	/*!< Offset:0x18 UARTn RX FIFO Register                                    */
	__O  uint32_t  TXFIFO;                            /*!< Offset:0x1C UARTn TX FIFO Register                                    */
	
  
} SN_I2S4_Type;



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
