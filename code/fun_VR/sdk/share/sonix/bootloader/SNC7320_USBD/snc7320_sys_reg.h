
#ifndef SNC7320_SYS_REG_H
#define SNC7320_SYS_REG_H

#include <stdio.h>
#include <string.h>
#include "snc_types.h"

/* =========================================================================================================================== */
/* ====================================                   SN_SYS0                      ======================================= */
/* =========================================================================================================================== */
/**
  * @brief System Control Registers (SN_SYS0):OPM
 */

typedef struct {                                    /*!< SN_SYS0 Structure                                                  */ 
    union {
        __IO uint32_t  OSC_CTRL;                    /*!< Offset:0x00 OSC Control Register                                   */
    
        struct {
            __IO uint32_t IHRCEN                    : 1;                
            __IO uint32_t PLL_DLY_SEL               : 1;
            __IO uint32_t PLL_ICP_SEL               : 2;
            __IO uint32_t HXTLEN                    : 1;
            __IO uint32_t PLL_TCK_SEL               : 2;
            __IO uint32_t RESERVED0                 : 7;
            __IO uint32_t PLL_CLK_SEL               : 1;
            __IO uint32_t PLL_EN                    : 1;
            __IO uint32_t LPF_OFF                   : 1;
            __IO uint32_t RESERVED1                 : 15;
        } OSC_CTRL_b;
    };
    union {
        __IO uint32_t  FPLL_CTRL;                   /*!< Offset:0x04 Fractional PLL Control Register                        */

        struct 
        {
            __IO uint32_t  PLL_FRA_H                : 4;                
            __IO uint32_t  PLL_INT                  : 5;              
            __IO uint32_t  VLD_IN                   : 1;                
            __IO uint32_t  SDM_EN                   : 1;                
            __IO uint32_t  PLL_IVCO_SEL             : 4;                
            __IO uint32_t  RESERVED0                : 1;                
            __IO uint32_t  PLL_FRA_L                : 16;                
        } FPLL_CTRL_b;                                      
    };    

    union {
        __IO uint32_t  SYS0_CSST;                   /*!< Offset:0x08 Clock Source Status register                                    */

        struct {        
            __IO uint32_t  IHRC_RDY                 :  1;   /*!< IHRC Ready Flag                                            */
            __IO uint32_t  RESERVED3                :  1;   /*!< RESERVED                                                   */
            __IO uint32_t  LXTAL_RDY                :  1;   /*!< LXTAL Ready Flag                                           */            
            __IO uint32_t  RESERVED4                :  1;   /*!< RESERVED                                                   */            
            __IO uint32_t  HXTAL_RDY                :  1;   /*!< HXTAL Ready Flag                                           */
            __IO uint32_t  RESERVED2                :  1;   /*!< RESERVED                                                   */
            __IO uint32_t  SPLL_RDY                 :  1;   /*!< System PLL Ready Flag                                      */                                        
            __IO uint32_t  RESERVED1                :  25;  /*!< RESERVED                                                   */    
        } SYS0_CSST_b;
    };

    union {
        __IO uint32_t  SYS_CLKCFG;                  /*!< Offset:0x0C System CLK Control Register                             */

        struct {        
            __IO uint32_t  SYSCLKSEL                :  3;   /*!< System CLK source select                                   */                                                                    
            __IO uint32_t  RESERVED1                :  1;   /*!< RESERVED                                                   */
            __IO uint32_t  SYSCLKST                 :  3;   /*!<                                                            */                            
            __IO uint32_t  RESERVED0                : 25;   /*!< RESERVED                                                   */
        } SYS_CLKCFG_b;
    };    
    
    __IO uint32_t  SYS_AHBCP;                       /*!< Offset:0x10 AHB Clock Prescale register                            */
    
    union {
        __IO uint32_t  SYS0_RSTST;                  /*!< Offset:0x14 Control Register                                       */

        struct {        
            __IO uint32_t  SWRSTF                   :  1;   /*!< Software Reset                                             */                                            
            __IO uint32_t  WDTRSTF                  :  1;   /*!< WDT0 Reset                                                 */                                                    
            __IO uint32_t  LVRRSTF                  :  1;   /*!< LVD Reset                                                  */                                            
            __IO uint32_t  DPDRSTF                  :  1;   /*!< RESERVED                                                   */
            __IO uint32_t  PORRSTF                  :  1;   /*!< POR Reset                                                  */                                            
            __IO uint32_t  RESERVED0                : 27;   /*!< RESERVED                                                   */
        } SYS0_RSTST_b;
    };

    union {
        __IO uint32_t  LVD_CTRL;                    /*!< Offset:0x18 LVD Control Register                                   */

        struct {        
            __IO uint32_t  LVD11EN                  :  1;   /*!< 1.1v LVD Enable                                            */                                            
            __IO uint32_t  LVD33EN                  :  1;   /*!< 3.3v LVD Enable                                            */                                                                
            __IO uint32_t  DISCHARGE_PD             :  1;   /*!< DISCHARGE_PD                                               */
            __IO uint32_t  LVD33VAL                 :  2;   /*!< 3.3v LVD value                                             */
            __IO uint32_t  LDO_ILRC_SEL             :  2;
            __IO uint32_t  LVD33FLAG                :  1;   /*!< Level Trigger                                              */
            __O  uint32_t  EFUSE_ERROR_FLAG         :  1;
            __O  uint32_t  RESERVED                 : 23;   /*!< RESERVED                                                   */
        } LVD_CTRL_b;
    };
    __IO uint32_t  RESERVED1C;                      /*!< Offset:0x1C RESERVED                                               */    


    union{
        __IO uint32_t  PINCTRL;                     /*!< Offset:0x20 Pin Control Register                                   */

        struct {

            __IO uint32_t   SDNF0                   :  1;   /*!< SDNF Pin Mapping                                           */
            __IO uint32_t   TFT_8080                :  1;   /*!< TFT/8080 Pin Mapping                                       */
            __IO uint32_t   RESERVED                :  1;   /*!< RESERVED                                                   */
            __IO uint32_t   HOST_PPC                :  1;   /*!< Host PPC Mapping                                           */
            __IO uint32_t   I2S4                    :  2;   /*!< I2S4 Pin Mapping                                           */
            __IO uint32_t   SWDV_0                  :  1;   /*!< SWO CPU Select                                             */
            __IO uint32_t   SWDV_1                  :  1;
            __IO uint32_t   SPIDMA0                 :  1;
            __IO uint32_t   OID_Sensor_IF           :  1;
            __IO uint32_t   PWMIO_0                 :  1;
            __IO uint32_t   PWMIO_1                 :  1;
            __IO uint32_t   PWMIO_8                 :  1;
            __IO uint32_t   PWMIO_9                 :  1;
            __IO uint32_t   PWMIO_10                :  1;
            __IO uint32_t   PWMIO_11                :  1;
            __IO uint32_t   PWMIO_18                :  1;
            __IO uint32_t   PWMIO_19                :  1;
            __IO uint32_t   PWMIO_20                :  1;
            __IO uint32_t   PWMIO_21                :  1;
            __IO uint32_t   PWMIO_22                :  1;
            __IO uint32_t   PWMIO_23                :  1;
            __IO uint32_t   I2C_SYNC_SW             :  1;
            __IO uint32_t   AudioIF2                :  1;
            __IO uint32_t   HOST_DEV_SEL            :  1;
            __IO uint32_t   USB_DBG_OPT             :  1;
            __IO uint32_t   USB_TSQU_SEL            :  1;
            __IO uint32_t   RESERVED0               :  5;   /*!< RESERVED                                                   */           
        }PINCTRL_b;
    };
    
    __IO uint32_t  SYS0_NDTCTRL;                    /*!< Offset:0x24 NDT Control register                                   */    
    __IO uint32_t  RESERVED28;                      /*!< Offset:0x28                                                        */    
    
    union {
        __IO uint32_t  SYS0_REMAP;                  /*!< Offset:0x2C Remap register                                         */

        struct {        
            __IO uint32_t  REMAP                    :  1;   /*!< Set to Remap PRAM and ROM                                  */
            __IO uint32_t  RESERVED0                :  31;  /*!< RESERVED                                                   */
        } SYS0_REMAP_b;
    };
    
    union {
        __IO uint32_t  FEUSE0;                      /*!< Offset:0x30 E-FUSE 0 Register                                      */

        struct {
            __O  uint32_t  LVD33                    :  4; 
            __O  uint32_t  LVR33                    :  2;
            __O  uint32_t  RESERVED0                : 10;
            __O  uint32_t  LVD11                    :  3;
        } FEUSE0_b;
    };
    
    union {
        __IO uint32_t  FEUSE1;                      /*!< Offset:0x34 E-FUSE 1 Register                                      */

        struct {        
            __O  uint32_t  IHRC_TRIM                :  8;
            __O  uint32_t  LDO11                    :  4;
            __O  uint32_t  LDO18                    :  4;
            __O  uint32_t  USB_PHY1                 :  8;
            __O  uint32_t  USB_PHY0                 :  8;
        } FEUSE1_b;                                         
    };
    
    union {
        __IO uint32_t  FEUSE2;                      /*!< Offset:0x38 E-FUSE 2 Register                                      */

        struct {        
            __O  uint32_t  AES_Key                  : 16;
            __O  uint32_t  Reserved                 : 14;
            __O  uint32_t  USB_DISABLE              :  1;
            __O  uint32_t  SECURITY                 :  1;
        } FEUSE2_b;
    };
    
  __IO uint32_t  FEUSE3;                            /*!< Offset:0x3C E-FUSE 3 Register                                      */
  __IO uint32_t  Reserved[3];                       /*!< Offset:0x40 - 0x4C Reserved                                        */  

    union {
        __IO uint32_t  NAP;                         /*!< Offset:0x50 RAM Power Down Register                                */

        struct {        
            __IO uint32_t  MATRIX_RAM               :  8;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  MAILBOX_RAM              :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  CACHE_RAM                :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  USB_HOST1                :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  USB_HOST2                :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  USB_DEV                  :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  PPU_RAM                  :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  CSC_RAM                  :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  JPEG_RAM_NAP             :  1;   /*!< 1: NAP, 0: Disable                                         */
            __IO uint32_t  RESERVED0                :  16;  /*!< RESERVED                                                   */
        } NAP_b;
    };  
    union {
        __IO uint32_t  DMA_CH_SEL;                  /*!< Offset:0x54 DDR Channel control register                           */

        struct {        
            __IO uint32_t  DMA1_OUT                 :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  DMA1_IN                  :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  JPEG_D_IN                :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  JPEG_E_OUT               :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  DMA0_OUT                 :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  DMA0_IN                  :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  CIS                      :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  PPU                      :  2;   /*!< 11:CH2, 10:CH1, 01:CH0, 00:None                            */
            __IO uint32_t  RESERVED0                :  16;  /*!< RESERVED                                                   */
        } DMA_CH_SEL_b;
    };  
    
    __IO uint32_t  M3_TICK;                         /*!< Offset:0x58 CortexM3 Tick Control register  */

} SN_SYS0_Type;  

