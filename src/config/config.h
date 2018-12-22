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

struct config* config_create();
int config_load_local_data(const char* filename);

#ifndef CONFIG_IMPLEMENT
#include "array.h"

struct config* config_create()
{
    struct config* cfg = (struct config*)malloc(sizeof(struct config));
    cfg->cfg_items = array_create(sizeof(struct config_item*));
    cfg->str_data = NULL;
}

int config_load_local_data(const char* filename)
{
    
}


#endif

#endif //__CONFIG_H__