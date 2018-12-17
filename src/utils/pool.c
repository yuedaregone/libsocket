#include <stdio.h>
#include <stdlib.h>
#include "pool.h"
#include "array.h"

struct pool* pool_create(pool_create_item_fun create, pool_destroy_item_fun destroy)
{
    struct pool* p = (struct pool*)malloc(sizeof(struct pool));
    p->create = create;
    p->destroy = destroy;
    p->act = array_create(sizeof(void*));
    p->dis = array_create(sizeof(void*));
    return p;
}

void pool_destroy(struct pool* p)
{
    array_destroy(p->act);
    array_destroy(p->dis);
    free(p);
}


void* pool_request(struct pool* p)
{
    void* data = NULL;
    if (p->dis->count > 0)
    {
        data = array_index(p->dis, p->dis->count - 1);
        array_remove(p->dis, p->dis->count - 1);        
    }
    else
    {
        void* crt = (void*)(*p->create)();
        data = (void*)&crt;
    }
    array_add(p->act, data);
    return data;
}

void pool_return(struct pool* p, void* item)
{
    for (int i = 0;i < p->act->count; ++i)
    {
        void* data = array_index(p->act, i);
        if (*(long*)data == *(long*)item)
        {
            array_remove(p->act, i);
            break;
        }
    }
    array_add(p->dis, item);
}