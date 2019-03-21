#include <string.h>
#include "snc_types.h"
#include "bootloader.h"
#include "snc_log_util.h"
#include "snc_spifc.h"

/* reference from main */
extern const address_map_t *p_map;
extern stBootInfo_t     *p_BootInfo;    
extern const uint32_t   flash_code_mark;
extern const uint32_t   BUF_SIZE;
extern uint8_t          buffer[];


/*****************************************************************************
* Function      : UpdateFirmware
* Description   : Check the header and update the exeuction space from others 
                  flash address.
* Input         : DestHeaderAddr- The destination address to update in flash.
                  SourceHeaderAddr- The source address of update data in flash
* Output        : None
* Return        : 0- Update sucess.
                  1- Update process fails.
* Note          : Only use in the situation flash to flash.
*****************************************************************************/
uint8_t UpdateFirmware(uint32_t DestHeaderAddr, uint32_t SourceHeaderAddr){

    uint32_t Err = 0;
    uint8_t *pSource = (uint8_t *)SourceHeaderAddr;
    uint8_t *pDest   = (uint8_t *)DestHeaderAddr;
    stFwHeader_t * pFw = (stFwHeader_t *)pSource;

    const stProjectInfo_t* pLastPrj = (stProjectInfo_t*)((uint32_t)pFw + pFw->Offset_Prj[pFw->PrjNum -1]);

    const uint32_t SIZE_UPDATE = pLastPrj->Rom[pLastPrj->RomNum - 1].AddrOffset +
                                 pLastPrj->Rom[pLastPrj->RomNum - 1].Size;


    /* -- Check the update unique Mark form the source's header ------------- */
    if(memcmp(((stFwHeader_t*) pSource)->Mark, FW_MARK,8) != 0){
        printf("[BLD] Source header mark not match, update cancel.\r\n");
        return UpdateErr_Mark;
    }

    /* Unlock the flash */
    flash_protect_adapter(PROTECT_AREA_CRITICAL);

    /* -- Set booloader updating flag --------------------------------------- */
    p_BootInfo = BootEvent_AddEvent(p_BootInfo,UPDATING);

    /************************************************************************/
    /* -- Erase All the Excuttion FW -------------------------------------- */
    /************************************************************************/
    for(int i = 0; i < (SIZE_UPDATE / SIZE_SECTOR); i++){
        SPIFC_Erase(SPIFC_ERASE_SECTOR, (uint32_t)pDest + i*SIZE_SECTOR, 1);    
    }

    /************************************************************************/
    /* -- Write FW buf to excution buf ------------------------------------ */
    /************************************************************************/
    for (uint32_t savePos = SIZE_HEADER; savePos < SIZE_UPDATE; savePos += BUF_SIZE){

        /* -- READ data out From SPI Flash source address and save to RAM buffer */
        
        SPIFC_ReadWithDma(  SPIFC_RD_MODE_FAST,
                            (uint32_t)pSource + savePos, 
                            BUF_SIZE,
                            (uint32_t)buffer ,1 
                         );
        
        /* -- WRITE data to SPI Flash destination */
        
        SPIFC_WriteWithDma(  SPIFC_WR_MODE_BYTE,
                            (uint32_t)pDest + savePos, 
                            BUF_SIZE,
                            (uint32_t)buffer ,1 
                         );
        
        /* -- Varify the SPI Flash -- */
        for(uint32_t index = 0; index < BUF_SIZE; index++){
            if(*(pDest + savePos + index) != buffer[index]){
                printf("[BLD] Function: %s Verify fail at offset: 0x%x\r\n",__FUNCTION__,index + savePos);
                printf("[BLD] Data in Flash: 0x%x, Data in RAM: 0x%x\r\n",*(pSource + savePos + index),buffer[index]);
                printf("[BLD] Function: %s Fail.!\r\n",__FUNCTION__);
                Err = UpdateErr_VerifyData;
                goto EXE_FW_UPDATE_FAIL;
            }
        }
    }
            
    /************************************************************************/
    /* Copy the Header at last                                              */
    /************************************************************************/
    SPIFC_ReadWithDma(  SPIFC_RD_MODE_FAST,
                        (uint32_t)pSource, 
                        SIZE_HEADER,
                        (uint32_t)buffer ,1 
                     );
    
    /* -- WRITE data to SPI Flash destination */
    SPIFC_WriteWithDma(  SPIFC_WR_MODE_BYTE,
                        (uint32_t)pDest, 
                        SIZE_HEADER,
                        (uint32_t)buffer ,1 
                      );

    if(memcmp(pSource, pDest, SIZE_HEADER) != 0){
        printf("[BLD] Function: %s Fails to copy FW Header!\r\n",__FUNCTION__);
        Err = UpdateErr_CopyHeader;
        goto EXE_FW_UPDATE_FAIL;
    }

    /* Update Success */
    p_BootInfo = BootEvent_AddEvent(p_BootInfo, UPDATE_OK);
    flash_protect_adapter(PROTECT_AREA_ALL);
    return 0;

    /* Error Handling */
    EXE_FW_UPDATE_FAIL:
    
    p_BootInfo = BootEvent_AddEvent(p_BootInfo, UPDATE_ERROR);
    if(SourceHeaderAddr != p_map->ap_default_header)
    {
        SPIFC_Erase(SPIFC_ERASE_SECTOR, (uint32_t)pSource, 1); 
    }
    SPIFC_Erase(SPIFC_ERASE_SECTOR, (uint32_t)pDest, 1); 

    flash_protect_adapter(PROTECT_AREA_ALL);
    return Err;
}



