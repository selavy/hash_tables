#pragma once

#include "utils.h"

typedef uint32_t loa_iter_t;
typedef struct loa_result_t {
    loa_iter_t it;
    int        rc;
} loa_result_t;

#define loa_key(t, i) (t)->keys[i]
#define loa_val(t, i) (t)->vals[i]
#define loa_end(t)    (t)->asize
#define loa_size(t)   (t)->size
#define loa_table(name)         loa_ ## name ## _table
#define loa_create(name)        loa_ ## name ## _create()
#define loa_destroy(name, t)    loa_ ## name ## _destroy(t)
#define loa_initialize(name, t) loa_ ## name ## _initialize(t)
#define loa_finalize(name, t)   loa_ ## name ## _finalize(t)
#define loa_put(name, t, k)     loa_ ## name ## _put(t, k)
#define loa_get(name, t, k)     loa_ ## name ## _get(t, k)
#define loa_resize(name, t, newsize) loa_ ## name ##_resize(t, newsize)
#define loa_resize_fast(name, t, newsize) loa_ ## name ## _resize_fast(t, newsize)

#define LOA__LOAD_FACTOR 0.7
#define loa__live(ms, i) ((ms[(i)/4] & (1u << (2*((i)%4)    ))) != 0)
#define loa__tomb(ms, i) ((ms[(i)/4] & (1u << (2*((i)%4) + 1))) != 0)
#define loa__animate(ms, i) do {             \
    ms[(i)/4] &= ~(1u << (2*((i) % 4) + 1)); \
    ms[(i)/4] |=  (1u << (2*((i) % 4)    )); \
} while (0)

#define LOA__TYPES(table_t, qkey_t, qval_t) \
    typedef struct table_t {                \
        uint32_t size, asize, cutoff;       \
        qkey_t*  keys;                      \
        qval_t*  vals;                      \
        uint8_t* msks;                      \
    } table_t;

#define LOA__PROTOS(ns, table_t, qkey_t, qval_t)          \
    table_t*     ns ## create();                          \
    void         ns ## destroy();                         \
    int          ns ## initialize();                      \
    void         ns ## finalize();                        \
    loa_iter_t   ns ## end(const table_t* t);             \
    loa_result_t ns ## put(table_t* t, qkey_t key);       \
    loa_iter_t   ns ## get(const table_t* t, qkey_t key); \
    int          ns ## resize(table_t* t, int newsize);   \
    int          ns ## resize_fast(table_t* t, uint32_t newsize);

