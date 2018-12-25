#include "msgs.h"
#include "array.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>


struct dispatch
{
    struct array* msg_listens[MAX_MSG_TAG];
};
static struct dispatch msg_dispatch;

void dispatch_init()
{
    for (int i = 0; i < MAX_MSG_TAG; ++i)
    {
		struct array* ar = msg_dispatch.msg_listens[i];
		if (ar != NULL)
		{
			array_destroy(ar);
			msg_dispatch.msg_listens[i] = NULL;
		}   
    }
}

void dispatch_destroy()
{
	for (int i = 0; i < MAX_MSG_TAG; ++i)
	{
		msg_dispatch.msg_listens[i] = NULL;
	}
}

void dispatch_register_listen(uint16_t id, msg_callback cb)
{
    struct array* ar = msg_dispatch.msg_listens[id];
    if (ar == NULL)
    {
        ar = array_create(sizeof((void*)&cb));        
        msg_dispatch.msg_listens[id] = ar;
    }
    array_add(ar, (void*)&cb);
}

void dispatch_unregister_listen(uint16_t id, msg_callback cb)
{
    struct array* ar = msg_dispatch.msg_listens[id];
    if (ar == NULL)
    {
        return;
    }        
    for (int i = 0; i < ar->count; ++i)
    {
        void* item = array_index(ar, i);
        msg_callback icb = *(msg_callback*)item;
        if (icb == cb)
        {
            array_remove(ar, i);
            break;
        }
    }
}

static void dispatch_dispatch_msg(int32_t skt, uint16_t id, void* data)
{
    struct array* ar = msg_dispatch.msg_listens[id];
    if (ar == NULL)
    {
        return;
    }      
    for (int i = 0; i < ar->count; ++i)
    {
        void* item = array_index(ar, i);
        msg_callback icb = *(msg_callback*)item;
        (*icb)(skt, data);
    }
}

void dispatch_update_data(int32_t skt, struct buf_circle* buf)
{
    static int8_t s_buff[MAX_MSG_SIZE] = {0};
    int32_t sz = buf_peek_circle(buf, s_buff, MSG_HEAD_SIZE);
    if (sz < (int32_t)MSG_HEAD_SIZE)
        return;

    struct msg_head* head = (struct msg_head*)s_buff;
    int32_t sum = head->length + MSG_HEAD_SIZE;
    if (sum < 0 || sum > (int32_t)MAX_MSG_SIZE)
    {
        printf("%s\n", "Error msg size!");
        buf_clear_circle(buf);
        return;
    }

    if (sum <= buf_space_circle(buf))   
        return;
    
    sz = buf_read_circle(buf, s_buff, sum);
    if (sum != sz)
    {
        printf("%s\n", "Error msg read!");
        buf_clear_circle(buf);
        return;
    }
    dispatch_dispatch_msg(skt, head->msg_id, s_buff + MSG_HEAD_SIZE);
}
