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
//      Constant Definition
//=============================================================================
/**
 *  7320 definition
 */
#define SN_GPIO0_BASE                   0x40018000UL
#define	SN_SPI0_DMA_IP_BASE							0x4002B000UL	
#define	SN_SPI1_DMA_IP_BASE							0x4002C000UL	
#define	SN_SPI0_DMA_BASE								0x4002B100UL	
#define	SN_SPI1_DMA_BASE								0x4002C100UL	

#define	SN_SPI0_DMA_IP                  ((SN_SPI_DMA            	*) SN_SPI0_DMA_IP_BASE)
#define	SN_SPI1_DMA_IP                  ((SN_SPI_DMA            	*) SN_SPI1_DMA_IP_BASE)
#define	SN_SPI0_DMA                    	((SN_IDMA_Type            *) SN_SPI0_DMA_BASE)
#define	SN_SPI1_DMA                    	((SN_IDMA_Type            *) SN_SPI1_DMA_BASE)
	
#define SN_GPIO0                        ((SN_GPIO0_Type           *) SN_GPIO0_BASE)	
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

typedef struct {                                  /*!< SN_SPI_DMA Structure                                                       */
  
  union {
    __IO uint32_t	SPICTRL;								/*!< Offset:0x00 SPI Control Register                         */	
    struct {
      __IO uint32_t	START     				    :  1;   	/*!< Set this bit to start SPI transfer/receive auto clear after operation finished */
      __IO uint32_t	SPI_3W     				    :  1;   	/*!< 0: MISO = Input SPIBUFF = MISO_I, 1: MISO = Output*/
			__IO uint32_t	SPI_MOSI_EN     			:  1;   	/*!< 0: MOSI = GPIO, 1: MOSI = MOSI */
      __I uint32_t	Reserved0     			 	:  3;   	/*!< */
			__IO uint32_t ENDIAN_SEL						:  1;			/*!< 0: Little Endian(ARM Default), 1: Big Endian*/
      __IO uint32_t	SPIEN     				    :  1;   	/*!< Enable bit*/
      __IO uint32_t	CPOL     				    	:  1;   	/*!< Clock polarity*/
      __IO uint32_t	CPHA     				    	:  1;   	/*!< Clock phase*/
      __I uint32_t	Reserved1     			 	:  2;   	/*!< */
      __IO uint32_t	DUAL     				    	:  1;   	/*!< 2bit mode*/
      __IO uint32_t	QUAD     				    	:  1;   	/*!< 4bit mode*/
      __IO uint32_t	MSMODE     				    :  1;   	/*!< SPI master/slave mode*/
      __IO uint32_t	RWMODE     				    :  1;   	/*!< SPI io control*/
      __I uint32_t	Reserved2     				:  16;   	/*!< */
    }SPICTRL_b;                         	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPIBAUD;								/*!< Offset:0x04 SPI Baud Rate Register                         */	
    struct {
      __IO uint32_t	SPIDIV     			 			:  8;   	/*!< */
      __IO uint32_t	SPIPRS     			 			:  3;   	/*!< */
      __I uint32_t	Reserved0     			 	:  21;   	/*!< */
    }SPIBAUD_b;                         	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPICS;									/*!< Offset:0x08 SPI Control Select Register                         */	
    struct {
      __IO uint32_t	CS     			 					:  1;   	/*!< Hardware of Software controls Chip1 Select bit*/
      __I uint32_t	Reserved0     			 	:  3;   	/*!< */
      __IO uint32_t	CSPOL     			 			:  1;   	/*!< Polarization (valid on hardware control CS)*/
      __I uint32_t	Reserved1     			 	:  9;   	/*!< */
      __IO uint32_t	SW_CS     			 			:  1;   	/*!< *Software control CS*/
      __I uint32_t	Reserved2     			 	:  1;   	/*!< */
      __I uint32_t	Reserved3     			 	:  16;   	/*!< */
    }SPICS_b;                         		/*!< BitSize                                                                */
  };
	
  __IO uint32_t	SPITRANSFER;							/*!< Offset:0x0C SPI Transfer Bit Register                       */
  __IO uint32_t	SPIDATABUF0;							/*!< Offset:0x10 SPI Transfer / Receive Data Register 0          */
  __IO uint32_t	SPIDATABUF1;							/*!< Offset:0x14 SPI Transfer / Receive Data Register 1          */
  __IO uint32_t	SPIDATABUF2;							/*!< Offset:0x18 SPI Transfer / Receive Data Register 2          */
  __IO uint32_t	SPIDATABUF3;							/*!< Offset:0x1C SPI Transfer / Receive Data Register 3          */
  __IO uint32_t	SPIDATABUF4;							/*!< Offset:0x20 SPI Transfer / Receive Data Register 4          */
  __IO uint32_t	SPIDATABUF5;							/*!< Offset:0x24 SPI Transfer / Receive Data Register 5          */
	__IO uint32_t SPITCW;										/*!< Offset:0x28 SPITCW for reset TC          */
	
  __IO uint32_t	Reserved0[21];						/*!< Offset:0x2C~0x7C          */
	
  union {
    __IO uint32_t	SPI_ECC_STATUS;					/*!< Offset:0x80 SPI ECC Status Register                         */	
    struct {
      __I uint32_t	ECC_Encode_ok     		:  1;   	/*!< ECC Encode done*/
      __I uint32_t	ECC_Decode_ok     		:  1;   	/*!< ECC Decode done*/
      __I uint32_t	ECC_ERR_occur     		:  1;   	/*!< ECC Engine Detect Error*/
      __I uint32_t	ECC_Dec_fail     			:  1;   	/*!< Detect Fail*/
      __I uint32_t	ECC_Error_Number     	:  4;   	/*!< ECC Error Number*/
      __I uint32_t	SPI_ECC_CORR     			:  1;   	/*!< ECC done*/
      __I uint32_t	ECC_REQ     					:  1;   	/*!< 1: No more address of ECC-error-bit to output*/
      __I uint32_t	ECC_RW     						:  1;   	/*!< ECC Data Read Write*/
      __I uint32_t	ECC_DMA_DN     				:  1;   	/*!< ECC Data DMA read write done*/
      __I uint32_t	ECC_DN     						:  1;   	/*!< ECC request output the address of ECC-error-bit*/
      __I uint32_t	ECC_ERROR     				:  1;   	/*!< Ecc engine error (Error bit more than 8 bit)*/
      __I uint32_t	Reserved     			 		:  18;   	/*!< */
    }SPI_ECC_STATUS_b;                    /*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_CTRL;						/*!< Offset:0x84 SPI ECC Control Register                         */	
    struct {
      __IO uint32_t	ECC_En     						:  1;   	/*!< ECC Enable*/
      __I uint32_t	Reserved0     				:  1;   	/*!< */
      __IO uint32_t	NAND_FLASH_SEL     		:  2;   	/*!< 00:512Byte , 01:2K , 10:4K*/
      __I uint32_t	Reserved1     				:  1;   	/*!< */
      __I uint32_t	Reserved2     				:  4;   	/*!< */
    }SPI_ECC_CTRL_b;                    	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_ADDR;						/*!< Offset:0x88 SPI ECC Address Register                         */	
    struct {
      __IO uint32_t	SPI_ECC_ADDR     			:  8;   	/*!< ECC error byte-address*/
      __IO uint32_t	SPI_ECC_BLK_CNT  			:  3;   	/*!< ECC error block cnt*/
      __I uint32_t	Reserved0     				:  21;   	/*!< */
    }SPI_ECC_ADDR_b;                    	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_DATA;						/*!< Offset:0x8C SPI ECC Data Register                         */	
    struct {
      __IO uint32_t	SPI_ECC_DATA     			:  16;   	/*!< ECC error data*/
      __I uint32_t	Reserved0     				:  16;   	/*!< */
    }SPI_ECC_DATA_b;                    	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_INFO1;					/*!< Offset:0x90 SPI ECC Page Info1 Register                         */	
    struct {
      __IO uint32_t	PG_WR_SEG_W     			:  4;   	/*!< Page sequence number for NF programming PAGE_INFO.*/
      __I uint32_t	Reserved0     				:  4;   	/*!< */
      __IO uint32_t	USER_DEF_W     				:  5;   	/*!< For user define area, only for DATA_CMD used.It supports write to NAND Flash.*/
      __I uint32_t	Reserved1     				:  3;   	/*!< */
      __I uint32_t	Reserved2     				:  16;   	/*!< */
    }SPI_ECC_INFO1_b;                    	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_INFO2;					/*!< Offset:0x94 SPI ECC Page Info2 Register                         */	
    struct {
      __IO uint32_t	LBA_W     						:  14;   	/*!< Logical Block Address LBA[13:0] for write to NAND Flash*/
      __I uint32_t	Reserved0     				:  2;   	/*!< */
      __I uint32_t	Reserved1     				:  16;   	/*!< */
    }SPI_ECC_INFO2_b;                    	/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_CRC;						/*!< Offset:0x98 SPI ECC CRC Register                         */	
    struct {
      __I uint32_t	SPI_ECC_CRC_OUT     	:  8;   	/*!< For Page INFO CRC*/
      __I uint32_t	Reserved0     				:  24;   	/*!< */
    }SPI_ECC_CRC_b;                    		/*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_INFO1_From_NF;	/*!< Offset:0x9C SPI ECC Page INFO1 From NAND Flash Register                        */	
    struct {
      __I uint32_t	PG_WR_SEQ     				:  4;   	/*!< Page sequence number PG_WR_SEQ[3:0] for read from NAND Flash*/
      __I uint32_t	Page_Empty     				:  1;   	/*!< 1: Empty 0: Not Empty*/
      __I uint32_t	Reserved0     				:  3;   	/*!< */
      __I uint32_t	USER_DEF	     				:  5;   	/*!< For user define area, only for DATA_CMD used.It supports read from NAND Flash.*/
      __I uint32_t	Reserved1     				:  3;   	/*!< */
      __I uint32_t	Reserved2     				:  16;   	/*!< */
    }SPI_ECC_INFO1_From_NF_b;             /*!< BitSize                                                                */
  };
	
  union {
    __IO uint32_t	SPI_ECC_INFO2_From_NF;	/*!< Offset:0xA0 SPI ECC Page INFO2 From NAND Flash Register                        */	
    struct {
      __I uint32_t	LBA		     						:  14;   	/*!< Logical Block Address LBA[13:0] for read from NAND Flash*/
      __I uint32_t	Reserved0     				:  2;   	/*!< */
      __I uint32_t	Reserved1     				:  16;   	/*!< */
    }SPI_ECC_INFO2_From_NF_b;             /*!< BitSize                                                                */
  };
	
}SN_SPI_DMA;


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


