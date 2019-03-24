/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_script.h
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_script_H_wXJ8KjcZ_l808_HwVT_ss1P_ue8YSBvGSSB2__
#define __snc_script_H_wXJ8KjcZ_l808_HwVT_ss1P_ue8YSBvGSSB2__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
#include "snc_jpeg.h"
#include "snc_csc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct script_info
{
    uint32_t jpeg_in_buf;
    uint32_t jpeg_out_buf_len;
    uint32_t jpeg_out_buf_0;
    uint32_t jpeg_out_buf_1;
    uint32_t csc_out_buf_len;
    uint32_t csc_out_buf_0;
    uint32_t csc_out_buf_1;
    uint32_t jpeg_bistrm_size;
    uint32_t header_length;
    uint32_t frame_w;
    uint32_t frame_h;
    bool en_scal;
    uint32_t scal_frame_w;
    uint32_t scal_frame_h;
    jpeg_yuv_format_t jpeg_fmt;
}script_handle_info_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int SCRIPT_Init(script_handle_info_t *script);
int SCRIPT_Proc(script_handle_info_t *script);
void SCRIPT_Set_Hblk(void);
void SCRIPT_Set_Vblk(void);
void SCRIPT_End(void);
uint32_t SCRIPT_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
