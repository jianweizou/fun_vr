/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file iso_rtc_ex.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/05/08
 * @license
 * @description
 */

#include "reg_util.h"
#include "snc_iso_rtc.h"
#include "register_7320.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define SECOND_PER_DAY     86400

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================


//=============================================================================
//                  Global Data Definition
//=============================================================================

const char week_of_2000_1_1 = 6;
const char day_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//=============================================================================
//                  Private Function Definition
//=============================================================================
uint32_t _rtc_second_2_calendar(uint32_t count, rtc_calendar_t* p);
uint32_t _rtc_calendar_2_second(rtc_calendar_t* p);
static uint32_t _rtc_int_count_to_second(uint32_t count);
static uint32_t _rtc_second_to_int_count(uint32_t second);

//=============================================================================
//                  Public Function Definition
//=============================================================================
/** 
 * Support 0~65535 seconds at this version, the rtc interrupt interval
 * should be the same with the calendar functions.
 **/

rtc_ret_value_t RTC_Set_DownCount(uint32_t after_seconds)
{
    //Maximun timeout:
    //unit-1    -> 65535s   => 18.2 hr
    //unit-5    -> 327675s  => 91 hr    => 3.7 day
    //unit-10   -> 655350s  => 182 hr   => 7.5 day
    //unit-30   -> 655350s  => 546 hr   => 22.7 day
    //unit-60   -> 655350s  => 1092 hr  => 45.5 day
    
    const uint8_t unit[] = {1, 5, 10, 30, 60};
    
    /* If RTC is running, check if the second is alignd with current unit */
    if(RTC_Check_Running())
    {
        uint32_t unit;
        switch(SN_ISO->ISO_CTRL_b.RTC_Timeout)
        {
            case RTC_INT_500_MS:
                after_seconds *=2;
                
            case RTC_INT_1_SEC: 
            case 2:
            case 3:
                unit = 1; 
                break;
            case RTC_INT_5_SEC:
                unit = 5;
                break;
            case RTC_INT_10_SEC:
                unit = 10;
                break;
            case RTC_INT_30_SEC:
                unit = 30;
                break;
            case RTC_INT_60_SEC:
                unit = 60;
                break;
        }
        uint32_t max_time = unit * 0xFFFF;
        
        if(after_seconds > max_time)
        {
            return RTC_SET_OVER_RANGE;
        }
        
        if((after_seconds % unit) != 0)
        {
            return RTC_UNALIGNED_TIME_SETTING;
        }
    }
    else
    {
        /* Caculate the effecitve setting */
        for(int i=0; i<5; i++)
        {
            uint32_t max_time = unit[i] * 0xFFFF;
            if(after_seconds < max_time)
            {
                if((after_seconds % unit[i]) != 0)
                {
                    return RTC_UNALIGNED_TIME_SETTING;
                }
                else
                {
                    break;
                }
            }
        }
        
        if(after_seconds > 60 * 0xFFFF)
        {
            return RTC_SET_OVER_RANGE;
        }
    }

    SN_ISO->ISO_CTRL_b.RTC_WKP  = RTC_WAKEUP_CPU_ENABLE;
    
    BK_REG->COUNTER_0          = 0;
    BK_REG->COUNTER_1          = 0;
    
    BK_REG->ALARM_SET_0        = after_seconds & 0xFF;
    BK_REG->ALARM_SET_1        = (after_seconds >> 8) & 0xFF;
    
    return RTC_SUCCESS;
}


void RTC_Clr_Alarm()
{
    {
        //Maximun timeout:
        //unit-1    -> 65535s   => 18.2 hr
        //unit-5    -> 327675s  => 91 hr    => 3.7 day
        //unit-10   -> 655350s  => 182 hr   => 7.5 day
        //unit-30   -> 1966050s  => 546 hr   => 22.7 day
        //unit-60   -> 3932100s  => 1092 hr  => 45.5 day
    }

    BK_REG->COUNTER_0          = 0;
    BK_REG->COUNTER_1          = 0;
    
    BK_REG->ALARM_SET_0        = 0;
    BK_REG->ALARM_SET_1        = 0;
}

