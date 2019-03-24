/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file codec_jpg.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */


#include "media_comm.h"
#include "codec_jpg_comm.h"
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
static uint32_t                     g_jpg_dec_initialized = false;
static jpg_msg_box_t                g_jpg_box = {.op_cmd = JPG_OP_CMD_NONE,};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
jpg_dec_init(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    media_assert(pCodec_setting != 0);

    do {
        if( g_jpg_dec_initialized == true )
            break;

        memset(&g_jpg_box, 0x0, sizeof(jpg_msg_box_t));

        g_jpg_box.op_cmd               = (pCodec_setting->dec_init.is_1st_frame) ? JPG_OP_CMD_INIT_EX : JPG_OP_CMD_INIT;
        g_jpg_box.init.op_type         = JPG_OPERATION_DECODING;
        g_jpg_box.init.pHeap_desc      = pCodec_setting->dec_init.pHeap_desc;
        g_jpg_box.init.pRender_dec     = pCodec_setting->dec_init.pRender_desc;
        g_jpg_box.init.dec.frm_rate    = pCodec_setting->dec_init.frm_rate;
        g_jpg_box.init.dec.frm_hdr_size = pCodec_setting->dec_init.frm_hdr_size;
        jpg_comm_init(&g_jpg_box);

        g_jpg_dec_initialized = true;

    } while(0);

    return rval;
}

static int
jpg_dec_proc(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    media_assert(pCodec_setting != 0);

    do {
        if( g_jpg_dec_initialized == false )
        {
            rval = -1;
            break;
        }

        if( pCodec_setting->frm_state & (MEDIA_FRM_STATE_PARTIAL | MEDIA_FRM_STATE_END) )
        {
            g_jpg_box.op_cmd        = JPG_OP_CMD_FILL_BIT_STREAM;
            g_jpg_box.bs.pBs_buf    = pCodec_setting->bs.pStream_buf;
            g_jpg_box.bs.bs_buf_len = pCodec_setting->bs.stream_buf_len;
            jpg_comm_proc(&g_jpg_box);

            if( pCodec_setting->frm_state & MEDIA_FRM_STATE_END )
            {
                uint32_t    buf[2] = {(uint32_t)(-1), (uint32_t)(-1)};

                g_jpg_box.bs.pBs_buf    = (uint8_t*)buf;
                g_jpg_box.bs.bs_buf_len = 8;
                jpg_comm_proc(&g_jpg_box);
            }
        }

    } while(0);

    return rval;
}

static int
jpg_dec_deinit(
    media_codec_set_t   *pCodec_setting)
{
    int     rval = 0;

    jpg_comm_deinit(0);

    g_jpg_dec_initialized = false;

    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
media_codec_desc_t      media_codec_desc_jpg_dec =
{
    .codec_type = MEDIA_CODEC_JPG_DEC,
    .cb_init    = jpg_dec_init,
    .cb_proc    = jpg_dec_proc,
    .cb_deinit  = jpg_dec_deinit,
};


