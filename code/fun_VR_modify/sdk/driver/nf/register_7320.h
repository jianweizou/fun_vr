
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
/* ====================================                   SN_NAND                      ======================================= */
/* =========================================================================================================================== */
/**
  * @brief NAND Flash Control Registers (SN_NF)
  */
#pragma anon_unions

typedef struct { 

    __IO uint32_t  NF_ADDRL;                        /*!< Offset:0x00 [15:0]                                                    */
    __IO uint32_t  NF_ADDRH;       			            /*!< Offset:0x04 [15:0]                                                    */
    __IO uint32_t  RESERVED2;        		            /*!< Offset:0x08                                                           */

  union {
    __IO uint32_t  NF_CMD;                          /*!< Offset:0x0C                                                           */
    
    struct {         
      __IO uint32_t  RESERVED5  	:  1; 
      __IO uint32_t  WR_CHK_SEL   :  2;
      __IO uint32_t  CMD_ADDR_SEL :  2; 
      __IO uint32_t  RESERVED1    :  3;  
      __IO uint32_t  NF_CMD_NIB   :  4;
    } NF_CMD_b;                                 
  };                                   
    __IO uint32_t  RESERVED0; 										  /*!< Offset:0x10                                                           */
  union {
    __IO uint32_t  NF_STAT;                         /*!< Offset:0x14                                                           */
    
    struct {                                               
			__IO  uint32_t  NF_CMD_STATUS  :  2;
      __I   uint32_t  RESERVED0   	 :  1;
      __IO  uint32_t  NF_ECC_FF   	 :  1;	    
			__IO  uint32_t  ECC_ERR 		   :  1;
      __IO  uint32_t  ECC_FAIL   	   :  1;	 
      __IO  uint32_t  PROG_ERR   	   :  1;   
      __IO  uint32_t  ERASE_ERR   	 :  1;	 
      __IO  uint32_t  CRC_ERR_CNT    :  2;   
      __IO  uint32_t  TIME_OUT_ERR   :  1;   
      __IO  uint32_t  NF_INFO_ERR    :  1; 
	    __IO  uint32_t  ECC_ERR_CNT    :  4;   
    } NF_STAT_b;                        
  };   
  union {
    __IO uint32_t  NF_CMD_BUF;                      /*!< Offset:0x18                                                           */
    
    struct {                                               
	    __IO  uint32_t  CMD_BUF_STATUS :  5;
      __IO  uint32_t  CMD_BUF_CLEAR  :  3; 	 
    } NF_CMD_BUF_b;                                 /*!< BitSize                                                               */
  };

  union {
    __IO uint32_t  NF_CTRL;                         /*!< Offset:0x1C                                                           */
    
    struct {                                               
	    __IO  uint32_t  NF_MODE 		 	 :  2;
	    __I   uint32_t  RESERVED0      :  2;
      __IO  uint32_t  MS_RDY     	   :  1; 
      __IO  uint32_t  MS_REG_RW      :  1;	    
			__IO  uint32_t  EXTRA_EN 		   :  1;
      __IO  uint32_t  ECC_EN   	     :  1;	 	 
    } NF_CTRL_b;                                    /*!< BitSize                                                               */
  }; 
    __IO uint32_t  NF_RW_LENGTH; 									  /*!< Offset:0x20                                                           */

  union {
    __IO uint32_t  NF_PAGE_INFO1;                   /*!< Offset:0x24                                                           */
    struct {                                               
			__IO  uint32_t  PG_WR_SEQ_W	   :  4;
			__IO  uint32_t  RESERVED0      :  4;
      __IO  uint32_t  USER_DEF_W  	 :  5;
    } NF_PAGE_INFO1_b;                              /*!< BitSize                                                               */
  };  

  union {
    __IO uint32_t  NF_PAGE_INFO2;                   /*!< Offset:0x28 */
    struct {                                               
			__IO  uint32_t  LBA_W	         :  14;
    } NF_PAGE_INFO2_b;                              /*!< BitSize                                                               */
  }; 

  union {
    __IO uint32_t  NF_INFO;                         /*!< Offset:0x2C */
    
    struct {                                               
			__IO  uint32_t  ADDR_CYCLE     :  2;
      __IO  uint32_t  PAGE_SIZE      :  2;
			__IO  uint32_t  BLOCK_SIZE     :  2;
    } NF_INFO_b;                                    /*!< BitSize                                                               */
	};

  union {
    __IO uint32_t  NF_ACC;                          /*!< Offset:0x30                                                           */
    
    struct {                                               
			__IO  uint32_t  RD_WIDTH       :  2;
      __IO  uint32_t  WR_WIDTH       :  2;
			__I   uint32_t  RESERVED       :  4;
			__O   uint32_t  INFO_CRC       :  8;
    } NF_ACC_b;                                     /*!< BitSize                                                               */
	};
 
    __IO uint32_t  RESERVED0C; 	                    /*!< Offset:0x34                                                           */
    __IO uint32_t  RESERVED0B; 	                    /*!< Offset:0x38                                                           */
    __IO uint32_t  RESERVED0A; 	                    /*!< Offset:0x3C                                                           */
    __IO uint32_t  RESERVED10; 	                    /*!< Offset:0x40                                                           */
    __IO uint32_t  RESERVED11; 	                    /*!< Offset:0x44                                                           */
    __IO uint32_t  TIME_CNT_L; 	                    /*!< Offset:0x48                                                           */
    __IO uint32_t  TIME_CNT_H; 	                    /*!< Offset:0x4C                                                           */
    __IO uint32_t  RESERVED4; 	                    /*!< Offset:0x50                                                           */
    __IO uint32_t  RESERVED5; 	                    /*!< Offset:0x54                                                           */
    __IO uint32_t  RESERVED6; 	                    /*!< Offset:0x58                                                           */
   
	union {
    __IO uint32_t  NF_PG_WR;                        /*!< Offset:0x5C                                                           */
    
    struct {                                               
			__O   uint32_t  PG_WR_SEQ      :  4;
      __O   uint32_t  PAGE_EMPTY     :  1;
			__IO  uint32_t  RESERVED       :  2;
			__O   uint32_t  USER_DEF       :  5;
    } NF_PG_WR_b;                                   /*!< BitSize                                                               */
	};
		
	  __O  uint32_t  NF_LBA;                          /*!< Offset:0x60                                                           */
    __IO uint32_t  RESERVED19;  									  /*!< Offset:0x64                                                           */
    __IO uint32_t  RESERVED1A;  									  /*!< Offset:0x68                                                           */
    __IO uint32_t  RESERVED1B;										  /*!< Offset:0x6C                                                           */
    __IO uint32_t  NF_REG1; 	            				  /*!< Offset:0x70                                                           */
    __IO uint32_t  NF_REG2; 	            				  /*!< Offset:0x74                                                           */
    __IO uint32_t  NF_REG3; 	              			  /*!< Offset:0x78                                                           */
    __IO uint32_t  NF_REG4; 	            				  /*!< Offset:0x7C                                                           */
    __IO uint32_t  RESERVED20;	  
    __IO uint32_t  RESERVED21; 	 
    __O uint32_t   NF_ECC_ADDR;               		  /*!< Offset:0x88                                                           */
    __O uint32_t   NF_ECC_MASK;               		  /*!< Offset:0x8C                                                           */

	union { 
    __O  uint32_t  NF_ECC_REQ;                      /*!< Offset:0x90                                                           */
    
    struct {                                               
			__O   uint32_t  ECC_DN         :  1;
      __O   uint32_t  ECC_REQ        :  1;
    } NF_ECC_REQ_b;                                 /*!< BitSize                                                               */		
	};
   
} SN_NF_Type;

