#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#endif
#include "http.h"
#include "skt.h"
#include "msgs.h"
#include "buffer.h"
#include "array.h"
#include "pool.h"
#define CONFIG_IMPLEMENT
#include "config.h"

static struct pool* s_client_pool = NULL;

static struct http_client* http_client_get_active_client(int32_t skt)
{
    struct array* arr = s_client_pool->act;
    for (int i = 0; i < arr->count; ++i)
    {
        struct http_client* clt = *(struct http_client**)array_index(arr, i);
        if (clt->sock->skt == (skt_d)skt)
            return clt;
    }
    return NULL;
}

static void http_client_data_recv(int32_t skt, struct buf_circle* buf)
{
    struct http_client* clt = http_client_get_active_client(skt);
    if (clt == NULL || clt->sta == sta_clt_resp_finish)
        return;
    http_respond_load_data(clt->resp, buf);
    if (clt->resp->sta == sta_data_finished)
    {
        clt->sta = sta_clt_resp_finish;
        skt_client_close(clt->sock);
        
        if (clt->cb != NULL)
        {
            (*clt->cb)(clt->resp);
        }
    }
    else
        clt->sta = sta_clt_resp_recving;
}

static struct http_client* http_client_create()
{
	struct http_client* clt = (struct http_client*)malloc(sizeof(struct http_client));
	clt->sta = sta_clt_none;
    clt->sock = skt_client_create();
	clt->sock->recv_cb = http_client_data_recv;	
	clt->req = http_request_create();
	clt->resp = http_respond_create();
	return clt;
}

static void http_client_destroy(struct http_client* clt)
{
	http_request_destroy(clt->req);
	http_respond_destroy(clt->resp);
	skt_client_destroy(clt->sock);
	free(clt);
}

static void http_client_reset(struct http_client* clt)
{
	clt->sta = sta_clt_none;
	http_request_reset(clt->req);
	http_respond_reset(clt->resp);
}

static void http_client_init(void)
{
	if (s_client_pool == NULL)
		s_client_pool = pool_create((pool_create_item_fun)http_client_create, (pool_destroy_item_fun)http_client_destroy);
}

static void http_client_send_request(struct http_client* clt)
{
	int32_t head_sz = 0;
	do
	{
		head_sz = buf_size_data(clt->req->head);
		struct buf_data* bh = clt->req->head;
		int32_t send_sz = skt_client_send_to(clt->sock, bh->buf + bh->st_idx, head_sz);
		bh->st_idx += send_sz;
	} while (head_sz > 0);

	int32_t data_sz = 0;
	do 
	{
		data_sz = buf_size_data(clt->req->data);
		struct buf_data* bd = clt->req->data;
		int32_t send_sz = skt_client_send_to(clt->sock, bd->buf + bd->st_idx, data_sz);
		bd->st_idx += send_sz;
	} while (data_sz > 0);
	clt->sta = sta_clt_req_send;
}

void http_client_update(void)
{
	if (s_client_pool == NULL)
		return;

	for (int i = 0; i < s_client_pool->act->count; ++i)
	{
		struct http_client* clt = *(struct http_client**)array_index(s_client_pool->act, i);
		if (clt != NULL)
		{
			if (clt->sta == sta_clt_req_ready)
				http_client_send_request(clt);
			if (clt->sta == sta_clt_req_send || clt->sta == sta_clt_resp_recving)
				skt_client_update_state(clt->sock);
		}
	}	
	
	for (int i = s_client_pool->act->count - 1; i >= 0; --i)
	{
		struct http_client* clt = *(struct http_client**)array_index(s_client_pool->act, i);
		if (clt != NULL && clt->sta == sta_clt_resp_finish)
		{
			pool_return(s_client_pool, (void*)&clt);
		}
	}
}

static int http_client_get_ip(const char* host, char buff[16])
{
	struct hostent *h = gethostbyname(host);
	if (h == NULL)
	{
		return HTTP_ERR;
	}
	char* ip = inet_ntoa(*((struct in_addr *)h->h_addr));
	strcpy(buff, ip);
	return HTTP_OK;
}

struct http_client* http_client_get(const char* url, const char* ext)
{
    http_client_init();
	struct http_client* clt = *(struct http_client**)pool_request(s_client_pool);
	http_client_reset(clt);
	http_request_init_with_url(clt->req, url, NULL);
	if (ext != NULL)
	{
		http_request_add_head_info(clt->req, ext);
	}
	clt->sta = sta_clt_req_ready;
    return clt;
}

void http_client_send(struct http_client* clt)
{
    skt_client_open(clt->sock, NULL, 0);
    
    char buf[256] = { 0 }; char ip[16] = { 0 };
    http_request_get_hostname(clt->req, buf, 256);
    http_client_get_ip(buf, ip);
    
    skt_client_connect(clt->sock, ip, 80);
}

/*
static void http_client_over(struct http_respond* resp)
{
    printf((const char*)resp->data->buf);
}

//static struct skt_client* s_sock = NULL;
int main()
{
    struct http_client* clt = http_client_get("http://daregone.pythonanywhere.com", NULL);
    clt->cb = http_client_over;
    http_client_send(clt);
    while (1) {
        http_client_update();
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    } 
	return 0;
}
*/