/* =========================================================================================================================== */
/* ====================================                   SN_SYS1                      ======================================= */
/* =========================================================================================================================== */
/**
  *@brief Peripheral Control Registers (SN_SYS1):Peripheral
*/

typedef struct {                                    /*!< SN_SYS1 Structure                                                  */
    union {
        __IO uint32_t  PCLK_EN;                     /*!< Offset:0x00 Peripheral Clock Enable Register                       */

        struct {        
            __IO uint32_t  CT32BCLKEN               :  1;   /*!< CT32BCLK CLK Enable                                        */
            __IO uint32_t  WDTCLKEN                 :  1;   /*!< WDT CLK Enable                                             */                        
            __IO uint32_t  APBCLKEN                 :  1;   /*!< APB CLK Enable                                             */                        
            __IO uint32_t  AUDIOCLKEN               :  1;   /*!< AUDIO CLK Enable                                           */                        
            __IO uint32_t  MSCLKEN                  :  1;   /*!< MS CLK Enable                                              */                                                
            __IO uint32_t  VIDEOCLKEN               :  1;   /*!< VIDEO CLK Enable                                           */                        
            __IO uint32_t  USBHOST1CLKEN            :  1;   /*!< USBHOST1 CLK Enable                                        */                        
            __IO uint32_t  USBHOST0CLKEN            :  1;   /*!< USBHOST0 CLK Enable                                        */
            __IO uint32_t  USBDEVCLKEN              :  1;   /*!< USBDEV CLK Enable                                          */
            __IO uint32_t  PPUCLKEN                 :  1;   /*!< PPU CLK Enable                                             */
            __IO uint32_t  EXTMEM                   :  1;   /*!< EXTMEM CLK Enable                                          */            
            __IO uint32_t  ICACHE                   :  1;   /*!< ICACHE CLK Enable                                          */
            __IO uint32_t  SARCLKEN                 :  1;   /*!< SAR CLK Enable                                             */
            __IO uint32_t  MCLK2EN                  :  1;   /*!< MCLK2 CLK Enable                                           */
            __IO uint32_t  MCLK1EN                  :  1;   /*!< MCLK1 CLK Enable                                           */
            __IO uint32_t  M3_1CLKEN                :  1;   /*!< M3_1 CLK Enable                                            */
            __O uint32_t   RESERVED0                :  16;  /*!< RESERVED                                                   */
        } PCLK_EN_b;
    };

    __IO uint32_t  RESERVED4;                       /*!< Offset:0x04 RESERVED8                                              */    

    union {
        __IO uint32_t  PERI_CLKCTL;                 /*!< Offset:0x08 Peripheral clock control register 1                    */

        struct {        
            __IO uint32_t  APBPRE                   :  3;     
            __IO uint32_t  Reserved3                :  2;                
            __IO uint32_t  SDNF_DIV                 :  2;    
            __IO uint32_t  Reserved7                :  1;    
            __IO uint32_t  CKOUT_DIV                :  3;    
            __IO uint32_t  Reserved11               :  1;    
            __IO uint32_t  CKOUT_SEL                :  3;    
            __IO uint32_t  Reserved15               :  17;    
        } PCLK_CTL_b;
    };
    
    union {
        __IO uint32_t  PCLK_RST;                    /*!< Offset:0x0C Peripheral Reset Control Register                      */

        struct {        
            __IO uint32_t  CT32BCLKRST              :  1;   /*!< CT32B/PWM  Clock Reset                                     */
            __IO uint32_t  WDTCLKRST                :  1;   /*!< WDT*2  Clock Reset                                         */
            __IO uint32_t  APBCLKRST                :  1;   /*!< SPI/GPIO/UART/I2C/GPIO    Clock Reset                      */
            __IO uint32_t  AUDIOCLKRST              :  1;   /*!< I2S*5  Clock Reset                                         */
            __IO uint32_t  MSCLKRST                 :  1;   /*!< IDMA0/IDMA1/SD/NF/SDIO/AES/SPIFlash/SPI0DMA/SPI1DMA/CRC Clock Reset */
            __IO uint32_t  VIDEOCLKRST              :  1;   /*!< JPEG/CSC/CIS   Clock Reset                                 */
            __IO uint32_t  USBHOST1CLKRST           :  1;   /*!< HOST1  Clock Reset                                         */
            __IO uint32_t  USBHOST0CLKRST           :  1;   /*!< HOST0  Clock Reset                                         */
            __IO uint32_t  USBDEVCLKRST             :  1;   /*!< USBDEV    Clock Reset                                      */                            
            __IO uint32_t  PPUCLKRST                :  1;   /*!< PPU    Clock Reset                                         */                            
            __IO uint32_t  EXT_MEM                  :  1;   /*!< Ext.Memory(DDR/PSRAM) Clock Reset                          */                                        
            __IO uint32_t  I_CACHE                  :  1;   /*!< I-Cache Clock    Reset                                     */                                        
            __IO uint32_t  SARCLKRST                :  1;   /*!< SAR    Clock Reset                                         */                                        
            __IO uint32_t  Reserved13               :  2;   
            __IO uint32_t  MCLK1RST                 :  1;   
            __IO uint32_t  M3_1CLKRST               :  1;   /*!< M3_1, FIR/FFT    Clock Reset                               */                                        
            __IO uint32_t  RESERVED0                :  16;  /*!< RESERVED                                                   */                                                                
        } PCLK_RST_b;
    };
    
    union {
        __IO uint32_t  USB_CTRL;                    /*!< Offset:0x10 USB SIE control register                               */

        struct {        
            __IO uint32_t  PHY0_PLL480En            :  1;   /*!< HostPLL480                                                 */
            __IO uint32_t  PHY1_PLL480En            :  1;   /*!< HostPLL480                                                 */
            __IO uint32_t  PHY0_PDN_MUX             :  1;   /*!< PHY_PDN_MUX                                                */
            __IO uint32_t  PHY0_CPUD_UTMI_PDN       :  1;   /*!< CPUD_UTMI_PDN                                              */
            __IO uint32_t  PHY1_PDN_MUX             :  1;   /*!< PHY_PDN_MUX                                                */
            __IO uint32_t  PHY1_CPUD_UTMI_PDN       :  1;   /*!< CPUD_UTMI_PDN                                              */
            __IO uint32_t  RESERVED6                :  2;
            __IO uint32_t  PHY_SEL                  :  2;
            __IO uint32_t  RESERVED10               : 23;
           
        } USB_CTRL_b;
    };    
    __IO uint32_t  LDO18_CTL;                       /*!< Offset:0x14 1.8V LDO control register                              */
    
    union {
        __IO uint32_t  AHBMUX_RST;                  /*!< Offset:0x18 USB PHY Control Register                               */
    
        struct {        
            __IO uint32_t  MUX0RST                  :  1;
            __IO uint32_t  MUX1RST                  :  1;
            __IO uint32_t  MUX2RST                  :  1;
        } AHBMUX_RST_b;
    };    
    
} SN_SYS1_Type;

