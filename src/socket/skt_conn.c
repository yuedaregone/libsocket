#include <stdio.h>
#include <string.h>
#include "skt.h"
#include "array.h"
#include "buffer.h"

struct skt_conn* skt_create()
{
    struct skt_conn* skt = (struct skt_conn*)malloc(sizeof(struct skt_conn));
    skt->skt = INVALID_SOCKET;
    skt->sta = skt_noactivate;   
	skt->err_no = SKT_OK; 
    memset(skt->conn_ip, 0, IP_MAX_LEN);    
    skt->conn_port = 0;
	skt->recv_cb = NULL;
	skt->skt_io = NULL;
	skt->ac_ios = NULL;
    return skt;
}

void skt_destroy(struct skt_conn* skt)
{
	if (skt->skt_io != NULL)
	{
		skt_destroy_io(skt->skt_io);
	}
	if (skt->ac_ios != NULL)
	{
		for (int i = 0; i < skt->ac_ios->count; ++i)
		{
			skt_destroy_io(*(struct skt_io**)array_index(skt->ac_ios, i));
		}
		array_destroy(skt->ac_ios);
	}
	free(skt);
}

int skt_open_as_servers(struct skt_conn* skt, const char* ip, uint16_t port)
{
    skt->skt = socket(AF_INET, SOCK_STREAM, 0);
	if (skt->skt == INVALID_SOCKET)
	{
		int err = GET_ERROR_CODE;
		skt_error("Socket Error: Create Server Socket Error! %d\n", err);
		skt->err_no = err;
		return SKT_ERR;
	}

	int32_t op = 1;
	int32_t ret = setsockopt(skt->skt, SOL_SOCKET, SO_REUSEADDR, (char*)&op, sizeof(op));
	if (ret != 0)
	{
		int err = GET_ERROR_CODE;
		skt_error("Socket Error: Set Socket Opt Error! %d\n", err);
		skt->err_no = err;
		return SKT_ERR;
	}
	if (ip != NULL || strlen(ip) > 0)
	{
		memcpy(skt->conn_ip, ip, strlen(ip));
	}
	skt->conn_port = port;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	if (strlen(skt->conn_ip) == 0)
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		addr.sin_addr.s_addr = inet_addr(skt->conn_ip);
	}
	addr.sin_port = htons(port);

	ret = bind(skt->skt, (struct sockaddr*)&addr, sizeof(addr));
	if (ret != 0)
	{
		int err = GET_ERROR_CODE;
		skt_error("Socket Error: Bind Server Socket Error! %d\n", err);
		skt_close(skt);
		skt->err_no = err;
		return SKT_ERR;
	}

	ret = listen(skt->skt, SOMAXCONN);
	if (ret != 0)
	{
		int err = GET_ERROR_CODE;
		skt_error("Socket Error: Listen Socket Error! %d\n", err);
		skt_close(skt);
		skt->err_no = err;
		return SKT_ERR;
	}
	skt->skt_io = skt_create_io(skt, skt->skt);
	skt_set_non_block(skt->skt);
    skt->type = skt_server;
	skt->sta = skt_created;
	return SKT_OK;
}

int skt_open_as_client(struct skt_conn* skt, const char* local_ip)
{
	skt->skt = socket(AF_INET, SOCK_STREAM, 0);
	if (skt->skt == INVALID_SOCKET)
	{
		int err = GET_ERROR_CODE;
		skt_error("Socket Error: Create Server Socket Error! %d\n", err);
		skt->err_no = err;
		return SKT_ERR;
	}
	if (strlen(local_ip) == 0)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(local_ip);
		int32_t ret = bind(skt->skt, (struct sockaddr *)&addr, sizeof(addr));
		if (0 != ret)
		{
			int err = GET_ERROR_CODE;
			skt_error("Socket Error: Bind Client Socket Error! %d\n", err);
			skt_close(skt);
			skt->err_no = err;
			return SKT_ERR;
		}
	}	
	skt_set_non_block(skt->skt);
	skt->type = skt_client;
	skt->sta = skt_created;
	return SKT_OK;
}

