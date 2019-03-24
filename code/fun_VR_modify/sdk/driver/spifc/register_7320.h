/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */

#ifndef __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__
#define __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 * 7320 definition
 */

#define SN_SPIFC_BASE       0x40022000UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  FLASH Memory (NOR) Control Registers (SN_FLASH)
 */
typedef struct sn_flash
{
    union {
        __IO uint32_t  FLASH_CTRL;     /*!< Offset:0x00 Comparator Capture Output Match register */

        struct {
            __IO uint32_t  PP        :  1;
            __IO uint32_t  BP        :  1;
            __IO uint32_t  WP4       :  1;
            __IO uint32_t  WP        :  1;
            __IO uint32_t  READ4     :  1;
            __IO uint32_t  READ2     :  1;
            __IO uint32_t  FAST_READ :  1;
            __IO uint32_t  READ      :  1;
            __IO uint32_t  DE_CYCLE  :  4;
            __IO uint32_t  CLK_DIV   :  2;
            __IO uint32_t  QUAD      :  1;
            __IO uint32_t  EN        :  1;
            __I  uint32_t  RSEREVED  :  16;
        } FLASH_CTRL_b;
    };

    union {
        __IO uint32_t  FLASH_CMD;     /*!< Offset:0x04 Comparator Capture Output Match register*/

        struct {
            __I  uint32_t  RESERVED0 :  1;
            __I  uint32_t  WIP       :  1;
            __IO uint32_t  DT        :  2;
            __IO uint32_t  CUSTOMER  :  1;
            __IO uint32_t  C_ADDR    :  1;
            __I  uint32_t  RESERVED6 :  1;
            __IO uint32_t  C_READ    :  1;
            __IO uint32_t  CE        :  1;
            __IO uint32_t  BE        :  1;
            __IO uint32_t  SE        :  1;
            __IO uint32_t  SR_W      :  1;
            __IO uint32_t  RDP       :  1;
            __IO uint32_t  DP        :  1;
            __IO uint32_t  SR_R      :  1;
            __IO uint32_t  ID_R      :  1;
            __I  uint32_t  RSEREVED  :  16;
        } FLASH_CMD_b;
    };

    union {
        __IO uint32_t  FLASH_ADDRH;   /*!< Offset:0x08 Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  ADDR     :  7;
            __I   uint32_t  RESERVED :  25;
        } FLASH_ADDRH_b;
    };

    union {
        __IO uint32_t  FLASH_ADDRL;   /*!< Offset:0x0C Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  ADDR     :  16;
            __I   uint32_t  RESERVED :  16;     /*!< Checksum calculation choosen*/
        } FLASH_ADDRL_b;      /*!< BitSize   */
    };

    union {
        __IO uint32_t  FLASH_CMD_DATA;/*!< Offset:0x10 Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  DATA     :  8;
            __I   uint32_t  RESERVED :  24;     /*!< Checksum calculation choosen*/
        } FLASH_CMD_DATA_b;   /*!< BitSize   */
    };

    union {
        __IO uint32_t  FLASH_STAT;    /*!< Offset:0x14 Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  DATA     :  8;
            __I   uint32_t  RESERVED :  24;     /*!< Checksum calculation choosen*/
        } FLASH_STAT_b;       /*!< BitSize   */
    };

    union {
        __IO uint32_t  FLASH_ID;      /*!< Offset:0x18 Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  DATA     :  16;
            __I   uint32_t  RESERVED :  16;     /*!< Checksum calculation choosen*/
        } FLASH_ID_b; /*!< BitSize   */
    };

    union {
        __IO uint32_t  FLASH_RDATA;   /*!< Offset:0x1C Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  DATA     :  16;
            __I   uint32_t  RESERVED :  16;     /*!< Checksum calculation choosen*/
        } FLASH_RDATA_b;      /*!< BitSize   */
    };
    union {
        __IO uint32_t  FLASH_WDATA;   /*!< Offset:0x20 Comparator Capture Output Match register*/

        struct {
            __IO  uint32_t  DATA     :  16;
            __I   uint32_t  RESERVED :  16;     /*!< Checksum calculation choosen*/
        } FLASH_WDATA_b;      /*!< BitSize   */
    };
    union {
        __IO uint32_t  FLASH_MODE;   /*!< Offset:0x24 Select flash mode register*/

        struct {
            __I  uint32_t BYTE4_STATUS :  1;
            __IO uint32_t EX4B         :  1;
            __IO uint32_t EN4B         :  1;
            __I  uint32_t RESERVED     :  29;     /*!< Select 3-byte / 4-byte Address Mode*/
        } FLASH_MODE_b;
    };

} sn_flash_t;
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
