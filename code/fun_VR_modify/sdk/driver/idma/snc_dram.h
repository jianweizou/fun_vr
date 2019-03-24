/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_dram.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/05/22
 * @license
 * @description
 */

#ifndef __snc_dram_H_6000cf24_1c17_41f3_a523_221dc9ea5182__
#define __snc_dram_H_6000cf24_1c17_41f3_a523_221dc9ea5182__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#ifndef SN_DDR_BASE
  #define SN_DDR_BASE   0x40040000
#endif


//=============================================================================
//                  Macro Definition
//=============================================================================

/* Channel: 0~3, Burst: 1~256 */
void ExtMem_Set_Burst_Num(
    uint32_t Channel, 
    uint32_t BurstRead, 
    uint32_t BurstWrite
)
{
    uint32_t RegAddr    = (Channel < 2) ? (SN_DDR_BASE + 0xA0) : (SN_DDR_BASE + 0xA4);
    uint32_t RegData    = *(uint32_t*)RegAddr;
    uint32_t RegOffset  = (Channel % 2) ? 16 : 0;

    RegData &= ~(0xFFFF << RegOffset);
    RegData |= ((BurstRead-1) | ((BurstWrite-1) << 8)) << RegOffset;    

    *(uint32_t*)RegAddr = RegData;
}

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


    
#ifdef __cplusplus
}
#endif

#endif
