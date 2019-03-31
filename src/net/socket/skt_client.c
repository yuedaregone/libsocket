#include <stdio.h>
#include <string.h>
#include "skt.h"
#include "buffer.h"

static int8_t buf_io[BUFFER_SOCKET_ONE_FRAME];

struct skt_client* skt_client_create()
{
#ifdef _WIN32
	WSADATA data;
	int result = WSAStartup(0x0202, &data);
	if (result != 0)
	{
		int err = GET_ERROR_CODE;
		skt_error("Error:Start up socket! %d\n", err);
		return NULL;
	}
#endif
    struct skt_client* skt = (struct skt_client*)malloc(sizeof(struct skt_client));
    skt->skt = INVALID_SOCKET;
    skt->sta = skt_noactivate;   
	skt->err_no = SKT_OK; 
    memset(skt->conn_ip, 0, IP_MAX_LEN);    
    skt->conn_port = 0;
	skt->recv_cb = NULL;
    skt->send_buf = buf_create_circle(BUFFER_SOCKET_DATA_SIZE);
    skt->recv_buf = buf_create_circle(BUFFER_SOCKET_DATA_SIZE);
    return skt;
}

void skt_client_destroy(struct skt_client* skt)
{
    buf_destroy_circle(skt->send_buf);
	buf_destroy_circle(skt->recv_buf);
    free(skt);
    
#ifdef _WIN32
	int result = WSACleanup();
	if (result != 0)
	{
		int err = GET_ERROR_CODE;
		skt_error("Error:Destroy socket! %d\n", err);
	}
#endif
}

int skt_client_open(struct skt_client* skt, const char* local_ip, uint16_t local_port)
{
    skt->skt = socket(AF_INET, SOCK_STREAM, 0);
	if (skt->skt == INVALID_SOCKET)
	{
		int err = GET_ERROR_CODE;
		skt_error("Socket Error: Create Server Socket Error! %d\n", err);
		skt->err_no = err;
		return SKT_ERR;
	}
	if (local_ip != NULL && strlen(local_ip) > 0)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(local_ip);
		addr.sin_port = htons(local_port);
		int32_t ret = bind(skt->skt, (struct sockaddr *)&addr, sizeof(addr));
		if (0 != ret)
		{
			int err = GET_ERROR_CODE;
			skt_error("Socket Error: Bind Client Socket Error! %d\n", err);
			skt_client_close(skt);
			skt->err_no = err;
			return SKT_ERR;
		}
	}	
	skt_set_non_block(skt->skt);
	skt->sta = skt_created;
	return SKT_OK;
}

int skt_client_connect(struct skt_client* skt, const char* ip, uint16_t port)
{
    if (ip == NULL)
	{
		skt_error("Socket Error: Remote IP is NULL");
		return SKT_ERR;
	}
	memcpy(skt->conn_ip, ip, strlen(ip));
	skt->conn_port = port;

	uint32_t ip_addr = (uint32_t)inet_addr(ip);	

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip_addr;
	addr.sin_port = htons(port);

	int32_t ret = connect(skt->skt, (const struct sockaddr*)&addr, sizeof(addr));
	if (ret != SKT_OK)
	{
		int err = GET_ERROR_CODE;;
#ifdef _WIN32		
		if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS)
#else
		if (err == EINPROGRESS || err == EWOULDBLOCK)
#endif
		{
			skt_warning("Socket Warning: Connect Busy\n");
			skt->sta = skt_conneting;
			skt->err_no = err;
			return SKT_OK;
		}
		else
		{
			skt_error("Socket Error: Connect Error %d\n", err);
			skt_client_close(skt);
			skt->err_no = err;
			return SKT_ERR;		
		}
	}	
	skt->sta = skt_conneting;
	return SKT_OK;
}

static void skt_check_connect(struct skt_client* skt)
{
	int32_t ret = skt_select_fd(skt->skt, 0.001, WAIT_FOR_WRITE);
	if (ret > 0)
	{
		skt->sta = skt_success;
		skt->err_no = SKT_OK;
	}	
	else if (ret < 0)
	{
		skt->sta = skt_unknown;
	}
}

int32_t skt_client_send_to(struct skt_client* skt, int8_t* buf, int32_t len)
{
	while (skt->send_buf->cap < len)
	{
		skt->send_buf = buf_relloc_circle(skt->send_buf);
	}
	return buf_write_circle(skt->send_buf, buf, len);
}

static void skt_client_update_send(struct skt_client* skt)
{
	if (skt->send_buf->data_sz <= 0) return;
	
	int dt_sz = buf_peek_circle(skt->send_buf, buf_io, BUFFER_SOCKET_ONE_FRAME);

	int32_t sz = 0;
	while (sz < dt_sz)
	{
        int len = (int)send(skt->skt, buf_io + sz, dt_sz - sz, 0);
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
                skt->err_no = err; 
                skt->sta = skt_disconnect;
            }              
            break;
        }
	}
	if (sz > 0)
	{
		buf_offset_circle(skt->send_buf, sz);
	}
}

static void skt_client_update_recv(struct skt_client* skt)
{
	int dt_sz = buf_space_circle(skt->recv_buf);	
	if (dt_sz <= 0) return;

	dt_sz = dt_sz > BUFFER_SOCKET_ONE_FRAME ? BUFFER_SOCKET_ONE_FRAME : dt_sz;

	int32_t sz = 0;
	while (sz < dt_sz)
	{
        int len = (int)recv(skt->skt, buf_io + sz, dt_sz - sz, 0);
        if (len > 0)
        {
			sz += len;
        }
        else
        {
            if (len == 0)
            {
                skt->sta = skt_disconnect;
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
                    skt->err_no = err; 
                    skt->sta = skt_disconnect;
                }
            }
            break;
        }		
	}

	if (sz > 0)
	{
		buf_write_circle(skt->recv_buf, buf_io, sz);

		if (skt->recv_cb != NULL)
		{
			(skt->recv_cb)(skt->skt, skt->recv_buf);
		}
	}
}

void skt_client_update_state(struct skt_client* skt)
{	
	if (skt->sta == skt_conneting)
	{
		skt_check_connect(skt);
	}
	else if (skt->sta == skt_success)
	{
		skt_client_update_send(skt);
        skt_client_update_recv(skt);
	}
	else
	{
		
	}
}

void skt_client_close(struct skt_client* skt)
{
	skt_close(skt->skt);
}
