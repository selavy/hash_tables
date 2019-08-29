#pragma once

#include <cstdint>
#include <functional>

template <
    class Key,
    class Value,
    class Hash = std::hash<Key>,
    class Pred = std::equal_to<Key>
>
class Table
    : private Hash,
      private Pred
{
public:
    using size_type = std::size_t;

    size_type size() const noexcept
    { return 0u; }

    bool empty() const noexcept
    { return size() == 0u; }
};
