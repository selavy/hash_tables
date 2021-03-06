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
    // require NoThrowConstructible as well?
    static_assert(std::is_nothrow_move_constructible_v<Key>);
    static_assert(std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_nothrow_move_assignable_v<Key>);
    static_assert(std::is_nothrow_move_assignable_v<T>);
    // static_assert(std::is_trivial_v<Key>, "Key type must be trivial");
    // static_assert(std::is_trivial_v<T>, "Mapped type must be trivial");
    // static_assert(std::is_trivially_copyable_v<Key>,
    //               "Key type must satisfy TriviallyCopyable");
    // static_assert(std::is_trivially_copyable_v<T>,
    //               "Mapped type must satisfy TriviallyCopyable");
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

    static bool insert_failed(InsertResult r) noexcept
    {
        return r == InsertResult::Error;
    }
    static bool item_inserted(InsertResult r) noexcept
    {
        return static_cast<int>(r) >= static_cast<int>(InsertResult::Inserted);
    }

    class iterator;
    class const_iterator;

    using key_type = Key;
    using mapped_type = T;
    using value_type =
      std::pair<std::reference_wrapper<const Key>, std::reference_wrapper<T>>;
    using hasher = Hash;
    using key_equal = KeyEq;

    constexpr loatable() noexcept = default;
    ~loatable() noexcept { clear(); }
    void clear() noexcept
    {
        // clang-format off
        // TODO: probably don't need this guard, verify gets optimized out anyways
        if constexpr (
                !std::is_trivially_destructible_v<Key> ||
                !std::is_trivially_destructible_v<T>
        ) {
            for (int i = 0; i < _asize; ++i) {
                if (_is_alive(_flags, i)) {
                    _keys[i].~Key();
                    _vals[i].~T();
                }
            }
        }
        // clang-format on
        free(_flags);
        free(_keys);
        free(_vals);
        _flags = nullptr;
        _keys = nullptr;
        _vals = nullptr;
        _asize = _size = _used = _cutoff = 0;
    }
    constexpr size_t capacity() const noexcept { return _asize; }
    constexpr size_t size() const noexcept { return _size; }
    constexpr bool empty() const noexcept { return _size == 0u; }
    hasher hash_function() const noexcept { return *this; }
    key_equal key_eq() const noexcept { return *this; }

    bool resize(size_t newsize)
    {
        newsize = _roundup_pow_2(std::max(newsize, _cutoff + 1));
        return _resize_fast(newsize);
    }

    bool reserve(size_t newsize)
    {
        newsize = std::max(newsize, size_t(1));
        newsize = std::max(newsize, _asize);
        newsize = _roundup_pow_2(newsize);
        return _resize_fast(newsize);
    }

    constexpr const_iterator find(key_type key) const noexcept
    {
        return _cfind(key);
    }

    constexpr iterator find(key_type key) noexcept
    {
        const_iterator it = _cfind(key);
        return { this, it._index };
    }

    constexpr iterator begin() noexcept
    {
        size_t i;
        for (i = 0; i < _asize; ++i) {
            if (_is_alive(_flags, i))
                break;
        }
        return { this, i };
    }

    constexpr const_iterator begin() const noexcept { return cbegin(); }

    constexpr const_iterator cbegin() const noexcept
    {
        size_t i;
        for (i = 0; i < _asize; ++i) {
            if (_is_alive(_flags, i))
                break;
        }
        return { this, i };
    }

    constexpr iterator end() noexcept { return { this, _asize }; }
    constexpr const_iterator end() const noexcept { return { this, _asize }; }
    constexpr const_iterator cend() const noexcept { return { this, _asize }; }

    // TODO: update noexcept specifier based on if _resize() is noexcept
    // TODO: add constraint that is_constructible<T, Args...>
    template <class... Args>
    std::pair<iterator, InsertResult>
    insert(key_type key, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<Key>&& std::is_nothrow_constructible_v<T>)
    {
        if (_used >= _cutoff)
            if (!_resize_fast(_size != 0u ? 2u * _asize : MinTableSize))
                return std::make_pair(end(), InsertResult::Error);
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
                new (&keys[i]) Key{ key };
                try {
                    new (&vals[i]) T(std::forward<Args>(args)...);
                } catch (...) {
                    keys[i].~Key();
                    // TODO: why is this throw causing a warning? is my noexcept
                    // specifier wrong?
                    // throw;
                }
                _animate(flags, i);
                ++_size;
                ++_used;
                return std::make_pair(iterator{ this, i }, result);
            } else if (keyeq(key, keys[i])) {
                return std::make_pair(iterator{ this, i },
                                      InsertResult::Present);
            }
            i = (i + 1) & mask;
        }
        __builtin_unreachable();
    }

    constexpr void erase(const_iterator it) noexcept
    {
        assert(it != end());
        _set_tombstone(_flags, it._index);
        --_size;
    }

    constexpr size_t erase(key_type key) noexcept
    {
        auto it = find(key);
        if (it == end())
            return 0u;
        erase(it);
        return 1u;
    }