///* =========================================================================================================================== */
///* ====================================                   SN_IDMA                      ======================================= */
///* =========================================================================================================================== */
///**
//  * @brief 
//  */

//typedef struct {                                    /*!< SN_IDMA Structure                                                     */
//  

//    __IO uint32_t  CH_RAM_ADDR;                     /*!< Offset:0x00 Ram address                                               */
//    
//    __IO uint32_t  CH_LEN;  
//	                      
//  union {
//    __IO uint32_t  CH_CTRL;                       	/*!< Offset:0x08 Comparator Capture Output Match register                  */
//    
//    struct {
//		__IO uint32_t  START              :  1;
//		__IO uint32_t  WR_RAM             :  1;
//		__IO uint32_t  PRAMM              :  1;
// 		__IO uint32_t  AD_PASS            :  1;
// 		__IO uint32_t  SCALAR             :  2;
//    } CH_CTRL_b;                                    /*!< BitSize                                                               */
//  };
//    __IO uint32_t  CH_DIP_ADDR;

//} SN_IDMA_Type;

#define SN_NF_BASE                  	  0x40023000UL
#define SN_NF                        	((SN_NF_Type              *) SN_NF_BASE)
//#define SN_IDMA_SDNF_BASE              0x40023110UL
//#define SN_IDMA_SDNF										((SN_IDMA_Type           *) SN_IDMA_SDNF_BASE) 
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
