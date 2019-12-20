#pragma once

#include "utils.h"

typedef uint32_t qoa_iter_t;
typedef struct qoa_result_t {
    qoa_iter_t it;
    int        rc;
} qoa_result_t;

#define qoa_key(t, i) (t)->keys[i]
#define qoa_val(t, i) (t)->vals[i]
#define qoa_size(t)   (t)->size
#define qoa_end(t)    (t)->asize
#define qoa_key_t(name)         qoa_ ## name ## _key_t
#define qoa_val_t(name)         qoa_ ## name ## _val_t
#define qoa_table_t(name)       qoa_ ## name ## _table
#define qoa_create(name)        qoa_ ## name ## _create()
#define qoa_destroy(name, t)    qoa_ ## name ## _destroy(t)
#define qoa_initialize(name, t) qoa_ ## name ## _initialize(t)
#define qoa_finalize(name, t)   qoa_ ## name ## _finalize(t)
#define qoa_put(name, t, k)     qoa_ ## name ## _put(t, k)
#define qoa_get(name, t, k)     qoa_ ## name ## _get(t, k)
#define qoa_next(name, t, k)    qoa_ ## name ## _next(t, k)
#define qoa_begin(name, t)      qoa_next(name, t, 0)
#define qoa_exists(t, k)        qoa__live(t->msks, k)
#define qoa_resize(name, t, newsize) qoa_ ## name ##_resize(t, newsize)
#define qoa_resize_fast(name, t, newsize) qoa_ ## name ## _resize_fast(t, newsize)

#define QOA__LOAD_FACTOR 0.7
#define qoa__live(ms, i) ((ms[(i)/4] & (1u << (2*((i)%4)    ))) != 0)
#define qoa__tomb(ms, i) ((ms[(i)/4] & (1u << (2*((i)%4) + 1))) != 0)
#define qoa__animate(ms, i) do {             \
    ms[(i)/4] &= ~(1u << (2*((i) % 4) + 1)); \
    ms[(i)/4] |=  (1u << (2*((i) % 4)    )); \
} while (0)
#define qoa__next_index(i) (2*(i)+1)


#define QOA__TYPES(name, table_t, key_t, val_t) \
    typedef key_t qoa_key_t(name);              \
    typedef val_t qoa_val_t(name);              \
    typedef struct table_t {                    \
        uint32_t size, asize, cutoff;           \
        qoa_key_t(name)*  keys;                 \
        qoa_val_t(name)*  vals;                 \
        uint8_t* msks;                          \
    } table_t;


#define QOA__PROTOS_EX(ns, table_t, key_t, val_t)                 \
    table_t*     ns ## create();                                  \
    void         ns ## destroy();                                 \
    int          ns ## initialize();                              \
    void         ns ## finalize();                                \
    qoa_iter_t   ns ## next(table_t* t, qoa_iter_t k);            \
    qoa_result_t ns ## put(table_t* t, key_t key);                \
    qoa_iter_t   ns ## get(const table_t* t, key_t key);          \
    int          ns ## resize(table_t* t, int newsize);           \
    int          ns ## resize_fast(table_t* t, uint32_t newsize);


