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

/* USBD includes. */
#include "mid_usbd.h"
#include "mid_usbd_msc.h"
#include "mid_sd.h"

static BaseType_t prvUsbdInitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvUsbdUninitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
static BaseType_t prvUsbdSDPlugOutCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvUsbdSDReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif	

static const CLI_Command_Definition_t xUsbdInit =
{
	"usbd_init", /* The command string to type. */
	"\r\nusbd_init <class> <xfer>\r\n usbd driver startup. <class - msc/uvc/uac/hid> <xfer - bulk/isoc/n> \r\n",
	prvUsbdInitCommand, /* The function to run. */
	2 /* parameters are expected. */
};

static const CLI_Command_Definition_t xUsbdUninit =
{
	"usbd_uninit", /* The command string to type. */
	"\r\nusbd_uninit\r\n",
	prvUsbdUninitCommand, /* The function to run. */
	0 /* parameters are expected. */
};


#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
static const CLI_Command_Definition_t xUsbdSDPlugOug =
{
	"usbd_spo", /* The command string to type. */
	"\r\nusbd_spo <val>\r\n usbd set sd plug out<val - 1:SD plug out/0:SD plug in> \r\n",
	prvUsbdSDPlugOutCommand, /* The function to run. */
	1 /* parameters are expected. */
};

static const CLI_Command_Definition_t xUsbdSDRead =
{
	"usbd_sdrd", /* The command string to type. */
	"\r\nusbd_sdrd <addr>:\r\n sd read\r\n",
	prvUsbdSDReadCommand, /* The function to run. */
	1 /* No parameters are expected. */
};
#endif

 
/*-----------------------------------------------------------*/

void vRegisterUsbdCLICommands( void )
{
    FreeRTOS_CLIRegisterCommand( &xUsbdInit ); 
	FreeRTOS_CLIRegisterCommand( &xUsbdUninit );
#if defined( CONFIG_MODULE_USBD_MSC_CLASS )     
    FreeRTOS_CLIRegisterCommand( &xUsbdSDPlugOug );
	FreeRTOS_CLIRegisterCommand( &xUsbdSDRead );
#endif	
}
/*-----------------------------------------------------------*/


static BaseType_t prvUsbdInitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char 	*usbd_class, *usbd_transfer;
    usbd_class_mode_t mode;
    usbd_class_submode_t xfer;
    int strlen;
    BaseType_t 	xParameterStringLength;
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    //( void ) pcCommandString;
    ( void ) xWriteBufferLen; 
    configASSERT( pcWriteBuffer );
  
    /* Start with an empty string. */
    pcWriteBuffer[0] = 0x00;

    /* Obtain the parameter string. */
    usbd_class = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      1,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                  );

    configASSERT( usbd_class );  
    
  
    usbd_transfer = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      2,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                  );
    configASSERT( usbd_transfer );
    
    strlen = (usbd_transfer-usbd_class-1);
    if(strncmp(usbd_class, "msc", strlen)==0)    {
        mode = USBD_MODE_MSC;
        xfer = USBD_SUBMODE_BULK;
    }
    else if(strncmp(usbd_class, "uvc", strlen)==0)    {
        mode = USBD_MODE_UVC;
        if(strcmp(usbd_transfer, "isoc")==0)
            xfer = USBD_SUBMODE_ISO;
        else
            xfer = USBD_SUBMODE_BULK;
    }
    else if(strncmp(usbd_class, "uac", strlen)==0)    {
        mode = USBD_MODE_UAC;
        xfer = USBD_SUBMODE_ISO;
    }
    else if(strncmp(usbd_class, "hid", strlen)==0)    {
        mode = USBD_MODE_HID;
        xfer = USBD_SUBMODE_NONE;
    }
    else  {
        printf("Bad command, try again.\n");
    }
    usbd_mid_test_init(mode, xfer);

    return pdFALSE;
}

static BaseType_t prvUsbdUninitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    //( void ) pcCommandString;
    ( void ) xWriteBufferLen; 
    configASSERT( pcWriteBuffer );
  
    /* Start with an empty string. */
    pcWriteBuffer[0] = 0x00;

 
    usbd_mid_test_uninit();

    return pdFALSE;
}



#if defined( CONFIG_MODULE_USBD_MSC_CLASS )
static BaseType_t prvUsbdSDPlugOutCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char 	*pcParameter;
    int val;
    BaseType_t 	xParameterStringLength;
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    //( void ) pcCommandString;
    ( void ) xWriteBufferLen; 
    configASSERT( pcWriteBuffer );
  
    /* Start with an empty string. */
    pcWriteBuffer[0] = 0x00;

    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      1,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                  );

    configASSERT( pcParameter );  
    val = atoi(pcParameter);

    /* Obtain the parameter string. */
    usbd_mid_set_plugout(val);

    return pdFALSE;
}

static BaseType_t prvUsbdSDReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{

  const char  *pcParameter;
  char *buf;
  BaseType_t  xParameterStringLength;
  int         addr = 0, i, j, len = 512;
      
  /* Remove compile time warnings about unused parameters, and check the
  write buffer is not NULL.  NOTE - for simplicity, this example assumes the
  write buffer length is adequate, so does not check for buffer overflows. */
( void ) pcCommandString;
( void ) xWriteBufferLen; 
configASSERT( pcWriteBuffer );
  
  /* Start with an empty string. */
  pcWriteBuffer[0] = 0x00;
  
  pcParameter = FreeRTOS_CLIGetParameter
                  (
                      pcCommandString,        /* The command string itself. */
                      1,                      /* Return the first parameter. */
                      &xParameterStringLength /* Store the parameter string length. */
                  );
  addr = atoi(pcParameter);

   if(( buf = pvPortMalloc(len))==NULL)  {
    printf("alloc buf fail\r\n");
    return pdFALSE;
   }
   memset(buf, 0, len);
  if(mid_sd_read(buf, addr, len>>9, MID_SD_BLOCK, NULL)!= MID_SD_QUEUE_FINISH)  {
      printf("ret != MID_SD_QUEUE_FINISH\r\n");
      return pdFAIL;
  }
  printf("addr = %x\r\n", addr);
  for(i=0;i<(len>>4);i++) {
    printf("\r\n%02x\t", i);
    for(j=0;j<16;j++)
        printf("%02x ", buf[i*16+j] );
  }  
  vPortFree(buf);
  
    return pdFALSE;
}

#endif
