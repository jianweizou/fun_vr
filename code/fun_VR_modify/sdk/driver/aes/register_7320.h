/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/30
 * @license
 * @description
 */

#ifndef __register_aes_H_
#define __register_aes_H_

#include "snc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================

#define SN_AES_BASE         0x40029000UL
#define SN_AES              ((SN_AES_Type           *) SN_AES_BASE)
    
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct {                                    /*!< SN_AES Structure                                                   */
  
    union {
        __IO uint32_t CTRL;                         /*!< Offset:0x00 AES Control Register                                   */
    
        struct {
            __IO uint32_t Enable                    :  1;   /*!< AES enable                                                 */
            __I  uint32_t Reserved_AES0             :  1;
            __IO uint32_t Key_GEN                   :  1;   /*!< AES generate key                                           */
            __I  uint32_t Reserved_AES1             :  1;
            __IO uint32_t Function_Mode             :  4;   /*!< AES function mode                                          */
            __IO uint32_t CRC_Read_Data             : 16;   /*!< CRC calculate data                                         */
            __IO uint32_t OP_Mode                   :  2;   /*!< AES/DES/3DES mode select                                   */
            __IO uint32_t Bit_S                     :  2;   /*!< AES/DES/3DES CFB/OFB mode bit number select                */
            __IO uint32_t Key_Len                   :  2;   /*!< AES key length select                                      */
            __IO uint32_t Update_IV                 :  1;   /*!< Update Initial vector control                              */
            __I  uint32_t Reserved_AES2             :  1;
        }CTRL_b;
    };
    
    __IO uint32_t BUSY;                             /*!< Offset:0x04 AES Busy Status Register                               */
    
    union {
        __IO uint32_t MASK;                         /*!< Offset:0x08 AES MASK Register                                      */
    
        struct {
            __IO uint32_t Func_Mask                 :  1;   /*!< AES function mask                                          */
            __IO uint32_t AHB_Master_Mask           :  1;   /*!< AES AHB master mask                                        */
            __I  uint32_t Reserved_AES3             : 30;
        } MASK_b;
    };
    
    union {
        __IO uint32_t STAT;                         /*!< Offset:0x0C AES Status Register                                    */
    
        struct {
            __IO uint32_t Func_STAT                 :  1;   /*!< AES function status                                        */
            __IO uint32_t AHB_Master_STAT           :  1;   /*!< AES AHB master status                                      */
            __IO uint32_t AHB_Slave_STAT            :  1;   /*!< AES AHB slave status                                       */ 
            __IO uint32_t CLR_Func_STAT             :  1;   /*!< Write 1 to clear FUN_STATUS                                */
            __IO uint32_t CLR_AHB_Master_STAT       :  1;   /*!< Write 1 to clear AHB_MASTER_STATUS                         */
            __I  uint32_t Reserved_AES4             : 27;
        } STAT_b;
    };
    
    union {
        __IO uint32_t SRC_ADDR;                     /*!< Offset:0x10 Source Address Register                                */
    
        struct {
            __I  uint32_t Reserved_AES5             :  4;
            __IO uint32_t Src_Addr                  : 28;   /*!< Memory start address of source data                        */
        } SRC_ADDR_b;
    };
    
    union {
        __IO uint32_t DEST_ADDR;                    /*!< Offset:0x14 Destination Address Register                           */
    
        struct {
            __I  uint32_t Reserved_AES6             :  4;
            __IO uint32_t Dest_Addr                 : 28;   /*!< Memory start address of destination data                   */
        } DEST_ADDR_b;
    };
    
    union {
        __IO uint32_t DATA_SIZE;                    /*!< Offset:0x18 Data Size (Byte) Register                                     */
        struct {
        __I  uint32_t Reserved_AES7                 :  4;	
        __IO uint32_t WORD_SIZE                     : 21;   /*                                                              */
        __I  uint32_t Reserved_AES8                 :  7;
    } DATA_SIZE_b;                                 /*!< BitSize                                                             */
    };
    
    __IO uint32_t KEY_IN_0;                         /*!< Offset:0x1C AES key input[31:0] or DES key in1[31:0] or CRC data input(REG mode)*/
    __IO uint32_t KEY_IN_1;                         /*!< Offset:0x20 AES key input[63:32] or DES key in1[63:32] or KEY_IN_1[0] is ready signal(REG mode)*/
    __IO uint32_t KEY_IN_2;                         /*!< Offset:0x24 AES key input[95:64] or DES key in2[31:0]              */
    __IO uint32_t KEY_IN_3;                         /*!< Offset:0x28 AES key input[127:96] or DES key in2[63:32]            */
    __I  uint32_t KEY_OUT_0;                        /*!< Offset:0x2C AES key output[31:0]                                   */
    __I  uint32_t KEY_OUT_1;                        /*!< Offset:0x30 AES key output[63:32]                                  */
    __I  uint32_t KEY_OUT_2;                        /*!< Offset:0x34 AES key output[95:64]                                  */
    __I  uint32_t KEY_OUT_3;                        /*!< Offset:0x38 AES key output[127:96]                                 */
    __I  uint32_t IP_VERSION;                       /*!< Offset:0x3C Version of AES codec                                   */
    __IO uint32_t KEY_IN_4;                         /*!< Offset:0x40 DES key in3[31:0]                                      */
    __IO uint32_t KEY_IN_5;                         /*!< Offset:0x44 DES key in3[63:32]                                     */
    __I  uint32_t Reserved_AES1[2];
    __IO uint32_t IV0_IN;                           /*!< Offset:0x50 Initial vector 0(for AES/DES/3DES)                     */
    __IO uint32_t IV1_IN;                           /*!< Offset:0x54 Initial vector 1(for AES/DES/3DES)                     */
    __IO uint32_t IV2_IN;                           /*!< Offset:0x58 Initial vector 2(for AES only)                         */
    __IO uint32_t IV3_IN;                           /*!< Offset:0x5C Initial vector 3(for AES only)                         */
    __I  uint32_t Reserved_AES2[8];  
    __IO uint32_t KEY_IN_6;                         /*!< Offset:0x80 Input key 6 (for AES 256-bit mode)                     */
    __IO uint32_t KEY_IN_7;                         /*!< Offset:0x84 Input key 7 (for AES 256-bit mode)                     */
    __I  uint32_t KEY_OUT_4;                        /*!< Offset:0x88 Output key 4 (AES output key)                          */
    __I  uint32_t KEY_OUT_5;                        /*!< Offset:0x8C Output key 5 (AES output key)                          */
    __I  uint32_t KEY_OUT_6;                        /*!< Offset:0x90 Output key 6 (AES output key)                          */
    __I  uint32_t KEY_OUT_7;                        /*!< Offset:0x94 Output key 7 (AES output key)                          */  
}SN_AES_Type;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
