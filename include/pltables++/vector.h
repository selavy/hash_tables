#pragma once

#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>

namespace plt {

template <class T>
class Vector
{
public:
    using value_type = T;

    constexpr Vector() noexcept = default;

    Vector(int size, T elem = T()) noexcept : _size{ size }, _asize{ size }
    {
        _data = static_cast<T*>(calloc(sizeof(T), _asize));
        for (int i = 0; i < _size; ++i) {
            new (&_data[i]) T{ elem };
        }
    }

    Vector(const Vector& other) noexcept : _size{ other._size },
                                           _asize{ other._asize }
    {
        _data = static_cast<T*>(calloc(sizeof(T), _asize));
        _copy_data(_data, other._data, other._size);
    }

    Vector(Vector&& other) noexcept : _data{ other._data },
                                      _size{ other._size },
                                      _asize{ other._asize }
    {
        other._data = nullptr;
        other._size = other._asize = 0;
    }

    Vector& operator=(const Vector& other) noexcept
    {
        T* tmp = static_cast<T*>(calloc(sizeof(T), other._asize));
        _copy_data(tmp, other._data, other._size);
        _free_data(_data, _size);
        _size = other._size;
        _asize = other._asize;
        _data = tmp;
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        _free_data(_data, _size);
        _size = other._size;
        other._size = 0;
        _asize = other._asize;
        other._asize = 0;
        _data = other._data;
        other._data = nullptr;
        return *this;
    }

    ~Vector() noexcept { clear(); }

    void append(const T& x) noexcept
    {
        if (_size == _asize)
            _grow(1.5 * _asize + 4);
        new (&_data[_size++]) T{ x };
    }

    void pop() noexcept
    {
        assert(!is_empty());
        _data[--_size].~T();
    }

    void clear() noexcept
    {
        _free_data(_data, _size);
        _size = _asize = 0;
    }

    void shrink_to_fit() noexcept { _grow(_size); }
    constexpr T& operator[](size_t i) noexcept { return _data[i]; }
    constexpr T& operator[](size_t i) const noexcept { return _data[i]; }
    constexpr bool is_empty() const noexcept { return _size == 0; }
    constexpr int size() const noexcept { return _size; }
    constexpr int capacity() const noexcept { return _asize; }

private:
    void _grow(int newasize) noexcept
    {
        assert(newasize >= _size);
        // clang-format off
        if constexpr(std::is_trivially_copyable_v<T>) {
            _data = static_cast<T*>(realloc(_data, sizeof(T) * newasize));
        } else {
            T* tmp = static_cast<T*>(calloc(sizeof(T), newasize));
            _data = _move_data_not_trivial(tmp, _data, _size);
        }
        // clang-format on
        _asize = newasize;
    }

    static void _free_data(T* ptr, const int size) noexcept
    {
        // clang-format off
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (int i = 0; i < size; ++i) {
                ptr[i].~T();
            }
        }
        // clang-format on
        free(ptr);
    }

    // `dst` must not be initialized yet
    static T* _move_data_not_trivial(T* dst, T* src, const int size)
    {
        assert(src != nullptr || size == 0);
        assert(dst != nullptr || size == 0);
        // clang-format off
        if constexpr (std::is_nothrow_move_constructible_v<T>) {
            for (int i = 0; i < size; ++i) {
                new (&dst[i]) T{std::move(src[i])};
            }
        } else if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            for (int i = 0; i < size; ++i) {
                new (&dst[i]) T{src[i]};
                src[i].~T();
            }
        } else {
            int i;
            try {
                for (i = 0; i < size; ++i) {
                    new (&dst[i]) T{src[i]};
                }
            } catch (...) {
                for (; i >= 0; --i) {
                    dst[i].~T();
                }
                throw;
            }
            for (i = 0; i < size; ++i) {
                src[i].~T();
            }
        }
        // clang-format on
        free(src);
        return dst;
    }

    // `dst` must not be initialized yet
    static void _copy_data(T* dst, const T* src, const int size)
    {
        assert(src != nullptr || size == 0);
        assert(dst != nullptr || size == 0);
        // clang-format off
        if constexpr (std::is_trivially_copyable_v<T>) {
            memcpy(dst, src, sizeof(T) * size);
        } else if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            for (int i = 0; i < size; ++i) {
                new (&dst[i]) T(src[i]);
            }
        } else {
            int i;
            try {
                for (i = 0; i < size; ++i) {
                    new (&dst[i]) T(src[i]);
                }
            } catch (...) {
                for (; i >= 0; --i) {
                    dst[i].~T();
                }
                throw;
            }
        }
        // clang-format on
    }

private:
    value_type* _data = nullptr;
    int _size = 0;
    int _asize = 0;
};

} // ~plt
