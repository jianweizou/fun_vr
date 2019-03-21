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

/* WiFi includes. */
#include "wifi_api.h"
#include "USBH.h"

#if CONFIG_MODULE_WIFI_BCM43362
static BaseType_t prvBcm43362InitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvBcm43362StaJoinCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvBcm43362StaLeaveCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvBcm43362ApStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvBcm43362ApStopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvBcm43362ScanCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*Global Variable*/
int bcm_init = 0;

static const CLI_Command_Definition_t xBcm43362_Init =
{
	"bcm43362_init", /* The command string to type. */
	"\r\nbcm43362_init\r\n Initialize bcm43362.\r\n",
	prvBcm43362InitCommand, /* The function to run. */
	0 /* parameters are expected. */
};

static const CLI_Command_Definition_t xBcm43362_StaJoin =
{
	"bcm43362_join", /* The command string to type. */
	"\r\nbcm43362_join [SSID] [Password]\r\n Join one AP router.\r\n",
	prvBcm43362StaJoinCommand, /* The function to run. */
	2 /* parameters are expected. */
};

static const CLI_Command_Definition_t xBcm43362_StaLeave =
{
	"bcm43362_leave", /* The command string to type. */
	"\r\nbcm43362_leave\r\n  Leave connected AP router .\r\n",
	prvBcm43362StaLeaveCommand, /* The function to run. */
	0 /* parameters are expected. */
};

static const CLI_Command_Definition_t xBcm43362_ApStart =
{
	"bcm43362_apstart", /* The command string to type. */
	"\r\nbcm43362_apstart [SSID] [open|wpa2_aes] [key]\r\n  Start ap mode.\r\n",
	prvBcm43362ApStartCommand, /* The function to run. */
	0/* parameters are expected. */
};

static const CLI_Command_Definition_t xBcm43362_ApStop =
{
	"bcm43362_apstop", /* The command string to type. */
	"\r\nbcm43362_apstop\r\n  Stop ap mode.\r\n",
	prvBcm43362ApStopCommand, /* The function to run. */
	0 /* parameters are expected. */
};

static const CLI_Command_Definition_t xBcm43362_Scan =
{
	"bcm43362_scan", /* The command string to type. */
	"\r\nbcm43362_scan\r\n  Bcm43362 Scan.\r\n",
	prvBcm43362ScanCommand, /* The function to run. */
	0 /* parameters are expected. */
};


/*-----------------------------------------------------------*/

static BaseType_t prvBcm43362InitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
		 if(!bcm43362_init())
				bcm_init = 1;

    return pdFALSE;
}

static BaseType_t prvBcm43362StaJoinCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	char ssid[32], pwd[32];
	const char *param;
	BaseType_t 	xParameterStringLength;  
  
	if(!bcm_init){
		printf(("Please initialize bcm43362 first.\n"));
		return pdFALSE;
	}
		
	memset(ssid, 0, sizeof(ssid));
	memset(pwd, 0, sizeof(pwd));
	
	param = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      1,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                  );
	strncpy(ssid, param, xParameterStringLength);
	
  param = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      2,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                 );
								 strncpy(pwd, param, xParameterStringLength);
	
	bcm43362_sta_join(ssid, pwd);

	return pdFALSE;
}

static BaseType_t prvBcm43362StaLeaveCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    
	if(!bcm_init){
		printf(("Please initialize bcm43362 first.\n"));
		return pdFALSE;
	}
	
	 bcm43362_sta_leave();

    return pdFALSE;
}

static BaseType_t prvBcm43362ApStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    
	if(!bcm_init){
		printf(("Please initialize bcm43362 first.\n"));
		return pdFALSE;
	}
	
	 bcm43362_ap_start("wayne", "open", NULL);

    return pdFALSE;
}

static BaseType_t prvBcm43362ApStopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    
	if(!bcm_init){
		printf(("Please initialize bcm43362 first.\n"));
		return pdFALSE;
	}
	
	 bcm43362_ap_stop();

    return pdFALSE;
}

static BaseType_t prvBcm43362ScanCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    
	if(!bcm_init){
		printf(("Please initialize bcm43362 first.\n"));
		return pdFALSE;
	}
	
	 bcm43362_scan();

    return pdFALSE;
}
#endif

#if CONFIG_MODULE_WIFI_MT7601
static BaseType_t prvMT7601InitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvMT7601StaJoinCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvMT7601ApStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static const CLI_Command_Definition_t xMT7601_Init =
{
	"mt7601_init", /* The command string to type. */
	"\r\nmt7601_init\r\n Initialize mt7601.\r\n",
	prvMT7601InitCommand, /* The function to run. */
	0 /* parameters are expected. */
};

static const CLI_Command_Definition_t xMT7601_StaJoin =
{
	"mt7601_join", /* The command string to type. */
	"\r\nmt7601_join [SSID] [Password]\r\n Join one AP router.\r\n",
	prvMT7601StaJoinCommand, /* The function to run. */
	2 /* parameters are expected. */
};

static const CLI_Command_Definition_t xMT7601_ApStart =
{
	"mt7601_apstart", /* The command string to type. */
	"\r\nmt7601_apstart [SSID] [open|wpa2_aes] [key]\r\n  Start ap mode.\r\n",
	prvMT7601ApStartCommand, /* The function to run. */
	0/* parameters are expected. */
};

static BaseType_t prvMT7601InitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	usbh_freertos_init_2();
	return pdFALSE;
}

static BaseType_t prvMT7601StaJoinCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  //mt7601_stajoin("D-Link_DIR-612", "111111");
	const char *param;
	char ssid[32], pwd[32];
	BaseType_t 	xParameterStringLength;
	
	memset(ssid, 0, sizeof(ssid));
	memset(pwd, 0, sizeof(pwd));
	
	param = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      1,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                  );
	strncpy(ssid, param, xParameterStringLength);
	
  param = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      2,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                 );
								 strncpy(pwd, param, xParameterStringLength); 
  		
	
	mt7601_stajoin(ssid, pwd);	
  return pdFALSE;
}

static BaseType_t prvMT7601ApStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    
		mt7601_apstart(NULL, NULL, 0);
    return pdFALSE;
}
#endif

/*-----------------------------------------------------------*/

void vRegisterWiFiCLICommands( void )
{
#if CONFIG_MODULE_WIFI_BCM43362	
	FreeRTOS_CLIRegisterCommand( &xBcm43362_Init ); 
  FreeRTOS_CLIRegisterCommand( &xBcm43362_StaJoin ); 
	FreeRTOS_CLIRegisterCommand( &xBcm43362_StaLeave ); 
	FreeRTOS_CLIRegisterCommand( &xBcm43362_ApStart ); 
	FreeRTOS_CLIRegisterCommand( &xBcm43362_ApStop ); 
	FreeRTOS_CLIRegisterCommand( &xBcm43362_Scan ); 
#endif	
#if CONFIG_MODULE_WIFI_MT7601
	FreeRTOS_CLIRegisterCommand( &xMT7601_Init ); 
	FreeRTOS_CLIRegisterCommand( &xMT7601_ApStart ); 
	FreeRTOS_CLIRegisterCommand( &xMT7601_StaJoin ); 
#endif	
}
