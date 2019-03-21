/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file main.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/07/06
 * @license
 * @description
 */

/* Sonix Libs include. */
#include "snc_types.h"
#include "snc_log_util.h"
#include "snc_spifc.h"

/* project include */
#include <string.h>
#include "bootloader.h"
#include "user_config.h"
#include "interface_device.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
const address_map_t map_flash = 
{
    .load_table         = 0x60000000,
    .bootloader         = 0x60001000,
    .ap_default_header  = 0x60011000,
    .ap_update_header   = 0x60200000,
    .ap_execute_header  = 0x60200000,
    .boot_info          = 0x603FE000,
    .private_config     = 0x603FF000,
};

const address_map_t map_sdc = 
{
    /* TODO */
};

const address_map_t map_nf = 
{
    /* TODO */
};

const address_map_t *p_map = &map_flash;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
/* Boot Information address in flash */
stBootInfo_t    *p_BootInfo;    

/* Global Buffer */
const uint32_t BUF_SIZE = 4096;
uint8_t buffer[BUF_SIZE] __attribute__((aligned (4)));  //Using DMA: Aligned with 4 Bytes.


//=============================================================================
//                  Structure Definition
//=============================================================================
 struct{
    uint8_t init;
    uint8_t Key;
    uint8_t Peripheral;     /* bit0: None, bit01: SD Card, bit02: USB Host */
    uint8_t	Internal;       /* bit0: None, bit01: OTA Buffer */
    uint8_t TestMode;
}EventList = {0};

//=============================================================================
//                  Main Function Definition
//=============================================================================



//=============================================================================
//                  Public Function Definition
//=============================================================================
typedef uint32_t (*pfunc_t)(void);

uint32_t Init_Bootloader()
{
    pfunc_t init_list[] = 
    {
        init_uart,      
        init_flash,
//        init_usb,
        0,
    };
    
    for(int i=0; init_list[i]; i++)
    {
        if( init_list[i]() )
        {
            return (i + 1);
        }
    }
    
    return 0;
}

uint32_t CheckKeyEvent()
{
    
    return 0;
}

uint32_t CheckPeripheralDev()
{
    
    return 0;
}


uint32_t CheckInternalEvent()
{
    /*Check OTA temp*/
    if(Check_UpdateBuffer() == 1){
        printf("[BLD] OTA Address MD5 Check Correct!\r\n");
        return 1;
    }    
    return 0;
}

uint32_t BootEventHandle()
{
    if(EventList.Internal)
    {
        if(EventList.Internal & (1 << 0))   /* New Firmware address */
        {
            Update_ViaInteralFlash();
            
            /* Erase the OTA's FW Header. */
            if(EventList.TestMode == 0)
                flash_protect_adapter(PROTECT_AREA_CRITICAL);
                memset( ((stFwHeader_t*) p_map->ap_update_header)->Mark, 
                        0, 
                        sizeof(((stFwHeader_t*) p_map->ap_update_header)->Mark)
                      );
                flash_protect_adapter(PROTECT_AREA_ALL);
        }
    }
    return 0;
} 

uint32_t Find_AP_Header(uint32_t storage)
{
    uint32_t ret = 0;
    switch(storage)
    {
        case STORAGE_SPIFC :
            
            ret = memcmp((void*)p_map->ap_update_header, FW_MARK, 8);
            //TODO Check CRC/MP5
        
            if( ret )
            {
                ret = memcmp((void*)p_map->ap_default_header, FW_MARK, 8);
                if(ret)
                {
                    return 0;
                }
                return p_map->ap_default_header;
            }
            else
            {
                return p_map->ap_update_header;
            }
        
        case STORAGE_SDC   :
            break;
        
        case STORAGE_SDIO  :
            break;
        
        case STORAGE_NF    :
            break;
    }
    
    return 0;
}

void RecoveryFirmware(void)
{
    uint32_t Err = 0;
    printf("[BLD] Reset to deafault FW version....\r\n");
    //	UEH_BeforeUpdate(0);

    Err = UpdateFirmware(p_map->ap_execute_header, p_map->ap_default_header);
    if(Err != 0){
    //		UEH_UpdateFail(Err);
        printf("[BLD] Fails to Recovery FW\r\n");
    }else{
    //		UEH_UpdateSuccess(0);
        printf("[BLD] Recovery FW success.\r\n");
    }
}

void Update_ViaInteralFlash(void)
{
    uint32_t Err = 0;

    printf("[BLD] Updating from OTA Flash Address....\r\n");
//    UEH_BeforeUpdate(1);

    Err = UpdateFirmware(p_map->ap_execute_header, p_map->ap_update_header);
    if(Err != 0){
//        UEH_UpdateFail(Err);
        printf("[BLD] Fails to update via OTA\r\n");
    }else{
//        UEH_UpdateSuccess(1);
        printf("[BLD] OTA Update Success\r\n");
    }
}


