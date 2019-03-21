/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file ap_header.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/07/30
 * @license
 * @description
 */

#ifndef     __ap_header_h__
#define     __ap_header_h__

#include "snc_types.h"
#include "bat_overwrite.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/*      Public functions.                                                   */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*      Structure Definition                                                */
/* ------------------------------------------------------------------------ */

/* Info of each +RO sector -------------------  */
typedef struct{
    uint32_t AddrOffset;
    uint32_t DestAddr;
    uint32_t Size;
}stRomInfo_t;

/* Info of mode */
typedef struct{
    stRomInfo_t Rom[ROM_NUM_IN_PROJ1];
}stPrjModeRom_t;

typedef struct{
    uint32_t    Setting;
    uint32_t    Tag;
    uint32_t    RomNum;
    stRomInfo_t Rom[MAXIMUM_ROM_NUM]; 
}stProjectInfo_t;

#define AES_CIPHER_SIZE     (128 / 8)   /* 128 Bits = 64Byte */	

typedef struct{

    /* Basic Information */
    char        Mark[8];
    char        Version[8];
    uint32_t    Md5[4];

    /* Firmware infotmation with static address offset */
    uint32_t EnableAes;
    uint32_t Offset_AesInfo;
    uint32_t PrjNum;
    uint32_t Offset_Prj[PROJECT_NUM];

    /* Project Information, offset ->  4 * (11 + PROJECT_NUM) */
    stProjectInfo_t PrjInfo[PROJECT_NUM];

    /* Encrypt Information, offset ->  4 * (11 + PROJECT_NUM + PROJECT_NUM * (3 + MAXIMUM_ROM_NUM))) */
    uint32_t 	AesKey[AES_CIPHER_SIZE/4];
}stFwHeader_t;


#ifdef __cplusplus
}
#endif

#endif
