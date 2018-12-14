#include "buffer.h"
#include <stdlib.h>
#include <string.h>

static int32_t power2(int32_t val)
{
    int rval = 1;
	while(rval < val) rval <<= 1;
    return rval;
}

struct buf_circle* buf_create_circle(int32_t capacity)
{
    capacity = power2(capacity);
    struct buf_circle* buf = (struct buf_circle*)malloc(sizeof(struct buf_circle) + capacity);
    buf->buf = (int8_t*)(buf + 1);
    buf->cap = capacity;
    buf->data_sz = buf->rd_idx = buf->wt_idx = 0;
    return buf;
}

struct buf_circle* buf_relloc_circle(struct buf_circle* buf)
{
    int32_t capacity = buf->cap * 2;
    struct buf_circle* new_buf = (struct buf_circle*)malloc(sizeof(struct buf_circle) + capacity);
    new_buf->buf = (int8_t*)(new_buf + 1);
    new_buf->cap = capacity;
    new_buf->data_sz = buf->data_sz;    
    buf_read_circle(buf, new_buf->buf, buf->data_sz);
    new_buf->rd_idx = 0;
    new_buf->wt_idx = buf->data_sz;

    buf_destroy_circle(buf);
    return new_buf;
}

void buf_destroy_circle(struct buf_circle* buf)
{
    free(buf);
}

int32_t buf_space_circle(struct buf_circle* buf)
{
    return buf->cap - buf->data_sz;
}

int32_t buf_read_circle(struct buf_circle* buf, int8_t* out_b, int32_t len)
{
    int32_t sz = buf->data_sz < len ? buf->data_sz : len;
    if (sz <= 0)
    {
        return 0;
    }
    if (buf->rd_idx < buf->wt_idx)
    {
        memcpy(out_b, buf->buf + buf->rd_idx, sz);
        buf->rd_idx += sz;
    }
    else
    {
        int32_t r = buf->cap - buf->rd_idx;
        if (sz <= r)
        {
            memcpy(out_b, buf->buf + buf->rd_idx, sz);
            if (sz == r) buf->rd_idx = 0;
            else buf->rd_idx += sz;
        }
        else
        {
            memcpy(out_b, buf->buf + buf->rd_idx, r);
            memcpy(out_b + r, buf->buf, sz - r);
            buf->rd_idx = sz - r;
        }
    }
    buf->data_sz -= sz;
    return sz;
}

int32_t buf_write_circle(struct buf_circle* buf, int8_t* in_b, int32_t len)
{
    int32_t sp_sz = buf_space_circle(buf);
	int32_t sz = sp_sz < len ? sp_sz : len;
    if (sz <= 0)
    {
        return 0;
    }
    if (buf->wt_idx >= buf->rd_idx)
    {
        int32_t r = buf->cap - buf->wt_idx;
        if (sz <= r)
        {
            memcpy(buf->buf + buf->wt_idx, in_b, sz);
            if (sz == r) buf->wt_idx = 0;
            else buf->wt_idx += sz;
        }
        else
        {
            memcpy(buf->buf + buf->wt_idx, in_b, r);
            memcpy(buf->buf, in_b + r, sz - r);
            buf->wt_idx = sz - r;
        }
    }
    else
    {
        memcpy(buf->buf + buf->wt_idx, in_b, sz);
        buf->wt_idx += sz;
    }
    buf->data_sz += sz;
    return sz;
}


struct buf_data* buf_create_data(int32_t capacity)
{
    capacity = power2(capacity);
    struct buf_data* buf = (struct buf_data*)malloc(sizeof(struct buf_data) + capacity);
    buf->buf = (int8_t*)(buf + 1);
    buf->cap = capacity;
    buf->st_idx = 0;
    buf->ed_idx = 0;
    return buf;
}

int32_t buf_space_data(struct buf_data* buf)
{
    return buf->cap - buf->ed_idx;
}

int32_t buf_size_data(struct buf_data* buf)
{
    return buf->ed_idx - buf->st_idx;
}

void buf_reinit_data(struct buf_data* buf)
{
    buf->st_idx = buf->ed_idx = 0;
}

int32_t buf_write_data(struct buf_data* buf, int8_t* in_b, int32_t len)
{
    if (buf->cap < len)
        return B_ERROR;
    int32_t sz = buf->cap - buf->ed_idx;
    sz = len < sz ? len : sz;
    memcpy(buf->buf + buf->ed_idx, in_b, sz);
    buf->ed_idx += sz;
    return sz;
}

int32_t buf_read_data(struct buf_data* buf, int8_t* out_b, int32_t len)
{
    int32_t space = buf_space_data(buf);
    if (space == 0)
        return 0;
    if (space < 0)
        return B_ERROR;
    int32_t sz = space < len ? space : len;
    memcpy(out_b, buf->buf + buf->st_idx, sz);
    buf->st_idx += sz;
    return sz;
}

void buf_destroy_data(struct buf_data* buf)
{
    free(buf);
}

