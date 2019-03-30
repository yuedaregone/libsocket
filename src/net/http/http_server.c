#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "http.h"
#include "skt.h"
#include "msgs.h"
#include "buffer.h"
#include "array.h"
#include "pool.h"
#define CONFIG_IMPLEMENT
#include "config.h"

static struct skt_server* s_sock = NULL;
static struct config* s_cfg = NULL;

void http_server_init()
{
    http_request_init();
	http_respond_init();
}

static void http_server_data_recv(int32_t skt, struct buf_circle* buf)
{
	struct http_request* req = http_request_get_request(skt);
	http_request_load_data(req, buf);
    if (req->sta == sta_data_finished)
    {
        http_execute_request(req);
    }
}

int main()
{
	const char* ip = "192.168.31.132";
	int port = 38086;

	s_cfg = config_create();
	config_load_local_data(s_cfg, "/Users/yuegangyang/ts.cfg");

	if (0 == config_load_local_data(s_cfg, "/Users/yuegangyang/ts.cfg"))
	{
		ip = config_get_str_value(s_cfg, "IP");
		port = config_get_int_value(s_cfg, "Port");
	}	
	
	http_server_init();
	s_sock = skt_server_create();
	s_sock->recv_cb = http_server_data_recv;
	skt_server_open(s_sock, ip, (uint16_t)port);

	while (1)
	{
		skt_server_update_state(s_sock);

		//if (s_resp != NULL && s_resp->sta == sta_finished)
		{
		//	skt_server_send_to(s_sock, s_resp->skt_id, s_resp->head->buf, buf_size_data(s_resp->head));
		//	skt_server_send_to(s_sock, s_resp->skt_id, s_resp->data->buf, buf_size_data(s_resp->data));
		}
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif        
	}
	return 0;
}
