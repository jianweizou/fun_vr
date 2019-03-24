/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_crc.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/05/14
 * @license
 * @description
 */

#ifndef __snc_crc_H_wP5u6Ynr_laZn_Hwuc_stfw_uVpcGl1Nc6UQ__
#define __snc_crc_H_wP5u6Ynr_laZn_Hwuc_stfw_uVpcGl1Nc6UQ__

#include <snc_types.h>

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
    
/* Default CRC polynomial standard. */
#define CRC_P_16        0xA001
#define CRC_P_DNP       0xA6BC
#define CRC_P_KERMIT    0x8408
#define CRC_P_T10DIF    0xEDD1
#define CRC_P_32        0xEDB88320L

typedef enum crc_poly
{
    CRC_POLY_16,
    CRC_POLY_16_MOD,
    CRC_POLY_DNP,
    CRC_POLY_KERMIT,
    CRC_POLY_T10DIF,
    CRC_POLY_32,
    
    CRC_POLY_NUM
}crc_poly_t;


//=============================================================================
//                  Macro Definition
//=============================================================================


//=============================================================================
//                  Structure Definition
//=============================================================================

typedef enum crc_ret
{
    CRC_RET_SUCCESS,
    CRC_RET_BUSY,
    CRC_RET_INVALID_LENGTH,
    CRC_RET_INVALID_MEMORY_SECTION,
    CRC_RET_INVALID_CYC_TYPE,
    
}crc_ret_t;

typedef struct crc_poly_info
{
  crc_poly_t    Tag;
  uint32_t      Poly; 
  uint32_t      Init;
  uint32_t      XOR;  
}crc_poly_info_t;


/* Default CRC polynomial standard. */
#define CRC_PREDEFINE_NUM   6
static const crc_poly_info_t crc_poly[CRC_PREDEFINE_NUM] = 
{
    {CRC_POLY_16     , CRC_P_16,     0, 0},
    {CRC_POLY_16_MOD , CRC_P_16,     1, 0},
    {CRC_POLY_DNP    , CRC_P_DNP,    0, 0},
    {CRC_POLY_KERMIT , CRC_P_KERMIT, 0, 0},
    {CRC_POLY_T10DIF , CRC_P_T10DIF, 0, 0},
    {CRC_POLY_32     , CRC_P_32,     1, 1}
};

//=============================================================================
//                  Private Function Definition
//=============================================================================


//=============================================================================
//                  Public Function Definition
//=============================================================================

/**
 *  \brief Start the crc calculation by given CRC type
 *  
 *  \param [in] crc_type:   list in @crc_poly_t
 *  \param [in] addr:       address of the crc data
 *  \param [in] len:        length of the crc data.
 *  
 *  \return                 constant in @crc_ret_t
 *  
 */
crc_ret_t CRC_Start(crc_poly_t crc_type, uint8_t *addr, uint32_t len);


/**
 *  \brief Start the byte checksum calculation.
 *  
 *  \param [in] addr:       address of the data
 *  \param [in] len:        length of the data.
 *  
 *  \return                 constant in @crc_ret_t
 *  
 */
crc_ret_t CRC_CheckSum_Start(uint8_t *addr, uint32_t len);


/**
 *  \brief Start the crc calculation by customer setting.
 *  
 *  \param [in] crc_setting @crc_poly_info_t, all items must be inplement.
 *  \param [in] addr        Description for addr
 *  \param [in] len         Description for len
 *  
 *  \return                 constant in @crc_ret_t
 *  
 */
crc_ret_t CRC_Customer_Poly_Start(crc_poly_info_t *p, uint8_t *addr, uint32_t len);


/**
 *  \brief Get crc status
 *  
 *  \return                 constant in @crc_ret_t
 */
crc_ret_t CRC_GetStatus(void);


/**
 *  \brief Get CRC Checksum
 *  
 *  \return                 CRC checksum.
 *  
 *  \details                This function will polling the crc status if the calculation is not finished yet.
 */
 
uint32_t CRC_Get_CheckSum(void);



/**
 *  \brief  version of the module
 *
 *  \return     version number
 *
 *  \details
 */
uint32_t CRC_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif
