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
	for (int i = 0; i < p->act->count; ++i)
	{
		void* data = array_index(p->act, i);
		(*p->destroy)(*((void**)&data));
	}
	for (int i = 0; i < p->dis->count; ++i)
	{
		void* data = array_index(p->dis, i);
		(*p->destroy)(*((void**)&data));
	}
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
        void* crt = (*p->create)();
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

/* unit test
void pool_test()
{
	struct pool* p = pool_create(skt_client_create, skt_client_destroy);

	void* v1 = pool_request(p);
	struct skt_client* c1 = *(struct skt_client**)v1;
	c1->skt = 1024;

	v1 = pool_request(p);
	struct skt_client* c2 = *(struct skt_client**)v1;
	c2->skt = 1026;

	pool_return(p, (void*)&c1);
	pool_return(p, (void*)&c2);

	v1 = pool_request(p);
	struct skt_client* c3 = *(struct skt_client**)v1;
	printf("%d\n", c3->skt);
}
*/