/* =========================================================================================================================== */
/* ====================================                   SN_PMU                       ======================================= */
/* =========================================================================================================================== */
/**
  *@brief PMU Control Registers (SN_PMU)
*/

typedef struct {                                    /*!< SN_PMU Structure                                                   */

    union {
        __IO uint32_t  PMU_CTRL;                    /*!< Offset:0x00 PMU Control Register                                   */

        struct {
            __IO uint32_t  DPD_EN                   :  1;   /*!< DPDEN                                                      */
            __IO uint32_t  DSLEEP_EN                :  1;   /*!< DSLEEPEN                                                   */    
            __IO uint32_t  SLEEP_EN                 :  1;   /*!< SLEEPEN                                                    */                
            __IO uint32_t  RESERVED0                :  1;   /*!< RESERVED0                                                  */    
            __IO uint32_t  RTC_FLAG                 :  1;   /*!< RTC_Flag                                                   */
            __IO uint32_t  RESERVED1                :  1;   /*!< RESERVED                                                   */
            __IO uint32_t  WKP_FLAG                 :  1;   /*!< WKP_Flag                                                   */
            __IO uint32_t  CLR_WKP_INT              :  1;
            __IO uint32_t  RESERVED8                :  7;   /*!< RESERVED                                                   */            
            __IO uint32_t  WKP                      :  1;   /*!< WKP PAD                                                    */                        
            __IO uint32_t  RESERVED16               : 15;   /*!< RESERVED                                                   */    
            __IO uint32_t  RTC_CNT_FLAG             :  1;   /*!< RTC Counter Valid flag                                     */    
        } PMU_CTRL_b;
    };    
 
  __IO uint32_t  RTC_C;                             /*!< Offset:0x04 ISO RTC Counter value Register                         */
 
} SN_PMU_Type;


/* =========================================================================================================================== */
/* ====================================                 SN_ISOBlock                    ======================================= */
/* =========================================================================================================================== */
/**
  *@brief ISO Control Registers (SN_ISO)
*/