#define LOA__IMPLS(ns, table_t, key_t, val_t, hashfn, cmpfn, alloc, dealloc) \
    void ns ## finalize(table_t* t) {                                        \
        if (t && (t->keys || t->vals || t->msks)) {                          \
            dealloc(t->keys); t->keys = NULL;                                \
            dealloc(t->vals); t->vals = NULL;                                \
            dealloc(t->msks); t->msks = NULL;                                \
            t->size = t->asize = 0;                                          \
        }                                                                    \
    }                                                                        \
                                                                             \
    int ns ## initialize(table_t* t) {                                       \
        uint32_t asize = 32;                                                 \
        t->size = 0;                                                         \
        t->cutoff = asize * LOA__LOAD_FACTOR;                                \
        t->keys = alloc(sizeof(t->keys[0]) * asize);                         \
        t->vals = alloc(sizeof(t->vals[0]) * asize);                         \
        t->msks = alloc(sizeof(t->msks[0]) * asize);                         \
        if (!t->keys || !t->vals || !t->msks) {                              \
            ns ## finalize(t); return -EMEM;                                 \
        }                                                                    \
        memset(t->msks, 0, sizeof(t->msks[0]) * asize);                      \
        t->asize = asize;                                                    \
        return OK;                                                           \
    }                                                                        \
                                                                             \
    table_t* ns ## create() {                                                \
        table_t* t = malloc(sizeof(*t));                                     \
        if (!t || ns ## initialize(t) != OK) {                               \
            free(t); return NULL;                                            \
        }                                                                    \
        return t;                                                            \
    }                                                                        \
                                                                             \
    void ns ## destroy(table_t* t) {                                         \
        ns ## finalize(t);                                                   \
        free(t);                                                             \
    }                                                                        \
                                                                             \
    int ns ## resize_fast(table_t* t, uint32_t newsize) {                    \
        assert((newsize & (newsize - 1)) == 0);                              \
        assert(newsize > t->size);                                           \
        uint32_t i, h, m = newsize - 1, oldsize = t->asize;                  \
        key_t* keys, *okeys = t->keys;                                       \
        val_t* vals, *ovals = t->vals;                                       \
        uint8_t*   msks, *omsks = t->msks;                                   \
        keys = alloc(sizeof(keys[0]) * newsize);                             \
        vals = alloc(sizeof(vals[0]) * newsize);                             \
        msks = alloc(sizeof(msks[0]) * newsize);                             \
        if (!keys || !vals || !msks) {                                       \
            dealloc(keys); dealloc(vals); dealloc(msks);                     \
            return -EMEM;                                                    \
        }                                                                    \
        memset(msks, 0, sizeof(msks[0]) * newsize);                          \
        for (i = 0; i < oldsize; ++i) {                                      \
            if (!loa__live(omsks, i))                                        \
                continue;                                                    \
            h = hashfn(&okeys[i], sizeof(okeys[i])) & m;                     \
            for (;;) {                                                       \
                if (!loa__live(msks, h)) {                                   \
                    loa__animate(msks, h);                                   \
                    keys[h] = okeys[i];                                      \
                    vals[h] = ovals[i];                                      \
                    break;                                                   \
                }                                                            \
                h = (h + 1) & m;                                             \
            }                                                                \
        }                                                                    \
        t->keys = keys; t->vals = vals; t->msks = msks;                      \
        t->asize = newsize;                                                  \
        t->cutoff = t->asize * LOA__LOAD_FACTOR;                             \
        free(okeys); free(ovals); free(omsks);                               \
        return 0;                                                            \
    }                                                                        \
                                                                             \
    loa_result_t ns ## put(table_t* t, key_t key) {                          \
        loa_result_t rv;                                                     \
        if (t->size >= t->cutoff) {                                          \
            if (ns ## resize_fast(t, 2*t->asize) != 0) {                     \
                rv.rc = -EMEM;                                               \
                return rv;                                                   \
            }                                                                \
        }                                                                    \
        key_t* keys = t->keys;                                               \
        uint8_t*   msks = t->msks;                                           \
        uint32_t m = t->asize - 1;                                           \
        uint32_t h = hashfn(&key, sizeof(key));                              \
        uint32_t i = h & m;                                                  \
        for (;;) {                                                           \
            if (!loa__live(msks, i)) {                                       \
                loa__animate(msks, i);                                       \
                keys[i] = key;                                               \
                t->size++;                                                   \
                rv.it = i;                                                   \
                rv.rc = 0;                                                   \
                return rv;                                                   \
            } else if (cmpfn(key, keys[i]) == 0) {                           \
                rv.it = i;                                                   \
                rv.rc = 1;                                                   \
                return rv;                                                   \
            }                                                                \
            i = (i + 1) & m;                                                 \
        }                                                                    \
        __builtin_unreachable();                                             \
    }                                                                        \
                                                                             \
    loa_iter_t ns ## get(const table_t* t, key_t key) {                      \
        const key_t* keys = t->keys;                                         \
        const uint8_t*   msks = t->msks;                                     \
        uint32_t m = t->asize - 1;                                           \
        uint32_t h = hashfn(&key, sizeof(key));                              \
        uint32_t i = h & m;                                                  \
        for (;;) {                                                           \
            if (loa__live(msks, i)) {                                        \
                if (cmpfn(key, keys[i]) == 0) {                              \
                    return i;                                                \
                }                                                            \
            } else if (!loa__tomb(msks, i)) {                                \
                break;                                                       \
            }                                                                \
        }                                                                    \
        return loa_end(t);                                                   \
    }                                                                        \
                                                                             \
    int ns ## resize(table_t* t, int newsize) {                              \
        newsize = newsize < t->size + 1 ? t->size + 1: newsize;              \
        if ((newsize & (newsize - 1)) != 0) {                                \
            newsize = kroundup32(newsize);                                   \
        }                                                                    \
        return ns ## resize_fast(t, newsize);                                \
    }


#define LOA_TABLE_INIT(name, key, val, hash, compfn, alloc, dealloc) \
    LOA__TYPES(loa_##name##_table, key, val)                         \
    LOA__IMPLS(loa_##name##_, loa_##name##_table, key, val, hash, compfn, alloc, dealloc)

#define LOA_TABLE(name, key, val) LOA_TABLE_INIT(name, key, val, murmur3_hash, basic_int_eq, malloc, free)
