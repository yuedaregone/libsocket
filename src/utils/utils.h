#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdio.h>

char* utils_trim_left_str(char* str);
char* utils_trim_right_str(char* str);

int utils_try_atoi(char* str, int* num);
int utils_try_atoi_hex(char* str, int* num);

int utils_indexof_data(char* buf, int buf_len, char* checker, int checker_len);

#endif //__UTILS_H__