/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_fat.h
 *
 * @author Vicky Tseng
 * @version 0.1
 * @date 2018/09/05
 * @license
 * @description
 */

#ifndef __snc_fat_h__
#define __snc_fat_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <semphr.h>
#include "snc_types.h"
#include "snc_fat_int.h"
#include "snc_fat_conf.h"	


/* Type of file size variables */
#if _FS_EXFAT
#if _USE_LFN == 0
#error LFN must be enabled when enable exFAT
#endif
typedef QWORD FSIZE_t;          /* QWORD = 2 DWORDs = 4 WORDs = ….. = 64 bits  */
#else
typedef DWORD FSIZE_t;          /* DWORD = 2 WORDs = 4 bytes = 8 nybbles = 32 bits */
#endif
	
/* Type of path name strings on FatFs API */
#if _LFN_UNICODE								/* Unicode (UTF-16) string */
#if _USE_LFN == 0
	#error _LFN_UNICODE must be 0 at non-LFN cfg.
#endif
#ifndef _INC_TCHAR
	typedef WCHAR TCHAR;
	#define _T(x) L ## x
	#define _TEXT(x) L ## x
#endif
#else														/* ANSI/OEM string */
#ifndef _INC_TCHAR
	typedef char TCHAR;
	#define _T(x) x
	#define _TEXT(x) x
#endif
#endif

//=============================================================================
//                  Constant Definition
//=============================================================================

/* File function return code (FRESULT) */
typedef enum
{
	FR_OK = 0,							/* (0) Succeeded */
	FR_DISK_ERR,						/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,							/* (2) Assertion failed */
	FR_NOT_READY,						/* (3) The physical drive cannot work */
	FR_NO_FILE,							/* (4) Could not find the file */
	FR_NO_PATH,							/* (5) Could not find the path */
	FR_INVALID_NAME,				/* (6) The path name format is invalid */
	FR_DENIED,							/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,								/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,			/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,			/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,				/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,					/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,				/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,				/* (14) The f_mkfs() aborted due to any problem */
	FR_TIMEOUT,							/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,							/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,			/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_LOCK */
	FR_INVALID_PARAMETER,		/* (19) Given parameter is invalid */
	FR_NOT_ENOUGH_BUF,			/* (20) buffer could not be allocated */
	FR_IS_ROOT_DIR,					/* (21) is root directory */
	FR_RES_NOT_INIT					/* (22) res value not init */
} FRESULT;


//=============================================================================
//                  Macro Definition
//=============================================================================
/* Definitions of physical drive number for each drive */
#define DEV_RAM						0
#define DEV_MMC						1
#define DEV_USB						2

/* File access mode and open method flags (3rd argument of f_open) */
#define	FA_READ						0x01
#define	FA_WRITE					0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW			0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30
#define FA__WRITTEN				0x20
#define FA__DIRTY					0x40
#define	FA_SONIX_WRITE		0x80

//=============================================================================
//                  Structure Definition
//=============================================================================

/* File system object structure (FATFS) */
typedef struct
{
	BYTE	fs_type;				/* File system type (0:N/A) */
  BYTE	drv;						/* Physical drive number */
  BYTE	n_fats;					/* Number of FATs (1 or 2) */
  BYTE	wflag;					/* win[] flag (b0:dirty) */
  BYTE	fsi_flag;				/* FSINFO flags (b7:disabled, b0:dirty) */
	BYTE	usb_device_id;
  WORD	id;							/* File system mount ID */
  WORD	n_rootdir;			/* Number of root directory entries (FAT12/16) */
  WORD	csize;					/* Cluster size [sectors] */
#if _MAX_SS != _MIN_SS
  WORD	ssize;					/* Sector size (512, 1024, 2048 or 4096) */
#endif
#if _USE_LFN != 0
  WCHAR*	lfnbuf;				/* LFN working buffer */
#endif
#if _FS_EXFAT
  BYTE*	dirbuf;					/* Directory entry block scratchpad buffer */
#endif
#if _FS_REENTRANT
  _SYNC_t	sobj;					/* Identifier of sync object */
#endif
#if !_FS_READONLY
  DWORD	last_clst;			/* Last allocated cluster */
  DWORD	free_clst;			/* Number of free clusters */
#endif
#if _FS_RPATH != 0
  DWORD	cdir;						/* Current directory start cluster (0:root) */
#if _FS_EXFAT
  DWORD	cdc_scl;				/* Containing directory start cluster (invalid when cdir is 0) */
  DWORD	cdc_size;				/* b31-b8:Size of containing directory, b7-b0: Chain status */
  DWORD	cdc_ofs;				/* Offset in the containing directory (invalid when cdir is 0) */
#endif
#endif
  DWORD	n_fatent;				/* Number of FAT entries (number of clusters + 2) */
  DWORD	fsize;					/* Size of an FAT [sectors] */
  DWORD	volbase;				/* Volume base sector */
  DWORD	fatbase;				/* FAT base sector */
  DWORD	dirbase;				/* Root directory base sector/cluster */
  DWORD	database;				/* Data base sector */
  DWORD	winsect;				/* Current sector appearing in the win[] */
  BYTE	win[_MAX_SS];		/* Disk access window for Directory, FAT (and file data at tiny cfg) */
} FATFS;