uint32_t Check_Address(uint32_t addr)
{
    struct
    {
        uint32_t addr_base;
        uint32_t addr_end;
    }effective_table[] = { 0,            0x10000,
                           0x18000000,   0x18040000,
                           0x20000000,   0x20001000,
                           0x30000000,   0x30800000,
                           0x60000000,   0x70000000,
                           0xFFFFFFFF
                          };
     
    for(int i=0; effective_table[i].addr_base != 0xFFFFFFFF; i++)
    {
        if(addr >= effective_table[i].addr_base && 
           addr < effective_table[i].addr_end )
        {
            return RTC_SUCCESS;
        }
    }
    return RTC_FAIL;
}

void RTC_Clr_Dlo()
{
    BK_REG->DLO_SYMBOL_L       = 0;
    BK_REG->DLO_SYMBOL_H       = 0;
}


rtc_ret_value_t RTC_Set_Dlo(dlo_table_t * p)
{
    uint32_t addr = (uint32_t)p;
    
    /* Reboot to main */
    if( (uint32_t) p == 0)
    {
        RTC_Clr_Dlo();
        return RTC_FAIL;
    }    
    
    if(addr < 0x60000000 || addr >= 0x70000000) 
    {
        return RTC_INVALID_DLO_ADDRESS;
    }
    
    if(p->load_str != 0x64616F6C)   /* string "load"   */
    {
        return RTC_INVALID_DLO_SETTING;
    }
    
    if(Check_Address((uint32_t)p->load_ro_base))
        return RTC_INVALID_DLO_ADDRESS;
    
    if(Check_Address((uint32_t)p->exec_zi_base))
        return RTC_INVALID_DLO_ADDRESS;
    
    if(p->ro_length == 0)
        return RTC_INVALID_DLO_SIZE;
    
    BK_REG->DLO_SYMBOL_L       = 0xAA;
    BK_REG->DLO_SYMBOL_H       = 0xAA;
    
    /* Load table address */ 
    BK_REG->DLO_ADDR_0 = (addr >> 0 ) & 0xFF;
    BK_REG->DLO_ADDR_1 = (addr >> 8 ) & 0xFF;
    BK_REG->DLO_ADDR_2 = (addr >> 16) & 0xFF;
    BK_REG->DLO_ADDR_3 = (addr >> 24) & 0xFF;
    
    SN_ISO->ISO_CTRL_b.RTC_WKP  = RTC_WAKEUP_CPU_ENABLE;
    
    return RTC_SUCCESS;
}

/**
 * RTC Calendar limitation: Calendar can't be stop/pause
 * Defined the calendar is start from 2000/1/1 to 2136/2/5
 **/

rtc_ret_value_t RTC_Set_Calendar(rtc_calendar_t *p)
{
    RTC_Stop();
    
    uint32_t count = _rtc_calendar_2_second(p); 
    
    count = _rtc_second_to_int_count(count);
       
    BK_REG->CALENDAR_0 = (count >>  0) & 0xFF;
    BK_REG->CALENDAR_1 = (count >>  8) & 0xFF;
    BK_REG->CALENDAR_2 = (count >> 16) & 0xFF;
    BK_REG->CALENDAR_3 = (count >> 24) & 0xFF;

    RTC_Start(); 

    return RTC_SUCCESS;
}


rtc_ret_value_t RTC_Update_Calendar(rtc_calendar_t *p, uint32_t rtc_tick)
{
    if(RTC_Check_Running() == 0)
    {
        return RTC_FAIL;
    }
    
    /* Defined the calendar is start from 2000/01/01 00:00 */
    uint32_t timestamp = rtc_tick;
    uint32_t offset =   (BK_REG->CALENDAR_0 <<  0) |
                        (BK_REG->CALENDAR_1 <<  8) |
                        (BK_REG->CALENDAR_2 << 16) |
                        (BK_REG->CALENDAR_3 << 24);
    
    timestamp = _rtc_int_count_to_second(timestamp + offset);
    
    _rtc_second_2_calendar(timestamp, p);
    
    return RTC_SUCCESS;
}


rtc_ret_value_t RTC_Set_Alarm(rtc_calendar_t *p)
{
    if(RTC_Check_Running() == 0)
    {
        return RTC_FAIL;
    }
    
    uint32_t count = _rtc_calendar_2_second(p); 
    
    count = _rtc_second_to_int_count(count);

    BK_REG->ALARM_CA_0 = (count >>  0) & 0xFF;
    BK_REG->ALARM_CA_1 = (count >>  8) & 0xFF;
    BK_REG->ALARM_CA_2 = (count >> 16) & 0xFF;
    BK_REG->ALARM_CA_3 = (count >> 24) & 0xFF;
      
    return RTC_SUCCESS;
}

