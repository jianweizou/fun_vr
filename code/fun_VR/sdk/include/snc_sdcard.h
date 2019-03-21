/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_sdcard.h
 *
 * @author Enter Hung
 * @version 0.1
 * @date 2018/03/20
 * @license
 * @description
 */

#ifndef __snc_sdcard_H_wLW2Q5RY_l4pK_Hioz_sv3h_uoG353hjL9Bt__
#define __snc_sdcard_H_wLW2Q5RY_l4pK_Hioz_sv3h_uoG353hjL9Bt__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================


typedef enum sdc_mode_idx
{
	SDC_WRITE_MODE,
	SDC_READ_MODE
}sdc_mode_idx_t;

typedef enum sd_err_idx
{
	SDC_NO_ERR = 0,
	SDC_RETRY_TIMEOUT = -1,
	SDC_CRC_ERR = -2,
	SDC_WRITE_ERR = -3,
	SDC_RW_TIMEOUT = -4,
	SDC_CMD_ERR = -5,
	SDC_NO_SUPPLY_CARD = -6,
	SDC_BUSY = -7,
	SDC_FUNC_PARAMETER_ERR = -8
}sd_err_idx_t;

typedef enum sd_bus_width
{
	SD_BUS_WIDTH_1BIT,
	SD_BUS_WIDTH_4BIT
}sd_bus_width_t;

typedef enum sd_card_event
{
	SD_CARD_INSERTED,
	SD_CARD_REMOVED
}sd_card_event_t;

typedef enum sd_card_port
{
	SD_CARD_PORT_0,
	SD_CARD_PORT_1
}sd_card_port_t;

typedef enum sd_block_type
{
	HEAD,
	MIDDLE,
	TAIL
}sd_block_type_t;

typedef enum sd_card_type_idx
{
	SD_CARD,
	MMC_CARD
}sd_card_type_idx_t;

typedef enum sd_card_version_idx
{
	V1X_SDSC,
	V20_SDSC,
	V20_SDHC
}sd_card_version_idx_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int SDC_Init(uint8_t sd_port);
void SDC_Deinit(uint8_t sd_port);
int SDC_Erase(uint8_t sd_port, uint32_t start_lba, uint32_t end_lba);
void SDC_Get_CSD(uint8_t sd_port, uint32_t *csd);
void SDC_Get_CID(uint8_t sd_port, uint32_t *cid);
void SDC_Get_Card_Type(uint8_t sd_port, uint8_t *card_type);
void SDC_Get_OCR(uint8_t sd_port, uint8_t *ocr);
void SDC_Get_Capacity(uint8_t sd_port, uint64_t *capacity);
void SDC_Get_Clock(uint8_t sd_port, uint32_t sys_clock, uint32_t *sd_clock);
int SDC_Write_Block_Blocking(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num);
int SDC_Read_Block_Blocking(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num);
int SDC_Write_Block(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num);
int SDC_Read_Block(uint8_t sd_port, uint32_t ram_addr, uint32_t lba, uint32_t block_num);
int SDC_RW_Polling(uint8_t sd_port);
uint64_t SDC_Get_Total_Block(void);
uint32_t SDC_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
