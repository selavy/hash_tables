#ifndef LOATABLE__H_
#define LOATABLE__H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* --- User Defines --- */

#define key_t int
#define val_t int

static inline int loa_hash_int(int x)
{
    return x;
}
static inline int loa_eq_int(int a, int b)
{
    return a == b;
}

/* --- Implementation --- */

typedef uint32_t flg_t;
enum
{
    LOA_ERROR = -1,
    LOA_PRESENT = 0,
    LOA_INSERTED = 1,
};
struct loatable_s
{
    flg_t *flgs;
    key_t *keys;
    val_t *vals;
    uint32_t size, asize, used, ubnd;
};
typedef struct loatable_s loatable;
typedef int loaiter;
struct loaresult_s
{
    loaiter iter;
    int result;
};
typedef struct loaresult_s loaresult;

#define loahash(x) loa_hash_int(x)
#define loaeq(a, b) loa_eq_int(a, b)
#ifndef reallocarray
#define reallocarray(ptr, nmemb, size) realloc(ptr, (nmemb) * (size))
#endif
#define loacalloc(nmemb, size) calloc(nmemb, size)
#define loafree(ptr, size) free(ptr)
#define loareallocarray(ptr, nmemb, size) reallocarray(ptr, nmemb, size)
#define loafreearray(ptr, nmemb, size) free(ptr)
#define loaswap(x, y, t)                                                       \
    do {                                                                       \
        t = x;                                                                 \
        x = y;                                                                 \
        y = t;                                                                 \
    } while (0)
#define LOAINLINE static inline

const static int LOA_MINSIZE = 4;

int loa_maxloadfactor(int asize)
{
    return 0.77 * asize + 0.5;
}
int loa_fsize(int asize)
{
    return asize / sizeof(flg_t);
}
uint32_t loa_flgbits(const flg_t *flgs, uint32_t i)
{
    // uint32_t bits_per_byte = 8;
    // uint32_t bits_per_entry = 2;
    // uint32_t stride = (bits_per_byte / bits_per_entry)*sizeof(flg_t);
    // flg_t    flag   = flgs[i / stride];
    // uint32_t shift  = 2u*(i % stride);
    // return (flag >> shift);

    // _Static_assert(sizeof(flg_t) == 4, "");
    return flgs[i >> 4] >> ((i & 0xFu) << 1u);
}
int loa_islive(const flg_t *flgs, int i)
{
    return (loa_flgbits(flgs, i) & 0x1u) != 0;
}
int loa_istomb(const flg_t *flgs, int i)
{
    return (loa_flgbits(flgs, i) & 0x2u) != 0;
}
int loa_isdead(const flg_t *flgs, int i)
{
    return (loa_flgbits(flgs, i) & 0x3u) == 0;
}
void loa_setlive(flg_t *flgs, int i)
{
    // uint32_t bits_per_byte = 8;
    // uint32_t bits_per_entry = 4;
    // uint32_t stride = (bits_per_byte / bits_per_entry)*sizeof(flg_t);
    // flg_t* flag = &flgs[i / stride];
    // uint32_t shift = 2u*(i % stride);

    // _Static_assert(8 / 2 * sizeof(flg_t) == 16, "");
    flg_t *flag = &flgs[i >> 4];
    uint32_t shift = (i & 0xFu) << 1u;
    *flag &= ~(0x3u << shift); /* clear flags */
    *flag |= (0x1u << shift);  /* set islive  */
}
void loa_settomb(flg_t *flgs, int i)
{
    // _Static_assert(8 / 2 * sizeof(flg_t) == 16, "");
    flg_t *flag = &flgs[i >> 4];
    uint32_t shift = (i & 0xFu) << 1u;
    *flag &= ~(0x3u << shift); /* clear flags */
    *flag |= (0x2u << shift);  /* set istomb  */
}
void loa_setdead(flg_t *flgs, int i)
{
    // _Static_assert(8 / 2 * sizeof(flg_t) == 16, "");
    flg_t *flag = &flgs[i >> 4];
    uint32_t shift = (i & 0xFu) << 1u;
    *flag &= ~(0x3u << shift); /* clear flags */
}
uint32_t loa_rounduppow2(uint32_t x)
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

loatable *loacreate()
{
    return (loatable *)loacalloc(1, sizeof(loatable));
}

void loainit(loatable *t)
{
    assert(t != NULL);
    t->flgs = NULL;
    t->keys = NULL;
    t->vals = NULL;
    t->size = t->asize = t->used = t->ubnd = 0;
}

void loaclear(loatable *t)
{
    loafreearray(t->flgs, loa_fsize(t->asize), sizeof(flg_t));
    loafreearray(t->keys, t->asize, sizeof(key_t));
    loafreearray(t->vals, t->asize, sizeof(val_t));
    loainit(t);
}

