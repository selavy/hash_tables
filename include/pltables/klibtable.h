#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

template <class T>
struct KlibHash
{
    int32_t operator()(T k) noexcept { return static_cast<int32_t>(k); }
};

template <class T>
struct KlibEq
{
    bool operator()(T a, T b) noexcept { return a == b; }
};

// TODO: make private functions
#define __ac_isempty(flag, i) ((flag[i >> 4] >> ((i & 0xfU) << 1)) & 2)
#define __ac_isdel(flag, i) ((flag[i >> 4] >> ((i & 0xfU) << 1)) & 1)
#define __ac_iseither(flag, i) ((flag[i >> 4] >> ((i & 0xfU) << 1)) & 3)
#define __ac_set_isdel_false(flag, i)                                          \
    (flag[i >> 4] &= ~(1ul << ((i & 0xfU) << 1)))
#define __ac_set_isempty_false(flag, i)                                        \
    (flag[i >> 4] &= ~(2ul << ((i & 0xfU) << 1)))
#define __ac_set_isboth_false(flag, i)                                         \
    (flag[i >> 4] &= ~(3ul << ((i & 0xfU) << 1)))
#define __ac_set_isdel_true(flag, i) (flag[i >> 4] |= 1ul << ((i & 0xfU) << 1))
#define __ac_fsize(m) ((m) < 16 ? 1 : (m) >> 4)

template <class Key, class Value, class Hash = KlibHash<Key>,
          class KeyEq = KlibEq<Key>>
class klibtable
{
public:
    using key_type = Key;
    using mapped_type = Value;
    // TODO: remove?
    using value_type = Value;
    using iterator = int32_t; // TODO: fix
    using khint_t = int32_t;
    using khiter_t = khint_t;

    constexpr klibtable() noexcept
    {
        h = static_cast<Table*>(calloc(1, sizeof(Table)));
    }

    ~klibtable() noexcept
    {
        if (h) {
            free(h->keys);
            free(h->flags);
            free(h->vals);
            free(h);
        }
    }

    constexpr int32_t size() const noexcept { return h->size; }
    constexpr bool empty() const noexcept { return h->size == 0; }

    void clear() noexcept
    {
        if (h && h->flags) {
            memset(h->flags, 0xaa, __ac_fsize(h->n_buckets) * sizeof(int32_t));
            h->size = h->n_occupied = 0;
        }
    }

    khiter_t get(key_type key) noexcept
    {
        if (h->n_buckets) {
            khint_t k, i, last, mask, step = 0;
            mask = h->n_buckets - 1;
            k = Hash{}(key);
            i = k & mask;
            last = i;
            while (
              !__ac_isempty(h->flags, i) &&
              (__ac_isdel(h->flags, i) || !KeyEq{}(h->keys[i], key))) {
                i = (i + (++step)) & mask;
                if (i == last)
                    return h->n_buckets;
            }
            return __ac_iseither(h->flags, i) ? h->n_buckets : i;
        } else
            return 0;
    }

    // int resize(int32_t new_n_buckets) noexcept {}

    // int put(key_type key, int* ret) noexcept {}

    // void del(iterator x) noexcept {}

private:
    struct Table
    {
        int32_t n_buckets;
        int32_t size;
        int32_t n_occupied;
        int32_t upper_bound;
        int32_t* flags;
        key_type* keys;
        value_type* vals;
    };

    Table* h = nullptr;
};
