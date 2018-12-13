#include "skt.h"
#include <stdlib.h>
#include "buffer.h"
#include <string.h>

void recv_data(skt_d skt, struct buf_circle* buf)
{
	char buff[512] = { 0 };
	buf_read_circle(buf, buff, 512);
	printf("server_recv - %d :", skt);
	printf(buff);
	printf("\n");
}

int main()
{
	struct skt_server* server = skt_server_create();
	skt_server_open(server, "127.0.0.1", 8086);
	server->recv_cb = recv_data;
	while (1)
	{
		//skt_send_io(server, "HelloWorld!", strlen("HelloWorld!"));
		skt_server_update_state(server);
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif        
	}
	return 0;
}