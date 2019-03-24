#ifndef __io_pin_mapping_H_wdfR99A5_lOAW_HG5a_swbf_uYkNdWroKhtZ__
#define __io_pin_mapping_H_wdfR99A5_lOAW_HG5a_swbf_uYkNdWroKhtZ__

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>

//=============================================================================
// <h>                 Module Select
//=============================================================================
#include "io_pins_definitions.h"

/************************************* JTag ******/
// <e> SWD
// <i> SWD pin: P4.5,P4.6,P4.7
#define CONFIG_ENABLE_SWD       1
// </e>

/************************************* Uart ******/
// <h> Uart
// <e> Uart 0
// <i> UART_0 pin: P0.2,P0.3
#define CONFIG_ENABLE_UART_0      1
// </e>

// <e> Uart 1
// <i> UART_1 pin: P4.0,P4.1
#define CONFIG_ENABLE_UART_1      0
// </e>
// </h>

/************************************* I2C ******/
// <h> I2C
// <e> I2C 0
// <i> I2C_0 pin: P0.0,P0.1
#define CONFIG_ENABLE_I2C_0      0
// </e>

// <e> I2C 1
// <i> I2C_1 pin: P4.8,P4.9
#define CONFIG_ENABLE_I2C_1      0
// </e>


// <e> I2C 2
// <i> I2C_2 pin: P3.2,P3.3
#define CONFIG_ENABLE_I2C_2      0
// </e>
// </h>


/************************************* I2S ******/
// <h> I2S
// <e> I2S 0
// <i> I2S_0 pin: P4.10,P4.11,P4.12
#define CONFIG_ENABLE_I2S_0      0
// </e>

// <e> I2S 1
// <i> I2S_1 pin: P3.4,P3.5,P3.6
#define CONFIG_ENABLE_I2S_1      0
// </e>

// <e> I2S 2
// <i> I2S_2 pin: P4.13,P4.14,P4.15
#define CONFIG_ENABLE_I2S_2      0
// </e>

// <e> I2S 3
// <i> I2S_3 pin: P3.7,P3.8,P3.9
#define CONFIG_ENABLE_I2S_3      0
// </e>

// <e> I2S 4
// <i> I2S_4 pin: 
// <i> group1: map to P1.14~P2.3
// <i> group2: map to P3.2~P3.6
// <i> group3: map to P2.4~P2.8

#define CONFIG_ENABLE_I2S_4      0
// </e>
// </h>


/************************************* SPI ******/
// <h> SPI
// <e> SPI 0
// <i> SPI_0 pin: P0.14~P1.1
#define CONFIG_ENABLE_SPI_0      0
// </e>


// <e> SPI 1
// <i> SPI_1 pin: P3.12~P3.15
#define CONFIG_ENABLE_SPI_1      0
// </e>



/************************************* SPI DMA ******/
// <e> SPIDMA 0
// <i> SPIDMA0 pin: 
// <i> group1: map to P0.8~P0.13
// <i> group2: map to P1.4~P1.9
#define CONFIG_ENABLE_SPI_DMA_0      0
// </e>


// <e> SPIDMA 0
// <i> SPIDMA0 pin: P3.12~P4.1
#define CONFIG_ENABLE_SPI_DMA_1      0
// </e>
// </h>


/************************************* SPIFC ******/
// <e> SPINORFLASH
// <i> SPIFC pin: P0.8~P0.13
#define CONFIG_ENABLE_SPIFC      1
// </e>


/************************************* SD/SDIO ******/
// <e> SDNF
// <i> SDNF pin:
// <i> group1: map to P0.15~P1.13
// <i> group2: map to P3.0~P3.13

#define CONFIG_ENABLE_SD_0      0
// </e>


// <e> SDIO/SD1
// <i> SDIO pin: P1.14~P2.3
#define CONFIG_ENABLE_SDIO      0
// </e>


/************************************* CIS ******/
// <e> CIS
// <i> CIS pin: P1.2~P1.13
#define CONFIG_ENABLE_CIS      0
// </e>



/************************************* OID ******/
// <e> OID Sensor
// <i> OID pin:
// <i> group1: map to P1.4~P1.9
// <i> group2: map to P4.10,P4.11,P2.4~P2.7
#define CONFIG_ENABLE_OID      0
// </e>


