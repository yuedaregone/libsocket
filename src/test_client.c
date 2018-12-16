#include "skt.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void recv_data(skt_d skt, struct buf_circle* buf)
{

    char buff[512] = {0};
    buf_read_circle(buf, (int8_t*)buff, 512);
    printf("client_recv - %d :", skt);
    printf("%s\n", (char*)buff);
}

int main()
{
    struct skt_client* client = skt_client_create();
	skt_client_open(client, NULL, 0);
    skt_client_connect(client, "192.168.31.51", 8086);
	client->recv_cb = recv_data;
    while (1)
    {
		if (client->sta == skt_success)
		{
			static char buff[512];
			scanf("%s", buff);
            
            printf("%s\n", buff);
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
