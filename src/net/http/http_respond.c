#include "http.h"
#include "buffer.h"
#include "array.h"
#include "pool.h"
#include "utils.h"

#define HTTP_RESPOND_MSG "HTTP/1.1 %d OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n"
#define HTTP_CONTENT_SIZE "Content-Length: "
#define HTTP_TRANSFER_ENCODING "Transfer-Encoding: "
#define HTTP_CHUNKED "chunked"

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
	resp->req = NULL;
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

static int http_respond_get_content_size(struct http_respond* resp)
{
	char buf[32] = { 0 };
	if (http_utils_get_tag(resp->head, buf, 32, HTTP_CONTENT_SIZE, HTTP_HEAD_LINE_END) != HTTP_ERR)
	{
		return atoi(buf);
	}
	return HTTP_ERR;
}

int http_respond_get_is_chunked(struct http_respond* resp)
{
	char buf[32] = { 0 };
	if (http_utils_get_tag(resp->head, buf, 32, HTTP_TRANSFER_ENCODING, HTTP_HEAD_LINE_END) != HTTP_ERR)
	{
		return strncmp(buf, HTTP_CHUNKED, strlen(HTTP_CHUNKED)) == 0 ? 1 : 0;
	}
	return 0;
}


static int http_respond_read_buf_data(struct http_respond* resp, int8_t* buf, int32_t len)
{
    int32_t sp = buf_space_data(resp->data);
    if (sp < len)
    {
		buf_relloc_data(resp->data);
    }
	if (http_respond_get_is_chunked(resp))
	{
		buf_write_data(resp->data, buf, len);
		static int datalen = -1;
		if (datalen == -1)
		{
			int32_t idx = buf_indexof_data(resp->data, 0, (int8_t*)HTTP_HEAD_LINE_END, (int32_t)strlen(HTTP_HEAD_LINE_END));
			char buffer[32] = { 0 };
			strncpy(buffer, (const char*)resp->data, idx);
			datalen = atoi(buffer);
		}

		const char* end_tag = "0\r\n\r\n";
		const char* end_str = (const char*)(resp->data->buf + resp->data->ed_idx - (int)strlen(end_tag));
		if (strncmp(end_tag, end_str, strlen(end_tag)) == 0)
		{
			resp->sta = sta_data_finished;
		}
	}
	else
	{
		buf_write_data(resp->data, buf, len);
		int size = http_respond_get_content_size(resp);
		if (buf_size_data(resp->data) >= size)
		{
			resp->sta = sta_data_finished;
		}
	}
    return 0;
}

static int http_respond_read_data(struct http_respond* resp, struct buf_circle* buf)
{
	int chunck = http_respond_get_is_chunked(resp);

    int8_t buffer[1024];
    while (buf->data_sz > 0)
    {
        int32_t rd_sz = buf_peek_circle(buf, buffer, 1024);
		if (chunck)
		{
			if (resp->data->st_idx <= resp->data->ed_idx)
			{
				int idx = utils_indexof_data(buffer, rd_sz, HTTP_HEAD_LINE_END, strlen(HTTP_HEAD_LINE_END));
				if (idx == -1)
				{
					return 0;
				}
				if (idx == 0)
				{
					buf_offset_circle(buf, strlen(HTTP_HEAD_LINE_END));
					continue;
				}
				char num_buf[64] = { 0 };
				strncpy(num_buf, buffer, idx);

				int ck_sz = 0;
				if (utils_try_atoi_hex(num_buf, &ck_sz) != 0)
				{
					printf("error - get chunck error: %s\n", num_buf);
					return 0;
				}
				if (ck_sz == 0)
				{
					resp->sta = sta_data_finished;
					resp->data->st_idx = 0;
				}
				resp->data->st_idx += ck_sz;
				buf_offset_circle(buf, idx + strlen(HTTP_HEAD_LINE_END));
			}
			else
			{
				if (buf_space_data(resp->data) < rd_sz)
				{
					buf_relloc_data(resp->data);
				}
				int len = resp->data->st_idx - resp->data->ed_idx;
				int wt_sz = rd_sz <= len ? rd_sz : len;
				buf_write_data(resp->data, buffer, wt_sz);
				buf_offset_circle(buf, wt_sz);
			}
		}
		else
		{
			if (buf_space_data(resp->data) < rd_sz)
			{
				buf_relloc_data(resp->data);
			}
			buf_write_data(resp->data, buffer, rd_sz);
			buf_offset_circle(buf, rd_sz);

			int size = http_respond_get_content_size(resp);
			if (buf_size_data(resp->data) >= size)
			{
				resp->sta = sta_data_finished;
			}
		}
    }
    return 0;
}


static int http_respond_read_head(struct http_respond* resp, struct buf_circle* buf)
{
    resp->sta = sta_data_reading_head;

    char tag[] = { "\r\n\r\n" };
    int32_t tag_len = (int32_t)strlen(tag);

	char buffer[1024] = { 0 };

	while (buf->data_sz > 0)
	{
		int32_t rd_sz = buf_peek_circle(buf, (int8_t*)buffer, 1024);
		int idx = utils_indexof_data(buffer, rd_sz, tag, tag_len);
		if (idx == -1)
		{
			if (buf_space_data(resp->head) < rd_sz)
			{
				buf_relloc_data(resp->head);
			}
			buf_write_data(resp->head, buffer, rd_sz);
			buf_offset_circle(buf, rd_sz);
		}
		else
		{
			if (buf_space_data(resp->head) < idx + tag_len)
			{
				buf_relloc_data(resp->head);
			}
			buf_write_data(resp->head, buffer, idx + tag_len);
			buf_offset_circle(buf, idx + tag_len);
			resp->sta = sta_data_reading_data;
			break;			
		}
	}	
	if (resp->sta == sta_data_reading_data && buf->data_sz > 0)
	{
		http_respond_read_data(resp, buf);
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