typedef struct {                                    /*!< SN_ISO Structure                                                   */
    union {
        __IO uint32_t  ISO_CTRL;                    /*!< Offset:0x00 ISO Control Register                                   */

        struct {        
            __IO uint32_t  RTC_Timeout              :  3;   /*!< RTC_Timeout                                                */
            __IO uint32_t  RTC_WKP                  :  1;   /*!< RTC WAKEUP                                                 */
            __IO uint32_t  LXTL_SpdRel              :  1;   /*!< LXTL Release speed up                                      */
            __IO uint32_t  LXTL_EN                  :  1;   /*!< LXTL EN                                                    */
            __IO uint32_t  RTC_SRC                  :  1;   /*!< RTC SRC SEL                                                */
            __IO uint32_t  RTC_EN                   :  1;   /*!< RTCEN                                                      */            
            __IO uint32_t  RESERVED8                : 24;   /*!< RESERVED                                                   */
        } ISO_CTRL_b;
    };
    union {
        __IO uint32_t  ISO_TRIM;                    /*!< Offset:0x04 RTC ILRC Trim register                                 */

        struct {        
            __IO uint32_t  Trim                     :  6;
            __IO uint32_t  RESERVED                 :  1;
            __IO uint32_t  RTC_Clear                :  1;
            __IO uint32_t  RESERVED0                :  16;
        } ISO_TRIM_b;
    };        

    __IO uint32_t  BK_REG0;                         /*!< Offset:0x08 Backup Register0                                       */
    __IO uint32_t  BK_REG1;                         /*!< Offset:0x0c Backup Register1                                       */
    __IO uint32_t  BK_REG2;                         /*!< Offset:0x10 Backup Register2                                       */
    __IO uint32_t  BK_REG3;                         /*!< Offset:0x14 Backup Register3                                       */
    __IO uint32_t  BK_REG4;                         /*!< Offset:0x18 Backup Register4                                       */
    __IO uint32_t  BK_REG5;                         /*!< Offset:0x1C Backup Register5                                       */
    __IO uint32_t  BK_REG6;                         /*!< Offset:0x20 Backup Register6                                       */
    __IO uint32_t  BK_REG7;                         /*!< Offset:0x24 Backup Register7                                       */
    __IO uint32_t  BK_REG8;                         /*!< Offset:0x28 Backup Register8                                       */
    __IO uint32_t  BK_REG9;                         /*!< Offset:0x2C Backup Register9                                       */
    __IO uint32_t  BK_REG10;                        /*!< Offset:0x30 Backup Register10                                      */
    __IO uint32_t  BK_REG11;                        /*!< Offset:0x34 Backup Register11                                      */
    __IO uint32_t  BK_REG12;                        /*!< Offset:0x38 Backup Register12                                      */
    __IO uint32_t  BK_REG13;                        /*!< Offset:0x3C Backup Register13                                      */
    __IO uint32_t  BK_REG16;                        /*!< Offset:0x40 Backup Register16                                      */
    __IO uint32_t  BK_REG17;                        /*!< Offset:0x44 Backup Register17                                      */    
    __IO uint32_t  BK_REG18;                        /*!< Offset:0x48 Backup Register18                                      */
    __IO uint32_t  BK_REG19;                        /*!< Offset:0x4C Backup Register19                                      */
    __IO uint32_t  BK_REG20;                        /*!< Offset:0x50 Backup Register20                                      */
    __IO uint32_t  BK_REG21;                        /*!< Offset:0x54 Backup Register21                                      */
    __IO uint32_t  BK_REG22;                        /*!< Offset:0x58 Backup Register22                                      */
} SN_ISO_Type;


/* =========================================================================================================================== */
/* ====================================                    SN_GPIO0                    ======================================= */
/* =========================================================================================================================== */
/**
  * @brief General Purpose I/O (SN_GPIO0)
  */

