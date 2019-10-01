#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <type_traits>
#include <utility>
#include <pmmintrin.h>
#include <immintrin.h>


#if 0
template <class Key, class Value>
union Slot {
    Key                         key;
    std::pair<Key, Value>       p1;
    std::pair<const Key, Value> p2;
};
#endif

template<class Key, class Value>
using Slot = std::pair<const Key, Value>;

struct _SimpleHashReducer
{
    uint8_t operator()(std::size_t h) {
        return reinterpret_cast<uint8_t*>((h >> 16) & 0xFFu)[0];
    }
};


template <
    class Key,
    class Value,
    class Hash = std::hash<Key>,
    class Pred = std::equal_to<Key>,
    class H2   = _SimpleHashReducer
>
class Table
    : private Hash,
      private Pred
{
    enum CtrlBits : uint8_t
    {
        kEmpty    = 0x80u, // -128
        kDeleted  = 0xFEu, // -2
        kSentinel = 0xFFu, // -1
    };

    using ctrl_type = int8_t;
    using slot_type = Slot<Key, Value>;

public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = Pred;
    using key_type = const Key;
    using value_type = std::pair<const Key, Value>;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

    struct iterator {
        iterator() noexcept : _table{nullptr}, _index{-1} {}
        iterator(Table* table, size_type index) noexcept : _table{table}, _index{index} {}
        iterator(const iterator& other) noexcept : _table{other._table}, _index{other._index} {}
        iterator(iterator&& other) noexcept : _table{other._table}, _index{other._index}
        { other._table = nullptr; other._index = -1; }
        iterator& operator=(const iterator& other) noexcept
        { _table = other._table; _index = other._index; }
        iterator& operator=(iterator&& other) noexcept
        {
            _table = other._table; _index = other._index;
            other._table = nullptr; other._index = -1;
        }
        iterator& operator++(int) noexcept
        { iterator tmp{*this}; ++(*this); return tmp; }
        iterator& operator++() noexcept
        {
            // TODO: implement
            return *this;
        }

        private:
        Table*    _table;
        size_type _index;
    };

    static_assert(!std::is_reference<key_type>::value, "");
    static_assert(!std::is_reference<value_type>::value, "");

    Table()//  noexcept(
           //      std::is_nothrow_default_constructible<hasher>::value &&
           //      std::is_nothrow_default_constructible<key_equal>::value
           //  )
        : _size{0u}
        , _capacity{32u}
        , _slots{nullptr}
        , _bits{nullptr}
    {
        _slots = reinterpret_cast<slot_type*>(malloc(sizeof(*_slots) * _capacity));
        // TODO: need to make sure len(_bits) % 16 == 0
        _bits  = reinterpret_cast<ctrl_type*>(malloc(sizeof(*_bits) * _capacity));
        if (!_slots || !_bits) {
            free(_slots);
            free(_bits);
            throw std::bad_alloc{};
        }
        memset(&_bits[0], kEmpty, _capacity);
        assert((_capacity & (_capacity - 1)) == 0);
    }

    ~Table()
    {
        free(_slots);
        free(_bits);
        _size = 0u;
        _capacity = 0u;
    }

    [[nodiscard]]
    size_type size() const noexcept
    { return _size; }

    [[nodiscard]]
    bool empty() const noexcept
    { return size() == 0u; }

    [[nodiscard]]
    size_type capacity() const noexcept
    { return _capacity; }

    [[nodiscard]]
    size_type max_size() const noexcept
    { return std::numeric_limits<size_type>::max(); }

    void clear()
    {
        // TODO: implement
    }

    hasher hash_function() const { return *this; }
    key_equal key_eq() const { return *this; }

    // TODO: make sure is convertible
    template <class K, class V>
    std::pair<iterator, bool> insert(std::pair<K, V>&& value)
    { return insert(std::make_pair(value.first, value.second)); }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        int position;
        uint32_t bits;
        size_type hash, h2, index, mask;
        __m128i sentinels, ctrl;

        mask = _capacity - 1;
        hash = hash_function()(value.first);
        h2 = H2(hash);
        index = hash & mask;
        sentinels = _mm_set1_epi8(kSentinel);
        for (;;) {
            bits = _find_equal(&_bits[index], h2);
            while (bits) {
                position = __builtin_ctz(bits);
                if (key_eq()(value.first, _slots[index + position].first)) {
                    return { iterator(this, index + position), false };
                }
                bits &= bits - 1;
            }
            bits = _find_empty(&_bits[index]);
            if (bits != 0) {
               _slots[index + position] = value;
               return { iterator(this, index + position), true };
            }
            index = (index + 16) & mask;
        }
    }

private:
    static uint32_t _find_empty_or_sentinel(ctrl_type* ctrl)
    {
        __m128i sentinels = _mm_set1_epi8(kSentinel);
        __m128i control   = _mm_loadu_epi8(ctrl);
        return _mm_cmpgt_epi8_mask(sentinels, control);
    }

    static uint32_t _find_empty(ctrl_type* ctrl)
    {
        __m128i x = _mm_loadu_epi8(ctrl);
        return _mm_movemask_epi8(_mm_sign_epi8(x, x));
    }

    static uint32_t _find_equal(ctrl_type* ctrl, ctrl_type x)
    { return _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_epi8(_ctrl), _mm_set1_epi8(x))); }

    static size_type force_power_of_2(size_type x) noexcept
    {
        --(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x);
        return x;
    }

private:
    size_type  _size;
    size_type  _capacity;
    slot_type* _slots;
    ctrl_type* _bits;
};
