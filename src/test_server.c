#include "skt.h"
#include <stdlib.h>
#include "buffer.h"
#include <string.h>

void recv_data(struct skt_io* io)
{
    char buff[512] = {0};
    buf_read_circle(io->recv_buf, (int8_t*)buff, 512);
    printf("server_recv:");
    printf("%s\n", buff);
}

int main()
{
    printf("test1");
    struct skt_conn* conn = skt_create();
    if (conn == NULL)
    {
        printf("Test");
    }
    skt_open_as_servers(conn, "127.0.0.1", 8086);
    conn->recv_cb = recv_data;
    printf("test");
    while (1)
    {
        //skt_send(conn, (int8_t*)"HelloWorld!", strlen("HelloWorld!"));
        skt_update_state(conn);

        usleep(100000);
    }
    return 0;
}
