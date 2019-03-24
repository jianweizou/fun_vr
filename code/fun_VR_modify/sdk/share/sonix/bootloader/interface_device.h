/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file interface_device.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/07/27
 * @license
 * @description
 */

#ifndef __interface_device_h__
#define __interface_device_h__

/* project include */
#include <stdint.h>

typedef union
{
    struct {
        uint32_t address;
    }flash;
    
    struct {
        uint16_t sector;
        uint16_t byte;
    }sdc;
    
    struct {
        uint32_t address;
    }nand;
}storage_addr;

uint32_t init_usb(void);
uint32_t init_flash(void);
uint32_t init_uart(void);
#endif