typedef struct {                                    /*!< SN_GPIO0 Structure                                                    */
  
  union {
    __IO uint32_t  DATA;                            /*!< Offset:0x00 GPIO Port n Data Register                                 */
    
    struct {
      __IO uint32_t  DATA0      :  1;               /*!< Data of Pn.0                                                          */
      __IO uint32_t  DATA1      :  1;               /*!< Data of Pn.1                                                          */
      __IO uint32_t  DATA2      :  1;               /*!< Data of Pn.2                                                          */
      __IO uint32_t  DATA3      :  1;               /*!< Data of Pn.3                                                          */
      __IO uint32_t  DATA4      :  1;               /*!< Data of Pn.4                                                          */
      __IO uint32_t  DATA5      :  1;               /*!< Data of Pn.5                                                          */
      __IO uint32_t  DATA6      :  1;               /*!< Data of Pn.6                                                          */
      __IO uint32_t  DATA7      :  1;               /*!< Data of Pn.7                                                          */
      __IO uint32_t  DATA8      :  1;               /*!< Data of Pn.8                                                          */
      __IO uint32_t  DATA9      :  1;               /*!< Data of Pn.9                                                          */
      __IO uint32_t  DATA10     :  1;               /*!< Data of Pn.10                                                         */
      __IO uint32_t  DATA11     :  1;               /*!< Data of Pn.11                                                         */
      __IO uint32_t  DATA12     :  1;               /*!< Data of Pn.12                                                         */
      __IO uint32_t  DATA13     :  1;               /*!< Data of Pn.13                                                         */
      __IO uint32_t  DATA14     :  1;               /*!< Data of Pn.14                                                         */
      __IO uint32_t  DATA15     :  1;               /*!< Data of Pn.15                                                         */
    } DATA_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  MODE;                            /*!< Offset:0x04 GPIO Port n Mode Register                                 */
    
    struct {
      __IO uint32_t  MODE0      :  1;               /*!< Mode of Pn.0                                                          */
      __IO uint32_t  MODE1      :  1;               /*!< Mode of Pn.1                                                          */
      __IO uint32_t  MODE2      :  1;               /*!< Mode of Pn.2                                                          */
      __IO uint32_t  MODE3      :  1;               /*!< Mode of Pn.3                                                          */
      __IO uint32_t  MODE4      :  1;               /*!< Mode of Pn.4                                                          */
      __IO uint32_t  MODE5      :  1;               /*!< Mode of Pn.5                                                          */
      __IO uint32_t  MODE6      :  1;               /*!< Mode of Pn.6                                                          */
      __IO uint32_t  MODE7      :  1;               /*!< Mode of Pn.7                                                          */
      __IO uint32_t  MODE8      :  1;               /*!< Mode of Pn.8                                                          */
      __IO uint32_t  MODE9      :  1;               /*!< Mode of Pn.9                                                          */
      __IO uint32_t  MODE10     :  1;               /*!< Mode of Pn.10                                                         */
      __IO uint32_t  MODE11     :  1;               /*!< Mode of Pn.11                                                         */
      __IO uint32_t  MODE12     :  1;               /*!< Mode of Pn.12                                                         */
      __IO uint32_t  MODE13     :  1;               /*!< Mode of Pn.13                                                         */
      __IO uint32_t  MODE14     :  1;               /*!< Mode of Pn.14                                                         */
      __IO uint32_t  MODE15     :  1;               /*!< Mode of Pn.15                                                         */
    } MODE_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  CFG;                             /*!< Offset:0x08 GPIO Port n Configuration Register                        */
    
    struct {
      __IO uint32_t  CFG0       :  2;               /*!< Configuration of Pn.0                                                 */
      __IO uint32_t  CFG1       :  2;               /*!< Configuration of Pn.1                                                 */
      __IO uint32_t  CFG2       :  2;               /*!< Configuration of Pn.2                                                 */
      __IO uint32_t  CFG3       :  2;               /*!< Configuration of Pn.3                                                 */
      __IO uint32_t  CFG4       :  2;               /*!< Configuration of Pn.4                                                 */
      __IO uint32_t  CFG5       :  2;               /*!< Configuration of Pn.5                                                 */
      __IO uint32_t  CFG6       :  2;               /*!< Configuration of Pn.6                                                 */
      __IO uint32_t  CFG7       :  2;               /*!< Configuration of Pn.7                                                 */
      __IO uint32_t  CFG8       :  2;               /*!< Configuration of Pn.8                                                 */
      __IO uint32_t  CFG9       :  2;               /*!< Configuration of Pn.9                                                 */
      __IO uint32_t  CFG10      :  2;               /*!< Configuration of Pn.10                                                */
      __IO uint32_t  CFG11      :  2;               /*!< Configuration of Pn.11                                                */
      __IO uint32_t  CFG12      :  2;               /*!< Configuration of Pn.12                                                */
      __IO uint32_t  CFG13      :  2;               /*!< Configuration of Pn.13                                                */
      __IO uint32_t  CFG14      :  2;               /*!< Configuration of Pn.14                                                */
      __IO uint32_t  CFG15      :  2;               /*!< Configuration of Pn.15                                                */
    } CFG_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IS;                              /*!< Offset:0x0C GPIO Port n Interrupt Sense Register                      */
    
    struct {
      __IO uint32_t  IS0        :  1;               /*!< Interrupt on Pn.0 is event or edge sensitive                          */
      __IO uint32_t  IS1        :  1;               /*!< Interrupt on Pn.1 is event or edge sensitive                          */
      __IO uint32_t  IS2        :  1;               /*!< Interrupt on Pn.2 is event or edge sensitive                          */
      __IO uint32_t  IS3        :  1;               /*!< Interrupt on Pn.3 is event or edge sensitive                          */
      __IO uint32_t  IS4        :  1;               /*!< Interrupt on Pn.4 is event or edge sensitive                          */
      __IO uint32_t  IS5        :  1;               /*!< Interrupt on Pn.5 is event or edge sensitive                          */
      __IO uint32_t  IS6        :  1;               /*!< Interrupt on Pn.6 is event or edge sensitive                          */
      __IO uint32_t  IS7        :  1;               /*!< Interrupt on Pn.7 is event or edge sensitive                          */
      __IO uint32_t  IS8        :  1;               /*!< Interrupt on Pn.8 is event or edge sensitive                          */
      __IO uint32_t  IS9        :  1;               /*!< Interrupt on Pn.9 is event or edge sensitive                          */
      __IO uint32_t  IS10       :  1;               /*!< Interrupt on Pn.10 is event or edge sensitive                         */
      __IO uint32_t  IS11       :  1;               /*!< Interrupt on Pn.11 is event or edge sensitive                         */
      __IO uint32_t  IS12       :  1;               /*!< Interrupt on Pn.12 is event or edge sensitive                         */
      __IO uint32_t  IS13       :  1;               /*!< Interrupt on Pn.13 is event or edge sensitive                         */
      __IO uint32_t  IS14       :  1;               /*!< Interrupt on Pn.14 is event or edge sensitive                         */
      __IO uint32_t  IS15       :  1;               /*!< Interrupt on Pn.15 is event or edge sensitive                         */
    } IS_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IBS;                             /*!< Offset:0x10 GPIO Port n Interrupt Both-edge Sense Register            */
    
    struct {
      __IO uint32_t  IBS0       :  1;               /*!< Interrupt on Pn.0 is triggered ob both edges                          */
      __IO uint32_t  IBS1       :  1;               /*!< Interrupt on Pn.1 is triggered ob both edges                          */
      __IO uint32_t  IBS2       :  1;               /*!< Interrupt on Pn.2 is triggered ob both edges                          */
      __IO uint32_t  IBS3       :  1;               /*!< Interrupt on Pn.3 is triggered ob both edges                          */
      __IO uint32_t  IBS4       :  1;               /*!< Interrupt on Pn.4 is triggered ob both edges                          */
      __IO uint32_t  IBS5       :  1;               /*!< Interrupt on Pn.5 is triggered ob both edges                          */
      __IO uint32_t  IBS6       :  1;               /*!< Interrupt on Pn.6 is triggered ob both edges                          */
      __IO uint32_t  IBS7       :  1;               /*!< Interrupt on Pn.7 is triggered ob both edges                          */
      __IO uint32_t  IBS8       :  1;               /*!< Interrupt on Pn.8 is triggered ob both edges                          */
      __IO uint32_t  IBS9       :  1;               /*!< Interrupt on Pn.9 is triggered ob both edges                          */
      __IO uint32_t  IBS10      :  1;               /*!< Interrupt on Pn.10 is triggered ob both edges                         */
      __IO uint32_t  IBS11      :  1;               /*!< Interrupt on Pn.11 is triggered ob both edges                         */
      __IO uint32_t  IBS12      :  1;               /*!< Interrupt on Pn.12 is triggered ob both edges                         */
      __IO uint32_t  IBS13      :  1;               /*!< Interrupt on Pn.13 is triggered ob both edges                         */
      __IO uint32_t  IBS14      :  1;               /*!< Interrupt on Pn.14 is triggered ob both edges                         */
      __IO uint32_t  IBS15      :  1;               /*!< Interrupt on Pn.15 is triggered ob both edges                         */
    } IBS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IEV;                             /*!< Offset:0x14 GPIO Port n Interrupt Event Register                      */
    
    struct {
      __IO uint32_t  IEV0       :  1;               /*!< Interrupt trigged evnet on Pn.0                                       */
      __IO uint32_t  IEV1       :  1;               /*!< Interrupt trigged evnet on Pn.1                                       */
      __IO uint32_t  IEV2       :  1;               /*!< Interrupt trigged evnet on Pn.2                                       */
      __IO uint32_t  IEV3       :  1;               /*!< Interrupt trigged evnet on Pn.3                                       */
      __IO uint32_t  IEV4       :  1;               /*!< Interrupt trigged evnet on Pn.4                                       */
      __IO uint32_t  IEV5       :  1;               /*!< Interrupt trigged evnet on Pn.5                                       */
      __IO uint32_t  IEV6       :  1;               /*!< Interrupt trigged evnet on Pn.6                                       */
      __IO uint32_t  IEV7       :  1;               /*!< Interrupt trigged evnet on Pn.7                                       */
      __IO uint32_t  IEV8       :  1;               /*!< Interrupt trigged evnet on Pn.8                                       */
      __IO uint32_t  IEV9       :  1;               /*!< Interrupt trigged evnet on Pn.9                                       */
      __IO uint32_t  IEV10      :  1;               /*!< Interrupt trigged evnet on Pn.10                                      */
      __IO uint32_t  IEV11      :  1;               /*!< Interrupt trigged evnet on Pn.11                                      */
      __IO uint32_t  IEV12      :  1;               /*!< Interrupt trigged evnet on Pn.12                                      */
      __IO uint32_t  IEV13      :  1;               /*!< Interrupt trigged evnet on Pn.13                                      */
      __IO uint32_t  IEV14      :  1;               /*!< Interrupt trigged evnet on Pn.14                                      */
      __IO uint32_t  IEV15      :  1;               /*!< Interrupt trigged evnet on Pn.15                                      */
    } IEV_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IE;                              /*!< Offset:0x18 GPIO Port n Interrupt Enable Register                     */
    
    struct {
      __IO uint32_t  IE0        :  1;               /*!< Interrupt on Pn.0 enable                                              */
      __IO uint32_t  IE1        :  1;               /*!< Interrupt on Pn.1 enable                                              */
      __IO uint32_t  IE2        :  1;               /*!< Interrupt on Pn.2 enable                                              */
      __IO uint32_t  IE3        :  1;               /*!< Interrupt on Pn.3 enable                                              */
      __IO uint32_t  IE4        :  1;               /*!< Interrupt on Pn.4 enable                                              */
      __IO uint32_t  IE5        :  1;               /*!< Interrupt on Pn.5 enable                                              */
      __IO uint32_t  IE6        :  1;               /*!< Interrupt on Pn.6 enable                                              */
      __IO uint32_t  IE7        :  1;               /*!< Interrupt on Pn.7 enable                                              */
      __IO uint32_t  IE8        :  1;               /*!< Interrupt on Pn.8 enable                                              */
      __IO uint32_t  IE9        :  1;               /*!< Interrupt on Pn.9 enable                                              */
      __IO uint32_t  IE10       :  1;               /*!< Interrupt on Pn.10 enable                                             */
      __IO uint32_t  IE11       :  1;               /*!< Interrupt on Pn.11 enable                                             */
      __IO uint32_t  IE12       :  1;               /*!< Interrupt on Pn.11 enable                                             */
      __IO uint32_t  IE13       :  1;               /*!< Interrupt on Pn.13 enable                                             */
      __IO uint32_t  IE14       :  1;               /*!< Interrupt on Pn.14 enable                                             */
      __IO uint32_t  IE15       :  1;               /*!< Interrupt on Pn.15 enable                                             */
    } IE_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  RIS;                             /*!< Offset:0x1C GPIO Port n Raw Interrupt Status Register                 */
    
    struct {
      __I  uint32_t  IF0        :  1;               /*!< Pn.0 raw interrupt flag                                               */
      __I  uint32_t  IF1        :  1;               /*!< Pn.1 raw interrupt flag                                               */
      __I  uint32_t  IF2        :  1;               /*!< Pn.2 raw interrupt flag                                               */
      __I  uint32_t  IF3        :  1;               /*!< Pn.3 raw interrupt flag                                               */
      __I  uint32_t  IF4        :  1;               /*!< Pn.4 raw interrupt flag                                               */
      __I  uint32_t  IF5        :  1;               /*!< Pn.5 raw interrupt flag                                               */
      __I  uint32_t  IF6        :  1;               /*!< Pn.6 raw interrupt flag                                               */
      __I  uint32_t  IF7        :  1;               /*!< Pn.7 raw interrupt flag                                               */
      __I  uint32_t  IF8        :  1;               /*!< Pn.8 raw interrupt flag                                               */
      __I  uint32_t  IF9        :  1;               /*!< Pn.9 raw interrupt flag                                               */
      __I  uint32_t  IF10       :  1;               /*!< Pn.10 raw interrupt flag                                              */
      __I  uint32_t  IF11       :  1;               /*!< Pn.11 raw interrupt flag                                              */
      __I  uint32_t  IF12       :  1;               /*!< Pn.12 raw interrupt flag                                              */
      __I  uint32_t  IF13       :  1;               /*!< Pn.13 raw interrupt flag                                              */
      __I  uint32_t  IF14       :  1;               /*!< Pn.14 raw interrupt flag                                              */
      __I  uint32_t  IF15       :  1;               /*!< Pn.15 raw interrupt flag                                              */
    } RIS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  IC;                              /*!< Offset:0x20 GPIO Port n Interrupt Clear Register                      */
    
    struct {
      __O  uint32_t  IC0        :  1;               /*!< Pn.0 interrupt flag clear                                             */
      __O  uint32_t  IC1        :  1;               /*!< Pn.1 interrupt flag clear                                             */
      __O  uint32_t  IC2        :  1;               /*!< Pn.2 interrupt flag clear                                             */
      __O  uint32_t  IC3        :  1;               /*!< Pn.3 interrupt flag clear                                             */
      __O  uint32_t  IC4        :  1;               /*!< Pn.4 interrupt flag clear                                             */
      __O  uint32_t  IC5        :  1;               /*!< Pn.5 interrupt flag clear                                             */
      __O  uint32_t  IC6        :  1;               /*!< Pn.6 interrupt flag clear                                             */
      __O  uint32_t  IC7        :  1;               /*!< Pn.7 interrupt flag clear                                             */
      __O  uint32_t  IC8        :  1;               /*!< Pn.8 interrupt flag clear                                             */
      __O  uint32_t  IC9        :  1;               /*!< Pn.9 interrupt flag clear                                             */
      __O  uint32_t  IC10       :  1;               /*!< Pn.10 interrupt flag clear                                            */
      __O  uint32_t  IC11       :  1;               /*!< Pn.11 interrupt flag clear                                            */
      __O  uint32_t  IC12       :  1;               /*!< Pn.12 interrupt flag clear                                            */
      __O  uint32_t  IC13       :  1;               /*!< Pn.13 interrupt flag clear                                            */
      __O  uint32_t  IC14       :  1;               /*!< Pn.14 interrupt flag clear                                            */
      __O  uint32_t  IC15       :  1;               /*!< Pn.15 interrupt flag clear                                            */
    } IC_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  BSET;                            /*!< Offset:0x24 GPIO Port n Bits Set Operation Register                   */
    
    struct {
      __O  uint32_t  BSET0      :  1;               /*!< Set Pn.0                                                              */
      __O  uint32_t  BSET1      :  1;               /*!< Set Pn.1                                                              */
      __O  uint32_t  BSET2      :  1;               /*!< Set Pn.2                                                              */
      __O  uint32_t  BSET3      :  1;               /*!< Set Pn.3                                                              */
      __O  uint32_t  BSET4      :  1;               /*!< Set Pn.4                                                              */
      __O  uint32_t  BSET5      :  1;               /*!< Set Pn.5                                                              */
      __O  uint32_t  BSET6      :  1;               /*!< Set Pn.6                                                              */
      __O  uint32_t  BSET7      :  1;               /*!< Set Pn.7                                                              */
      __O  uint32_t  BSET8      :  1;               /*!< Set Pn.8                                                              */
      __O  uint32_t  BSET9      :  1;               /*!< Set Pn.9                                                              */
      __O  uint32_t  BSET10     :  1;               /*!< Set Pn.10                                                             */
      __O  uint32_t  BSET11     :  1;               /*!< Set Pn.11                                                             */
      __O  uint32_t  BSET12     :  1;               /*!< Set Pn.12                                                             */
      __O  uint32_t  BSET13     :  1;               /*!< Set Pn.13                                                             */
      __O  uint32_t  BSET14     :  1;               /*!< Set Pn.14                                                             */
      __O  uint32_t  BSET15     :  1;               /*!< Set Pn.15                                                             */
    } BSET_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  BCLR;                            /*!< Offset:0x28 GPIO Port n Bits Clear Operation Register                 */
    
    struct {
      __O  uint32_t  BCLR0      :  1;               /*!< Clear Pn.0                                                            */
      __O  uint32_t  BCLR1      :  1;               /*!< Clear Pn.1                                                            */
      __O  uint32_t  BCLR2      :  1;               /*!< Clear Pn.2                                                            */
      __O  uint32_t  BCLR3      :  1;               /*!< Clear Pn.3                                                            */
      __O  uint32_t  BCLR4      :  1;               /*!< Clear Pn.4                                                            */
      __O  uint32_t  BCLR5      :  1;               /*!< Clear Pn.5                                                            */
      __O  uint32_t  BCLR6      :  1;               /*!< Clear Pn.6                                                            */
      __O  uint32_t  BCLR7      :  1;               /*!< Clear Pn.7                                                            */
      __O  uint32_t  BCLR8      :  1;               /*!< Clear Pn.8                                                            */
      __O  uint32_t  BCLR9      :  1;               /*!< Clear Pn.9                                                            */
      __O  uint32_t  BCLR10     :  1;               /*!< Clear Pn.10                                                           */
      __O  uint32_t  BCLR11     :  1;               /*!< Clear Pn.11                                                           */
      __O  uint32_t  BCLR12     :  1;               /*!< Clear Pn.12                                                           */
      __O  uint32_t  BCLR13     :  1;               /*!< Clear Pn.13                                                           */
      __O  uint32_t  BCLR14     :  1;               /*!< Clear Pn.14                                                           */
      __O  uint32_t  BCLR15     :  1;               /*!< Clear Pn.15                                                           */
    } BCLR_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  ODCTRL;                          /*!< Offset:0x2C GPIO Port n Open-drain Control Register                   */
    
    struct {
      __IO uint32_t  Pn0OC      :  1;               /*!< Pn.0 open-drain control                                               */
      __IO uint32_t  Pn1OC      :  1;               /*!< Pn.1 open-drain control                                               */
      __IO uint32_t  Pn2OC      :  1;               /*!< Pn.2 open-drain control                                               */
      __IO uint32_t  Pn3OC      :  1;               /*!< Pn.3 open-drain control                                               */
           uint32_t             :  8;
      __IO uint32_t  Pn12OC     :  1;               /*!< Pn.12 open-drain control                                              */
      __IO uint32_t  Pn13OC     :  1;               /*!< Pn.13 open-drain control                                              */
      __IO uint32_t  Pn14OC     :  1;               /*!< Pn.14 open-drain control                                              */
      __IO uint32_t  Pn15OC     :  1;               /*!< Pn.15 open-drain control                                              */
    } ODCTRL_b;                                     /*!< BitSize                                                               */
  };
} SN_GPIO0_Type;

