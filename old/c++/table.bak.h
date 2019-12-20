#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <memory>
#include <functional>
#include <type_traits>


template <
    class Key,
    class Value,
    class Hash = std::hash<Key>,
    class Pred = std::equal_to<Key>
>
class Table : private Hash, private Pred {
    constexpr static double CUTOFF = 0.70;

public:
    // struct Node {
    //     const Key& first;
    //     Value&     second;

    //     constexpr Node(const Key& key, Value& value) noexcept : first{key}, second{value} {}
    //     constexpr const Key& key() const noexcept { return first; }
    //     constexpr Value& value() const noexcept { return second; }
    // };

    class Iterator {
        Table* _table;
        int    _index;

    public:
        constexpr Iterator() noexcept : _table(nullptr), _index(0) {}
        constexpr Iterator(Table& t, int i) noexcept : _table(&t), _index(i) {}
        Iterator operator++() noexcept {
            // TODO: clean up
            for (;;) {
                ++_index;
                if (!(_index < _table->_asize))
                    return *this;
                if (_table->_is_alive(_index))
                    return *this;
            }
        }

        Iterator operator++(int) noexcept {
            Iterator tmp{*this};
            ++(*this);
            return tmp;
        }

        bool operator==(Iterator other) const noexcept {
            return _table == other._table && _index == other._index;
        }

        bool operator!=(Iterator other) const noexcept {
            return !(*this == other);
        }

        std::pair<const Key&, Value&> operator*() const noexcept {
            return { key(), value() };
        }

        // TODO: how to return a pointer result?
        // Node* operator->() const noexcept {
        //     return &_n;
        // }

        const Key& key() const noexcept {
            return _table->_keys[_index];
        }

        Value& value() const noexcept {
            return _table->_vals[_index];
        }
    };

public:
    using key_type   = const Key;
    using value_type = Value;
    using iterator = Iterator;
    using const_iterator = const Iterator;
    using key_equal = Pred;

    Table() : Table(32) {}

    Table(int size)
        : _size(0)
        , _asize(_roundup(size))
        , _limit(_asize * CUTOFF)
        , _keys(std::make_unique<Key[]>(_asize))
        , _vals(std::make_unique<Value[]>(_asize))
        , _msks(std::make_unique<uint8_t[]>(_asize))
    {
        memset(&_msks[0], 0, sizeof(_msks[0]) * _asize);
    }

    bool empty() const noexcept { return _size == 0; }

    int  size() const noexcept { return _size; }

    iterator end() noexcept {
        return iterator(*this, _asize);
    }

    const_iterator end() const noexcept {
        return end();
    }

    iterator begin() noexcept {
        iterator ret{*this, 0};
        if (!_is_alive(0))
            ++ret;
        return ret;
    }

    std::pair<bool, Iterator> insert(Key key, Value value) {
        size_t hash = hash_function()(key);
        size_t mask = _asize - 1;
        size_t index = hash & mask;
        for (;;) {
            if (!_is_alive(index)) {
                ++_size;
                _keys[index] = key;
                _vals[index] = value;
                _mark_alive(index);
                return std::make_pair(true, Iterator(*this, index));
            }
            index = _next_index(index) & mask;
        }
        __builtin_unreachable();
        assert(0);
        return std::make_pair(false, end());
    }

    Hash& hash_function() noexcept { return *this; }

    key_equal& key_eq() noexcept { return *this; }

private:
    static size_t _next_index(size_t h) noexcept { return h + 1; }

    static int _roundup(int x) noexcept {
        --(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x);
        return x;
    }

    static uint8_t _alive_mask(int x) noexcept {
        return 1u << (2*(x % 4));
    }

    static uint8_t _tombstone_mask(int x) noexcept {
        return 1u << (2*(x % 4) + 1);
    }

    void _mark_alive(int index) {
        assert(0 <= index && index < _asize);
        _msks[index / 4] &= ~_tombstone_mask(index);
        _msks[index / 4] |=  _alive_mask(index);
    }

    bool _is_alive(int x) const noexcept {
        assert(0 <= x && x < _asize);
        return (_msks[x / 4] & _alive_mask(x)) != 0;
    }

    bool _is_tombstone(int x) const noexcept {
        assert(0 <= x && x < _asize);
        return (_msks[x / 4] & _tombstone_mask(x)) != 0;
    }

    int                        _size;
    int                        _asize;
    int                        _limit;
    std::unique_ptr<Key[]>     _keys;
    std::unique_ptr<Value[]>   _vals;
    std::unique_ptr<uint8_t[]> _msks;
};