private:
    constexpr const_iterator _cfind(key_type key) const noexcept
    {
        const auto* flags = _flags;
        const auto* keys = _keys;
        const size_t mask = _asize - 1;
        auto keyeq = key_eq();
        size_t i = hash_function()(key) & mask;
        if (!_flags) // TODO: always allocate?
            return end();
        for (;;) {
            if (_is_alive(flags, i)) {
                if (keyeq(key, keys[i]))
                    return { this, i };
            } else if (!_is_tombstone(flags, i)) {
                break;
            }
            i = (i + 1) & mask;
        }
        return { this, _asize };
    }

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
        const size_t mask = newsize - 1;
        for (size_t i = 0; i < _asize; ++i) {
            if (!_is_alive(oldflgs, i))
                continue;
            size_t j = hashfn(oldkeys[i]) & mask;
            for (;;) {
                if (!_is_alive(flgs, j))
                    break;
                assert(!_is_tombstone(flgs, j));
                j = (j + 1) & mask;
            }
            assert(!_is_alive(flgs, j));

            // clang-format off
            // keys[j] = oldkeys[i];
            // vals[j] = oldvals[i];
            new (&keys[j]) Key{ std::move(oldkeys[i]) };
            new (&vals[j]) T{ std::move(oldvals[i]) };
            // clang-format on
            _set_live(flgs, j);
        }
        free(_flags);
        free(_keys);
        free(_vals);
        _flags = flgs;
        _keys = keys;
        _vals = vals;
        _asize = newsize;
        _cutoff = newsize * MaxLoadFactor;
        _used = _size;
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

    static void _set_live(size_t* flags, size_t i) noexcept
    {
        constexpr size_t n = sizeof(*flags);
        flags[i / n] |= (1u << (2 * (i % n)));
    }

    static void _set_tombstone(size_t* flags, size_t i) noexcept
    {
        constexpr size_t n = sizeof(*flags);
        flags[i / n] &= ~(1u << (2 * (i % n)));
        flags[i / n] |= (1u << (2 * (i % n) + 1));
    }

    constexpr size_t _next_occupied_slot(size_t i) noexcept
    {
        assert(i != _asize);
        for (i = i + 1; i != _asize; ++i) {
            if (_is_alive(_flags, i))
                break;
        }
        return i;
    }

private:
    size_t* _flags = nullptr;
    key_type* _keys = nullptr;
    mapped_type* _vals = nullptr;
    size_t _size = 0;
    size_t _asize = 0;
    size_t _used = 0;
    size_t _cutoff = 0;
};

template <class Key, class T, class Hash, class KeyEq>
class loatable<Key, T, Hash, KeyEq>::iterator
{
    using table_type = loatable<Key, T, Hash, KeyEq>;
    friend class loatable<Key, T, Hash, KeyEq>;
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

