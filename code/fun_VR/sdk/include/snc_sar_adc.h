/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_sar_adc.h
 *
 * @author Kaishiang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_sar_adc_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__
#define __snc_sar_adc_H_w9zgzwXd_l8tY_HqbU_sJ8i_uUIaPhvIRBqD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum CHANNEL_SELECT
{	
		SAR_ADC_AIN0=0,	
		SAR_ADC_AIN1,
		SAR_ADC_AIN2,
		SAR_ADC_AIN3,
		SAR_ADC_AIN4,
		SAR_ADC_AIN5,
		SAR_ADC_ALL_disable,
	
} channel_select_t;

typedef enum AD_2M_CLK
{
		SAR_ADC_CLK_1_92M 	=0, 	
		SAR_ADC_CLK_16_2M,		
} ad_2m_clk_t;

typedef enum ADC_CLOCK_SEL_0
{		
		ADC_CLOCK_1_93M     =0,
		ADC_CLOCK_964K,
		ADC_CLOCK_482K,
		ADC_CLOCK_241K,
		ADC_CLOCK_120K,
		ADC_CLOCK_60K,
		ADC_CLOCK_30K,
		ADC_CLOCK_15K,	
} adc_clock_sel_0_t;

typedef enum ADC_CLOCK_SEL_1
{	
		ADC_CLOCK_16_2M     =0,  		
		ADC_CLOCK_8_1M,
		ADC_CLOCK_4_05M,
		ADC_CLOCK_2_03M,		
		ADC_CLOCK_1_01M,
		ADC_CLOCK_506K,
		ADC_CLOCK_253K,
		ADC_CLOCK_126K,		
} adc_clock_sel_1_t;

typedef enum ADC_MODE
{
		SAR_ADC_NORMAL 	=0, 	
		SAR_ADC_SINGLE,		
} ad_mode_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sar_adc_info_t
{    

    ad_mode_t       			ad_mode;
    uint32_t    				adc_clock_sel;
    channel_select_t            channel_select;       
	ad_2m_clk_t                 adc_2M_clk;
	  
} sar_adc_info;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void SAR_ADC_init(sar_adc_info *sar_adc_info);
void SAR_ADC_Start(void);
void SAR_ADC_Enable(void);
void SAR_ADC_Disable(void);
uint32_t SAR_ADC_DATA(void);
uint32_t SAR_ADC_GetVersion(void);
uint32_t SAR_ADC_DATA_valid(void);
#ifdef __cplusplus
}
#endif

#endif
