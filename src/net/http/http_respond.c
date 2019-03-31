#include "http.h"
#include "buffer.h"
#include "array.h"
#include "pool.h"

#define HTTP_RESPOND_MSG "HTTP/1.1 %d OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n"
#define HTTP_CONTENT_SIZE "Content-Length: "

struct http_respond* http_respond_create(void)
{
    struct http_respond* resp = (struct http_respond*)malloc(sizeof(struct http_respond));
    resp->head = buf_create_data(HTTP_REQUEST_DATA_BUFF);
    resp->data = buf_create_data(HTTP_REQUEST_DATA_BUFF);
	http_respond_reset(resp);
    return resp;
}

void http_respond_destroy(struct http_respond* resp)
{
    buf_destroy_data(resp->head);
    buf_destroy_data(resp->data);
    free(resp);
}

void http_respond_reset(struct http_respond* resp)
{
	resp->sta = sta_no_recv;
	buf_reinit_data(resp->head);
	buf_reinit_data(resp->data);
}

static int http_execute(const char* cmd, struct buf_data* data)
{
    buf_write_data(data, (int8_t*)"Hello World!", (int32_t)strlen("Hello World!"));
    return 200;
}

struct http_respond* s_resp = NULL;

static int http_execute_request(struct http_request* req)
{
    char path[256] = {0};
    http_request_get_path(req, path, 256);
    
    int ret = http_execute(path, s_resp->data);
    s_resp->sta = ret;

    char buff[HTTP_REQUEST_DATA_BUFF] = {0};
	sprintf(buff, HTTP_RESPOND_MSG, ret, "text/html", buf_size_data(s_resp->data));
	buf_write_data(s_resp->head, (int8_t*)buff, (int32_t)strlen(buff));
	
	s_resp->sta = sta_data_finished;
    return HTTP_OK;
}

static int http_respond_read_content_size(struct http_respond* resp, char* buf, int len)
{
    return http_utils_get_tag(resp->head, buf, len, HTTP_CONTENT_SIZE, HTTP_HEAD_LINE_END);
}

static void http_respond_check_recv_finish(struct http_respond* resp)
{
    if (resp->sta != sta_data_reading_data)
        return;

    char buf[32] = {0};
    int ret = http_respond_read_content_size(resp, buf, 32);
    if (ret == -1)
    {
        const char* end_tag = "\r\n\r\n";
        const char* end_str = (const char*)(resp->data->buf + resp->data->ed_idx - (int)strlen(end_tag));
        if (strncmp(end_tag, end_str, strlen(end_tag)) == 0)
        {
            resp->sta = sta_data_finished;
        }
    }
    else
    {
        int size = atoi(buf);
        if (buf_size_data(resp->data) >= size)
        {
            resp->sta = sta_data_finished;
        }
    }
}

static int http_respond_read_buf_data(struct http_respond* resp, int8_t* buf, int32_t len)
{
    int32_t sp = buf_space_data(resp->data);
    if (sp < len)
    {
        printf("respond data is too long!\n");
        return -1;
    }
    buf_write_data(resp->data, buf, len);
    http_respond_check_recv_finish(resp);
    return 0;
}

static int http_respond_read_data(struct http_respond* resp, struct buf_circle* buf)
{
    int8_t buff[1024];
    while (buf->data_sz > 0)
    {
        int32_t len = buf_read_circle(buf, buff, 1024);
        int ret = http_respond_read_buf_data(resp, buff, len);
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}


static int http_respond_read_head(struct http_respond* resp, struct buf_circle* buf)
{
    resp->sta = sta_data_reading_head;
    if (resp->head->cap - resp->head->ed_idx == 0)
    {
        printf("buffer is full!\n");
        return -1;
    }
    char tag[] = { "\r\n\r\n" };
    int32_t tag_len = (int32_t)strlen(tag);
    
    int32_t st_idx = resp->head->ed_idx;
    int32_t sz = buf_read_circle(buf, resp->head->buf + st_idx, resp->head->cap - st_idx);
    resp->head->ed_idx += sz;
    
    int32_t rd_idx = st_idx < tag_len ? st_idx : st_idx - tag_len + 1;
    int32_t idx = buf_indexof_data(resp->head, rd_idx, (int8_t*)tag, tag_len);
    if (idx != -1)
    {
        resp->sta = sta_data_reading_data;
        if (idx + tag_len < resp->head->ed_idx)
        {
            http_respond_read_buf_data(resp, resp->head->buf + idx + tag_len, resp->head->ed_idx - idx - tag_len);
            resp->head->ed_idx = idx + tag_len;
        }
    }    
    return 0;
}

int http_respond_load_data(struct http_respond* resp, struct buf_circle* buf)
{
    switch (resp->sta)
    {
        case sta_no_recv:
        case sta_data_reading_head:
        {
            return http_respond_read_head(resp, buf);
        }
        case sta_data_reading_data:
        {
            return http_respond_read_data(resp, buf);
        }
        default:
            break;
    }
    return 0;
}
