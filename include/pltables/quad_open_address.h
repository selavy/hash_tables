#ifndef QUAD_OPEN_ADDRESS__H_
#define QUAD_OPEN_ADDRESS__H_

/*
 * Quadratic Probing Open Addressing Hash Table
 *
 * create()
 * destroy(T*)
 * size(T*)
 * valid(T*, I) = exist(T*, I)
 * resize(T*, int)
 * put(T*, K)
 * insert(T*, K)
 * get(T*, K)
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef reallocarray
#define reallocarray(ptr, nmemb, size) realloc(ptr, (nmemb)*(size))
#endif

#define qoa_calloc(nmemb, size) calloc(nmemb, size)
#define qoa_free(ptr, size) free(ptr)
#define qoa_reallocarray(ptr, nmemb, size) reallocarray(ptr, nmemb, size)
#define qoa_freearray(ptr, nmemb, size) free(ptr)

#define QOA_MIN_TABLE_SIZE 4
#define qoa_max_load_factor(asize) ((int)(0.77 * (asize) + 0.5))
#define flag_t uint32_t
#define key_t  int
#define val_t  int
// typedef uint32_t flag_t;
// typedef int key_t;
// typedef int val_t;
typedef int qoaiter;
struct qoatable_s
{
    flag_t  *flags;
    key_t   *keys;
    val_t   *vals;
    uint32_t size;
    uint32_t asize;
    uint32_t used;
    uint32_t upbnd;
};
typedef struct qoatable_s qoatable;

enum
{
    QOA_ERROR   = -1,
    QOA_PRESENT =  0,
    QOA_NEW     =  1,
    QOA_DELETED =  2,
};

struct qoaresult_s
{
    qoaiter iter;
    int     result;
};
typedef struct qoaresult_s qoaresult;

int qoa_int_hash_func(key_t k) { return (int)k; }
int qoa_int_equal(key_t a, key_t b) { return a - b; }

#define qoa__hash_func(k) qoa_int_hash_func(k)
#define qoa__equal(a, b)  qoa_int_equal(a, b)
#define qoa__eq(a, b)  (qoa_int_equal(a, b) == 0)
#define qoa__neq(a, b) (qoa_int_equal(a, b) != 0)

/* 2 bits per entry, 0 = EMPTY, 1 = LIVE, 2 = DEL */
#define qoa__fsize(x) ((x) / sizeof(flag_t))
/* TODO: base these on sizeof(flag_t) */
#define qoa__isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define qoa__isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define qoa__iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define qoa__islive(flag, i) (qoa__iseither(flag, i) == 0)
#define qoa__set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define qoa__set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define qoa__set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define qoa__set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))
#define qoa__swapkeys(a, b) do { key_t tmp = a; a = b; b = tmp; } while (0)
#define qoa__swapvals(a, b) do { val_t tmp = a; a = b; b = tmp; } while (0)

uint32_t qoa__rounduppow2(uint32_t x)
{
    assert(x != 0);
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    ++x;
    return x;
}

qoatable *qoa_create_i32()
{
    return (qoatable*)qoa_calloc(1, sizeof(qoatable));
}

void qoa_init_i32(qoatable* t)
{
    t->flags = NULL;
    t->keys = NULL;
    t->vals = NULL;
    t->size = t->asize = t->used = t->upbnd = 0;
}

void qoa_destroy_i32(qoatable* t)
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

int qoa_size_i32(const qoatable* t)
{
    return t->size;
}

int qoa_valid_i32(const qoatable *t, qoaiter it)
{
    return qoa__islive(t->flags, it);
}

int qoa_exist_i32(const qoatable *t, qoaiter it)
{
    return qoa_valid_i32(t, it);
}

