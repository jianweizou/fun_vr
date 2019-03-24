
#include "sonix_config.h"

#if defined (CONFIG_CLI_FATFS)	

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
#include "ff.h"
#include "sd_ctrl.h"

#ifdef _WINDOWS_
	#define snprintf _snprintf
#endif

#define	MAX_DATA_SIZE		(MAX_SRAM_BUF_SIZE/2+4096)
#define	RW_DATA_SIZE		(MAX_SRAM_BUF_SIZE/2)

enum{
	CMD_FS_LS=0,
	CMD_FS_RM,
	CMD_FS_DU,
};

typedef struct cmd_fs_ls{
	uint8_t cmd;	
	int 		option;
	int			drive_type;	
	int 		*pret;	
	char 		*drive_path;
	FILLIST **ppfinfo; 
}cmd_fs_ls_t;

typedef struct cmd_fs_path{
	uint8_t cmd;
	char *path; 
	unsigned long *psize;
	int *pret;
}cmd_fs_path_t;


uint8_t 		task_work = 0;
uint32_t		sd_burning_count = 0;
cmd_fs_ls_t ls_parm;

void cli_sd_burning_task(void *pvParameters);
void task_fs_cmd(void *pvParameters);
	
/*
 * Print out information on a single file.
 */
//static void prvCreateFileInfoString( char *pcBuffer, F_FIND *pxFindStruct );

/*
 * Copies an existing file into a newly created file.
 */
static BaseType_t prvPerformCopy( const char *pcSourceFile,
									int32_t lSourceFileLength,
									const char *pcDestinationFile,
									char *pxWriteBuffer,
									size_t xWriteBufferLen );

static BaseType_t prvMountCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvUMountCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvLSCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvWriteCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvCopyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvFormatCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvSDBurningCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
									