/* Object ID and allocation information (_FDID) */
typedef struct
{
	FATFS*	fs;						/* Pointer to the owner file system object */
  WORD	id;							/* Owner file system mount ID */
  BYTE	attr;						/* Object attribute */
  BYTE	stat;						/* Object chain status (b1-0: =0:not contiguous, =2:contiguous (no data on FAT), =3:flagmented in this session, b2:sub-directory stretched) */
  DWORD	sclust;					/* Object start cluster (0:no cluster or root directory) */
  FSIZE_t	objsize;			/* Object size (valid when sclust != 0) */
#if _FS_EXFAT
  DWORD	n_cont;					/* Size of first fragment, clusters - 1 (valid when stat == 3) */
  DWORD	n_frag;					/* Size of last fragment needs to be written (valid when not zero) */
  DWORD	c_scl;					/* Containing directory start cluster (valid when sclust != 0) */
  DWORD	c_size;					/* b31-b8:Size of containing directory, b7-b0: Chain status (valid when c_scl != 0) */
  DWORD	c_ofs;					/* Offset in the containing directory (valid when sclust != 0 and non-directory object) */
#endif
#if _FS_LOCK != 0
  UINT	lockid;					/* File lock ID origin from 1 (index of file semaphore table Files[]) */
#endif
} _FDID;

/* File object structure (FIL) */
typedef struct 
{
	_FDID	obj;						/* Object identifier (must be the 1st member to detect invalid object pointer) */
  BYTE	flag;						/* File status flags */
  BYTE	err;						/* Abort flag (error code) */
	BYTE	err_type;				/* Diskio error type */
  FSIZE_t	fptr;					/* File read/write pointer (Zeroed on file open) */
  DWORD	clust;					/* Current cluster of fpter (invalid when fptr is 0) */
  DWORD	sect;						/* Sector number appearing in buf[] (0:invalid) */
#if !_FS_READONLY
  DWORD	dir_sect;				/* Sector number containing the directory entry */
  BYTE*	dir_ptr;				/* Pointer to the directory entry in the win[] */
#endif
#if _USE_FASTSEEK
  DWORD*	cltbl;				/* Pointer to the cluster link map table (nulled on open, set by application) */
#endif
#if !_FS_TINY
	BYTE	bufnum;
	BYTE*	buf;
#endif
} FIL;


/* File information structure (FILINFO) */
typedef struct
{
	FSIZE_t	fsize;			/* File size */
  WORD	fdate;				/* Modified date */
  WORD	ftime;				/* Modified time */
  BYTE	fattrib;			/* File attribute */
	TCHAR   fname[13];  /* Short file name (8.3 format) */
#if _USE_LFN != 0
	TCHAR	altname[13];	/* Altenative file name */
  TCHAR*  lfname;     /* Pointer to the LFN buffer */
  UINT    lfsize;     /* Size of LFN buffer in TCHAR */
#else
  TCHAR	lfname[13];		/* File name */
#endif
} FILINFO;

typedef struct _FILLIST
{
	FILINFO finfo;
#if _USE_LFN
  TCHAR   lfname[_MAX_LFN + 1];
#endif
  struct _FILLIST *next;
} FILLIST;

typedef struct cmd_fs_ls
{
	uint8_t cmd;	
	int 		option;
	int			drive_type;	
	int 		*pret;	
	char 		*drive_path;
	FILLIST **ppfinfo; 
}cmd_fs_ls_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int 		fs_cmd_mount(BYTE drive_num, BYTE usb_device_id);
int 		fs_cmd_umount(BYTE drive_num);
FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);					/* Open or create a file */
FRESULT f_close(FIL* fp);																				/* Close an open file object */
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);				/* Read data from the file */
FRESULT f_write(FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to the file */
void 		mid_fat_ls(int drive);

#ifdef __cplusplus
}
#endif

#endif
