#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>


#ifndef restrict
#define restrict __restrict
#endif

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
        if (this != &other) {
            if (other._size > _asize) {
                T* tmp = static_cast<T*>(calloc(sizeof(T), other._asize));
                _copy_data(tmp, other._data, other._size);
                _free_data(_data, _size);
                _data = tmp;
                _asize = other._asize;
            } else {
                // already have enough room, try to use it
                _copy_assign(_data, other._data, other._size, _size);
            }
            _size = other._size;
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        if (this != &other) {
            _free_data(_data, _size);
            _size = other._size;
            other._size = 0;
            _asize = other._asize;
            other._asize = 0;
            _data = other._data;
            other._data = nullptr;
        }
        return *this;
    }

    ~Vector() noexcept { clear(); }

    // TODO: set noexcept specifier
    void append(const T& x)
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
    // TODO: set noexcept specifier
    void _grow(int newasize) noexcept(std::is_trivially_copyable_v<T> ||
                                      std::is_nothrow_move_constructible_v<T> ||
                                      std::is_nothrow_copy_constructible_v<T>)
    {
        assert(newasize >= _size);
        // clang-format off
        if constexpr (std::is_trivially_copyable_v<T>) {
            // TODO:  does this need IsTriviallyDestructible as well?
            _data = static_cast<T*>(realloc(_data, sizeof(T) * newasize));
        } else {
            T* tmp = static_cast<T*>(calloc(sizeof(T), newasize));
            _data = _move_data_not_trivial(tmp, _data, _size);
        }
        // clang-format on
        _asize = newasize;
    }

    static void _free_data(T* restrict ptr, const int size) noexcept
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

    // `dst` must not be initialized yet, `src` is free'd
    static T* _move_data_not_trivial(
      T* restrict dst, T* restrict src,
      const int size) noexcept(std::is_nothrow_move_constructible_v<T> ||
                               std::is_nothrow_copy_constructible_v<T>)
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

    static void _copy_data(
      T* restrict dst, const T* restrict src,
      const int size) noexcept(std::is_trivially_copyable_v<T> ||
                               std::is_nothrow_copy_constructible_v<T>)
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

    static void _copy_assign(
      T* restrict dst, const T* restrict src, const int size,
      int oldsize) noexcept(std::is_trivially_copyable_v<T> ||
                            std::is_nothrow_copy_assignable_v<T>)
    {
        assert(src != nullptr || size == 0);
        assert(dst != nullptr || size == 0);
        // clang-format off
        if constexpr (std::is_trivially_copyable_v<T>) {
            memcpy(dst, src, sizeof(T) * size);
        } else if constexpr (std::is_nothrow_copy_assignable_v<T>) {
            for (int i = 0; i < size; ++i) {
                dst[i] = src[i];
            }
        } else {
            // can't maintain strong exception guarantee AND reuse the memory
            // we already have.
            T* tmp = static_cast<T*>(calloc(sizeof(T), size));
            _copy_data(tmp, src, size);
            _free_data(dst, oldsize);
            dst = tmp;
        }
        // clang-format on
    }

private:
    value_type* _data = nullptr;
    int _size = 0;
    int _asize = 0;
};

} // ~plt