///*****************************************************************************
//* Function		: UpdateFirmwareFromFAT
//* Description	: Check the header and update the exeuction space from FAT 
//				  system's firmware.
//* Input			: DestHeaderAddr- The destination address to update in flash.
//				  SourceHeaderAddr- The source file's full path in FAT system.
//* Output		: None
//* Return		: 0- Update sucess.
//				  1- Update process fails.
//* Note			: Only use in the situation from FAT file to flash.
//*****************************************************************************/
//uint8_t UpdateFirmwareFromFAT(uint32_t DestHeaderAddr, char *fileName){

//    uint32_t Err = 0;
//    uint8_t *pDest   = (uint8_t *)DestHeaderAddr;	
//    uint32_t SIZE_UPDATE;
//    stFwHeader_t HeaderTemp;

//    printf("%s: Des: 0x%X, File: %s\r\n",__FUNCTION__, DestHeaderAddr, fileName);

//    FS_FILE *pFile = FS_FOpen(fileName, "rb" );
//    if(pFile == 0){
//        printf("[BLD] File: %s Not Found, update cancel.\r\n", fileName);
//        return UpdateErr_InvaildSource;
//    }
//    else{
//        SIZE_UPDATE = pFile->Size;
//    }
//    if(fat_read4K((uint16_t*)buffer, pFile) == 1){
//        printf("[BLD] Function: %s Read data error \r\n",__FUNCTION__);
//        Err = UpdateErr_FAT_ReadFail;
//        goto FAT_UPDATE_FAIL;
//    }
//    memcpy(&HeaderTemp, buffer, sizeof(HeaderTemp));
//        
//    /* -- Check the update unique Mark form the source's header ------------- */
//    if(memcmp(HeaderTemp.Mark, FW_MARK,8) != 0){
//        printf("[BLD] Source header mark not match, update cancel.\r\n");
//        Err = UpdateErr_Mark;
//        goto FAT_UPDATE_FAIL;
//    }

//    /* Unlock the flash */
//    flash_protect_adapter(PROTECT_AREA_CRITICAL);