/*****************************************************************************
* Function      : GetModeTag
* Description   : Check the "user-defined adderss" and return the status.
* Return        : 0xAAAAAAAA - AP mode
                  0x55555555 - Station mode
* Note          : User can define the address and implement the machanism with 
                  another API - spif_config.c 
*****************************************************************************/
uint32_t GetModeTag(stFwHeader_t* pHeader)
{
    extern stFlashDetail_t *p_flash_detail;
    
    stProjectInfo_t *pPrj;
    const uint32_t END_OF_STATUS_REGION = _IC_FLASH_REGION_BASE + p_flash_detail->size - 4;
        
    for(uint32_t i=0; i<SIZE_SECTOR ; i += 4){
        uint32_t *ptr = (uint32_t*)(END_OF_STATUS_REGION - i);

        /* Compare with every tags in the firmware */
        for(uint32_t i=0; i<pHeader->PrjNum; i++){
            pPrj = (stProjectInfo_t*)((uint32_t)pHeader + pHeader->Offset_Prj[i]);
            
            if(*ptr == pPrj->Tag)
                return *ptr;
        }   
    }
    return 0;
}


uint32_t DeviceModeSwitch(stFwHeader_t* p_header )
{
    stProjectInfo_t * pPrj;
    
    if(p_header == 0)
    {
        while(1);
    }
    
    uint32_t tag = GetModeTag(p_header);
    
    /* Find if There's project match the tag */
    for(uint32_t i=0; i<p_header->PrjNum; i++)
    {
        pPrj = (stProjectInfo_t*)((uint32_t)p_header + p_header->Offset_Prj[i]);
        
        if(tag == pPrj->Tag){
            
            printf("[BLD]Status(Tag): 0x%X, switch to mode: %d \r\n", tag, i);
            
            if(i == 0)
            {
                printf("[BLD]Switch to project 0\r\n");
            }
            if(i == 1)
            {
                printf("[BLD]Switch to project 1\r\n");
            }
            
//            UEH_BeforeSwitchMode(i+1);
            Reload_PRAM(p_header, pPrj);
        }
    }
    
    
    /* Task reachs here only if the status does not match any Tags in fw header */
    printf("[BLD]Mode tag [0x%X] is undefined.\r\n", tag);
    printf("[BLD]Now reload default project \r\n");

//    UEH_BeforeSwitchMode(0);

    /* AP mode is Project 1 */
    if(p_header->PrjNum == 1)
        pPrj = (stProjectInfo_t*)((uint32_t)p_header + p_header->Offset_Prj[0]);
    else
        pPrj = (stProjectInfo_t*)((uint32_t)p_header + p_header->Offset_Prj[1]);
    Reload_PRAM(p_header, pPrj);
      
    while(1);
}

stBootInfo_t* BootEvent_AddEvent(stBootInfo_t* ptr, BootFlag data)
{
    const uint32_t MAXIMUN_EVENT_NUM = SIZE_SECTOR / sizeof(stBootInfo_t);
    const uint32_t EVENT_SECTION_END = p_map->boot_info + ((MAXIMUN_EVENT_NUM - 1) * sizeof(stBootInfo_t));
    uint32_t flashNotIdle = 0;
    uint32_t retryCnt = MAXIMUN_EVENT_NUM + 1;
    uint32_t initFlag = 0x12345678;

    while(--retryCnt){
        if(++ptr >= (stBootInfo_t*)EVENT_SECTION_END){
            ptr = (stBootInfo_t*)p_map->boot_info;
            SPIFC_Erase(SPIFC_ERASE_SECTOR, p_map->boot_info, 1);
        }
        
        for(int i=0; i<(sizeof(stBootInfo_t)/4); i++){
            
            /* Check if flash occur error */
            if(*((uint32_t*)ptr + i) != 0xFFFFFFFF){
                flashNotIdle = 1;
                break;
            }
        }
        if(flashNotIdle == 1){
            flashNotIdle = 0;
            continue;
        }
        
        SPIFC_WriteWithDma( SPIFC_WR_MODE_BYTE, 
                            (uint32_t)&ptr->InitFlag, 
                            sizeof(int), 
                            (uint32_t)&initFlag, 
                            1
                          );
        SPIFC_WriteWithDma( SPIFC_WR_MODE_BYTE, 
                            (uint32_t)&ptr->BootFlag, 
                            sizeof(int), 
                            (uint32_t)&data, 
                            1
                          );
        return ptr;
    }
    return 0;
}

