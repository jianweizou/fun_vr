/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_cis.h
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/15
 * @license
 * @description
 */

#ifndef __snc_ae_H_wRFYKH18_lPyp_HPzN_ssQW_uMqhIq1xc3pt__
#define __snc_ae_H_wRFYKH18_lPyp_HPzN_ssQW_uMqhIq1xc3pt__

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
typedef struct ae_handle_info
{
    uint32_t ae_window_start_x;             /* user input ,AE report window start x pixel */
    uint32_t ae_window_start_y;             /* user input ,AE report window start y pixel */
    uint32_t ae_window_size_x;              /* user input ,AE report window size x */
    uint32_t ae_window_size_y;              /* user input ,AE report window size y */
    uint32_t ae_window_skip_x;              /* user input ,AE report window skip x */
    uint32_t ae_window_skip_y;              /* user input ,AE report window skip y */
    uint32_t ae_y_hist_mode;                /* user input ,AE report y hist mode */
} ae_handle_info_t;

typedef struct ae_get_report
{
    uint32_t hist[32];                      /* Get value by function AE_Get_Report */
    uint32_t ysum[9];                       /* Get value by function AE_Get_Report */
    uint32_t ymax;                          /* Get value by function AE_Get_Report */
    uint32_t ymin;                          /* Get value by function AE_Get_Report */
} ae_get_report_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int AE_Init(ae_handle_info_t *ae_info);
int AE_Get_Report(ae_get_report_t *pget_report);
void AE_Get_Report_Restart(void);

uint32_t AE_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
