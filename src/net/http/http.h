#ifndef __HTTP_H__
#define __HTTP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HTTP_REQUEST_HEAD_SIZE 4096
#define HTTP_REQUEST_DATA_BUFF 4096
#define HTTP_OK 0
#define HTTP_ERR -1

enum
{
	sta_reading_head,
	sta_reading_data,
	sta_finished,
};

void http_server_init();

//http request
struct http_request
{
	int sta;
	char* method;
	char* req_path;
	char* protocol;
	struct buf_data* head;
	struct buf_data* data;
};

struct buf_circle;

struct http_request* http_request_create();
void http_request_destroy(struct http_request* req);
void http_request_init_with_url(struct http_request* req, const char* url, const char* post_data);
void http_request_add_head_info(struct http_request* req, const char* head_info);
int http_request_load_data(struct http_request* req, struct buf_circle* buf);

//http respond
struct http_respond
{
    int sta;
    struct buf_data* head;
    struct buf_data* data;
};

struct http_respond* http_respond_create();
void http_respond_destroy(struct http_respond* resp);

enum _clt_sta
{
	sta_none,
	sta_req_ready,
	sta_req_send,
	sta_resp_recving,
	sta_resp_finish,
	sta_error,
};

struct skt_client;

struct http_client
{
	int sta;
	struct skt_client* sock;
	struct http_request* req;
	struct http_respond* resp;
};
// struct http_client* http_client_create();
// void http_client_destroy(struct http_client* clt);


#endif //__HTTP_H__
