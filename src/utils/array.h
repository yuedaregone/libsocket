#ifndef __ARRAY_H__
#define __ARRAY_H__

#define DEFAULT_CAPACITY 8
#define AR_OK 0
#define AR_ERR -1

struct array 
{
    int cap;   
    int count;
    int im_sz;
    void* data;
};

struct array* array_create(int item_size);
struct array* array_create_with_capacity(int item_size, int capacity);
void array_destroy(struct array* ar);
int array_add(struct array* ar, void* data);
int array_remove(struct array* ar, int idx); 
void* array_index(struct array* ar, int idx);

#endif