typedef struct {                                    /*!< SN_GPIO0 Structure                                                    */
  
  union {
    __IO uint32_t  DATA;                            /*!< Offset:0x00 GPIO Port n Data Register                                 */
    
    struct {
      __IO uint32_t  DATA0      :  1;               /*!< Data of Pn.0                                                          */
      __IO uint32_t  DATA1      :  1;               /*!< Data of Pn.1                                                          */
      __IO uint32_t  DATA2      :  1;               /*!< Data of Pn.2                                                          */
      __IO uint32_t  DATA3      :  1;               /*!< Data of Pn.3                                                          */
      __IO uint32_t  DATA4      :  1;               /*!< Data of Pn.4                                                          */
      __IO uint32_t  DATA5      :  1;               /*!< Data of Pn.5                                                          */
      __IO uint32_t  DATA6      :  1;               /*!< Data of Pn.6                                                          */
      __IO uint32_t  DATA7      :  1;               /*!< Data of Pn.7                                                          */
      __IO uint32_t  DATA8      :  1;               /*!< Data of Pn.8                                                          */
      __IO uint32_t  DATA9      :  1;               /*!< Data of Pn.9                                                          */
      __IO uint32_t  DATA10     :  1;               /*!< Data of Pn.10                                                         */
      __IO uint32_t  DATA11     :  1;               /*!< Data of Pn.11                                                         */
      __IO uint32_t  DATA12     :  1;               /*!< Data of Pn.12                                                         */
      __IO uint32_t  DATA13     :  1;               /*!< Data of Pn.13                                                         */
      __IO uint32_t  DATA14     :  1;               /*!< Data of Pn.14                                                         */
      __IO uint32_t  DATA15     :  1;               /*!< Data of Pn.15                                                         */
    } DATA_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  MODE;                            /*!< Offset:0x04 GPIO Port n Mode Register                                 */
    
    struct {
      __IO uint32_t  MODE0      :  1;               /*!< Mode of Pn.0                                                          */
      __IO uint32_t  MODE1      :  1;               /*!< Mode of Pn.1                                                          */
      __IO uint32_t  MODE2      :  1;               /*!< Mode of Pn.2                                                          */
      __IO uint32_t  MODE3      :  1;               /*!< Mode of Pn.3                                                          */
      __IO uint32_t  MODE4      :  1;               /*!< Mode of Pn.4                                                          */
      __IO uint32_t  MODE5      :  1;               /*!< Mode of Pn.5                                                          */
      __IO uint32_t  MODE6      :  1;               /*!< Mode of Pn.6                                                          */
      __IO uint32_t  MODE7      :  1;               /*!< Mode of Pn.7                                                          */
      __IO uint32_t  MODE8      :  1;               /*!< Mode of Pn.8                                                          */
      __IO uint32_t  MODE9      :  1;               /*!< Mode of Pn.9                                                          */
      __IO uint32_t  MODE10     :  1;               /*!< Mode of Pn.10                                                         */
      __IO uint32_t  MODE11     :  1;               /*!< Mode of Pn.11                                                         */
      __IO uint32_t  MODE12     :  1;               /*!< Mode of Pn.12                                                         */
      __IO uint32_t  MODE13     :  1;               /*!< Mode of Pn.13                                                         */
      __IO uint32_t  MODE14     :  1;               /*!< Mode of Pn.14                                                         */
      __IO uint32_t  MODE15     :  1;               /*!< Mode of Pn.15                                                         */
    } MODE_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  CFG;                             /*!< Offset:0x08 GPIO Port n Configuration Register                        */
    
    struct {
      __IO uint32_t  CFG0       :  2;               /*!< Configuration of Pn.0                                                 */
      __IO uint32_t  CFG1       :  2;               /*!< Configuration of Pn.1                                                 */
      __IO uint32_t  CFG2       :  2;               /*!< Configuration of Pn.2                                                 */
      __IO uint32_t  CFG3       :  2;               /*!< Configuration of Pn.3                                                 */
      __IO uint32_t  CFG4       :  2;               /*!< Configuration of Pn.4                                                 */
      __IO uint32_t  CFG5       :  2;               /*!< Configuration of Pn.5                                                 */
      __IO uint32_t  CFG6       :  2;               /*!< Configuration of Pn.6                                                 */
      __IO uint32_t  CFG7       :  2;               /*!< Configuration of Pn.7                                                 */
      __IO uint32_t  CFG8       :  2;               /*!< Configuration of Pn.8                                                 */
      __IO uint32_t  CFG9       :  2;               /*!< Configuration of Pn.9                                                 */
      __IO uint32_t  CFG10      :  2;               /*!< Configuration of Pn.10                                                */
      __IO uint32_t  CFG11      :  2;               /*!< Configuration of Pn.11                                                */
      __IO uint32_t  CFG12      :  2;               /*!< Configuration of Pn.12                                                */
      __IO uint32_t  CFG13      :  2;               /*!< Configuration of Pn.13                                                */
      __IO uint32_t  CFG14      :  2;               /*!< Configuration of Pn.14                                                */
      __IO uint32_t  CFG15      :  2;               /*!< Configuration of Pn.15                                                */
    } CFG_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IS;                              /*!< Offset:0x0C GPIO Port n Interrupt Sense Register                      */
    
    struct {
      __IO uint32_t  IS0        :  1;               /*!< Interrupt on Pn.0 is event or edge sensitive                          */
      __IO uint32_t  IS1        :  1;               /*!< Interrupt on Pn.1 is event or edge sensitive                          */
      __IO uint32_t  IS2        :  1;               /*!< Interrupt on Pn.2 is event or edge sensitive                          */
      __IO uint32_t  IS3        :  1;               /*!< Interrupt on Pn.3 is event or edge sensitive                          */
      __IO uint32_t  IS4        :  1;               /*!< Interrupt on Pn.4 is event or edge sensitive                          */
      __IO uint32_t  IS5        :  1;               /*!< Interrupt on Pn.5 is event or edge sensitive                          */
      __IO uint32_t  IS6        :  1;               /*!< Interrupt on Pn.6 is event or edge sensitive                          */
      __IO uint32_t  IS7        :  1;               /*!< Interrupt on Pn.7 is event or edge sensitive                          */
      __IO uint32_t  IS8        :  1;               /*!< Interrupt on Pn.8 is event or edge sensitive                          */
      __IO uint32_t  IS9        :  1;               /*!< Interrupt on Pn.9 is event or edge sensitive                          */
      __IO uint32_t  IS10       :  1;               /*!< Interrupt on Pn.10 is event or edge sensitive                         */
      __IO uint32_t  IS11       :  1;               /*!< Interrupt on Pn.11 is event or edge sensitive                         */
      __IO uint32_t  IS12       :  1;               /*!< Interrupt on Pn.12 is event or edge sensitive                         */
      __IO uint32_t  IS13       :  1;               /*!< Interrupt on Pn.13 is event or edge sensitive                         */
      __IO uint32_t  IS14       :  1;               /*!< Interrupt on Pn.14 is event or edge sensitive                         */
      __IO uint32_t  IS15       :  1;               /*!< Interrupt on Pn.15 is event or edge sensitive                         */
    } IS_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IBS;                             /*!< Offset:0x10 GPIO Port n Interrupt Both-edge Sense Register            */
    
    struct {
      __IO uint32_t  IBS0       :  1;               /*!< Interrupt on Pn.0 is triggered ob both edges                          */
      __IO uint32_t  IBS1       :  1;               /*!< Interrupt on Pn.1 is triggered ob both edges                          */
      __IO uint32_t  IBS2       :  1;               /*!< Interrupt on Pn.2 is triggered ob both edges                          */
      __IO uint32_t  IBS3       :  1;               /*!< Interrupt on Pn.3 is triggered ob both edges                          */
      __IO uint32_t  IBS4       :  1;               /*!< Interrupt on Pn.4 is triggered ob both edges                          */
      __IO uint32_t  IBS5       :  1;               /*!< Interrupt on Pn.5 is triggered ob both edges                          */
      __IO uint32_t  IBS6       :  1;               /*!< Interrupt on Pn.6 is triggered ob both edges                          */
      __IO uint32_t  IBS7       :  1;               /*!< Interrupt on Pn.7 is triggered ob both edges                          */
      __IO uint32_t  IBS8       :  1;               /*!< Interrupt on Pn.8 is triggered ob both edges                          */
      __IO uint32_t  IBS9       :  1;               /*!< Interrupt on Pn.9 is triggered ob both edges                          */
      __IO uint32_t  IBS10      :  1;               /*!< Interrupt on Pn.10 is triggered ob both edges                         */
      __IO uint32_t  IBS11      :  1;               /*!< Interrupt on Pn.11 is triggered ob both edges                         */
      __IO uint32_t  IBS12      :  1;               /*!< Interrupt on Pn.12 is triggered ob both edges                         */
      __IO uint32_t  IBS13      :  1;               /*!< Interrupt on Pn.13 is triggered ob both edges                         */
      __IO uint32_t  IBS14      :  1;               /*!< Interrupt on Pn.14 is triggered ob both edges                         */
      __IO uint32_t  IBS15      :  1;               /*!< Interrupt on Pn.15 is triggered ob both edges                         */
    } IBS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IEV;                             /*!< Offset:0x14 GPIO Port n Interrupt Event Register                      */
    
    struct {
      __IO uint32_t  IEV0       :  1;               /*!< Interrupt trigged evnet on Pn.0                                       */
      __IO uint32_t  IEV1       :  1;               /*!< Interrupt trigged evnet on Pn.1                                       */
      __IO uint32_t  IEV2       :  1;               /*!< Interrupt trigged evnet on Pn.2                                       */
      __IO uint32_t  IEV3       :  1;               /*!< Interrupt trigged evnet on Pn.3                                       */
      __IO uint32_t  IEV4       :  1;               /*!< Interrupt trigged evnet on Pn.4                                       */
      __IO uint32_t  IEV5       :  1;               /*!< Interrupt trigged evnet on Pn.5                                       */
      __IO uint32_t  IEV6       :  1;               /*!< Interrupt trigged evnet on Pn.6                                       */
      __IO uint32_t  IEV7       :  1;               /*!< Interrupt trigged evnet on Pn.7                                       */
      __IO uint32_t  IEV8       :  1;               /*!< Interrupt trigged evnet on Pn.8                                       */
      __IO uint32_t  IEV9       :  1;               /*!< Interrupt trigged evnet on Pn.9                                       */
      __IO uint32_t  IEV10      :  1;               /*!< Interrupt trigged evnet on Pn.10                                      */
      __IO uint32_t  IEV11      :  1;               /*!< Interrupt trigged evnet on Pn.11                                      */
      __IO uint32_t  IEV12      :  1;               /*!< Interrupt trigged evnet on Pn.12                                      */
      __IO uint32_t  IEV13      :  1;               /*!< Interrupt trigged evnet on Pn.13                                      */
      __IO uint32_t  IEV14      :  1;               /*!< Interrupt trigged evnet on Pn.14                                      */
      __IO uint32_t  IEV15      :  1;               /*!< Interrupt trigged evnet on Pn.15                                      */
    } IEV_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IE;                              /*!< Offset:0x18 GPIO Port n Interrupt Enable Register                     */
    
    struct {
      __IO uint32_t  IE0        :  1;               /*!< Interrupt on Pn.0 enable                                              */
      __IO uint32_t  IE1        :  1;               /*!< Interrupt on Pn.1 enable                                              */
      __IO uint32_t  IE2        :  1;               /*!< Interrupt on Pn.2 enable                                              */
      __IO uint32_t  IE3        :  1;               /*!< Interrupt on Pn.3 enable                                              */
      __IO uint32_t  IE4        :  1;               /*!< Interrupt on Pn.4 enable                                              */
      __IO uint32_t  IE5        :  1;               /*!< Interrupt on Pn.5 enable                                              */
      __IO uint32_t  IE6        :  1;               /*!< Interrupt on Pn.6 enable                                              */
      __IO uint32_t  IE7        :  1;               /*!< Interrupt on Pn.7 enable                                              */
      __IO uint32_t  IE8        :  1;               /*!< Interrupt on Pn.8 enable                                              */
      __IO uint32_t  IE9        :  1;               /*!< Interrupt on Pn.9 enable                                              */
      __IO uint32_t  IE10       :  1;               /*!< Interrupt on Pn.10 enable                                             */
      __IO uint32_t  IE11       :  1;               /*!< Interrupt on Pn.11 enable                                             */
      __IO uint32_t  IE12       :  1;               /*!< Interrupt on Pn.11 enable                                             */
      __IO uint32_t  IE13       :  1;               /*!< Interrupt on Pn.13 enable                                             */
      __IO uint32_t  IE14       :  1;               /*!< Interrupt on Pn.14 enable                                             */
      __IO uint32_t  IE15       :  1;               /*!< Interrupt on Pn.15 enable                                             */
    } IE_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  RIS;                             /*!< Offset:0x1C GPIO Port n Raw Interrupt Status Register                 */
    
    struct {
      __I  uint32_t  IF0        :  1;               /*!< Pn.0 raw interrupt flag                                               */
      __I  uint32_t  IF1        :  1;               /*!< Pn.1 raw interrupt flag                                               */
      __I  uint32_t  IF2        :  1;               /*!< Pn.2 raw interrupt flag                                               */
      __I  uint32_t  IF3        :  1;               /*!< Pn.3 raw interrupt flag                                               */
      __I  uint32_t  IF4        :  1;               /*!< Pn.4 raw interrupt flag                                               */
      __I  uint32_t  IF5        :  1;               /*!< Pn.5 raw interrupt flag                                               */
      __I  uint32_t  IF6        :  1;               /*!< Pn.6 raw interrupt flag                                               */
      __I  uint32_t  IF7        :  1;               /*!< Pn.7 raw interrupt flag                                               */
      __I  uint32_t  IF8        :  1;               /*!< Pn.8 raw interrupt flag                                               */
      __I  uint32_t  IF9        :  1;               /*!< Pn.9 raw interrupt flag                                               */
      __I  uint32_t  IF10       :  1;               /*!< Pn.10 raw interrupt flag                                              */
      __I  uint32_t  IF11       :  1;               /*!< Pn.11 raw interrupt flag                                              */
      __I  uint32_t  IF12       :  1;               /*!< Pn.12 raw interrupt flag                                              */
      __I  uint32_t  IF13       :  1;               /*!< Pn.13 raw interrupt flag                                              */
      __I  uint32_t  IF14       :  1;               /*!< Pn.14 raw interrupt flag                                              */
      __I  uint32_t  IF15       :  1;               /*!< Pn.15 raw interrupt flag                                              */
    } RIS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  IC;                              /*!< Offset:0x20 GPIO Port n Interrupt Clear Register                      */
    
    struct {
      __O  uint32_t  IC0        :  1;               /*!< Pn.0 interrupt flag clear                                             */
      __O  uint32_t  IC1        :  1;               /*!< Pn.1 interrupt flag clear                                             */
      __O  uint32_t  IC2        :  1;               /*!< Pn.2 interrupt flag clear                                             */
      __O  uint32_t  IC3        :  1;               /*!< Pn.3 interrupt flag clear                                             */
      __O  uint32_t  IC4        :  1;               /*!< Pn.4 interrupt flag clear                                             */
      __O  uint32_t  IC5        :  1;               /*!< Pn.5 interrupt flag clear                                             */
      __O  uint32_t  IC6        :  1;               /*!< Pn.6 interrupt flag clear                                             */
      __O  uint32_t  IC7        :  1;               /*!< Pn.7 interrupt flag clear                                             */
      __O  uint32_t  IC8        :  1;               /*!< Pn.8 interrupt flag clear                                             */
      __O  uint32_t  IC9        :  1;               /*!< Pn.9 interrupt flag clear                                             */
      __O  uint32_t  IC10       :  1;               /*!< Pn.10 interrupt flag clear                                            */
      __O  uint32_t  IC11       :  1;               /*!< Pn.11 interrupt flag clear                                            */
      __O  uint32_t  IC12       :  1;               /*!< Pn.12 interrupt flag clear                                            */
      __O  uint32_t  IC13       :  1;               /*!< Pn.13 interrupt flag clear                                            */
      __O  uint32_t  IC14       :  1;               /*!< Pn.14 interrupt flag clear                                            */
      __O  uint32_t  IC15       :  1;               /*!< Pn.15 interrupt flag clear                                            */
    } IC_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  BSET;                            /*!< Offset:0x24 GPIO Port n Bits Set Operation Register                   */
    
    struct {
      __O  uint32_t  BSET0      :  1;               /*!< Set Pn.0                                                              */
      __O  uint32_t  BSET1      :  1;               /*!< Set Pn.1                                                              */
      __O  uint32_t  BSET2      :  1;               /*!< Set Pn.2                                                              */
      __O  uint32_t  BSET3      :  1;               /*!< Set Pn.3                                                              */
      __O  uint32_t  BSET4      :  1;               /*!< Set Pn.4                                                              */
      __O  uint32_t  BSET5      :  1;               /*!< Set Pn.5                                                              */
      __O  uint32_t  BSET6      :  1;               /*!< Set Pn.6                                                              */
      __O  uint32_t  BSET7      :  1;               /*!< Set Pn.7                                                              */
      __O  uint32_t  BSET8      :  1;               /*!< Set Pn.8                                                              */
      __O  uint32_t  BSET9      :  1;               /*!< Set Pn.9                                                              */
      __O  uint32_t  BSET10     :  1;               /*!< Set Pn.10                                                             */
      __O  uint32_t  BSET11     :  1;               /*!< Set Pn.11                                                             */
      __O  uint32_t  BSET12     :  1;               /*!< Set Pn.12                                                             */
      __O  uint32_t  BSET13     :  1;               /*!< Set Pn.13                                                             */
      __O  uint32_t  BSET14     :  1;               /*!< Set Pn.14                                                             */
      __O  uint32_t  BSET15     :  1;               /*!< Set Pn.15                                                             */
    } BSET_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  BCLR;                            /*!< Offset:0x28 GPIO Port n Bits Clear Operation Register                 */
    
    struct {
      __O  uint32_t  BCLR0      :  1;               /*!< Clear Pn.0                                                            */
      __O  uint32_t  BCLR1      :  1;               /*!< Clear Pn.1                                                            */
      __O  uint32_t  BCLR2      :  1;               /*!< Clear Pn.2                                                            */
      __O  uint32_t  BCLR3      :  1;               /*!< Clear Pn.3                                                            */
      __O  uint32_t  BCLR4      :  1;               /*!< Clear Pn.4                                                            */
      __O  uint32_t  BCLR5      :  1;               /*!< Clear Pn.5                                                            */
      __O  uint32_t  BCLR6      :  1;               /*!< Clear Pn.6                                                            */
      __O  uint32_t  BCLR7      :  1;               /*!< Clear Pn.7                                                            */
      __O  uint32_t  BCLR8      :  1;               /*!< Clear Pn.8                                                            */
      __O  uint32_t  BCLR9      :  1;               /*!< Clear Pn.9                                                            */
      __O  uint32_t  BCLR10     :  1;               /*!< Clear Pn.10                                                           */
      __O  uint32_t  BCLR11     :  1;               /*!< Clear Pn.11                                                           */
      __O  uint32_t  BCLR12     :  1;               /*!< Clear Pn.12                                                           */
      __O  uint32_t  BCLR13     :  1;               /*!< Clear Pn.13                                                           */
      __O  uint32_t  BCLR14     :  1;               /*!< Clear Pn.14                                                           */
      __O  uint32_t  BCLR15     :  1;               /*!< Clear Pn.15                                                           */
    } BCLR_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  ODCTRL;                          /*!< Offset:0x2C GPIO Port n Open-drain Control Register                   */
    
    struct {
      __IO uint32_t  Pn0OC      :  1;               /*!< Pn.0 open-drain control                                               */
      __IO uint32_t  Pn1OC      :  1;               /*!< Pn.1 open-drain control                                               */
      __IO uint32_t  Pn2OC      :  1;               /*!< Pn.2 open-drain control                                               */
      __IO uint32_t  Pn3OC      :  1;               /*!< Pn.3 open-drain control                                               */
           uint32_t             :  8;
      __IO uint32_t  Pn12OC     :  1;               /*!< Pn.12 open-drain control                                              */
      __IO uint32_t  Pn13OC     :  1;               /*!< Pn.13 open-drain control                                              */
      __IO uint32_t  Pn14OC     :  1;               /*!< Pn.14 open-drain control                                              */
      __IO uint32_t  Pn15OC     :  1;               /*!< Pn.15 open-drain control                                              */
    } ODCTRL_b;                                     /*!< BitSize                                                               */
  };
} SN_GPIO0_Type;
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
