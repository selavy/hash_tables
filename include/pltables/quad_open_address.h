#ifndef QUAD_OPEN_ADDRESS__H_
#define QUAD_OPEN_ADDRESS__H_

/*
 * Quadratic Probing Open Addressing Hash Table
 *
 * Heavily borrowed from khash. TODO: add MIT license
 *
 * create()
 * destroy(T*)
 * size(T*)
 * valid(T*, Iter) = exist(T*, Iter)
 * resize(T*, int)
 * put(T*, K)
 * insert(T*, K)
 * get(T*, K)
 * del(T*, Iter)
 * erase(T*, K)
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef reallocarray
#define reallocarray(ptr, nmemb, size) realloc(ptr, (nmemb)*(size))
#define QOA_DEFINED_REALLOCARRAY
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

static inline uint32_t qoa__rounduppow2(uint32_t x)
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

int qoa_valid_i32(const qoatable *t, qoaiter iter)
{
    assert(t != NULL);
    assert(0 <= iter && iter <= t->asize);
    return iter != t->asize && qoa__islive(t->flags, iter);
}

int qoa_exist_i32(const qoatable *t, qoaiter iter)
{
    return qoa_valid_i32(t, iter);
}

int qoa_resize_fast_i32(qoatable *t, int newasize)
{
    flag_t *flags, *oldflags = t->flags;
    key_t key, *keys;
    val_t val, *vals;
    int j, k, i, step, oldasize = t->asize, mask = newasize - 1;
    assert(newasize >= QOA_MIN_TABLE_SIZE);
    assert((newasize & (newasize - 1)) == 0);
    assert(t->size <= qoa_max_load_factor(newasize));
    flags = (flag_t*)qoa_calloc(qoa__fsize(newasize), sizeof(flag_t));
    keys = (key_t*)qoa_reallocarray(t->keys, newasize, sizeof(key_t));
    vals = (val_t*)qoa_reallocarray(t->vals, newasize, sizeof(val_t));
    if (!flags || !keys || !vals) {
        free(flags); free(keys); free(vals);
        return -1;
    }
    memset(flags, 0xaa, qoa__fsize(newasize) * sizeof(flag_t));
    for (j = 0; j < oldasize; ++j) {
        if (!qoa__islive(oldflags, j))
            continue;
        key = keys[j];
        val = vals[j];
        qoa__set_isdel_true(oldflags, j);
        for (;;) {
            k = qoa__hash_func(key);
            i = k & mask;
            step = 0;
            while (!qoa__isempty(oldflags, i)) i = (i + (++step)) & mask;
            qoa__set_isempty_false(flags, i);
            if (i < oldasize && qoa__islive(oldflags, i)) {
                qoa__swapkeys(keys[i], key);
                qoa__swapvals(vals[i], val);
                qoa__set_isdel_true(oldflags, i);
            } else {
                keys[i] = key;
                vals[i] = val;
                break;
            }
        }
    }
    t->flags = flags;
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
    flag_t *flags;
    key_t *keys;
    int x, k, i, site, last, step, mask, asize = t->asize;
    if (t->used >= t->upbnd) {
        int newasize = asize > 2*t->size ? asize : 2*asize;
        newasize = newasize >= QOA_MIN_TABLE_SIZE ? newasize : QOA_MIN_TABLE_SIZE;
        if (qoa_resize_fast_i32(t, newasize) < 0) {
            res.iter = asize;
            res.result = QOA_ERROR;
            return res;
        }
        asize = t->asize;
    }
    mask = asize - 1;
    flags = t->flags;
    keys = t->keys;
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
            (qoa__isdel(flags, i) || qoa__neq(keys[i], key))
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
        res.iter = x;
        res.result = QOA_NEW;
    } else if (qoa__isdel(flags, x)) {
        t->keys[x] = key;
        qoa__set_isboth_false(flags, x);
        ++t->size;
        res.iter = x;
        res.result = QOA_DELETED;
    } else {
        res.iter = x;
        res.result = QOA_PRESENT;
    }
    return res;
}

qoaiter qoa_put_i32(qoatable *t, key_t key, int* res)
{
    qoaresult r = qoa_insert_i32(t, key);
    *res = r.result;
    return r.iter;
}

const key_t* qoa_key_i32(const qoatable *t, qoaiter iter)
{
    assert(qoa_valid_i32(t, iter));
    return &t->keys[iter];
}

val_t* qoa_val_i32(const qoatable *t, qoaiter iter)
{
    assert(qoa_valid_i32(t, iter));
    return &t->vals[iter];
}

qoaiter qoa_get_i32(const qoatable *t, key_t key)
{
    const flag_t *flags = t->flags;
    const key_t *keys = t->keys;
    int k, i, last, step, mask = t->asize - 1;
    if (!t->asize)
        return 0; /* t->asize; */
    step = 0;
    k = qoa__hash_func(key);
    i = k & mask;
    last = i;
    while (
        qoa__islive(flags, i) &&
        (qoa__isdel(flags, i) || qoa__neq(keys[i], key))
    ) {
        i = (i + (++step)) & mask;
        if (i == last)
            return t->asize;
    }
    return qoa__iseither(flags, i) ? t->asize : i;
}

