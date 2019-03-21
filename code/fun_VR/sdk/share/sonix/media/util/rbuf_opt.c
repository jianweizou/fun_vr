/**
 * Copyright (c) 2015 Shiu
 *
 * rbuf_opt is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * rbuf_opt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#include <string.h>
#include "rbuf_opt.h"

int
rb_opt_init(
    rb_operator_t   *pRbOpt,
    unsigned char   *pStart_ptr,
    unsigned int    buf_size)
{
    if( !pRbOpt )   return RB_ERR_INVALID_PARAM;
    pRbOpt->pBuf_start_ptr                      = (unsigned char*)(((unsigned int)pStart_ptr + 0x3) & ~0x3);
    pRbOpt->pRead_ptr[RB_READ_TYPE_FETCH]       = pRbOpt->pBuf_start_ptr;
    pRbOpt->pRead_ptr[RB_READ_TYPE_REMOVE]      = pRbOpt->pBuf_start_ptr;
    pRbOpt->pWrite_ptr                          = pRbOpt->pBuf_start_ptr;
    pRbOpt->pBuf_end_ptr                        = (unsigned char*)(((unsigned int)pStart_ptr + buf_size) & ~0x3);
    pRbOpt->pValid_end_ptr[RB_READ_TYPE_FETCH]  = pRbOpt->pBuf_end_ptr;
    pRbOpt->pValid_end_ptr[RB_READ_TYPE_REMOVE] = pRbOpt->pBuf_end_ptr;
    return RB_ERR_OK;
}

int
rb_opt_update_w(
    rb_operator_t       *pRbOpt,
    unsigned char       *pData,
    int                 data_size)
{
    unsigned char    *w_ptr = pRbOpt->pWrite_ptr;
    unsigned char    *r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_REMOVE];

    if( w_ptr > r_ptr ) {
        if( (w_ptr + data_size) < pRbOpt->pBuf_end_ptr ) {
            memcpy((void*)w_ptr, (void*)pData, data_size);
            w_ptr += data_size;
        } else {
            int     remain = pRbOpt->pBuf_end_ptr - w_ptr;

            if( remain + (r_ptr - pRbOpt->pBuf_start_ptr) <= data_size )
                return RB_ERR_NO_SPACE;

            memcpy((void*)w_ptr, (void*)pData, remain);
            pData += remain;
            remain = data_size - remain;
            if( remain )    memcpy((void*)pRbOpt->pBuf_start_ptr, (void*)pData, remain);

            w_ptr = pRbOpt->pBuf_start_ptr + remain;
        }
    } else {
        if( w_ptr != r_ptr && (w_ptr + data_size) >= r_ptr )
            return RB_ERR_W_CATCH_R;

        memcpy((void*)w_ptr, (void*)pData, data_size);
        w_ptr += data_size;
    }

    pRbOpt->pWrite_ptr = w_ptr;
    return RB_ERR_OK;
}

int
rb_opt_update_r(
    rb_operator_t       *pRbOpt,
    rb_read_type_t      read_idx,
    unsigned char       **ppData,
    int                 *pData_size,
    cb_get_data_size    get_item_size)
{
    int               rst = RB_ERR_OK;
    unsigned char     *w_ptr;
    unsigned char     *r_ptr;
    unsigned char     *e_ptr;
    unsigned int      item_size = 0;
    unsigned int      bDummy_item = 0;

    if( get_item_size == NULL )
        return RB_ERR_INVALID_PARAM;

    if( read_idx == RB_READ_TYPE_FETCH ) {
        w_ptr = pRbOpt->pWrite_ptr;
        r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_FETCH];
        e_ptr = pRbOpt->pValid_end_ptr[RB_READ_TYPE_FETCH];
    } else if( read_idx == RB_READ_TYPE_REMOVE ) {
        w_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_FETCH];
        r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_REMOVE];
        e_ptr = pRbOpt->pValid_end_ptr[RB_READ_TYPE_REMOVE];
    } else {
        return RB_ERR_INVALID_PARAM;
    }

    {
        rb_data_info_t  data_info = {.w_ptr = w_ptr, .r_ptr = r_ptr, .end_ptr = e_ptr,};

        if( w_ptr == r_ptr )    return RB_ERR_NO_DATA;

        item_size = (w_ptr < r_ptr) ? e_ptr - r_ptr : w_ptr - r_ptr - 4;
        if( *pData_size )
            item_size = (*pData_size < item_size) ? *pData_size : item_size;

        data_info.read_idx  = read_idx;
        data_info.data_size = item_size;
        if( (rst = get_item_size(&data_info)) != 0 ) {
            *ppData     = NULL;
            *pData_size = 0;
            return rst;
        }

        item_size = data_info.data_size;
    }

    if( item_size == 0 )    return RB_ERR_NO_DATA;

    if( w_ptr < r_ptr || (r_ptr + item_size) < w_ptr ) {
        *ppData     = r_ptr;
        *pData_size = item_size;

        r_ptr += item_size;

        if( r_ptr == pRbOpt->pValid_end_ptr[read_idx] ) {
            pRbOpt->pValid_end_ptr[read_idx] = pRbOpt->pBuf_end_ptr;
            r_ptr = pRbOpt->pBuf_start_ptr;
        }

        if( bDummy_item ) {
            *pData_size = RB_INVALID_SIZE;
        }
    } else {
        *ppData     = NULL;
        *pData_size = 0;
        rst = RB_ERR_R_CATCH_W;
    }

    pRbOpt->pRead_ptr[read_idx] = r_ptr;

    return rst;
}

int
rb_opt_peek_r(
    rb_operator_t       *pRbOpt,
    rb_read_type_t      read_idx,
    unsigned char       **ppData,
    int                 *pData_size,
    cb_get_data_size    get_item_size)
{
    int               rst = RB_ERR_OK;
    unsigned char     *w_ptr;
    unsigned char     *r_ptr;
    unsigned char     *e_ptr;
    unsigned int      item_size = 0;

    if( get_item_size == NULL )
        return  RB_ERR_INVALID_PARAM;

    if( read_idx == RB_READ_TYPE_FETCH ) {
        w_ptr = pRbOpt->pWrite_ptr;
        r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_FETCH];
        e_ptr = pRbOpt->pValid_end_ptr[RB_READ_TYPE_FETCH];
    } else if( read_idx == RB_READ_TYPE_REMOVE ) {
        w_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_FETCH];
        r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_REMOVE];
        e_ptr = pRbOpt->pValid_end_ptr[RB_READ_TYPE_REMOVE];
    } else
        return RB_ERR_INVALID_PARAM;

    {
        rb_data_info_t  data_info = {.w_ptr = w_ptr, .r_ptr = r_ptr, .end_ptr = e_ptr,};

        if( w_ptr == r_ptr )    return RB_ERR_NO_DATA;

        item_size = (w_ptr < r_ptr) ? e_ptr - r_ptr : w_ptr - r_ptr - 4;
        data_info.read_idx  = read_idx;
        data_info.data_size = item_size;
        if( (rst = get_item_size(&data_info)) != 0 ) {
            *ppData     = NULL;
            *pData_size = 0;
            return rst;
        }

        item_size = data_info.data_size;
    }

    if( w_ptr < r_ptr || (r_ptr + item_size) < w_ptr ) {
        *ppData     = r_ptr;
        *pData_size = item_size;
    } else {
        *ppData     = NULL;
        *pData_size = 0;
    }

    return RB_ERR_OK;
}

int
rb_opt_confirm_space(
    rb_operator_t   *pRbOpt,
    unsigned int    target_size)
{
    unsigned char    *w_ptr = pRbOpt->pWrite_ptr;
    unsigned char    *r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_REMOVE];
    unsigned int     remain_size = 0;

    if( w_ptr > r_ptr ) {
        remain_size = pRbOpt->pBuf_end_ptr - w_ptr;
        remain_size += (r_ptr - pRbOpt->pBuf_start_ptr);
    } else if( w_ptr == r_ptr ) {
        remain_size = pRbOpt->pBuf_end_ptr - pRbOpt->pBuf_start_ptr;
    } else
        remain_size = r_ptr - w_ptr;

    return (remain_size < target_size) ? 0 : 1;
}

int
rb_opt_remain_size(
    rb_operator_t   *pRbOpt)
{
    unsigned char    *w_ptr = pRbOpt->pWrite_ptr;
    unsigned char    *r_ptr = pRbOpt->pRead_ptr[RB_READ_TYPE_REMOVE];
    int              remain_size = 0;

    if( w_ptr > r_ptr ) {
        remain_size = pRbOpt->pBuf_end_ptr - w_ptr;
        remain_size += (r_ptr - pRbOpt->pBuf_start_ptr);
    } else if( w_ptr == r_ptr ) {
        remain_size = pRbOpt->pBuf_end_ptr - pRbOpt->pBuf_start_ptr;
    } else
        remain_size = r_ptr - w_ptr;

    return remain_size;
}

