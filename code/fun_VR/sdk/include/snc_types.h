/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_types.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_types_h_wrWxjHcr_l918_HSN0_saQu_u3TAtqoRTIEQ__
#define __snc_types_h_wrWxjHcr_l918_HSN0_saQu_u3TAtqoRTIEQ__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

/**
 *  Interrupt Number Definition
 */
typedef enum _IRQn_Type
{
    /* -------------------  Cortex-M3 Processor Exceptions Numbers -------------------------------------------------------------- */
    Reset_IRQn                  = -15,              /*!<   -15  Reset Vector, invoked on Power up and warm reset        */
    NonMaskableInt_IRQn         = -14,              /*!<   -14  NMI, cannot be stopped or preempted, for WDT0/WDT1      */
    HardFault_IRQn              = -13,              /*!<   -13  Hard Fault, all classes of Fault                        */
    MemManage_IRQn              = -12,              /*!<   -12  MPU Fault Fault, all classes of Fault                   */
    BusFault_IRQn               = -11,              /*!<   -11  Bus Fault Fault, all classes of Fault                   */
    UsageFault_IRQn             = -10,              /*!<   -10  Usage Fault Fault, all classes of Fault                 */
    SVCall_IRQn                 =  -5,              /*!<   -5  System Service Call via SVC instruction                  */
    DebugMonitor_IRQn           =  -4,              /*!<   -4  Debug Monitor                                            */
    PendSV_IRQn                 =  -2,              /*!<   -2  Pendable request for system service                      */
    SysTick_IRQn                =  -1,              /*!<   -1  System Tick Timer                                        */
    /* ---------------------  SNC7320 Specific Interrupt Numbers  ----------------------------------------------------------------*/
    Core0_Issue_IRQn            =  0,               /*!<  0  Core0 Issue                */
    Core1_Issue_IRQn            =  1,               /*!<  1  Core1 Issue                */
    RTC_IRQn                    =  2,               /*!<  2  RTC                        */
    WKP_IRQn                    =  3,               /*!<  3  KEY_WKP                    */
    PPU_VBLK_IRQn               =  4,               /*!<  4  PPU_VBLK                   */
    PPU_HBLK_IRQn               =  5,               /*!<  5  PPU_HBLK                   */
    USBDEV_IRQn                 =  6,               /*!<  6  USB DEV                    */
    USB_HOST0_IRQn              =  7,               /*!<  7  USB_HOST0                  */
    USB_HOST1_IRQn              =  8,               /*!<  8  USB_HOST1                  */
    SAR_ADC_IRQn                =  9,               /*!<  9  SAR ADC                    */
    CIS_VSYNC_IRQn              =  10,              /*!<  10  CIS_VSYNC                 */
    CIS_DMA_IRQn                =  11,              /*!<  11  CIS_DMA                   */
    JPEG_ENC_END_IRQn           =  12,              /*!<  12  JPEG_ENC DMA              */
    JPEG_DEC_IN_DMA_IRQn        =  13,              /*!<  13  JPEG_DEC_IN DMA           */
    JPEG_DEC_OUT_DMA_IRQn       =  14,              /*!<  14  JPEG_DEC_OUT DMA          */
    SDIO_IRQn                   =  15,              /*!<  15  SDIO                      */
    SDIO_DMA_IRQn               =  16,              /*!<  16  SDIO_DMA                  */
    NFECC_IRQn                  =  17,              /*!<  17  SD/NF ECC                 */
    SDNF_DMA_IRQn               =  18,              /*!<  18  SD_CARD | NF _DMA         */
    IDMA0_IRQn                  =  19,              /*!<  19  IDMA0                     */
    IDMA1_IRQn                  =  20,              /*!<  20  IDMA1                     */
    I2S0_DMA_IRQn               =  21,              /*!<  21  I2S0_DMA                  */
    I2S1_DMA_IRQn               =  22,              /*!<  22  I2S1_DMA                  */
    I2S2_DMA_IRQn               =  23,              /*!<  23  I2S2_DMA                  */
    I2S3_DMA_IRQn               =  24,              /*!<  24  I2S3_DMA                  */
    I2S4_IRQn                   =  25,              /*!<  25  I2S4                      */
    GPIO0_IRQn                  =  26,              /*!<  26  GPIO0 //P0_IRQn in 7312   */
    GPIO1_IRQn                  =  27,              /*!<  27  GPIO1 //P1_IRQn in 7312   */
    GPIO2_IRQn                  =  28,              /*!<  28  GPIO2 //P2_IRQn in 7312   */
    GPIO3_IRQn                  =  29,              /*!<  29  GPIO3 //P3_IRQn in 7312   */
    GPIO4_IRQn                  =  30,              /*!<  30  GPIO4                     */
    I2C0_IRQn                   =  31,              /*!<  31  I2C0                      */
    I2C1_IRQn                   =  32,              /*!<  32  I2C1                      */
    I2C2_IRQn                   =  33,              /*!<  33  I2C2                      */
    SPI0_IRQn                   =  34,              /*!<  34  SPI0                      */
    SPI1_IRQn                   =  35,              /*!<  35  SPI1                      */
    UART0_IRQn                  =  36,              /*!<  36  UART0                     */
    UART1_IRQn                  =  37,              /*!<  37  UART1                     */
    CT32B0_IRQn                 =  38,              /*!<  38  CT32B0                    */
    CT32B1_IRQn                 =  39,              /*!<  39  CT32B1                    */
    CT32B2_IRQn                 =  40,              /*!<  40  CT32B2                    */
    CT32B3_IRQn                 =  41,              /*!<  41  CT32B3                    */
    CT32B4_IRQn                 =  42,              /*!<  42  CT32B4                    */
    CT32B5_IRQn                 =  43,              /*!<  43  CT32B5                    */
    CT32B6_IRQn                 =  44,              /*!<  44  CT32B6                    */
    CT32B7_IRQn                 =  45,              /*!<  45  CT32B7                    */
    SPI0_DMA_IRQn               =  46,              /*!<  46  SPI0_DMA                  */
    SPI0_ECC_IRQn               =  47,              /*!<  47  SPI0_ECC                  */
    SPI1_DMA_IRQn               =  48,              /*!<  48  SPI1_DMA                  */
    SPI1_ECC_IRQn               =  49,              /*!<  49  SPI1_ECC                  */
    CSC_DMA_IN_IRQn             =  50,              /*!<  50  CSC_DMA_IN DMA            */
    CSC_DMA_OUT_IRQn            =  51,              /*!<  51  CSC_DMA_OUT DMA           */
    SCRIPT_IRQn                 =  52,              /*!<  52  SCRIPT_INT                */
    CRC16_IRQn                  =  53,              /*!<  53  CRC16_INT                 */
    USB_SUSPEND_IRQn            =  54,              /*!<  54  USB_SUSPEND_IRQn          */
    JPEG_ENC_IN_DMA_IRQn        =  55,              /*!<  55  JPEG_ENC_IN_DMA_IRQn      */
    JPEG_ENC_OUT_DMA_IRQn       =  56,              /*!<  56  JPEG_ENC_OUT_DMA_IRQn     */
} IRQn_Type;


