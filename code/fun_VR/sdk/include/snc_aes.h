/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_aes.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef     __snc_aes_H_lc2SzdYy_laZn_Hwuc_stfw_uVpcGl1Nc6UQ__
#define     __snc_aes_H_lc2SzdYy_laZn_Hwuc_stfw_uVpcGl1Nc6UQ__

#include "snc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                  Constant Definition
//=============================================================================
    
typedef enum aes_ret
{
    AES_SUCCESS,
    AES_BUSY,
    
    AES_INVALID_ADDRESS     = -1,
    
}aes_ret_t;
    
typedef enum
{
    //AES
    AES_ENC             = 0,
    AES_DEC             ,
    //DES
    DES_ENC             = 0x4,
    DES_DEC             ,
    //3DES
    DES3_ENC            = 0x0C,
    DES3_DEC            ,
}aes_func_mode_t;

typedef enum
{
    AES_ECB             = 0,
    AES_CBC             ,
    AES_CFB             ,
    AES_OFB             ,      
}aes_op_mode_t;

typedef enum
{
    AES_BIT_128         = 0,
    AES_BIT_8           ,
    AES_BIT_1
}aes_bit_s_t;

typedef enum
{
    AES_KEY_128         = 0,
    AES_KEY_192         ,
    AES_KEY_256         ,
}aes_key_len_t;

    
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct aes_config
{
    aes_func_mode_t     function_mode;
    aes_op_mode_t       operation_mode;
    aes_bit_s_t         bit_per_unit;
    aes_key_len_t       key_length;
    uint32_t *          p_key_buf;
    uint32_t *          p_init_vec_buf;
    
}aes_config_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================


/**
 *  \brief: Start the aes prcedure
 *  
 *  \param [in] p AES       Setting structure defined @aes_config_t
 *  \param [in] p_src       Address of the encrypt/decrypt source data(SRAM ONLY).
 *  \param [in] p_des       Address of the encrypt/decrypt destination data(SRAM ONLY).
 *  \param [in] len         Length of the encrypt/decrypt data.
 *  
 *  \return                 Values @aes_ret_t
 */
aes_ret_t AES_Start(
                    aes_config_t *p, 
                    uint32_t *p_src, 
                    uint32_t *p_des, 
                    uint32_t len);
                    
/**
 *  \brief: Get AES IP status.
 *  
 *  \return                 0-AES encrypt/decrypt done,
 *                          1-Busy.
 */
uint32_t AES_GetStatus(void);
                    
/**
 *  \brief  version of the module
 *
 *  \return     version number
 *
 *  \details
 */
uint32_t AES_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