int qoa_resize_fast_i32(qoatable *t, int newasize)
{
    flag_t *newflags, *oldflags = t->flags;
    key_t key, *keys;
    val_t val, *vals;
    int j, k, i, step, oldasize = t->asize, mask = newasize - 1;
    assert(newasize >= QOA_MIN_TABLE_SIZE);
    assert((newasize & (newasize - 1)) == 0); /* table size must be a power of 2 */
    assert(t->size <= qoa_max_load_factor(newasize));
    newflags = (flag_t*)qoa_calloc(qoa__fsize(newasize), sizeof(flag_t));
    keys = (key_t*)qoa_reallocarray(t->keys, newasize, sizeof(key_t));
    vals = (val_t*)qoa_reallocarray(t->vals, newasize, sizeof(val_t));
    if (!newflags || !keys || !vals) {
        free(newflags); free(keys); free(vals);
        return -1;
    }
    memset(newflags, 0xaa, qoa__fsize(newasize) * sizeof(flag_t));
    for (j = 0; j < oldasize; ++j) {
        if (!qoa__islive(oldflags, j))
            continue;
        key = keys[j];
        val = vals[j];
        qoa__set_isdel_true(oldflags, j);
        for (;;) { /* kick-out process */
            k = qoa__hash_func(key);
            i = k & mask;
            step = 0;
            while (!qoa__isempty(oldflags, i)) i = (i + (++step)) & mask;
            qoa__set_isempty_false(newflags, i);
            if (i < oldasize && qoa__islive(oldflags, i)) {
                qoa__swapkeys(keys[i], key);
                qoa__swapvals(vals[i], val);
                qoa__set_isdel_true(oldflags, i);
            } else {
                /* write the element and jump out of the loop */
                keys[i] = key;
                vals[i] = val;
                break;
            }
        }
    }
    t->flags = newflags;
    t->keys = keys;
    t->vals = vals;
    t->asize = newasize;
    t->used = t->size;
    t->upbnd = qoa_max_load_factor(newasize);
    free(oldflags);
    return 0;
}

int qoa_resize_i32(qoatable *t, int newasize)
{
    newasize = newasize >= QOA_MIN_TABLE_SIZE ? newasize : QOA_MIN_TABLE_SIZE;
    newasize = newasize >= t->upbnd ? newasize : t->upbnd;
    newasize = qoa__rounduppow2(newasize);
    return qoa_resize_fast_i32(t, newasize);
}

qoaresult qoa_insert_i32(qoatable *t, key_t key)
{
    qoaresult res;
    flag_t *flags = t->flags;
    key_t *keys = t->keys;
    int x, k, i, site, last, step, asize = t->asize, mask = t->asize - 1;
    if (t->used >= t->upbnd) {
        int newasize = asize > 2*t->size ? asize : 2*asize;
        if (qoa_resize_fast_i32(t, newasize) < 0) {
            res.iter = asize; res.result = QOA_ERROR;
            // return { asize, QOA_ERROR };
            return res;
        }
    }
    step = 0;
    x = site = asize;
    k = qoa__hash_func(key);
    i = k & mask;
    if (qoa__isempty(flags, i)) {
        x = i;
    } else {
        last = i;
        while (
            qoa__islive(flags, i) &&
            (qoa__isdel(flags, i) || !qoa__eq(keys[i], key))
        ) {
            if (qoa__isdel(flags, i))
                site = i;
            i = (i + (++step)) & mask;
            if (i == last) {
                x = site;
                break;
            }
        }
        if (x == asize) {
            x = qoa__isempty(flags, i) && site != asize ? site : i;
        }
    }
    if (qoa__isempty(flags, x)) {
        keys[x] = key;
        qoa__set_isboth_false(flags, x);
        ++t->size;
        ++t->used;
        // return { x, QOA_NEW };
        res.iter = x;
        res.result = QOA_NEW;
    } else if (qoa__isdel(flags, x)) {
        t->keys[x] = key;
        qoa__set_isboth_false(flags, x);
        ++t->size;
        // return { x, QOA_DELETED };
        res.iter = x;
        res.result = QOA_DELETED;
    } else {
        // return { x, QOA_PRESENT };
        res.iter = x;
        res.result = QOA_PRESENT;
    }
    return res;
}

/* Public Interface */
#define qoa_create(name)           qoa_create_##name()
#define qoa_init(name, t)          qoa_init_##name(t)
#define qoa_destroy(name, t)       qoa_destroy_##name(t)
#define qoa_size(name, t)          qoa_size_##name(t)
#define qoa_valid(name, t, iter)   qoa_valid_##name(t, it)
#define qoa_exist(name, t, iter)   qoa_exist_##name(t, it)
#define qoa_resize(name, t, asize) qoa_resize_##name(t, asize)
#define qoa_resize_fast(name, t, asize) qoa_resize_fast_##name(t, asize)
#define qoa_insert(name, t, key)   qoa_insert_##name(t, key)

#undef flag_t
#undef key_t
#undef val_t

#endif // QUAD_OPEN_ADDRESS__H_
