#ifndef QUAD_OPEN_ADDRESS__H_
#define QUAD_OPEN_ADDRESS__H_

/*
 * Quadratic Probing Open Addressing Hash Table
 *
 * create()
 * destroy(T*)
 * put(T*, K)
 * get(T*, K)
 * size(T*)
 * resize(T*, int)
 * isvalid(T*, I)
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define qoacalloc(nmemb, size) calloc(nmemb, size)
#define qoafree(ptr, size) free(ptr)
#define qoareallocarray(ptr, nmemb, size) reallocarray(ptr, nmemb, size)
#define qoafreearray(ptr, nmemb, size) free(ptr)

#define flag_t uint32_t
#define key_t  int
#define val_t  int
// typedef uint32_t flag_t;
// typedef int key_t;
// typedef int val_t;
typedef int qoaiter;
struct qoatable_s
{
    flag_t *flags;
    key_t *keys;
    val_t *vals;
    uint32_t size;
    uint32_t asize;
    uint32_t used;
};
typedef struct qoatable_s qoatable;

#define qoa__fsize(x) ((x) / sizeof(flag_t))

qoatable *qoatable_create()
{
    return (qoatable*)qoacalloc(1, sizeof(qoatable));
}

void qoatable_destroy(qoatable* table)
{
    if (table) {
        qoafreearray(table->flags, qoa__fsize(table->size), sizeof(flag_t));
        qoafreearray(table->keys, table->size, sizeof(key_t));
        qoafreearray(table->vals, table->size, sizeof(val_t));
#ifndef NDEBUG
        table->flags = NULL;
        table->keys = NULL;
        table->vals = NULL;
        table->size = table->asize = table->used = 0;
#endif
        qoafree(table, sizeof(qoatable));
    }
}

#undef flag_t
#undef key_t
#undef val_t

#endif // QUAD_OPEN_ADDRESS__H_
