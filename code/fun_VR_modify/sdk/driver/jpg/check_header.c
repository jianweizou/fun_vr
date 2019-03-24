/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file check_header.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */

#include <stdio.h>
#include "snc_jpeg.h"
#include "string.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_MARKER_START                0xFF

#define JPG_APP_MARK_FILTER             0xE0

#define JPG_BASELINE_MARKER             0xC0
#define JPG_EXTENDED_SEQUENTIAL         0xC1
#define JPG_PROGRESSIVE                 0xC2
#define JPG_LOSSLESS_SEQUENTIAL         0xC3
#define JPG_HUFFMAN_TABLE_MARKER        0xC4
#define JPG_DIFF_SEQUENTIAL             0xC5
#define JPG_DIFF_PROGRESSIVE            0xC6
#define JPG_DIFF_LOSSLESS               0xC7

#define JPG_START_OF_IMAGE_MARKER       0xD8
#define JPG_END_OF_IMAGE_MARKER         0xD9
#define JPG_START_OF_SCAN_MARKER        0xDA
#define JPG_Q_TABLE_MARKER              0xDB
#define JPG_DRI_MARKER                  0xDD

//=============================================================================
//                  Macro Definition
//=============================================================================
#define GET_HELF_BYTE_HIGH(pCurr)        ((*(pCurr) & 0xF0) >> 4)
#define GET_HELF_BYTE_LOW(pCurr)         (*(pCurr) & 0x0F)
#define GET_BYTE(pCurr)                  (*(pCurr))
#define GET_2_BYTES(pCurr)               (*(pCurr)<<8 | *(pCurr+1))
#define GET_4_BYTES(pCurr)               (*(pCurr)<<24 | *(pCurr+1)<<16 | *(pCurr+2)<<8 | *(pCurr+3))
#define GET_VALUE(pCurr, byte_cnt)       (((byte_cnt) == 2) ? GET_2_BYTES(pCurr) : GET_4_BYTES(pCurr))

