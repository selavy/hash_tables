#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <type_traits>
#include <utility>

template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEq = std::equal_to<Key>>
class loatable : private Hash, private KeyEq
{
    static_assert(std::is_trivial_v<Key>, "Key type must be trivial");
    static_assert(std::is_trivial_v<T>, "Mapped type must be trivial");
    static_assert(std::is_trivially_copyable_v<Key>,
                  "Key type must satisfy TriviallyCopyable");
    static_assert(std::is_trivially_copyable_v<T>,
                  "Mapped type must satisfy TriviallyCopyable");
    constexpr static double MaxLoadFactor = 0.77;
    constexpr static size_t MinTableSize = 8;

public:
    enum class InsertResult
    {
        Error = -1,
        Present = 0,
        Inserted = 1,
        ReusedSlot = 2,
    };

    struct iterator;

    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key*, T*>;
    using hasher = Hash;
    using key_equal = KeyEq;

    constexpr loatable() noexcept = default;
    constexpr size_t capacity() const noexcept { return _asize; }
    constexpr size_t size() const noexcept { return _size; }
    constexpr bool empty() const noexcept { return _size == 0u; }
    hasher hash_function() const noexcept { return *this; }
    key_equal key_eq() const noexcept { return *this; }

    bool resize(size_t newsize)
    {
        newsize = std::max(newsize, _cutoff);
        newsize = _roundup_pow_2(newsize);
        return _resize_fast(newsize);
    }

    constexpr iterator find(key_type key) noexcept
    {
        // TODO: always allocate?
        if (!_flags)
            return end();
        const auto* flags = _flags;
        const auto* keys = _keys;
        const size_t mask = _asize - 1;
        auto keyeq = key_eq();
        size_t i = hash_function()(key) & mask;
        for (;;) {
            if (_is_alive(flags, i)) {
                if (keyeq(key, keys[i])) {
                    return { this, i };
                }
            } else if (!_is_tombstone(flags, i)) {
                break;
            }
            i = (i + 1) & mask;
        }
        return { this, _asize };
    }

    constexpr iterator begin() noexcept
    {
        // TODO: fast forward to first element
        const auto* flags = _flags;
        size_t i = 0;
        for (; i != _asize; ++i) {
            if (_is_alive(flags, i))
                break;
        }
        return { this, i };
    }

    constexpr iterator end() noexcept { return { this, _asize }; }

    std::pair<iterator, InsertResult> insert(key_type key,
                                             mapped_type value) noexcept
    {
        if (_size <= _cutoff)
            if (!_resize_fast(_size ? 2*_size : MinTableSize))
                return std::make_pair(end(), InsertResult::Error);
        ++_size;
        assert(_asize > _size);

        const size_t mask = _asize - 1;
        auto* flags = _flags;
        auto* keys = _keys;
        auto* vals = _vals;
        auto keyeq = key_eq();
        size_t i = hash_function()(key) & mask;
        for (;;) {
            if (!_is_alive(flags, i)) {
                auto result = _is_tombstone(flags, i) ? InsertResult::ReusedSlot
                                                      : InsertResult::Inserted;
                // NOTE: key and value are IsTriviallyCopyable
                keys[i] = key;
                vals[i] = value;
                _animate(flags, i);
                return std::make_pair(iterator{ this, i }, result);
            } else if (keyeq(key, keys[i])) {
                return std::make_pair(iterator{ this, i },
                                      InsertResult::Present);
            }
            i = (i + 1) & mask;
        }
    }

private:
    static constexpr size_t _roundup_pow_2(size_t x) noexcept
    {
        x = std::max(x, MinTableSize);
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        ++x;
        return x;
    }

    bool _resize_fast(size_t newsize) noexcept
    {
        assert(newsize != 0);
        assert((newsize & (newsize - 1)) == 0); // table size must be power of 2
        assert(newsize * MaxLoadFactor > _size);
        auto* flgs =
          static_cast<size_t*>(calloc(newsize / sizeof(*flgs), sizeof(*flgs)));
        auto* keys = static_cast<key_type*>(calloc(newsize, sizeof(key_type)));
        auto* vals =
          static_cast<mapped_type*>(calloc(newsize, sizeof(mapped_type)));
        if (!flgs || !keys || !vals) {
            free(flgs);
            free(keys);
            free(vals);
            return false;
        }
        auto hashfn = hash_function();
        const auto* oldflgs = _flags;
        const auto* oldkeys = _keys;
        const auto* oldvals = _vals;
        const auto oldasize = _asize;
        const auto mask = newsize - 1u;
        for (size_t i = 0; i < _asize; ++i) {
            if (!_is_alive(oldflgs, i))
                continue;
            size_t j = hashfn(oldkeys[i]) & mask;
            for (; !_is_alive(flgs, j); j = (j + 1) & mask)
                ;
            assert(_is_alive(flgs, j));
            _animate(flgs, j);
            keys[j] = oldkeys[i];
            vals[j] = oldvals[j];
        }
        _flags = flgs;
        _keys = keys;
        _vals = vals;
        _asize = newsize;
        _cutoff = newsize * MaxLoadFactor;
        return true;
    }

    static constexpr bool _is_alive(const size_t* flags, size_t i) noexcept
    {
        constexpr size_t n = sizeof(*flags);
        return ((flags[i / n] & (1u << (2 * (i % n)))) != 0);
    }

    static constexpr bool _is_tombstone(const size_t* flags, size_t i) noexcept
    {
        constexpr size_t n = sizeof(*flags);
        return ((flags[i / n] & (1u << (2 * (i % n) + 1))) != 0);
    }

    static void _animate(size_t* flags, size_t i) noexcept
    {
        constexpr size_t n = sizeof(*flags);
        flags[i / n] &= ~(1u << (2 * (i % n) + 1));
        flags[i / n] |= (1u << (2 * (i % n)));
    }

private:
    size_t* _flags = nullptr;
    key_type* _keys = nullptr;
    mapped_type* _vals = nullptr;
    size_t _size = 0;
    size_t _asize = 0;
    size_t _cutoff = 0;
};

template <class Key, class T, class Hash, class KeyEq>
class loatable<Key, T, Hash, KeyEq>::iterator
{
    using table_type = loatable<Key, T, Hash, KeyEq>;
    table_type* _table = nullptr;
    size_t _index = 0;

public:
    constexpr iterator() noexcept = default;

    constexpr iterator(table_type* table, size_t index) noexcept
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
        other._index = 0;
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
        other._index = 0;
        return *this;
    }

    constexpr table_type::value_type operator*() noexcept
    {
        return std::make_pair(&_table->_keys[_index], &_table->_vals[_index]);
    }

    constexpr iterator operator++() noexcept
    {
        ++_index;
        return *this;
    }

    constexpr iterator operator++(int)noexcept
    {
        iterator tmp{ *this };
        ++(*this);
        return tmp;
    }

    constexpr iterator operator+=(size_t d) noexcept
    {
        _index += d;
        return *this;
    }

    friend constexpr iterator operator+(iterator lhs, size_t dist) noexcept
    {
        lhs += dist;
        return lhs;
    }

    friend constexpr bool operator==(iterator lhs, iterator rhs) noexcept
    {
        return lhs._table == rhs._table && lhs._index == rhs._index;
    }

    friend constexpr bool operator!=(iterator lhs, iterator rhs) noexcept
    {
        return lhs._table != rhs._table || lhs._index != rhs._index;
    }
};