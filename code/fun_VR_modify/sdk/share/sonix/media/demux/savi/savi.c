/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file savi_demuxer.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "savi.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define _FOURCC(a, b, c, d)         (((d) << 24) | ((c) <<16) | ((b) << 8) | (a))

#define AVI_FCC_SAVI        _FOURCC('S', 'A', 'V', 'I')
#define AVI_FCC_RIFF        _FOURCC('R', 'I', 'F', 'F')
#define AVI_FCC_WAVE        _FOURCC('W', 'A', 'V', 'E')
#define AVI_FCC_FMT         _FOURCC('f', 'm', 't', ' ')
#define AVI_FCC_FACT        _FOURCC('f', 'a', 'c', 't')
#define AVI_FCC_DATA        _FOURCC('d', 'a', 't', 'a')
#define AVI_FCC_LIST        _FOURCC('L', 'I', 'S', 'T')
//=============================================================================
//                  Macro Definition
//=============================================================================
#define stringize(s)    #s
#define to_str(s)       stringize(s)


#define _assert(expression)  \
    ((void)((expression) ? 0 : printf("%s[%u] err '%s'\n", __func__, __LINE__, to_str(expression))))

#define SAVI_VID_DATA_OFFSET(frame_idx, pSavi_hdr)  \
    ((pSavi_hdr)->idx_table_offset_vid + ((frame_idx) << 2))
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct avi_riff
{
    uint32_t        tag_riff;
    uint32_t        size;
    uint32_t        fcc;

} avi_riff_t;

typedef struct avi_chunk
{
    uint32_t    fcc;
    uint32_t    size;

} avi_chunk_t;

typedef struct avi_list
{
    uint32_t    tag_list;
    uint32_t    size;
    uint32_t    fcc;

} avi_list_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
savi_parse_header(
    uint32_t        *pBuf,
    uint32_t        buf_size,
    savi_header_t   *pHeader)
{
    int     rval = 0;

    _assert(pBuf != 0);
    _assert(buf_size >= 512);

    do {
        savi_header_t     *pHdr = (savi_header_t*)pBuf;

        if( pHdr->tag != AVI_FCC_SAVI )
        {
            rval = -1;
            break;
        }

        memcpy(pHeader, pBuf, sizeof(savi_header_t));

        pHeader->frame_max_num -= 2;

    } while(0);

    return rval;
}

int
savi_demux_video(
    savi_header_t       *pHdr,
    savi_demux_ctrl_t   *pCtrl_info)
{
    int                 rval = 0;

    _assert(pHdr != 0);
    _assert(pCtrl_info != 0);
    _assert(pCtrl_info->cb_frame_info != 0);

    do {
        savi_frm_info_t     frm_info = {0};


        frm_info.header_offset = pHdr->jpg_hdr_offset;
        frm_info.header_length = pHdr->jpg_hdr_size;
        frm_info.curr_frm_offset_in_idx_table
            = SAVI_VID_DATA_OFFSET(pCtrl_info->frame_idx, pHdr);

        frm_info.next_frm_offset_in_idx_table
            = (pCtrl_info->frame_idx + 1 > pHdr->frame_max_num )
            ? frm_info.curr_frm_offset_in_idx_table
            : SAVI_VID_DATA_OFFSET(pCtrl_info->frame_idx + 1, pHdr);

        rval = pCtrl_info->cb_frame_info(pCtrl_info, &frm_info);

    } while(0);

    return rval;
}

