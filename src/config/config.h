#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct array;

struct config_item
{
    char* key;
    short vt;
    union {
        int int_v;
        char* ch_v;
    } val;
};

struct config
{
    struct array* cfg_items;
    char* str_data;
};

struct config* config_create(void);
void config_destroy(struct config* cfg);
int config_load_local_data(struct config* cfg, const char* filename);
int config_get_int_value(struct config* cfg, const char* key);
const char* config_get_str_value(struct config* cfg, const char* key);

#ifdef CONFIG_IMPLEMENT
#include "array.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct config* config_create()
{
    struct config* cfg = (struct config*)malloc(sizeof(struct config));
    cfg->cfg_items = array_create(sizeof(struct config_item));
    cfg->str_data = NULL;
    return cfg;
}

void config_destroy(struct config* cfg)
{
    array_destroy(cfg->cfg_items);
    if (cfg->str_data != NULL)
    {
        free(cfg->str_data);
    }
    free(cfg);
}

static void config_parse_line(struct config* cfg, char* l)
{
    size_t len = strlen(l);
    if (len == 0)
        return;
    if (*l == '#')
        return;

    size_t pos = 0;
    while (pos < len && l[pos] != '=')
        pos++;

    if (pos == len)
        return;

    l[pos] = '\0';

    struct config_item item;
    item.key = l;
    int ret = utils_try_atoi(l + pos + 1, &item.val.int_v);
    if (ret == -1)
    {
        item.val.ch_v = l + pos + 1;
        item.vt = 1;
    }
    else
    {
        item.vt = 0;
    }
    array_add(cfg->cfg_items, &item);
}

int config_load_local_data(struct config* cfg, const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) 
        return -1;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    cfg->str_data = (char*)malloc((size_t)size + 1);
    fread(cfg->str_data, (size_t)size, 1, fp);
    *(cfg->str_data + size) = '\0';

    char* ctx = cfg->str_data;
    char* line = ctx;
    long idx = 0;
    while (idx < size)
    {
        if (*(ctx + idx) == '\n')
        {
            *(ctx + idx) = '\0';
            if (idx > 0 && *(ctx + idx - 1) == '\r')
                *(ctx + idx - 1) = '\0';
            config_parse_line(cfg, line);
            line = ctx + idx + 1;            
        }
        idx++;
    }
    if (line != ctx + idx)
    {
        config_parse_line(cfg, line);
    }
    return 0;
}

int config_get_int_value(struct config* cfg, const char* key)
{
    for (int i = 0; i < cfg->cfg_items->count; ++i)
    {
        struct config_item* item = (struct config_item*)array_index(cfg->cfg_items, i);
        if (strcmp(key, item->key) == 0)
        {
            return item->val.int_v;
        }
    }
    return -1;
}

const char* config_get_str_value(struct config* cfg, const char* key)
{
    for (int i = 0; i < cfg->cfg_items->count; ++i)
    {
        struct config_item* item = (struct config_item*)array_index(cfg->cfg_items, i);
        if (strcmp(key, item->key) == 0)
        {
            return item->val.ch_v;
        }
    }
    return NULL;
}

#endif //CONFIG_IMPLEMENT

#endif //__CONFIG_H__
