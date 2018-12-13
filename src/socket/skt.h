#ifndef __SKT_H__
#define __SKT_H__
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

#ifdef _WIN32
typedef SOCKET	skt_d;
#else
typedef int32_t skt_d;
#endif

#define IP_MAX_LEN 16
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#define SKT_OK 0
#define SKT_ERR -1

#ifdef _WIN32
#define GET_ERROR_CODE WSAGetLastError()
#else
#define GET_ERROR_CODE errno
#endif

#define BUFFER_SOCKET_DATA_SIZE 0x00040000
#define BUFFER_SOCKET_ONE_FRAME 0x00002000

struct buf_circle;
struct buf_data;
struct array;

typedef void (*skt_recv_data)(skt_d skt, struct buf_circle* buf);

struct skt_client
{
	skt_d skt;
	int err_no;
    int16_t sta;
	char conn_ip[IP_MAX_LEN];
    uint16_t conn_port;
	skt_recv_data recv_cb;
	struct buf_circle* send_buf;
	struct buf_circle* recv_buf;
	struct buf_data* cur_send;
	struct buf_data* cur_recv;
};

struct skt_server
{
	skt_d skt;
	int err_no;
    int16_t sta;
	char conn_ip[IP_MAX_LEN];
    uint16_t conn_port;
	skt_recv_data recv_cb;
	struct array* skt_ios;
};

struct skt_io
{
	skt_d skt;
	int err_no;
	struct skt_server* server;
	struct buf_circle* send_buf;
	struct buf_circle* recv_buf;
	struct buf_data* cur_send;
	struct buf_data* cur_recv;
};

enum
{
	skt_noactivate = -1,
	skt_success,
	skt_created,
	skt_conneting,	
	skt_disconnect,
	skt_unknown,
};

enum {
	WAIT_FOR_READ = 1,
	WAIT_FOR_WRITE = 2
};

#pragma pack(1)

typedef struct  
{
	uint32_t tag;
	uint16_t length;
}msg_head;

#pragma pack()

//socket_api
int32_t skt_set_non_block(int32_t fd);
void skt_delay(uint32_t usec);
int32_t skt_select_fd(int32_t fd, double maxtime, int32_t wait_for);

//socket_log
void skt_error(const char* str, ...);
void skt_warning(const char* str, ...);
void skt_log(const char* str, ...);

//socket_io
struct skt_io* skt_create_io(skt_d skt, struct skt_server* server);
void skt_destroy_io(struct skt_io* io);
void skt_update_io(struct skt_io* io);
int32_t skt_send_io(struct skt_io* io, int8_t* buf, int32_t len);

//socket_client
struct skt_client* skt_create_client();
void skt_destroy_client(struct skt_client* skt);
int skt_open_client(struct skt_client* skt, const char* ip, uint16_t port);
int skt_connect_server(struct skt_client* skt, const char* ip, uint16_t port);
int32_t skt_send_to_server(struct skt_client* skt, int8_t* buf, int32_t len);
void skt_update_state_client(struct skt_client* skt);
void skt_close_client(struct skt_client* skt);

//socket_server
struct skt_server* skt_create_server();
void skt_destroy_server(struct skt_server* skt);
int skt_open_server(struct skt_server* skt, const char* ip, uint16_t port);
//int32_t skt_send_to_client(struct skt_server* skt, int8_t* buf, int32_t len);
void skt_update_state_server(struct skt_server* skt);
void skt_close_server(struct skt_server* skt);

#endif