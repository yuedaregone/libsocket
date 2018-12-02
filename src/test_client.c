#include "skt.h"
#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void recv_data(struct skt_io* io)
{
    char buff[512] = {0};
    buf_read_circle(io->recv_buf, buff, 512);
    printf("client_recv:");
    printf(buff);
    printf("\n");
}

int main()
{
    struct skt_conn* conn = skt_create();
    skt_open_as_client(conn, NULL);
    skt_connect(conn, "127.0.0.1", 8086);
    conn->recv_cb = recv_data;
    while (1)
    {
        skt_send(conn, "HelloWorld!", strlen("HelloWorld!"));

        skt_update_state(conn);

        usleep(100000);
    }
    return 0;
} 
