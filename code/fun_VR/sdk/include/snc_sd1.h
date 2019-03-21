/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_sd1.h
 *
 * @author Vicky Tseng
 * @version 0.1
 * @date 2018/09/05
 * @license
 * @description
 */

#ifndef __snc_sd1_h__
#define __snc_sd1_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 * an enum for function return status
 */
typedef enum
{
	MID_SD_QUEUE_FINISH = 0,				/**< cmd has finished */
	MID_SD_QUEUE_PROCESSING,	
	MID_SD_QUEUE_ACCESS,						/**< cmd sends to queue and wait for finishing(only in block mode) */
	MID_SD_QUEUE_CARD_REMOVED,			/**< cmd executes failed because of SD card is removed */
	MID_SD_QUEUE_CARD_WRITEPROTECT,	/**< cmd executes failed because of SD card is write protected */
	MID_SD_QUEUE_FAIL								/**< cmd executes failed */
}mid_sd_queue_status_t;

/**
 * an enum for function is block or not
 */
typedef enum
{
	MID_SD_NONBLOCK = 0,						/**< function nonblock */
	MID_SD_BLOCK										/**< function block until finish */
}mid_sd_block_t;

//=============================================================================
//                  Macro Definition
//=============================================================================
#define	SD_RTN_PASS								1

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
int 		mid_sd_init(void);
int  		mid_sd_identify(uint8_t is_block, mid_sd_queue_status_t *status);                //same as mount
int  		mid_sd_write(uint32_t dst_addr, void *src_addr, uint32_t size, uint8_t is_block, mid_sd_queue_status_t *status);
int  		mid_sd_read(void *dst_addr, uint32_t src_addr, uint32_t size, uint8_t is_block, mid_sd_queue_status_t *status);
int 		mid_sd_get_capacity(uint64_t *size, uint8_t is_block, mid_sd_queue_status_t *status);
int 		mid_sd_sync(void);
struct 	sd_info* mid_sd_get_driver_info(void);
int 		mid_sd_card_detect(void);
int 		mid_sd_write_protect(void);
int 		mid_sd_rwtest(void);
int 		mid_sd_get_card_status(uint8_t is_block, mid_sd_queue_status_t *status);
#ifdef __cplusplus
}
#endif

#endif
