/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file video_comm.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */

#ifndef __video_comm_H_wL8dRTV5_liPg_H1TD_sRXH_u0Nkow0bsr1F__
#define __video_comm_H_wL8dRTV5_liPg_H1TD_sRXH_u0Nkow0bsr1F__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include "snc_uart.h"
#include "snc_gpio.h"
#include "snc_idma.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define MEDIA_RAW_BUF_SIZE              2048

#define MEDIA_ERR_NOT_SUPPORT           -4
#define MEDIA_ERR_NO_SPACE              -5
#define MEDIA_ERR_DECODING_FAIL         -6

/**
 *  supporting type of media container
 */
typedef enum media_fmt
{
    MEDIA_FMT_UNKOWN    = 0,
    MEDIA_FMT_SAVI      = 1,
    MEDIA_FMT_USER      = 4,

} media_fmt_t;

/**
 *  supporting type of media codec
 */
typedef enum media_codec
{
    MEDIA_CODEC_UNKNOWN     = 0,
    MEDIA_CODEC_JPG_DEC,

} media_codec_t;

typedef enum media_stream_type
{
    MEDIA_STREAM_UNKNOWN        = 0,
    MEDIA_STREAM_VIDEO,
    MEDIA_STREAM_AUDIO,

} media_stream_type_t;

/**
 *  media source type
 */
typedef enum media_io
{
    MEDIA_IO_UNKNOWN       = 0,
    MEDIA_IO_MEM,
    MEDIA_IO_USER,

} media_io_t;

/**
 *  media memory type
 */
typedef enum media_mem_type
{
    MEDIA_MEM_TYPE_PARSING_BUF          = 1,
    MEDIA_MEM_TYPE_VID_RING_BUF,
    MEDIA_MEM_TYPE_VID_DEC_YUV_SLICE_BUF,
    MEDIA_MEM_TYPE_VID_DEC_RGB_SLICE_BUF,

} media_mem_type_t;

/**
 *  input device type of media stream
 */
typedef enum media_stream_dev_type
{
    MEIDA_STREAM_UNKNOWN = 0,
    MEIDA_STREAM_USER,

} media_stream_dev_type_t;


/**
 *  media controlling event type
 */
typedef enum media_ctrl_event
{
    MEDIA_CTRL_EVENT_NONE           = 0,
    MEDIA_CTRL_EVENT_STOP,

} media_ctrl_event_t;

/**
 *  media frame state
 */
#define MEDIA_FRM_STATE_UNKNOWN                     0
#define MEDIA_FRM_STATE_START                       (0x1 << 1)
#define MEDIA_FRM_STATE_PARTIAL                     (0x1 << 2)
#define MEDIA_FRM_STATE_END                         (0x1 << 3)
#define MEDIA_FRM_STATE_PRE_START_NEXT_FRM          (0x1 << 5)
#define MEDIA_FRM_STATE_END_PREV_FRM                (0x1 << 6)

/**
 *  notify event to user.
 *  ps. user MUST register cb_user_notify() first.
 */
typedef enum media_user_event
{
    MEIDA_USER_EVENT_NONE   = 0,
    MEIDA_USER_EVENT_PLAY_END,
    MEIDA_USER_EVENT_REC_END,
    MEIDA_USER_EVENT_NOT_SUPPORT,
    MEIDA_USER_EVENT_NO_SAPCE,
    MEIDA_USER_EVENT_UNKNOWN_FORMAT,

} media_user_event_t;

//=============================================================================
//                  Macro Definition
//=============================================================================
//extern int printf(const char * format, ...);
extern int printf_uart(const char * format, ...);
extern int printf_swo(const char * format, ...);

#define MEDIA_STACK_BYTES(a)                ((a) >> 2)

#define MEDIA_4CC(a, b, c, d)               (((d) << 24) | ((c) <<16) | ((b) << 8) | (a))

#define MEDIA_SET_FLAG(flag, value)         ((flag) |= (value))
#define MEDIA_CLR_FLAG(flag, value)         ((flag) &= ~(value))
#define MEDIA_GET_FLAG(flag, value)         ((flag) & (value))

#define MEDIA_RGB_565(r, g, b)              ((((r) & 0x1F) << 11) | (((g) & 0x3F) << 5) | ((b) & 0x1F))

#if 1

    #define msg(str, argv...)           printf("%s[%u] " str, __func__, __LINE__, ##argv)

    #if 1
        #include "FreeRTOS.h"
        #include "task.h"
        #include "semphr.h"
        #undef msg
        extern SemaphoreHandle_t        g_log_mtx;
        #define msg(str, argv...)       do{ xSemaphoreTake(g_log_mtx, portMAX_DELAY); \
                                            printf("%s[%u] " str, __func__, __LINE__, ##argv); \
                                            xSemaphoreGive(g_log_mtx); \
                                        }while(0)
    #endif

    #define media_assert(expression)                \
            do{ if(expression) break;               \
                msg("%s", "error !!!\n");           \
                while(1);                           \
            }while(0)


