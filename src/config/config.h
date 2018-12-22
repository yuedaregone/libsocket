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
int config_load_local_data(struct config* cfg, const char* filename);

#ifndef CONFIG_IMPLEMENT
#include "array.h"

struct config* config_create()
{
    struct config* cfg = (struct config*)malloc(sizeof(struct config));
    cfg->cfg_items = array_create(sizeof(struct config_item*));
    cfg->str_data = NULL;
    return cfg;
}

int config_load_local_data(struct config* cfg, const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) 
        return -1;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    cfg->str_data = (char*)malloc((size_t)size);
    fread(cfg->str_data, (size_t)size, 1, fp);

    char* ctx = cfg->str_data;
    long index = 0;
    while (index < size)
    {
        char ch = *(ctx + index);
        switch (ch)
        {
            case '=':

                break;
            case '\n':
                *(ctx + index) = '\0';
                if (index > 0 && *(ctx + index - 1) == '\r')
                     *(ctx + index - 1) = '\0';
                
                break;
        }
    }
    return 0;
}


#endif //CONFIG_IMPLEMENT

#endif //__CONFIG_H__