/************************************* SAR-ADC ******/
// <e> SAR_ADC
// <i> SAR_ADC pin: P0.4~P0.7,P4.2~P4.3
#define CONFIG_ENABLE_SAR_ADC      0
// </e>


/************************************* Timer/PWM ******/
// <h> PWM/Timer
// <e> CT32B0_PWM
// <i> CT32B0_PWM pin: P4.5~P4.7, PWM0/PWM1 can switch to P0.12~P0.13
#define CONFIG_ENABLE_CT32B0_PWM      0
#if(CONFIG_ENABLE_CT32B0_PWM == 1)
	// <e> PWM_0
    // <i> CONFIG_PINS_GROUP_ID_CT32B0_PWM0 Default: 1
	// <i> P4.7 or P0.13
	#define USE_PWM0       1
    // </e>

	// <e> PWM_1
    // <i> CONFIG_PINS_GROUP_ID_CT32B0_PWM1 Default: 1
	// <i> P4.6 or P0.12
	#define USE_PWM1       1
	// </e>

    // <e> PWM_2
    // <i> CONFIG_PINS_GROUP_ID_CT32B0_PWM2 Default: 1
	// <i> P4.5
	#define USE_PWM2       1
	// </e>
#endif
// </e>

// <e> CT32B1_PWM
// <i> CT32B1_PWM pin: P4.2~P4.4
#define CONFIG_ENABLE_CT32B1_PWM      0
#if(CONFIG_ENABLE_CT32B1_PWM == 1)
	// <e> PWM_3
    // <i> CONFIG_PINS_GROUP_ID_CT32B1_PWM0 Default: 1
	// <i> P4.4
	#define USE_PWM3       1
    // </e>

	// <e> PWM_4
    // <i> CONFIG_PINS_GROUP_ID_CT32B1_PWM1 Default: 1
	// <i> P4.3
	#define USE_PWM4       1
	// </e>

    // <e> PWM_5
    // <i> CONFIG_PINS_GROUP_ID_CT32B1_PWM2 Default: 1
	// <i> P4.2
	#define USE_PWM5       1
	// </e>
#endif
// </e>

// <e> CT32B2_PWM
// <i> CT32B2_PWM pin: P3.15~P4.1,PWM_8 can switch to P4.11
#define CONFIG_ENABLE_CT32B2_PWM      0
#if(CONFIG_ENABLE_CT32B2_PWM == 1)
	// <e> PWM_6
    // <i> CONFIG_PINS_GROUP_ID_CT32B2_PWM0 Default: 1
	// <i> P4.1
	#define USE_PWM6       1
    // </e>

	// <e> PWM_7
    // <i> CONFIG_PINS_GROUP_ID_CT32B2_PWM1 Default: 1
	// <i> P4.0
	#define USE_PWM7       1
	// </e>

    // <e> PWM_8
    // <i> CONFIG_PINS_GROUP_ID_CT32B2_PWM2 Default: 1
	// <i> P3.15 or P4.11
	#define USE_PWM8       1
	// </e>
#endif
// </e>

// <e> CT32B3_PWM
// <i> CT32B3_PWM pin:
// <i> group1: map to P3.12~P3.14
// <i> group2: map to P4.8~P4.10
#define CONFIG_ENABLE_CT32B3_PWM      0
#if(CONFIG_ENABLE_CT32B3_PWM == 1)
	// <e> PWM_9
    // <i> CONFIG_PINS_GROUP_ID_CT32B3_PWM0 Default: 1
	// <i> P3.14 or P4.10
	#define USE_PWM9       1
    // </e>

	// <e> PWM_10
    // <i> CONFIG_PINS_GROUP_ID_CT32B3_PWM1 Default: 1
	// <i> P3.13 or P4.9
	#define USE_PWM10       1
	// </e>

    // <e> PWM_11
    // <i> CONFIG_PINS_GROUP_ID_CT32B3_PWM2 Default: 1
	// <i> P3.12 or P4.8
	#define USE_PWM11       1
	// </e>
#endif
// </e>

// <e> CT32B4_PWM
// <i> CT32B4_PWM pin: P3.11,P3.10,P2.7
#define CONFIG_ENABLE_CT32B4_PWM      0
#if(CONFIG_ENABLE_CT32B4_PWM == 1)
	// <e> PWM_12
    // <i> CONFIG_PINS_GROUP_ID_CT32B4_PWM0 Default: 1
	// <i> P3.11
	#define USE_PWM12       1
    // </e>

	// <e> PWM_13
    // <i> CONFIG_PINS_GROUP_ID_CT32B4_PWM1 Default: 1
	// <i> P3.10
	#define USE_PWM13       1
	// </e>

    // <e> PWM_14
    // <i> CONFIG_PINS_GROUP_ID_CT32B4_PWM2 Default: 1
	// <i> P2.7
	#define USE_PWM14       1
	// </e>