/* =========================================================================================================================== */
/* ====================================                    SN_I2S4                     ======================================= */
/* =========================================================================================================================== */
/**
  * @brief I2S_2 (SN_I2S_2)
  */

typedef struct {                                    /*!< SN_I2S_2 Structure                                                    */
  
  union {
    __IO uint32_t  CTRL;                            /*!< Offset:0x00 I2S Control Register                                      */
    
    struct {
      __IO uint32_t  START      :  1;               /*!< Start Transmit/Receive                                                */
      __IO uint32_t  MUTE       :  1;               /*!< Mute enable                                                           */
      __IO uint32_t  MONO       :  1;               /*!< Mono/stereo selection                                                 */
      __IO uint32_t  MS         :  1;               /*!< Master/Slave selection bit                                            */
      __IO uint32_t  FORMAT     :  2;               /*!< I2S operation format                                                  */
      __IO uint32_t  TXEN       :  1;               /*!< Transmit enable bit                                                   */
      __O  uint32_t  RXEN       :  1;               /*!< Receiver enable bit                                                   */
      __O  uint32_t  CLRTXFIFO  :  1;               /*!< Clear I2S TX FIFO                                                     */
      __O  uint32_t  CLRRXFIFO  :  1;               /*!< Clear I2S RX FIFO                                                     */
      __IO uint32_t  DL         :  2;               /*!< Data length                                                           */
      __IO uint32_t  TXFIFOTH   :  3;               /*!< Tx FIFO threshold level                                               */
           uint32_t             :  1;
      __IO uint32_t  RXFIFOTH   :  3;               /*!< Rx FIFO threshold level                                               */
           uint32_t             :  1;
      __IO uint32_t  CHLENGTH   :  5;               /*!< Bit number of single channel                                          */
           uint32_t             :  6;
      __IO uint32_t  I2SEN      :  1;               /*!< I2S enable                                                            */
    } CTRL_b;                                       /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  CLK;                             /*!< Offset:0x04 I2S Clock Register                                        */
    
    struct {
        __IO uint32_t  IMCLKDIV                 :  3;         		/*!< IMCLK -> IMCLKDIV -> BCLK, For BCLK PreScaler										*/
        __IO uint32_t  MCLKOEN					:  1;							/*!< 12M MCLK output enable										*/
        __IO uint32_t  IMCLKSEL					:  2;							/*!< I2S IP Clock Source Select																				*/
        __IO uint32_t  MCLK_OUT0_SEL            :  2;							/*!< Select MCLK Output Pad 0 clock source, For Aud Chip Clock				*/
        __IO uint32_t  BCLKDIV                  :  8;         		/*!< BCLK -> BCLKDIV -> BCLK PAD, BCLK divider												*/
        __IO uint32_t  FPLL_DIV    			    :  2;         		/*!< 00: FPLL VCO/9, 01:FPLL VCO/10 10:FPLL VCO/14										*/
        __IO uint32_t  MCLK_OUT1_SEL            :  2;							/*!< Select MCLK Output Pad 1 clock source, For Aud Chip Clock				*/
        __I  uint32_t  RESERVED01               :  12;
    } CLK_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  STATUS;                          /*!< Offset:0x08 I2S Status Register                                       */
    
    struct {
      __I  uint32_t  I2SINT     :  1;               /*!< I2S interrupt flag                                                    */
      __I  uint32_t  RIGHTCH    :  1;               /*!< Current channel status                                                */
           uint32_t             :  4;
      __I  uint32_t  TXFIFOTHF  :  1;               /*!< TX FIFO threshold flag                                                */
      __I  uint32_t  RXFIFOTHF  :  1;               /*!< RX FIFO threshold flag                                                */
      __I  uint32_t  TXFIFOFULL :  1;               /*!< TX FIFO full flag                                                     */
      __I  uint32_t  RXFIFOFULL :  1;               /*!< RX FIFO full flag                                                     */
      __I  uint32_t  TXFIFOEMPTY:  1;               /*!< TX FIFO empty flag                                                    */
      __I  uint32_t  RXFIFOEMPTY:  1;               /*!< RX FIFO empty flag                                                    */
      __I  uint32_t  TXFIFOLV   :  4;               /*!< TX FIFO used level                                                    */
           uint32_t             :  1;
      __I  uint32_t  RXFIFOLV   :  4;               /*!< RX FIFO used level                                                    */
    } STATUS_b;                                     /*!< BitSize                                                               */
  };
  
  union {
    __IO uint32_t  IE;                              /*!< Offset:0x0C I2S Interrupt Enable Register                             */
    
    struct {
           uint32_t             :  4;
      __IO uint32_t  TXFIFOOVFIEN:  1;              /*!< TX FIFO overflow interrupt enable                                     */
      __IO uint32_t  RXFIFOUDFIEN:  1;              /*!< RX FIFO underflow interrupt enable                                    */
      __IO uint32_t  TXFIFOTHIEN:  1;               /*!< TX FIFO threshold interrupt enable                                    */
      __IO uint32_t  RXFIFOTHIEN:  1;               /*!< RX FIFO threshold interrupt enable                                    */
    } IE_b;                                         /*!< BitSize                                                               */
  };
  
  union {
    __I  uint32_t  RIS;                             /*!< Offset:0x10 I2S Raw Interrupt Status Register                         */
    
    struct {
           uint32_t             :  4;
      __I  uint32_t  TXFIFOOVIF :  1;               /*!< TX FIFO overflow interrupt flag                                       */
      __I  uint32_t  RXFIFOUDIF :  1;               /*!< RX FIFO underflow interrupt flag                                      */
      __I  uint32_t  TXFIFOTHIF :  1;               /*!< TX FIFO threshold interrupt flag                                      */
      __I  uint32_t  RXFIFOTHIF :  1;               /*!< RX FIFO threshold interrupt flag                                      */
    } RIS_b;                                        /*!< BitSize                                                               */
  };
  
  union {
    __O  uint32_t  IC;                              /*!< Offset:0x14 I2S Interrupt Clear Register                              */
    
    struct {
           uint32_t             :  4;
      __O  uint32_t  TXFIFOOVIC :  1;               /*!< TX FIFO overflow interrupt clear                                      */
      __O  uint32_t  RXFIFOUDIC :  1;               /*!< RX FIFO underflow interrupt clear                                     */
      __O  uint32_t  TXFIFOTHIC :  1;               /*!< TX FIFO threshold interrupt clear                                     */
      __O  uint32_t  RXFIFOTHIC :  1;               /*!< RX FIFO threshold interrupt clear                                     */
    } IC_b;                                         /*!< BitSize                                                               */
  };
	
	__I  uint32_t  RXFIFO;                          	/*!< Offset:0x18 UARTn RX FIFO Register                                    */
	__O  uint32_t  TXFIFO;                            /*!< Offset:0x1C UARTn TX FIFO Register                                    */
	
  
} SN_I2S4_Type;

