 /**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file bsp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#include <string.h>
#include "snc_types.h"
#include "bsp.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
//System
#define REG_SYS_IP_BASE                 0x45000000
#define REG_SYS1_PERI_CLK_EN            0x45000100
#define REG_SYS1_CLK_RST                0x4500010C
#define REG_SYS0_PIN_CTRL               0x45000020

//SPIFC
#define REG_SPIFC_BASE                  0x40022000
#define BIT_SPIFC_QUAD_MODE             14

//Cache
#define REG_CACHE_CTRL                  0x4002F000
#define REG_CACHE_OFFSET                0x4002F004
#define CACHE_SEL_DRAM                  2
#define CACHE_SEL_FLASH                 3

//Dram
#define REG_DRAM_IP_BASE                0x40040000

//bsp initial script
#define INI_SEQ_CMD_START               0xFFFFFFFF
#define INI_SEQ_CMD_FINISH              0xEEEEEEEE
#define INI_SEQ_CMD_DELAY               0x77777777
#define INI_SEQ_CMD_WAIT_SET            0xAAAAAAAA
#define INI_SEQ_CMD_WAIT_CLR            0x55555555


#define ROM1_COMMAND_PROGRAM_JUMP       17

typedef enum
{
    SYSTEM_SOURCE_IHRC,
    SYSTEM_SOURCE_ILRC,
    SYSTEM_SOURCE_HXTAL,
    SYSTEM_SOURCE_LXTAL,
    SYSTEM_SOURCE_PLL,

} system_clk_src;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define ICACHE_SET_OFFSET(X)            (*(volatile uint32_t*)(REG_CACHE_BASE + 4) = (X))
#define ICACHE_RESET()                  (*(volatile uint32_t*)(REG_SYS1_CLK_RST) |= (0x1<<11))

#define CACHE_ENABLE(X,Y)               do{ ICACHE_RESET();                     \
                                            *(volatile uint32_t*)REG_CACHE_OFFSET = (Y);   \
                                            *(volatile uint32_t*)REG_CACHE_CTRL = (X);     \
                                        }while(0)

#define ENABLE_LDO18()                  {*(uint32_t*)(REG_SYS1_PERI_CLK_EN + 0x14) |= 1;}
                       
#define SPIFC_ENABLE_FAST_R()                                       \
{                                                                   \
    uint32_t data = *(uint32_t*)REG_SPIFC_BASE;                     \
    data &= ~(0xF << 4);                                            \
    data |=  (1 << (6));                                            \
    *(uint32_t*)REG_SPIFC_BASE = data;                              \
}

#define SPIFC_DISABLE_FAST_R()                                      \
{                                                                   \
    uint32_t data = *(uint32_t*)REG_SPIFC_BASE;                     \
    data &= ~(0xF << 4);                                            \
    data |=  (1 << (7));                                            \
    *(uint32_t*)REG_SPIFC_BASE = data;                              \
}
                    
#define SPIFC_WAIT_BUSY()                                           \
{                                                                   \
    while((*(uint32_t*)REG_SPIFC_BASE+4) & 2);                      \
}

/**
 *  register operator
 */
#define reg_mask_write(pAddr, value, mask)   \
    do{ *(volatile uint32_t*)(pAddr) = (*(volatile uint32_t*)(pAddr) & ~mask) | (value & mask); } while(0)

#define reg_write(pAddr, value)     do{ *(volatile uint32_t*)(pAddr) = (value); } while(0)
#define reg_set_bit(pAddr, bit_idx) do{ *(volatile uint32_t*)(pAddr) |= (0x1UL << (bit_idx)); }while(0)
#define reg_clr_bit(pAddr, bit_idx) do{ *(volatile uint32_t*)(pAddr) &= (~(0x1UL << (bit_idx))); }while(0)


/**
 *  Delay microseconds
 *  Tolerance:  162Mhz,     10us    ->      10%
 *              162Mhz,     1ms     ->      0.02%
 *               12Mhz,     10us    ->      35%
 *               12Mhz,     100us   ->      2.25%
 *               12Mhz,     1ms     ->      0.3%
 */
#define DELAY_US(sys_clk, usecond)                           \
    do{ volatile uint32_t tick = (sys_clk) * (usecond) / 7;  \
        while(tick--);                                       \
    }while(0)


