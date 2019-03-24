/**
 * @file
 * this is middleware SD file 
 * mid_fat.c
 * @author 
 */


#include <FreeRTOS.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sglib.h>
#include "ff.h"
#include "diskio.h"
#include "mid_fat.h"		


uint8_t 		task_work = 0;
cmd_fs_ls_t ls_parm;
FATFS 			*pFatWorkSpace[_VOLUMES] = {NULL};
FATFS 			*pFatWorkSpace_t = NULL;

void task_fs_cmd(void *pvParameters);

extern FRESULT sync_fs (FATFS* fs);		   
extern volatile int fatfs_res;

#define NAME_COMPARATOR(e1, e2) fs_name_cmp(e1->finfo, e2->finfo)
#define TIME_COMPARATOR(e1, e2) fs_rtime_cmp(e1->finfo, e2->finfo)

void fs_add_path(char *path, char *item)
{
	int pathlen = strlen(path);
	if((pathlen && item[0] == '/') || (pathlen>=2 && item[1] == ':'))	//absolute path
		strcpy(path, item);
	else if(strcmp(item, ".")!=0)
	{
		strcat(path, "/");
		strcat(path, item);
	}
}

void fs_drive_num_to_path(char *path, uint8_t drive_num)
{
	if(drive_num > 10)
	{
		FF_DBG("%s: drive number(%d) overflow\n", __func__, drive_num);
	}	
	path[0] = (drive_num%10) + '0';
	strcpy(path+1, ":");
}

/**
* @brief interface function - mount a filesystem and create a working area
* @param drive_num 1:SD card  2:USB
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_mount(BYTE drive_num, BYTE usb_device_id)
{
	FRESULT ret;
	char 		path[5] = {0};
	uint8_t	mount_index = 0;
	
	init_ff();
	
	mount_index = drive_num-1;	
	if(pFatWorkSpace[mount_index] != NULL)
	{	
		vPortFree(pFatWorkSpace[mount_index]);
		printf("drive number(%d) has already mounted\n", mount_index);
		return FR_OK;
	}	
		
	if( drive_num == DEV_USB ){
		if (pFatWorkSpace_t != NULL) {
			vPortFree(pFatWorkSpace_t);
			pFatWorkSpace_t = NULL;
		}

#if defined( CONFIG_SN_GCC_SDK )
		pFatWorkSpace_t = pvPortMalloc((sizeof(FATFS)+512), GFP_KERNEL, MODULE_MID_FATFS);
#else
		pFatWorkSpace_t = pvPortMalloc((sizeof(FATFS)+512));
#endif		

		if (pFatWorkSpace_t == NULL) {
			printf("fatfs buf malloc fail !! \n");
			return FR_INVALID_DRIVE;
		}		
		pFatWorkSpace[mount_index] = (FATFS *)( (((unsigned int) pFatWorkSpace_t) + 512) &(0xFFFFFE00));
	}
	else {
#if defined( CONFIG_SN_GCC_SDK )
		pFatWorkSpace[mount_index] = pvPortMalloc(sizeof(FATFS), GFP_KERNEL, MODULE_MID_FATFS);
#else
		pFatWorkSpace[mount_index] = pvPortMalloc(sizeof(FATFS));
#endif		
	}
	
	if(pFatWorkSpace[mount_index] == NULL)
		return FR_INT_ERR;
	
	memset(pFatWorkSpace[mount_index], 0, sizeof(FATFS) );
	
	//mount workspace
	fs_drive_num_to_path(path, drive_num);
	pFatWorkSpace[mount_index]->usb_device_id = usb_device_id;
	ret = f_mount(pFatWorkSpace[mount_index], path ,1);
	if(ret != FR_OK)	
	{
		/*if sd fast remove error, these free action will be done by fs_cmd_umount*/
		printf("fs_cmd_mount : mount fail (%d)\n", ret);
		if( drive_num == DEV_USB )
		{
			vPortFree(pFatWorkSpace_t);
			pFatWorkSpace_t = NULL;
		}
		else
		{
			vPortFree(pFatWorkSpace[mount_index]);
			pFatWorkSpace[mount_index] = NULL;
		}
	}	
	
	return ret;
}


