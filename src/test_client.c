#include "skt.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void recv_data(skt_d skt, struct buf_circle* buf)
{

    char buff[512] = {0};
    buf_read_circle(buf, (int8_t*)buff, 512);
    printf("client_recv:");
    printf("%s\n", (char*)buff);
}

int main()
{
    struct skt_client* client = skt_client_create();
	skt_client_open(client, NULL, 0);
    skt_client_connect(client, "127.0.0.1", 8086);
	client->recv_cb = recv_data;
    while (1)
    {
		if (client->sta == skt_success)
		{
			static char buff[512];
			scanf("%s", buff);

			skt_client_send_to(client, buff, strlen(buff));
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
