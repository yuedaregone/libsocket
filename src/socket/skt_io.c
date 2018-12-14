#include "skt.h"
#include "buffer.h"

struct skt_io* skt_create_io(skt_d skt, skt_recv_data cb)
{
    struct skt_io* io = (struct skt_io*)malloc(sizeof(struct skt_io));
    io->skt = skt;
	io->data_cb = cb;
    io->err_no = SKT_OK;
    io->send_buf = buf_create_circle(BUFFER_SOCKET_DATA_SIZE);
	io->recv_buf = buf_create_circle(BUFFER_SOCKET_DATA_SIZE);
	io->cur_send = buf_create_data(BUFFER_SOCKET_ONE_FRAME);
	io->cur_recv = buf_create_data(BUFFER_SOCKET_ONE_FRAME);
    return io;
}

void skt_destroy_io(struct skt_io* io)
{
    buf_destroy_circle(io->send_buf);
	buf_destroy_circle(io->recv_buf);
	buf_destroy_data(io->cur_send);
	buf_destroy_data(io->cur_recv);
    free(io);
}

int32_t skt_send_io(struct skt_io* io, int8_t* buf, int32_t len)
{
	while (io->send_buf->cap < len)
	{
		io->send_buf = buf_relloc_circle(io->send_buf);
	}
	return buf_write_circle(io->send_buf, buf, len);
}


static void skt_update_send_io(struct skt_io* io)
{
	if (buf_size_data(io->cur_send) <= 0)
	{
		buf_reinit_data(io->cur_send);
		if (io->send_buf->data_sz <= 0)
			return;
		io->cur_send->ed_idx = buf_read_circle(io->send_buf, io->cur_send->buf, io->cur_send->cap);
	}

	int32_t sz = 0;
	while ((sz = buf_size_data(io->cur_send)) > 0)
	{
        int len = send(io->skt, io->cur_send->buf + io->cur_send->st_idx, sz, 0);
        if (len > 0)
        {
            io->cur_send->st_idx += len;
        }
        else
        {
            int err = GET_ERROR_CODE;
#ifdef _WIN32		
            if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS)
#else
            if (err == EINTR)
                continue;
            if (err != EAGAIN && err != EWOULDBLOCK && err != EINPROGRESS)
#endif
            {
                io->err_no = err; 
            }              
            break;
        }
	}
}

static void skt_update_recv_io(struct skt_io* io)
{
	if (buf_size_data(io->cur_recv) > 0)
	{
		io->cur_recv->st_idx += buf_write_circle(io->recv_buf, io->cur_recv->buf + io->cur_recv->st_idx, buf_size_data(io->cur_recv));
		if (buf_size_data(io->cur_recv) <= 0)
		{
			buf_reinit_data(io->cur_recv);
		}
		//cb
		if (io->data_cb != NULL)
		{
			(*io->data_cb)(io->skt, io->recv_buf);
		}		
	}

	int32_t sz = 0;
	while ((sz = buf_space_data(io->cur_recv)) > 0)
	{
        int len = recv(io->skt, io->cur_recv->buf + io->cur_recv->ed_idx, sz, 0);
        if (len > 0)
        {
            io->cur_recv->ed_idx += len;
        }
        else
        {
            if (len == 0)
            {
                io->err_no = SKT_ERR; 
            }	
            else
            {
                int err = GET_ERROR_CODE;
#ifdef _WIN32		
                if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS)
#else
                if (err == EINTR)
                    continue;
                if (err != EAGAIN && err != EWOULDBLOCK && err != EINPROGRESS)
#endif
                {
                    io->err_no = err; 
                }
            }
            break;
        }		
	}
}

void skt_update_io(struct skt_io* io)
{
    skt_update_send_io(io);
    skt_update_recv_io(io);
}