//    /************************************************************************/
//    /* -- Erase All the Excuttion FW -------------------------------------- */
//    /************************************************************************/
//    for(int i = 0; i < (SIZE_UPDATE / SIZE_SECTOR); i++){
//        SPIFC_Sector_Erase((uint32_t)pDest + i*SIZE_SECTOR);	
//        while(SPIFC_Erase_Data_Polling()){}
//    }

//    /************************************************************************/	
//    /* -- Write FW buf to excution buf ------------------------------------ */
//    /************************************************************************/
//    for (uint32_t offset = SIZE_HEADER; offset < SIZE_UPDATE; offset += BUF_SIZE){

//        /* -- READ data out From Card */
//        if(fat_read4K((uint16_t*)buffer, pFile) == 1){
//            printf("[BLD] Function: %s Read data error \r\n",__FUNCTION__);
//            Err = UpdateErr_FAT_ReadFail;
//            goto FAT_UPDATE_FAIL;
//        }
//                
//        /* Note: when update from peripheral device to SPI Flash, there's no need  */
//        /* to Add the tail's mark on it */
//            
//        /* -- WRITE data to SPI Flash destination */
//        SPIFC_Write_DMA_Mode(3,	((uint32_t)pDest + offset),
//                            BUF_SIZE, (uint32_t*)buffer);
//        while(SPIFC_Write_Data_Polling());
//        
//        /* -- Varify the SPI Flash -- */
//        for(uint32_t index = 0; index < BUF_SIZE; index++){
//            if(*(pDest + offset + index) != buffer[index]){
//                printf("[BLD] Function: %s Verify fail at offset: 0x%x\r\n",__FUNCTION__,index + offset);
//                printf("[BLD] Data in Flash: 0x%x, Data in RAM: 0x%x\r\n",*(buffer + offset + index),buffer[index]);
//                printf("[BLD] Function: %s Fail.!\r\n",__FUNCTION__);
//                Err = UpdateErr_VerifyData;
//                goto FAT_UPDATE_FAIL;
//            }	
//        }
//    }
//        
//    if(pFile->FilePos != SIZE_UPDATE){
//        printf("[BLD] File size error, cur: 0x%X, expected: 0x%X", pFile->FilePos, pFile->Size);
//        Err = UpdateErr_SizeNotMatch;
//        goto FAT_UPDATE_FAIL;
//    }

//    /************************************************************************/
//    /* Copy the Header at last 												*/
//    /************************************************************************/
//    FS_FSeek(pFile,0,FS_SEEK_SET);		/* Move the handle to the begainnig of file */

//    /* -- READ data out From SD card */
//    if(fat_read4K((uint16_t*)buffer, pFile) == 1){
//        printf("[BLD] Function: %s Read data error \r\n",__FUNCTION__);
//        Err = UpdateErr_FAT_ReadFail;
//        goto FAT_UPDATE_FAIL;
//    }

//    /* -- WRITE data to SPI Flash destination */
//    SPIFC_Write_DMA_Mode(3,	((uint32_t)pDest),	SIZE_HEADER, (uint32_t*)buffer);
//    while(SPIFC_Write_Data_Polling());

//    if(memcmp(buffer, pDest, SIZE_HEADER) != 0){
//        printf("[BLD] Function: %s Fails to copy FW Header!\r\n",__FUNCTION__);
//        Err = UpdateErr_CopyHeader;
//        goto FAT_UPDATE_FAIL;
//    }
//    p_BootInfo = BootEvent_AddEvent(p_BootInfo, NEW_FW_READY);

//    flash_protect_adapter(PROTECT_AREA_ALL);
//    return 0;

//    /* Error Handling */
//    FAT_UPDATE_FAIL:
//    FS_FClose(pFile);
//    SPIFC_Sector_Erase((int32_t)pDest);
//    while(SPIFC_Erase_Data_Polling());
//    flash_protect_adapter(PROTECT_AREA_ALL);
//    return Err;
//}