#endif
// </e>

// <e> CT32B5_PWM
// <i> CT32B5_PWM pin: P2.4~P2.6
#define CONFIG_ENABLE_CT32B5_PWM      0
#if(CONFIG_ENABLE_CT32B5_PWM == 1)
	// <e> PWM_15
    // <i> CONFIG_PINS_GROUP_ID_CT32B5_PWM0 Default: 1
	// <i> P2.6
	#define USE_PWM15       1
    // </e>

	// <e> PWM_16
    // <i> CONFIG_PINS_GROUP_ID_CT32B5_PWM1 Default: 1
	// <i> P2.5
	#define USE_PWM16       1
	// </e>

    // <e> PWM_17
    // <i> CONFIG_PINS_GROUP_ID_CT32B5_PWM2 Default: 1
	// <i> P2.4
	#define USE_PWM17       1
	// </e>
#endif
// </e>

// <e> CT32B6_PWM
// <i> CT32B6_PWM pin:
// <i> group1: map to P0.5~P0.7
// <i> group2: map to P1.7~P1.9
#define CONFIG_ENABLE_CT32B6_PWM      0
#if(CONFIG_ENABLE_CT32B6_PWM == 1)
	// <e> PWM_18
    // <i> CONFIG_PINS_GROUP_ID_CT32B6_PWM0 Default: 1
	// <i> P0.7 or P1.9
	#define USE_PWM18       1
    // </e>

	// <e> PWM_19
    // <i> CONFIG_PINS_GROUP_ID_CT32B6_PWM1 Default: 1
	// <i> P0.6 or P1.8
	#define USE_PWM19       1
	// </e>

    // <e> PWM_20
    // <i> CONFIG_PINS_GROUP_ID_CT32B6_PWM2 Default: 1
	// <i> P0.5 or P1.7
	#define USE_PWM20       1
	// </e>
#endif
// </e>

// <e> CT32B7_PWM
// <i> CT32B7_PWM pin:
// <i> group1: map to P0.2~P0.4
// <i> group2: map to P1.4~P1.6
#define CONFIG_ENABLE_CT32B7_PWM      0
#if(CONFIG_ENABLE_CT32B7_PWM == 1)
	// <e> PWM_21
    // <i> CONFIG_PINS_GROUP_ID_CT32B7_PWM0 Default: 1
	// <i> P0.4 or P1.6
	#define USE_PWM21       1
    // </e>

	// <e> PWM_22
    // <i> CONFIG_PINS_GROUP_ID_CT32B7_PWM1 Default: 1
	// <i> P0.3 or P1.5
	#define USE_PWM22       1
	// </e>

    // <e> PWM_23
    // <i> CONFIG_PINS_GROUP_ID_CT32B7_PWM2 Default: 1
	// <i> P0.2 or P1.4
	#define USE_PWM23       1
	// </e>
#endif
// </e>

// <h> CAP
// <e> CT32B0 CAP
// <i> CT32B0_CAP pin: P3.12 
#define CONFIG_ENABLE_CT32B0_CAP      0
// </e>

// <e> CT32B1 CAP
// <i> CT32B1_CAP pin: P3.13
#define CONFIG_ENABLE_CT32B1_CAP      0
// </e>

// <e> CT32B2 CAP
// <i> CT32B2_CAP pin: P3.14
#define CONFIG_ENABLE_CT32B2_CAP      0
// </e>

// <e> CT32B3 CAP
// <i> CT32B3_CAP pin: P3.15
#define CONFIG_ENABLE_CT32B3_CAP      0
// </e>

// <e> CT32B4 CAP
// <i> CT32B4_CAP pin: P4.0
#define CONFIG_ENABLE_CT32B4_CAP      0
// </e>

// <e> CT32B5 CAP
// <i> CT32B5_CAP pin: P4.1
#define CONFIG_ENABLE_CT32B5_CAP      0
// </e>

// <e> CT32B6 CAP
// <i> CT32B6_CAP pin: P4.2
#define CONFIG_ENABLE_CT32B6_CAP      0
// </e>

