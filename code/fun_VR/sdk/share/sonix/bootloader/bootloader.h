/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file bootloader.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef     __main_h__
#define     __main_h__


#include "snc_types.h"
#include "ap_header.h"
#include "bat_overwrite.h"
#include "flash_compatible_fun.h"


#ifdef __cplusplus
extern "C" {
#endif 

/* ------------------------------------------------------------------------ */
/*      Definitions                                                         */
/* ------------------------------------------------------------------------ */
#define FW_MARK             "SNC_APP_"          /* 0x5F434E53, 0x5F505041 */
        
    
#define UpdateErr_Mark              1
#define UpdateErr_VerifyData        2
#define UpdateErr_CopyHeader        3
#define UpdateErr_MD5Error          4
#define UpdateErr_InvaildSource     5
#define UpdateErr_FAT_ReadFail      6
#define UpdateErr_SizeNotMatch      7


/* === It is defined by FLASH Bit-map(from 0xFF ~ 0) ===*/
/* === DO NOT change the value ===*/
typedef enum{
    FACTORY_RESET       = 0x7FFFFFFF,
    NEW_FW_DOWNLOADING  = 0x7F7F7F7F,   /* Unstable status 	*/
    NEW_FW_ERROR        = 0x7F3F7F3F,   /* Occour Errors    */
    NEW_FW_READY        = 0x3F3F3F3F,
    UPDATING            = 0x1F1F1F1F,   /* Unstable status 	*/
    UPDATE_ERROR        = 0x1F171F17,   /* Occour Errors    */
    UPDATE_OK           = 0x17171717,
    COPY_FLASH_EXE_OK   = 0x13131313,
    IDLE                = 0x01010101,   
    UNKNOW              = 0,
    
}BootFlag;

/* Align with 4 Byte */
typedef struct{
	uint32_t	InitFlag;
	BootFlag 	BootFlag;
	uint8_t		isUpdateFw;
	uint8_t		isRecoverFw;
	uint8_t		isResetPriConfig;
	uint8_t		isResetCusConfig;
	uint32_t	FwAddress;
	uint32_t	FwLength;
	uint32_t 	ErrorCode;
}stBootInfo_t;    
    
/* ------------------------------------------------------------------------ */
/*      Public functions.                                                   */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*      Structure Definition                                                */
/* ------------------------------------------------------------------------ */

typedef struct 
{
    uint32_t load_table;
    uint32_t bootloader;
    uint32_t ap_default_header;
    uint32_t ap_update_header;
    uint32_t ap_execute_header;
    uint32_t boot_info;
    uint32_t private_config;
    
}address_map_t;
    

    
/* ------------------------------------------------------------------------ */
/*      Private functions.                                                  */
/* ------------------------------------------------------------------------ */
uint32_t Init_Bootloader(void);
uint32_t CheckKeyEvent(void);
uint32_t CheckPeripheralDev(void);
uint32_t CheckInternalEvent(void);
uint32_t BootEventHandle(void);
uint32_t DeviceModeSwitch(stFwHeader_t*);
void Reload_PRAM(stFwHeader_t *pHeader, stProjectInfo_t *pMode);
void usbd_rom_isp(void);
int Check_UpdateBuffer(void);
void Update_ViaInteralFlash(void);
stBootInfo_t* BootEvent_AddEvent(stBootInfo_t* ptr, BootFlag data);

/* update.c */
uint8_t UpdateFirmware(uint32_t DestHeaderAddr, uint32_t SourceHeaderAddr);

#ifdef __cplusplus
}
#endif

#endif
