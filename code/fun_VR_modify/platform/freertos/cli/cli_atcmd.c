/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "pthread.h"
#include "mqueue.h"

#include "snc_uart.h"
#include "reg_util.h"
#include "register_7320.h"

#define msg(str, args...)       printf("%s[%u] " str, __func__, __LINE__, ##args);

//*****************************************************************************
// defines
//*****************************************************************************
#define APPLICATION_NAME        "AT Commands"
#define APPLICATION_VERSION     "1.1.1"

#define ATCOMMANDS_TASK_STACK_SIZE   (4096)
#define ATCOMMANDS_STOP_TIMEOUT      (200)
#define ATCOMMANDS_CMD_BUFFER_SIZE   (1024)
#define ATCOMMANDS_EVENT_BUFFER_SIZE (1024)

//****************************************************************************
// globals
//****************************************************************************
pthread_t  ATCommands_eventThread = (pthread_t)NULL;
char       ATCommands_cmdBuffer[ATCOMMANDS_CMD_BUFFER_SIZE];
char       ATCommands_eventBuffer[ATCOMMANDS_EVENT_BUFFER_SIZE];
#define    THREADSTACKSIZE    4096

void *ATCommands_eventTask(void *pvParameters)
{
    int status;
    while(1)
    {
        status = ATCmd_recv(ATCommands_eventBuffer,0);
        if (status >= 0)
        {
            char string_end[10]="\r\n";
            //msg("%s\n\r",ATCommands_eventBuffer);
	        UART_WriteMultiBytes(UART_PORT_1,ATCommands_eventBuffer,strlen(ATCommands_eventBuffer));
            UART_WriteMultiBytes(UART_PORT_1,string_end,strlen(string_end)); 
        }  
    }
}




static sn_uart_t* _uart_get_handle(uart_port_t     port_id)
{
    sn_uart_t   *pDev = 0;

    if( port_id == UART_PORT_0 )
        pDev = (sn_uart_t*)UART0_REG_BASE;
    else if( port_id == UART_PORT_1 )
        pDev = (sn_uart_t*)UART1_REG_BASE;

    return pDev;
}


int UART_Available(uart_port_t  port_id)
{
    sn_uart_t   *pDev = 0;

    pDev = _uart_get_handle(port_id);
    if( !pDev )
    {
        snc_assert(pDev != 0);
    }
    return reg_read_mask_bits(&pDev->LS, (0x1 << 0));
}


int GetCmd(char *pcBuffer, unsigned int uiBufLen)
{
    char    cChar='0';
    int     iLen = 0;

    iLen = 0;
    while(1)
    {
        if(iLen >= uiBufLen)
        {
            return -1;
        }
        if(UART_Available(UART_PORT_1))
        {
            cChar = UART_ReadByte(UART_PORT_1);
            if((cChar == '\r') || (cChar =='\n'))
            {
                break;
            }   
            *(pcBuffer + iLen) = cChar;
            iLen++;
        }
    }
    *(pcBuffer + iLen) = '\0';
    return iLen;
}



int32_t ATCommands_readCmd(void)
{
    int32_t     lRetVal;
    uint32_t    i = 1;

    while(i)
    {
        usleep(100);
        /* Poll UART terminal to receive user command terminated by '/r' */
        lRetVal = GetCmd((char *)ATCommands_cmdBuffer, ATCOMMANDS_CMD_BUFFER_SIZE);
        if (lRetVal <= 0)
        {
            continue;
        }
	    msg("ATCommands_cmdBuffer=%s\r\n",ATCommands_cmdBuffer);
        ATCmd_send(ATCommands_cmdBuffer);
    }
    return 0 ;
}

uart_port_1_init()
{
	uint32_t        sys_clk = 162 * 1000000;
    uint32_t        buad = 115200;
    UART_Init(UART_PORT_1, sys_clk, buad);
}


void mainThread(void *pvParameters)
{
    int     counter = 0;
	int32_t status = 0;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
#if 0
	IO_Pin_Setting();
    /* Configure the UART */
	  {   // enable log of driver
        log_driver_init_t  init_log = {0};
        init_log.uart.sys_clock = 162000000;
        init_log.uart.baud_rate = 115200;
        LOG_Driver_Init(&init_log);
    }
#endif    
    uart_port_1_init();	

    ATCmd_create();
 
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 5;
    status = pthread_attr_setschedparam(&pAttrs, &priParam);
    status |= pthread_attr_setstacksize(&pAttrs, ATCOMMANDS_TASK_STACK_SIZE);

    status = pthread_create(&ATCommands_eventThread, &pAttrs, ATCommands_eventTask, NULL);
    if(status != 0)
    {
        msg("could not create task\n\r");
        /* error handling */
        while(1);
    }
    ATCommands_readCmd();
}


static BaseType_t ATCmdInitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );


static const CLI_Command_Definition_t xATCmd_Init =
{
	"atcmd", /* The command string to type. */
	"\r\natcmd\r\n atcmd process start\r\n",
	ATCmdInitCommand, /* The function to run. */
	0 /* parameters are expected. */
};


static BaseType_t ATCmdInitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    pthread_t           thread;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Set priority and stack size attributes */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 1;
    detachState = PTHREAD_CREATE_DETACHED;
		
    retc = pthread_attr_setdetachstate(&pAttrs, detachState);
    if (retc != 0) {
        while (1);
    }
    pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE);
    if (retc != 0) {
        while (1);
    }
    retc = pthread_create(&thread, &pAttrs, mainThread, NULL);
    if (retc != 0) {
        while (1);
    }
	return pdFALSE;
}


void vRegisterAtCmdCLICommands( void )
{
	FreeRTOS_CLIRegisterCommand( &xATCmd_Init ); 	
}