// <e> CT32B7 CAP
// <i> CT32B7_CAP pin: P4.3
#define CONFIG_ENABLE_CT32B7_CAP      0
// </e>
// </h>
// </h>


/************************************* TFT ******/
// <e> TFT
// <i> TFT pin:
// <i> group1: map to P2.4~P3.9
// <i> group2: map to P1.2~P1.13
#define CONFIG_ENABLE_TFT      0
   
// </e>
// </h>
//=============================================================================
// <h>                 Pin Switch Control
//=============================================================================

#if defined(USE_PWM0) && (USE_PWM0 == 1)
    // <o> PWM0: I/O Pin Group index
    //    <1=> CT32B0_PWM0 = GPIO Port_4[7]
    //    <2=> CT32B0_PWM0 = GPIO Port_0[13]
    // <i> CONFIG_PINS_GROUP_ID_CT32B0_PWM0 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B0_PWM0       1
#endif

#if defined(USE_PWM1) && (USE_PWM1 == 1)
    // <o> PWM1: I/O Pin Group index
    //    <1=> CT32B0_PWM1 = GPIO Port_4[6]
    //    <2=> CT32B0_PWM1 = GPIO Port_0[12]
    // <i> CONFIG_PINS_GROUP_ID_CT32B0_PWM1 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B0_PWM1       1
#endif

#if defined(USE_PWM8) && (USE_PWM8 == 1)
    // <o> PWM8: I/O Pin Group index
    //    <1=> CT32B2_PWM2 = GPIO Port_3[15]
    //    <2=> CT32B2_PWM2 = GPIO Port_4[11]
    // <i> CONFIG_PINS_GROUP_ID_CT32B2_PWM2 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B2_PWM2       1
#endif

#if defined(USE_PWM9) && (USE_PWM9 == 1)
    // <o> PWM9: I/O Pin Group index
    //    <1=> CT32B3_PWM0 = GPIO Port_3[14]
    //    <2=> CT32B3_PWM0 = GPIO Port_4[10]
    // <i> CONFIG_PINS_GROUP_ID_CT32B3_PWM0 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B3_PWM0       1
#endif

#if defined(USE_PWM10) && (USE_PWM10 == 1)
    // <o> PWM10: I/O Pin Group index
    //    <1=> CT32B3_PWM1 = GPIO Port_3[13]
    //    <2=> CT32B3_PWM1 = GPIO Port_4[9]
    // <i> CONFIG_PINS_GROUP_ID_CT32B3_PWM1 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B3_PWM1       1
#endif

#if defined(USE_PWM11) && (USE_PWM11 == 1)
    // <o> PWM11: I/O Pin Group index
    //    <1=> CT32B3_PWM2 = GPIO Port_3[12]
    //    <2=> CT32B3_PWM2 = GPIO Port_4[8]
    // <i> CONFIG_PINS_GROUP_ID_CT32B3_PWM2 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B3_PWM2       1
#endif

#if defined(USE_PWM18) && (USE_PWM18 == 1)
    // <o> PWM18: I/O Pin Group index
    //    <1=> CT32B6_PWM0 = GPIO Port_1[9]
    //    <2=> CT32B6_PWM0 = GPIO Port_0[7]
    // <i> CONFIG_PINS_GROUP_ID_CT32B6_PWM0 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B6_PWM0       1
#endif

#if defined(USE_PWM19) && (USE_PWM19 == 1)
    // <o> PWM19: I/O Pin Group index
    //    <1=> CT32B6_PWM1 = GPIO Port_1[8]
    //    <2=> CT32B6_PWM1 = GPIO Port_0[6]
    // <i> CONFIG_PINS_GROUP_ID_CT32B6_PWM1 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B6_PWM1       1
#endif

#if defined(USE_PWM20) && (USE_PWM20 == 1)
    // <o> PWM20: I/O Pin Group index
    //    <1=> CT32B6_PWM2 = GPIO Port_1[7]
    //    <2=> CT32B6_PWM2 = GPIO Port_0[5]
    // <i> CONFIG_PINS_GROUP_ID_CT32B6_PWM2 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B6_PWM2       1
#endif

