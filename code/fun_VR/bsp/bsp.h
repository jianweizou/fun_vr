/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file bsp.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/05/31
 * @license
 * @description This is the default settings of the bsp, user can include
 *              include sonix_config.h by define "USE_SONIX_CONFIG",
                The configuration template is in the path:
                    snc_7320_sdk\config\config_template\sonix_config.h
                Copy it to the local file to customize youre project.
 */

/*
 * Include user defined options first. Anything not defined in these files
 * will be set to standard values. Override anything you don't like!
 */

#ifndef __bsp_h__
#define __bsp_h__

#include "bsp_setting.h"
#ifdef USE_SONIX_CONFIG
#include "sonix_config.h" 
#endif

/**********************************
 *  System
 **********************************/

//PLL Source Select
#ifndef CONFIG_USE_EXTERNAL_CLK
#define CONFIG_USE_EXTERNAL_CLK     0
#endif

//Clock
#ifndef CONFIG_CPU_CLOCK_MHZ
#define CONFIG_CPU_CLOCK_MHZ        162
#endif

#ifndef CONFIG_CLK_PRECSALAR
#define CONFIG_CLK_PRECSALAR        0
#endif
 
/**********************************
 *  Memory
 **********************************/

//Flash Status
#ifndef CONFIG_USE_SPI_FLASH
#define CONFIG_USE_SPI_FLASH        1
#endif

//DRAM
#ifndef CONFIG_USE_DRAM
#define CONFIG_USE_DRAM             0
#endif

//I-Cache
#ifndef CONFIG_USE_ICACHE
#define CONFIG_USE_ICACHE           0
#endif




#endif
