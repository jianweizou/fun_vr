/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/15
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
#define SN_PMU_BASE                     0x45000300UL
#define SN_PMU                          ((SN_PMU_Type           *) SN_PMU_BASE)

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
