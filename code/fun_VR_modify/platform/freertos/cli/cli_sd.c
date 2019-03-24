
#include "sonix_config.h"

#if defined (CONFIG_CLI_SD)	

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

/* File system includes. */
#include "sd_ctrl.h"
#include "sdv2_sd.h"
#include "mid_sd.h"

#ifdef _WINDOWS_
	#define snprintf _snprintf
#endif

uint32_t	rw_test_count = 0;

/*
 * Print out information on a single file.
 */
//static void prvCreateFileInfoString( char *pcBuffer, F_FIND *pxFindStruct );

/*
 * Copies an existing file into a newly created file.
 */
void 		sd_init_task(void *pvParameters);
void 		sd_rw_test_task(void *pvParameters);
 
static 	BaseType_t prvSDInitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static 	BaseType_t prvSDRWTestCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );									
static 	BaseType_t prvSDMemoryPresentCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );									

static const CLI_Command_Definition_t xSDInit =
{
	"sd_init", /* The command string to type. */
	"\r\nsd_init:\r\n sd initialize <1:SPI mode 2:SD mode>\r\n",
	prvSDInitCommand, /* The function to run. */
	1 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xSDRWTest =
{
	"sd_rwtest", /* The command string to type. */
	"\r\nsd_rwtest:\r\n sd read/write test\r\n",
	prvSDRWTestCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xSDMemoryPresent =
{
	"sd_mem", /* The command string to type. */
	"\r\nsd_mem:\r\n sd memory present\r\n",
	prvSDMemoryPresentCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

/*-----------------------------------------------------------*/

void vRegisterSDCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */	
	FreeRTOS_CLIRegisterCommand( &xSDInit );
	FreeRTOS_CLIRegisterCommand( &xSDRWTest );	
	FreeRTOS_CLIRegisterCommand( &xSDMemoryPresent );	
}
/*-----------------------------------------------------------*/

static BaseType_t prvSDInitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	uint8_t			mode = 0;
		
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
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	mode = atoi(pcParameter);
		
	if( (mode == MS_SD_MODE) || (mode == MS_SPI_MODE) )
	{
#if(OS_ON == 1)		
		xTaskCreate(
			sd_init_task,
			( const char * )"SD_INIT_TASK",
			256,
			mode,
			20,
			NULL
		);
#endif		
	}
	else
		printf("Error Mode!!\r\n");
			
	return pdFALSE;
}

static BaseType_t prvSDRWTestCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
		
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
#if(OS_ON == 1)		
	xTaskCreate(
		sd_rw_test_task,
		( const char * )"SD_TEST_TASK",
		512,
		NULL,
		20,
		NULL
	);
#endif		
				
	return pdFALSE;
}

static BaseType_t prvSDMemoryPresentCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	int					memory_present = SD_MEMORY_UNKNOWN;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
		
	memory_present = sd_get_memory_present();
	if( memory_present == SD_MEMORY_CARD )
		printf("SD_MEMORY_CARD\r\n");
	else if( memory_present == SD_MEMORY_IO )
		printf("SD_MEMORY_IO\r\n");
	else
		printf("SD_MEMORY_UNKNOWN !!\n");
		
	return pdFALSE;
}

void sd_rw_test_task(void *pvParameters)
{
	for(;;)
	{
		rw_test_count++;
		printf("sd_rw_test %d.............................\r\n", rw_test_count);
		mid_sd_rwtest();
		vTaskDelay(100);
	}
}

void sd_init_task(void *pvParameters)
{
	uint8_t 	*mode = (uint8_t *) pvParameters;	
	int				ret = 0;
	int				memory_present = SD_MEMORY_UNKNOWN;
	struct 		sd_info *sd_card_info;
	uint32_t	sd_block_length = 0;
	uint32_t	sd_capacity = 0;
	uint64_t 	sd_size = 0;
	
	sd_init(2);
#if(OS_ON == 1)	
	mid_sd_init();
#endif
	
	memory_present = sd_get_memory_present();
	if( memory_present == SD_MEMORY_CARD )
	{
		printf("SD_MEMORY_CARD\r\n");
		ret = mid_sd_identify(MID_SD_BLOCK, NULL);
		if( ret != MID_SD_QUEUE_FINISH )
		{
			printf("sd_init : SD identify fail\r\n");
		}
		else
		{
			sd_card_info = mid_sd_get_driver_info();
			sd_block_length = sd_card_info->sd_rd_blk_len;
			sd_capacity = sd_card_info->sd_capacity;
			sd_size = ((uint64_t)sd_block_length * (uint64_t)sd_capacity);
			printf("sd_size = %llu bytes\r\n", sd_size);
		}		
	}
	else if( memory_present == SD_MEMORY_IO )
		printf("SD_MEMORY_IO !!\r\n");
	else
		printf("SD_MEMORY_UNKNOWN !!\n");
		
	for(;;)
		vTaskDelay(1000);
}

#endif 	//end of #if defined (CONFIG_CLI_SD)	
