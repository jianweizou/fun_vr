/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file fmt_savi.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */


#include "media_comm.h"
#include "demux/savi/savi.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define FF_ERR_OPERATOR         0
#define MEDIA_ERR_OPERATOR      0
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct savi_io
{
    media_io_t     in_type;

    union {
        uint8_t     *pStream_buf;

        struct {
            media_user_io_desc_t    *pIO_desc;
            uint8_t                 *pMedia_addr;
            void                    *pUser_info;
        } nor;

        struct {
            media_user_io_desc_t    *pIO_desc;
            uint8_t                 *pStart_addr;
            void                    *pExtra_info;
        } usr;
    };

    media_user_render_desc_t     *pRender_desc;
    media_user_heap_desc_t       *pHeap_desc;

    uint32_t                     is_1st_frame;

} savi_io_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static savi_header_t    g_savi_hdr = {0};
static savi_io_t        g_savi_io = {.in_type = MEDIA_IO_UNKNOWN,};

static int              g_frame_cnt = 0;
static int              g_block_idx = (int)-1;

static uint8_t             *g_pMeida_raw_buf = 0;
static uint32_t             g_media_raw_buf_len = 0;

extern media_codec_desc_t      media_codec_desc_jpg_dec;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_savi_read_block(
    savi_io_t       *pSavi_io,
    uint32_t        position,
    uint8_t         **ppCur,
    uint32_t        *pValid_len)
{
    int     rval = 0;

    do {
        long        offset = 0;

        if( g_block_idx != (position & ~0x1FF) )
        {
            g_block_idx = (position & ~0x1FF);

            if( pSavi_io->in_type == MEDIA_IO_MEM )
            {
                memcpy(g_pMeida_raw_buf, &pSavi_io->pStream_buf[g_block_idx], g_media_raw_buf_len);
            }
            else if( pSavi_io->in_type == MEDIA_IO_USER )
            {
                media_user_io_desc_t    *pIO_desc = pSavi_io->usr.pIO_desc;

                rval = pIO_desc->cb_read(g_pMeida_raw_buf, pSavi_io->usr.pStart_addr + g_block_idx,
                                         g_media_raw_buf_len, (int*)&offset, pSavi_io->usr.pExtra_info);
                if( rval )      return rval;
            }
        }

        offset = position & 0x1FF;

        *ppCur      = (uint8_t*)((uint32_t)g_pMeida_raw_buf + offset);
        *pValid_len = (uint32_t)g_pMeida_raw_buf + g_media_raw_buf_len - (uint32_t)(*ppCur);
    } while(0);

    return rval;
}

