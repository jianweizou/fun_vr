/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file crc.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/30
 * @license
 * @description
 */


#include "register_7320.h"
#include "snc_types.h"
#include "snc_crc.h"


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CRC_VERSION     0x73200000
    
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

crc_ret_t CRC_Start(crc_poly_t crc_type, uint8_t *addr, uint32_t len)
//uint32_t CRC_Start(uint32_t *addr, uint32_t len, uint32_t init, uint32_t XOR, uint32_t Polynomial)
{
    crc_ret_t ret        = CRC_RET_SUCCESS;
    uint32_t init       = 0;
    uint32_t x_or       = 0;
    uint32_t poly       = 0;
    uint8_t order;
    
    do{
        /* Check CRC length */
        if(len < 4)
        {
            ret = CRC_RET_INVALID_LENGTH;
        }
        
        /* Check CRC memory region */
        if( ((uint32_t)addr < 0x18000000) || ((uint32_t)addr + len > 0x18040000) )
        {
            ret = CRC_RET_INVALID_MEMORY_SECTION;
            break;
        }
            
        /* Check crc_type */
        if(crc_type >= CRC_POLY_NUM)
        {
            ret = CRC_RET_INVALID_CYC_TYPE;
            break;
        }
        
        for(int i=0; i<CRC_PREDEFINE_NUM; i++)
        {
            if (crc_type == crc_poly[i].Tag)
            {
                poly = crc_poly[i].Poly;
                init = crc_poly[i].Init;
                x_or = crc_poly[i].XOR;
            }
        }
        
        if(poly == 0)
        {
            ret = CRC_RET_INVALID_CYC_TYPE;
            break;
        }
        
        /* Check if CRC Ready */ 
        if(SN_CRC->CRC_CTRL_b.CRC_EN_RDY != 1)
        {
            ret = CRC_RET_BUSY;
            break;
        }            

        /* Caulculate Polynomial Order. */
        uint32_t temp = poly;
        for(order=0; temp; order++){
            temp = temp >> 1;
        }

        SN_CRC->CRC_CTRL_b.CRC_EN   = 0;          /* Need to Turn it off first. */

        SN_CRC->CRC_CTRL_b.Mode     = 0;          /* 0: CRC, 1: Check sum */
        SN_CRC->CRC_SET_b.Init      = init;       /* 0: 0x0, 1:0xFFFF FFFF */
        SN_CRC->CRC_SET_b.XOR       = x_or;       /* Output XOR value 0: 0x0, 1:0xFFFF FFFF */
        SN_CRC->CRC_POLY            = poly;       /* CRC Polynomial. */

        SN_CRC->CRC_SET_b.Order     = order - 1;  /* CRC Order = actual order - 1 */

        SN_CRC->CRC_ADDR            = (uint32_t)addr;
        
        SN_CRC->R_LINE_SIZE         = len;
        SN_CRC->R_LINE_NUM          = 1;
        SN_CRC->GAP                 = 1;

        SN_CRC->CRC_CTRL_b.CRC_EN = 1;
        
    }while(0);
    
    return ret;
}

crc_ret_t CRC_CheckSum_Start(uint8_t *addr, uint32_t len)
{

    /* Switch to checksum mode */
    SN_CRC->CRC_CTRL_b.Mode = 1;
    
    SN_CRC->CRC_CTRL_b.CRC_EN   = 0;          /* Need to Turn it off first. */
    
    /* Check CRC memory region */
    if( ((uint32_t)addr < 0x18000000) || ((uint32_t)addr + len > 0x18040000) )
    {
        return CRC_RET_INVALID_MEMORY_SECTION;
    }
    
    SN_CRC->CRC_ADDR            = (uint32_t)addr;
    
    SN_CRC->R_LINE_SIZE         = len;
    SN_CRC->R_LINE_NUM          = 1;
    SN_CRC->GAP                 = 1;
    
    SN_CRC->CRC_CTRL_b.CRC_EN = 1;
    
    return CRC_RET_SUCCESS;
}



crc_ret_t CRC_Customer_Poly_Start( crc_poly_info_t *p, uint8_t *addr, uint32_t len)
{
    crc_ret_t ret       = CRC_RET_SUCCESS;
    uint32_t init       = p->Init;
    uint32_t x_or       = p->XOR;
    uint32_t poly       = p->Poly;
    uint8_t order;
    
    do{
        /* Check CRC length */
        if(len < 4)
        {
            ret = CRC_RET_INVALID_LENGTH;
        }
        
        /* Check CRC memory region */
        if( ((uint32_t)addr < 0x18000000) || ((uint32_t)addr + len > 0x18040000) )
        {
            ret = CRC_RET_INVALID_MEMORY_SECTION;
            break;
        }

        
        
        /* Check if CRC Ready */ 
        if(SN_CRC->CRC_CTRL_b.CRC_EN_RDY != 1)
        {
            ret = CRC_RET_BUSY;
            break;
        }            

        /* Caulculate Polynomial Order. */
        uint32_t temp = poly;
        for(order=0; temp; order++){
            temp = temp >> 1;
        }

        SN_CRC->CRC_CTRL_b.CRC_EN   = 0;          /* Need to Turn it off first. */

        SN_CRC->CRC_CTRL_b.Mode     = 0;          /* 0: CRC, 1: Check sum */
        SN_CRC->CRC_SET_b.Init      = init;       /* 0: 0x0, 1:0xFFFF FFFF */
        SN_CRC->CRC_SET_b.XOR       = x_or;       /* Output XOR value 0: 0x0, 1:0xFFFF FFFF */
        SN_CRC->CRC_POLY            = poly;       /* CRC Polynomial. */

        SN_CRC->CRC_SET_b.Order     = order - 1;  /* CRC Order = actual order - 1 */

        SN_CRC->CRC_ADDR            = (uint32_t)addr;
        
        SN_CRC->R_LINE_SIZE         = len;
        SN_CRC->R_LINE_NUM          = 1;
        SN_CRC->GAP                 = 1;

        SN_CRC->CRC_CTRL_b.CRC_EN = 1;

    }while(0);
    
    return ret;
}


crc_ret_t CRC_GetStatus(void)
{
    if(SN_CRC->CRC_CTRL_b.CRC_EN_RDY && SN_CRC->CRC_CTRL_b.CRC_OK)
    {
        return CRC_RET_SUCCESS;
    }
    else
    {
        return CRC_RET_BUSY;
    }
}

    
uint32_t CRC_Get_CheckSum(void)
{
    while(! (SN_CRC->CRC_CTRL_b.CRC_EN_RDY && SN_CRC->CRC_CTRL_b.CRC_OK) );
    
    return SN_CRC->CHECKSUM;
}


uint32_t CRC_GetVersion(void)
{
    return CRC_VERSION;
}
#ifdef __cplusplus
}
#endif
