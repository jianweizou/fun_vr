/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_log_util.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */


#ifndef __snc_log_H_wSOFi54t_lkgV_HOLK_s3j9_uJUCs5Ye5tYQ__
#define __snc_log_H_wSOFi54t_lkgV_HOLK_s3j9_uJUCs5Ye5tYQ__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "snc_uart.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum app_log
{
    APP_LOG_ERR     = 0,
    APP_LOG_INFO,
    APP_LOG_WARNING,
    APP_LOG_DBG,

} app_log_t;

/**
 * APP layer API, driver log type
 */
typedef enum _sn_log_type
{
    SN_LOG_ERR            = 0,
    SN_LOG_WARNING,
    SN_LOG_INFO,
    SN_LOG_DBG,

} sn_log_type;

typedef struct log_driver_init
{
    union {
        struct {
            uint32_t        sys_clock;
            uint32_t        baud_rate;
            uart_port_t     port;
        } uart;
    };

} log_driver_init_t;


//=============================================================================
//                  Macro Definition
//=============================================================================
/**
 *  This log macros are used at application layer or share lib
 *      - act_flags: flags of on/off log,
 *                      normally it will be a integer variable, which declaring by user.
 *      - log_type : enum app_log
 *
 */
#if (SNX_DEBUG)

    #define LOG_RED           "\033[31m"
    #define LOG_GREEN         "\033[32m"
    #define LOG_YELLOW        "\033[33m"
    #define LOG_BLUE          "\033[34m"
    #define LOG_MAGENTA       "\033[35m"
    #define LOG_CYAN          "\033[36m"
    #define LOG_RESET         "\033[m"

    #define snc_log_color(act_flags, log_type, color, str, args...) \
                (void)(((act_flags) & (0x1 << (log_type))) ? printf(color str LOG_RESET, ##args) : 0)

    #define snc_log_color_ex(act_flags, log_type, color, str, args...) \
                (void)(((act_flags) & (0x1 << (log_type))) ? printf(color "%s[%u] " str LOG_RESET, __func__, __LINE__, ##args) : 0)

    #define snc_log(act_flags, log_type, str, args...) \
                (void)(((act_flags) & (0x1 << (log_type))) ? printf(str, ##args) : 0)

    #define snc_log_ex(act_flags, log_type, str, args...) \
                (void)(((act_flags) & (0x1 << (log_type))) ? printf("%s[%u] " str, __func__, __LINE__, ##args) : 0)

#else
    #define snc_log_color(act_flags, log_type, color, str, args...)
    #define snc_log(act_flags, log_type, str, args...)
    #define snc_log_color_ex(act_flags, log_type, color, str, args...)
    #define snc_log_ex(act_flags, log_type, str, args...)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

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
 *  \brief init driver log system
 *
 *  \param [in] pInit   initial info
 *  \return             0: ok, other: fail
 *
 *  \details
 */
int
LOG_Driver_Init(log_driver_init_t *pInit);


uint32_t
LOG_Util_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif // LOG_UTIL_H_INCLUDED
