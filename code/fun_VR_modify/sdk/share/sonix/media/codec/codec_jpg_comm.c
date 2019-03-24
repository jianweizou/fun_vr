/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file codec_jpg_comm.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */

#include <string.h>
#include "codec/codec_jpg_comm.h"
#include "util/rbuf_opt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define JFLAG_IS_INIT_JPG           (0x1 << 0)
#define JFLAG_IS_INIT_CSC           (0x1 << 1)
#define JFLAG_IS_FRAME_END          (0x1 << 2)

//=============================================================================
//                  Macro Definition
//=============================================================================
#define RB_ERR_CHK(err_code)                                                               \
    do { if(err_code == RB_ERR_NO_SPACE)            msg("%s", "no space \n");              \
         else if(err_code == RB_ERR_OK)             ;                                      \
         else if(err_code == RB_ERR_NO_DATA)        msg("%s", "no data \n");               \
         else if(err_code == RB_ERR_W_CATCH_R)      msg("%s", "w_idx catch r_id \n");      \
         else if(err_code == RB_ERR_R_CATCH_W)      msg("%s", "r_idx catch w_id \n");      \
         else if(err_code == RB_ERR_INVALID_PARAM)  msg("%s", "invalid param \n");         \
         else                                       msg("%s", "unknown err!\n");           \
    }while(0)


#define CMD_EOS                     MEDIA_4CC('E', 'O', 'S', ' ')

#define SET_FLAG(flag, value)       ((flag) |= (value))
#define CLR_FLAG(flag, value)       ((flag) &= ~(value))
#define GET_FLAG(flag, value)       ((flag) & (value))

#define RGB_565(r, g, b)            ((((r) & 0x1F) << 11) | (((g) & 0x3F) << 5) | ((b) & 0x1F))
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct render_info
{
    media_render_msg_t          msg;
    media_user_render_desc_t    *pRender_desc;

    uint32_t                    screen_buf_len;
    uint32_t                    frm_rate;

    uint8_t                     *pCur_slice_base_addr;

} render_info_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
jpg_mgr_t            g_jpg_mgr = {.operation_type = JPG_OPERATION_IDLE,};
jbuf_mgr_t           g_jbuf_mgr = {0};
csc_handle_info_t    g_csc_mgr = {0};

static uint32_t             g_flags = 0lu;
static rb_operator_t        g_hRB = {0};
static int                  g_hdr_size = 0;
static uint32_t             *g_v_rbuf = 0;
static uint32_t             g_v_rbuf_len = 0;

static TaskHandle_t         g_hTask_jproc;
static QueueHandle_t        g_vCodecQ = NULL;


static render_info_t                g_render_info = {.pRender_desc = 0,};
static media_user_heap_desc_t       *g_pHeap_desc = 0;
//=============================================================================
//                  IRQ Function Definition
//=============================================================================
__irq void JPEG_DEC_DMA_IN_IRQHandler(void)
{
    JPEG_DEC_Set_Slice_Done(JPEG_SLICE_INPUT);
}

__irq void JPEG_DEC_DMA_OUT_IRQHandler(void)
{
    if( GET_FLAG(g_flags, JFLAG_IS_INIT_CSC) )
    {
        CSC_Set_Slice_Start();
    }
    else
    {
        JPEG_DEC_Set_Slice_Done(JPEG_SLICE_OUTPUT);
    }
}


__irq void CSC_DMA_IN_IRQHandler(void)
{
    if( GET_FLAG(g_flags, JFLAG_IS_INIT_CSC) )
    {
        JPEG_DEC_Set_Slice_Start();
    }
    else
    {
        CSC_Set_Slice_Done(CSC_SLICE_INPUT);
    }
}

__irq void CSC_DMA_OUT_IRQHandler(void)
{
    CSC_Set_Slice_Done(CSC_SLICE_OUTPUT);
}

//=============================================================================
//                  Private Function Definition
//=============================================================================
#if 0
static int
_mem_int(uint32_t id)
{
    IDMA_Set_Burst_Unit((idma_ch_t)id, IDMA_BURST_UNIT_32B);
    return 0;
}

static int
_mem_copy(uint32_t id, void *to, void *from, uint32_t len)
{
    int             rval = 0;
    idma_setting_t  setting = {0};

    setting.dip_addr = (uint32_t)from;
    setting.ram_addr = (uint32_t)to;
    setting.length   = len;

    while(IDMA_Get_Status((idma_ch_t)id)) {}

    rval = IDMA_Start((idma_ch_t)id, &setting);

    return rval;
}