uint32_t _rtc_second_2_calendar(uint32_t count, rtc_calendar_t* p)
{
    uint32_t timestamp = count;
    uint32_t day_total = timestamp     / SECOND_PER_DAY;
    
    p->week = week_of_2000_1_1 + (day_total % 7);
    p->week = (p->week > 7) ? (p->week - 7) : p->week;
    
    /* Caculate the year */
    p->year = 2000;
    
    while(1)
    {
        uint32_t day_of_the_year = ((p->year % 4) == 0) ? 366 : 365;
        
        if(day_total >= day_of_the_year)
        {
            p->year++;
            day_total -= day_of_the_year;
        }
        else
        {
            break;
        }
    }
    
    /* Caculate the month of the year */
    p->month = 1;
    
    for(int month=1; month<=12; month++)
    {
        uint32_t days_in_month = day_table[month-1];
        
        if((month == 2) && ((p->year % 4) == 0))
        {
            days_in_month = 29;
        }
        
        if(day_total >= days_in_month)
        {
            day_total -= days_in_month;
            p->month++;
        }
        else
        {
            break;
        }
    }
    
    p->day = 1 + day_total;
    
    /* Caculate the HH:MM:SS */
    uint32_t time_of_the_day = timestamp    % SECOND_PER_DAY;
    
    p->hour             = time_of_the_day   / 3600;
    time_of_the_day    -= p->hour           * 3600;
    
    p->min              = time_of_the_day   / 60;
    time_of_the_day    -= p->min            * 60;

    p->sec              = time_of_the_day;   
    
    return 0;
}

uint32_t _rtc_calendar_2_second(rtc_calendar_t* p)
{
    uint32_t total_count = 0;
    uint32_t temp;
     
    /* Caculate the year */   
    for(temp = 2000; temp < p->year; temp++)
    {
        uint32_t day_of_the_year = ((temp % 4) == 0) ? 366 : 365;
        
        total_count += day_of_the_year * SECOND_PER_DAY;
    }
    
    /* Caculate the month */
    for(temp = 1; temp < p->month; temp++)
    {
        uint32_t day_of_the_month = day_table[temp-1];
        
        if((temp == 2) && ((p->year % 4) == 0))
        {
            day_of_the_month = 29;
        }
        
        total_count += SECOND_PER_DAY * day_of_the_month;
    }
    
    /* Caculate the day */
    total_count += (p->day - 1) * SECOND_PER_DAY;
    
        
    /* Caculate the HH:MM:SS */
    total_count += p->hour       * 3600;
    total_count += p->min        * 60;
    total_count += p->sec;
    
    return total_count;
}

static uint32_t _rtc_int_count_to_second(uint32_t count)
{
    switch(SN_ISO->ISO_CTRL_b.RTC_Timeout)
    {
        case 0:     /* 500 ms */ 
            count /= 2;             /* 2 tick = 1 second*/
            break;
        case 1:
        case 2:
        case 3:     /* 1 second */ 
            break;
        case 4:     /* 5 seconds */ 
            count *= 5;              /* 1 tick = 5 second*/
            break;
        case 5:     /* 10 seconds */ 
            count *= 10;             /* 1 tick = 10 second*/
            break;
        case 6:     /* 30 seconds */ 
            count *= 30;             /* 1 tick = 30 second*/
            break;
        case 7:     /* 60 seconds */ 
            count *= 60;             /* 1 tick = 60 second*/
            break;
    }
    
    return count;
}

static uint32_t _rtc_second_to_int_count(uint32_t second)
{
    switch(SN_ISO->ISO_CTRL_b.RTC_Timeout)
    {
        case 0:     /* 500 ms */ 
            second *= 2;             /* 2 tick = 1 second*/
            break;
        case 1:
        case 2:
        case 3:     /* 1 second */ 
            break;
        case 4:     /* 5 seconds */ 
            second /= 5;              /* 1 tick = 5 second*/
            break;
        case 5:     /* 10 seconds */ 
            second /= 10;             /* 1 tick = 10 second*/
            break;
        case 6:     /* 30 seconds */ 
            second /= 30;             /* 1 tick = 30 second*/
            break;
        case 7:     /* 60 seconds */ 
            second /= 60;             /* 1 tick = 60 second*/
            break;
    }
    
    return second;
}




