#ifndef QUAD_OPEN_ADDRESS__H_
#define QUAD_OPEN_ADDRESS__H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

/* --- Public API --- */
#define qoatable_t(name) qoatable__##name##_t
#define qoakey_t(name) qoakey_##name##_t
#define qoaval_t(name) qoaval_##name##_t
#define qoa_create(name) qoa_create_##name()
#define qoa_init(name, t) qoa_init_##name(t)
#define qoa_destroy(name, t) qoa_destroy_##name(t)
#define qoa_destroy2(name, t, dtor) qoa_destroy2_##name(t, dtor)
#define qoa_size(name, t) qoa_size_##name(t)
#define qoa_valid(name, t, iter) qoa_valid_##name(t, iter)
#define qoa_exist(name, t, iter) qoa_exist_##name(t, iter)
#define qoa_resize(name, t, asize) qoa_resize_##name(t, asize)
#define qoa_resize_fast(name, t, asize) qoa_resize_fast_##name(t, asize)
#define qoa_insert(name, t, key) qoa_insert_##name(t, key)
#define qoa_put(name, t, key, ret) qoa_put_##name(t, key, ret)
#define qoa_key(name, t, iter) qoa_key_##name(t, iter)
#define qoa_val(name, t, iter) qoa_val_##name(t, iter)
#define qoa_get(name, t, key) qoa_get_##name(t, key)
#define qoa_find(name, t, key) qoa_find_##name(t, key)
#define qoa_end(name, t) qoa_end_##name(t)
#define qoa_del(name, t, iter) qoa_del_##name(t, iter)
#define qoa_erase(name, t, key) qoa_erase_##name(t, key)
#define qoa_isempty(name, t) qoa_isempty_##name(t)

/* --- Type Creation API --- */

#define QOA_DECLARE(name, key_t, val_t)                                        \
    QOA__TYPES(name, key_t, val_t)                                             \
    QOA__PROTOS(name, qoatable_t(name), key_t, val_t)

#define QOA_INIT2(name, scope, key_t, val_t, hashfn, keyeq)                    \
    QOA__TYPES(name, key_t, val_t)                                             \
    QOA__IMPLS(name, scope, qoatable_t(name), key_t, val_t, hashfn, keyeq)

#define QOA_INIT(name, key_t, val_t, hashfn, keyeq)                            \
    QOA_INIT2(name, static inline, key_t, val_t, hashfn, keyeq)

/* define a table of int -> val_t */
#define QOA_INT_INIT(name, val_t, hashfn)                                      \
    QOA_INIT(name, int, val_t, hashfn, qoa_i32_eq)

typedef char *qoastr_t;
/* define a table of str -> val_t */
#define QOA_STR_INIT(name, val_t, hashfn)                                      \
    QOA_INIT(name, qoastr_t, val_t, hashfn, qoa_str_eq)

/* --- Common Hash Functions --- */

static inline int qoa_i32_hash_identity(int key)
{
    return key;
}

static inline int qoa_str_hash_X31(const char *s)
{
    int h = (int)*s;
    if (h)
        for (++s; *s; ++s)
            h = (h << 5) - h + (int)*s;
    return h;
}

