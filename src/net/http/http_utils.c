#include "http.h"
#include "buffer.h"

int http_utils_get_tag(struct buf_data* data, char* buf, int len, char* st_str, char* ed_str)
{
	int32_t st_idx = buf_indexof_data(data, 0, (int8_t*)st_str, (int32_t)strlen(st_str));
	if (st_idx == -1)
		return HTTP_ERR;
	st_idx += strlen(st_str);

	int32_t ed_idx = buf_indexof_data(data, st_idx, (int8_t*)ed_str, (int32_t)strlen(ed_str));
	if (ed_idx == -1)
		return HTTP_ERR;

	if (len > ed_idx - st_idx)
	{
		strncpy(buf, (const char*)(data->buf + st_idx), ed_idx - st_idx);
	}
	return ed_idx - st_idx;
}

