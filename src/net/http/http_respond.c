#include "http.h"
#include "buffer.h"
#include "array.h"
#include "pool.h"

#define HTTP_RESPOND_MSG "HTTP/1.1 %d OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n"

static struct pool* respond_pool = NULL;

static void http_respond_reset(struct http_respond* resp)
{
    resp->skt_id = 0;
    resp->sta = sta_none;
}

static struct http_respond* http_respond_create()
{
    struct http_respond* resp = (struct http_respond*)malloc(sizeof(struct http_respond));
    resp->head = buf_create_data(HTTP_REQUEST_DATA_BUFF);
    resp->data = buf_create_data(HTTP_REQUEST_DATA_BUFF);
    http_respond_reset(resp);
    return resp;
}

static void http_respond_destroy(struct http_respond* resp)
{
    buf_destroy_data(resp->head);
    buf_destroy_data(resp->data);
    free(resp);
}

void http_respond_init()
{
    respond_pool = pool_create(http_respond_create, http_respond_destroy);
}

static int http_execute(const char* cmd, struct buf_data* data)
{
    buf_write_data(data, "Hello World!", strlen("Hello World!"));
    return 200;
}

struct http_respond* s_resp = NULL;

static int http_execute_request(struct http_request* req)
{	
    int ret = http_execute(req->req_path, s_resp->data);
    s_resp->skt_id = req->skt_id;
    s_resp->sta = ret;

    char buff[HTTP_REQUEST_DATA_BUFF] = {0};
	sprintf(buff, HTTP_RESPOND_MSG, ret, "text/html", buf_size_data(s_resp->data));
	buf_write_data(s_resp->head, (int8_t*)buff, strlen(buff));
	
	s_resp->sta = sta_finished;
}

struct http_respond* http_respond_get_respond(int32_t skt)
{
	void* data = NULL;
	struct http_respond* resp = NULL;

	struct array* arr = respond_pool->act;
	for (int i = 0; i < arr->count; ++i)
	{
		data = array_index(arr, i);
		resp = *(struct http_respond**)data;
		if (resp->skt_id == skt)
			return resp;
	}
	data = pool_request(respond_pool);
	resp = *(struct http_respond**)data;
	http_respond_reset(resp);
	resp->skt_id = skt;
	return resp;
}

void http_respond_return_respond(struct http_respond* resp)
{
    
}

