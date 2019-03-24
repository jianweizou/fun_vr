/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "sonix_config.h"

#include <FreeRTOS.h>
#include <string.h>
#include "diskio.h"		/* FatFs lower layer API */

#if defined (CONFIG_MODULE_USB_MSC_CLASS)
#include <USBH.h>
#include <USBH_MSC.h>
#endif

#if defined( CONFIG_PLATFORM_SN7320 )
#include <mid_sd.h>
#endif

#if defined( CONFIG_SN_GCC_SDK )
#include <bsp.h>
#endif

//uint8_t global_status = STA_NOINIT;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE usb_id
)
{	
//	int 		result;
	DSTATUS	status = STA_NOINIT;
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
	USBH_Device_Structure *dev = NULL;
	MSC_DEVICE_STRUCT 		*msc_dev = NULL;
#endif	
	
	switch (pdrv) {
		case DEV_RAM :
			//result = RAM_disk_status();
		
			//translate the reslut code here

			return status;

		case DEV_MMC :
#if defined( CONFIG_PLATFORM_SN7320 )			
		mid_sd_get_card_status(MID_SD_BLOCK, NULL);
		status = RES_OK;
		
//		result = mid_sd_get_card_status(MID_SD_BLOCK, NULL);
//		if(result == MID_SD_QUEUE_FINISH)
//			status = RES_OK;			
//		else if(result == MID_SD_QUEUE_CARD_REMOVED)
//			global_status = STA_NODISK;				
//		else if(result == MID_SD_QUEUE_CARD_WRITEPROTECT)
//			global_status = STA_PROTECT;
//		else
//			status = RES_NOTRDY;
#endif			
			return status;

		case DEV_USB : 
#if defined (CONFIG_MODULE_USB_MSC_CLASS)
			dev = (USBH_Device_Structure*) usbh_msc_init(usb_id);	
			msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV[usb_id];
		
			msc_check_dev_sts(dev, msc_dev);
			if (msc_dev->status == MSC_DISCONNECT) {
				status = RES_NOTRDY;
			}
			else if (msc_dev->status == MSC_ACTIVE) {
				status = RES_OK;
			}
#endif
			return status;		
	}
	
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv,				/* Physical drive nmuber to identify the drive */
	BYTE usb_id
)
{	
	int 		result;
	DSTATUS status = STA_NOINIT;
	
	switch (pdrv) {
		case DEV_RAM :
			//result = RAM_disk_initialize();

			//translate the reslut code here

			return status;
		
		case DEV_MMC :
#if defined( CONFIG_PLATFORM_SN7320 )			
			result = mid_sd_identify(MID_SD_BLOCK, NULL);
			if(result == MID_SD_QUEUE_FINISH)
			{	
				status = RES_OK;
//				global_status &= (~STA_NOINIT); 				
			}	
			else
			{
				status = RES_NOTRDY;			
				FF_DBG("disk_initialize : mid_sd_identify fail !!\n");
			}	
#endif
			
			return status;
				
		case DEV_USB :
#if defined (CONFIG_MODULE_USB_MSC_CLASS)				
			if(msc_ready(usb_id) == SUCCESS)
			{
				status = RES_OK;
			}
			else
			{
				status = RES_NOTRDY;
			}
#endif
		
			return status;
	}

	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE 	pdrv,			/* Physical drive nmuber to identify the drive */
	BYTE 	*buff,		/* Data buffer to store read data */
	DWORD sector,		/* Sector address in LBA */
	UINT 	count,		/* Number of sectors to read */
	BYTE 	usb_id
)
{	
	int 			result;
	DRESULT 	res = RES_ERROR;	
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)					
	USBH_Device_Structure *dev = NULL;
