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

#ifndef __rbuf_opt___H__
#define __rbuf_opt___H__

#ifdef __cplusplus
extern "C" {
#endif


#define RB_INVALID_SIZE         0xFFFFFFFFu

typedef int     rb_err_t;
#define RB_ERR_OK               0
#define RB_ERR_NO_SPACE         -1
#define RB_ERR_NO_DATA          -2
#define RB_ERR_W_CATCH_R        -3
#define RB_ERR_R_CATCH_W        -4
#define RB_ERR_INVALID_PARAM    -5


struct rb_data_info;
typedef int (*cb_get_data_size)(struct rb_data_info *pInfo);


typedef enum rb_read_type
{
    RB_READ_TYPE_FETCH       = 0,
    RB_READ_TYPE_REMOVE,
    RB_READ_TYPE_ALL,
}rb_read_type_t;


typedef struct rb_operator
{
    unsigned char     *pRead_ptr[RB_READ_TYPE_ALL];
    unsigned char     *pWrite_ptr;
    unsigned char     *pBuf_start_ptr;
    unsigned char     *pBuf_end_ptr;
    unsigned char     *pValid_end_ptr[RB_READ_TYPE_ALL];

}rb_operator_t;

typedef struct rb_data_info
{
    unsigned char   *w_ptr;
    unsigned char   *r_ptr;
    unsigned char   *end_ptr;

    rb_read_type_t  read_idx;
    unsigned int    data_size;
    unsigned int    is_dummy;

}rb_data_info_t;


int
rb_opt_init(
    rb_operator_t   *pRbOpt,
    unsigned char   *pStart_ptr,
    unsigned int    buf_size);


int
rb_opt_update_w(
    rb_operator_t       *pRbOpt,
    unsigned char       *pData,
    int                 data_size);


int
rb_opt_update_r(
    rb_operator_t       *pRbOpt,
    rb_read_type_t      read_idx,
    unsigned char       **ppData,
    int                 *pData_size,
    cb_get_data_size    get_data_size);


int
rb_opt_peek_r(
    rb_operator_t       *pRbOpt,
    rb_read_type_t      read_idx,
    unsigned char       **ppData,
    int                 *pData_size,
    cb_get_data_size    get_item_size);


int
rb_opt_confirm_space(
    rb_operator_t   *pRbOpt,
    unsigned int    target_size);

int
rb_opt_remain_size(
    rb_operator_t   *pRbOpt);


#ifdef __cplusplus
}
#endif

#endif
