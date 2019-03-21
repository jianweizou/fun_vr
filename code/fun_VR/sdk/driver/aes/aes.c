/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */

/** @file aes.c 
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/30
 * @license
 * @description
 */


#include    "snc_aes.h"
#include    "register_7320.h"

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
#define AES_VERSION        0x73200000

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


aes_ret_t AES_Start(
                    aes_config_t *p, 
                    uint32_t *p_src, 
                    uint32_t *p_des, 
                    uint32_t len)
{
    
    /* Check sram address */
    if(((uint32_t)p_src + len >= 0x18040000) || ((uint32_t)p_src < 0x18000000))
        return AES_INVALID_ADDRESS;
    
    if(((uint32_t)p_des + len >= 0x18040000) || ((uint32_t)p_des < 0x18000000))
        return AES_INVALID_ADDRESS;
    
    SN_AES->CTRL_b.Key_Len = p->key_length;
    
    /* Assign IN KEY */
    SN_AES->KEY_IN_0            = *(p->p_key_buf + 0);
    SN_AES->KEY_IN_1            = *(p->p_key_buf + 1);
    SN_AES->KEY_IN_2            = *(p->p_key_buf + 2);
    SN_AES->KEY_IN_3            = *(p->p_key_buf + 3);
    
    if(( p->key_length >= AES_KEY_192 ))
    {
        SN_AES->KEY_IN_4        = *(p->p_key_buf + 4);
        SN_AES->KEY_IN_5        = *(p->p_key_buf + 5);
        
        if( p->key_length >= AES_KEY_256 )
        {
            SN_AES->KEY_IN_6    = *(p->p_key_buf + 6);
            SN_AES->KEY_IN_7    = *(p->p_key_buf + 7);
        }
    }
    

    if(  (p->operation_mode > AES_ECB) )
    {
        SN_AES->IV0_IN              = *(p->p_init_vec_buf + 0);
        SN_AES->IV1_IN              = *(p->p_init_vec_buf + 1);
        SN_AES->IV2_IN              = *(p->p_init_vec_buf + 2);
        SN_AES->IV3_IN              = *(p->p_init_vec_buf + 3);
    }
    
    #if 1
    /* Rolling Key function, it makes different key between encrypt and decrypt. */
    
    /* Decrypt check & op mode check */    
    if( (p->function_mode & 1) && (p->operation_mode < AES_CFB) )
    {
        SN_AES->CTRL_b.Key_GEN = 1;
        while(SN_AES->CTRL_b.Key_GEN);
        SN_AES->KEY_IN_0        = SN_AES->KEY_OUT_0;
        SN_AES->KEY_IN_1        = SN_AES->KEY_OUT_1;
        SN_AES->KEY_IN_2        = SN_AES->KEY_OUT_2;
        SN_AES->KEY_IN_3        = SN_AES->KEY_OUT_3;
        if(( p->key_length >= AES_KEY_192 ))
        {
         SN_AES->KEY_IN_4       = SN_AES->KEY_OUT_4;
         SN_AES->KEY_IN_5       = SN_AES->KEY_OUT_5;
            if( p->key_length >= AES_KEY_256 )
            {
                SN_AES->KEY_IN_6 = SN_AES->KEY_OUT_6;
                SN_AES->KEY_IN_7 = SN_AES->KEY_OUT_7;
            }
        }
    }
    #endif
    
    SN_AES->SRC_ADDR                = (uint32_t)p_src;
    SN_AES->DEST_ADDR               = (uint32_t)p_des;
    SN_AES->DATA_SIZE               = len - 0x10;
    
    SN_AES->CTRL_b.Function_Mode    = p->function_mode;
    SN_AES->CTRL_b.OP_Mode          = p->operation_mode;
    if(p->operation_mode > AES_CFB)
    {
        SN_AES->CTRL_b.Bit_S        = p->bit_per_unit;
    }
    
    SN_AES->CTRL_b.Update_IV        = 1;
    SN_AES->CTRL_b.Enable           = 1;

    return AES_SUCCESS;
}


uint32_t AES_GetStatus()
{
    if(SN_AES->BUSY)
        return AES_BUSY;
    else
        return AES_SUCCESS;
}


uint32_t AES_GetVersion(void)
{
    return AES_VERSION;
}

#ifdef __cplusplus
}
#endif

