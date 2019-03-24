/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
 
/** @file snc_iso_rtc.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/04/27
 * @license
 * @description
 */

#ifndef __SNC_ISO_RTC_H_f89b0bce_1a92_4749_b85c_841a5fd39346__
#define __SNC_ISO_RTC_H_f89b0bce_1a92_4749_b85c_841a5fd39346__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

#define RTC_CTRL_ENABLE_POS             7

#define PMU_FLAG_RTC_MSK                SET_BIT4
#define PMU_FLAG_WKP_MSK                SET_BIT6
#define PMU_FLAG_WAKE_INT_MSK           SET_BIT7
 
typedef enum rtc_ret_value
{
    RTC_SUCCESS             = 0,
    
    RTC_FAIL                    ,
    RTC_INVALID_DLO_SETTING     ,
    RTC_INVALID_DLO_ADDRESS     ,
    RTC_INVALID_DLO_SIZE        ,
    RTC_UNALIGNED_TIME_SETTING  ,
    RTC_SET_OVER_RANGE          ,
    
}rtc_ret_value_t;

typedef enum rtc_wake_cpu
{
    RTC_WAKEUP_CPU_ENABLE   = 0,
    RTC_WAKEUP_CPU_DISABLE  = 1,
        
}rtc_wake_cpu_t;
    
typedef enum rtc_interval
{
    
    RTC_INT_500_MS          = 0,
    RTC_INT_1_SEC           = 1,
    RTC_INT_5_SEC           = 4,
    RTC_INT_10_SEC          = 5,
    RTC_INT_30_SEC          = 6,
    RTC_INT_60_SEC          = 7,
    
}rtc_interval_t;


typedef enum rtc_src
{
    
    RTC_SRC_ILRC            = 0,
    RTC_SRC_LXTL            = 1,
    
}rtc_src_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct rtc_setting
{
    
    rtc_wake_cpu_t  wakeup_cpu;
    rtc_interval_t  timeout_interval;
    rtc_src_t       rtc_clk_source;
    
}rtc_setting_t;

typedef struct iso_bk_reg
{
    
    
}iso_bk_reg_t;

typedef struct iso_bk_var
{
    uint16_t    dlo_mark;
    uint32_t    dlo_addr;
    uint16_t    alarm_tick;
    uint16_t    cur_ticks;
    
}iso_bk_var_t;

typedef struct calendar
{
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
    uint32_t week;  /* 1-MON, 2-TUE, .... 7-SUN */
    
}rtc_calendar_t;

typedef struct dlo_table
{
    uint32_t    load_str;
    uint32_t    load_ro_base;
    uint32_t    exec_ro_base;
    uint32_t    ro_length;
    
    uint32_t    load_rw_base;
    uint32_t    exec_rw_base;
    uint32_t    rw_length;
    
    uint32_t    exec_zi_base;
    uint32_t    zi_length;
} dlo_table_t;


//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
rtc_ret_value_t RTC_Initial(rtc_setting_t *pSet);
void RTC_Deinitial(void);
void RTC_Start(void);
void RTC_Stop(void);
void RTC_Clear_INT(void);

/**
 *  \brief: RTC_Get_Int_Count
 *  
 *  \return: RTC interrupted count after RTC enabled.
 *  
 *  \details: Clear to zero if RTC had been enable.
 *  
 */
uint32_t RTC_Get_Int_Count(void);

/**
 *  \brief: RTC_Check_Running
 *  
 *  \return: 0- RTC is stopped, 1-RTC is running
 *  
 *  \details:
 *  
 */
uint32_t RTC_Check_Running(void);

/**
 *  \brief: RTC_Get_WKP
 *  
 *  \return: 0- Wake pin idle, 1-Wake pin is triggered
 *  
 *  \details:
 *  
 */
uint32_t RTC_Get_WKP(void);


//=============================================================================
//                  RTC DLO function
//=============================================================================

