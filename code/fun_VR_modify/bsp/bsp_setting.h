#ifndef __bspsetting_H_wdfR99A5_lOAW_HG5a_swbf_uYkNdWroKhtZ__
#define __bspsetting_H_wdfR99A5_lOAW_HG5a_swbf_uYkNdWroKhtZ__

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
//=============================================================================
//                      Version
//=============================================================================

//=============================================================================
// <h>                  System 
//=============================================================================
    // <i> System setting
    // <s.32> Chip ID
    // <i> This is the id of chip
    // <i> Default: "7320"
#define CONFIG_CHIP_ID  "7320"

    // <o0> Package         <0 => SNC7320_LQFT144
    //                      <1 => SNC73200M1NFG_A
    //                      <2 => SNC73200M3NFG_A
    // <i> Default: "SNC7320_LQFT144"

#define CONFIG_CHIP_PACKAGE    0

#if (CONFIG_CHIP_PACKAGE    ==      0 )
    #include ".\series\lqfp144.h"
#elif (CONFIG_CHIP_PACKAGE  ==      1 )
    #include ".\series\00m1nfg_a.h"
#elif (CONFIG_CHIP_PACKAGE  ==      2 )
    #include ".\series\00m3nfg_a.h"
#else
    #error "Please select System->Package in bsp_setting.h."
#endif




#define CONFIG_CHIP_SUC7320_A   1
#define CONFIG_ARM_CM3          1

    // <e> Use External Crystal
    #define CONFIG_USE_EXTANAL_CLK  1
    // </e>

    // <o> CPU Clock
    //    <162=> Default: 162 Mhz
    //    <148=> 148.5 Mhz
    //    <12=>  12 Mhz
    // <i> CONFIG_CPU_CLOCK_MHZ Default: 162
    #define CONFIG_CPU_CLOCK_MHZ    162
    
    // <o> AHB Clock Pre Scalar
    //    <0=> Default: 1
    //    <1=> 1/2
    //    <2=> 1/4
    //    <3=> 1/8
    //    <4=> 1/16
    // <i> CONFIG_CLK_PRECSALAR Default: 1
    #define CONFIG_CLK_PRECSALAR    0



#define CONFIG_1_US_TICK      (CONFIG_CPU_CLOCK_MHZ)
#define CONFIG_1_MS_TICK      (CONFIG_CPU_CLOCK_MHZ * 1000)

// </h>

//=============================================================================
// <h>                  Memory Configuration
//=============================================================================

// <e> Enable SPI NOR-Flash
// <i> CONFIG_USE_SPI_FLASH
#define CONFIG_USE_SPI_FLASH    1
#if (CONFIG_USE_SPI_FLASH)
    // <o> SPI Flash Size
    //    <0x80000  => 512KB
    //    <0x100000 => 1MB
    //    <0x200000 => 2MB
    //    <0x400000 => 4MB
    //    <0x800000 => 8MB
    //    <0x1000000=> 16MB
    // <i> CONFIG_FLASH_SIZE Default: 0x400000 (hex)
    #define CONFIG_FLASH_SIZE   0x400000
#endif
// </e>

//DRAM setting -> depends on package
#if (CONFIG_DRAM_TYPE == 0)
    #define CONFIG_USE_DRAM         0
#else
    #define CONFIG_USE_DRAM         1
#endif

//*********************************************
//#. Cache Setting
//*********************************************

// <e> Enable I-Cache
// <i> CONFIG_USE_ICACHE
#define CONFIG_USE_ICACHE       1
#if CONFIG_USE_ICACHE

    #define TYPE_CACHE_FLASH    1
    #define TYPE_CACHE_DRAM     2
    //  <o>I-Cache Device   <1=> SPI NOR-Flash
    //                      <2=> DRAM
    //  <i> CONFIG_CACHE_TYPE
    //  <i> Select Cache device, DRAM or SPI-Nor Flash.
    #define CONFIG_CACHE_TYPE   1

    #if (CONFIG_CACHE_TYPE == TYPE_CACHE_FLASH)
        #if (CONFIG_USE_SPI_FLASH == 0)
            #error "Please enable Flash/DRAM to activate I-Cache"
        #endif
    #elif (CONFIG_CACHE_TYPE == TYPE_CACHE_DRAM)
        #if (CONFIG_USE_DRAM == 0)
            #error "Please enable Flash/DRAM to activate I-Cache"
        #endif
    #endif

    //  <o> I-Cache Load Base
    //  <i> CONFIG_CACHE_LOAD_REGION
    //  <i> Default: 0x60011000 (hex)
    #define CONFIG_CACHE_LOAD_REGION    0x60200000

    //  <o> I-Cache Size    <0x2000=> 1M Byte
    //                      <0x1C00=> 869K Byte
    //                      <0x1800=> 768K Byte
    //                      <0x1400=> 640K Byte
    //                      <0x1000=> 512K Byte
    //                      <0x0C00=> 384K Byte
    //                      <0x0800=> 256K Byte
    //                      <0x0400=> 128K Byte
    //  <i> CONFIG_CACHE_MAXIMUN_SIZE
    //  <i> Default: 1M Byte
    #define CONFIG_CACHE_SIZE_UNIT  0x1000

    #define CONFIG_CACHE_MAXIMUN_SIZE (CONFIG_CACHE_SIZE_UNIT * 128)

#endif
// </e>


// </h>


// <<< end of configuration section >>>


#ifdef __cplusplus
}
#endif

#endif