static int
_mem_deinit(uint32_t id)
{
    IDMA_Stop((idma_ch_t)id);
    return 0;
}
#else
    #define _mem_int(a)
    #define _mem_deinit(a)
    #define _mem_copy(id, to, from, len)    memcpy(to, from, len)
#endif

static int
_scaling_conv(
    csc_handle_info_t   *pCSC_mgr,
    uint32_t            dest_w,
    uint32_t            dest_h)
{
    int         rval = 0;

    do {
        pCSC_mgr->scal_en       = false;
        pCSC_mgr->scal_width    = dest_w;
        pCSC_mgr->scal_height   = dest_h;

        if( pCSC_mgr->frame_w == dest_w )
            break;

        if( dest_w == 320 )
        {
            if( pCSC_mgr->frame_w == 640 ||
                pCSC_mgr->frame_w == 1280 )
            {
                pCSC_mgr->scal_en = true;
            }
        }
        else if( dest_w == 640 )
        {
            if( pCSC_mgr->frame_w == 1024 ||
                pCSC_mgr->frame_w == 800 ||
                pCSC_mgr->frame_w == 1280 )
            {
                pCSC_mgr->scal_en = true;
            }
        }
        else
        {
            media_assert(0);
        }
    } while(0);
    return rval;
}

static int
_jpg_get_buf_zc(
    uint8_t     **ppBuf,
    uint32_t    *pBuf_size)
{
    int         rval = 0;

    while( g_jpg_mgr.cur_frm_buf_addr == 0 ||
           g_jpg_mgr.cur_frm_buf_len == 0 )
    {
        vTaskDelay(3);
    }

    *ppBuf      = (uint8_t*)g_jpg_mgr.cur_frm_buf_addr;
    *pBuf_size  = g_jpg_mgr.cur_frm_buf_len;
    return rval;
}

static int
_jpg_dec_fill_buf_zc(
    cb_jpeg_info_t  *pInfo)
{
    int                 rval = 0;
    jpeg_header_info_t  *pJHdr = &g_jpg_mgr.jpg_hdr;
    uint32_t            offset = pJHdr->header_length & (~0x3);

    pInfo->buffer_addr = (uint32_t)g_jpg_mgr.cur_frm_buf_addr + offset;
    pInfo->len         = g_jpg_mgr.cur_frm_buf_len - offset;

    return rval;
}

static int
_get_data_size(
    rb_data_info_t  *pInfo)
{
    static int      end_flag = 0;
    int             rval = 0;
    unsigned char   *pTmp = pInfo->r_ptr + (pInfo->data_size - 4);

    if( end_flag && pInfo->read_idx == RB_READ_TYPE_FETCH )
    {
        end_flag = 0;
        pInfo->is_dummy = 1;
        return CMD_EOS;
    }

    if( pTmp[0] == 0xFF && pTmp[1] == 0xFF && pTmp[2] == 0xFF && pTmp[3] == 0xFF )
    {
        pInfo->data_size -= 4;
        pInfo->data_size = ((pInfo->data_size + 0x3 ) & ~0x3);
        end_flag = 1;
    }

    pInfo->data_size &= (~0x3);
    pInfo->is_dummy = 0;
    return rval;
}

static int
_jpg_get_buf(
    uint8_t     **ppBuf,
    uint32_t    *pBuf_size)
{
#define JPG_HDR_PARSING_THRESHOLD       656
    int         rval = 0;
    do {
        int             target_len = 0;
        uint8_t         *pData = 0;
        int             data_size = 0;

        g_hdr_size = (g_hdr_size) ? g_hdr_size : JPG_HDR_PARSING_THRESHOLD;
        target_len = g_hdr_size + 32;

        do {
            vTaskDelay(2);
            rb_opt_peek_r(&g_hRB, RB_READ_TYPE_FETCH, &pData, &data_size, _get_data_size);
        } while( data_size < target_len );

        *ppBuf      = pData;
        *pBuf_size  = data_size;

    } while(0);

    return rval;
}

