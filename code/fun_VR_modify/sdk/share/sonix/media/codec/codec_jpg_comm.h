/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file codec_jpg_comm.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */

#ifndef __codec_jpg_comm_H_wQMw6WkC_l5Tk_HPwG_sLuH_udr4wPz6vkuq__
#define __codec_jpg_comm_H_wQMw6WkC_l5Tk_HPwG_sLuH_udr4wPz6vkuq__

#ifdef __cplusplus
extern "C" {
#endif

#include "media_comm.h"
#include "snc_jpeg.h"
#include "snc_csc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//#define JPG_CIS_HANDSHAKING_MODE

typedef enum jpg_operation_type
{
    JPG_OPERATION_IDLE          = 0,
    JPG_OPERATION_DECODING,
    JPG_OPERATION_DECODING_ZC,

} jpg_operation_type_t;

/**
 *  jpg operation commands
 */
typedef enum jpg_op_cmd
{
    JPG_OP_CMD_NONE,
    JPG_OP_CMD_INIT,
    JPG_OP_CMD_INIT_EX,
    JPG_OP_CMD_FILL_BIT_STREAM,
    JPG_OP_CMD_FILL_BIT_STREAM_ZC,
    JPG_OP_CMD_FILL_FRAME,
    JPG_OP_CMD_CHK_ENCODING_END,

} jpg_op_cmd_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct jpg_mgr
{
    jpg_operation_type_t    operation_type;
    jpeg_header_info_t      jpg_hdr;

    union {
        jpeg_decoder_handle_info_t      hJpg_dec;
        jpeg_encoder_handle_info_t      hJpg_enc;
    };

    uint32_t             cur_frm_buf_addr;
    uint32_t             cur_frm_buf_len;

} jpg_mgr_t;

typedef struct jpg_msg_box
{
    jpg_op_cmd_t    op_cmd;

    union {
        struct {
            jpg_operation_type_t        op_type;
            media_user_render_desc_t    *pRender_dec;
            media_user_heap_desc_t      *pHeap_desc;

            union {
                struct {
                    uint32_t                    frm_rate;
                    int                         frm_hdr_size;
                } dec;

                struct {
                    uint16_t                width;
                    uint16_t                height;
                    uint8_t                 *pOut_bs_buf;
                    int                     out_bs_buf_len;
                } enc;
            };
        } init;

        struct {
            uint8_t         *pBs_buf;
            int             bs_buf_len;
        } bs;

        struct {
            uint8_t         *pFrm_buf;
            int             frm_buf_len;
        } frm;

        uint32_t            frm_state;
    };

} jpg_msg_box_t;

typedef struct jbuf_mgr
{
    union {
        struct {
            uint32_t             *pSlice_buf_yuv;
            uint32_t             slice_buf_yuv_len;
            uint32_t             *pSlice_buf_rgb;
            uint32_t             slice_buf_rgb_len;
        } dec;

        struct {
            uint32_t            *pQ_table;
            uint32_t            q_table_len;
            uint32_t            *pJpg_header;
            uint32_t            jpg_header_len;
            uint32_t            *pLine_buf;
            uint32_t            line_buf_len;
        } enc;
    };
} jbuf_mgr_t;
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
jpg_comm_init(
    jpg_msg_box_t   *pBox);


int
jpg_comm_proc(
    jpg_msg_box_t   *pBox);


int
jpg_comm_deinit(
    jpg_msg_box_t   *pBox);



#ifdef __cplusplus
}
#endif

#endif