    // NOTE: proxy iterator!
    constexpr table_type::value_type operator*() noexcept
    {
        return std::make_pair(std::cref(_table->_keys[_index]),
                              std::ref(_table->_vals[_index]));
    }

    const table_type::key_type& key() const noexcept
    {
        assert(_table != nullptr);
        assert(_index != _table->_asize);
        return _table->_keys[_index];
    }

    table_type::mapped_type& value() const noexcept
    {
        assert(_table != nullptr);
        assert(_index != _table->_asize);
        return _table->_vals[_index];
    }

    table_type::mapped_type& val() const noexcept { return value(); }

    constexpr iterator& operator++() noexcept
    {
        _index = _table->_next_occupied_slot(_index);
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

    constexpr void swap(iterator other) noexcept
    {
        iterator tmp{ *this };
        (*this) = other;
        other = tmp;
    }
};

template <class Key, class T, class Hash, class KeyEq>
class loatable<Key, T, Hash, KeyEq>::const_iterator
{
    using table_type = loatable<Key, T, Hash, KeyEq>;
    friend class loatable<Key, T, Hash, KeyEq>;
    const table_type* _table = nullptr;
    size_t _index = 0;

public:
    constexpr const_iterator() noexcept = default;

    constexpr const_iterator(const table_type* table, size_t index) noexcept
      : _table{ table },
        _index{ index }
    {
    }

    constexpr const_iterator(iterator other) noexcept : _table{ other._table },
                                                        _index{ other._index }
    {
    }

    constexpr const_iterator(const const_iterator& other) noexcept
      : _table{ other._table },
        _index{ other._index }
    {
    }

    constexpr const_iterator(const_iterator&& other) noexcept
      : _table{ other._table },
        _index{ other._index }
    {
        other._table = nullptr;
        other._index = 0;
    }

    constexpr const_iterator& operator=(const const_iterator& other) noexcept
    {
        _table = other._table;
        _index = other._index;
        return *this;
    }

    constexpr const_iterator& operator=(const_iterator&& other) noexcept
    {
        _table = other._table;
        _index = other._index;
        other._table = nullptr;
        other._index = 0;
        return *this;
    }

    // NOTE: proxy iterator!
    constexpr const table_type::value_type operator*() noexcept
    {
        return std::make_pair(std::cref(_table->_keys[_index]),
                              std::ref(_table->_vals[_index]));
    }

    const table_type::key_type& key() const noexcept
    {
        assert(_table != nullptr);
        assert(_index != _table->_asize);
        return _table->_keys[_index];
    }

    const table_type::mapped_type& value() const noexcept
    {
        assert(_table != nullptr);
        assert(_index != _table->_asize);
        return _table->_vals[_index];
    }

    table_type::mapped_type& val() const noexcept { return value(); }

    constexpr const_iterator& operator++() noexcept
    {
        _index = _table->_next_occupied_slot(_index);
        return *this;
    }

    constexpr const_iterator operator++(int)noexcept
    {
        const_iterator tmp{ *this };
        ++(*this);
        return tmp;
    }

    constexpr const_iterator operator+=(size_t d) noexcept
    {
        _index += d;
        return *this;
    }

    friend constexpr const_iterator operator+(const_iterator lhs,
                                              size_t dist) noexcept
    {
        lhs += dist;
        return lhs;
    }

    friend constexpr bool operator==(const_iterator lhs,
                                     const_iterator rhs) noexcept
    {
        return lhs._table == rhs._table && lhs._index == rhs._index;
    }

    friend constexpr bool operator!=(const_iterator lhs,
                                     const_iterator rhs) noexcept
    {
        return lhs._table != rhs._table || lhs._index != rhs._index;
    }

    constexpr void swap(const_iterator other) noexcept
    {
        const_iterator tmp{ *this };
        (*this) = other;
        other = tmp;
    }
};
