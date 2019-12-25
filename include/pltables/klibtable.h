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
static const double __ac_HASH_UPPER = 0.77;

template <class Key, class Value, class Hash = KlibHash<Key>,
          class KeyEq = KlibEq<Key>>
class klibtable
{
public:
    class iterator;
    using key_type = Key;
    using mapped_type = Value;
    // TODO: remove?
    using value_type = Value;
    // using iterator = int32_t; // TODO: fix
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

    // NOTE(peter): probe sequence will have maximum of (2*asize - 1) iterations
    // before it wraps around
    khiter_t get(key_type key) noexcept
    {
        if (h->n_buckets) {
            khint_t k, i, last, mask, step = 0;
            mask = h->n_buckets - 1;
            k = Hash{}(key);
            i = k & mask;
            last = i;
            while (!__ac_isempty(h->flags, i) &&
                   (__ac_isdel(h->flags, i) || !KeyEq{}(h->keys[i], key))) {
                i = (i + (++step)) & mask;
                if (i == last)
                    return h->n_buckets;
            }
            return __ac_iseither(h->flags, i) ? h->n_buckets : i;
        } else
            return 0;
    }

    int resize(int32_t new_n_buckets) noexcept
    { /* This function uses 0.25*n_buckets bytes of working space instead of
         [sizeof(key_t+val_t)+.25]*n_buckets. */
        int32_t* new_flags = 0;
        khint_t j = 1;
        {
            new_n_buckets = _roundup(new_n_buckets);
            if (new_n_buckets < 4)
                new_n_buckets = 4;
            if (h->size >= (khint_t)(new_n_buckets * __ac_HASH_UPPER + 0.5))
                j = 0; /* requested size is too small */
            else { /* hash table size to be changed (shrink or expand); rehash
                      */
                new_flags = static_cast<int32_t*>(
                  malloc(__ac_fsize(new_n_buckets) * sizeof(int32_t)));
                if (!new_flags)
                    return -1;
                memset(new_flags, 0xaa,
                       __ac_fsize(new_n_buckets) * sizeof(int32_t));
                if (h->n_buckets < new_n_buckets) { /* expand */
                    key_type* new_keys = static_cast<key_type*>(realloc(
                      (void*)h->keys, new_n_buckets * sizeof(key_type)));
                    if (!new_keys) {
                        free(new_flags);
                        return -1;
                    }
                    h->keys = new_keys;
                    value_type* new_vals = static_cast<value_type*>(realloc(
                      (void*)h->vals, new_n_buckets * sizeof(value_type)));
                    if (!new_vals) {
                        free(new_flags);
                        return -1;
                    }
                    h->vals = new_vals;
                } /* otherwise shrink */
            }
        }
        if (j) { /* rehashing is needed */
            for (j = 0; j != h->n_buckets; ++j) {
                if (__ac_iseither(h->flags, j) == 0) {
                    key_type key = h->keys[j];
                    value_type val;
                    khint_t new_mask;
                    new_mask = new_n_buckets - 1;
                    val = h->vals[j];
                    __ac_set_isdel_true(h->flags, j);
                    while (1) { /* kick-out process; sort of like in Cuckoo
                                   hashing */
                        khint_t k, i, step = 0;
                        k = Hash{}(key);
                        i = k & new_mask;
                        while (!__ac_isempty(new_flags, i))
                            i = (i + (++step)) & new_mask;
                        __ac_set_isempty_false(new_flags, i);
                        if (i < h->n_buckets &&
                            __ac_iseither(h->flags, i) ==
                              0) { /* kick out the existing element */
                            {
                                key_type tmp = h->keys[i];
                                h->keys[i] = key;
                                key = tmp;
                            }
                            value_type tmp = h->vals[i];
                            h->vals[i] = val;
                            val = tmp;
                            __ac_set_isdel_true(
                              h->flags,
                              i); /* mark it as deleted in the old hash table */
                        } else {  /* write the element and jump out of the loop
                                     */
                            h->keys[i] = key;
                            if (1)
                                h->vals[i] = val;
                            break;
                        }
                    }
                }
            }
            if (h->n_buckets > new_n_buckets) { /* shrink the hash table */
                h->keys =
                  (key_type*)realloc(h->keys, new_n_buckets * sizeof(key_type));
                h->vals = (value_type*)realloc(h->vals, new_n_buckets *
                                                          sizeof(value_type));
            }
            free(h->flags); /* free the working space */
            h->flags = new_flags;
            h->n_buckets = new_n_buckets;
            h->n_occupied = h->size;
            h->upper_bound = (khint_t)(h->n_buckets * __ac_HASH_UPPER + 0.5);
        }
        return 0;
    }