#define SN_I2S4_BASE                    0x40034000UL	//SNC7320 NEW
#define SN_SYS0_BASE                    0x45000000UL //SNC7320 NEW
#define SN_SYS1_BASE                    0x45000100UL //SNC7320 NEW SN_PERI_BASE
#define SN_PMU_BASE                     0x45000300UL
#define SN_ISO_BASE                     0x45000200UL //SNC7320 NEW
#define SN_GPIO0_BASE                   0x40018000UL

#define SN_SYS0                         ((SN_SYS0_Type          *) SN_SYS0_BASE)
#define SN_SYS1                         ((SN_SYS1_Type          *) SN_SYS1_BASE)
#define SN_PMU                          ((SN_PMU_Type           *) SN_PMU_BASE)
#define SN_ISO                          ((SN_ISO_Type           *) SN_ISO_BASE)
#define SN_GPIO0                        ((SN_GPIO0_Type         *) SN_GPIO0_BASE)
#define SN_I2S4                         ((SN_I2S4_Type            *) SN_I2S4_BASE)


/*IP MCLK Output Pad0 Pad1 Option*/
#define	MCLK_PAD_OUTPUT_IHRC_CLK		0
#define	MCLK_PAD_OUTPUT_XTAL_CLK		2
#define	MCLK_PAD_OUTPUT_I2S4_CLK		1