/**
 *  \brief: RTC_Set_DownCount
 *  
 *  \param [in] after_seconds: The alarm settings in seconds.
 *  
 *         |Aligned unit of ---|Maximun count
 *         |alarm setting   ---|
 *  -------|-------------------|-----------------------------------
 *         | 1                 | 65535s    => 18   hr
 *         | 5                 | 327675s   => 91   hr  => 3.7 day
 *         | 10                | 655350s   => 182  hr  => 7.5 day
 *         | 30                | 1966050s  => 546  hr  => 22.7 day
 *         | 60                | 3932100s  => 1092 hr  => 45.5 day
 *
 *  \return:    RTC return value defined @rtc_ret_valuse_t.
 * 
 *  \details:   Set and enable the alarm (unit: second) function,
 *              If the chip is in deep power down mode while alarm event occurs,
 *              it wakes up form DPD and boot from main(by default) or DLO(if DLO is enabled)
 *  
 *              the minimum unit is according to the current @rtc_interval_t 
 *              setting and the "[in] after_seconds" must be the multiple of it.
 */
rtc_ret_value_t RTC_Set_DownCount(uint32_t after_seconds);


/**
 *  \brief: RTC_Clr_DownCount
 *  
 *  \return: None
 *  
 *  \details: Clear the DownCount setting.
 *  
 */
void RTC_Clr_DownCount(void);


/**
 *  \brief: RTC_Set_Dlo
 *  
 *  \param [in] p: Pointer of dlo table data @dlo_table_t in spi-nor-flash space.
                if (p == 0), will clear the dlo setting.
 *  \return:    RTC return value defined @rtc_ret_valuse_t.
 *  
 *  \details:   Set the dlo table to activate the rom code dlo function.
 *  
 */
rtc_ret_value_t RTC_Set_Dlo(dlo_table_t * p);


/**
 *  \brief: RTC_Clr_Dlo
 *  
 *  \return: None
 *  
 *  \details:   Disable DLO function of rom code.
 *  
 */
void RTC_Clr_Dlo(void);



//=============================================================================
//                  RTC driver extension 
//=============================================================================

/**
 *  \brief: RTC_Set_Calendar
 *  
 *  \param [in] p: pointer of sturcture @rtc_calendar_t
 *  \return:    RTC return value defined @rtc_ret_value_t.
 *  
 *  \details:   Set the calendar and time into the rtc register,
 *              This function will also start the RTC counter.
 *              The effective date is from 2000/1/1 to 2136/2/5
 **/
rtc_ret_value_t RTC_Set_Calendar(rtc_calendar_t *p);


/**
 *  \brief: RTC_Update_Calendar
 *   
 *  \param [in] p: pointer of sturcture @rtc_calendar_t
 *         [in] rtc_tick: the interrupt number from RTC enable.
 *  \return:    RTC return value defined @rtc_ret_valuse_t.
 *  
 *  \details:   Get the calendar in rtc module, it updates the sturcture
 *              with input pointer. returns fail if rtc is not enable. 
 *              The effective date is from 2000/1/1 to 2136/2/5
 */
rtc_ret_value_t RTC_Update_Calendar(rtc_calendar_t *p, uint32_t rtc_tick);

/**
 *  \brief: RTC_Set_Alarm
 *   
 *  \param [in] p: pointer of sturcture @rtc_calendar_t
 *  \return:    RTC return value defined @rtc_ret_valuse_t.
 *
 *  \limitation: This function must implement with "rtc_dlo_table.s" and 
                 "rtc_dlo_prog.c"
 *  
 *  \details:   Set a alarm by calendar in rtc module, it boot to main
                if alarm occour or WKP has been trigger.
 *              The effective date is from 2000/1/1 to 2136/2/5
 */
rtc_ret_value_t RTC_Set_Alarm(rtc_calendar_t *p);



/**
 *  \brief: Return driver version.
 */
uint32_t RTC_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif
