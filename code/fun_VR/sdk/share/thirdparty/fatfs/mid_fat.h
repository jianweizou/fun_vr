/**
 * @file
 * this is middleware FatFs header file, include this file before use
 * @author Algorithm Dept Sonix.
 */
 
#ifndef __MID_FAT_H__ 
#define __MID_FAT_H__ 

#include <stdint.h>
#include "ff.h"

enum
{
	CMD_FS_LS=0,
	CMD_FS_RM,
	CMD_FS_DU,
};

typedef struct cmd_fs_ls
{
	uint8_t cmd;	
	int 		option;
	int			drive_type;	
	int 		*pret;	
	char 		*drive_path;
	FILLIST **ppfinfo; 
}cmd_fs_ls_t;

void mid_fat_ls(int drive);

#endif	//__MID_FAT_H__
