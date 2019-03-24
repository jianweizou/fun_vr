#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define LOG_COLOR_RED           "\033[31m"
#define LOG_COLOR_GREEN         "\033[32m"
#define LOG_COLOR_YELLOW        "\033[33m"
#define LOG_COLOR_BLUE          "\033[34m"
#define LOG_COLOR_MAGENTA       "\033[35m"
#define LOG_COLOR_CYAN          "\033[36m"
#define LOG_COLOR_RESET         "\033[m"


/**
 *  log type and it MUST be less than LOG_FLAGS_MAX_NUM
 */
typedef enum _LOG_TYPE
{
    LOG_ERR            = 0,
    LOG_WARNING,
    LOG_INFO,
    LOG_DBG,

    LOG_FLAGS_MAX_NUM = 31  // uint32_t

} LOG_TYPE;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define LOG_FLAG_SET(LogFlags, bit_order)       ((LogFlags) |=  (1<<((bit_order))))
#define LOG_FLAG_CLR(LogFlags, bit_order)       ((LogFlags) &= ~(1<<((bit_order))))
#define LOG_FLAG_IS_SET(LogFlags, bit_order)    ((LogFlags) &   (1<<((bit_order))))
#define LOG_FLAG_ZERO(LogFlags)                 ((LogFlags) = 0x1)

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern uint32_t    gLog_flags;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

// #define err_msg(str, args...)               fprintf(stderr, LOG_COLOR_RED "%s[#%u] " str LOG_COLOR_RESET, __func__, __LINE__, ## args)

#define LOG_DRIVER_ENABLE

#if defined(LOG_DRIVER_ENABLE)

    #define _assert(expression)                                             \
            do{ if(expression) break;                                       \
                printf("%s: %s[#%u]\n", #expression, __FILE__, __LINE__);   \
                while(1);                                                   \
            }while(0)

    #define sn_log(type, str, args...)             ((void)((LOG_FLAG_IS_SET(gLog_flags, type)) ? \
                                                            printf("%s[#%u] " str, __func__, __LINE__, ## args): 0))
    #define sn_log_on(type)                        LOG_FLAG_SET(gLog_flags, type)
    #define sn_log_off(type)                       LOG_FLAG_CLR(gLog_flags, type)

    #define trace(str, args...)                    do{ printf("%s[%d]=> " str, __func__, __LINE__, ## args); \
                                                   }while(0)
#else /* #if (LOG_DRIVER_ENABLE) */

    #define _assert(expression)

    #define sn_log(type, str, args...)
    #define sn_log_on(type)
    #define sn_log_off(type)


    #define trace(str, args...)

#endif /* #if (LOG_DRIVER_ENABLE) */


#ifdef __cplusplus
}
#endif

#endif // LOG_H_INCLUDED
