#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "skt.h"
#include "msgs.h"
#include "buffer.h"
#include "array.h"
#define CONFIG_IMPLEMENT
#include "config.h"

static struct skt_server* s_sock = NULL;
static struct config* s_cfg = NULL;

struct http_request
{
	int finished;
	char* method;
	char* protocol;
	char* head;
	int8_t* data;
};

struct http_request* http_request_create()
{
	struct http_request* req = (struct http_request*)malloc(sizeof(struct http_request));
	req->method = NULL;
	req->protocol = NULL;
	req->head = NULL;
	req->data = NULL;
	return req;
}

void http_request_destroy(struct http_request* req)
{
	array_destroy(req->head);
	free(req);
}

int http_request_load_content(struct http_request* req, struct buf_circle* buf)
{

}


void net_data_recv(int32_t skt, struct buf_circle* buf)
{
	struct http_request* req = http_request_create();
	http_request_load_content(req, buf);

}

int main()
{
	const char* ip = "10.10.1.92";
	int port = 38086;

	s_cfg = config_create();
	config_load_local_data(s_cfg, "/Users/yuegangyang/ts.cfg");

	if (0 == config_load_local_data(s_cfg, "/Users/yuegangyang/ts.cfg"))
	{
		ip = config_get_str_value(s_cfg, "IP");
		port = config_get_int_value(s_cfg, "Port");
	}	
	s_sock = skt_server_create();
	s_sock->recv_cb = net_data_recv;
	skt_server_open(s_sock, ip, (uint16_t)port);

	while (1)
	{
		skt_server_update_state(s_sock);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif        
	}
	return 0;
}
