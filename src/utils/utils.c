#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char* utils_trim_left_str(char* str)
{
    char* s = str;
    while (*s != '\0')
    {
        if (isspace(*s))
            s++;
        else
            break;
    }
    return s;
}

char* utils_trim_right_str(char* str)
{
    size_t len = strlen(str);
    if (len == 0)
        return NULL;

    char* trail = str + len - 1;
    while (str != trail)
    {
        if (isspace(*trail))
            *(trail--) = '\0';
        else
            break;
    }
    return str;
}

int utils_try_atoi(char* str, int* num)
{
    char* s = str;
    char ch;
    while ((ch = *(s++)) != '\0')
    {
        if (!isspace(ch) && !isdigit(ch))
            return -1;
    }
    *num = atoi(str);
    return 0;
}

int utils_try_atoi_hex(char* str, int* num)
{
	char* s = str;
	char ch;
	while ((ch = *(s++)) != '\0')
	{
		if (!isspace(ch) && !isdigit(ch))
			return -1;
	}
	*num = (int)strtol(str, NULL, 16);
	return 0;
}

int utils_indexof_data(char* buf, int buf_len, char* checker, int checker_len)
{
	int idx = 0;
	while (idx < buf_len - checker_len + 1)
	{
		int bingo = 1;
		for (int i = 0; i < checker_len; ++i)
		{
			if (*(buf + idx + i) != *(checker + i))
			{
				bingo = 0;
				break;
			}
		}
		if (bingo)
		{
			return idx;
		}
		else
		{
			idx++;
		}
	}
	return -1;
}
