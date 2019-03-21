/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file cis_oid.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */


#include "snc_ae.h"

#include "register_7320.h"
#define AE_VERSION        0x73200000
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
/**
 *  \brief ae report initial
 *
 *  \param [in] ae_info : a pointer to an ae_info_t structure
 *  \return -1 : initial fail
 *           0 : initial success
 *
 *  \details ae report initial script
 */
int 
AE_Init(
    ae_handle_info_t *ae_info)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    
    if(ae_info->ae_window_size_x != 0 && ae_info->ae_window_size_y != 0)
    {
        SN_CIS->CIS_AE_START_XY_b.STARTX = ae_info->ae_window_start_x;
        SN_CIS->CIS_AE_START_XY_b.STARTY = ae_info->ae_window_start_y;
        SN_CIS->CIS_AE_SIZE_SKIP_b.SIZE_X = ae_info->ae_window_size_x;
        SN_CIS->CIS_AE_SIZE_SKIP_b.SIZE_Y = ae_info->ae_window_size_y;
        SN_CIS->CIS_AE_SIZE_SKIP_b.SKIP_X = ae_info->ae_window_skip_x;
        SN_CIS->CIS_AE_SIZE_SKIP_b.SKIP_Y = ae_info->ae_window_skip_y;
        SN_CIS->CIS_AE_YHIST_MODE_b.YHIST_MODE = ae_info->ae_y_hist_mode;
        SN_CIS->CIS_AE_YHIST_MODE_b.AE_RPT_EN = true;
    }
    else
    {
        return -1;
    }
    if(SN_CIS->CIS_AE_YHIST_MODE_b.AE_RPT_EN)
    {
        return 0;
    }
    return -1;
}

/**
 *  \brief get ae report data
 *
 *  \param [in] pget_report : a pointer to an ae_get_report_t structure
 *  \return -1 : report not ready
 *           0 : read report ok 
 *
 *  \details get y histogram, y sum, y min and y max
 */
int 
AE_Get_Report(
    ae_get_report_t *pget_report)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    uint32_t i;
    
    if(SN_CIS->CIS_AE_YHIST_MODE_b.RDY)
    {
        /* Read Y histogram report */
        for(i=0; i<32; i++)
        {
            SN_CIS->CIS_AE_YHIST_b.YHIST_IDX = i;
            pget_report->hist[i] = SN_CIS->CIS_AE_YHIST_b.YHIST_CNT_R;
        }
        /* Read Y sum report */
        for(i=0; i<9; i++)
        {
            SN_CIS->CIS_AE_REPORT_b.YREPORT_IDX = i;
            pget_report->ysum[i] = SN_CIS->CIS_AE_REPORT_b.YREPORT;
        }
        
        pget_report->ymax = SN_CIS->CIS_AE_MAX_MIN_b.MAX_Y;
        pget_report->ymin = SN_CIS->CIS_AE_MAX_MIN_b.MIN_Y;
        

        return 0;
    }
    return -1;
}

/**
 *  \brief get ae report restart
 *
 *  \param [in]
 *  \return 
 *
 *  \details restart ae report
 */
void
AE_Get_Report_Restart(void)
{
    sn_cis_t *SN_CIS = (sn_cis_t*)(SN_CIS_BASE);
    SN_CIS->CIS_AE_YHIST_MODE_b.AE_RPT_EN = false;
    SN_CIS->CIS_AE_YHIST_MODE_b.AE_RPT_EN = true;
}

/**
 *  \brief get ae library version
 *
 *  \param 
 *  \return ae library version
 *
 *  \details
 */
uint32_t
AE_GetVersion(void)
{
    return AE_VERSION;
}