static const CLI_Command_Definition_t xMount =
{
	"mount", /* The command string to type. */
	"\r\nmount:\r\n mount a file system <drive num - 1:SD 2:USB> <usb device id. SD default is 0>\r\n",
	prvMountCommand, /* The function to run. */
	2 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xUMount =
{
	"umount", /* The command string to type. */
	"\r\numount:\r\n unmount file system <drive num - 1:SD 2:USB>\r\n",
	prvUMountCommand, /* The function to run. */
	1 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xLS =
{
	"ls", /* The command string to type. */
	"\r\nls:\r\n Lists the files in the current directory <drive num - 1:SD 2:USB>\r\n",
	prvLSCommand, /* The function to run. */
	1 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xWrite =
{
	"wf", /* The command string to type. */
	"\r\nwf:\r\n write data to file <drive num - 1:SD 2:USB> <file name> <data pattern> <data size>\r\n",
	prvWriteCommand, /* The function to run. */
	4 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xRead =
{
	"rf", /* The command string to type. */
	"\r\nrf:\r\n read file <drive num - 1:SD 2:USB> <file name> <read size>\r\n",
	prvReadCommand, /* The function to run. */
	3 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xCopy =
{
	"cp", /* The command string to type. */
	"\r\ncp:\r\n copy file <src file> <dest file>\r\n",
	prvCopyCommand, /* The function to run. */
	2 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xFormat =
{
	"fm", /* The command string to type. */
	"\r\nfm:\r\n format <drive num - 1:SD 2:USB> <usb device id. SD default is 0>\r\n",
	prvFormatCommand, /* The function to run. */
	2 /* No parameters are expected. */
};

static const CLI_Command_Definition_t xSDBurning =
{
	"sd_burn", /* The command string to type. */
	"\r\nsd_burn:\r\n SD Burning Test <file name>\r\n",
	prvSDBurningCommand, /* The function to run. */
	1 /* No parameters are expected. */
};
/*-----------------------------------------------------------*/

void vRegisterFatFsCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */
	FreeRTOS_CLIRegisterCommand( &xMount );
	FreeRTOS_CLIRegisterCommand( &xUMount );
	FreeRTOS_CLIRegisterCommand( &xLS );
	FreeRTOS_CLIRegisterCommand( &xWrite );
	FreeRTOS_CLIRegisterCommand( &xRead );
	FreeRTOS_CLIRegisterCommand( &xCopy );
	FreeRTOS_CLIRegisterCommand( &xFormat );
	FreeRTOS_CLIRegisterCommand( &xSDBurning );
}
/*-----------------------------------------------------------*/

void task_fs_cmd(void *pvParameters)
{	
	uint8_t *cmd = (uint8_t *) pvParameters;	
	switch(*cmd)
	{
		case CMD_FS_LS:
		{
			cmd_fs_ls_t *ls_parm = (cmd_fs_ls_t *)pvParameters;
			*ls_parm->pret = fs_cmd_ls(ls_parm->ppfinfo, ls_parm->option, ls_parm->drive_type);
			break;
		}	
		case CMD_FS_RM:	
		{
			cmd_fs_path_t *rm_parm = (cmd_fs_path_t *)pvParameters;
//			*rm_parm->pret= fs_cmd_rm(rm_parm->path);
			break;
		}	
		case CMD_FS_DU:
		{
			cmd_fs_path_t *du_parm = (cmd_fs_path_t *)pvParameters;
//			*du_parm->pret= fs_cmd_du(du_parm->path, du_parm->psize);
			break;
		}	
	}
	task_work = 0;
		
	vTaskDelete(NULL);
}

static BaseType_t prvMountCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	int 				drive;
	int 				ret;
	uint8_t 		usb_device_id = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	/* Obtain the parameter string. */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	drive = atoi(pcParameter);
	
	
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						2,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	usb_device_id = atoi(pcParameter);
		
	ret = fs_cmd_mount(drive, usb_device_id);
	if( ret != FR_OK )
		printf("mount fail(%d) !!\n", ret);
	else
		printf("mount success !!\n");
		
	return pdFALSE;
}

static BaseType_t prvUMountCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	int 				drive;
	int 				ret;
	
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
	drive = atoi(pcParameter);
	
	ret = fs_cmd_umount(drive);
	
	if(ret != FR_OK)
		printf("umount fail (%d) !!\n", ret);
	else
		printf("umount success !!\n");
	
	return pdFALSE;
}

static BaseType_t prvLSCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	int 				drive;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;	
	
	/* Obtain the parameter - drive type */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	drive = atoi(pcParameter);
	
	cli_fatfs_ls(drive);
	
	return pdFALSE;
}

void cli_fatfs_ls(int drive)
{
	int 				ret;
	int 				option = FF_SORT_BY_NAME;
	FILLIST 		*pfitem;
	
	ls_parm.cmd = CMD_FS_LS;
	ls_parm.option = option;
	ls_parm.pret= &ret;
	ls_parm.drive_type = drive;
	
#if defined( CONFIG_SN_GCC_SDK )	
	if((ls_parm.ppfinfo=(FILLIST**)pvPortMalloc(sizeof(FILLIST*), GFP_KERNEL, MODULE_MID_FATFS))==NULL) //this **ppfinfo address is =0 ,so add malloc  to avoid 
#else
	if((ls_parm.ppfinfo=(FILLIST**)pvPortMalloc(sizeof(FILLIST*)))==NULL) //this **ppfinfo address is =0 ,so add malloc  to avoid 
#endif	
		return 0;
	
	task_work = 1;
	if (pdPASS != xTaskCreate(task_fs_cmd, "test_fs_cmd", 256, &ls_parm, 2, NULL))
	{
		printf("Task Create Fail !!\n");
		return 0;
	}
	//wait for task finish
	while(task_work) 
		vTaskDelay( 10 / portTICK_RATE_MS );

//	if(*ls_parm.pret == FR_OK && (*ls_parm.ppfinfo)!=NULL)
//	{
//		pfitem = *ls_parm.ppfinfo;
//		while(pfitem)	
//		{
//			FILINFO *pfno = &pfitem->finfo;
//			
//			printf("%c %10ld %4d-%02d-%02d %2d:%02d", 
//					(pfno->fattrib&AM_DIR)?'d':'-', pfno->fsize, 
//					FF_YEAR(pfno->fdate), FF_MONTH(pfno->fdate), FF_DATE(pfno->fdate),
//					FF_HOUR(pfno->ftime), FF_MINUTE(pfno->ftime));
//			printf(" %s\r\n", GET_FN(pfitem->finfo));
//			pfitem = pfitem->next;
//		}	
//	}	
	fs_cmd_ls_clear(ls_parm.ppfinfo);
	ls_parm.ppfinfo = NULL;
	if(ret != FR_OK)
		printf("ls fail (%d) !!\n", ret);
}

static BaseType_t prvWriteCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	FIL 				file;
	int 				drive;	
	int 				ret;
	char 				fileName[24];
	char 				driveFileName[24];
	char 				dataPattern[4];
	char 				*dataBuffer;	
	uint32_t 		totalWriteSize = 0;
	uint32_t		i = 0;
	uint32_t		writeCount = 0;	
	uint32_t		writeSize = 0;	
	uint32_t		writedBytes = 0;	
	uint32_t		addr4K = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	/* Obtain the parameter - drive type */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	drive = atoi(pcParameter);
	
	/* Obtain the parameter - file name */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						2,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);	
	configASSERT( pcParameter );
	strncpy(fileName, pcParameter, xParameterStringLength);	
	fileName[xParameterStringLength] = 0x00;
		
	/* Obtain the parameter - data pattern */
	pcParameter = ( char * )FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						3,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);	
	configASSERT( pcParameter );	
	strncpy(dataPattern, pcParameter, xParameterStringLength);
	dataPattern[xParameterStringLength] = 0x00;
			
	/* Obtain the parameter - data size */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						4,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	totalWriteSize = atoi(pcParameter);
	if( totalWriteSize < RW_DATA_SIZE )
		totalWriteSize = RW_DATA_SIZE;
		
