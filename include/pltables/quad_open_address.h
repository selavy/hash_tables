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

#define qoa_calloc(nmemb, size) calloc(nmemb, size)
#define qoa_free(ptr, size) free(ptr)
#define qoa_reallocarray(ptr, nmemb, size) reallocarray(ptr, nmemb, size)
#define qoa_freearray(ptr, nmemb, size) free(ptr)

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
    uint32_t upbnd;
};
typedef struct qoatable_s qoatable;

int qoa_int_hash_func(key_t k) { return (int)k; }

/* TODO: should 0 be equal like strcmp? */
int qoa_int_equal(key_t a, key_t b) { return a == b; }


#define qoa__fsize(x) ((x) / sizeof(flag_t))

qoatable *qoa_i32_create()
{
    return (qoatable*)qoa_calloc(1, sizeof(qoatable));
}

void qoa_i32_init(qoatable* t)
{
    t->flags = NULL;
    t->keys = NULL;
    t->vals = NULL;
    t->size = t->asize = t->used = t->upbnd = 0;
}

void qoa_i32_destroy(qoatable* t)
{
    if (t) {
        qoa_freearray(t->flags, qoa__fsize(t->size), sizeof(flag_t));
        qoa_freearray(t->keys, t->size, sizeof(key_t));
        qoa_freearray(t->vals, t->size, sizeof(val_t));
#ifndef NDEBUG
        t->flags = NULL;
        t->keys = NULL;
        t->vals = NULL;
        t->size = t->asize = t->used = t->upbnd = 0;
#endif
        qoa_free(t, sizeof(qoatable));
    }
}

int qoa_i32_size(const qoatable* t)
{
    return t->size;
}

#define qoa_create(name) qoa_ ## name ## _create()
#define qoa_init(name, t) qoa_ ## name ## _init(t)
#define qoa_destroy(name, t) qoa_ ## name ## _destroy(t)
#define qoa_size(name, t) qoa_ ## name ## _size(t)

#undef flag_t
#undef key_t
#undef val_t

#endif // QUAD_OPEN_ADDRESS__H_
