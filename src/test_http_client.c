#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <windows.h>
#endif
#include "buffer.h"
#include "http.h"


static void http_client_over(struct http_respond* resp)
{
	printf((const char*)resp->data->buf);
}

int main()
{
	struct http_client* clt = http_client_get("http://daregone.pythonanywhere.com/wintoy?method=file&name=shader_cloud.toy", NULL);
	clt->cb = http_client_over;
	http_client_send(clt);
	while (1) {
		http_client_update();
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}
	return 0;
}