#if defined(__ARMCC_VERSION) || defined(__ICCARM__)
    /**
     *  Configuration of the Cortex-M3 Processor and Core Peripherals
     */
    #define __CM3_REV                 0x0200            /*!< Cortex-M3 Core Revision */
    #define __MPU_PRESENT                  1            /*!< MPU present or not */
    #define __NVIC_PRIO_BITS               3            /*!< Number of Bits used for Priority Levels */
    #define __Vendor_SysTickConfig         0            /*!< Set to 1 if different SysTick Config is used */

    /**
     *  Cortex-M3 processor and core peripherals
     */
    #include <core_cm3.h>

    #if defined(__ARMCC_VERSION)
        // #define __ASM            __asm                /*!< asm keyword for ARM Compiler */

        #ifndef __INLINE
        #define __INLINE         __inline             /*!< inline keyword for ARM Compiler */
        #endif

    #elif defined(__ICCARM__)
        // #define __ASM            __asm                /*!< asm keyword for IAR Compiler */

        #ifndef __INLINE
        #define __INLINE         inline               /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
        #endif

    #else
        #error Unknown compiler
    #endif


#elif defined(__GNUC__)

    #ifdef __cplusplus
        #define __I     volatile              /*!< defines 'read only' permissions */
    #else
        #define __I     volatile const        /*!< defines 'read only' permissions */
    #endif

    #define __O     volatile                  /*!< defines 'write only' permissions */
    #define __IO    volatile                  /*!< defines 'read / write' permissions */

    // #define __ASM            __asm                /*!< asm keyword for GNU Compiler */

    #ifndef __INLINE
    #define __INLINE         inline               /*!< inline keyword for GNU Compiler */
    #endif

