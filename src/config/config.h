#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct array;

struct config
{
    struct config_item
    {
        char* key;
        union {
            int int_v;
            char* ch_v;
        } val;
    };
    struct array* cfg_items;
    char* str_data;
};


#endif //__CONFIG_H__