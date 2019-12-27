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

#define loa_hash(x) loa_hash_int(x)
#define loa_eq(a, b) loa_eq_int(a, b)
#ifndef reallocarray
#define reallocarray(ptr, nmemb, size) realloc(ptr, (nmemb) * (size))
#endif
#define qoa_calloc(nmemb, size) calloc(nmemb, size)
#define qoa_free(ptr, size) free(ptr)
#define qoa_reallocarray(ptr, nmemb, size) reallocarray(ptr, nmemb, size)
#define qoa_freearray(ptr, nmemb, size) free(ptr)
#define LOAINLINE static inline

typedef uint32_t flg_t;
struct loatable_s
{
    flg_t *flags;
    key_t *keys;
    val_t *vals;
    uint32_t size, asize, used;
};
typedef struct loatable_s loatable;

int loa_max_load_factor(int asize)
{
    return 0.77 * asize + 0.5;
}
int loa_fsize(int asize)
{
    return asize / sizeof(flg_t);
}
uint32_t loa_flgbits(const flg_t *flags, uint32_t i)
{
    // uint32_t bits_per_byte = 8;
    // uint32_t bits_per_entry = 2;
    // uint32_t stride = (bits_per_byte / bits_per_entry)*sizeof(flg_t);
    // flg_t    flag   = flags[i / stride];
    // uint32_t shift  = 2u*(i % stride);
    // return (flag >> shift);

    _Static_assert(sizeof(flg_t) == 4, "");
    return flags[i >> 4] >> ((i & 0xFu) << 1u);
}
int loa_islive(const flg_t *flags, int i)
{
    return (loa_flgbits(flags, i) & 0x1u) != 0;
}
int loa_istomb(const flg_t *flags, int i)
{
    return (loa_flgbits(flags, i) & 0x2u) != 0;
}
int loa_isdead(const flg_t *flags, int i)
{
    return (loa_flgbits(flags, i) & 0x3u) == 0;
}
void loa_setlive(flg_t *flags, int i)
{
    // uint32_t bits_per_byte = 8;
    // uint32_t bits_per_entry = 4;
    // uint32_t stride = (bits_per_byte / bits_per_entry)*sizeof(flg_t);
    // flg_t* flag = &flags[i / stride];
    // uint32_t shift = 2u*(i % stride);

    _Static_assert(8 / 2 * sizeof(flg_t) == 16, "");
    flg_t *flag = &flags[i >> 4];
    uint32_t shift = (i & 0xFu) << 1u;
    *flag &= ~(0x3u << shift); /* clear flags */
    *flag |= (0x1u << shift); /* set islive  */
}
void loa_settomb(flg_t *flags, int i)
{
    _Static_assert(8 / 2 * sizeof(flg_t) == 16, "");
    flg_t *flag = &flags[i >> 4];
    uint32_t shift = (i & 0xFu) << 1u;
    *flag &= ~(0x3u << shift); /* clear flags */
    *flag |= (0x2u << shift); /* set istomb  */
}
void loa_setdead(flg_t *flags, int i)
{
    _Static_assert(8 / 2 * sizeof(flg_t) == 16, "");
    flg_t *flag = &flags[i >> 4];
    uint32_t shift = (i & 0xFu) << 1u;
    *flag &= ~(0x3u << shift); /* clear flags */
}

#endif /* LOATABLE__H_ */