#else /* #if 1 */

    #define msg(str, argv...)
    #define media_assert(expression)

#endif /* #if 1 */



#if 0
    #define err(str, argv...)                           do{ msg(str, ##argv); while(1); }while(0)

    #define MEDIA_CHK(rval, act_func, err_operator)     do{ if((rval = act_func)) {                                 \
                                                                msg("call %s: rval (%d) !!\n", #act_func, rval);    \
                                                                err_operator;                                       \
                                                            }                                                       \
                                                        }while(0)
#else   /* #if 0 */

    #define err(str, argv...)                           msg(str, ##argv);

    #define MEDIA_CHK(rval, act_func, dummy)            do{ rval = act_func;if(rval != 0) {                 \
                                                              msg("error: rval (%d) !!\n", rval);           \
                                                              return rval;                                  \
                                                            }                                               \
                                                        }while(0)
#endif  /* #if 0 */

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct media_notification
{
    media_user_event_t      event;

    union {
        struct {
            uint32_t    muxing_len;
        } mux;
    };

} media_notification_t;

typedef int (*CB_USER_NOTIFY)(media_notification_t *pNotification, void *pUser_info);

struct media_stream_dev;
typedef int (*CB_SET_USER_STREAM_DEV)(struct media_stream_dev **ppStream_dev, media_stream_type_t stream_type);


typedef struct user_stream_info
{
    union {
        struct {
            // uint32_t    dev_id;
            uint32_t    resolution;
            uint32_t    frame_rate;
        } pcam;
    };
} user_stream_info_t;

typedef int (*CB_USER_CONFIGURATE_STERAM)(user_stream_info_t *pInfo);

/**
 *  media initial info
 */
struct media_user_io_desc;
struct media_render_msg;
struct media_user_render_desc;
struct media_user_heap_desc;
struct media_fmt_desc;
typedef struct media_mbox
{
    media_fmt_t     media_fmt;

    union {
        media_io_t      in_type;
        media_io_t      out_type;
    };

    union {
        char        *pPath;

        struct {
            uint8_t     *pStream_buf;
            uint32_t    stream_size;
        } mem;

        struct {
            uint8_t                     *pMedia_start_addr;
            uint32_t                    media_size;
            struct media_user_io_desc   *pIo_desc;
            void                        *pExtra_info;
        } usr;
    };

#define MEDIA_TRACK_VIDEO       (0x1)
#define MEDIA_TRACK_AUDIO       (0x1 << 1)

    uint32_t        tracks;

    struct media_user_heap_desc     *pHeap_desc;
    struct media_user_render_desc   *pRender_desc;

    media_codec_t       vcodec;
    media_codec_t       acodec;

    media_stream_dev_type_t     stream_dev_vid;
    CB_SET_USER_STREAM_DEV      cb_set_vid_stream_dev;

    media_stream_dev_type_t     stream_dev_aud;
    CB_SET_USER_STREAM_DEV      cb_set_aud_stream_dev;

    struct media_fmt_desc       *pUser_fmt_desc;
    CB_USER_CONFIGURATE_STERAM  cb_usr_configurate_stream;

    uint32_t            is_pause;

} media_mbox_t;

/**
 *  media controlling message box
 */
typedef struct media_ctrl_mbox
{
    media_ctrl_event_t      ctrl_event;

} media_ctrl_mbox_t;

/**
 *  media codec info
 */
typedef struct media_codec_set
{
    media_codec_t       codec_type;

    uint32_t            frm_state;

    union {
        struct {
            struct media_user_render_desc   *pRender_desc;
            struct media_user_heap_desc     *pHeap_desc;
            uint32_t                        frm_rate;
            int                             frm_hdr_size;
            uint32_t                        is_1st_frame;
        } dec_init;

        struct {
            uint8_t         *pStream_buf;
            int             stream_buf_len;
        } bs;

        struct {
            uint8_t         *pFrm_buf;
            int             frm_buf_len;
        } frm;
    };

} media_codec_set_t;

/**
 *  media operator description of media container
 */
typedef struct media_fmt_desc
{
    media_fmt_t     media_fmt;

    int (*cb_init)(media_mbox_t *pMedia_init);
    int (*cb_proc)(media_ctrl_mbox_t *pMedia_ctrl);
    int (*cb_deinit)(media_mbox_t *pMedia_init, media_notification_t *pNotification);

} media_fmt_desc_t;

/**
 *  media operator description of media codec
 */
