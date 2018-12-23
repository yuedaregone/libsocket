#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdio.h>

char* utils_trim_left_str(char* str);
char* utils_trim_right_str(char* str);

int utils_try_atoi(char* str, int* num);

#endif //__UTILS_H__