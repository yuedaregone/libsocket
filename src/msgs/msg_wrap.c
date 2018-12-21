#include "msgs.h"

int8_t* msg_make_wrap(uint16_t id, int8_t* buf, uint16_t* len)
{
    static int8_t s_buff[MAX_MSG_SIZE] = {0};
    struct msg_head head;
    head.msg_id = id;
    head.length = *len;

    memcpy(s_buff, &head, MSG_HEAD_SIZE);    
    memcpy(s_buff, buf, len);
    *len += MSG_HEAD_SIZE;
    return s_buff;
}
