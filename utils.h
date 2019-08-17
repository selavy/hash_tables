#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

#ifndef OK
#define OK 0
#endif

#ifndef EMEM
#define EMEM 1
#endif

int basic_int_eq(int a, int b) {
    return a == b ? 0 : 1;
}

#include "MurmurHash3.h"

#ifndef BIG_CONSTANT
#define BIG_CONSTANT(x) (x##LLU)
#endif

uint32_t murmur3_hash(const void* key, size_t len) {
    uint32_t out;
    MurmurHash3_x86_32(key, len, /*seed*/0x42, &out);
    return out;
}

// taken from https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
void FNV ( const void * key, int len, uint32_t seed, void * out )
{
    unsigned int h = seed;

    const uint8_t * data = (const uint8_t*)key;

    h ^= BIG_CONSTANT(2166136261);

    for(int i = 0; i < len; i++)
    {
        h ^= data[i];
        h *= 16777619;
    }

    *(uint32_t*)out = h;
}

uint32_t fnv_hash(const void *key, size_t len) {
    uint32_t out;
    FNV(key, len, /*seed*/0x42, &out);
    return out;
}

// taken from https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
void Bernstein ( const void * key, int len, uint32_t seed, void * out )
{
    const uint8_t * data = (const uint8_t*)key;

    for(int i = 0; i < len; ++i)
    {
        seed = 33 * seed + data[i];
    }

    *(uint32_t*)out = seed;
}

uint32_t bernstein_hash(const void *key, size_t len) {
    uint32_t out;
    Bernstein(key, len, /*seed*/0x42, &out);
    return out;
}
