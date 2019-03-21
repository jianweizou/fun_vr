/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file interface_device.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/07/27
 * @license
 * @description
 */

/* project include */
#include <stdint.h>
#include "interface_device.h"
#include "flash_compatible_fun.h"
#include "bsp.h"

#include "snc_spifc.h"
#include "snc_log_util.h"

/* SPI Flash's information */
stFlashDetail_t *p_flash_detail;

uint32_t init_flash()
{
    uint32_t id = SPIFC_GetFactoryId();
    /* Find the Flash Id in support table, And redirect the PROTECT function.	*/
    p_flash_detail = flash_model_select(id);
    if(p_flash_detail == 0){
        printf("[BLD] Unknow Flash ID: 0x%x \r\n", id);
        printf("\r\n");
        printf("--- Warning! unknow flash may cause unpredictable error! ---\r\n");
        printf("--- Warning! unknow flash may cause unpredictable error! ---\r\n");
//        UEH_Booting(1);
//        while(1);
        return 1;
    }
    else{
        printf("[BLD] Flash ID: 0x%x \r\n", id);
        
        /* Copy Static Structure from ROM space to RAM space */
//        memcpy(&PrjAddrMap, p_flash_detail->pFlashMapType, sizeof(PrjAddrMap));		
//        p_BootInfo = BootInfo_Reload();
    }

    return 0;
}

uint32_t init_uart()
{
    /* #. Initial Log Driver -------------------------------------------------*/
    {
        log_driver_init_t log_set = {
            CONFIG_CPU_CLOCK_MHZ * 1000000,
            115200,
            UART_PORT_0
        };
        
        LOG_Driver_Init(&log_set);
    }
    return 0;
}
    
uint32_t init_usb()
{
    
    return 0;
}



