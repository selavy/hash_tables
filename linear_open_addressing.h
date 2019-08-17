#pragma once

#include "utils.h"

typedef int loa_key_t;
typedef int loa_val_t;
typedef uint32_t loa_iter_t;
typedef uint32_t (*loa_hash_t)(const void* key, size_t len);
typedef int (*loa_cmp_t)(loa_key_t, loa_key_t);
// TODO: different interface than malloc/free?
// typedef void* (*loa_alloc_t)(size_t nmem, size_t size);
// typedef void  (*loa_dealloc_t)(void* mem, size_t size);
typedef void* (*loa_alloc_t)(size_t size);
typedef void (*loa_dealloc_t)(void* mem);

struct loa_result_t {
    loa_iter_t it;
    int        rc;
};
typedef struct loa_result_t loa_result_t;

struct loa_table {
    uint32_t size, asize, cutoff;
    loa_key_t* keys;
    loa_val_t* vals;
    uint8_t*   msks;
};
typedef struct loa_table loa_table;
loa_hash_t    _loa_hash = &murmur3_hash;
loa_cmp_t     _loa_cmp = &basic_int_eq;
loa_alloc_t   _loa_alloc = &malloc;
loa_dealloc_t _loa_dealloc = &free;
// TODO: tune
#define _LOA_LOAD_FACTOR 0.70

loa_table*   loa_create();
void         loa_destroy(loa_table* t);
int          loa_initialize(loa_table* t);
void         loa_finalize(loa_table* t);
loa_iter_t   loa_end(const loa_table* t);
loa_result_t loa_put(loa_table* t, loa_key_t key);
loa_iter_t   loa_get(const loa_table* t, loa_key_t key);
int          loa_size(const loa_table* t);
loa_key_t    loa_key(const loa_table* t, loa_iter_t i);
int          loa_resize(loa_table* t, int newsize);
#define      loa_val(t, i) t->vals[i]
int          _loa_alive(const uint8_t* msks, int i);
int          _loa_tombstone(const uint8_t* msks, int i);
void         _loa_set_alive(uint8_t* msks, int i);
int          _loa_resize_fast(loa_table* t, uint32_t newsize);

//
// Implementations
//

loa_table* loa_create() {
    loa_table* t = malloc(sizeof(*t));
    if (!t || loa_initialize(t) != OK) {
        free(t);
        return NULL;
    }
    return t;
}

void loa_destroy(loa_table* t) {
    loa_finalize(t);
    free(t);
}

int loa_initialize(loa_table* t) {
    uint32_t asize = 32;
	t->size = 0;
    t->cutoff = t->asize * _LOA_LOAD_FACTOR;
    t->keys = _loa_alloc(sizeof(t->keys[0]) * asize);
    t->vals = _loa_alloc(sizeof(t->vals[0]) * asize);
    t->msks = _loa_alloc(sizeof(t->msks[0]) * asize);
    if (!t->keys || !t->vals || !t->msks) {
        loa_destroy(t);
        return -EMEM;
    }
    t->asize = asize;
    return OK;
}

void loa_finalize(loa_table* t) {
    if (t && (t->keys || t->vals || t->msks)) {
        _loa_dealloc(t->keys); t->keys = NULL;
        _loa_dealloc(t->vals); t->vals = NULL;
        _loa_dealloc(t->msks); t->msks = NULL;
        t->size = t->asize = 0;
    }
}

loa_iter_t loa_end(const loa_table* t) {
    return t->asize;
}

loa_result_t loa_put(loa_table* t, loa_key_t key) {
    loa_result_t rv;

    if (t->size >= t->cutoff) {
        if (_loa_resize_fast(t, 2*t->asize) != 0) {
            rv.rc = -EMEM;
            return rv;
        }
    }

    loa_key_t* keys = t->keys;
    uint8_t*   msks = t->msks;
    uint32_t m = t->asize - 1;
    uint32_t h = _loa_hash(&key, sizeof(key));
    uint32_t i = h & m;
    for (;;) {
        if (!_loa_alive(msks, i)) {
            _loa_set_alive(msks, i);
            keys[i] = key;
            t->size++;
            rv.it = i;
            rv.rc = 0;
            return rv;
        } else if (_loa_cmp(key, keys[i]) == 0) {
            rv.it = i;
            rv.rc = 1;
            return rv;
        }
        i = (i + 1) & m;
    }
    __builtin_unreachable();
}

loa_iter_t loa_get(const loa_table* t, loa_key_t key) {
    const loa_key_t* keys = t->keys;
    const uint8_t*   msks = t->msks;
    uint32_t m = t->asize - 1;
    uint32_t h = _loa_hash(&key, sizeof(key));
    uint32_t i = h & m;
    for (;;) {
        if (_loa_alive(msks, i)) {
            if (_loa_cmp(key, keys[i]) == 0) {
                return i;
            }
        } else if (!_loa_tombstone(msks, i)) {
            break;
        }
    }
    return loa_end(t);
}

int loa_size(const loa_table* t) {
    return t->size;
}

loa_key_t loa_key(const loa_table* t, loa_iter_t i) {
    assert(i != loa_end(t));
    return t->keys[i];
}

int loa_resize(loa_table* t, int newsize) {
    newsize = newsize < t->size + 1 ? t->size + 1: newsize;
    if ((newsize & (newsize - 1)) != 0) {
        newsize = kroundup32(newsize);
    }
    return _loa_resize_fast(t, newsize);
}

int _loa_alive(const uint8_t* msks, int i) {
    uint8_t byte = msks[i / 4];
    uint8_t index = 2*(i % 4);
    return (byte & (1u << index)) != 0;
}

int _loa_tombstone(const uint8_t* msks, int i) {
    uint8_t byte = msks[i / 4];
    uint8_t index = 2*(i % 4) + 1;
    return (byte & (1u << index)) != 0;
}

void _loa_set_alive(uint8_t* msks, int i) {
    msks[i / 4] &= ~(1u << (2*(i % 4) + 1));
    msks[i / 4] |=  (1u << (2*(i % 4)));
}

int _loa_resize_fast(loa_table* t, uint32_t newsize) {
    printf("resizing... (%u) %u -> %u\n", t->size, t->asize, newsize);
    assert((newsize & (newsize - 1)) == 0);
    assert(newsize > t->size);
    uint32_t i, h, m = newsize - 1, oldsize = t->asize;
    loa_key_t* keys, *okeys = t->keys;
    loa_val_t* vals, *ovals = t->vals;
    uint8_t*   msks, *omsks = t->msks;
    keys = _loa_alloc(sizeof(keys[0]) * newsize);
    vals = _loa_alloc(sizeof(vals[0]) * newsize);
    msks = _loa_alloc(sizeof(msks[0]) * newsize);
    if (!keys || !vals || !msks) {
        _loa_dealloc(keys);
        _loa_dealloc(vals);
        _loa_dealloc(msks);
        return -EMEM;
    }
    for (i = 0; i < oldsize; ++i) {
        if (!_loa_alive(omsks, i))
            continue;
        h = _loa_hash(&okeys[i], sizeof(okeys[i])) & m;
        for (;;) {
            if (!_loa_alive(msks, h)) {
                _loa_set_alive(msks, h);
                keys[h] = okeys[i];
                vals[h] = ovals[i];
                break;
            }
            h = (h + 1) & m;
        }
    }
    t->keys = keys;
    t->vals = vals;
    t->msks = msks;
    t->asize = newsize;
    t->cutoff = t->asize * _LOA_LOAD_FACTOR;
    free(okeys);
    free(ovals);
    free(omsks);
    return 0;
}

