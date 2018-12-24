#ifndef __MSGS_H__
#define __MSGS_H__
#include <stdint.h>
#define MAX_MSG_TAG 512
#define MAX_MSG_SIZE 8192

typedef void (*msg_callback)(int32_t skt, void* data);
struct buf_circle;

void dispatch_init();
void dispatch_register_listen(uint16_t id, msg_callback cb);
void dispatch_unregister_listen(uint16_t id, msg_callback cb);
void dispatch_update_data(int32_t skt, struct buf_circle* buf);

int8_t* msg_make_wrap(uint16_t id, int8_t* buf, uint16_t* len);

#pragma pack(1)

struct msg_head
{
	uint16_t msg_id;
	uint16_t length;
};

#pragma pack()

#define MSG_HEAD_SIZE sizeof(struct msg_head)
#endif