qoaiter qoa_find_i32(const qoatable *t, key_t key)
{
    return qoa_get_i32(t, key);
}

qoaiter qoa_end_i32(const qoatable *t)
{
    return t->asize;
}

void qoa_del_i32(qoatable *t, qoaiter iter)
{
    assert(t != NULL);
    if (iter != t->asize && qoa__islive(t->flags, iter)) {
        assert(t->size > 0);
        qoa__set_isdel_true(t->flags, iter);
        --t->size;
    }
}

int qoa_erase_i32(qoatable *t, key_t key)
{
    qoaiter iter = qoa_find_i32(t, key);
    if (iter == qoa_end_i32(t))
        return 0;
    qoa_del_i32(t, iter);
    return 1;
}

int qoa_isempty_i32(const qoatable *t)
{
    return t->size != 0;
}

/* Public Interface */
#define qoa_create(name)           qoa_create_##name()
#define qoa_init(name, t)          qoa_init_##name(t)
#define qoa_destroy(name, t)       qoa_destroy_##name(t)
#define qoa_size(name, t)          qoa_size_##name(t)
#define qoa_valid(name, t, iter)   qoa_valid_##name(t, iter)
#define qoa_exist(name, t, iter)   qoa_exist_##name(t, iter)
#define qoa_resize(name, t, asize) qoa_resize_##name(t, asize)
#define qoa_resize_fast(name, t, asize) qoa_resize_fast_##name(t, asize)
#define qoa_insert(name, t, key)   qoa_insert_##name(t, key)
#define qoa_put(name, t, key, ret) qoa_put_##name(t, key, ret)
#define qoa_key(name, t, iter)     qoa_key_##name(t, iter)
#define qoa_val(name, t, iter)     qoa_val_##name(t, iter)
#define qoa_get(name, t, key)      qoa_get_##name(t, key)
#define qoa_find(name, t, key)     qoa_find_##name(t, key)
#define qoa_end(name, t)           qoa_end_##name(t)
#define qoa_del(name, t, iter)     qoa_del_##name(t, iter)
#define qoa_erase(name, t, key)    qoa_erase_##name(t, key)
#define qoa_isempty(name, t)       qoa_isempty_##name(t)

#ifdef QOA_DEFINED_REALLOCARRAY
#undef reallocarray
#endif
#undef qoa_calloc
#undef qoa_free
#undef qoa_reallocarray
#undef qoa_freearray
#undef QOA_MIN_TABLE_SIZE
#undef qoa_max_load_factor
#undef flag_t
#undef key_t
#undef val_t
#undef qoa__hash_func
#undef qoa__equal
#undef qoa__eq
#undef qoa__neq
#undef qoa__fsize
#undef qoa__isempty
#undef qoa__isdel
#undef qoa__iseither
#undef qoa__islive
#undef qoa__set_isdel_false
#undef qoa__set_isempty_false
#undef qoa__set_isboth_false
#undef qoa__set_isdel_true
#undef qoa__swapkeys
#undef qoa__swapvals

#endif // QUAD_OPEN_ADDRESS__H_