void loadestroy(loatable *t)
{
    if (t) {
        loaclear(t);
        loafree(t, sizeof(t));
    }
}

int loaresizefast(loatable *t, int newasize)
{
    flg_t *flgs, *oldflgs = t->flgs;
    key_t key, tmpkey, *keys;
    val_t val, tmpval, *vals;
    int i, j, mask, oldasize = t->asize;
    newasize = newasize >= LOA_MINSIZE ? newasize : LOA_MINSIZE;
    assert((newasize & (newasize - 1)) == 0);
    assert(newasize >= LOA_MINSIZE);
    assert(t->size <= loa_maxloadfactor(newasize));
    flgs = (flg_t *)loacalloc(loa_fsize(newasize), sizeof(flg_t));
    keys = (key_t *)loareallocarray(t->keys, newasize, sizeof(key_t));
    vals = (val_t *)loareallocarray(t->vals, newasize, sizeof(val_t));
    if (!flgs || !keys || !vals) {
        free(flgs);
        free(keys);
        free(vals);
        return -1;
    }
    mask = newasize - 1;
    for (j = 0; j < oldasize; ++j) {
        if (!loa_islive(oldflgs, j))
            continue;
        key = keys[j];
        val = vals[j];
        loa_settomb(oldflgs, j);
        for (;;) {
            i = loahash(key) & mask;
            while (!loa_isdead(flgs, i))
                i = (i + 1) & mask;
            loa_setlive(flgs, i);
            if (i < oldasize && loa_islive(oldflgs, i)) {
                loaswap(key, keys[i], tmpkey);
                loaswap(val, vals[i], tmpval);
                loa_settomb(oldflgs, i);
            } else {
                keys[i] = key;
                vals[i] = val;
                break;
            }
        }
    }
    t->flgs = flgs;
    t->keys = keys;
    t->vals = vals;
    t->asize = newasize;
    t->used = t->size;
    t->ubnd = loa_maxloadfactor(t->asize);
    free(oldflgs);
    return 0;
}

int loaresize(loatable *t, int newasize)
{
    newasize = loa_maxloadfactor(newasize) >= t->size ? newasize
                                                      : (newasize / 0.77 + 0.5);
    newasize = loa_rounduppow2(newasize);
    return loaresizefast(t, newasize);
}

loaresult loainsert(loatable *t, key_t key)
{
    loaresult res;
    flg_t *flgs;
    key_t *keys;
    int mask;
    if (t->used >= t->ubnd) {
        if (loaresizefast(t, 2 * t->asize) < 0) {
            res.iter = t->asize;
            res.result = LOA_ERROR;
            return res;
        }
    }
    mask = t->asize - 1;
    flgs = t->flgs;
    keys = t->keys;
    res.iter = loahash(key) & mask;
    for (;;) {
        if (loa_istomb(flgs, res.iter)) {
            /* TODO: special tomb -> live function */
            loa_setlive(flgs, res.iter);
            keys[res.iter] = key;
            res.result = LOA_INSERTED;
            ++t->size;
            return res;
        } else if (loa_isdead(flgs, res.iter)) {
            /* TODO: special dead -> live function */
            loa_setlive(flgs, res.iter);
            keys[res.iter] = key;
            res.result = LOA_INSERTED;
            ++t->size;
            ++t->used;
            return res;
        } else if (loaeq(key, keys[res.iter])) {
            res.result = LOA_PRESENT;
            return res;
        }
        res.iter = (res.iter + 1) & mask;
    }
}

const key_t *loakey(loatable *t, loaiter it)
{
    return &t->keys[it];
}

val_t *loaval(loatable *t, loaiter it)
{
    return &t->vals[it];
}

loaiter loafind(const loatable *t, key_t key)
{
    const flg_t *flgs = t->flgs;
    const key_t *keys = t->keys;
    if (!t->asize)
        return 0;
    int mask = t->asize - 1;
    loaiter i = loahash(key) & mask;
    for (;;) {
        if (loa_isdead(flgs, i))
            return mask + 1;
        else if (loa_islive(flgs, i) && loaeq(keys[i], key))
            return i;
        i = (i + 1) & mask;
    }
}

loaiter loaget(const loatable *t, key_t key)
{
    return loafind(t, key);
}

loaiter loaend(const loatable *t)
{
    return t->asize;
}

void loadel(loatable *t, loaiter iter)
{
    assert(t != NULL);
    if (iter != t->asize && loa_islive(t->flgs, iter)) {
        assert(t->size > 0);
        loa_settomb(t->flgs, iter);
        --t->size;
    }
}

int loaerase(loatable *t, key_t key)
{
    loaiter iter = loafind(t, key);
    if (iter == loaend(t))
        return 0;
    loadel(t, iter);
    return 1;
}

int loasize(const loatable *t)
{
    return t->size;
}

#endif /* LOATABLE__H_ */
