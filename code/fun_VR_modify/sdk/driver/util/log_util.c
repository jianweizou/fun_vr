
#include <stdarg.h>
#include <stdio.h>
#include "snc_types.h"
#include "log.h"
#include "snc_log_util.h"

#define PRINT_BUFFER_SIZE       128

#define LOG_UTIL_VERSION        0x73200001

uint32_t        gLog_flags = 0x1UL;

static uart_port_t      g_dbg_port = UART_PORT_NUM;
static char             g_buffer[PRINT_BUFFER_SIZE] = {0};

uint32_t
LOG_Util_GetVersion(void)
{
    return LOG_UTIL_VERSION;
}

/**
 * APP layer API, enable log
 */
void LOG_Driver_Enable(sn_log_type type)
{
    gLog_flags |=  (1<<type);
}

/**
 * APP layer API, disable log
 */
void LOG_Driver_Disable(sn_log_type type)
{
    gLog_flags &= ~(1<<type);
}


int
LOG_Driver_Init(log_driver_init_t *pInit)
{
    int     rval = 0;

    do {
        if( pInit->uart.port != UART_PORT_0 &&
            pInit->uart.port != UART_PORT_1 )
        {
            rval = -1;
            break;
        }

        g_dbg_port = pInit->uart.port;
        UART_Init(g_dbg_port, pInit->uart.sys_clock, pInit->uart.baud_rate);
    } while(0);

    return rval;
}



int
printf_uart(const char* fmt, ...)
{
    int         result = 0;
    va_list     args;

    do {
        uint8_t    *pCur = 0;

        if( g_dbg_port == UART_PORT_NUM )
            break;

        va_start(args, fmt);
        result = vsnprintf(g_buffer, PRINT_BUFFER_SIZE, fmt, args);
        va_end(args);

        if( result < 0 )
        {
            return result;
        }

        pCur = (uint8_t*)&g_buffer[0];
        while( *pCur )
        {
            UART_WriteByte(g_dbg_port, *pCur++);
        }

    } while(0);

    return result;
}

int
fprintf_uart(FILE *p, const char* fmt, ...)
{
    int         result = 0;
    va_list     args;

    va_start(args, fmt);
    result = printf_uart(fmt, args);
    va_end(args);

    return result;
}


/** Jlik SWO debug port ************************************************/

int
printf_swo(const char* fmt, ...)
{
    int         result = 0;
    va_list     args;

    do {
        uint8_t    *pCur = 0;

        va_start(args, fmt);
        result = vsnprintf(g_buffer, PRINT_BUFFER_SIZE, fmt, args);
        va_end(args);

        if( result < 0 )
        {
            return result;
        }

        pCur = (uint8_t*)&g_buffer[0];
        while( *pCur )
        {
            ITM_SendChar(*pCur++);
        }

    } while(0);

    return result;
}


int
fprintf_swo(FILE *p, const char* fmt, ...)
{
    int         result = 0;
    va_list     args;

    va_start(args, fmt);
    result = printf_swo(fmt, args);
    va_end(args);

    return result;
}

/* used for Debug Input */
volatile int ITM_RxBuffer = ITM_RXBUFFER_EMPTY;
int getchar_swo(void)
{
  while( ITM_CheckChar() != 1 ) {}

  return ITM_ReceiveChar();
}

/* Redirect printf  */
