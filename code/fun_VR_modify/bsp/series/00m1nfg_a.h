#ifdef _snc7320_package_
    #error "Multidefine of snc7320 package"
#else
#define _snc7320_package_

// <<< Use Configuration Wizard in Context Menu >>>

    #define TYPE_DRAM_NONE      0
    #define TYPE_DRAM_QPI       1
    #define TYPE_DRAM_OPI       2
    #define TYPE_DRAM_DDR       3

    //  <o>DRAM Type    <1=> Pseudo SRAM - QPI_8MB
    //                  <2=> Pseudo SRAM - OPI_8MB
    //                  <3=> DDR_2MB
    //  <i>CONFIG_DRAM_TYPE
    //  <i>Select DRAM type.
    #define CONFIG_DRAM_TYPE    2

    #if (CONFIG_DRAM_TYPE == TYPE_DRAM_DDR)
        #define CONFIG_DRAM_SIZE    0x200000
    #else
        #define CONFIG_DRAM_SIZE    0x800000
    #endif
    
// <<< end of configuration section >>>

#endif
