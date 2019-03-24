/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_pmu.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/5
 * @license
 * @description
 */

#ifndef __snc_pmu_H_eef60688_ec05_4f6c_bb1d_a915a4c46839__
#define __snc_pmu_H_eef60688_ec05_4f6c_bb1d_a915a4c46839__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

#define PMU_FLAG_RTC_Msk            0x00000010     /* Bit4 */    
#define PMU_FLAG_WKP_Msk            0x00000040     /* Bit6 */
#define PMU_FLAG_WKP_INT_Msk        0x00000080     /* Bit7 */

    
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef enum pmu_mode
{
    
    PMU_DEEP_POWER_DOWN         = 1,
    PMU_DEEP_SLEEP              = 2,
    PMU_SLEEP                   = 3,
    
}pmu_mode_t;

typedef enum pmu_wkp_flag
{ 
    
    FLAG_WAKE_PIN,
    FLAG_RTC,
    
}pmu_wkp_flag_t;


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
 *  \brief Instruct CPU to enter low power mode.
 *  
 *  \param [in] mode: Low power mode @pmu_mode_t_t
 *  
 */
void PMU_Enter_Mode(pmu_mode_t mode);



/**
 *  \brief: Return the Wake Pin status
 *  \return: 0- WKP has been pressed, 1-WKP Idle.
 */
uint32_t PMU_WAKE_PIN(void);



/**
 *  \brief: Returns the status of pmu flag
 *  
 *  \param [in] flag: Wake up flags @pmu_wkp_flag_t
 *  \return: 0- none, 1-wake event occurred, else- invaild.
 *  
 *  \details: Read the status of wakeup event.
 */
uint32_t PMU_Get_Wake_Flag(pmu_wkp_flag_t flag);



/**
 *  \brief: Clear the wakeup event flag.
 *  
 *  \param [in] flag: Wake up flags @pmu_wkp_flag_t
 *
 */
void PMU_Clr_Wake_Flag(pmu_wkp_flag_t flag);



/**
 *  \brief: Return driver version.
 */
uint32_t PMU_GetVersion(void);



#ifdef __cplusplus
}
#endif

#endif
