#include "skt.h"
#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void recv_data(skt_d skt, struct buf_circle* buf)
{

    char buff[512] = {0};
    buf_read_circle(buf, buff, 512);
    printf("client_recv - %d:", skt);
    printf(buff);
    printf("\n");
}

int main()
{
    struct skt_client* client = skt_create_client();
	skt_open_client(client, NULL, 0);
    skt_connect_server(client, "127.0.0.1", 8086);
	client->recv_cb = recv_data;
    while (1)
    {
        skt_send_to_server(client, "HelloWorld!", strlen("HelloWorld!"));

        skt_update_state_client(client);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif        
    }
    return 0;
} 