#if defined(USE_PWM21) && (USE_PWM21 == 1)
    // <o> PWM21: I/O Pin Group index
    //    <1=> CT32B7_PWM0 = GPIO Port_1[6]
    //    <2=> CT32B7_PWM0 = GPIO Port_0[4]
    // <i> CONFIG_PINS_GROUP_ID_CT32B7_PWM0 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B7_PWM0       1
#endif

#if defined(USE_PWM22) && (USE_PWM22 == 1)
    // <o> PWM22: I/O Pin Group index
    //    <1=> CT32B7_PWM1 = GPIO Port_1[5]
    //    <2=> CT32B7_PWM1 = GPIO Port_0[3]
    // <i> CONFIG_PINS_GROUP_ID_CT32B7_PWM1 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B7_PWM1       1
#endif

#if defined(USE_PWM23) && (USE_PWM23 == 1)
    // <o> PWM23: I/O Pin Group index
    //    <1=> CT32B7_PWM2 = GPIO Port_1[4]
    //    <2=> CT32B7_PWM2 = GPIO Port_0[2]
    // <i> CONFIG_PINS_GROUP_ID_CT32B7_PWM2 Default: 1
#define CONFIG_PINS_GROUP_ID_CT32B7_PWM2       1
#endif

#if (CONFIG_ENABLE_OID == 1)
    // <o> OID: I/O Pin Group index
    //    <1=> OID_CLK = GPIO Port_1[4]
    //    <2=> OID_CLK = GPIO Port_2[4]
    // <i> CONFIG_PINS_GROUP_ID_OID Default: 1
#define CONFIG_PINS_GROUP_ID_OID       1
#endif

#if (CONFIG_ENABLE_SPI_DMA_0 == 1)
    // <o> SPI_DMA 0: I/O Pin Group index
    //    <1=> SPI0_DMA_CS = GPIO Port_0[8]
    //    <2=> SPI0_DMA_CS = GPIO Port_1[4]
    // <i> CONFIG_PINS_GROUP_ID_SPI_DMA_0 Default: 2
#define CONFIG_PINS_GROUP_ID_SPI_DMA_0    2
#endif

#if (CONFIG_ENABLE_I2S_4 == 1)
    // <o> I2S 4: I/O Pin Group index
    //    <1=> I2S_MCLK = GPIO Port_1[14]
    //    <2=> I2S_MCLK = GPIO Port_3[2]
    //    <3=> I2S_MCLK = GPIO Port_2[4]
    // <i> CONFIG_PINS_GROUP_ID_I2S_4 Default: 1
#define CONFIG_PINS_GROUP_ID_I2S_4    1
#endif

#if (CONFIG_ENABLE_TFT == 1)

    // <o> TFT: I/O Pin Group index
    //    <1=> HSYNC = GPIO Port_2[4]
    //    <2=> HSYNC = GPIO Port_1[2]
    // <i> CONFIG_PINS_GROUP_ID_TFT Default: 1
#define CONFIG_PINS_GROUP_ID_TFT    1

    // <o> TFT: 8/16/18 bits select
    //    <8=> 8 bits
    //    <16=> 16 bits
	//    <18=> 18 bits
    // <i> TFT Default: 8 bits
#define CONFIG_ENABLE_TFT_BITS    8
#endif

#if (CONFIG_ENABLE_SD_0 == 1)
    // <o> SDNF: I/O Pin Group index
    //    <1=> SD0_CLK = GPIO Port_1[4]
    //    <2=> SD0_CLK = GPIO Port_3[5]
    // <i> CONFIG_PINS_GROUP_ID_SD0 Default: 1
#define CONFIG_PINS_GROUP_ID_SD0    1
#endif

// </h>

//=============================================================================
// <h>                 I/O Pin Share Control
//=============================================================================
// <e> I/O Pin Share
// <i> Default: share
#define IO_SHARE_PIN      1
// </e>
// </h>
#include "io_pins_swd.h"
#include "io_pins_uart.h"
#include "io_pins_i2c.h"
#include "io_pins_i2s.h"
#include "io_pins_spi.h"
#include "io_pins_spi_dma.h"
#include "io_pins_sd.h"
#include "io_pins_spifc.h"
#include "io_pins_cis.h"
#include "io_pins_oid.h"
#include "io_pins_sar_adc.h"
#include "io_pins_timer_pwm.h"
#include "io_pins_tft.h"
#include "io_pin_verify.h"


// <<< end of configuration section >>>

void IO_Pin_Setting(void);

#ifdef __cplusplus
}
#endif

#endif