//	printf("prvWriteCommand : fileName = %s\n", fileName);
//	printf("prvWriteCommand : dataPattern = %s\n", dataPattern);
//	printf("prvWriteCommand : totalWriteSize = %d\n", totalWriteSize);

	dataBuffer = pvPortMalloc(MAX_DATA_SIZE);	
	if( ((uint32_t)dataBuffer % 4096) != 0 )
		addr4K = (((uint32_t)dataBuffer/4096)+1)*4096;
	else
		addr4K = ((uint32_t)dataBuffer);	
	memset(dataBuffer, dataPattern[0], MAX_DATA_SIZE);
	
	writeSize = RW_DATA_SIZE;
	writeCount = totalWriteSize/writeSize;
//	printf("writeCount = %d\n", writeCount);
		
	sprintf(driveFileName, "%d:%s", drive, fileName);
	printf("write %s start\r\n", driveFileName);
	
	if( (ret = f_open(&file, driveFileName, FA_WRITE|FA_OPEN_ALWAYS)) != FR_OK )
	{
		printf("open %s fail (%d) !!\n", driveFileName, ret);
		return 0;
	}
	
	for( i=0; i<writeCount; i++ )
	{
		if( (ret = f_write(&file, (char *)addr4K, writeSize, &writedBytes)) != FR_OK ||
				(writedBytes != writeSize) )
		{
			printf("write %s fail (%d) !!\r\n", fileName, ret);
		}
	}	
	printf("write %d bytes data '%s' to %s completed !!\r\n", totalWriteSize, dataPattern, fileName);
	
	vPortFree(dataBuffer);
	
	f_close(&file);
	
	return pdFALSE;
}

static BaseType_t prvReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	char 				fileName[24];
	FIL 				file;
	int 				ret;
	int 				drive;
	char 				*dataBuffer;
	char 				driveFileName[24];
	uint32_t		i = 0;
	uint32_t		readCount = 0;	
	uint32_t 		totalReadSize;
	uint32_t 		readSize;
	uint32_t 		readedBytes;	
	uint32_t		addr4K = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;	
	
	/* Obtain the parameter - drive type */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	
	drive = atoi(pcParameter);
	
	/* Obtain the parameter - file name */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						2,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);		
	strncpy(fileName, pcParameter, xParameterStringLength);	
	fileName[xParameterStringLength] = 0x00;
	
	/* Obtain the parameter - file size */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						3,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	totalReadSize = atoi(pcParameter);
	
	readSize = RW_DATA_SIZE;
	readCount = totalReadSize/readSize;
//	printf("readCount = %d\n", readCount);
	
	dataBuffer = pvPortMalloc(MAX_DATA_SIZE);
	if( ((uint32_t)dataBuffer % 4096) != 0 )
		addr4K = (((uint32_t)dataBuffer/4096)+1)*4096;
	else
		addr4K = ((uint32_t)dataBuffer);	
	
	sprintf(driveFileName, "%d:%s", drive, fileName);
	printf("read %s start\n", driveFileName);
	if( (ret = f_open(&file, driveFileName, FA_READ|FA_OPEN_EXISTING)) != FR_OK )
	{
		printf("read %s open fail (%d)\n", driveFileName, ret);
		return 0;
	}	
		
	for( i=0; i<readCount; i++ )
	{
//		memset(readBuffer, 0x00, MAX_DATA_SIZE);
		if( (ret = f_read(&file, (char *)addr4K, readSize, &readedBytes)) != FR_OK ||
				(readedBytes != readSize) )
		{
			printf("read data fail (%d)!!\n", ret);
		}
	}
	
	printf("readed %d bytes from %s completed !!\n", totalReadSize, driveFileName);

	f_close(&file);
	
	vPortFree(dataBuffer);
	
	return pdFALSE;
}