static int
_cb_savi_vframe_info(
    savi_demux_ctrl_t   *pCtrl,
    savi_frm_info_t     *pFrm_info)
{
    int                 rval = 0;
    savi_io_t           *pSavi_io = (savi_io_t*)pCtrl->pTunnel_info;
    long                curr_raw_offset = 0, next_raw_offset = 0;
    uint8_t             *pCur = 0;
    uint32_t            valid_len = 0;
    media_codec_desc_t  *pCodec_desc = &media_codec_desc_jpg_dec;

    MEDIA_CHK(rval, _savi_read_block(pSavi_io, pFrm_info->header_offset, &pCur, &valid_len), MEDIA_ERR_OPERATOR);

    {
        uint32_t            next_pos = pFrm_info->header_offset + valid_len;
        int                 remain = pFrm_info->header_length;
        media_codec_set_t   codec_setting = {.codec_type = MEDIA_CODEC_JPG_DEC,};

        codec_setting.dec_init.pRender_desc = g_savi_io.pRender_desc;
        codec_setting.dec_init.pHeap_desc   = g_savi_io.pHeap_desc;
        codec_setting.dec_init.frm_rate     = g_savi_hdr.frame_rate;
        codec_setting.dec_init.frm_hdr_size = pFrm_info->header_length;
        if( g_savi_io.is_1st_frame == true )
        {
            codec_setting.dec_init.is_1st_frame = true;
            g_savi_io.is_1st_frame              = false;
        }
        MEDIA_CHK(rval, pCodec_desc->cb_init(&codec_setting), MEDIA_ERR_OPERATOR);

        while( remain )
        {
            valid_len = (remain < valid_len) ? remain : valid_len;

            remain -= valid_len;

            codec_setting.bs.pStream_buf    = pCur;
            codec_setting.bs.stream_buf_len = valid_len;
            codec_setting.frm_state         = MEDIA_FRM_STATE_PARTIAL;

            MEDIA_CHK(rval, pCodec_desc->cb_proc(&codec_setting), MEDIA_ERR_OPERATOR);

            if( remain )
            {
                MEDIA_CHK(rval, _savi_read_block(pSavi_io, next_pos, &pCur, &valid_len), MEDIA_ERR_OPERATOR);
                next_pos += valid_len;
                g_block_idx = -1;
            }
        }
    }

    MEDIA_CHK(rval, _savi_read_block(pSavi_io, pFrm_info->curr_frm_offset_in_idx_table, &pCur, &valid_len), MEDIA_ERR_OPERATOR);
    curr_raw_offset = *((uint32_t*)pCur);
    MEDIA_CHK(rval, _savi_read_block(pSavi_io, pFrm_info->next_frm_offset_in_idx_table, &pCur, &valid_len), MEDIA_ERR_OPERATOR);
    next_raw_offset = *((uint32_t*)pCur);

    MEDIA_CHK(rval, _savi_read_block(pSavi_io, curr_raw_offset, &pCur, &valid_len), MEDIA_ERR_OPERATOR);

    {
        uint32_t            next_pos = curr_raw_offset + valid_len;
        media_codec_set_t   codec_setting = {.codec_type = MEDIA_CODEC_JPG_DEC,};

        if( next_raw_offset > curr_raw_offset )
        {
            int         remain = next_raw_offset - curr_raw_offset;

            while( remain )
            {
                valid_len = (remain < valid_len) ? remain : valid_len;

                remain -= valid_len;

                codec_setting.bs.pStream_buf    = pCur;
                codec_setting.bs.stream_buf_len = valid_len;
                codec_setting.frm_state         = MEDIA_FRM_STATE_PARTIAL;
                if( remain == 0 )
                    codec_setting.frm_state |= MEDIA_FRM_STATE_END;

                MEDIA_CHK(rval, pCodec_desc->cb_proc(&codec_setting), MEDIA_ERR_OPERATOR);

                if( remain )
                {
                    MEDIA_CHK(rval, _savi_read_block(pSavi_io, next_pos, &pCur, &valid_len), MEDIA_ERR_OPERATOR);
                    next_pos += valid_len;
					for (int i = 0; i < valid_len; i += 16)
					{
						//check next 'FF DA'
						if (*(u16*)(pCur + i) == 0xDAFF)
						{
							//i is deadline
							remain = valid_len;
						}
					}
                    g_block_idx = -1;
                }
            }
        }
        else
        {
            bool    bSection_start = false;
            bool    bFrame_end = false;

            while( bFrame_end == false )
            {
                uint8_t     *pStart = pCur;
                uint8_t     *pEnd = pCur + valid_len;

                while( pCur < pEnd )
                {
                    if( bSection_start == true )
                    {
                        bSection_start = false;
                        if( *pCur == 0xD9 )
                        {
                            pCur++;
                            bFrame_end = true;
                            break;
                        }
                    }

                    if( *pCur == 0xFF && *(pCur + 1) == 0xD9 )
                    {
                        pCur += 2;
                        bFrame_end = true;
                        break;
                    }

                    pCur++;
                }

                codec_setting.bs.pStream_buf    = pStart;
                codec_setting.bs.stream_buf_len = pCur - pStart;
                codec_setting.frm_state         = MEDIA_FRM_STATE_PARTIAL;
                if( bFrame_end )
                    codec_setting.frm_state |= MEDIA_FRM_STATE_END;

                MEDIA_CHK(rval, pCodec_desc->cb_proc(&codec_setting), MEDIA_ERR_OPERATOR);

                if( *pCur == 0xFF )     bSection_start = true;

                if( bFrame_end == false && *(pCur + 1) != 0xD9 )
                {
                    MEDIA_CHK(rval, _savi_read_block(pSavi_io, next_pos, &pCur, &valid_len), MEDIA_ERR_OPERATOR);
                    next_pos += valid_len;
                    g_block_idx = -1;
                    continue;
                }
            }
        }

        MEDIA_CHK(rval, pCodec_desc->cb_deinit(NULL), MEDIA_ERR_OPERATOR);
    }

    return 0;
}

