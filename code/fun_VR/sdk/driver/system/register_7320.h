/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file register.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/09
 * @license
 * @description
 */

#ifndef __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__
#define __register_H_wsSeqW16_ltGb_H4QG_sdWn_uMFLWG2vO8nD__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                      Constant Definition
//=============================================================================
/**
 *  7320 definition
 */
#define SN_SYS_CTRL_0_BASE   0x45000000UL
#define SN_SYS_CTRL_1_BASE   0x45000100UL
//=============================================================================
//                      Macro Definition
//=============================================================================

//=============================================================================
//                      Structure Definition
//=============================================================================
/**
 *  SN_SYS0 Structure
 */
typedef struct sn_sys_ctrl_0
{
    union {
        __IO uint32_t  OSC_CTRL; /*!< Offset:0x00 OSC Control Register */

        struct {
            __IO uint32_t IHRCEN       : 1;
            __IO uint32_t PLL_DLY_SEL  : 1;
            __IO uint32_t PLL_ICP_SEL  : 2;
            __IO uint32_t HXTLEN       : 1;
            __IO uint32_t PLL_TCK_SEL  : 2;
            __IO uint32_t RESERVED0    : 7;
            __IO uint32_t PLL_CLK_SEL  : 1;
            __IO uint32_t PLL_EN       : 1;
            __IO uint32_t LPF_OFF      : 1;
            __IO uint32_t RESERVED1    : 15;
        } OSC_CTRL_b;
    };
    union {
        __IO uint32_t  FPLL_CTRL;   /*!< Offset:0x04 Fractional PLL Control Register   */

        struct {
            __IO uint32_t  PLL_FRA_H      : 4;
            __IO uint32_t  PLL_INT        : 5;
            __IO uint32_t  VLD_IN         : 1;
            __IO uint32_t  SDM_EN         : 1;
            __IO uint32_t  PLL_IVCO_SEL   : 4;
            __IO uint32_t  RESERVED0      : 1;
            __IO uint32_t  PLL_FRA_L      : 16;
        } FPLL_CTRL_b;
    };

    union {
        __IO uint32_t  OSC_STAT; /*!< Offset:0x08 CLK Status Register  */

        struct {
            __IO uint32_t  IHRC_RDY    :  1;   /*!< IHRC Ready Flag */
            __IO uint32_t  RESERVED3   :  1;   /*!< RESERVED */
            __IO uint32_t  LXTAL_RDY   :  1;   /*!< LXTAL Ready Flag   */
            __IO uint32_t  RESERVED4   :  1;   /*!< RESERVED */
            __IO uint32_t  HXTAL_RDY   :  1;   /*!< HXTAL Ready Flag   */
            __IO uint32_t  RESERVED2   :  1;   /*!< RESERVED */
            __IO uint32_t  SPLL_RDY    :  1;   /*!< System PLL Ready Flag */
            __IO uint32_t  RESERVED1   :  25;  /*!< RESERVED */
        } OSC_STAT_b;
    };

    union {
        __IO uint32_t  SYS_CLKCFG; /*!< Offset:0x0C System CLK Control Register  */

        struct {
            __IO uint32_t  SYSCLKSEL   :  3;   /*!< System CLK source select */
            __IO uint32_t  RESERVED1   :  1;   /*!< RESERVED */
            __IO uint32_t  SYSCLKST    :  3;   /*!<   */
            __IO uint32_t  RESERVED0   : 25;   /*!< RESERVED */
        } SYS_CLKCFG_b;
    };
    __IO uint32_t  SYS_CLKDIV;   /*!< Offset:0x10 AHB Clock Prescale register (SYS_AHBCP) */

    union {
        __IO uint32_t  RST_FLAG; /*!< Offset:0x14 Control Register  */

        struct {
            __IO uint32_t  SWRSTF      :  1;   /*!< Software Reset  */
            __IO uint32_t  WDTRSTF     :  1;   /*!< WDT0 Reset  */
            __IO uint32_t  LVRRSTF     :  1;   /*!< LVD Reset   */
            __IO uint32_t  DPDRSTF     :  1;   /*!< RESERVED */
            __IO uint32_t  PORRSTF     :  1;   /*!< POR Reset   */
            __IO uint32_t  RESERVED0   : 26;   /*!< RESERVED */
        } RST_FLAG_b;
    };

    union {
        __IO uint32_t  LVD_CTRL; /*!< Offset:0x18 LVD Control Register */

        struct {
            __IO uint32_t  LVD11EN           :  1;   /*!< 1.1v LVD Enable */
            __IO uint32_t  LVD33EN           :  1;   /*!< 3.3v LVD Enable */
            __IO uint32_t  DISCHARGE_PD      :  1;   /*!< DISCHARGE_PD */
            __IO uint32_t  LVD33VAL          :  2;   /*!< 3.3v LVD value  */
            __IO uint32_t  LDO_ILRC_SEL      :  2;
            __IO uint32_t  LVD33FLAG         :  1;   /*!< Level Trigger   */
            __O  uint32_t  EFUSE_ERROR_FLAG  :  1;
            __O  uint32_t  RESERVED          : 22;   /*!< RESERVED */
        } LVD_CTRL_b;
    };
    __IO uint32_t  RESERVED11;  /*!< Offset:0x1C RESERVED */


    union {
        __IO uint32_t  PINCTRL; /*!< Offset:0x20 Pin Control Register */

        struct {

            __IO uint32_t   SDNF0         :  1;   /*!< SDNF Pin Mapping   */
            __IO uint32_t   TFT_8080      :  1;   /*!< TFT/8080 Pin Mapping  */
            __IO uint32_t   RESERVED      :  1;   /*!< RESERVED */
            __IO uint32_t   HOST_PPC      :  1;   /*!< Host PPC Mapping   */
            __IO uint32_t   I2S4          :  2;   /*!< I2S4 Pin Mapping   */
            __IO uint32_t   SWDV_0        :  1;   /*!< SWO CPU Select  */
            __IO uint32_t   SWDV_1        :  1;
            __IO uint32_t   SPIDMA0       :  1;
            __IO uint32_t   OID_Sensor_IF :  1;
            __IO uint32_t   PWMIO_0       :  1;
            __IO uint32_t   PWMIO_1       :  1;
            __IO uint32_t   PWMIO_8       :  1;
            __IO uint32_t   PWMIO_9       :  1;
            __IO uint32_t   PWMIO_10      :  1;
            __IO uint32_t   PWMIO_11      :  1;
            __IO uint32_t   PWMIO_18      :  1;
            __IO uint32_t   PWMIO_19      :  1;
            __IO uint32_t   PWMIO_20      :  1;
            __IO uint32_t   PWMIO_21      :  1;
            __IO uint32_t   PWMIO_22      :  1;
            __IO uint32_t   PWMIO_23      :  1;
            __IO uint32_t   I2C_SYNC_SW   :  1;
            __IO uint32_t   AudioIF2      :  1;
            __IO uint32_t   HOST_DEV_SEL  :  1;
            __IO uint32_t   USB_DBG_OPT   :  1;
            __IO uint32_t   USB_TSQU_SEL  :  1;
            __IO uint32_t   RESERVED0     :  6;   /*!< RESERVED */
        } PINCTRL_b;
    };

    __IO uint32_t  SYS0_NDTCTRL; /*!< Offset:0x24 NDT Control register */
    __IO uint32_t  RESERVED28;  /*!< Offset:0x28  */

    union {
        __IO uint32_t  SYS0_REMAP;  /*!< Offset:0x2C Remap register */

        struct {
            __IO uint32_t  REMAP       :  1;   /*!< Set to Remap PRAM and ROM */
            __IO uint32_t  RESERVED0   :  31;  /*!< RESERVED */
        } REMAP_b;
    };

    union {
        __IO uint32_t  FEUSE0;  /*!< Offset:0x30 E-FUSE 0 Register */

        struct {
            __O  uint32_t  LVD33       :  4;
            __O  uint32_t  LVR33       :  2;
            __O  uint32_t  RESERVED0   : 10;
            __O  uint32_t  LVD11       :  3;
        } FEUSE0_b;
    };

    union {
        __IO uint32_t  FEUSE1;  /*!< Offset:0x34 E-FUSE 1 Register */

        struct {
            __O  uint32_t  IHRC_TRIM   :  8;
            __O  uint32_t  LDO11       :  4;
            __O  uint32_t  LDO18       :  4;
            __O  uint32_t  USB_PHY1    :  8;
            __O  uint32_t  USB_PHY0    :  8;
        } FEUSE1_b;
    };

    union {
        __IO uint32_t  FEUSE2;  /*!< Offset:0x38 E-FUSE 2 Register */

        struct {
            __O  uint32_t  AES_Key     : 16;
            __O  uint32_t  Reserved    : 14;
            __O  uint32_t  USB_DISABLE :  1;
            __O  uint32_t  SECURITY    :  1;
        } FEUSE2_b;
    };

    __IO uint32_t  FEUSE3; /*!< Offset:0x3C E-FUSE 3 Register */
    __IO uint32_t  Reserved[3];   /*!< Offset:0x40 - 0x4C Reserved   */

    union {
        __IO uint32_t  NAP;  /*!< Offset:0x50 RAM Power Down Register  */

        struct {
            __IO uint32_t  MATRIX_RAM     :  8;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  MAILBOX_RAM    :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  CACHE_RAM      :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  USB_HOST1      :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  USB_HOST2      :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  USB_DEV        :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  PPU_RAM        :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  CSC_RAM        :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  JPEG_RAM_NAP   :  1;   /*!< 1: NAP, 0: Disable */
            __IO uint32_t  RESERVED0      :  16;  /*!< RESERVED  */
        } NAP_b;
    };
    union {
        __IO uint32_t  DMA_CH_SEL;  /*!< Offset:0x54 DDR Channel control register */

        struct {
            __IO uint32_t  DMA1_OUT    :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  DMA1_IN     :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  JPEG_D_IN   :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  JPEG_E_OUT  :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  DMA0_OUT    :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  DMA0_IN     :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  CIS         :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  PPU         :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None */
            __IO uint32_t  RESERVED0   :  16;  /*!< RESERVED */
        } DMA_CH_SEL_b;
    };

    __IO uint32_t  M3_TICK;  /*!< Offset:0x58 CortexM3 Tick Control register  */

} sn_sys_ctrl_0_t;