static inline int qoa_i32_hash_Wang(int key)
{
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

/* --- Common Equality Functions --- */

static inline int qoa_i32_eq(int a, int b)
{
    return a == b;
}

static inline int qoa_str_eq(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

/* --- Implementation -- */

#ifndef reallocarray
// #ifndef SIZE_MAX
// #define SIZE_MAX UINTPTR_MAX
// #endif
// #define MUL_NO_OVERFLOW ((size_t)1 << (sizeof(size_t) * 4))
// void *reallocarray(void *optr, size_t nmemb, size_t size)
// {
//     if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && nmemb > 0 &&
//         SIZE_MAX / nmemb < size) {
//         errno = ENOMEM;
//         return NULL;
//     }
//     if (size == 0 || nmemb == 0)
//         return NULL;
//     return realloc(optr, size * nmemb);
// }
#define reallocarray(ptr, nmemb, size) realloc(ptr, (nmemb) * (size))
#endif

#define qoa_calloc(nmemb, size) calloc(nmemb, size)
#define qoa_free(ptr, size) free(ptr)
#define qoa_reallocarray(ptr, nmemb, size) reallocarray(ptr, nmemb, size)
#define qoa_freearray(ptr, nmemb, size) free(ptr)
#define QOA_MIN_TABLE_SIZE 4
#define qoa__max_load_factor(asize) ((int)(0.77 * (asize) + 0.5))
#define qoa__fsize(x) ((x) / sizeof(uint32_t))
#define qoa__isempty(flag, i) ((flag[i >> 4] >> ((i & 0xfU) << 1)) & 2)
#define qoa__isdel(flag, i) ((flag[i >> 4] >> ((i & 0xfU) << 1)) & 1)
#define qoa__iseither(flag, i) ((flag[i >> 4] >> ((i & 0xfU) << 1)) & 3)
#define qoa__islive(flag, i) (qoa__iseither(flag, i) == 0)
#define qoa__set_isdel_false(flag, i)                                          \
    (flag[i >> 4] &= ~(1ul << ((i & 0xfU) << 1)))
#define qoa__set_isempty_false(flag, i)                                        \
    (flag[i >> 4] &= ~(2ul << ((i & 0xfU) << 1)))
#define qoa__set_isboth_false(flag, i)                                         \
    (flag[i >> 4] &= ~(3ul << ((i & 0xfU) << 1)))
#define qoa__set_isdel_true(flag, i) (flag[i >> 4] |= 1ul << ((i & 0xfU) << 1))
#define qoa__swap(a, b, tmp)                                                   \
    {                                                                          \
        tmp = a;                                                               \
        a = b;                                                                 \
        b = tmp;                                                               \
    }
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

#ifndef NDEBUG
#define QOA_DEBUG(stmt) stmt
#else
#define QOA_DEBUG(stmt)                                                        \
    do {                                                                       \
    } while (0)
#endif

typedef int qoaiter;
enum
{
    QOA_ERROR = -1,
    QOA_PRESENT = 0,
    QOA_NEW = 1,
    QOA_DELETED = 2,
};

struct qoaresult_s
{
    qoaiter iter;
    int result;
};
typedef struct qoaresult_s qoaresult;

#define QOA__TYPES(name, key_t, val_t)                                         \
    typedef key_t qoakey_t(name);                                              \
    typedef val_t qoaval_t(name);                                              \
    struct qoatable__##name##_s                                                \
    {                                                                          \
        uint32_t *flags;                                                       \
        key_t *keys;                                                           \
        val_t *vals;                                                           \
        uint32_t size;                                                         \
        uint32_t asize;                                                        \
        uint32_t used;                                                         \
        uint32_t upbnd;                                                        \
    };                                                                         \
    typedef struct qoatable__##name##_s qoatable_t(name);

#define QOA__PROTOS(name, table_t, key_t, val_t)                               \
    extern table_t *qoa_create_##name();                                       \
    extern void qoa_init_##name(table_t *t);                                   \
    extern void qoa_destroy_##name(table_t *t);                                \
    extern void qoa_destroy2_##name(table_t *t);                               \
    extern int qoa_size_##name(const table_t *t);                              \
    extern int qoa_valid_##name(const table_t *t, qoaiter iter);               \
    extern int qoa_exist_##name(const table_t *t, qoaiter iter);               \
    extern int qoa_resize_fast_##name(table_t *t, int newasize);               \
    extern int qoa_resize_##name(table_t *t, int newasize);                    \
    extern qoaresult qoa_insert_##name(table_t *t, key_t key);                 \
    extern qoaiter qoa_put_##name(table_t *t, key_t key, int *res);            \
    extern const key_t *qoa_key_##name(const table_t *t, qoaiter iter);        \
    extern val_t *qoa_val_##name(const table_t *t, qoaiter iter);              \
    extern qoaiter qoa_get_##name(const table_t *t, key_t key);                \
    extern qoaiter qoa_find_##name(const table_t *t, key_t key);               \
    extern qoaiter qoa_end_##name(const table_t *t);                           \
    extern void qoa_del_##name(table_t *t, qoaiter iter);                      \
    extern int qoa_erase_##name(table_t *t, key_t key);                        \
    extern int qoa_isempty_##name(const table_t *t);

#define QOA__IMPLS(name, scope, table_t, key_t, val_t, qoa__hash, qoa__eq)     \
                                                                               \
    scope table_t *qoa_create_##name()                                         \
    {                                                                          \
        return (table_t *)qoa_calloc(1, sizeof(table_t));                      \
    }                                                                          \
                                                                               \
    scope void qoa_init_##name(table_t *t)                                     \
    {                                                                          \
        t->flags = NULL;                                                       \
        t->keys = NULL;                                                        \
        t->vals = NULL;                                                        \
        t->size = t->asize = t->used = t->upbnd = 0;                           \
    }                                                                          \
                                                                               \
    scope void qoa_destroy_##name(table_t *t)                                  \
    {                                                                          \
        if (t) {                                                               \
            qoa_freearray(t->flags, qoa__fsize(t->size), sizeof(uint32_t));    \
            qoa_freearray(t->keys, t->size, sizeof(key_t));                    \
            qoa_freearray(t->vals, t->size, sizeof(val_t));                    \
            QOA_DEBUG(qoa_init_##name(t));                                     \
            qoa_free(t, sizeof(table_t));                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    scope void qoa_destroy2_##name(table_t *t, void (*dtor)(key_t *, val_t *)) \
    {                                                                          \
        if (t) {                                                               \
            qoaiter i = 0;                                                     \
            for (i = 0; i < t->asize; ++i) {                                   \
                if (!qoa__islive(t->flags, i))                                 \
                    continue;                                                  \
                dtor(&t->keys[i], &t->vals[i]);                                \
            }                                                                  \
            qoa_destroy_##name(t);                                             \
        }                                                                      \
    }                                                                          \
                                                                               \
    scope int qoa_size_##name(const table_t *t) { return t->size; }            \
                                                                               \
    scope int qoa_valid_##name(const table_t *t, qoaiter iter)                 \
    {                                                                          \
        assert(t != NULL);                                                     \
        assert(0 <= iter && iter <= t->asize);                                 \
        return iter != t->asize && qoa__islive(t->flags, iter);                \
    }                                                                          \
                                                                               \
    scope int qoa_exist_##name(const table_t *t, qoaiter iter)                 \
    {                                                                          \
        return qoa_valid_##name(t, iter);                                      \
    }                                                                          \
                                                                               \
    scope int qoa_resize_fast_##name(table_t *t, int newasize)                 \
    {                                                                          \
        uint32_t *flags, *oldflags = t->flags;                                 \
        key_t key, tmpkey, *keys;                                              \
        val_t val, tmpval, *vals;                                              \
        int j, k, i, step, oldasize = t->asize, mask = newasize - 1;           \
        assert(newasize >= QOA_MIN_TABLE_SIZE);                                \
        assert((newasize & (newasize - 1)) == 0);                              \
        assert(t->size <= qoa__max_load_factor(newasize));                     \
        flags =                                                                \
          (uint32_t *)qoa_calloc(qoa__fsize(newasize), sizeof(uint32_t));      \
        keys = (key_t *)qoa_reallocarray(t->keys, newasize, sizeof(key_t));    \
        vals = (val_t *)qoa_reallocarray(t->vals, newasize, sizeof(val_t));    \
        if (!flags || !keys || !vals) {                                        \
            free(flags);                                                       \
            free(keys);                                                        \
            free(vals);                                                        \
            return -1;                                                         \
        }                                                                      \
        memset(flags, 0xaa, qoa__fsize(newasize) * sizeof(uint32_t));          \
        for (j = 0; j < oldasize; ++j) {                                       \
            if (!qoa__islive(oldflags, j))                                     \
                continue;                                                      \
            key = keys[j];                                                     \
            val = vals[j];                                                     \
            qoa__set_isdel_true(oldflags, j);                                  \
            for (;;) {                                                         \
                k = qoa__hash(key);                                            \
                i = k & mask;                                                  \
                step = 0;                                                      \
                while (!qoa__isempty(flags, i))                                \
                    i = (i + (++step)) & mask;                                 \
                qoa__set_isempty_false(flags, i);                              \
                if (i < oldasize && qoa__islive(oldflags, i)) {                \
                    qoa__swap(keys[i], key, tmpkey);                           \
                    qoa__swap(vals[i], val, tmpval);                           \
                    qoa__set_isdel_true(oldflags, i);                          \
                } else {                                                       \
                    keys[i] = key;                                             \
                    vals[i] = val;                                             \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
        t->flags = flags;                                                      \
        t->keys = keys;                                                        \
        t->vals = vals;                                                        \
        t->asize = newasize;                                                   \
        t->used = t->size;                                                     \
        t->upbnd = qoa__max_load_factor(newasize);                             \
        free(oldflags);                                                        \
        return 0;                                                              \
    }                                                                          \
                                                                               \
    scope int qoa_resize_##name(table_t *t, int newasize)                      \
    {                                                                          \
        newasize =                                                             \
          newasize >= QOA_MIN_TABLE_SIZE ? newasize : QOA_MIN_TABLE_SIZE;      \
        newasize = newasize >= t->upbnd ? newasize : t->upbnd;                 \
        newasize = qoa__rounduppow2(newasize);                                 \
        return qoa_resize_fast_##name(t, newasize);                            \
    }                                                                          \
                                                                               \
    scope qoaresult qoa_insert_##name(table_t *t, key_t key)                   \
    {                                                                          \
        qoaresult res;                                                         \
        uint32_t *flags;                                                       \
        key_t *keys;                                                           \
        int x, k, i, site, last, step, mask, asize = t->asize;                 \
        if (t->used >= t->upbnd) {                                             \
            int newasize = asize > 2 * t->size ? asize : 2 * asize;            \
            newasize =                                                         \
              newasize >= QOA_MIN_TABLE_SIZE ? newasize : QOA_MIN_TABLE_SIZE;  \
            if (qoa_resize_fast_##name(t, newasize) < 0) {                     \
                res.iter = asize;                                              \
                res.result = QOA_ERROR;                                        \
                return res;                                                    \
            }                                                                  \
            asize = t->asize;                                                  \
        }                                                                      \
        mask = asize - 1;                                                      \
        flags = t->flags;                                                      \
        keys = t->keys;                                                        \
        step = 0;                                                              \
        x = site = asize;                                                      \
        k = qoa__hash(key);                                                    \
        i = k & mask;                                                          \
        if (qoa__isempty(flags, i)) {                                          \
            x = i;                                                             \
        } else {                                                               \
            last = i;                                                          \
            while (qoa__islive(flags, i) &&                                    \
                   (qoa__isdel(flags, i) || !qoa__eq(keys[i], key))) {         \
                if (qoa__isdel(flags, i))                                      \
                    site = i;                                                  \
                i = (i + (++step)) & mask;                                     \
                if (i == last) {                                               \
                    x = site;                                                  \
                    break;                                                     \
                }                                                              \
            }                                                                  \
            if (x == asize) {                                                  \
                x = qoa__isempty(flags, i) && site != asize ? site : i;        \
            }                                                                  \
        }                                                                      \
        if (qoa__isempty(flags, x)) {                                          \
            keys[x] = key;                                                     \
            qoa__set_isboth_false(flags, x);                                   \
            ++t->size;                                                         \
            ++t->used;                                                         \
            res.iter = x;                                                      \
            res.result = QOA_NEW;                                              \
        } else if (qoa__isdel(flags, x)) {                                     \
            t->keys[x] = key;                                                  \
            qoa__set_isboth_false(flags, x);                                   \
            ++t->size;                                                         \
            res.iter = x;                                                      \
            res.result = QOA_DELETED;                                          \
        } else {                                                               \
            res.iter = x;                                                      \
            res.result = QOA_PRESENT;                                          \
        }                                                                      \
        return res;                                                            \
    }                                                                          \
                                                                               \
    scope qoaiter qoa_put_##name(table_t *t, key_t key, int *res)              \
    {                                                                          \
        qoaresult r = qoa_insert_##name(t, key);                               \
        *res = r.result;                                                       \
        return r.iter;                                                         \
    }                                                                          \
                                                                               \
    scope const key_t *qoa_key_##name(const table_t *t, qoaiter iter)          \
    {                                                                          \
        assert(qoa_valid_##name(t, iter));                                     \
        return &t->keys[iter];                                                 \
    }                                                                          \
                                                                               \
    scope val_t *qoa_val_##name(const table_t *t, qoaiter iter)                \
    {                                                                          \
        assert(qoa_valid_##name(t, iter));                                     \
        return &t->vals[iter];                                                 \
    }                                                                          \
                                                                               \
    scope qoaiter qoa_get_##name(const table_t *t, key_t key)                  \
    {                                                                          \
        const uint32_t *flags = t->flags;                                      \
        const key_t *keys = t->keys;                                           \
        int k, i, last, step, mask = t->asize - 1;                             \
        if (!t->asize)                                                         \
            return 0;                                                          \
        step = 0;                                                              \
        k = qoa__hash(key);                                                    \
        i = k & mask;                                                          \
        last = i;                                                              \
        while (qoa__islive(flags, i) &&                                        \
               (qoa__isdel(flags, i) || !qoa__eq(keys[i], key))) {             \
            i = (i + (++step)) & mask;                                         \
            if (i == last)                                                     \
                return t->asize;                                               \
        }                                                                      \
        return qoa__iseither(flags, i) ? t->asize : i;                         \
    }                                                                          \
                                                                               \
    scope qoaiter qoa_find_##name(const table_t *t, key_t key)                 \
    {                                                                          \
        return qoa_get_##name(t, key);                                         \
    }                                                                          \
                                                                               \
    scope qoaiter qoa_end_##name(const table_t *t) { return t->asize; }        \
                                                                               \
    scope void qoa_del_##name(table_t *t, qoaiter iter)                        \
    {                                                                          \
        assert(t != NULL);                                                     \
        if (iter != t->asize && qoa__islive(t->flags, iter)) {                 \
            assert(t->size > 0);                                               \
            qoa__set_isdel_true(t->flags, iter);                               \
            --t->size;                                                         \
        }                                                                      \
    }                                                                          \
                                                                               \
    scope int qoa_erase_##name(table_t *t, key_t key)                          \
    {                                                                          \
        qoaiter iter = qoa_find_##name(t, key);                                \
        if (iter == qoa_end_##name(t))                                         \
            return 0;                                                          \
        qoa_del_##name(t, iter);                                               \
        return 1;                                                              \
    }                                                                          \
                                                                               \
    scope int qoa_isempty_##name(const table_t *t) { return t->size != 0; }    \
                                                                               \
    struct qoa__empty_dummy_struct_##name                                      \
    {                                                                          \
    }

#endif // QUAD_OPEN_ADDRESS__H_
