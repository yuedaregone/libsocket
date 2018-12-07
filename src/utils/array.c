#include "array.h"
#include <stdlib.h>
#include <string.h>

struct array* array_create(int item_size)
{
    return array_create_with_capacity(item_size, DEFAULT_CAPACITY);
}

struct array* array_create_with_capacity(int item_size, int capacity)
{
    struct array* ar = (struct array*)malloc(sizeof(struct array));
    ar->cap = capacity;
    ar->count = 0;
    ar->im_sz = item_size;
    ar->data = malloc(ar->cap * ar->im_sz);
    return ar;
}

void array_destroy(struct array* ar)
{
    free(ar->data);
    free(ar);
}

static void array_relloc_array(struct array* ar)
{
    int capacity = ar->cap * 2;
    void* data = malloc(capacity * ar->im_sz);
    memcpy(data, ar->data, ar->cap * ar->im_sz);
    ar->cap = capacity;
    ar->data = data;
}

int array_add(struct array* ar, void* data)
{
    if (ar->count >= ar->cap)
    {
        array_relloc_array(ar);
    }    
    memcpy((char*)ar->data + ar->count * ar->im_sz, data, ar->im_sz);
    ar->count++;
    return AR_OK;
}

int array_remove(struct array* ar, int idx)
{
    if (idx < 0 || idx >= ar->count)
        return AR_ERR;
    if (idx == ar->count - 1)
    {
        ar->count--;
        return AR_OK;
    }
    memmove((char*)ar->data + idx * ar->im_sz, (char*)ar->data + (idx + 1) * ar->im_sz, (ar->count - idx - 1) * ar->im_sz);    
    ar->count--;
    return AR_OK;
}

void* array_index(struct array* ar, int idx)
{
    return (char*)ar->data + idx * ar->im_sz;
}