/**
* @brief interface function - umount a filesystem and release a working area
* @param drive_num 1:SD card 2:USB
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_umount(BYTE drive_num)
{	
	int 	ret;
	int 	umount_index;
	char 	path[5] = {0};
		
	umount_index = drive_num-1;
		
	if( !pFatWorkSpace[umount_index] )
	{
		printf("drive number %d has umount\n", drive_num);
		return FR_OK;
	}
	
	sync_fs(pFatWorkSpace[umount_index]);
	
	fs_drive_num_to_path(path, drive_num);
	ret = f_mount(NULL, path, 0);
		
	//if(pFatWorkSpace[drive_num] != NULL)
	if(pFatWorkSpace[umount_index] != NULL)
	{			
		if( drive_num == DEV_USB ){
			vPortFree(pFatWorkSpace_t);
			pFatWorkSpace_t = NULL;
			pFatWorkSpace[umount_index] = NULL;		
		}
		else{			
			vPortFree(pFatWorkSpace[umount_index]);
			pFatWorkSpace[umount_index] = NULL;
		}
	}	

	pFatWorkSpace[umount_index] = NULL;
	
	return ret;
}

void fs_strlwr(char *str, char *str_lw)
{
	char w, index = 0;
	while((w = *(str+index)) == true)
	{
		if(w>=0x41 && w<=0x5a)	//A-Z	
			w+=0x20;
		*(str_lw+index) = w;
		index ++;
	}
	*(str_lw+index) = *(str+index);
}

int fs_cmp_name(char* fn1, char* fn2)
{
	char *fn_lw1, *fn_lw2;	/* This function assumes non-Unicode configuration */
	int rst=0;
	
#if defined( CONFIG_SN_GCC_SDK )	
	fn_lw1 = pvPortMalloc(strlen(fn1)+1, GFP_KERNEL, MODULE_MID_FATFS);
#else
	fn_lw1 = pvPortMalloc(strlen(fn1)+1);	
#endif	

	if(!fn_lw1)
	{	
		FF_DBG("alloc fn_lw1 fail\n");
		return 0;
	}
#if defined( CONFIG_SN_GCC_SDK )	
	fn_lw2 = pvPortMalloc(strlen(fn2)+1, GFP_KERNEL, MODULE_MID_FATFS);
#else
	fn_lw2 = pvPortMalloc(strlen(fn2)+1);
#endif	

	if(!fn_lw2)
	{	
		FF_DBG("alloc fn_lw2 fail\n");
		vPortFree(fn_lw1);
		return 0;
	}
	fs_strlwr(fn1, fn_lw1);
	fs_strlwr(fn2, fn_lw2);
	rst = strcmp(fn_lw1,fn_lw2);
	vPortFree(fn_lw2);
	vPortFree(fn_lw1);
	return rst;
}

int fs_name_cmp(FILINFO finfo1, FILINFO finfo2)
{
	return fs_cmp_name(GET_FN(finfo1), GET_FN(finfo2));
}

int fs_rtime_cmp(FILINFO finfo1, FILINFO finfo2)
{
	int result;
	//compare last modified date
	result = finfo1.fdate-finfo2.fdate;
	if(result)
		return result;
	//compare last modified time	
	result = finfo1.ftime-finfo2.ftime;
	if(result)
	{
		return result;
	}	
	else
	{
		//compare file name
		return fs_cmp_name(GET_FN(finfo1), GET_FN(finfo2));
	}	

}

