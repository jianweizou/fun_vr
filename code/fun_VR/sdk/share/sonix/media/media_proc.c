/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights reserved.
 */
/** @file media_proc.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */


#include <stdint.h>
#include <stdio.h>
#include "media_comm.h"


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define MEDIA_CTRL_BUSY         (0x1 << 1)

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern media_fmt_desc_t        media_fmt_desc_savi_demux;

static media_fmt_desc_t     *g_fmt_table[] =
{
    0,
    &media_fmt_desc_savi_demux,
};

static int                      g_is_media_init = 0;
static TaskHandle_t             g_htask_meida;
static SemaphoreHandle_t        g_media_mtx = 0;
static uint32_t                 g_media_ctrl_flag = 0;
static CB_USER_NOTIFY           g_cb_user_notify = 0;
static void                     *g_pNotify_user_info = 0;

QueueHandle_t           g_media_sys_q;
QueueHandle_t           g_media_ctrl_q;
media_codec_desc_t      *g_vcodec_desc = 0;
media_codec_desc_t      *g_acodec_desc = 0;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_def_render_method(
    media_render_msg_t  *pMsg)
{
    return 0;
}

static media_user_render_desc_t    g_def_render_desc =
{
    .cb_get_backend_window = _def_render_method,
    .cb_reset_render       = _def_render_method,
    .cb_rendering          = _def_render_method,
};
//=============================================================================
//                  Public Function Definition
//=============================================================================
static void
_task_media(void *argv)
{
    do {
        if( !g_media_mtx )
        {
            g_media_mtx = xSemaphoreCreateMutex();
            if( !g_media_mtx )
            {
                break;
            }
        }

        while(1)
        {
            media_mbox_t            media_mbox = {.media_fmt = MEDIA_FMT_UNKOWN,};
            media_notification_t    notification = {.event = MEIDA_USER_EVENT_PLAY_END,};

            if( xSemaphoreTake(g_media_mtx, portMAX_DELAY) == pdFALSE )
            {
                vTaskDelay(5);
                continue;
            }

            if( xQueueReceive(g_media_sys_q, &media_mbox, portMAX_DELAY) )
            {
                int                 rval = 0;
                media_fmt_desc_t    *pFmt_desc_cur = g_fmt_table[media_mbox.media_fmt];

                do {
                    if( pFmt_desc_cur && pFmt_desc_cur->media_fmt == media_mbox.media_fmt )
                    {
                    }
                    else if( media_mbox.media_fmt == MEDIA_FMT_USER && media_mbox.pUser_fmt_desc )
                    {
                        pFmt_desc_cur = media_mbox.pUser_fmt_desc;
                    }
                    else
                    {
                        err("unknown format type %d or wrong fmt_desc !\n", media_mbox.media_fmt);
                        notification.event = MEIDA_USER_EVENT_UNKNOWN_FORMAT;
                        break;
                    }

                    if( media_mbox.pRender_desc == 0 )
                        media_mbox.pRender_desc = &g_def_render_desc;

                    media_assert(media_mbox.pHeap_desc);
                    if( media_mbox.pHeap_desc->cb_init )
                        media_mbox.pHeap_desc->cb_init(media_mbox.pHeap_desc);

                    g_media_ctrl_flag |= MEDIA_CTRL_BUSY;

                    rval = pFmt_desc_cur->cb_init(&media_mbox);

                    while( rval == 0 )
                    {
                        media_ctrl_mbox_t       ctrl_mbox = { .ctrl_event = MEDIA_CTRL_EVENT_NONE, };

                        if( xQueueReceive(g_media_ctrl_q, &ctrl_mbox, 0) )
                        {
                        }

                        rval = pFmt_desc_cur->cb_proc(&ctrl_mbox);
                    }

                    pFmt_desc_cur->cb_deinit(&media_mbox, &notification);

                    if( media_mbox.pHeap_desc->cb_deinit )
                        media_mbox.pHeap_desc->cb_deinit(media_mbox.pHeap_desc);

                    if( rval )
                    {
                        if( rval == MEDIA_ERR_NOT_SUPPORT )
                            notification.event = MEIDA_USER_EVENT_NOT_SUPPORT;
                        else if( rval == MEDIA_ERR_NO_SPACE )
                            notification.event = MEIDA_USER_EVENT_NO_SAPCE;

                    }
                } while(0);

            }

            if( g_cb_user_notify )
                g_cb_user_notify(&notification, g_pNotify_user_info);

            xSemaphoreGive(g_media_mtx);

            g_media_ctrl_flag &= (~MEDIA_CTRL_BUSY);
        }

    } while(0);

}

void media_init(void)
{
    do {
        if( g_is_media_init )
        {
            vTaskResume(g_htask_meida);
            break;
        }

        g_media_sys_q  = xQueueCreate(1, sizeof(media_mbox_t));
        g_media_ctrl_q = xQueueCreate(1, sizeof(media_ctrl_mbox_t));

        xTaskCreate(_task_media, "VPly",
                    MEDIA_STACK_BYTES(2048),
                    NULL,
                    0,
                    &g_htask_meida);

        g_is_media_init = 1;
    } while(0);

    return;
}

void media_deinit(void)
{
    media_ctrl_mbox_t   msg = {
        .ctrl_event = MEDIA_CTRL_EVENT_STOP,
    };

    xQueueSend(g_media_ctrl_q, &msg, 0);

    while( (g_media_ctrl_flag & MEDIA_CTRL_BUSY) )
        vTaskDelay(5);

    if( g_is_media_init )
        vTaskSuspend(g_htask_meida);

    g_cb_user_notify    = 0;
    g_pNotify_user_info = 0;

    return;
}

int
media_start_msg(
    media_mbox_t    *pMsg)
{
    xQueueSend(g_media_sys_q, pMsg, portMAX_DELAY);
    return 0;
}




int
media_register_notify(
    CB_USER_NOTIFY      cb_notify,
    void                *pUser_info)
{
    g_cb_user_notify    = cb_notify;
    g_pNotify_user_info = pUser_info;
    return 0;
}
