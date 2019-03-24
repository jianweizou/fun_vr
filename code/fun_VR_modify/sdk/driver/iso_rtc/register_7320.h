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

#include "snc_types.h"

#ifndef __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__
#define __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
    

    
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 * 7320 definition
 */
#define SN_ISO_BASE                     0x45000200UL
#define SN_PMU_BASE                     0x45000300UL
#define SN_PMU                          ((SN_PMU_Type           *) SN_PMU_BASE)
#define SN_ISO                          ((SN_ISO_Type           *) SN_ISO_BASE)

/* Backup register alias define */
typedef struct rtc_backup_alias
{
    uint8_t DLO_SYMBOL_L    __attribute__((aligned(4)));    /* BKP_00 register - used in ROM code. */
    uint8_t DLO_SYMBOL_H    __attribute__((aligned(4)));    /* BKP_01 register - used in ROM code. */
    uint8_t DLO_ADDR_0      __attribute__((aligned(4)));    /* BKP_02 register - used in ROM code. */
    uint8_t DLO_ADDR_1      __attribute__((aligned(4)));    /* BKP_03 register - used in ROM code. */
    uint8_t DLO_ADDR_2      __attribute__((aligned(4)));    /* BKP_04 register - used in ROM code. */
    uint8_t DLO_ADDR_3      __attribute__((aligned(4)));    /* BKP_05 register - used in ROM code. */
    uint8_t ALARM_SET_0     __attribute__((aligned(4)));    /* BKP_06 register - used in ROM code. */
    uint8_t ALARM_SET_1     __attribute__((aligned(4)));    /* BKP_07 register - used in ROM code. */
    uint8_t COUNTER_0       __attribute__((aligned(4)));    /* BKP_08 register - used in ROM code. */
    uint8_t COUNTER_1       __attribute__((aligned(4)));    /* BKP_09 register - used in ROM code. */
    
    uint32_t customize[3];  /* Reserved for user. */
    
    uint8_t ALARM_CA_0      __attribute__((aligned(4)));    /* BKP_12 register - used in ROM code. */
    uint8_t ALARM_CA_1      __attribute__((aligned(4)));    /* BKP_13 register - used in ROM code. */
    uint8_t ALARM_CA_2      __attribute__((aligned(4)));    /* BKP_14 register - used in ROM code. */
    uint8_t ALARM_CA_3      __attribute__((aligned(4)));    /* BKP_15 register - used in ROM code. */
    uint8_t CALENDAR_0      __attribute__((aligned(4)));    /* BKP_16 register - used by RTC_Driver. */
    uint8_t CALENDAR_1      __attribute__((aligned(4)));    /* BKP_17 register - used by RTC_Driver. */
    uint8_t CALENDAR_2      __attribute__((aligned(4)));    /* BKP_18 register - used by RTC_Driver. */
    uint8_t CALENDAR_3      __attribute__((aligned(4)));    /* BKP_19 register - used by RTC_Driver. */
    
    /* Addtional */
    
}rtc_backup_alias_t;

#define BASE_ISO_REG        0x45000208
#define BK_REG             ((rtc_backup_alias_t*) BASE_ISO_REG)    
    
//=============================================================================
//                  Macro Definition
//=============================================================================


//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  SN_PMU Structure
 */

/* =========================================================================================================================== */
/* ====================================                   SN_PMU                       ======================================= */
/* =========================================================================================================================== */
/**
  *@brief PMU Control Registers (SN_PMU)
*/

typedef struct {                                    /*!< SN_PMU Structure                                                   */

    union {
        __IO uint32_t  PMU_CTRL;                    /*!< Offset:0x00 PMU Control Register                                   */

        struct {
            __IO uint32_t  DPD_EN                   :  1;   /*!< DPDEN                                                      */
            __IO uint32_t  DSLEEP_EN                :  1;   /*!< DSLEEPEN                                                   */
            __IO uint32_t  SLEEP_EN                 :  1;   /*!< SLEEPEN                                                    */
            __IO uint32_t  RESERVED03               :  1;   /*!< RESERVED                                                   */ 
            __IO uint32_t  RTC_FLAG                 :  1;   /*!< RTC_Flag                                                   */
            __IO uint32_t  RESERVED05               :  1;   /*!< RESERVED                                                   */
            __IO uint32_t  WKP_FLAG                 :  1;   /*!< WKP_Flag                                                   */
            __IO uint32_t  CLR_WKP_INT              :  1;
            __IO uint32_t  RESERVED08               :  7;   /*!< RESERVED                                                   */
            __IO uint32_t  WKP                      :  1;   /*!< WKP PAD                                                    */
            __IO uint32_t  RESERVED16               : 15;   /*!< RESERVED                                                   */
            __IO uint32_t  RTC_CNT_FLAG             :  1;   /*!< RTC Counter Valid flag                                     */    
        } PMU_CTRL_b;
    };    
 
  __IO uint32_t  RTC_C;                             /*!< Offset:0x04 ISO RTC Counter value Register                         */
 
} SN_PMU_Type;


