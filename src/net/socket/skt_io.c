#include "skt.h"
#include "buffer.h"

static int8_t buf_io[BUFFER_SOCKET_ONE_FRAME];

struct skt_io* skt_create_io(skt_d skt, skt_recv_data cb)
{
    struct skt_io* io = (struct skt_io*)malloc(sizeof(struct skt_io));
    io->skt = skt;
	io->wt_flag = 0;
	io->data_cb = cb;
    io->err_no = SKT_OK;
    io->send_buf = buf_create_circle(BUFFER_SOCKET_DATA_SIZE);
	io->recv_buf = buf_create_circle(BUFFER_SOCKET_DATA_SIZE);
    return io;
}

void skt_destroy_io(struct skt_io* io)
{
    buf_destroy_circle(io->send_buf);
	buf_destroy_circle(io->recv_buf);
    free(io);
}

int32_t skt_send_io(struct skt_io* io, int8_t* buf, int32_t len)
{
	while (io->send_buf->cap < len)
	{
		io->send_buf = buf_relloc_circle(io->send_buf);
	}
	io->wt_flag |= len > 0;
	return buf_write_circle(io->send_buf, buf, len);
}


void skt_update_send_io(struct skt_io* io)
{
	if (io->send_buf->data_sz <= 0) return;

	int dt_sz = buf_peek_circle(io->send_buf, buf_io, BUFFER_SOCKET_ONE_FRAME);

	int32_t sz = 0;
	while (sz < dt_sz)
	{
		int len = (int)send(io->skt, buf_io + sz, dt_sz - sz, 0);
		if (len > 0)
		{
			sz += len;
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
	if (sz > 0)
	{
		buf_offset_circle(io->send_buf, sz);

		if (io->send_buf->data_sz <= 0)
			io->wt_flag = 0;
	}
}

void skt_update_recv_io(struct skt_io* io)
{
	int dt_sz = buf_space_circle(io->recv_buf);
	if (dt_sz <= 0) return;

	dt_sz = dt_sz > BUFFER_SOCKET_ONE_FRAME ? BUFFER_SOCKET_ONE_FRAME : dt_sz;

	int32_t sz = 0;
	while (sz < dt_sz)
	{
		int len = (int)recv(io->skt, buf_io + sz, dt_sz - sz, 0);
		if (len > 0)
		{
			sz += len;
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

	if (sz > 0)
	{
		buf_write_circle(io->recv_buf, buf_io, sz);

		if (io->data_cb != NULL)
		{
			(*io->data_cb)(io->skt, io->recv_buf);
		}
	}
}


