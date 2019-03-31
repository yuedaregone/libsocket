#ifndef __HTTP_H__
#define __HTTP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HTTP_HEAD_LINE_END "\r\n"
#define HTTP_REQUEST_HEAD_SIZE 4096
#define HTTP_REQUEST_DATA_BUFF 4096
#define HTTP_OK 0
#define HTTP_ERR -1

enum _recv_data
{
	sta_no_recv,
	sta_data_reading_head,
	sta_data_reading_data,
	sta_data_finished,
};

void http_server_init(void);

struct buf_data;

//http utils
int http_utils_get_tag(struct buf_data* data, char* buf, int len, char* st_str, char* ed_str);

//http request
struct http_request
{
	int sta;	
	struct buf_data* head;
	struct buf_data* data;
};

struct buf_circle;

struct http_request* http_request_create(void);
void http_request_destroy(struct http_request* req);
void http_request_reset(struct http_request* req);
void http_request_init_with_url(struct http_request* req, const char* url, const char* post_data);
void http_request_add_head_info(struct http_request* req, const char* head_info);
int http_request_load_data(struct http_request* req, struct buf_circle* buf);
int http_request_get_hostname(struct http_request* req, char* buf, int len);
int http_request_get_path(struct http_request* req, char* buf, int len);

//http respond
struct http_respond
{
    int sta;
    struct buf_data* head;
    struct buf_data* data;
};

struct http_respond* http_respond_create(void);
void http_respond_destroy(struct http_respond* resp);
void http_respond_reset(struct http_respond* resp);
int http_respond_load_data(struct http_respond* resp, struct buf_circle* buf);

enum _clt_sta
{
	sta_clt_none,
	sta_clt_req_ready,
	sta_clt_req_send,
	sta_clt_resp_recving,
	sta_clt_resp_finish,
	sta_error,
};

struct skt_client;
typedef void (*http_recv_finish)(struct http_respond* resp);

struct http_client
{
	int sta;
    http_recv_finish cb;
	struct skt_client* sock;
	struct http_request* req;
	struct http_respond* resp;
};

struct http_client* http_client_get(const char* url, const char* ext);
void http_client_send(struct http_client* clt);
void http_client_update(void);

#endif //__HTTP_H__