#endif

#define SET_BIT0            0x00000001
#define SET_BIT1            0x00000002
#define SET_BIT2            0x00000004
#define SET_BIT3            0x00000008
#define SET_BIT4            0x00000010
#define SET_BIT5            0x00000020
#define SET_BIT6            0x00000040
#define SET_BIT7            0x00000080
#define SET_BIT8            0x00000100
#define SET_BIT9            0x00000200
#define SET_BIT10           0x00000400
#define SET_BIT11           0x00000800
#define SET_BIT12           0x00001000
#define SET_BIT13           0x00002000
#define SET_BIT14           0x00004000
#define SET_BIT15           0x00008000
#define SET_BIT16           0x00010000
#define SET_BIT17           0x00020000
#define SET_BIT18           0x00040000
#define SET_BIT19           0x00080000
#define SET_BIT20           0x00100000
#define SET_BIT21           0x00200000
#define SET_BIT22           0x00400000
#define SET_BIT23           0x00800000
#define SET_BIT24           0x01000000
#define SET_BIT25           0x02000000
#define SET_BIT26           0x04000000
#define SET_BIT27           0x08000000
#define SET_BIT28           0x10000000
#define SET_BIT29           0x20000000
#define SET_BIT30           0x40000000
#define SET_BIT31           0x80000000

// typedef int32_t  s32;
// typedef int16_t  s16;
// typedef int8_t    s8;

// typedef const int32_t sc32;
// typedef const int16_t sc16;
// typedef const int8_t   sc8;

// typedef __IO int32_t  vs32;
// typedef __IO int16_t  vs16;
// typedef __IO int8_t    vs8;

// typedef __I int32_t  vsc32;
// typedef __I int16_t  vsc16;
// typedef __I int8_t    vsc8;

typedef uint32_t       u32;
typedef uint16_t       u16;
typedef uint8_t         u8;

// typedef const uint32_t uc32;
// typedef const uint16_t uc16;
// typedef const uint8_t   uc8;

// typedef __IO uint32_t  vu32;
// typedef __IO uint16_t  vu16;
// typedef __IO uint8_t    vu8;

// typedef __I uint32_t vuc32;
// typedef __I uint16_t vuc16;
// typedef __I uint8_t   vuc8;


/**
 *  error code
 *  if > 0  => status
 *  if == 0 => success
 *  if < 0  => error code
 */
typedef enum snc_err
{
    SNC_ERR_OK              = 0,
    SNC_ERR_FAIL            = -1,
    SNC_ERR_INVALID_PARAM   = -2,
    SNC_ERR_NULL_POINTER    = -3,
    SNC_ERR_UNKNOWN         = -4,

} snc_err_t;