static FILLIST* create_flist_item(void)
{
	FILLIST *pfitem = NULL;  
	
#if defined( CONFIG_SN_GCC_SDK )	
	if((pfitem = pvPortMalloc(sizeof(FILLIST), GFP_KERNEL, MODULE_MID_FATFS)) == NULL) {
#else		
	if((pfitem = pvPortMalloc(sizeof(FILLIST))) == NULL) {
#endif		
		return pfitem;
	}
	memset(pfitem, 0, sizeof(FILLIST));

	return pfitem;
}
	

int fs_cmd_ls(FILLIST **finfo, int option, int drive_num)
{
//	FRESULT ret;
	char cur_path[100] = {0x00};
	
	*finfo = NULL;
	fs_drive_num_to_path(cur_path, drive_num);
//	if((ret = f_getcwd(cur_path, 100))!=FR_OK)
//	{
//		printf("fs_cmd_ls : cur_path = %s\n", cur_path);
//		goto fail0;
//	}

	return fs_cmd_ls_by_path(cur_path, finfo, option);
	
//fail0:
//	return ret;	
}

int fs_cmd_ls_by_path(char* path, FILLIST **finfo, int option)
{	
	FRESULT ret;
	DIR dir;
	FILLIST* pitem = NULL;
	FILINFO *pfno;
	//int len=0;

	*finfo = NULL;

	if((ret = f_opendir(&dir, path))!=FR_OK)					   /* Open the directory */
	{
		return ret;
	}

	if((pitem = create_flist_item())==NULL)
	if( pitem == NULL )
	{
		printf("fs_cmd_ls_by_path : pitem is NULL\n");
		ret = FR_NOT_ENOUGH_BUF;
		return ret;
	}

	while((ret = f_readdir(&dir, &pitem->finfo))==FR_OK)
	{
		pfno = &pitem->finfo;		
		if (ret != FR_OK || pfno->lfname[0] == 0) break;  /* Break on error or end of dir */
		if (pfno->fname[0] == '.') continue;			   /* Ignore dot entry */
#if 1			
		printf("%c %10d %4d-%2d-%2d %02d:%02d ", (pfno->fattrib&AM_DIR)?'d':'-', (int)pfno->fsize, 
						(int)FF_YEAR(pfno->fdate), (int)FF_MONTH(pfno->fdate), (int)FF_DATE(pfno->fdate),
						(int)FF_HOUR(pfno->ftime), (int)FF_MINUTE(pfno->ftime));//GET_FN(pitem->finfo));
		printf("%s\r\n", GET_FN(pitem->finfo));//pfno->lfname);
#endif		
		SGLIB_LIST_ADD(FILLIST, *finfo, pitem, next);

		//free space for fail item	
		if( pitem != NULL )
		{
			vPortFree(pitem);
			pitem = NULL;
		}
		
		if( (pitem = create_flist_item()) == NULL )
		{
			printf("fs_cmd_ls_by_path : create_flist_item is NULL\n");
			ret = FR_NOT_ENOUGH_BUF;
			goto fail;
		}
	}

//	SGLIB_LIST_LEN(FILLIST, *finfo, next, len);
//	FF_DBG("%s:total_item = %d\n",__func__,  len);
	
	fs_cmd_flist_sort(finfo, option);

fail:		
	f_closedir(&dir);
	
	return ret;
	
}

void fs_cmd_flist_sort(FILLIST **pflist, int option)
{
	if(option == FF_SORT_BY_NAME)
	{
		SGLIB_LIST_SORT(FILLIST, *pflist, NAME_COMPARATOR, next); 
	}	
	else
	{
		SGLIB_LIST_SORT(FILLIST, *pflist, TIME_COMPARATOR, next); 
	}
}

void fs_cmd_ls_clear(FILLIST **pflist)
{
	FILLIST *pItem, *pNextItem;
	//int len = 0;
	if(*pflist==NULL)
		return;
	
	pItem = *pflist;
	while(pItem)
	{
		pNextItem = pItem->next;
		vPortFree(pItem);
		//len ++;
		pItem = pNextItem;
	}
	vPortFree(pflist);
	pflist = NULL;
	//FF_DBG("%s:total_item = %d\n",__func__,  len);	
}

int fs_cmd_format(BYTE drive_num, BYTE usb_device_id)
{
	int ret;
	char path[10];
	
	fs_drive_num_to_path(path, drive_num);
	
	if((ret = f_mkfs(path, FM_EXFAT, 2048))!=FR_OK)
		printf("format fail(ret = %d)\n", ret);
	else
		printf("format success !!\n");
	
	return ret;
}

unsigned long long fs_cmd_ffdu (char* path)
{
	FRESULT res;
  FILINFO fno;
  DIR dir;
  unsigned long long totfiles_size = 0;
  static char lfn[255 + 1];   /* Buffer to store the LFN */
	
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);

  res = f_opendir(&dir, path);

  if( res == FR_OK )
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno);

      if ((res != FR_OK)||(fno.fname[0] == 0))
				break;
            
      if (fno.fname[0] == '.')
        continue;

      totfiles_size += (fno.fsize);
      //print_msg("%s,  %10lu\n",fno.lfname,fno.fsize);
    } 
    f_closedir(&dir);
  }

  return  (totfiles_size >> 20);  
}

void mid_fat_ls(int drive)
{
	int 				ret;
	int 				option = FF_SORT_BY_NAME;
//	FILLIST 		*pfitem;
	
	ls_parm.cmd = CMD_FS_LS;
	ls_parm.option = option;
	ls_parm.pret= &ret;
	ls_parm.drive_type = drive;
	
#if defined( CONFIG_SN_GCC_SDK )	
	if((ls_parm.ppfinfo=(FILLIST**)pvPortMalloc(sizeof(FILLIST*), GFP_KERNEL, MODULE_MID_FATFS))==NULL) //this **ppfinfo address is =0 ,so add malloc  to avoid 
#else
	if((ls_parm.ppfinfo=(FILLIST**)pvPortMalloc(sizeof(FILLIST*)))==NULL) //this **ppfinfo address is =0 ,so add malloc  to avoid 
#endif	
		return;
	
	task_work = 1;
	if (pdPASS != xTaskCreate(task_fs_cmd, "test_fs_cmd", 256, &ls_parm, 2, NULL))
	{
		printf("Task Create Fail !!\n");
		return;
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
//			cmd_fs_path_t *rm_parm = (cmd_fs_path_t *)pvParameters;
//			*rm_parm->pret= fs_cmd_rm(rm_parm->path);
			break;
		}	
		case CMD_FS_DU:
		{
//			cmd_fs_path_t *du_parm = (cmd_fs_path_t *)pvParameters;
//			*du_parm->pret= fs_cmd_du(du_parm->path, du_parm->psize);
			break;
		}	
	}
	task_work = 0;
		
	vTaskDelete(NULL);
}


