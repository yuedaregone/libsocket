#include "skt.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#define CONFIG_IMPLEMENT
#include "config.h"


void recv_data(skt_d skt, struct buf_circle* buf)
{

    char buff[512] = {0};
    buf_read_circle(buf, (int8_t*)buff, 512);
    printf("client_recv - %d :", skt);
    printf("%s\n", (char*)buff);
}

int main()
{
    const char* ip = "192.168.31.132";
    int port = 38086;
    struct config* cfg = config_create();
    if (0 == config_load_local_data(cfg, "/Users/yuegangyang/ts.cfg"))
    {
        ip = config_get_str_value(cfg, "IP");
        port = config_get_int_value(cfg, "Port");
    }
    
    struct skt_client* client = skt_client_create();
	skt_client_open(client, NULL, 0);
    skt_client_connect(client, ip, port);
	client->recv_cb = recv_data;
    while (1)
    {
		if (client->sta == skt_success)
		{
			//static char buff[512];
			//scanf("%s", buff);    

			const char* buff = "HelloWorld!";
            //printf("%s\n", buff);
			skt_client_send_to(client, (int8_t*)buff, (int32_t)strlen(buff)+1);
		}            

        skt_client_update_state(client);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif        
    }
    return 0;
} 