static int
_jpg_dec_fill_buf(
    cb_jpeg_info_t  *pInfo)
{
    int             rval = 0;
    rb_err_t        rb_err = RB_ERR_OK;

    do {
        uint8_t         *pData = 0;
        int             data_size = 0;

        rb_opt_update_r(&g_hRB, RB_READ_TYPE_REMOVE, &pData, &data_size, _get_data_size);

        rb_err = rb_opt_update_r(&g_hRB, RB_READ_TYPE_FETCH, &pData, &data_size, _get_data_size);
        if( rb_err == RB_ERR_OK && data_size && data_size != RB_INVALID_SIZE )
        {
            pInfo->buffer_addr = (uint32_t)pData;
            pInfo->len         = data_size;
            if( pData[0] == 0xFF && pData[1] == 0xD8 )
            {
                jpeg_header_info_t          *pJHdr = &g_jpg_mgr.jpg_hdr;
                pInfo->buffer_addr += (pJHdr->header_length & (~0x3));
                pInfo->len         -= (pJHdr->header_length & (~0x3));
            }
            break;
        }
        else if( rb_err == CMD_EOS )
        {
            break;
        }

        if( data_size == 0 )
        {
            vTaskDelay(2);
        }
    } while( rb_err );

    return rval;
}

static int
_jpg_dec_empty_buf(
    cb_jpeg_info_t  *pInfo)
{
    uint8_t     rval = 0;

    _mem_copy((uint32_t)IDMA_CH_0, g_render_info.pCur_slice_base_addr, (void*)pInfo->buffer_addr, pInfo->len);

    g_render_info.pCur_slice_base_addr += pInfo->len;

    pInfo->buffer_addr = (uint32_t)g_jbuf_mgr.dec.pSlice_buf_rgb;

    //media_assert((uint32_t)g_render_info.pCur_slice_base_addr <= ((uint32_t)g_render_info.msg.get_output_window.pOut_addr + g_render_info.screen_buf_len));

    return rval;
}


static int
_jpg_decoder_init(
    jpeg_decoder_handle_info_t  *pHJpg_dec,
    jpg_msg_box_t               *pMsg_box)
{
    int     rval = 0;
    do {
        jpeg_header_info_t          *pJHdr = &g_jpg_mgr.jpg_hdr;

        pJHdr->qtable = (uint32_t)g_jbuf_mgr.enc.pQ_table;

        {
            jpg_io_desc_t   jio_desc = {0};

            jio_desc.cb_get_buf = (g_jpg_mgr.operation_type == JPG_OPERATION_DECODING)
                                ? _jpg_get_buf : _jpg_get_buf_zc;

            jio_desc.cb_fill_buf = 0;
            rval = JPEG_Header_Parsing(pJHdr, &jio_desc, 0);
            if( rval )
            {
                break;
            }
        }

        if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING )
        {
            uint8_t         *pData = 0;
            int             data_size = 0;

            data_size = pJHdr->header_length & (~0x3);

            rb_opt_update_r(&g_hRB, RB_READ_TYPE_FETCH, &pData, &data_size, _get_data_size);
            rb_opt_update_r(&g_hRB, RB_READ_TYPE_REMOVE, &pData, &data_size, _get_data_size);
        }

        pHJpg_dec->idma_burst      = IDMA_BURST_UNIT_4B;
        pHJpg_dec->set_qtable_gain = 32;
        pHJpg_dec->en_irq          = true;
        pHJpg_dec->output_buf_len  = JPEG_DEC_Get_Output_Buf_Len(pJHdr);

        if( pHJpg_dec->output_buf_len == 0 ||
            pHJpg_dec->output_buf_len > g_jbuf_mgr.dec.slice_buf_yuv_len )
        {
            err("jpg dec OutBuf too small (%d/%d) !\n",
                g_jbuf_mgr.dec.slice_buf_yuv_len, pHJpg_dec->output_buf_len);
            rval = -1;
            break;
        }

        if( JPEG_DEC_Set_Buffer(pHJpg_dec, pJHdr, (uint8_t*)g_jbuf_mgr.dec.pSlice_buf_yuv, pHJpg_dec->output_buf_len) < 0 )
        {
            rval = -1;
            break;
        }

        if( JPEG_DEC_Init(pHJpg_dec, pJHdr) < 0 )
        {
            rval = -1;
            break;
        }

        memset((void *)&g_csc_mgr, 0x0, sizeof(g_csc_mgr));
        {
            csc_handle_info_t    *pCSC_mgr = &g_csc_mgr;
            media_render_msg_t   *pRender_msg = &g_render_info.msg;
            uint32_t             out_slice_len = g_jbuf_mgr.dec.slice_buf_rgb_len;

            pCSC_mgr->cvt_mode       = CSC_Y2R;
            pCSC_mgr->yuv_fmt        = (pJHdr->jpeg_fmt == JPEG_YCBCR_422) ? CSC_YCBCR_422 : CSC_YCBCR_420;
            pCSC_mgr->en_irq         = true;
            pCSC_mgr->dithering      = true;
            pCSC_mgr->frame_w        = pJHdr->frame_w;
            pCSC_mgr->frame_h        = pJHdr->frame_h;

            _scaling_conv(pCSC_mgr, pRender_msg->get_output_window.width, pRender_msg->get_output_window.height);

            pCSC_mgr->input_buf_len  = CSC_Get_Input_Buf_Len(pCSC_mgr);
            pCSC_mgr->output_buf_len = CSC_Get_Output_Buf_Len(pCSC_mgr);

            if( pCSC_mgr->input_buf_len == 0 || pCSC_mgr->output_buf_len == 0 ||
                pCSC_mgr->input_buf_len > g_jbuf_mgr.dec.slice_buf_yuv_len ||
                pCSC_mgr->output_buf_len > out_slice_len )
            {
                err("CSC I/O buf too small (In: %d/%d, Out: %d/%d) !\n",
                    pCSC_mgr->input_buf_len, g_jbuf_mgr.dec.slice_buf_yuv_len,
                    pCSC_mgr->output_buf_len, out_slice_len);
                rval = -1;
                break;
            }

            if( CSC_Set_Buffer(pCSC_mgr, (uint8_t*)g_jbuf_mgr.dec.pSlice_buf_yuv, g_jbuf_mgr.dec.slice_buf_yuv_len,
                                (uint8_t*)g_jbuf_mgr.dec.pSlice_buf_rgb,
                                out_slice_len) < 0 )
            {
                rval = -1;
                break;
            }

            if( CSC_Init(pCSC_mgr) < 0 )
            {
                rval = -1;
                break;
            }

            SET_FLAG(g_flags, JFLAG_IS_INIT_CSC);

            if( pMsg_box->op_cmd == JPG_OP_CMD_INIT_EX )
            {
                media_render_msg_t      rmsg = {0};
                uint32_t                width = 0;
                uint32_t                height = 0;

                width  = (pCSC_mgr->scal_en) ? pCSC_mgr->scal_width : pJHdr->frame_w;
                height = (pCSC_mgr->scal_en) ? pCSC_mgr->scal_height : pJHdr->frame_h;

                rmsg.reset_render.align_w = (pJHdr->jpeg_fmt == JPEG_YCBCR_422)
                                          ? (width + 0x7) & ~0x7
                                          : (width + 0xF) & ~0xF;

                rmsg.reset_render.align_h = (pJHdr->jpeg_fmt == JPEG_YCBCR_422)
                                          ? (height + 0x7) & ~0x7
                                          : (height + 0xF) & ~0xF;

                rmsg.reset_render.is_flushing = 0;
                g_render_info.pRender_desc->cb_reset_render(&rmsg);
            }
        }
    } while(0);

    return rval;
}