static BaseType_t prvCopyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	char 				srcFileName[32];
	char 				destFileName[32];
	FIL 				srcFile;
	FIL 				destFile;
	int 				ret;
	int 				drive;
	char 				*dataBuffer;
	uint32_t		i = 0;
	uint32_t		copySize = 0;	
	uint32_t		totalCopySize = 0;	
	uint32_t 		srcFileSize = 0;
	uint32_t 		readedBytes = 0;
	uint32_t		writedBytes = 0;
	uint32_t		addr4K = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	/* Obtain the parameter - source file name */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);		
	strncpy(srcFileName, pcParameter, xParameterStringLength);	
	srcFileName[xParameterStringLength] = 0x00;
	configASSERT( pcParameter );
	
	/* Obtain the parameter - destination file size */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						2,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	strncpy(destFileName, pcParameter, xParameterStringLength);	
	destFileName[xParameterStringLength] = 0x00;
	
	//open source file
	if( (ret = f_open(&srcFile, srcFileName, FA_READ|FA_OPEN_EXISTING)) != FR_OK )
	{
		printf("open %s fail (%d)\r\n", srcFileName, ret);
		return 0;
	}	
//	else
//	{
//	  printf("srcFileName = %s  open ok\r\n", srcFileName);	
//	}
	
	srcFileSize = f_size(&srcFile);
	totalCopySize = srcFileSize;
	printf("%s file size is %d bytes\r\n", srcFileName, srcFileSize);
	
	//open destination file
	if( (ret = f_open(&destFile, destFileName, FA_WRITE|FA_OPEN_ALWAYS)) != FR_OK )
	{
		printf("open %s fail (%d)\r\n", destFileName, ret);
		return 0;
	}	
//	else
//	{
//		printf("\r\ndestFileName = %s open ok\r\n", destFileName);
//	}
		
	//read source file
	dataBuffer = pvPortMalloc(MAX_DATA_SIZE);
	if( ((uint32_t)dataBuffer % 4096) != 0 )
		addr4K = (((uint32_t)dataBuffer/4096)+1)*4096;
	else
		addr4K = ((uint32_t)dataBuffer);	
		
	do
	{
		if( totalCopySize >= RW_DATA_SIZE )
			copySize = RW_DATA_SIZE;
		else
			copySize = totalCopySize;
		
		//read source data
		memset(dataBuffer, 0x00, MAX_DATA_SIZE);
		if( (ret = f_read(&srcFile, (char *)addr4K, copySize, &readedBytes)) != FR_OK ||
				(copySize != readedBytes) )
		{
			printf("read %s fail (%d)!!\r\n", srcFileName, ret);
			break;
		}
		
		//write to destination file
		if( (ret = f_write(&destFile, (char *)addr4K, copySize, &writedBytes)) != FR_OK ||
				(copySize != writedBytes) )
		{
			printf("write %s fail (%d) !!\r\n", destFileName, ret);
			break;
		}
	
		totalCopySize -= copySize;
		printf("remainder %d bytes ...\r\n", totalCopySize);
		
	} while(totalCopySize > 0);
	
		
	printf("copy %s to %s completed !!\r\n", srcFileName, destFileName);

	f_close(&srcFile);
	f_close(&destFile);
	
	vPortFree(dataBuffer);
	
	return pdFALSE;
}

static BaseType_t prvFormatCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	int 				drive;
	int 				ret;
	uint8_t 		usb_device_id = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	/* Obtain the parameter string. */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	drive = atoi(pcParameter);
	
	
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						2,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);
	usb_device_id = atoi(pcParameter);
		
	ret = fs_cmd_format(drive, usb_device_id);
	if( ret != FR_OK )
		printf("mount fail(%d) !!\r\n", ret);
	else
		printf("mount success !!\r\n");
		
	return pdFALSE;
}

