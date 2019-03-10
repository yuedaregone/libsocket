#include "http.h"
#include "pool.h"
#include "buffer.h"

#define GET_METHOD "GET"
#define POST_METHOD "POST"
#define GET_LENGTH 3
#define POST_LENGTH 4
#define UNUSED(p) p; 

static struct pool* request_pool = NULL;


static struct http_request* http_request_create()
{
	struct http_request* req = (struct http_request*)malloc(sizeof(struct http_request));
	req->head = buf_create_data(HTTP_REQUEST_DATA_BUFF);
	req->data = buf_create_data(HTTP_REQUEST_DATA_BUFF);
	http_request_reset(req);
	return req;
}

static void http_request_destroy(struct http_request* req)
{
	buf_destroy_data(req->head);
    buf_destroy_data(req->data);
	free(req);
}

void http_server_init()
{
    request_pool = pool_create(http_request_create, http_request_destroy);
}

static void http_request_reset(struct http_request* req)
{
	req->skt_id = 0;
	req->sta = sta_none;
	req->method = NULL;
	req->protocol = NULL;
}

struct http_request* http_request_get_request(int32_t skt)
{
	void* data = NULL;
	struct http_request* req = NULL;

	struct array* arr = request_pool->act;
	for (int i = 0; i < arr->count; ++i)
	{
		data = array_index(arr, i);
		req = *(struct http_request**)data;
		if (req->skt_id == skt)
			return req;
	}
	data = pool_request(request_pool);
	req = *(struct http_request**)data;
	http_request_reset(req);
	req->skt_id = skt;
	return req;
}

void http_request_return_request(struct http_request* req)
{
    UNUSED(req)
}

static void http_request_response_error(struct http_request* req)
{
	UNUSED(req)
}



static int http_request_parse_head(struct http_request* req)
{
	if (strncmp((char*)req->head->buf, GET_METHOD, GET_LENGTH) == 0)
	{
		req->method = GET_METHOD;
		req->head->st_idx += GET_LENGTH;
	}
	else if (strncmp((char*)req->head->buf, POST_METHOD, POST_LENGTH) == 0)
	{
		req->method = POST_METHOD;
		req->head->st_idx += POST_LENGTH;
	}
	else
	{
		http_request_response_error(req);
		return -1;
	}
	req->head->st_idx += 1;
	req->req_path = (char*)(req->head->buf + req->head->st_idx);

	char ch = 0;
	while ((ch = (char)*(req->head->buf + ++req->head->st_idx)) != ' ')
	{
	}
	*(req->head->buf + req->head->st_idx++) = 0;

	req->protocol = (char*)req->head->buf + req->head->st_idx;
	req->head->st_idx = buf_indexof_data(req->head, 0, (int8_t*)("\r\n"), 2);
	for (int i = 0; i < 2; ++i)
		*(req->head->buf + req->head->st_idx++) = 0;


	
	req->sta = sta_finished;
	return 0;
}

static int http_request_read_buf_data(struct http_request* req, int8_t* buf, int32_t len)
{
	int32_t sp = buf_space_data(req->data);
	if (sp < len)
	{
		printf("request data is too long!\n");
		return -1;
	}
	buf_write_data(req->data, buf, len);
	return 0;
}

static int http_request_read_data(struct http_request* req, struct buf_circle* buf)
{
	int8_t buff[1024];
	while (buf->data_sz > 0)
	{
		int32_t len = buf_read_circle(buf, buff, 1024);
		int ret = http_request_read_buf_data(req, buff, len);
		if (ret != 0)
		{
			return ret;
		}
	}
	return 0;
}

static int http_request_read_head(struct http_request* req, struct buf_circle* buf)
{
	req->sta = sta_reading_head;
	if (req->head->cap - req->head->ed_idx == 0)
	{
		printf("buffer is full!\n");
		return -1;
	}
	char tag[] = { "\r\n\r\n" };
	int32_t tag_len = (int32_t)strlen(tag);

	int32_t st_idx = req->head->ed_idx;
	int32_t sz = buf_read_circle(buf, req->head->buf + st_idx, req->head->cap - st_idx);
	req->head->ed_idx += sz;

	int32_t rd_idx = st_idx < tag_len ? st_idx : st_idx - tag_len + 1;
	int32_t idx = buf_indexof_data(req->head, rd_idx, (int8_t*)tag, tag_len);
	if (idx != -1)
	{
		req->sta = sta_reading_data;		
		if (idx + tag_len < req->head->ed_idx)
		{
			http_request_read_buf_data(req, req->head->buf + idx + tag_len, req->head->ed_idx - idx - tag_len);
			req->head->ed_idx = idx + tag_len;
		}	
		http_request_parse_head(req);		
	}
	return 0;
}

int http_request_load_data(struct http_request* req, struct buf_circle* buf)
{
	switch (req->sta)
	{
	case sta_none:
	case sta_reading_head:
	{		
		http_request_read_head(req, buf);
		break;
	}
	case sta_reading_data:
	{
		http_request_read_data(req, buf);
		break;
	}
	default:
		break;
	}
	return 0;
}