static void _task_jpg_proc(void *argv)
{
    SemaphoreHandle_t       jpg_mtx = NULL;

    jpg_mtx = xSemaphoreCreateMutex();
    if( !jpg_mtx )
    {
        vTaskDelete(NULL);
        return;
    }

    while(1)
    {
        jpg_msg_box_t           msg_box = {.op_cmd = JPG_OP_CMD_NONE,};
        jpeg_process_status_t   status = JPEG_PROC_IDLE;

        if( xSemaphoreTake(jpg_mtx, portMAX_DELAY) == pdFALSE )
            continue;

        if( xQueueReceive(g_vCodecQ, &msg_box, portMAX_DELAY) )
        {
            int     rval = 0;
            int     frm_cnt = 0;

            do {
                uint32_t            width = 0;
                uint32_t            height = 0;
                jpeg_yuv_format_t   jpg_fmt = JPEG_YCBCR_420;

                if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING ||
                    g_jpg_mgr.operation_type == JPG_OPERATION_DECODING_ZC )
                {
                    jpeg_decoder_handle_info_t  *pHJpg_dec = &g_jpg_mgr.hJpg_dec;
                    jpeg_header_info_t          *pJHdr = &g_jpg_mgr.jpg_hdr;

                    if( msg_box.op_cmd == JPG_OP_CMD_INIT || msg_box.op_cmd == JPG_OP_CMD_INIT_EX )
                    {
                        rval = _jpg_decoder_init(pHJpg_dec, &msg_box);
                        if( rval )      break;

                        pHJpg_dec->input_buf     = 0;
                        pHJpg_dec->input_buf_len = 0;

                        width   = pJHdr->frame_w;
                        height  = pJHdr->frame_h;
                        jpg_fmt = pJHdr->jpeg_fmt;
                    }

                    do {
                        status = (g_jpg_mgr.operation_type == JPG_OPERATION_DECODING)
                                ? JPEG_DEC_Proc(pHJpg_dec, _jpg_dec_fill_buf, _jpg_dec_empty_buf, 0, &g_csc_mgr)
                                : JPEG_DEC_Proc(pHJpg_dec, _jpg_dec_fill_buf_zc, _jpg_dec_empty_buf, 0, &g_csc_mgr);
                        if( status != JPEG_PROC_SUCCESS )
                        {
                            msg("jDec (%d) proc %s %d !\n", frm_cnt, (status < 0) ? "err": "state", status);
                            rval = (status < 0) ? MEDIA_ERR_DECODING_FAIL : rval;
                        }
                    } while( status == JPEG_PROC_PAUSE );
                }

                SET_FLAG(g_flags, JFLAG_IS_FRAME_END);

                if( rval )      break;

                if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING ||
                    g_jpg_mgr.operation_type == JPG_OPERATION_DECODING_ZC )
                {
                    media_render_msg_t      msg = {
                        .rendering.frm_rate = g_render_info.frm_rate,
                    };

                    width  = (g_csc_mgr.scal_en) ? g_csc_mgr.scal_width : width;
                    height = (g_csc_mgr.scal_en) ? g_csc_mgr.scal_height : height;

                    msg.rendering.align_w = (jpg_fmt == JPEG_YCBCR_422)
                                              ? (width + 0x7) & ~0x7 : (width + 0xF) & ~0xF;

                    msg.rendering.align_h = (jpg_fmt == JPEG_YCBCR_422)
                                              ? (height + 0x7) & ~0x7 : (height + 0xF) & ~0xF;

                    g_render_info.pRender_desc->cb_rendering(&msg);

                    g_render_info.pCur_slice_base_addr = (uint8_t*)msg.rendering.pBackend_addr;

                    vTaskDelay(pdMS_TO_TICKS(msg.rendering.blocking_ms));
                    frm_cnt++;
                }

            }while(0);
        }
        xSemaphoreGive(jpg_mtx);
    }
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
jpg_comm_init(
    jpg_msg_box_t   *pBox)
{
    int     rval = 0;
    do {
        if( GET_FLAG(g_flags, JFLAG_IS_INIT_JPG) )
            break;

        media_assert(pBox);
        media_assert(pBox->init.pHeap_desc);
        media_assert(pBox->init.pHeap_desc->cb_malloc);
        media_assert(pBox->init.pHeap_desc->cb_free);

        g_hdr_size = 0;

        memset(&g_jbuf_mgr, 0x0, sizeof(g_jbuf_mgr));

        _mem_int((uint32_t)IDMA_CH_0);

        g_pHeap_desc = pBox->init.pHeap_desc;

        memset(&g_jpg_mgr, 0x0, sizeof(jpg_mgr_t));
        if( pBox->op_cmd == JPG_OP_CMD_INIT || pBox->op_cmd == JPG_OP_CMD_INIT_EX )
        {
            g_jpg_mgr.operation_type = pBox->init.op_type;

            if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING ||
                g_jpg_mgr.operation_type == JPG_OPERATION_DECODING_ZC )
            {

                if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING )
                {
                    if( g_pHeap_desc->cb_malloc((uint8_t**)&g_v_rbuf, MEDIA_MEM_TYPE_VID_RING_BUF,
                                                MEDIA_MEM_SIZE_AUTO, (int*)&g_v_rbuf_len, 32) )
                    {
                        rval = -1;
                        media_assert(g_v_rbuf);
                        break;
                    }

                    rb_opt_init(&g_hRB, (unsigned char*)g_v_rbuf, g_v_rbuf_len);
                }

                if( g_pHeap_desc->cb_malloc((uint8_t**)&g_jbuf_mgr.dec.pSlice_buf_yuv, MEDIA_MEM_TYPE_VID_DEC_YUV_SLICE_BUF,
                                            MEDIA_MEM_SIZE_AUTO, (int*)&g_jbuf_mgr.dec.slice_buf_yuv_len, 32) )
                {
                    rval = -1;
                    media_assert(g_jbuf_mgr.dec.pSlice_buf_yuv);
                    break;
                }

                if( g_pHeap_desc->cb_malloc((uint8_t**)&g_jbuf_mgr.dec.pSlice_buf_rgb, MEDIA_MEM_TYPE_VID_DEC_RGB_SLICE_BUF,
                                            MEDIA_MEM_SIZE_AUTO, (int*)&g_jbuf_mgr.dec.slice_buf_rgb_len, 32) )
                {
                    rval = -1;
                    media_assert(g_jbuf_mgr.dec.pSlice_buf_rgb);
                    break;
                }

                g_render_info.pRender_desc = pBox->init.pRender_dec;

                media_assert(g_render_info.pRender_desc);

                g_render_info.pRender_desc->cb_get_backend_window(&g_render_info.msg);

                g_render_info.screen_buf_len        = g_render_info.msg.get_output_window.pitch * g_render_info.msg.get_output_window.height;
                g_render_info.pCur_slice_base_addr  = (uint8_t*)g_render_info.msg.get_output_window.pOut_addr;
                g_render_info.frm_rate              = pBox->init.dec.frm_rate;

                g_hdr_size = pBox->init.dec.frm_hdr_size;
            }
        }

        if( !g_vCodecQ )
        {
            g_vCodecQ = xQueueCreate(1, sizeof(jpg_msg_box_t));

            xTaskCreate(_task_jpg_proc, "jproc",
                        MEDIA_STACK_BYTES(1024),
                        NULL,
                        0,
                        &g_hTask_jproc);

            xQueueSend(g_vCodecQ, pBox, 5u);
        }
        else
        {
            BaseType_t         xHigherPriorityTaskWoken;
            xQueueSendFromISR(g_vCodecQ, pBox, &xHigherPriorityTaskWoken);
        }

        SET_FLAG(g_flags, JFLAG_IS_INIT_JPG);
    } while(0);

    return rval;
}

