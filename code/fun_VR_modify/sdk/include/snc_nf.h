/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_nf.h
 *
 * @author Enter Hung
 * @version 0.1
 * @date 2018/03/20
 * @license
 * @description
 */

#ifndef __snc_nf_H_wLW2Q5RY_l4pK_Hioz_sv3h_uoG353hjL9Bt__
#define __snc_nf_H_wLW2Q5RY_l4pK_Hioz_sv3h_uoG353hjL9Bt__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================	

typedef enum nf_ecc_toggle
{
	NF_ECC_DISABLE,
	NF_ECC_ENABLE
}nf_ecc_toggle_t;
typedef struct  {											
    uint32_t	Byte_cnt			:11;			//Byte	A0~A10		(2048byte), don't care A11
    uint32_t	Page_cnt			:6;				//Page	A12~A17		(64 page)
    uint32_t	Plane_cnt			:1;				//Plane	A18				(2 plane)
    uint32_t	Block_cnt			:11;			//Block	A19~A29		(2048 block)
    uint32_t	reserved0			:3;				
}NAND_address_strc;
typedef enum nf_err_idx
{
	NF_NO_ERR = 0,
	NF_RETRY_TIME_OUT = -1,
	NF_ECC_ERR = -2,
	NF_ECC_FAIL = -3,
	NF_PROGRAM_FAIL = -4,
	NF_ERASE_FAIL = -5,
	NF_CRC_ERR = -6,
	NF_RW_TIME_OUT = -7,
	NF_INFO_ERR = -8,
	NF_NOT_SUPPORT = -9
}nf_err_idx_t;

typedef enum nf_type_idx
{
	NF_SLC,
	NF_MLC,
	NF_TLC
}nf_type_idx_t;

typedef struct
{
	uint16_t id;
	uint16_t page_size;
	uint16_t pages_per_block;
	uint16_t address_cycle;
	uint16_t read_width;
	uint16_t write_width;
	uint16_t blocks_per_nf;
	uint16_t page_bit;
	uint16_t block_bit;
	uint16_t nf_type;
}NFTYPE;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

int NF_Init(NFTYPE *nf);
void NF_Deinit(void);
int NF_Page_Read(uint32_t *Addr, uint32_t data_size, uint16_t *data_buf, uint32_t ecc_enable);
int NF_Page_Write(uint32_t *Addr, uint32_t data_size, uint16_t *data_buf, uint32_t ecc_enable);
int NF_Block_Erase(uint32_t *Addr);
uint32_t NF_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