    iterator begin() noexcept { return { this, _advance_index(-1) }; }

    iterator end() noexcept { return { this, h->n_buckets }; }

    iterator insert(key_type key, value_type value)
    {
        int ret;
        auto it = put(key, &ret);
        if (ret >= 1)
            it.value() = value;
        return it;
    }

    iterator put(key_type key, int* ret) noexcept
    {
        khint_t x;
        if (h->n_occupied >= h->upper_bound) { /* update the hash table */
            if (h->n_buckets > (h->size << 1)) {
                /* clear "deleted" elements */
                if (resize(h->n_buckets - 1) < 0) {
                    *ret = -1;
                    return { this, h->n_buckets };
                }
            } /* expand the hash table */
            else if (resize(h->n_buckets + 1) < 0) {
                *ret = -1;
                return { this, h->n_buckets };
            }
        }
        {
            khint_t k;
            khint_t i;
            khint_t site;
            khint_t last;
            khint_t mask = h->n_buckets - 1;
            khint_t step = 0;
            x = site = h->n_buckets;
            k = Hash{}(key);
            i = k & mask;
            if (__ac_isempty(h->flags, i))
                x = i; /* for speed up */
            else {
                last = i;
                while (!__ac_isempty(h->flags, i) &&
                       (__ac_isdel(h->flags, i) || !KeyEq{}(h->keys[i], key))) {
                    if (__ac_isdel(h->flags, i))
                        site = i;
                    i = (i + (++step)) & mask;
                    if (i == last) {
                        x = site;
                        break;
                    }
                }
                if (x == h->n_buckets) {
                    if (__ac_isempty(h->flags, i) && site != h->n_buckets)
                        x = site;
                    else
                        x = i;
                }
            }
        }
        if (__ac_isempty(h->flags, x)) { /* not present at all */
            h->keys[x] = key;
            __ac_set_isboth_false(h->flags, x);
            ++h->size;
            ++h->n_occupied;
            *ret = 1;
        } else if (__ac_isdel(h->flags, x)) { /* deleted */
            h->keys[x] = key;
            __ac_set_isboth_false(h->flags, x);
            ++h->size;
            *ret = 2;
        } else
            *ret = 0; /* Don't touch h->keys[x] if present and not deleted */
        return { this, x };
    }

    void del(iterator it) noexcept
    {
        int32_t x = it._index;
        if (x != h->n_buckets && !__ac_iseither(h->flags, x)) {
            __ac_set_isdel_true(h->flags, x);
            --h->size;
        }
    }

private:
    constexpr int32_t _advance_index(int32_t i) const noexcept
    {
        // TODO: implement
        return i + 1;
    }

    static constexpr uint32_t _roundup(uint32_t x) noexcept
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

template <class Key, class Value, class Hash, class KeyEq>
class klibtable<Key, Value, Hash, KeyEq>::iterator
{
    friend class klibtable<Key, Value, Hash, KeyEq>;
    using table_t = klibtable<Key, Value, Hash, KeyEq>;

    constexpr static int32_t InvalidIndex = -1;

    table_t* _table = nullptr;
    int32_t _index = InvalidIndex;

public:
    constexpr iterator() noexcept {}
    constexpr iterator(table_t* table, int32_t index) noexcept
      : _table{ table },
        _index{ index }
    {
    }
    constexpr iterator(const iterator& other) noexcept : _table{ other._table },
                                                         _index{ other._index }
    {
    }
    constexpr iterator(iterator&& other) noexcept : _table{ other._table },
                                                    _index{ other._index }
    {
        other._table = nullptr;
        other._index = InvalidIndex;
    }
    constexpr iterator& operator=(const iterator& other) noexcept
    {
        _table = other._table;
        _index = other._index;
        return *this;
    }
    constexpr iterator& operator=(iterator&& other) noexcept
    {
        _table = other._table;
        _index = other._index;
        other._table = nullptr;
        other._index = InvalidIndex;
        return *this;
    }
    iterator& operator++() noexcept
    {
        _index = _table->_advance_index(_index);
        return *this;
    }
    iterator operator++(int)noexcept
    {
        iterator tmp{ *this };
        ++(*this);
        return tmp;
    }
    friend bool operator==(iterator lhs, iterator rhs) noexcept
    {
        return lhs._table == rhs._table && lhs._index == rhs._index;
    }
    friend bool operator!=(iterator lhs, iterator rhs) noexcept
    {
        return lhs._table != rhs._table || lhs._index != rhs._index;
    }
    key_type key() const noexcept { return _table->h->keys[_index]; }
    value_type& val() noexcept { return _table->h->vals[_index]; }
    value_type& value() noexcept { return val(); }
};