int
jpg_comm_proc(
    jpg_msg_box_t   *pBox)
{
    int     rval = 0;
    do {
        if( !GET_FLAG(g_flags, JFLAG_IS_INIT_JPG) )
        {
            rval = -1;
            break;
        }

        if( pBox->op_cmd == JPG_OP_CMD_FILL_BIT_STREAM )
        {
            rb_err_t    rb_err = RB_ERR_OK;
            uint8_t     *pData = pBox->bs.pBs_buf;
            int         data_size = (int)pBox->bs.bs_buf_len;

            while( (rb_err = rb_opt_update_w(&g_hRB, pData, data_size)) != 0 )
            {
                vTaskDelay(3);
            }

            (void )rb_err;
        }
        else if( pBox->op_cmd == JPG_OP_CMD_FILL_BIT_STREAM_ZC )
        {
            g_jpg_mgr.cur_frm_buf_addr = (uint32_t)pBox->bs.pBs_buf;
            g_jpg_mgr.cur_frm_buf_len  = pBox->bs.bs_buf_len;
        }
    } while(0);

    return rval;
}

int
jpg_comm_deinit(
    jpg_msg_box_t   *pBox)
{
    int     rval = 0;

    do {
        while( !GET_FLAG(g_flags, JFLAG_IS_FRAME_END) )
        {
            vTaskDelay(3);
        }

        CSC_Proc_Pause();

        if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING ||
            g_jpg_mgr.operation_type == JPG_OPERATION_DECODING_ZC )
        {
            if( (rval = JPEG_DEC_Deinit()) != 0 )
            {
                msg("%s", "jpg decorder de-inti fail\n");
            }
        }

        if( (rval = CSC_Deinit()) != 0 )
        {
            msg("%s", "csc de-inti fail\n");
        }

        _mem_deinit((uint32_t)IDMA_CH_0);

        if( g_jpg_mgr.operation_type == JPG_OPERATION_DECODING )
            g_pHeap_desc->cb_free((uint8_t**)&g_v_rbuf, MEDIA_MEM_TYPE_VID_RING_BUF);

        g_pHeap_desc->cb_free((uint8_t**)&g_jbuf_mgr.dec.pSlice_buf_yuv, MEDIA_MEM_TYPE_VID_DEC_YUV_SLICE_BUF);
        g_pHeap_desc->cb_free((uint8_t**)&g_jbuf_mgr.dec.pSlice_buf_rgb, MEDIA_MEM_TYPE_VID_DEC_RGB_SLICE_BUF);

        CLR_FLAG(g_flags, JFLAG_IS_INIT_JPG | JFLAG_IS_FRAME_END | JFLAG_IS_INIT_CSC);
    } while(0);

    return rval;
}


