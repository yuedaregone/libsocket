#ifndef __POOL_H__
#define __POOL_H__
#include <stdio.h>
#include "array.h"

typedef void* (*pool_create_item_fun)();
typedef void (*pool_destroy_item_fun)(void* item);

struct pool
{
    pool_create_item_fun create;
    pool_destroy_item_fun destroy;
    struct array* act;
    struct array* dis;
};

struct pool* pool_create(pool_create_item_fun create, pool_destroy_item_fun destroy);
void pool_destroy(struct pool* p);
void* pool_request(struct pool* p);
void pool_return(struct pool* p, void* item);

#endif