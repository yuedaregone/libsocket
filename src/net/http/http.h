#ifndef __HTTP_H__
#define __HTTP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_REQUEST_HEAD_SIZE 4096
#define HTTP_REQUEST_DATA_BUFF 4096

enum
{
	sta_none,
	sta_reading_head,
	sta_reading_data,
	sta_finished,
};

void http_server_init();

//http request
struct http_request
{
	int32_t skt_id;
	int sta;
	char* method;
	char* req_path;
	char* protocol;
	struct buf_data* head;
	struct buf_data* data;
};

void http_request_init();
struct http_request* http_request_get_request(int32_t skt);
void http_request_return_request(struct http_request* req);
int http_request_load_data(struct http_request* req, struct buf_circle* buf);

//http respond
struct http_respond
{
    int32_t skt_id;
    int sta;
    struct buf_data* head;
    struct buf_data* data;
};
void http_respond_init();
struct http_respond* http_respond_get_respond(int32_t skt);
void http_respond_return_respond(struct http_respond* resp);


#endif //__HTTP_H__