#if 0
#define _assert(expression)                                        \
        do{ if(expression) break;                                       \
            printf("%s: %s[#%u]\n", #expression, __func__, __LINE__);   \
            while(1);                                                   \
        }while(0)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
uint32_t header_size = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
static uint8_t*
_complete_section(
    jpeg_header_info_t  *pJHdr,
    jpg_io_desc_t       *pIO_desc,
    uint8_t             *pCur,
    uint8_t             **ppEnd,
    void                *pUser_info)
{
    do {
        int         rval = 0;
        int         section_len = 0;
        int         keep_len = 0;
        uint32_t    read_len = 0;
        uint8_t     *pBS_buf = (uint8_t*)pJHdr->input_buf_addr;

        section_len = GET_VALUE(pCur, 2);
        if( (pCur + section_len) < *ppEnd )
            break;

        keep_len = *ppEnd - pCur;
        keep_len = (keep_len + 0x3) & (~0x3);
        memmove((void*)pBS_buf, (void*)((uint32_t)pCur & ~0x3), keep_len);

        if( !pIO_desc->cb_fill_buf )
        {
            pCur = 0;
            break;
        }

        rval = pIO_desc->cb_fill_buf(pBS_buf + keep_len, pJHdr->input_buf_len - keep_len, &read_len, pUser_info);
        if( rval )
        {
            pCur = 0;
            break;
        }

        *ppEnd = pBS_buf + keep_len + read_len;
        pCur = pBS_buf + ((uint32_t)pCur & 0x3);

    } while(0);

    return pCur;
}

static uint8_t*
_jpg_parse_SOF00(
    jpeg_header_info_t  *pJHdr,
    uint8_t             *pCur)
{
#define JPG_YUV420_SPACE        0x00221111
#define JPG_YUV422_SPACE        0x00211111
#define JPG_YUV422R_SPACE       0x00121111

    do {
        uint32_t    samp_factor = 0;
        int         comp_cnt = 0;
        // int         section_len = GET_2_BYTES(pCur);

        pJHdr->sof0_addr = (uint32_t)pCur - 2 - pJHdr->input_buf_addr;

        pCur += 3;

        pJHdr->frame_h = GET_2_BYTES(pCur); pCur += 2;
        pJHdr->frame_w = GET_2_BYTES(pCur); pCur += 2;

        comp_cnt = GET_BYTE(pCur);  pCur++;

        while( comp_cnt-- )
        {
            pCur++;
            samp_factor = (samp_factor << 8) | GET_BYTE(pCur);
            pCur += 2;
        }

        if( samp_factor == JPG_YUV420_SPACE )
            pJHdr->jpeg_fmt = JPEG_YCBCR_420;
        else if( samp_factor == JPG_YUV422_SPACE ||
                 samp_factor == JPG_YUV422R_SPACE )
            pJHdr->jpeg_fmt = JPEG_YCBCR_422;

    } while(0);
    return pCur;
}

static uint8_t*
_jpg_parse_DQT(
    jpeg_header_info_t  *pJHdr,
    uint8_t             *pCur)
{
    int     i;
    int     table_cnt = 0;
    int     section_len = 0;

    if( pJHdr->dqt_addr == 0 )
    {
        pJHdr->dqt_addr = (uint32_t)pCur - 2 - pJHdr->input_buf_addr;
    }

    section_len = GET_2_BYTES(pCur); pCur += 2;

    table_cnt = (section_len - 2) / 65;

    for(i = 0; i < table_cnt; ++i)
    {
        int        q_idx = 0;

        q_idx = GET_HELF_BYTE_LOW(pCur);    pCur++;
        q_idx &= 0x1;

        memcpy((void*)(pJHdr->qtable + (q_idx << 6)), pCur, 64);
        pCur += 64;
    }

    return pCur;
}

static uint8_t*
_jpg_parse_SOS(
    jpeg_header_info_t  *pJHdr,
    uint8_t             *pCur)
{
    int     section_len = 0;

    pJHdr->sos_addr = (uint32_t)pCur - 2 - pJHdr->input_buf_addr;

    section_len = GET_2_BYTES(pCur);

    pCur += section_len;

    return pCur;
}

int
JPEG_Header_Parsing(
    jpeg_header_info_t  *pJHdr,
    jpg_io_desc_t       *pIO_desc,
    void                *pUser_info)
{
    int         rval = 0;
    uint8_t     *pCur = 0, *pEnd = 0;

    // _assert(pJHdr);
    // _assert(pIO_desc);

    if( !pJHdr->input_buf_addr || !pJHdr->input_buf_len )
    {
        uint32_t    buf_size = 0;
        if( !pIO_desc->cb_get_buf )
            return -1;

        pIO_desc->cb_get_buf(&pCur, &buf_size);
        // _assert(pCur != 0);
        // _assert(buf_size != 0);

        buf_size &= (~0x3);

        pJHdr->input_buf_addr = (uint32_t)pCur;
        pJHdr->input_buf_len  = buf_size;

        pEnd = pCur + buf_size;
    }
    else
    {
        uint32_t    read_len = 0;
        rval = pIO_desc->cb_fill_buf((uint8_t*)pJHdr->input_buf_addr, pJHdr->input_buf_len, &read_len, pUser_info);
        if( rval )
            return rval;

        pCur = (uint8_t*)pJHdr->input_buf_addr;
        pEnd = pCur + read_len;
    }

    // check 0xFFD8
    if( *pCur != JPG_MARKER_START ||
        *(pCur + 1) != JPG_START_OF_IMAGE_MARKER )
        return -1;

    pJHdr->header_length = 0;

    while( pCur < pEnd )
    {
        uint32_t    bExit = false;

        if( *pCur++ != JPG_MARKER_START )
            continue;

        switch( *pCur )
        {
            default:
                if( (*pCur & 0xF0) == JPG_APP_MARK_FILTER ||
                    (*pCur & 0xF0) == 0xF0 )
                {
                    int   section_len = 0;
                    // APP section
                    pCur++;
                    section_len = GET_VALUE(pCur, 2);

                    if( (pCur + section_len) < pEnd )
                        pCur += section_len;
                    else
                    {
                        uint32_t    read_len = 0;

                        section_len -= (pEnd - pCur);
                        while( section_len >= 0 )
                        {
                            rval = pIO_desc->cb_fill_buf((uint8_t*)pJHdr->input_buf_addr, pJHdr->input_buf_len,
                                                         &read_len, pUser_info);
                            if( rval )
                            {
                                bExit = true;
                                break;
                            }

                            if( section_len < read_len )
                            {
                                pCur = (uint8_t*)(pJHdr->input_buf_addr + section_len);
                                pEnd = (uint8_t*)(pJHdr->input_buf_addr + read_len);
                                break;
                            }
                            else
                            {
                                section_len -= read_len;
                            }
                        }
                    }
                    break;
                }

                pCur++;
                break;

            case JPG_BASELINE_MARKER:
                pCur++;
                pCur = _complete_section(pJHdr, pIO_desc, pCur, &pEnd, pUser_info);
                if( !pCur )
                {
                    rval = -1;
                    bExit = true;
                    break;
                }

                pCur = _jpg_parse_SOF00(pJHdr, pCur);
                break;

            case JPG_HUFFMAN_TABLE_MARKER:
                {
                    int     section_len = 0;
                    pCur++;
                    pCur = _complete_section(pJHdr, pIO_desc, pCur, &pEnd, pUser_info);
                    if( !pCur )
                    {
                        rval = -1;
                        bExit = true;
                        break;
                    }

                    section_len = GET_VALUE(pCur, 2);
                    if( section_len != 31 && section_len != 181 && section_len != 418 )
                    {
                        rval = -2;
                        bExit = true;
                        break;
                    }

                    pCur += section_len;
                }
                break;

            case JPG_START_OF_IMAGE_MARKER:
                pCur++;
                break;
            case JPG_END_OF_IMAGE_MARKER:
                bExit = true;
                break;

            case JPG_START_OF_SCAN_MARKER:
                pCur++;
                pCur = _complete_section(pJHdr, pIO_desc, pCur, &pEnd, pUser_info);
                if( !pCur )
                {
                    rval = -1;
                    bExit = true;
                    break;
                }

                pCur = _jpg_parse_SOS(pJHdr, pCur);
                bExit = true;
                break;

            case JPG_Q_TABLE_MARKER:
                pCur++;
                pCur = _complete_section(pJHdr, pIO_desc, pCur, &pEnd, pUser_info);
                if( !pCur )
                {
                    rval = -1;
                    bExit = true;
                    break;
                }

                pCur = _jpg_parse_DQT(pJHdr, pCur);
                break;

            case JPG_EXTENDED_SEQUENTIAL:
            case JPG_PROGRESSIVE:
            case JPG_LOSSLESS_SEQUENTIAL:
            case JPG_DIFF_SEQUENTIAL:
            case JPG_DIFF_PROGRESSIVE:
            case JPG_DIFF_LOSSLESS:
            case JPG_DRI_MARKER:
                {   // skip section
                    int   section_len = 0;

                    pCur++;
                    section_len = GET_VALUE(pCur, 2);


                    if( (pCur + section_len) < pEnd )
                        pCur += section_len;
                    else
                    {
                        uint32_t    read_len = 0;

                        section_len -= (pEnd - pCur);
                        while( section_len >= 0 )
                        {
                            rval = pIO_desc->cb_fill_buf((uint8_t*)pJHdr->input_buf_addr, pJHdr->input_buf_len,
                                                         &read_len, pUser_info);
                            if( rval )
                            {
                                bExit = true;
                                break;
                            }

                            if( section_len < read_len )
                            {
                                pCur = (uint8_t*)(pJHdr->input_buf_addr + section_len);
                                pEnd = (uint8_t*)(pJHdr->input_buf_addr + read_len);
                                break;
                            }
                            else
                            {
                                section_len -= read_len;
                            }
                        }
                    }
                }
                break;
        }

        if( bExit )
        {
            pJHdr->header_length = (uint32_t)pCur - pJHdr->input_buf_addr;
            pJHdr->skip_byte     = pJHdr->header_length & 0x3;
            break;
        }
    }

    return rval;
}

int JPEG_Header_Update(jpeg_header_info_t *header, uint32_t bitstream_addr, uint32_t size)
{
    uint16_t Note1, Note2;
    uint16_t Tag = *(uint16_t*)bitstream_addr;
    uint32_t cunt = 0;
    uint32_t i = 0;

    header_size = size;
//  Tag = *(uint16_t*)Bitstream_addr;
    if(Tag == 0xD8FF){
        while(header->dqt_addr == 0 || header->sof0_addr == 0)
        {
            Tag = *(uint16_t*)(bitstream_addr+cunt);
            if(Tag == 0xC0FF)
            {
                header->sof0_addr = cunt;
                *(uint8_t *)(bitstream_addr+header->sof0_addr+0x5) = (header->frame_h>>8) & 0xFF;
                *(uint8_t *)(bitstream_addr+header->sof0_addr+0x6) = header->frame_h & 0xFF;
                *(uint8_t *)(bitstream_addr+header->sof0_addr+0x7) = (header->frame_w>>8) & 0xFF;
                *(uint8_t *)(bitstream_addr+header->sof0_addr+0x8) = header->frame_w & 0xFF;
                if(header->jpeg_fmt == JPEG_YCBCR_420)
                {
                    *(uint8_t *)(bitstream_addr+header->sof0_addr+0xB) = 0x22;
                    *(uint8_t *)(bitstream_addr+header->sof0_addr+0xE) = 0x11;
                    *(uint8_t *)(bitstream_addr+header->sof0_addr+0x11) = 0x11;
                }
                else if(header->jpeg_fmt == JPEG_YCBCR_422)
                {
                    *(uint8_t *)(bitstream_addr+header->sof0_addr+0xB) = 0x21;
                    *(uint8_t *)(bitstream_addr+header->sof0_addr+0xE) = 0x11;
                    *(uint8_t *)(bitstream_addr+header->sof0_addr+0x11) = 0x11;
                }
                Note1 = (*(uint16_t*)(bitstream_addr + cunt + 2)<<8) & 0xFF00;
                Note2 = (*(uint16_t*)(bitstream_addr + cunt + 2)>>8) & 0x00FF;
                cunt += Note1 | Note2 + 2;
            }
            else if(Tag == 0xDBFF)
            {
                header->dqt_addr =  cunt;
                for(i = 0; i < 64; i ++)
                {
                    *(uint8_t *)(bitstream_addr+header->dqt_addr + 5 + i) = *(uint8_t *)(header->qtable + i);
                }
                if(*(uint8_t*)(bitstream_addr+header->dqt_addr+5+64) == 0x1)
                {
                    for(i=0 ; i<64 ; i++)
                    {
                        *(uint8_t*)(bitstream_addr+header->dqt_addr+5+i+65) = *(uint8_t *)(header->qtable+i+64);
                    }
                }
                else if(*(uint8_t*)(bitstream_addr+header->dqt_addr+5+64) == 0xFF)
                {
                    if(*(uint8_t*)(bitstream_addr+header->dqt_addr+5+65) == 0xDB)
                    {
                        for(i=0 ; i<64 ; i++)
                        {
                            *(uint8_t*)(bitstream_addr+header->dqt_addr+5+i+69) =  *(uint8_t *)(header->qtable+i+64);
                        }
                    }
                    else
                    {
                        for(i=0 ; i<64 ; i++)
                        {
                            *(uint8_t*)(bitstream_addr+header->dqt_addr+5+i) = *(uint8_t *)(header->qtable+i+64);
                        }
                    }
                }
                Note1 = (*(uint16_t*)(bitstream_addr + cunt + 2)<<8) & 0xFF00;
                Note2 = (*(uint16_t*)(bitstream_addr + cunt + 2)>>8) & 0x00FF;
                cunt += Note1 | Note2 + 2;
            }
            else if(Tag == 0xE0FF)
            {
                Note1 = (*(uint16_t*)(bitstream_addr + cunt + 2)<<8) & 0xFF00;
                Note2 = (*(uint16_t*)(bitstream_addr + cunt + 2)>>8) & 0x00FF;
                cunt += Note1 | Note2 + 2;
            }
            else if(Tag == 0xEDFF)
            {
                Note1 = (*(uint16_t*)(bitstream_addr + cunt + 2)<<8) & 0xFF00;
                Note2 = (*(uint16_t*)(bitstream_addr + cunt + 2)>>8) & 0x00FF;
                cunt += Note1 | Note2 + 2;
            }
            else if(Tag == 0xFEFF)
            {
                Note1 = (*(uint16_t*)(bitstream_addr + cunt + 2)<<8) & 0xFF00;
                Note2 = (*(uint16_t*)(bitstream_addr + cunt + 2)>>8) & 0x00FF;
                cunt += Note1 | Note2 + 2;
            }
            else if(Tag == 0xC4FF)
            {
                Note1 = (*(uint16_t*)(bitstream_addr + cunt + 2)<<8) & 0xFF00;
                Note2 = (*(uint16_t*)(bitstream_addr + cunt + 2)>>8) & 0x00FF;
                cunt += Note1 | Note2 + 2;
            }
            else
            {
                cunt++;
            }
            if(cunt > size)
            {
                return -1;
            }
        }
    }
        return 0;
}

uint32_t JPEG_Header_Size(void)
{
    return header_size;
}

static uint32_t _Header_Tag_Len(uint8_t uHeighByte, uint8_t uLowByte)
{
    return ((uHeighByte*0x100) | uLowByte) + 2;
}

int JPEG_Header_Check(
            jpeg_header_info_t          *header,
            CB_JPEG_FILL_BUF            cb_fill_data,
            uint32_t                    file_size,
            void                        *pUser_info)
{
    uint8_t Tag1, Tag2;
    uint32_t cunt = 0;
    uint32_t i, offset=0;
    uint32_t Tag_adr, Tag_len;
    uint16_t Hbyte, Lbyte;
    cb_jpeg_info_t cb_info;

    cb_info.buffer_addr = header->input_buf_addr;
    cb_info.len = header->input_buf_len;
    cb_info.ptunnel_info = pUser_info;
    if(cb_fill_data(&cb_info) == -1)
    {
        return -1;
    }
    header->input_buf_addr = cb_info.buffer_addr;
    header->input_buf_len = cb_info.len;

    header->header_length += header->input_buf_len;
    while(1)
    {
        Tag1 = *(uint8_t*)(header->input_buf_addr + cunt);

        if(Tag1 == 0xFF)
        {
            if(cunt +1 >= header->input_buf_len)
            {
//                header->Input_buf_addr += header->Input_buf_len;
                cb_info.buffer_addr = header->input_buf_addr;
                cb_info.len = header->input_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_fill_data(&cb_info) == -1)
                {
                    return -1;
                }
                header->input_buf_addr = cb_info.buffer_addr;
                header->input_buf_len = cb_info.len;

                header->header_length += header->input_buf_len;
                cunt = 0;
                Tag2 = *(uint8_t*)(header->input_buf_addr + cunt);
                offset = 1;
            }
            else
            {
                offset = 0;
                Tag2 = *(uint8_t*)(header->input_buf_addr + cunt + 1);
            }

            if(Tag2 == 0xC0)
            {
                header->sof0_addr = cunt;
                Tag_adr = cunt;
                for(i=2;i<=3;i++)
                {
                    if(Tag_adr + i >= header->input_buf_len)
                    {
                        if(i == 3)
                        {
                            Hbyte = *(uint8_t*)(header->input_buf_addr + cunt + 2);
                        }
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;

                        header->header_length += header->input_buf_len;
                        cunt = 0;
                        offset = i;
                        Tag_adr += offset - header->input_buf_len;
                        break;
                    }
                }
                if(offset == 3)
                {
                    Tag_len = _Header_Tag_Len(Hbyte,*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                else
                {
                    Tag_len =
                    _Header_Tag_Len(*(uint8_t*)(header->input_buf_addr + cunt + 2-offset),*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                cunt += Tag_len;

                for(i=5;i<9;i++)
                {
                    if((Tag_adr + i - offset) >= header->input_buf_len)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;

                        header->header_length += header->input_buf_len;
                        cunt %= header->input_buf_len;
                        Tag_adr = Tag_adr + i - offset - header->input_buf_len;
                        offset = i;
                        if(i%2 == 1)
                        {
                            Hbyte = *(uint8_t*)(header->input_buf_addr + Tag_adr);
                        }
                        else
                        {
                            Lbyte = *(uint8_t*)(header->input_buf_addr + Tag_adr);
                            if(i== 6)
                            {
                                header->frame_h = Hbyte<<8 | Lbyte;
                            }
                            else if(i == 8)
                            {
                                header->frame_w = Hbyte<<8 | Lbyte;
                            }
                        }
                    }
                    else
                    {
                        if(i%2 == 1)
                        {
                            Hbyte = *(uint8_t*)(header->input_buf_addr + Tag_adr + i - offset);
                        }
                        else
                        {
                            Lbyte = *(uint8_t*)(header->input_buf_addr + Tag_adr + i - offset);
                            if(i== 6)
                            {
                                header->frame_h = Hbyte<<8 | Lbyte;
                            }
                            else if(i == 8)
                            {
                                header->frame_w = Hbyte<<8 | Lbyte;
                            }
                        }
                    }
                }
                for(i=11;i<=17;i+=3)
                {
                    if((Tag_adr + i - offset) >= header->input_buf_len)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;

                        header->header_length += header->input_buf_len;
                        cunt %= header->input_buf_len;
                        Tag_adr = Tag_adr + i - offset - header->input_buf_len;
                        offset = i;
                        header->jpeg_fmt += ((*(uint8_t*)(header->input_buf_addr + Tag_adr))>>4) *
                                            ((*(uint8_t*)(header->input_buf_addr + Tag_adr)) & 0x0F);
                    }
                    else
                    {
                        header->jpeg_fmt += ((*(uint8_t*)(header->input_buf_addr + Tag_adr + i - offset))>>4) *
                                            ((*(uint8_t*)(header->input_buf_addr + Tag_adr + i - offset)) & 0x0F);
                    }
                }

                if(header->jpeg_fmt%4 == 0)
                {
                    header->jpeg_fmt = JPEG_YCBCR_422;
                }
                else if(header->jpeg_fmt%6 == 0)
                {
                    header->jpeg_fmt = JPEG_YCBCR_420;
                }

                if(cunt >= header->input_buf_len)
                {
                    for(i = 0; i<cunt/header->input_buf_len; i++)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;

                        header->header_length += header->input_buf_len;
                    }
                    cunt %= header->input_buf_len;
                }
                offset = 0;
            }
            else if(Tag2 == 0xDB)
            {
                header->dqt_addr = cunt;
                for(i=0;i<=128;i++)
                {
                    *(uint8_t*)(header->qtable+i) = 0;
                }

                Tag_adr = cunt;
                for(i=2;i<=3;i++)
                {
                    if(Tag_adr + i >= header->input_buf_len)
                    {
                        if(i == 3)
                        {
                            Hbyte = *(uint8_t*)(header->input_buf_addr + cunt + 2);
                        }
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;

                        header->header_length += header->input_buf_len;
                        cunt = 0;
                        offset = i;
                        Tag_adr += offset - header->input_buf_len;
                        break;
                    }
                }
                if(offset == 3)
                {
                    Tag_len = _Header_Tag_Len(Hbyte,*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                else
                {
                    Tag_len =
                    _Header_Tag_Len(*(uint8_t*)(header->input_buf_addr + cunt + 2-offset),*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                cunt += Tag_len;
                Tag_adr += 5-offset;
                if(Tag_adr >= header->input_buf_len)
                {
                    cb_info.buffer_addr = header->input_buf_addr;
                    cb_info.len = header->input_buf_len;
                    cb_info.ptunnel_info = pUser_info;
                    if(cb_fill_data(&cb_info) == -1)
                    {
                        return -1;
                    }
                    header->input_buf_addr = cb_info.buffer_addr;
                    header->input_buf_len = cb_info.len;
                    header->header_length += header->input_buf_len;
                    Tag_adr -= header->input_buf_len;
                }

                for(i=0 ; i<64 ; i++)
                {
                    *(uint8_t*)(header->qtable+i) = *(uint8_t*)(header->input_buf_addr + Tag_adr);
                    Tag_adr ++;
                    if(Tag_adr >= header->input_buf_len)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                        Tag_adr = 0;
                        offset = 0;
                    }
                }

                if(*(uint8_t*)(header->input_buf_addr + Tag_adr) == 0x1)
                {
                    Tag_adr ++;
                    if(Tag_adr >= header->input_buf_len)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                        Tag_adr = 0;
                    }
                    for(i=0 ; i<64 ; i++)
                    {
                        *(uint8_t*)(header->qtable + i + 64) = *(uint8_t*)(header->input_buf_addr + Tag_adr);
                        Tag_adr ++;
                        if(Tag_adr >= header->input_buf_len)
                        {
                            cb_info.buffer_addr = header->input_buf_addr;
                            cb_info.len = header->input_buf_len;
                            cb_info.ptunnel_info = pUser_info;
                            if(cb_fill_data(&cb_info) == -1)
                            {
                                return -1;
                            }
                            header->input_buf_addr = cb_info.buffer_addr;
                            header->input_buf_len = cb_info.len;
                            header->header_length += header->input_buf_len;
                            Tag_adr = 0;
                        }
                    }
                }
                else if(*(uint8_t*)(header->input_buf_addr+Tag_adr) == 0xFF)
                {
                    Tag_adr ++;
                    if(Tag_adr >= header->input_buf_len)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                        Tag_adr = 0;
                    }
                    if(*(uint8_t*)(header->input_buf_addr+Tag_adr) == 0xDB)
                    {
                        Tag_adr += 4;
                        if(Tag_adr >= header->input_buf_len)
                        {
                            cb_info.buffer_addr = header->input_buf_addr;
                            cb_info.len = header->input_buf_len;
                            cb_info.ptunnel_info = pUser_info;
                            if(cb_fill_data(&cb_info) == -1)
                            {
                                return -1;
                            }
                            header->input_buf_addr = cb_info.buffer_addr;
                            header->input_buf_len = cb_info.len;
                            header->header_length += header->input_buf_len;
                            Tag_adr -= header->input_buf_len;
                        }
                        for(i=0 ; i<64 ; i++)
                        {
                            *(uint8_t*)(header->qtable + i + 64) = *(uint8_t*)(header->input_buf_addr + Tag_adr);
                            Tag_adr ++;
                            if(Tag_adr >= header->input_buf_len)
                            {
                                cb_info.buffer_addr = header->input_buf_addr;
                                cb_info.len = header->input_buf_len;
                                cb_info.ptunnel_info = pUser_info;
                                if(cb_fill_data(&cb_info) == -1)
                                {
                                    return -1;
                                }
                                header->input_buf_addr = cb_info.buffer_addr;
                                header->input_buf_len = cb_info.len;
                                header->header_length += header->input_buf_len;
                                Tag_adr = 0;
                            }
                        }
                    }
                }
                else
                {
                    for(i=0 ; i<64 ; i++)
                    {
                        *(uint8_t*)(header->qtable + i + 64) = *(uint8_t*)(header->qtable + i);
                    }
                }
                cunt %= header->input_buf_len;
            }
            else if(Tag2 == 0xDA)
            {
                Tag_adr = cunt;

                for(i=2;i<=3;i++)
                {
                    if(Tag_adr + i >= header->input_buf_len)
                    {
                        if(i == 3)
                        {
                            Hbyte = *(uint8_t*)(header->input_buf_addr + cunt + 2);
                        }
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                        cunt = 0;
                        offset = i;
                        break;
                    }
                }
                if(offset == 3)
                {
                    Tag_len = _Header_Tag_Len(Hbyte,*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                else
                {
                    Tag_len =
                    _Header_Tag_Len(*(uint8_t*)(header->input_buf_addr + cunt + 2-offset),*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                cunt += Tag_len;

                if(cunt >= header->input_buf_len)
                {
                    for(i = 0; i<cunt/header->input_buf_len; i++)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                    }
                    cunt %= header->input_buf_len;
                }
                offset = 0;

                header->sos_addr = cunt;
                header->skip_byte = header->sos_addr%4;
                header->header_length = header->header_length - header->input_buf_len + header->sos_addr;
                return 0;
            }
            else if((Tag2 == 0xE0) || (Tag2 == 0xED) || (Tag2 == 0xFE) || (Tag2 == 0xC4))
            {
                Tag_adr = cunt;

                for(i=2;i<=3;i++)
                {
                    if(Tag_adr + i >= header->input_buf_len)
                    {
                        if(i == 3)
                        {
                            Hbyte = *(uint8_t*)(header->input_buf_addr + cunt + 2);
                        }
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                        cunt = 0;
                        offset = i;
                        Tag_adr += offset - header->input_buf_len;
                        break;
                    }
                }
                if(offset == 3)
                {
                    Tag_len = _Header_Tag_Len(Hbyte,*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                else
                {
                    Tag_len =
                    _Header_Tag_Len(*(uint8_t*)(header->input_buf_addr + cunt + 2-offset),*(uint8_t*)(header->input_buf_addr + cunt + 3-offset))-offset;
                }
                cunt += Tag_len;

                if(cunt >= header->input_buf_len)
                {
                    for(i = 0; i<cunt/header->input_buf_len; i++)
                    {
                        cb_info.buffer_addr = header->input_buf_addr;
                        cb_info.len = header->input_buf_len;
                        cb_info.ptunnel_info = pUser_info;
                        if(cb_fill_data(&cb_info) == -1)
                        {
                            return -1;
                        }
                        header->input_buf_addr = cb_info.buffer_addr;
                        header->input_buf_len = cb_info.len;
                        header->header_length += header->input_buf_len;
                    }
                    cunt %= header->input_buf_len;
                }
                offset = 0;
            }
            else
            {
                cunt ++;
            }
        }
        else
        {
            cunt ++;
            if(cunt >= header->input_buf_len)
            {
                cb_info.buffer_addr = header->input_buf_addr;
                cb_info.len = header->input_buf_len;
                cb_info.ptunnel_info = pUser_info;
                if(cb_fill_data(&cb_info) == -1)
                {
                    return -1;
                }
                header->input_buf_addr = cb_info.buffer_addr;
                header->input_buf_len = cb_info.len;
                header->header_length += header->input_buf_len;
                cunt = 0;
            }
        }
    }
}