int skt_accept(struct skt_conn* skt)
{
	struct sockaddr_in addr;
	socklen_t size = (socklen_t)sizeof(addr);
	int ret = accept(skt->skt, (struct sockaddr*)&addr, &size);
	if (ret != SKT_OK)
	{
		int err = GET_ERROR_CODE;;
#ifdef _WIN32
		if (err != WSAEWOULDBLOCK)
#else
		if (err != EINPROGRESS)
#endif
		{
			skt_error("Socket Error: Connect Error %d\n", err);
			skt_close(skt);
			skt->err_no = err;
			return SKT_ERR;
		}
		skt->err_no = err;
		return SKT_ERR;
	}
	return ret;
}

int skt_connect(struct skt_conn* skt, const char* ip, uint16_t port)
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
		if (err == WSAEWOULDBLOCK)
#else
		if (err == EINPROGRESS)
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
			skt_close(skt);
			skt->err_no = err;
			return SKT_ERR;		
		}
	}	
	skt->sta = skt_conneting;
	return SKT_OK;
}

static void skt_check_connect(struct skt_conn* skt)
{
	int32_t ret = skt_select_fd(skt->skt, 0.01, WAIT_FOR_WRITE);
	if (ret > 0)
	{
		skt->sta = skt_success;
		skt->err_no = SKT_OK;
		skt->skt_io = skt_create_io(skt, skt->skt);
	}	
	else if (ret < 0)
	{
		skt->sta = skt_unknown;
	}
}

static void skt_check_accept(struct skt_conn* skt)
{
	int sk = skt_accept(skt);
	if (sk != SKT_ERR)
	{
		struct skt_io* io = skt_create_io(skt, sk);
		array_add(skt->ac_ios, (void*)&io);
	}
}

void skt_update_state(struct skt_conn* skt)
{
	if (skt->type == skt_server)
	{
		skt_check_accept(skt);
	}
	
	if (skt->sta == skt_conneting)
	{
		skt_check_connect(skt);
	}
	else if (skt->sta == skt_success)
	{
		skt_update_io(skt->skt_io);
	}
	else
	{
		
	}
}

static int skt_find_io_index(struct skt_conn* skt, skt_d id)
{
	int i = -1;
	for (i = 0; i < skt->ac_ios->count; ++i)
	{
		struct skt_io* io = *(struct skt_io**)array_index(skt->ac_ios, i);
		if (io->skt == id)
		{
			break;
		}
	}
	return i;
}

int32_t skt_send(struct skt_conn* skt, int8_t* buf, int32_t len)
{
	return skt_send_io(skt->skt_io, buf, len);
}

int32_t skt_send_to_skt(struct skt_conn* skt, skt_d id, int8_t* buf, int32_t len)
{
	if (id == skt->skt)
	{
		return skt_send(skt, buf, len);
	}
	int idx = skt_find_io_index(skt, id);
	return skt_send_to_skt_index(skt, idx, buf, len);
}

int32_t skt_send_to_skt_index(struct skt_conn* skt, int idx, int8_t* buf, int32_t len)
{
	if (idx < 0 || idx >= skt->ac_ios->count)
		return SKT_ERR;
	struct skt_io* io = *(struct skt_io**)array_index(skt->ac_ios, idx);
	return skt_send_io(io, buf, len);
}

void skt_onioerror(struct skt_conn* skt, struct skt_io* io)
{
	if (skt->type == skt_server)
	{
		if (io->skt != skt->skt)
		{
			int idx = skt_find_io_index(skt, io->skt);
			skt_destroy_io(*(struct skt_io**)array_index(skt->ac_ios, idx));
			array_remove(skt->ac_ios, idx);
		}
	}
	else
	{
		skt->sta = skt_disconnect;
	}
}

void skt_close(struct skt_conn* skt)
{
#ifdef _WIN32
	closesocket(skt->skt);
#else
	close(skt->skt);
#endif 
}

