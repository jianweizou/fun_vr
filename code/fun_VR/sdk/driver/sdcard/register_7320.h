
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
/* =========================================================================================================================== */
/* ====================================                 SN_SDCARD0                     ======================================= */
/* =========================================================================================================================== */
/**
  * @brief SDCARD Control Registers (SN_SDCARD)
  */
#pragma anon_unions

typedef struct { 

    __IO uint32_t  SPI_CMD_ARGL;                    /*!< Offset:0x00 SD Card Command Argument Low Word	[15:0]								 */
    __IO uint32_t  SPI_CMD_ARGH;       	            /*!< Offset:0x04 SD Card Command Argument High Word	[15:0]								 */
    __IO uint32_t  SPI_CMD_INDEX;                   /*!< Offset:0x08 SD Card Command Index	[7:0]															 */

  union {
    __IO uint32_t  SD_CMD;                          /*!< Offset:0x0C SD Card Command Register																	 */
    
    struct {         
      __I  uint32_t  RESERVED0   	:  1;
      __IO uint32_t  CM12_SEL    	:  2; 
      __IO uint32_t  CM25_18_SEL 	:  2; 
      __IO uint32_t  RESERVED1   	:  3;
      __IO uint32_t  SD_CMD_NIB  	:  4;  
    } SD_CMD_b;                             
  };                                   
    __IO uint32_t  RESERVED0; 										  /*!< Offset:0x10                   																				 */
  union {
    __IO uint32_t  SD_STAT;                         /*!< Offset:0x14 SD Card Status Register                                   */
    
    struct {                                               
	    __IO  uint32_t  TRAN_ERR 		 :  1;
      __IO  uint32_t  TRAN_DONE    :  1; 
      __I   uint32_t  RESERVED0  	 :  2;	    
	    __IO  uint32_t  CRC_ERR 		 :  1;
      __I   uint32_t  RESERVED1  	 :  1;	 
      __IO  uint32_t  WR_DONE   	 :  1;   
      __I   uint32_t  RESERVED2  	 :  3;	 
      __IO  uint32_t  TO_DONE   	 :  1;   
    } SD_STAT_b;                        
  };   
    __IO uint32_t  RESERVED1; 									    /*!< Offset:0x18                                                           */
  union {
    __IO uint32_t  SD_CTRL;                         /*!< Offset:0x1C SD Card Control Register                                  */
    
    struct {                                               
	    __IO  uint32_t  SD_MODE 		 :  2;
      __O   uint32_t  SPI_CMD_TRIG :  1;
      __O   uint32_t  SPI_BUSY_TRIG : 1;
	    __I   uint32_t  SPI_DONE 		 :  1;
      __IO  uint32_t  SPI_RW   	   :  1;
      __IO  uint32_t  CRC_EN	   	 :  2;
      __IO  uint32_t  CMD_DATA_R   :  1;
    } SD_CTRL_b;                                    /*!< BitSize                                                               */
  }; 
    __IO uint32_t  SD_RW_LEN; 										  /*!< Offset:0x20                                                           */
    __IO uint32_t  RESERVED3; 										  /*!< Offset:0x24                                                           */

  union {
    __IO uint32_t  RESPONSE_MODE;                   /*!< Offset:0x28 SD Card Response Register                                 */    
    struct {                                               
	    __IO  uint32_t  NUM	         :  4;
      __IO  uint32_t  MODE  	     :  1;
    } RESPONSE_MODE_b;                              /*!< BitSize                                                               */
  }; 

		__IO uint32_t  SD_SPEED;                        /*!< Offset:0x2C SD Card Speed Control Register                            */
		__IO uint32_t  RESERVED4; 						          /*!< Offset:0x30  [15:0]                                                   */
    __IO uint32_t  CRC7; 	    							        /*!< Offset:0x34 SD Card 7bit CRC for CMD/RESPONSE	[6:0]									 */
    __IO uint32_t  CRC16_0; 							          /*!< Offset:0x38 SD Card 16bit CRC for DATA0	[15:0]											 */
    __IO uint32_t  CRC16_1; 							          /*!< Offset:0x3C SD Card 16bit CRC for DATA1  [15:0]											 */
    __IO uint32_t  CRC16_2; 							          /*!< Offset:0x40 SD Card 16bit CRC for DATA2	[15:0]											 */
    __IO uint32_t  CRC16_3; 							          /*!< Offset:0x44 SD Card 16bit CRC for DATA3	[15:0]											 */
    __IO uint32_t  TIME_CNT_L; 						          /*!< Offset:0x48 SD Card Time out counter [15:0]													 */
    __IO uint32_t  TIME_CNT_H; 						          /*!< Offset:0x4C SD Card Time out counter [29:16]													 */
    __IO uint32_t  RESERVED5; 						          /*!< Offset:0x50	                                                         */
    __IO uint32_t  RESERVED6; 						          /*!< Offset:0x54	                                                         */
    __IO uint32_t  RESERVED7; 			                /*!< Offset:0x58	                                                         */
    __IO uint32_t  RESERVED8;											  /*!< Offset:0x5C	                                                         */
    __IO uint32_t  RESERVED9;											  /*!< Offset:0x60	                                                         */
    		
	union {
		__IO uint32_t  SDIO_MODE;						/*!< Offset:0x64	SDIO_MODE Control                                        */
	struct {
		__IO	uint32_t	SDIO_EN					:	1;					
		__IO	uint32_t	SDIO_4BIT_MODE			:	1;
		__IO	uint32_t	SDIO_INT_EN				:	1;
		__I		uint32_t	SDIO_INT_FLG			:	1;
		__O		uint32_t	CLR_SDIO_INT			:	1;
		__IO	uint32_t	SDIO_IO_ABORT_EN		:	1;//Dummy for test
		__IO	uint32_t	RESERVED0				:	2;
		__IO	uint32_t	SDIO_REG_FUNC			:	3;
		__IO	uint32_t	RESERVED1				:	1;				
		__IO	uint32_t	SDIO_REG_OPCODE			:	1;
		__IO	uint32_t	SDIO_REG_BLK			:	1;
		}SDIO_MODE_b;
	};											  
    __IO uint32_t  RESERVEDB;											  /*!< Offset:0x68	                                                         */
    __IO uint32_t  RESERVEDC;											  /*!< Offset:0x6C	                                                         */
    __IO uint32_t  SD_REG1; 	 				              /*!< Offset:0x70	[7:0] 1ch                                                */
    __IO uint32_t  SD_REG2; 					              /*!< Offset:0x74	[7:0] 1dh                                                */
    __IO uint32_t  SD_REG3; 					              /*!< Offset:0x78	[7:0] 1eh                                                */
    __IO uint32_t  SD_REG4; 					              /*!< Offset:0x7C	[7:0] 1fh                                                */
} SN_SDCARD_Type;

typedef struct {                                    /*!< SN_IDMA Structure                                                     */
  

    __IO uint32_t  CH_RAM_ADDR;                     /*!< Offset:0x00 Ram address                                               */
    
    __IO uint32_t  CH_LEN;  
	                      
  union {
    __IO uint32_t  CH_CTRL;                       	/*!< Offset:0x08 Comparator Capture Output Match register                  */
    
    struct {
		__IO uint32_t  START              :  1;
		__IO uint32_t  WR_RAM             :  1;
		__IO uint32_t  PRAMM              :  1;
 		__IO uint32_t  AD_PASS            :  1;
 		__IO uint32_t  SCALAR             :  2;
    } CH_CTRL_b;                                    /*!< BitSize                                                               */
  };
    __IO uint32_t  CH_DIP_ADDR;

} SN_IDMA_Type;

#define SN_SDC_BASE		0x40023000UL
#define SN_SDC				((SN_SDCARD_Type	*) SN_SDC_BASE)
#define SN_IDMA_SDC_BASE             		0x40023110UL
#define SN_IDMA_SDC											((SN_IDMA_Type           *) SN_IDMA_SDC_BASE) 

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