typedef struct media_codec_desc
{
    media_codec_t       codec_type;

    int (*cb_init)(media_codec_set_t *pCodec_setting);
    int (*cb_proc)(media_codec_set_t *pCodec_setting);
    int (*cb_deinit)(media_codec_set_t *pCodec_setting);

} media_codec_desc_t;

/**
 *  setting info of media stream
 */
typedef struct media_stream_info
{
    media_stream_type_t     stream_type;
    uint32_t                frm_state;

    union {
        struct {
            uint16_t    width;
            uint16_t    height;
            uint32_t    fps;
        } vid;

        struct {
            uint32_t    sample_rate;
        } aud;
    };

    struct media_user_heap_desc     *pHeap_desc;

} media_stream_info_t;


/**
 *  descriptor of media stream device
 */
typedef struct media_stream_dev
{
#define MEDIA_OP_CODE_FREE_OUT_BUF          0x2

#define MEDIA_OP_CODE_CTRL_MASK             0xF0000000
#define MEDIA_OP_CODE_START_REC             (MEDIA_OP_CODE_CTRL_MASK | 0x1)
#define MEDIA_OP_CODE_STOP_REC              (MEDIA_OP_CODE_CTRL_MASK | 0x2)
#define MEDIA_OP_CODE_START_VIEW            (MEDIA_OP_CODE_CTRL_MASK | 0x3)
#define MEDIA_OP_CODE_STOP_VIEW             (MEDIA_OP_CODE_CTRL_MASK | 0x4)

    media_stream_dev_type_t    dev_type;

    int     (*cb_init)(media_stream_info_t *pInfo);
    int     (*cb_deinit)(media_stream_info_t *pInfo);

    int     (*cb_read)(media_stream_info_t *pInfo, uint8_t **ppBuf, int request_len, int *pReal_len, void *pExtra_info);
    int     (*cb_write)(media_stream_info_t *pInfo, uint8_t *pBuf, int request_len, int *pReal_len, void *pExtra_info);

    int     (*cb_set)(media_stream_info_t *pInfo, uint32_t op_code,void *pExtra_info);

} media_stream_dev_t;

/**
 *  user io description of media
 */
typedef struct media_user_io_desc
{
#define MEIDA_USER_IO_MODE_READ         0x1
#define MEIDA_USER_IO_MODE_WRITE        0x2

    int     (*cb_init)(uint32_t io_mode, void *pExtra_info);
    int     (*cb_deinit)(void *pExtra_info);

    int     (*cb_read)(void *pTo, void *pFrom, int request_len, int *pReal_len, void *pExtra_info);
    int     (*cb_write)(void *pTo, void *pFrom, int request_len, int *pReal_len, void *pExtra_info);
} media_user_io_desc_t;

/**
 *  message box of render
 */
typedef struct media_render_msg
{
    union{
        struct {
            uint32_t    *pOut_addr;
            uint32_t    width;
            uint32_t    pitch;
            uint32_t    height;
        } get_output_window;

        struct {
        #define MEIDA_FLAG_FLUSH_FRONTEND_SCREEN     (0x1 << 0)
        #define MEIDA_FLAG_FLUSH_BACKTEND_SCREEN     (0x1 << 1)

            uint32_t    align_w;
            uint32_t    align_h;
            uint32_t    is_flushing;
            uint32_t    color_rgb565;
        } reset_render;

        struct {
            uint32_t    frm_rate;
            uint32_t    align_w;
            uint32_t    align_h;
            uint32_t    blocking_ms;
            uint32_t    *pBackend_addr;
        } rendering;
    };
} media_render_msg_t;

/**
 *  user description of media render
 */
typedef struct media_user_render_desc
{
    int     (*cb_get_backend_window)(media_render_msg_t *pMsg);
    int     (*cb_reset_render)(media_render_msg_t *pMsg);
    int     (*cb_rendering)(media_render_msg_t *pMsg);
} media_user_render_desc_t;

/**
 *  descriptor of user handling memory heap
 */
typedef struct media_user_heap_desc
{
#define MEDIA_MEM_SIZE_AUTO                 (-1)

    int     (*cb_init)(struct media_user_heap_desc *pHeap_desc);
    int     (*cb_deinit)(struct media_user_heap_desc *pHeap_desc);
    int     (*cb_malloc)(uint8_t **ppBuf, media_mem_type_t mem_type, int request_size, int *pReal_size, uint32_t alignment_num);
    int     (*cb_free)(uint8_t **ppBuf, media_mem_type_t mem_type);

} media_user_heap_desc_t;



//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

void media_init(void);


void media_deinit(void);


int
media_start_msg(
    media_mbox_t    *pMsg);


int
media_register_notify(
    CB_USER_NOTIFY      cb_notify,
    void                *pUser_info);



#ifdef __cplusplus
}
#endif

#endif