#endif	

	switch (pdrv) {
		case DEV_RAM :
			// translate the arguments here

			//result = RAM_disk_read(buff, sector, count);

			// translate the reslut code here

			return res;
		
		case DEV_MMC : 
#if defined( CONFIG_PLATFORM_SN7320 )			
			result = mid_sd_read(buff, sector, count, MID_SD_BLOCK,NULL);
			if(result == MID_SD_QUEUE_FINISH)
				res = RES_OK;
			else
			{
				res = RES_NOTRDY;
				FF_DBG("disk_read : mid_sd_read fail !!\n");
			}	
#endif
			
			return res;
		
		case DEV_USB :
#if defined (CONFIG_MODULE_USB_MSC_CLASS)			
			dev = (USBH_Device_Structure*) usbh_msc_init(usb_id);
			if (dev == NULL)
				return RES_NOTRDY;
						
			result = msc_read(dev, (uint8_t*) buff, count*512, sector);
			if (result == SUCCESS) { 
				res = RES_OK;
			}
			else {
				FF_DBG ("disk_read : msc_read fail !!\n");
				res = RES_ERROR;	
			}
#endif
			return res;		
	}

	return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT disk_write (
	BYTE pdrv,				/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,			/* Sector address in LBA */
	UINT count,				/* Number of sectors to write */
	BYTE usb_id
)
{
	
	int 			result;
	DRESULT 	res = RES_ERROR;	
	
#if defined (CONFIG_MODULE_USB_MSC_CLASS)					
	USBH_Device_Structure *dev = NULL;
#endif	
	
	switch (pdrv) {
		case DEV_RAM :
			// translate the arguments here

			//result = RAM_disk_write(buff, sector, count);

			// translate the reslut code here

			return res;
		
		case DEV_MMC : 
#if defined( CONFIG_PLATFORM_SN7320 )			
			result = mid_sd_write(sector, (void*)buff, count, MID_SD_BLOCK, NULL);
			if(result == MID_SD_QUEUE_FINISH)
			{
				res = RES_OK;
			}
			else if(result == MID_SD_QUEUE_CARD_REMOVED)
			{
				res = RES_NOTRDY;
				FF_DBG("%s: No SD card\n", __func__);
			}
			else if(result == MID_SD_QUEUE_CARD_WRITEPROTECT)
			{
				res = RES_WRPRT;
				FF_DBG("%s: SD card is write protected", __func__);
			}
			else
			{
				res = RES_ERROR;
				FF_DBG("%s: R/W failed !!", __func__);
			}
#endif			
			
			return res;	
				

		case DEV_USB : 
#if defined (CONFIG_MODULE_USB_MSC_CLASS)			
			dev = (USBH_Device_Structure*) usbh_msc_init(usb_id);
		
			if (dev == NULL) {
				return RES_NOTRDY;
			}
			
//			FF_DBG ("disk_write : buff = 0x%x, count = %d\n", buff, count);			
			result = msc_write(dev, (uint8_t*) buff, count*512, sector);;
			if (result == SUCCESS) { 
				res = RES_OK;
			}
			else {
				FF_DBG ("disk_write : Error !!\n");
				res = RES_ERROR;	
			}		
#endif	
		
		return res;

	}
	
	return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff,		/* Buffer to send/receive control data */
	BYTE usb_id
)
{
	
//	printf("disk_ioctl\n");
	DRESULT res = RES_ERROR;
#if defined (CONFIG_MODULE_USB_MSC_CLASS)			
	int result;
#endif	
	
	switch (pdrv) {
		case DEV_RAM :

			// Process of the command for the RAM drive

			return res;
		
		case DEV_MMC :
#if defined( CONFIG_PLATFORM_SN7320 )			
			// Process of the command for the MMC/SD card
			if(cmd == CTRL_SYNC)
			{
				if(MID_SD_QUEUE_FINISH == mid_sd_sync())
					res = RES_OK;
				else
				{
					res = RES_ERROR;
					FF_DBG("%s: fail\n",__func__);
				}
			}
			else if(cmd == GET_SECTOR_COUNT)
			{				
				if(MID_SD_QUEUE_FINISH == mid_sd_get_capacity( (uint64_t*)buff, MID_SD_BLOCK, NULL))
					res = RES_OK;
				else
				{
					res = RES_ERROR;
					FF_DBG("%s: fail\n",__func__);
				}
				//MID_FF_PRINT_QUEUE("size = %x\n",  *((DWORD*)buff));
			}
#endif
			
			return res;

		case DEV_USB :
#if defined (CONFIG_MODULE_USB_MSC_CLASS)			
			if(cmd == CTRL_SYNC)
			{
				res = RES_OK;
			}
			else if(cmd == GET_SECTOR_COUNT)
			{
				USBH_Device_Structure *dev = NULL;
				MSC_REQ_Struct msc_req;
				memset(&msc_req, 0, sizeof(msc_req));
				msc_req.lba = 0;

				dev = (USBH_Device_Structure*) usbh_msc_init(usb_id);
				if (dev == NULL) {
					return RES_NOTRDY;
				}
						
				result = msc_get_capacity((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req);			
				if(result == SUCCESS) {				
					*(uint32_t *)buff = msc_req.lba;
				
					//FF_DBG_QUEUE("lba = 0x%x\n", msc_req.lba);
					//FF_DBG_QUEUE("block_len = 0x%x\n", msc_req.block_len);
					//FF_DBG_QUEUE("buff = 0x%x\n", buff);
					if(buff > 0)
						res = RES_OK;
					else
						res = RES_ERROR;
				}
				else {
					res = RES_ERROR;
					FF_DBG_QUEUE("%s: fail\n",__func__);
				}
			}
#endif		
		
			return res;		
	}

	return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* reset global status                                                   */
/*-----------------------------------------------------------------------*/
void disk_Reset (void)
{
//	global_status = STA_NOINIT;
}