#define QOA__IMPLS_EX(ns, table_t, key_t, val_t, hashfn, cmpfn, alloc, dealloc) \
    void ns ## finalize(table_t* t) {                                           \
        if (t && (t->keys || t->vals || t->msks)) {                             \
            dealloc(t->keys); t->keys = NULL;                                   \
            dealloc(t->vals); t->vals = NULL;                                   \
            dealloc(t->msks); t->msks = NULL;                                   \
            t->size = t->asize = 0;                                             \
        }                                                                       \
    }                                                                           \
                                                                                \
    int ns ## initialize(table_t* t) {                                          \
        uint32_t asize = 32;                                                    \
        t->size = 0;                                                            \
        t->cutoff = asize * QOA__LOAD_FACTOR;                                   \
        t->keys = alloc(sizeof(t->keys[0]) * asize);                            \
        t->vals = alloc(sizeof(t->vals[0]) * asize);                            \
        t->msks = alloc(sizeof(t->msks[0]) * asize);                            \
        if (!t->keys || !t->vals || !t->msks) {                                 \
            ns ## finalize(t); return -EMEM;                                    \
        }                                                                       \
        memset(t->msks, 0, sizeof(t->msks[0]) * asize);                         \
        t->asize = asize;                                                       \
        return OK;                                                              \
    }                                                                           \
                                                                                \
    table_t* ns ## create() {                                                   \
        table_t* t = malloc(sizeof(*t));                                        \
        if (!t || ns ## initialize(t) != OK) {                                  \
            free(t); return NULL;                                               \
        }                                                                       \
        return t;                                                               \
    }                                                                           \
                                                                                \
    void ns ## destroy(table_t* t) {                                            \
        ns ## finalize(t);                                                      \
        free(t);                                                                \
    }                                                                           \
                                                                                \
    qoa_iter_t ns ## next(table_t* t, qoa_iter_t k) {                           \
        do { ++k; } while (k != qoa_end(t) && !qoa_exists(t, k));               \
        return k;                                                               \
    }                                                                           \
                                                                                \
    int ns ## resize_fast(table_t* t, uint32_t newsize) {                       \
        assert((newsize & (newsize - 1)) == 0);                                 \
        assert(newsize > t->size);                                              \
        uint32_t i, h, m = newsize - 1, oldsize = t->asize;                     \
        key_t *keys, *okeys = t->keys;                                          \
        val_t *vals, *ovals = t->vals;                                          \
        uint8_t*   msks, *omsks = t->msks;                                      \
        keys = alloc(sizeof(keys[0]) * newsize);                                \
        vals = alloc(sizeof(vals[0]) * newsize);                                \
        msks = alloc(sizeof(msks[0]) * newsize);                                \
        if (!keys || !vals || !msks) {                                          \
            dealloc(keys); dealloc(vals); dealloc(msks);                        \
            return -EMEM;                                                       \
        }                                                                       \
        memset(msks, 0, sizeof(msks[0]) * newsize);                             \
        for (i = 0; i < oldsize; ++i) {                                         \
            if (!qoa__live(omsks, i))                                           \
                continue;                                                       \
            h = hashfn(&okeys[i], sizeof(okeys[i])) & m;                        \
            for (;;) {                                                          \
                if (!qoa__live(msks, h)) {                                      \
                    qoa__animate(msks, h);                                      \
                    keys[h] = okeys[i];                                         \
                    vals[h] = ovals[i];                                         \
                    break;                                                      \
                }                                                               \
                h = qoa__next_index(h) & m;                                     \
            }                                                                   \
        }                                                                       \
        t->keys = keys; t->vals = vals; t->msks = msks;                         \
        t->asize = newsize;                                                     \
        t->cutoff = t->asize * QOA__LOAD_FACTOR;                                \
        free(okeys); free(ovals); free(omsks);                                  \
        return 0;                                                               \
    }                                                                           \
                                                                                \
    qoa_result_t ns ## put(table_t* t, key_t key) {                             \
        qoa_result_t rv;                                                        \
        if (t->size >= t->cutoff) {                                             \
            if (ns ## resize_fast(t, 2*t->asize) != 0) {                        \
                rv.rc = -EMEM;                                                  \
                return rv;                                                      \
            }                                                                   \
        }                                                                       \
        key_t* keys = t->keys;                                                  \
        uint8_t*   msks = t->msks;                                              \
        uint32_t m = t->asize - 1;                                              \
        uint32_t h = hashfn(&key, sizeof(key));                                 \
        uint32_t i = h & m;                                                     \
        for (;;) {                                                              \
            if (!qoa__live(msks, i)) {                                          \
                qoa__animate(msks, i);                                          \
                keys[i] = key;                                                  \
                t->size++;                                                      \
                rv.it = i;                                                      \
                rv.rc = 0;                                                      \
                return rv;                                                      \
            } else if (cmpfn(key, keys[i]) == 0) {                              \
                rv.it = i;                                                      \
                rv.rc = 1;                                                      \
                return rv;                                                      \
            }                                                                   \
            i = (i + 1) & m;                                                    \
        }                                                                       \
        __builtin_unreachable();                                                \
    }                                                                           \
                                                                                \
    qoa_iter_t ns ## get(const table_t* t, key_t key) {                         \
        const key_t* keys = t->keys;                                            \
        const uint8_t* msks = t->msks;                                          \
        uint32_t m = t->asize - 1;                                              \
        uint32_t h = hashfn(&key, sizeof(key));                                 \
        uint32_t i = h & m;                                                     \
        for (;;) {                                                              \
            if (qoa__live(msks, i)) {                                           \
                if (cmpfn(key, keys[i]) == 0) {                                 \
                    return i;                                                   \
                }                                                               \
            } else if (!qoa__tomb(msks, i)) {                                   \
                break;                                                          \
            }                                                                   \
        }                                                                       \
        return qoa_end(t);                                                      \
    }                                                                           \
                                                                                \
    int ns ## resize(table_t* t, int newsize) {                                 \
        newsize = newsize < t->size + 1 ? t->size + 1: newsize;                 \
        if ((newsize & (newsize - 1)) != 0) {                                   \
            newsize = kroundup32(newsize);                                      \
        }                                                                       \
        return ns ## resize_fast(t, newsize);                                   \
    }


#define QOA__IMPLS(name, hash, compfn, alloc, dealloc) \
    QOA__IMPLS_EX(                                     \
        qoa_##name##_,                                 \
        qoa_table_t(name),                             \
        qoa_key_t(name),                               \
        qoa_val_t(name),                               \
        hash,                                          \
        compfn,                                        \
        alloc,                                         \
        dealloc                                        \
    )

#define QOA__PROTOS(name) \
    QOA__PROTOS_EX(qoa_##name##_, qoa_table_t(name), qoa_key_t(name), qoa_val_t(name))

#define QOA_TABLE_INIT(name, key, val, hash, compfn, alloc, dealloc) \
    QOA__TYPES(name, qoa_table_t(name), key, val)                    \
    QOA__IMPLS(name, hash, compfn, alloc, dealloc)

#define QOA_TABLE(name, key, val) \
    QOA_TABLE_INIT(name, key, val, murmur3_hash, basic_int_eq, malloc, free)