typedef enum snc_dev
{
    SNC_DEV_NONE        = 0,
    SNC_DEV_CT32_0,
    SNC_DEV_CT32_1,
    SNC_DEV_CT32_2,
    SNC_DEV_CT32_3,
    SNC_DEV_CT32_4,
    SNC_DEV_CT32_5,
    SNC_DEV_CT32_6,
    SNC_DEV_CT32_7,
    SNC_DEV_WDT_0,
    SNC_DEV_WDT_1,
    SNC_DEV_UART_0,
    SNC_DEV_UART_1,
    SNC_DEV_SPI_0,
    SNC_DEV_SPI_1,
    SNC_DEV_I2C_0,
    SNC_DEV_I2C_1,
    SNC_DEV_I2C_2,
    SNC_DEV_IDMA,
    SNC_DEV_SPIFC,
    SNC_DEV_SDNF,
    SNC_DEV_SDIO,
    SNC_DEV_CIS,
    SNC_DEV_AES,
    SNC_DEV_SPI_DMA_0,
    SNC_DEV_SPI_DMA_1,
    SNC_DEV_ICACHE,
    SNC_DEV_I2S_0,
    SNC_DEV_I2S_1,
    SNC_DEV_I2S_2,
    SNC_DEV_I2S_3,
    SNC_DEV_I2S_4,
    SNC_DEV_SARADC,
    SNC_DEV_DDR,
    SNC_DEV_USB_DEV,
    SNC_DEV_USB_HOST_0,
    SNC_DEV_USB_HOST_1,
    SNC_DEV_USB_0,
    SNC_DEV_USB_1,
    SNC_DEV_OPM_RTC_ISO,
    SNC_DEV_PMU,
    SNC_DEV_PWM,
    SNC_DEV_RTC_COUNTER,

} snc_dev_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
/**
 *  Bit band SRAM definitions
 */
#define BITBAND_SRAM_REF        0x20000000ul
#define BITBAND_SRAM_BASE       0x22000000ul

/**
 *  Bit band PERIPHERAL definitions
 */
#define BITBAND_PERI_REF        0x40000000ul
#define BITBAND_PERI_BASE       0x42000000ul

/**
 *  Convert SRAM address
 */
#define BITBAND_SRAM(addr, bit_idx)             (BITBAND_SRAM_BASE + (((uint32_t)(addr) - BITBAND_SRAM_REF) << 5) + ((bit_idx) << 2))
#define BITBAND_SRAM_CLEAR_BIT(addr, bit_idx)   (*(volatile uint32_t *)(BITBAND_SRAM(addr, bit_idx)) = 0)
#define BITBAND_SRAM_SET_BIT(addr, bit_idx)     (*(volatile uint32_t *)(BITBAND_SRAM(addr, bit_idx)) = 1)
#define BITBAND_SRAM_GET_BIT(addr, bit_idx)     (*(volatile uint32_t *)(BITBAND_SRAM(addr, bit_idx)))

/**
 *  Convert peripheral address
 */
#define BITBAND_PERI(addr, bit_idx)             (BITBAND_PERI_BASE + (((uint32_t)(addr) - BITBAND_PERI_REF) << 5) + ((bit_idx) << 2))
#define BITBAND_PERI_CLEAR_BIT(addr, bit_idx)   (*(volatile uint32_t *)(BITBAND_PERI(addr, bit_idx)) = 0)
#define BITBAND_PERI_SET_BIT(addr, bit_idx)     (*(volatile uint32_t *)(BITBAND_PERI(addr, bit_idx)) = 1)
#define BITBAND_PERI_GET_BIT(addr, bit_idx)     (*(volatile uint32_t *)(BITBAND_PERI(addr, bit_idx)))


/**
 *  Simple lock with bit banding, it should use hot code pointer
 */
#define BB_LOCK(pBB_Addr)       do{ while(*(volatile uint32_t *)(pBB_Addr)); \
                                    *(volatile uint32_t *)(pBB_Addr) = 0x1;  \
                                }while(0)

/**
 *  Simple unlock with bit banding, it should use hot code pointer
 */
#define BB_UNLOCK(pBB_Addr)     do{*(volatile uint32_t *)(pBB_Addr) = 0x0;}while(0)

#if defined(SNX_DEBUG) && (SNX_DEBUG)
#include <stdio.h>
#define snc_assert(expression)                                              \
        do{ if(expression) break;                                           \
            printf("\033[31m" "%s error: %s[#%u]\n" "\033[0m", #expression, __func__, __LINE__); \
            while(1);                                                       \
        }while(0)
#else
#define snc_assert(expression)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

//__weak void _sys_exit(void);     /* prevent from undefined function */
//__weak void _ttywrch(void);      /* prevent from undefined function */

#ifdef __cplusplus
}
#endif

#endif
