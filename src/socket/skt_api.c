#include "skt.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

int32_t skt_set_non_block(int32_t fd)
{
	int flag = 0;
	flag = fcntl(fd, F_GETFL, 0);
	if (flag < 0)
	{
		return -1;
	}
	flag |= O_NONBLOCK;
	flag |= O_NDELAY;

	if (fcntl(fd, F_SETFL, flag) < 0)
	{
		return -1;
	}
	return 0;
}
#else

int32_t skt_set_non_block(int32_t fd)
{
	u_long nNonBlocking = 1;
	ioctlsocket(fd, FIONBIO, &nNonBlocking);
	return 0;
}
#endif

void skt_delay(uint32_t usec)
{
	struct timeval timeout;
	fd_set fds;
	FD_ZERO(&fds);

	timeout.tv_sec = usec / 1000000;
	timeout.tv_usec = usec % 1000000;
	int ret = select(0, NULL, NULL, &fds, &timeout);
	if (0 > ret)
	{
		perror("select");
	}
}

int32_t skt_select_fd(int32_t fd, double maxtime, int32_t wait_for)
{
	fd_set fdset;
	fd_set *rd = NULL, *wr = NULL;
	struct timeval tmout;

	FD_ZERO(&fdset);
	FD_SET((uint32_t)fd, &fdset);
	if (wait_for & WAIT_FOR_READ)
		rd = &fdset;
	if (wait_for & WAIT_FOR_WRITE)
		wr = &fdset;

	tmout.tv_sec = (long)maxtime;
	tmout.tv_usec = (long)(1000000 * (maxtime - (long)maxtime));
	
	return select(fd + 1, rd, wr, NULL, &tmout);
}

void skt_close(skt_d id)
{
#ifdef _WIN32
	closesocket(id);
#else
	close(id);
#endif 
}

