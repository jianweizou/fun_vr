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
#define SN_GPIO_0_BASE      0x40018000UL
#define SN_GPIO_1_BASE      0x40019000UL
#define SN_GPIO_2_BASE      0x4001A000UL
#define SN_GPIO_3_BASE      0x4001B000UL
#define SN_GPIO_4_BASE      0x4001C000UL

#define SN_GPIO_BASE_OFFSET         0 // 0x100UL
//=============================================================================
//                      Macro Definition
//=============================================================================

//=============================================================================
//                      Structure Definition
//=============================================================================
/**
 *  SN_GPIO Structure
 */
typedef struct sn_gpio
{
    union {
        __IO uint32_t  DATA;   /*!< Offset:0x00 GPIO Port n Data Register */

        struct {
            __IO uint32_t  DATA0  :  1; /*!< Data of Pn.0 */
            __IO uint32_t  DATA1  :  1; /*!< Data of Pn.1 */
            __IO uint32_t  DATA2  :  1; /*!< Data of Pn.2 */
            __IO uint32_t  DATA3  :  1; /*!< Data of Pn.3 */
            __IO uint32_t  DATA4  :  1; /*!< Data of Pn.4 */
            __IO uint32_t  DATA5  :  1; /*!< Data of Pn.5 */
            __IO uint32_t  DATA6  :  1; /*!< Data of Pn.6 */
            __IO uint32_t  DATA7  :  1; /*!< Data of Pn.7 */
            __IO uint32_t  DATA8  :  1; /*!< Data of Pn.8 */
            __IO uint32_t  DATA9  :  1; /*!< Data of Pn.9 */
            __IO uint32_t  DATA10 :  1; /*!< Data of Pn.10 */
            __IO uint32_t  DATA11 :  1; /*!< Data of Pn.11 */
            __IO uint32_t  DATA12 :  1; /*!< Data of Pn.12 */
            __IO uint32_t  DATA13 :  1; /*!< Data of Pn.13 */
            __IO uint32_t  DATA14 :  1; /*!< Data of Pn.14 */
            __IO uint32_t  DATA15 :  1; /*!< Data of Pn.15 */
        } DATA_b; /*!< BitSize */
    };

    union {
        __IO uint32_t  MODE;   /*!< Offset:0x04 GPIO Port n Mode Register */

        struct {
            __IO uint32_t  MODE0  :  1; /*!< Mode of Pn.0 */
            __IO uint32_t  MODE1  :  1; /*!< Mode of Pn.1 */
            __IO uint32_t  MODE2  :  1; /*!< Mode of Pn.2 */
            __IO uint32_t  MODE3  :  1; /*!< Mode of Pn.3 */
            __IO uint32_t  MODE4  :  1; /*!< Mode of Pn.4 */
            __IO uint32_t  MODE5  :  1; /*!< Mode of Pn.5 */
            __IO uint32_t  MODE6  :  1; /*!< Mode of Pn.6 */
            __IO uint32_t  MODE7  :  1; /*!< Mode of Pn.7 */
            __IO uint32_t  MODE8  :  1; /*!< Mode of Pn.8 */
            __IO uint32_t  MODE9  :  1; /*!< Mode of Pn.9 */
            __IO uint32_t  MODE10 :  1; /*!< Mode of Pn.10 */
            __IO uint32_t  MODE11 :  1; /*!< Mode of Pn.11 */
            __IO uint32_t  MODE12 :  1; /*!< Mode of Pn.12 */
            __IO uint32_t  MODE13 :  1; /*!< Mode of Pn.13 */
            __IO uint32_t  MODE14 :  1; /*!< Mode of Pn.14 */
            __IO uint32_t  MODE15 :  1; /*!< Mode of Pn.15 */
        } MODE_b; /*!< BitSize */
    };

    union {
        __IO uint32_t  CFG; /*!< Offset:0x08 GPIO Port n Configuration Register  */

        struct {
            __IO uint32_t  CFG0  :  2; /*!< Configuration of Pn.0 */
            __IO uint32_t  CFG1  :  2; /*!< Configuration of Pn.1 */
            __IO uint32_t  CFG2  :  2; /*!< Configuration of Pn.2 */
            __IO uint32_t  CFG3  :  2; /*!< Configuration of Pn.3 */
            __IO uint32_t  CFG4  :  2; /*!< Configuration of Pn.4 */
            __IO uint32_t  CFG5  :  2; /*!< Configuration of Pn.5 */
            __IO uint32_t  CFG6  :  2; /*!< Configuration of Pn.6 */
            __IO uint32_t  CFG7  :  2; /*!< Configuration of Pn.7 */
            __IO uint32_t  CFG8  :  2; /*!< Configuration of Pn.8 */
            __IO uint32_t  CFG9  :  2; /*!< Configuration of Pn.9 */
            __IO uint32_t  CFG10 :  2; /*!< Configuration of Pn.10 */
            __IO uint32_t  CFG11 :  2; /*!< Configuration of Pn.11 */
            __IO uint32_t  CFG12 :  2; /*!< Configuration of Pn.12 */
            __IO uint32_t  CFG13 :  2; /*!< Configuration of Pn.13 */
            __IO uint32_t  CFG14 :  2; /*!< Configuration of Pn.14 */
            __IO uint32_t  CFG15 :  2; /*!< Configuration of Pn.15 */
        } CFG_b;  /*!< BitSize */
    };

    union {
        __IO uint32_t  IS; /*!< Offset:0x0C GPIO Port n Interrupt Sense Register */

        struct {
            __IO uint32_t  IS0  :  1; /*!< Interrupt on Pn.0 is event or edge sensitive */
            __IO uint32_t  IS1  :  1; /*!< Interrupt on Pn.1 is event or edge sensitive */
            __IO uint32_t  IS2  :  1; /*!< Interrupt on Pn.2 is event or edge sensitive */
            __IO uint32_t  IS3  :  1; /*!< Interrupt on Pn.3 is event or edge sensitive */
            __IO uint32_t  IS4  :  1; /*!< Interrupt on Pn.4 is event or edge sensitive */
            __IO uint32_t  IS5  :  1; /*!< Interrupt on Pn.5 is event or edge sensitive */
            __IO uint32_t  IS6  :  1; /*!< Interrupt on Pn.6 is event or edge sensitive */
            __IO uint32_t  IS7  :  1; /*!< Interrupt on Pn.7 is event or edge sensitive */
            __IO uint32_t  IS8  :  1; /*!< Interrupt on Pn.8 is event or edge sensitive */
            __IO uint32_t  IS9  :  1; /*!< Interrupt on Pn.9 is event or edge sensitive */
            __IO uint32_t  IS10 :  1; /*!< Interrupt on Pn.10 is event or edge sensitive */
            __IO uint32_t  IS11 :  1; /*!< Interrupt on Pn.11 is event or edge sensitive */
            __IO uint32_t  IS12 :  1; /*!< Interrupt on Pn.12 is event or edge sensitive */
            __IO uint32_t  IS13 :  1; /*!< Interrupt on Pn.13 is event or edge sensitive */
            __IO uint32_t  IS14 :  1; /*!< Interrupt on Pn.14 is event or edge sensitive */
            __IO uint32_t  IS15 :  1; /*!< Interrupt on Pn.15 is event or edge sensitive */
        } IS_b; /*!< BitSize */
    };

    union {
        __IO uint32_t  IBS; /*!< Offset:0x10 GPIO Port n Interrupt Both-edge Sense Register   */

        struct {
            __IO uint32_t  IBS0  :  1; /*!< Interrupt on Pn.0 is triggered ob both edges */
            __IO uint32_t  IBS1  :  1; /*!< Interrupt on Pn.1 is triggered ob both edges */
            __IO uint32_t  IBS2  :  1; /*!< Interrupt on Pn.2 is triggered ob both edges */
            __IO uint32_t  IBS3  :  1; /*!< Interrupt on Pn.3 is triggered ob both edges */
            __IO uint32_t  IBS4  :  1; /*!< Interrupt on Pn.4 is triggered ob both edges */
            __IO uint32_t  IBS5  :  1; /*!< Interrupt on Pn.5 is triggered ob both edges */
            __IO uint32_t  IBS6  :  1; /*!< Interrupt on Pn.6 is triggered ob both edges */
            __IO uint32_t  IBS7  :  1; /*!< Interrupt on Pn.7 is triggered ob both edges */
            __IO uint32_t  IBS8  :  1; /*!< Interrupt on Pn.8 is triggered ob both edges */
            __IO uint32_t  IBS9  :  1; /*!< Interrupt on Pn.9 is triggered ob both edges */
            __IO uint32_t  IBS10 :  1; /*!< Interrupt on Pn.10 is triggered ob both edges */
            __IO uint32_t  IBS11 :  1; /*!< Interrupt on Pn.11 is triggered ob both edges */
            __IO uint32_t  IBS12 :  1; /*!< Interrupt on Pn.12 is triggered ob both edges */
            __IO uint32_t  IBS13 :  1; /*!< Interrupt on Pn.13 is triggered ob both edges */
            __IO uint32_t  IBS14 :  1; /*!< Interrupt on Pn.14 is triggered ob both edges */
            __IO uint32_t  IBS15 :  1; /*!< Interrupt on Pn.15 is triggered ob both edges */
        } IBS_b;  /*!< BitSize */
    };

    union {
        __IO uint32_t  IEV; /*!< Offset:0x14 GPIO Port n Interrupt Event Register */

        struct {
            __IO uint32_t  IEV0  :  1; /*!< Interrupt trigged evnet on Pn.0 */
            __IO uint32_t  IEV1  :  1; /*!< Interrupt trigged evnet on Pn.1 */
            __IO uint32_t  IEV2  :  1; /*!< Interrupt trigged evnet on Pn.2 */
            __IO uint32_t  IEV3  :  1; /*!< Interrupt trigged evnet on Pn.3 */
            __IO uint32_t  IEV4  :  1; /*!< Interrupt trigged evnet on Pn.4 */
            __IO uint32_t  IEV5  :  1; /*!< Interrupt trigged evnet on Pn.5 */
            __IO uint32_t  IEV6  :  1; /*!< Interrupt trigged evnet on Pn.6 */
            __IO uint32_t  IEV7  :  1; /*!< Interrupt trigged evnet on Pn.7 */
            __IO uint32_t  IEV8  :  1; /*!< Interrupt trigged evnet on Pn.8 */
            __IO uint32_t  IEV9  :  1; /*!< Interrupt trigged evnet on Pn.9 */
            __IO uint32_t  IEV10 :  1; /*!< Interrupt trigged evnet on Pn.10 */
            __IO uint32_t  IEV11 :  1; /*!< Interrupt trigged evnet on Pn.11 */
            __IO uint32_t  IEV12 :  1; /*!< Interrupt trigged evnet on Pn.12 */
            __IO uint32_t  IEV13 :  1; /*!< Interrupt trigged evnet on Pn.13 */
            __IO uint32_t  IEV14 :  1; /*!< Interrupt trigged evnet on Pn.14 */
            __IO uint32_t  IEV15 :  1; /*!< Interrupt trigged evnet on Pn.15 */
        } IEV_b;  /*!< BitSize */
    };

    union {
        __IO uint32_t  IE; /*!< Offset:0x18 GPIO Port n Interrupt Enable Register */

        struct {
            __IO uint32_t  IE0  :  1; /*!< Interrupt on Pn.0 enable */
            __IO uint32_t  IE1  :  1; /*!< Interrupt on Pn.1 enable */
            __IO uint32_t  IE2  :  1; /*!< Interrupt on Pn.2 enable */
            __IO uint32_t  IE3  :  1; /*!< Interrupt on Pn.3 enable */
            __IO uint32_t  IE4  :  1; /*!< Interrupt on Pn.4 enable */
            __IO uint32_t  IE5  :  1; /*!< Interrupt on Pn.5 enable */
            __IO uint32_t  IE6  :  1; /*!< Interrupt on Pn.6 enable */
            __IO uint32_t  IE7  :  1; /*!< Interrupt on Pn.7 enable */
            __IO uint32_t  IE8  :  1; /*!< Interrupt on Pn.8 enable */
            __IO uint32_t  IE9  :  1; /*!< Interrupt on Pn.9 enable */
            __IO uint32_t  IE10 :  1; /*!< Interrupt on Pn.10 enable */
            __IO uint32_t  IE11 :  1; /*!< Interrupt on Pn.11 enable */
            __IO uint32_t  IE12 :  1; /*!< Interrupt on Pn.11 enable */
            __IO uint32_t  IE13 :  1; /*!< Interrupt on Pn.13 enable */
            __IO uint32_t  IE14 :  1; /*!< Interrupt on Pn.14 enable */
            __IO uint32_t  IE15 :  1; /*!< Interrupt on Pn.15 enable */
        } IE_b; /*!< BitSize */
    };

    union {
        __I  uint32_t  RIS; /*!< Offset:0x1C GPIO Port n Raw Interrupt Status Register */

        struct {
            __I  uint32_t  IF0  :  1; /*!< Pn.0 raw interrupt flag */
            __I  uint32_t  IF1  :  1; /*!< Pn.1 raw interrupt flag */
            __I  uint32_t  IF2  :  1; /*!< Pn.2 raw interrupt flag */
            __I  uint32_t  IF3  :  1; /*!< Pn.3 raw interrupt flag */
            __I  uint32_t  IF4  :  1; /*!< Pn.4 raw interrupt flag */
            __I  uint32_t  IF5  :  1; /*!< Pn.5 raw interrupt flag */
            __I  uint32_t  IF6  :  1; /*!< Pn.6 raw interrupt flag */
            __I  uint32_t  IF7  :  1; /*!< Pn.7 raw interrupt flag */
            __I  uint32_t  IF8  :  1; /*!< Pn.8 raw interrupt flag */
            __I  uint32_t  IF9  :  1; /*!< Pn.9 raw interrupt flag */
            __I  uint32_t  IF10 :  1; /*!< Pn.10 raw interrupt flag */
            __I  uint32_t  IF11 :  1; /*!< Pn.11 raw interrupt flag */
            __I  uint32_t  IF12 :  1; /*!< Pn.12 raw interrupt flag */
            __I  uint32_t  IF13 :  1; /*!< Pn.13 raw interrupt flag */
            __I  uint32_t  IF14 :  1; /*!< Pn.14 raw interrupt flag */
            __I  uint32_t  IF15 :  1; /*!< Pn.15 raw interrupt flag */
        } RIS_b;  /*!< BitSize */
    };

    union {
        __O  uint32_t  IC; /*!< Offset:0x20 GPIO Port n Interrupt Clear Register */

        struct {
            __O  uint32_t  IC0  :  1; /*!< Pn.0 interrupt flag clear */
            __O  uint32_t  IC1  :  1; /*!< Pn.1 interrupt flag clear */
            __O  uint32_t  IC2  :  1; /*!< Pn.2 interrupt flag clear */
            __O  uint32_t  IC3  :  1; /*!< Pn.3 interrupt flag clear */
            __O  uint32_t  IC4  :  1; /*!< Pn.4 interrupt flag clear */
            __O  uint32_t  IC5  :  1; /*!< Pn.5 interrupt flag clear */
            __O  uint32_t  IC6  :  1; /*!< Pn.6 interrupt flag clear */
            __O  uint32_t  IC7  :  1; /*!< Pn.7 interrupt flag clear */
            __O  uint32_t  IC8  :  1; /*!< Pn.8 interrupt flag clear */
            __O  uint32_t  IC9  :  1; /*!< Pn.9 interrupt flag clear */
            __O  uint32_t  IC10 :  1; /*!< Pn.10 interrupt flag clear   */
            __O  uint32_t  IC11 :  1; /*!< Pn.11 interrupt flag clear   */
            __O  uint32_t  IC12 :  1; /*!< Pn.12 interrupt flag clear   */
            __O  uint32_t  IC13 :  1; /*!< Pn.13 interrupt flag clear   */
            __O  uint32_t  IC14 :  1; /*!< Pn.14 interrupt flag clear   */
            __O  uint32_t  IC15 :  1; /*!< Pn.15 interrupt flag clear   */
        } IC_b; /*!< BitSize */
    };

    union {
        __O  uint32_t  BSET;   /*!< Offset:0x24 GPIO Port n Bits Set Operation Register   */

        struct {
            __O  uint32_t  BSET0 :  1; /*!< Set Pn.0 */
            __O  uint32_t  BSET1 :  1; /*!< Set Pn.1 */
            __O  uint32_t  BSET2 :  1; /*!< Set Pn.2 */
            __O  uint32_t  BSET3 :  1; /*!< Set Pn.3 */
            __O  uint32_t  BSET4 :  1; /*!< Set Pn.4 */
            __O  uint32_t  BSET5 :  1; /*!< Set Pn.5 */
            __O  uint32_t  BSET6 :  1; /*!< Set Pn.6 */
            __O  uint32_t  BSET7 :  1; /*!< Set Pn.7 */
            __O  uint32_t  BSET8 :  1; /*!< Set Pn.8 */
            __O  uint32_t  BSET9 :  1; /*!< Set Pn.9 */
            __O  uint32_t  BSET10 :  1; /*!< Set Pn.10 */
            __O  uint32_t  BSET11 :  1; /*!< Set Pn.11 */
            __O  uint32_t  BSET12 :  1; /*!< Set Pn.12 */
            __O  uint32_t  BSET13 :  1; /*!< Set Pn.13 */
            __O  uint32_t  BSET14 :  1; /*!< Set Pn.14 */
            __O  uint32_t  BSET15 :  1; /*!< Set Pn.15 */
        } BSET_b; /*!< BitSize */
    };

    union {
        __O  uint32_t  BCLR;   /*!< Offset:0x28 GPIO Port n Bits Clear Operation Register */

        struct {
            __O  uint32_t  BCLR0 :  1; /*!< Clear Pn.0   */
            __O  uint32_t  BCLR1 :  1; /*!< Clear Pn.1   */
            __O  uint32_t  BCLR2 :  1; /*!< Clear Pn.2   */
            __O  uint32_t  BCLR3 :  1; /*!< Clear Pn.3   */
            __O  uint32_t  BCLR4 :  1; /*!< Clear Pn.4   */
            __O  uint32_t  BCLR5 :  1; /*!< Clear Pn.5   */
            __O  uint32_t  BCLR6 :  1; /*!< Clear Pn.6   */
            __O  uint32_t  BCLR7 :  1; /*!< Clear Pn.7   */
            __O  uint32_t  BCLR8 :  1; /*!< Clear Pn.8   */
            __O  uint32_t  BCLR9 :  1; /*!< Clear Pn.9   */
            __O  uint32_t  BCLR10 :  1; /*!< Clear Pn.10  */
            __O  uint32_t  BCLR11 :  1; /*!< Clear Pn.11  */
            __O  uint32_t  BCLR12 :  1; /*!< Clear Pn.12  */
            __O  uint32_t  BCLR13 :  1; /*!< Clear Pn.13  */
            __O  uint32_t  BCLR14 :  1; /*!< Clear Pn.14  */
            __O  uint32_t  BCLR15 :  1; /*!< Clear Pn.15  */
        } BCLR_b; /*!< BitSize */
    };

    __I uint32_t RESERVED0;

    union {
        __IO uint32_t  BS; /*!< Offset: GPIO Port Base Switch register   */

        struct {
            __IO uint32_t  BS :  1;
        } BS_b; /*!< BitSize */
    };

    union {
        __IO uint32_t   DRVCTRL; /*!< Offset: GPIO Port n Driving Control register   */

        struct {
            __IO uint32_t  DRV00 :  2;
            __IO uint32_t  DRV01 :  2;
            __IO uint32_t  DRV02 :  2;
            __IO uint32_t  DRV03 :  2;
            __IO uint32_t  DRV04 :  2;
            __IO uint32_t  DRV05 :  2;
            __IO uint32_t  DRV06 :  2;
            __IO uint32_t  DRV07 :  2;
            __IO uint32_t  DRV08 :  2;
            __IO uint32_t  DRV09 :  2;
            __IO uint32_t  DRV10 :  2;
            __IO uint32_t  DRV11 :  2;
            __IO uint32_t  DRV12 :  2;
            __IO uint32_t  DRV13 :  2;
            __IO uint32_t  DRV14 :  2;
            __IO uint32_t  DRV15 :  2;
        } DRVCTRL_b; /*!< BitSize */
    };

    union {
        __IO uint32_t   DRVEN;    /*!< Offset: GPIO pin Driving Enable   */

        struct {
            __IO uint32_t  DRVEN00 :  1;
            __IO uint32_t  DRVEN01 :  1;
            __IO uint32_t  DRVEN02 :  1;
            __IO uint32_t  DRVEN03 :  1;
            __IO uint32_t  DRVEN04 :  1;
            __IO uint32_t  DRVEN05 :  1;
            __IO uint32_t  DRVEN06 :  1;
            __IO uint32_t  DRVEN07 :  1;
            __IO uint32_t  DRVEN08 :  1;
            __IO uint32_t  DRVEN09 :  1;
            __IO uint32_t  DRVEN10 :  1;
            __IO uint32_t  DRVEN11 :  1;
            __IO uint32_t  DRVEN12 :  1;
            __IO uint32_t  DRVEN13 :  1;
            __IO uint32_t  DRVEN14 :  1;
            __IO uint32_t  DRVEN15 :  1;
        } DRVEN_b;
    };
} sn_gpio_t;


//=============================================================================
//  Global Data Definition
//=============================================================================

//=============================================================================
//  Private Function Definition
//=============================================================================

//=============================================================================
//  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