static BaseType_t prvSDBurningCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char 	*pcParameter;
	BaseType_t 	xParameterStringLength;
	char 				srcFileName[32];
		
	/* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
  ( void ) pcCommandString;
  ( void ) xWriteBufferLen;	
  configASSERT( pcWriteBuffer );
	
	
	/* Start with an empty string. */
	pcWriteBuffer[0] = 0x00;
	
	/* Obtain the parameter - source file name */
	pcParameter = FreeRTOS_CLIGetParameter
					(
						pcCommandString,		/* The command string itself. */
						1,						/* Return the first parameter. */
						&xParameterStringLength	/* Store the parameter string length. */
					);		
	strncpy(srcFileName, pcParameter, xParameterStringLength);	
	srcFileName[xParameterStringLength] = 0x00;
	configASSERT( pcParameter );
	
	xTaskCreate(
		cli_sd_burning_task,
		( const char * )"CLI_SD_BURN_TASK",
		256,
		(void *)&srcFileName,
		20,
		NULL
	);
	
	return pdFALSE;
}

void cli_sd_burning_task(void *pvParameters)
{
	int				ret = 0;
	char 			*srcFileName = (char *)pvParameters;	
	char 			destFileName[32];
	FIL 			srcFile;
	FIL 			destFile;
	int 			drive;
	char 			*dataBuffer;
	uint32_t	i = 0;
	uint32_t	copySize = 0;	
	uint32_t	totalCopySize = 0;	
	uint32_t 	srcFileSize = 0;
	uint32_t 	readedBytes = 0;
	uint32_t	writedBytes = 0;
	uint32_t	addr4K = 0;
	
	printf("cli_sd_burning_task : srcFileName = %s\r\n", srcFileName);
	
	//open source file
	if( (ret = f_open(&srcFile, srcFileName, FA_READ|FA_OPEN_EXISTING)) != FR_OK )
	{
		printf("open %s fail (%d)\r\n", &srcFileName, ret);
		vTaskDelete(NULL);
	}
	srcFileSize = f_size(&srcFile);
	totalCopySize = srcFileSize;
	printf("%s file size is %d bytes\r\n", srcFileName, srcFileSize);
		
	//set 4K address
	dataBuffer = pvPortMalloc(MAX_DATA_SIZE);
	if( ((uint32_t)dataBuffer % 4096) != 0 )
		addr4K = (((uint32_t)dataBuffer/4096)+1)*4096;
	else
		addr4K = ((uint32_t)dataBuffer);	
		
	for(;;)
	{
		sd_burning_count++;		
		
		//open destination file
//		sprintf(destFileName, "%s_%d", srcFileName, sd_burning_count);		
		sprintf(destFileName, "1:test_%d.pdf", sd_burning_count);		
		if( (ret = f_open(&destFile, destFileName, FA_WRITE|FA_OPEN_ALWAYS)) != FR_OK )
		{
			printf("open %s fail (%d)\r\n", destFileName, ret);
			break;
		}		
			
		do
		{
			if( totalCopySize >= RW_DATA_SIZE )
				copySize = RW_DATA_SIZE;
			else
				copySize = totalCopySize;
			
			//read source data
//			memset(dataBuffer, 0x00, MAX_DATA_SIZE);
			if( (ret = f_read(&srcFile, (char *)addr4K, copySize, &readedBytes)) != FR_OK ||
					(copySize != readedBytes) )
			{
				printf("%d. read %s fail (%d)!!\n", sd_burning_count, srcFileName, ret);
				break;
			}
			
			//write to destination file
			if( (ret = f_write(&destFile, (char *)addr4K, copySize, &writedBytes)) != FR_OK ||
					(copySize != writedBytes) )
			{
				printf("%d. write %s fail (%d) !!\n", sd_burning_count, destFileName, ret);
				break;
			}
		
			totalCopySize -= copySize;
//			printf("remainder %d bytes ...\r\n", totalCopySize);
			
		} while(totalCopySize > 0);
			
		printf("%d. copy %s to %s completed !!\n", sd_burning_count, srcFileName, destFileName);
	
		f_close(&destFile);
		
		vTaskDelay(100);
	}
	
fail :
	f_close(&srcFile);
	f_close(&destFile);
	vPortFree(dataBuffer);
	vTaskDelete(NULL);
}

#endif	//end of #if defined (CONFIG_CLI_FATFS)	