/*****************************************************************************
* Function      : Check_MD5
*****************************************************************************/
#include "md5_2.h"
int Check_MD5_OK(uint32_t __HeaderAddr, uint32_t __Size){

    const uint32_t BUFSIZE = 512;
    const uint32_t MD5_BOLCK_SIZE = 64;
    uint32_t md5Temp[2][4]  __attribute__((aligned (4))) = {0};
    uint8_t buffer[BUFSIZE]  __attribute__((aligned (4))) = {0};
    MD5_CTX ctx __attribute__((aligned (4))) = {0};	

    const stFwHeader_t *pHeader = (stFwHeader_t*)__HeaderAddr;

    const uint32_t VerifySize = __Size;

    if(ENABLE_CHECK_MD5 == 0){
        return 1;
    }

    MD5Init(&ctx);

    for(uint32_t i = 0; i < (VerifySize/BUFSIZE); i++){

        uint32_t Offset = (i * BUFSIZE);
        
        SPIFC_ReadWithDma( SPIFC_RD_MODE_FAST, (uint32_t)pHeader + Offset, BUFSIZE, (uint32_t)buffer , 1);
        
        if(i == 0){
            memset(buffer, 0xFF, 32);	//Fixed with Tool
        }
        
        for(uint32_t j = 0 ; j < (BUFSIZE / MD5_BOLCK_SIZE) ; j++ ){
            MD5Update(&ctx, (uint8_t*)buffer  + (j * MD5_BOLCK_SIZE), MD5_BOLCK_SIZE);
        }
    }
    MD5Final(&ctx, (uint8_t*)&md5Temp[0][0]);

    /* Read the MD5 from OTA buffer. */
    SPIFC_ReadWithDma( SPIFC_RD_MODE_FAST, (uint32_t)pHeader->Md5, sizeof(pHeader->Md5), (uint32_t)&md5Temp[1][0] , 1);


    /* Compair MD5 */
    if(memcmp(md5Temp, &md5Temp[1][0], sizeof(pHeader->Md5) != 0)){
        return 1;
    }	
    return 0;
}

/*****************************************************************************
* Function      : Check_UpdateBuffer
* Description   : To check the internal update buffer with Specific Headers 
                  and do the MD5 verification, if the data in update buffer 
                  is correct, return 1, else erase the update buffer's header 
                  and return 0.
* Input         : None
* Output        : None
* Return        : 1- Firmware in internal update buffer is ready to update.
                  0- No firmware in update buffer.
* Note          : None
*****************************************************************************/
int Check_UpdateBuffer(void)
{
    char buffer[8];
    
    const stFwHeader_t *pHeader 	= (stFwHeader_t*)p_map->ap_update_header;

    /* Check if there is a FW header. */
    SPIFC_ReadWithDma(SPIFC_RD_MODE_FAST, p_map->ap_update_header, 8, (uint32_t)buffer, 1);
    
    if(memcmp(FW_MARK, buffer, 8) )
        return 0;

    const stProjectInfo_t* pLastPrj = (stProjectInfo_t*)((uint32_t)pHeader + pHeader->Offset_Prj[pHeader->PrjNum -1]);

    const uint32_t OTA_Length = pLastPrj->Rom[pLastPrj->RomNum - 1].AddrOffset + 
                                pLastPrj->Rom[pLastPrj->RomNum - 1].Size;  

    /* Return True if ignore MD5 Check. */
    if(ENABLE_CHECK_MD5 != 1)
        return 1;


    /* Compair MD5 */
    if(Check_MD5_OK((uint32_t) pHeader,OTA_Length) != 0){

       printf("[BLD]checkSum error\r\n");

        flash_protect_adapter(PROTECT_AREA_CRITICAL);

        SPIFC_Erase(SPIFC_ERASE_SECTOR, p_map->ap_update_header, 1);
        
        flash_protect_adapter(PROTECT_AREA_ALL);
        
//        UEH_UpdateFail(4);
        return 0;
    }

    return 1;
}

__irq void SysTick_Handler()
{

}


int main (void)
{
    /* #.Initial ------------------------------------------------------------*/
    EventList.init          = Init_Bootloader();
    if(EventList.init)
    {
        // fail to initial item.
    }
    
    /* Priority 1 - Key Event detect */
    EventList.Key           = CheckKeyEvent();

    
    /* Priority 2 - Peripheral Device Event Check */
    EventList.Peripheral    = CheckPeripheralDev();

    
    /* Priority 3 - Internal Event Check */
    EventList.Internal      = CheckInternalEvent();

    
    BootEventHandle();

    
    /* #.Find/Parse header form devices ---------------------------------------------*/
    storage_addr addr; 
    
    addr.flash.address = Find_AP_Header( DEFAULT_STORAGE );
    
    /* #.Switch to Specific Device mode -------------------------------------*/
    DeviceModeSwitch( (stFwHeader_t*) addr.flash.address);
    
    /* #.Entering USB ISP Procedure */
    usbd_rom_isp();
    
    while(1); 
    
}