static int
media_fmt_savi_init(
    media_mbox_t    *pMedia_init)
{
    int             rval = 0;

    media_assert(pMedia_init != 0);

    do {
        uint32_t        len = 0;

        g_frame_cnt       = 0;

        media_assert(pMedia_init->pRender_desc);
        media_assert(pMedia_init->pHeap_desc);
        media_assert(pMedia_init->pHeap_desc->cb_malloc);
        media_assert(pMedia_init->pHeap_desc->cb_free);

        g_savi_io.in_type      = pMedia_init->in_type;
        g_savi_io.pRender_desc = pMedia_init->pRender_desc;
        g_savi_io.pHeap_desc   = pMedia_init->pHeap_desc;
        g_savi_io.is_1st_frame = true;

        if( g_savi_io.pHeap_desc->cb_malloc((uint8_t**)&g_pMeida_raw_buf, MEDIA_MEM_TYPE_PARSING_BUF,
                                            MEDIA_RAW_BUF_SIZE, (int*)&g_media_raw_buf_len, 32) )
        {
            rval = -1;
            media_assert(g_pMeida_raw_buf);
            break;
        }

        len = g_media_raw_buf_len;

        if( pMedia_init->in_type == MEDIA_IO_MEM )
        {
            media_assert(pMedia_init->mem.pStream_buf != 0);

            g_savi_io.pStream_buf = pMedia_init->mem.pStream_buf;
            memcpy(g_pMeida_raw_buf, g_savi_io.pStream_buf, g_media_raw_buf_len);
        }
        else if( pMedia_init->in_type == MEDIA_IO_USER )
        {
            media_user_io_desc_t    *pIO_desc = pMedia_init->usr.pIo_desc;

            media_assert(pIO_desc != 0);
            media_assert(pIO_desc->cb_read != 0);

            g_savi_io.usr.pIO_desc    = pMedia_init->usr.pIo_desc;
            g_savi_io.usr.pExtra_info = pMedia_init->usr.pExtra_info;
            g_savi_io.usr.pStart_addr = pMedia_init->usr.pMedia_start_addr;
            if( !pIO_desc ||  !pIO_desc->cb_read )
            {
                rval = -1;
                break;
            }

            if( pIO_desc->cb_init )
            {
                rval = pIO_desc->cb_init(MEIDA_USER_IO_MODE_READ, g_savi_io.usr.pExtra_info);
                if( rval )      break;
            }

            MEDIA_CHK(rval, pIO_desc->cb_read(g_pMeida_raw_buf, g_savi_io.usr.pStart_addr, (int)len, (int*)&len, g_savi_io.usr.pExtra_info), MEDIA_ERR_OPERATOR);
        }

        rval = savi_parse_header((uint32_t*)g_pMeida_raw_buf, len, &g_savi_hdr);
        if( rval )
        {
            rval = MEDIA_ERR_NOT_SUPPORT;
            break;
        }
    } while(0);

    return rval;
}


static int
media_fmt_savi_proc(
    media_ctrl_mbox_t   *pMedia_ctrl)
{
    int             rval = 0;

    if( pMedia_ctrl->ctrl_event == MEDIA_CTRL_EVENT_STOP )
    {
        rval = 1;
        return rval;
    }


    do {
        savi_demux_ctrl_t   ctrl_info = {0};

        ctrl_info.frame_idx     = g_frame_cnt;
        ctrl_info.cb_frame_info = _cb_savi_vframe_info;
        ctrl_info.pTunnel_info  = (void*)&g_savi_io;
        rval = savi_demux_video(&g_savi_hdr, &ctrl_info);
        if( rval )      break;

        g_frame_cnt++;
    } while(0);

    if( g_frame_cnt == g_savi_hdr.frame_max_num )
    {
        rval = 1;
    }

    return rval;
}

static int
media_fmt_savi_deinit(
    media_mbox_t            *pMedia_init,
    media_notification_t    *pNotification)
{
    int     rval = 0;

    if( g_savi_io.in_type == MEDIA_IO_USER )
    {
        media_user_io_desc_t    *pIO_desc = pMedia_init->usr.pIo_desc;

        if( pIO_desc->cb_deinit )
        {
            rval = pIO_desc->cb_deinit(g_savi_io.usr.pExtra_info);
        }
    }

    g_savi_io.pHeap_desc->cb_free((uint8_t**)&g_pMeida_raw_buf, MEDIA_MEM_TYPE_PARSING_BUF);

    pNotification->event = MEIDA_USER_EVENT_PLAY_END;

    memset(&g_savi_io, 0x0, sizeof(g_savi_io));
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
media_fmt_desc_t        media_fmt_desc_savi_demux =
{
    .media_fmt = MEDIA_FMT_SAVI,
    .cb_init   = media_fmt_savi_init,
    .cb_proc   = media_fmt_savi_proc,
    .cb_deinit = media_fmt_savi_deinit,
};
