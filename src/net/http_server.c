#include "skt.h"
#include "msgs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#define CONFIG_IMPLEMENT
#include "config.h"

static struct skt_server* s_sock = NULL;
static struct config* s_cfg = NULL;

void net_data_recv(int32_t skt, struct buf_circle* buf)
{

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
	s_sock = skt_server_create();
	s_sock->recv_cb = net_data_recv;
	skt_server_open(s_sock, ip, port);

	while (1)
	{
		skt_server_update_state(s_sock);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif        
	}
	net_destroy();
	return 0;
}
