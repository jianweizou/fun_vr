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
//                  Constant Definition
//=============================================================================
/**
 *  7320 definition
 */
#define SN_CT32B0_BASE       0x40000000UL
#define SN_CT32B1_BASE       0x40001000UL
#define SN_CT32B2_BASE       0x40002000UL
#define SN_CT32B3_BASE       0x40003000UL
#define SN_CT32B4_BASE       0x40004000UL
#define SN_CT32B5_BASE       0x40005000UL
#define SN_CT32B6_BASE       0x40006000UL
#define SN_CT32B7_BASE       0x40007000UL
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  SN_CT32Bits Structure
 */
typedef struct sn_ct32bits
{
    union {
        __IO uint32_t  TMRCTRL;                /*!< Offset:0x00 CT32Bn Timer Control Register   */

        struct {
            __IO uint32_t  CEN  :  1;          /*!< Counter Enable      */
            __IO uint32_t  CRST :  1;          /*!< Counter Reset       */
        } TMRCTRL_b;                           /*!< BitSize     */
    };
    __IO uint32_t  TC;                         /*!< Offset:0x04 CT32Bn Timer Counter Register   */
    __IO uint32_t  PRE;                        /*!< Offset:0x08 CT32Bn Prescale Register    */
    __IO uint32_t  PC;                         /*!< Offset:0x0C CT32Bn Prescale Counter Register       */

    union {
        __IO uint32_t  CNTCTRL;                /*!< Offset:0x10 CT32Bn Counter Control Register */

        struct {
            __IO uint32_t  CTM :  2;           /*!< Counter/Timer Mode  */
            __IO uint32_t  CIS :  2;           /*!< Counter Input Select    */
        } CNTCTRL_b;                           /*!< BitSize     */
    };

    union {
        __IO uint32_t  MCTRL;                  /*!< Offset:0x14 CT32Bn Match Control Register   */

        struct {
            __IO uint32_t  MR0IE      :  1;    /*!< Enable generating an interrupt when MR0 matches TC */
            __IO uint32_t  MR0RST     :  1;    /*!< Enable reset TC when MR0 matches TC     */
            __IO uint32_t  MR0STOP    :  1;    /*!< Stop TC and PC and clear CEN bit when MR0 matches TC       */
            __IO uint32_t  MR1IE      :  1;    /*!< Enable generating an interrupt when MR1 matches TC */
            __IO uint32_t  MR1RST     :  1;    /*!< Enable reset TC when MR1 matches TC     */
            __IO uint32_t  MR1STOP    :  1;    /*!< Stop TC and PC and clear CEN bit when MR1 matches TC       */
            __IO uint32_t  MR2IE      :  1;    /*!< Enable generating an interrupt when MR2 matches TC */
            __IO uint32_t  MR2RST     :  1;    /*!< Enable reset TC when MR2 matches TC     */
            __IO uint32_t  MR2STOP    :  1;    /*!< Stop TC and PC and clear CEN bit when MR2 matches TC       */
            __IO uint32_t  MR3IE      :  1;    /*!< Enable generating an interrupt when MR3 matches TC */
            __IO uint32_t  MR3RST     :  1;    /*!< Enable reset TC when MR3 matches TC     */
            __IO uint32_t  MR3STOP    :  1;    /*!< Stop TC and PC and clear CEN bit when MR3 matches TC       */
        } MCTRL_b;                             /*!< BitSize     */
    };

    __IO uint32_t  MR0;                        /*!< Offset:0x18 CT32Bn MR0 Register  */
    __IO uint32_t  MR1;                        /*!< Offset:0x1C CT32Bn MR1 Register  */
    __IO uint32_t  MR2;                        /*!< Offset:0x20 CT32Bn MR2 Register  */
    __IO uint32_t  MR3;                        /*!< Offset:0x24 CT32Bn MR3 Register  */

    union {
        __IO uint32_t  CAPCTRL;                /*!< Offset:0x28 CT32Bn Capture Control Register */

        struct {
            __IO uint32_t  CAP0RE :  2;        /*!< Capture/Reset on CT32Bn_CAP0/Comparator capture output signal rising edge. */
            __IO uint32_t  CAP0FE :  2;        /*!< Capture/Reset on CT32Bn_CAP0/Comparator capture output signal falling edge.       */
            __IO uint32_t  CAP0IE :  1;        /*!< Interrupt on CT32Bn_CAP0/Comparator capture output event   */
            __IO uint32_t  CAP0EN :  2;        /*!< Capture 0 function enable bit  */
        } CAPCTRL_b;                           /*!< BitSize     */
    };
    __I  uint32_t  CAP0;                       /*!< Offset:0x2C CT32Bn CAP0 Register */

    union {
        __IO uint32_t  EM;                     /*!< Offset:0x30 CT32Bn External Match Register  */

        struct {
            __IO uint32_t  EM3        :  1;    /*!< When the TC matches MR0, this bit will act according to EMC0[1:0], and also drive the state of CT32Bn_PWM output.    */
            __IO uint32_t  RESERVED0  :  3;
            /**
             *  EMC0: In PWM mode, the behavior AFTER match MR value
             *      00: Do Nothing.
             *      01: CT32Bn_PWM pin is LOW
             *      10: CT32Bn_PWM pin is HIGH
             *      11: Toggle CT32Bn_PWM.
             *          e.g. cur_period (duty 30%) -> next_1_period (duty 70%) -> next_2_period (duty 30%)
             */
            __IO uint32_t  EMC0       :  2;    /*!< CT32Bn_PWM functionality      */
            __IO uint32_t  RESERVED1  :  26;
        } EM_b;                                /*!< BitSize     */
    };

    union {
        __IO uint32_t  PWMCTRL;                /*!< Offset:0x34 CT32Bn PWM Control Register */

        struct {
            __IO uint32_t  PWM0EN     :  1;    /*!< PWM0 enable */
            __IO uint32_t  PWM1EN     :  1;    /*!< PWM1 enable */
            __IO uint32_t  PWM2EN     :  1;    /*!< PWM2 enable */
            __IO uint32_t  RESERVED0  :  17;
            __IO uint32_t  PWM0IOEN   :  1;    /*!< CT32Bn_PWM0/GPIO selection     */
            __IO uint32_t  PWM1IOEN   :  1;    /*!< CT32Bn_PWM1/GPIO selection     */
            __IO uint32_t  PWM2IOEN   :  1;    /*!< CT32Bn_PWM2/GPIO selection     */
            __IO uint32_t  RESERVED1  :  9;
        } PWMCTRL_b;                           /*!< BitSize     */
    };

    union {
        __I  uint32_t  RIS;                    /*!< Offset:0x38 CT32Bn Raw Interrupt Status Register   */

        struct {
            __I  uint32_t  MR0IF  :  1;        /*!< Match channel 0 interrupt flag */
            __I  uint32_t  MR1IF  :  1;        /*!< Match channel 1 interrupt flag */
            __I  uint32_t  MR2IF  :  1;        /*!< Match channel 2 interrupt flag */
            __I  uint32_t  MR3IF  :  1;        /*!< Match channel 3 interrupt flag */
            __I  uint32_t  CAP0IF :  1;        /*!< Capture channel 0 interrupt flag */
        } RIS_b;                               /*!< BitSize     */
    };

    union {
        __O  uint32_t  IC;                     /*!< Offset:0x3C CT32Bn Interrupt Clear Register */

        struct {
            __O  uint32_t  MR0IC  :  1;        /*!< MR0IF clear bit     */
            __O  uint32_t  MR1IC  :  1;        /*!< MR1IF clear bit     */
            __O  uint32_t  MR2IC  :  1;        /*!< MR2IF clear bit     */
            __O  uint32_t  MR3IC  :  1;        /*!< MR3IF clear bit     */
            __O  uint32_t  CAP0IC :  1;        /*!< CAP0IF clear bit    */
        } IC_b;                                /*!< BitSize     */
    };
} sn_ct32bits_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