/**
 *  SN_SYS1 Structure
 */
typedef struct sn_sys_ctrl_1
{
    union {
        __IO uint32_t  PCLK_EN;  /*!< Offset:0x00 Peripheral Clock Enable Register   */

        struct {
            __IO uint32_t  CT32BCLKEN     :  1;   /*!< CT32BCLK CLK Enable */
            __IO uint32_t  WDTCLKEN       :  1;   /*!< WDT CLK Enable  */
            __IO uint32_t  APBCLKEN       :  1;   /*!< APB CLK Enable  */
            __IO uint32_t  AUDIOCLKEN     :  1;   /*!< AUDIO CLK Enable   */
            __IO uint32_t  MSCLKEN        :  1;   /*!< MS CLK Enable */
            __IO uint32_t  VIDEOCLKEN     :  1;   /*!< VIDEO CLK Enable   */
            __IO uint32_t  USBHOST1CLKEN  :  1;   /*!< USBHOST1 CLK Enable */
            __IO uint32_t  USBHOST0CLKEN  :  1;   /*!< USBHOST0 CLK Enable */
            __IO uint32_t  USBDEVCLKEN    :  1;   /*!< USBDEV CLK Enable  */
            __IO uint32_t  PPUCLKEN       :  1;   /*!< PPU CLK Enable  */
            __IO uint32_t  EXTMEM         :  1;   /*!< EXTMEM CLK Enable  */
            __IO uint32_t  ICACHE         :  1;   /*!< ICACHE CLK Enable  */
            __IO uint32_t  SARCLKEN       :  1;   /*!< SAR CLK Enable  */
            __IO uint32_t  MCLK2EN        :  1;   /*!< MCLK2 CLK Enable   */
            __IO uint32_t  MCLK1EN        :  1;   /*!< MCLK1 CLK Enable   */
            __IO uint32_t  M3_1CLKEN      :  1;   /*!< M3_1 CLK Enable */
            __O uint32_t   RESERVED0      :  16;  /*!< RESERVED   */
        } PCLK_EN_b;
    };

    __IO uint32_t  RESERVED4;   /*!< Offset:0x04 RESERVED8 */

    union {
        __IO uint32_t  PERI_CLKCTL; /*!< Offset:0x08 Peripheral clock control register 1 */

        struct {
            __IO uint32_t  APBPRE      :  3;
            __IO uint32_t  Reserved3   :  2;
            __IO uint32_t  SDNF_DIV    :  2;
            __IO uint32_t  Reserved7   :  1;
            __IO uint32_t  CKOUT_DIV   :  3;
            __IO uint32_t  Reserved11  :  1;
            __IO uint32_t  CKOUT_SEL   :  3;
            __IO uint32_t  Reserved15  :  17;
        } PCLK_CTL_b;
    };

    union {
        __IO uint32_t  PCLK_RST; /*!< Offset:0x0C Peripheral Reset Control Register  */

        struct {
            __IO uint32_t  CT32BCLKRST    :  1;   /*!< CT32B/PWM  Clock Reset */
            __IO uint32_t  WDTCLKRST      :  1;   /*!< WDT*2  Clock Reset */
            __IO uint32_t  APBCLKRST      :  1;   /*!< SPI/GPIO/UART/I2C/GPIO Clock Reset  */
            __IO uint32_t  AUDIOCLKRST    :  1;   /*!< I2S*5  Clock Reset */
            __IO uint32_t  MSCLKRST       :  1;   /*!< IDMA0/IDMA1/SD/NF/SDIO/AES/SPIFlash/SPI0DMA/SPI1DMA/CRC Clock Reset */
            __IO uint32_t  VIDEOCLKRST    :  1;   /*!< JPEG/CSC/CIS   Clock Reset   */
            __IO uint32_t  USBHOST1CLKRST :  1;   /*!< HOST1  Clock Reset */
            __IO uint32_t  USBHOST0CLKRST :  1;   /*!< HOST0  Clock Reset */
            __IO uint32_t  USBDEVCLKRST   :  1;   /*!< USBDEV Clock Reset  */
            __IO uint32_t  PPUCLKRST      :  1;   /*!< PPU Clock Reset */
            __IO uint32_t  EXT_MEM        :  1;   /*!< Ext.Memory(DDR/PSRAM) Clock Reset */
            __IO uint32_t  I_CACHE        :  1;   /*!< I-Cache Clock Reset */
            __IO uint32_t  SARCLKRST      :  1;   /*!< SAR Clock Reset */
            __IO uint32_t  Reserved13     :  2;
            __IO uint32_t  MCLK1RST       :  1;
            __IO uint32_t  M3_1CLKRST     :  1;   /*!< M3_1, FIR/FFT Clock Reset */
            __IO uint32_t  RESERVED0      :  16;  /*!< RESERVED   */
        } PCLK_RST_b;
    };

    union {
        __IO uint32_t  USB_CTRL; /*!< Offset:0x10 USB SIE control register */

        struct {
            __IO uint32_t  PHY0_PLL480En      :  1;   /*!< HostPLL480 */
            __IO uint32_t  PHY1_PLL480En      :  1;   /*!< HostPLL480 */
            __IO uint32_t  PHY0_PDN_MUX       :  1;   /*!< PHY_PDN_MUX   */
            __IO uint32_t  PHY0_CPUD_UTMI_PDN :  1;   /*!< CPUD_UTMI_PDN */
            __IO uint32_t  PHY1_PDN_MUX       :  1;   /*!< PHY_PDN_MUX   */
            __IO uint32_t  PHY1_CPUD_UTMI_PDN :  1;   /*!< CPUD_UTMI_PDN */
            __IO uint32_t  RESERVED6          :  2;
            __IO uint32_t  PHY_SEL            :  2;
            __IO uint32_t  RESERVED10         : 23;

        } USB_CTRL_b;
    };
    __IO uint32_t  LDO18_CTL;   /*!< Offset:0x14 1.8V LDO control register  */

    union {
        __IO uint32_t  AHBMUX_RST;  /*!< Offset:0x18 USB PHY Control Register */

        struct {
            __IO uint32_t  MUX0RST  :  1;
            __IO uint32_t  MUX1RST  :  1;
            __IO uint32_t  MUX2RST  :  1;
        } AHBMUX_RST_b;
    };

} sn_sys_ctrl_1_t;

//=============================================================================
//                      Global Data Definition
//=============================================================================

//=============================================================================
//                      Private Function Definition
//=============================================================================

//=============================================================================
//                      Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