/* =========================================================================================================================== */
/* ====================================                 SN_ISOBlock                    ======================================= */
/* =========================================================================================================================== */
/**
  *@brief ISO Control Registers (SN_ISO)
*/

typedef struct {                                    /*!< SN_ISO Structure                                                   */
    union {
        __IO uint32_t  ISO_CTRL;                    /*!< Offset:0x00 ISO Control Register                                   */

        struct {        
            __IO uint32_t  RTC_Timeout              :  3;   /*!< RTC_Timeout                                                */
            __IO uint32_t  RTC_WKP                  :  1;   /*!< RTC WAKEUP                                                 */
            __IO uint32_t  LXTL_SpdRel              :  1;   /*!< LXTL Release speed up                                      */
            __IO uint32_t  LXTL_EN                  :  1;   /*!< LXTL EN                                                    */
            __IO uint32_t  RTC_SRC                  :  1;   /*!< RTC SRC SEL                                                */
            __IO uint32_t  RTC_EN                   :  1;   /*!< RTCEN                                                      */            
            __IO uint32_t  RESERVED8                : 24;   /*!< RESERVED                                                   */
        } ISO_CTRL_b;
    };
    union {
        __IO uint32_t  ISO_TRIM;                    /*!< Offset:0x04 RTC ILRC Trim register                                 */

        struct {        
            __IO uint32_t  Trim                     :  6;
            __IO uint32_t  RESERVED                 :  1;
            __IO uint32_t  RTC_Clear                :  1;
            __IO uint32_t  RESERVED0                :  16;
        } ISO_TRIM_b;
    };        

    __IO uint32_t  BK_REG0;                         /*!< Offset:0x08 Backup Register0                                       */
    __IO uint32_t  BK_REG1;                         /*!< Offset:0x0c Backup Register1                                       */
    __IO uint32_t  BK_REG2;                         /*!< Offset:0x10 Backup Register2                                       */
    __IO uint32_t  BK_REG3;                         /*!< Offset:0x14 Backup Register3                                       */
    __IO uint32_t  BK_REG4;                         /*!< Offset:0x18 Backup Register4                                       */
    __IO uint32_t  BK_REG5;                         /*!< Offset:0x1C Backup Register5                                       */
    __IO uint32_t  BK_REG6;                         /*!< Offset:0x20 Backup Register6                                       */
    __IO uint32_t  BK_REG7;                         /*!< Offset:0x24 Backup Register7                                       */
    __IO uint32_t  BK_REG8;                         /*!< Offset:0x28 Backup Register8                                       */
    __IO uint32_t  BK_REG9;                         /*!< Offset:0x2C Backup Register9                                       */
    __IO uint32_t  BK_REG10;                        /*!< Offset:0x30 Backup Register10                                      */
    __IO uint32_t  BK_REG11;                        /*!< Offset:0x34 Backup Register11                                      */
    __IO uint32_t  BK_REG12;                        /*!< Offset:0x38 Backup Register12                                      */
    __IO uint32_t  BK_REG13;                        /*!< Offset:0x3C Backup Register13                                      */
    __IO uint32_t  BK_REG14;                        /*!< Offset:0x40 Backup Register14                                      */
    __IO uint32_t  BK_REG15;                        /*!< Offset:0x44 Backup Register15                                      */    
    __IO uint32_t  BK_REG16;                        /*!< Offset:0x48 Backup Register16                                      */
    __IO uint32_t  BK_REG17;                        /*!< Offset:0x4C Backup Register17                                      */
    __IO uint32_t  BK_REG18;                        /*!< Offset:0x50 Backup Register18                                      */
    __IO uint32_t  BK_REG19;                        /*!< Offset:0x54 Backup Register19                                      */
    __IO uint32_t  BK_REG20;                        /*!< Offset:0x58 Backup Register20                                      */
}SN_ISO_Type;

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