/**
 *  SPIFC clock MUST be less then 40MHz
 */
#define SET_SPIFC_CLK_DIVIDER()                                                \
    do{ uint32_t    divider = 0, clk = CONFIG_CPU_CLOCK_MHZ * 1000000;         \
        while( (clk >> divider) > 40000000 ) divider++;                        \
        reg_mask_write(REG_SPIFC_BASE, (0x8000 | (divider << 12)), 0xF000);    \
    } while(0);
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef int (*INIT_FUNC)(void);

typedef union
{ 
    struct{
        uint32_t    offset;
        uint32_t    mask;
        uint32_t    data;
    } reg;

    struct{
        uint32_t    cmd_num;
        uint32_t    ref1;
        uint32_t    ref2;
    } cmd;
} init_sequence_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static const int low_priority_table[] =
{
    USBDEV_IRQn,
    USB_HOST0_IRQn,
    USB_HOST1_IRQn,
    SAR_ADC_IRQn,
    CIS_VSYNC_IRQn,
    CIS_DMA_IRQn,
    JPEG_ENC_END_IRQn,
    JPEG_DEC_IN_DMA_IRQn,
    JPEG_DEC_OUT_DMA_IRQn,
    SDIO_IRQn,
    SDIO_DMA_IRQn,
    NFECC_IRQn,
    SDNF_DMA_IRQn,
    IDMA0_IRQn,
    IDMA1_IRQn,
    I2S0_DMA_IRQn,
    I2S1_DMA_IRQn,
    I2S2_DMA_IRQn,
    I2S3_DMA_IRQn,
    I2S4_IRQn,
    I2C0_IRQn,
    I2C1_IRQn,
    I2C2_IRQn,
    SPI0_IRQn,
    SPI1_IRQn,
    UART0_IRQn,
    UART1_IRQn,
    SPI0_DMA_IRQn,
    SPI0_ECC_IRQn,
    SPI1_DMA_IRQn,
    SPI1_ECC_IRQn,
    CSC_DMA_IN_IRQn,
    CSC_DMA_OUT_IRQn,
    SCRIPT_IRQn,
    CRC16_IRQn,
    USB_SUSPEND_IRQn,
    JPEG_ENC_IN_DMA_IRQn,
    JPEG_ENC_OUT_DMA_IRQn,
    (-1),
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
/**
 *  \brief  Execute init script
 *
 *  \param [in] IP_base     register base address
 *  \param [in] p           init script
 *  \return
 *
 *  \details
 */

static uint32_t __attribute((unused)) 
_exec_init_script(
    uint32_t        IP_base,
    init_sequence_t *p)
{
    register uint32_t   *p_data = 0;

    if(p->cmd.cmd_num != INI_SEQ_CMD_START)
        return 1;                           /* Return Fail */

    while(1)
    {
        ++p;

        switch(p->cmd.cmd_num)
        {
            case INI_SEQ_CMD_FINISH:           /* Return success */
                return 0;

            case INI_SEQ_CMD_DELAY:            /* Loop for a while */
                DELAY_US(CONFIG_CPU_CLOCK_MHZ , p->cmd.ref2);
                break;

            case INI_SEQ_CMD_WAIT_SET:         /* Waiting for bit set status */
                p_data = (uint32_t*)(IP_base + p->cmd.ref1);
                while( (*p_data & p->cmd.ref2) != p->cmd.ref2 ) {}

                break;

            case INI_SEQ_CMD_WAIT_CLR:         /* Waiting for bit clear status */
                p_data = (uint32_t*)(IP_base + p->cmd.ref1);
                while( (*p_data & p->cmd.ref2) ) {}
                break;

            default:
                reg_mask_write((volatile uint32_t*)(p->reg.offset + IP_base),
                                p->reg.data,
                                p->reg.mask);
                break;
        }
    }
}


#if (CONFIG_SNC7320_FPGA)
/**
 *  Initial clock setting.
 */
static int
_init_clock_setting(void)
{
    reg_write(REG_SYS1_PERI_CLK_EN, 0xFFFF);
    reg_write(REG_SYS1_CLK_RST, 0xFFFF);

    //SET_SPIFC_CLK_DIVIDER();
    if(CONFIG_USE_SPI_FLASH)
    {
        reg_write(REG_SPIFC_BASE, 0x9488); // FPGA timing issue
    }
    return 0;
}
#else   // #if (CONFIG_SNC7320_FPGA)

    #if     (CONFIG_CPU_CLOCK_MHZ == 162)
        #include "clk_setting/init_clk_162mhz.h"

    #elif   (CONFIG_CPU_CLOCK_MHZ == 12) && (CONFIG_USE_EXTERNAL_CLK)
        #include "clk_setting/init_clk_ext_12mhz.h"

    #elif   (CONFIG_CPU_CLOCK_MHZ == 12) && (!CONFIG_USE_EXTERNAL_CLK)
        #include "clk_setting/init_clk_12mhz.h"

    // #elef defined(CONFIG_CPU_CLOCK_MHZ) && (CONFIG_CPU_CLOCK_MHZ == 148.5)
    #else
        #error "Unknown CPU Clock !!"
    #endif

/**
 *  Initial clock setting.
 */
static int
_init_clock_setting(void)
{

    /* Set clock Source */
    if(CONFIG_USE_EXTERNAL_CLK)
    {
        *(uint32_t*)REG_SYS_IP_BASE |= ((1<<4) | (1<<14));
        
        /* Wait for HTXL stable */
        while( ! (*(uint32_t*)(REG_SYS_IP_BASE + 0x08) & (1<<4)) ); 
    }

    _exec_init_script(REG_SYS_IP_BASE, (init_sequence_t*)init_clk_table);
    
    /* AHB Clock Prescalar */
    if(CONFIG_CLK_PRECSALAR)
    {
        *(uint32_t*)(REG_SYS_IP_BASE + 0x10) = CONFIG_CLK_PRECSALAR;
    }

    reg_write(REG_SYS1_PERI_CLK_EN, 0xFFFF);
    reg_write(REG_SYS1_CLK_RST, 0xFFFF);

    if(CONFIG_USE_SPI_FLASH)
    {
        SET_SPIFC_CLK_DIVIDER();
    }
    return 0;
}
#endif  // #if (CONFIG_SNC7320_FPGA)


/**
 *  Initial DRAM.
 */
#if defined(CONFIG_USE_DRAM) && (CONFIG_USE_DRAM)

#include "ext_memory/comm.c"   // Directory: snc7320_sdk\bsp\ext_memory

    #if (CONFIG_DRAM_TYPE == TYPE_DRAM_DDR)

        #include "ext_memory/ddr_init_v22.c"   // Directory: snc7320_sdk\bsp\ext_memory
        #define INIT_DRAM()     do{ ddr_init(); }while(0);

    #elif (CONFIG_DRAM_TYPE == TYPE_DRAM_OPI)

        #include "ext_memory/opi_init_v22.c"   // Directory: snc7320_sdk\bsp\ext_memory
        #define INIT_DRAM()     do{ opi_init(); }while(0);

    #elif (CONFIG_DRAM_TYPE == TYPE_DRAM_QPI)

        #include "ext_memory/qpi_init_v22.c"   // Directory: snc7320_sdk\bsp\ext_memory
        #define INIT_DRAM()     do{ qpi_init(); }while(0);

    #else
        #error "This 7320 package NOT support DRAM. "
    #endif

static int
_init_dram(void)
{
    ENABLE_LDO18(); /* 1.8v LDO enable */
    
    INIT_DRAM();
    
    /* Initial the dram channel usage */
    *(uint32_t*)0x45000054 =    0x9     |   /* IDMA1 */
                                0xF0    |   /* JPEG */
                                0x600   |   /* IDMA0 */
                                0xE000;     /* PPU/CIS */
    
    
    return 0;
}
#else
    #define _init_dram      0
#endif



/**
 *  Core_1 customize initial procedure
 *  ps. core_0 should control PC of core_1 to jump to user's address of image
 */
#if defined(CONFIG_CORE_1_DEVP_ENABLE) && (CONFIG_CORE_1_DEVP_ENABLE && (CONFIG_CORE_SELECT == 0))
static const uint8_t    core1_img[] __attribute__((at(CONFIG_CORE1_LD_ADDR))) =
{
    #include "core_1_img.dat"
};

static int
_init_core_1_customize(void)
{
    /* Prepair the execution region for core 1 */
    memcpy( (void*) CONFIG_CORE1_ER_ADDR,
            (void*) CONFIG_CORE1_LD_ADDR,
            CONFIG_CORE1_ER_SIZE);

    /* Initial command to the core 1 */
    reg_write(0x20000008, ROM1_COMMAND_PROGRAM_JUMP);
    reg_write(0x2000000C, CONFIG_CORE1_ER_ADDR);
    reg_write(0x20000000, 0x1);
    return 0;
}
#else
    #define _init_core_1_customize      0
#endif



/**
 * Initialize I-Cache
 */
#if defined(CONFIG_USE_ICACHE) && (CONFIG_USE_ICACHE)
static int
_init_icache(void)
{
    #if (CONFIG_USE_SPI_FLASH != 1)
        #error " Unable to use Cache without Flash "
    #endif
    
    #if (CONFIG_CACHE_TYPE == TYPE_CACHE_FLASH)

        /**
         *  Cache-Nor region
         */
        
        #define CONFIG_CACHE_OFFSET     (CONFIG_CACHE_LOAD_REGION - 0x60000000)
        CACHE_ENABLE(CACHE_SEL_FLASH, CONFIG_CACHE_OFFSET);

    #elif (CONFIG_CACHE_TYPE == TYPE_CACHE_DRAM)
        #if (CONFIG_USE_DRAM != 1)
            #error " Unable to use Cache without DRAM "
        #endif  
        /**
         * If Cache-DRAM, copy the data from Flash to ddr
         */
         
        #define DRAM_SIZE               (CONFIG_DRAM_SIZE - CONFIG_CACHE_MAXIMUN_SIZE)
        SPIFC_ENABLE_FAST_R();
        memcpy((void*)(0x30000000 + DRAM_SIZE), (void*)CONFIG_CACHE_LOAD_REGION, CONFIG_CACHE_MAXIMUN_SIZE);
        SPIFC_WAIT_BUSY();
        SPIFC_DISABLE_FAST_R();
        
        CACHE_ENABLE(CACHE_SEL_DRAM, DRAM_SIZE);
    #endif
    return 0;
}
#else
    #define _init_icache      0
#endif

/**
 * Initialize SPI-NAND
 */
static int
_init_spi_nand(void)
{
    /* IO PIN switch */
    *(uint32_t*)0x45000020 |= (1 << 8);
    
    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  This order CAN NOT be changed
 */
static const INIT_FUNC   g_init_list[] =
{
    (INIT_FUNC)_init_clock_setting,
    (INIT_FUNC)_init_dram,
    (INIT_FUNC)_init_core_1_customize,
    (INIT_FUNC)_init_icache,
    (INIT_FUNC)_init_spi_nand,
    (INIT_FUNC)(-1)
};

/**
 *  \brief This function do hard fault irq, it will system reset when hard fault happen.
 *
 *  \return Return     __irq in keil
 *
 *  \details
 */
__irq void HardFault_Handler(void)
{
	reg_clr_bit((REG_SYS_IP_BASE + 0x2c),0);
	NVIC_SystemReset();
}


/**
 *  \brief BSP initialize before APP start
 *
 *  \return Return      void
 *
 *  \details
 */
void System_Init(void)
{
    /**
     * Unlock Interrupt Mask
     */
    __set_PRIMASK(0);


    /**
     * Modify preempt priority
     */
    for(int i = 0; low_priority_table[i] != (-1); i++)
    {
        NVIC_SetPriority((IRQn_Type)low_priority_table[i], 3);
    }

    /**
     * return if current cpu is core 1
     */
    #if defined(CONFIG_CORE_SELECT) && (CONFIG_CORE_SELECT == 1)
        return;
    #else
        /* Initial the IPC buffer */
        memset( (void*) 0x20000000, 0, 0x10);
    #endif
	*(u32*)0x45000018 &= 0xFFFFFFEF;
    /**
     * initial modules
     */
    for(int i = 0; g_init_list[i] != (INIT_FUNC)(-1); i++)
    {
        if( g_init_list[i] )
        {
            int     rval = 0;
            rval = g_init_list[i]();
            if( rval )      break;
        }
    }

    return;
}


