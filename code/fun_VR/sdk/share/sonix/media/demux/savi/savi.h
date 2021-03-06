/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file savi.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */

#ifndef __savi_demuxer_H_w9relPd9_lbJq_H6QR_sT9X_uZxfSQMDC24y__
#define __savi_demuxer_H_w9relPd9_lbJq_H6QR_sT9X_uZxfSQMDC24y__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum savi_track
{
    SAVI_TRACK_NONE = 0,
    SAVI_TRACK_AUDIO,
    SAVI_TRACK_VIDEO,

} savi_track_t;

typedef enum savi_audio_codec
{
    SAVI_CODEC_AUDIO32       =  0x5541,
    SAVI_CODEC_SN_IMA_ADPCM  =  0x04D4,
    SAVI_CODEC_WINDOWS_PCM   =  0x5601,
    SAVI_CODEC_IMA_ADPCM     =  0x5600,

} savi_audio_codec_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  frame info
 */
typedef struct savi_frm_info
{
    long        header_offset;
    int         header_length;

    long        curr_frm_offset_in_idx_table;
    long        next_frm_offset_in_idx_table;

} savi_frm_info_t;

struct savi_demux_ctrl;

/**
 *  \brief CB_FRAME_INFO    callback function when demuxes a frame
 *
 *  \param [in] pCtrl           pass control info from savi_demux()
 *  \param [in] pFrm_info       info of a frame
 *  \return Return              0: ok, other: fail
 *
 *  \details
 */
typedef int (*CB_FRAME_INFO)(struct savi_demux_ctrl *pCtrl, savi_frm_info_t *pFrm_info);

/**
 *  demux controlling setting
 */
typedef struct savi_demux_ctrl
{
    int                     frame_idx;
    CB_FRAME_INFO           cb_frame_info;


    /**
     *  attach user data
     */
    void    *pTunnel_info;

} savi_demux_ctrl_t;

/**
 *  header structure
 */
typedef struct savi_header
{
    uint32_t        tag;

    uint32_t        raw_data_offset_vid;
    uint32_t        raw_data_size_vid;
    uint32_t        frame_rate;
    uint32_t        frame_max_num;

    uint32_t        raw_data_offset_aud;
    uint32_t        raw_data_size_aud;
    uint32_t        sample_rate;

    uint32_t        idx_table_offset_vid;
    uint32_t        idx_table_size_vid;

    uint32_t        jpg_hdr_offset;
    uint32_t        jpg_hdr_size;

} savi_header_t;





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
 *  \brief  Parse header of a SAVI file
 *
 *  \param [in] pBuf        stream of a savi file
 *  \param [in] buf_size    buffer size
 *  \param [in] pHdr        report result after parsing
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
savi_parse_header(
    uint32_t        *pBuf,
    uint32_t        buf_size,
    savi_header_t   *pHdr);

/**
 *  \brief  Only demux video frame
 *
 *  \param [in] pHdr            header info which is generated by savi_parse_header()
 *  \param [in] pCtrl_info      controlling setting for demuxing
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
savi_demux_video(
    savi_header_t       *pHdr,
    savi_demux_ctrl_t   *pCtrl_info);





#ifdef __cplusplus
}
#endif

#endif