typedef enum LVD33_SEL
{
    LVD24v = 0,
    LVD26v,
    LVD28v,
    LVD30v,
}LVD33_SEL_t;

typedef enum TEST_MODE_SWT
{
    T_DFT = 0,
    T_RAM_ROM_BIST,
    T_TEST_DSP_M0,
    T_ROM_CODE,
    T_SAR_ADC_0,
    T_LVD_LVR,
    T_FRN_PLL,
    T_LDO,
    T_RTC33V,
    T_ILRC256K,
    T_IHRC12M,
    T_LPF,
    T_EFUSE,
    T_USB_UTMI,
}TEST_MODE_SWT_t;

typedef enum CKOUT_SEL
{
    GPIO_014_EN = 0,
    GPIO_014_ILRC_DIV2,
    GPIO_014_LXTL_DIV2,
    GPIO_014_PLL_DIV2,
    GPIO_014_SYS_DIV2,
    GPIO_014_IHRC_DIV2,
    GPIO_014_HXTL_DIV2,
}CKOUT_SEL_t;

typedef enum CKOUT_DIV
{
    HCLK_DIV1 = 0,
    HCLK_DIV2,
    HCLK_DIV4,
    HCLK_DIV8,
    HCLK_DIV16,
}CKOUT_DIV_t;

enum sys_clk_sel
{
    SYS_IHRC = 0,
    SYS_ILRC,
    SYS_HXTAL,
    SYS_LXTAL,
    SYS_SPLL,
};

enum rtc_tick
{
    rtc_500ms = 0,
    rtc_1s,
    rtc_5s=4,
    rtc_10s,
    rtc_30s,
    rtc_60s,
};

enum rtc_src_sel
{
    RTC_ILRC = 0,
    RTC_LXTAL,
};

enum ahb_clk_div
{
    AHB_DIV1 = 0,
    AHB_DIV2,
    AHB_DIV4,
    AHB_DIV8,
    AHB_DIV16,
    AHB_DIV32,
    AHB_DIV64,
    AHB_DIV128,
};

#endif


