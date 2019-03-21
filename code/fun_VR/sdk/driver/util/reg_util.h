/**
 * Copyright (c) 2017 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file reg_util.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2017/12/26
 * @license
 * @description
 */

#ifndef __reg_util_H_wedCIcNA_lJqx_HfUj_sP5n_uWJC189SLcf1__
#define __reg_util_H_wedCIcNA_lJqx_HfUj_sP5n_uWJC189SLcf1__

#ifdef __cplusplus
extern "C" {
#endif


#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

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
static __INLINE void reg_clear_bit(volatile uint32_t *pAddr, uint32_t bit_idx)
{
    *(uint32_t volatile*)pAddr &= (~(0x1UL << bit_idx));
    return;
}

static __INLINE void reg_set_bit(volatile uint32_t *pAddr, uint32_t bit_idx)
{
    *(uint32_t volatile*)pAddr |= (0x1UL << bit_idx);
    return;
}

static __INLINE void reg_toggle_bit(volatile uint32_t *pAddr, uint32_t bit_idx)
{
    *(uint32_t volatile*)pAddr ^= (0x1UL << bit_idx);
    return;
}

static __INLINE void reg_write_bits(volatile uint32_t *pAddr, uint32_t value)
{
    *(uint32_t volatile*)(pAddr) = value;
    return;
}

static __INLINE uint32_t reg_read_bits(volatile uint32_t *pAddr)
{
    return *(uint32_t volatile*)(pAddr);
}

static __INLINE void reg_write_mask_bits(volatile uint32_t *pAddr, uint32_t value, uint32_t mask)
{
    reg_write_bits(pAddr, (reg_read_bits(pAddr) & ~mask) | (value & mask));
    return;
}

static __INLINE uint32_t reg_read_mask_bits(volatile uint32_t *pAddr, uint32_t mask)
{
    return (reg_read_bits(pAddr) & mask);
}



#ifdef __cplusplus
}
#endif

#endif
