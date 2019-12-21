#include <catch2/catch.hpp>
#include <pltables/linear_open_address.h>
#include <unordered_map>

TEST_CASE("Default constructed table is empty")
{
    loatable<int, int> table;
    REQUIRE(table.capacity() == 0u);
    REQUIRE(table.size() == 0u);
    REQUIRE(table.empty() == true);
}

TEST_CASE("Resize changes table size")
{
    loatable<int, int> table;
    REQUIRE(table.capacity() == 0u);
    REQUIRE(table.size() == 0u);

    bool result = table.resize(7);
    REQUIRE(result == true);
    REQUIRE(table.capacity() == 8u);

    result = table.resize(16);
    REQUIRE(result == true);
    REQUIRE(table.capacity() == 16u);
}

TEST_CASE("Insert and Find keys")
{
    using IntTable = loatable<int, int>;
    IntTable table;
    IntTable::iterator it = table.find(42);
    REQUIRE(it == table.end());
    table.resize(8u);
    it = table.find(55);
    REQUIRE(it == table.end());

    {
        auto result = table.insert(1, 42);
        REQUIRE(result.second == IntTable::InsertResult::Inserted);
        REQUIRE(result.first != table.end());
        REQUIRE((*result.first).first == 1);
        REQUIRE((*result.first).second == 42);
        REQUIRE(table.size() == 1u);
    }

    {
        auto result = table.insert(1, 43);
        REQUIRE(result.second == IntTable::InsertResult::Present);
        REQUIRE(result.first != table.end());
        REQUIRE((*result.first).first == 1);
        REQUIRE((*result.first).second == 42); // NOTE: value *not* changed
        REQUIRE(table.size() == 1u);
    }

    constexpr int N1 = 1024;
    for (int i = 2; i < N1; ++i) {
        auto result = table.insert(i, i + 55);
        REQUIRE(result.second == IntTable::InsertResult::Inserted);
        REQUIRE((*result.first).first == i);
        REQUIRE((*result.first).second == i + 55);
        REQUIRE(table.size() == size_t(i));
    }

    for (int i = N1; i < N1 + 1024; ++i) {
        auto it = table.find(i);
        REQUIRE(it == table.end());
    }
    for (int i = 2; i < N1; ++i) {
        auto it = table.find(i);
        REQUIRE((*it).first == i);
        REQUIRE((*it).second == i + 55);
    }

    SECTION("const iterator")
    {
        const IntTable& ctable = table;
        IntTable::const_iterator it = ctable.find(2);
        IntTable::const_iterator endIt = ctable.end();
        REQUIRE(it != endIt);
        REQUIRE(it.key() == 2);
        REQUIRE(it.value() == 2 + 55);
        REQUIRE((*it).first == 2);
        REQUIRE((*it).second == 2 + 55);
    }
}

TEST_CASE("Insert and erase keys")
{
    using Table = loatable<int, int>;
    Table table;
    std::unordered_map<int, int> t2;

    constexpr int N = 256;
    for (int i = 0; i < N; ++i) {
        auto result = table.insert(i, i + 1);
        REQUIRE(result.second == Table::InsertResult::Inserted);
        t2.emplace(i, i + 1);
    }
    REQUIRE(table.size() == size_t(N));
    REQUIRE(table.size() == t2.size());

    for (int i = 0; i < N; i += 2) {
        auto result = table.erase(i);
        REQUIRE(result == 1u);
        auto r2 = t2.erase(i);
        REQUIRE(result == r2);
    }

    for (int i = 0; i < N; ++i) {
        auto it = table.find(i);
        if (i % 2 == 0) {
            REQUIRE(it == table.end());
        } else {
            REQUIRE(it != table.end());
            REQUIRE((*it).first == i);
            REQUIRE((*it).second == i + 1);
        }
    }

    for (int i = 0; i < N; ++i) {
        auto result = table.insert(i, i + 2);
        if (i % 2 == 0) {
            REQUIRE(result.second == Table::InsertResult::ReusedSlot);
            REQUIRE((*result.first).first == i);
            REQUIRE((*result.first).second == i + 2);
        } else {
            REQUIRE(result.second == Table::InsertResult::Present);
            REQUIRE(result.first.key() == i);
            REQUIRE(result.first.val() == i + 1);
        }
        auto r2 = t2.emplace(i, i + 2);
        REQUIRE(result.first.key() == r2.first->first);
        REQUIRE(result.first.val() == r2.first->second);
    }
    REQUIRE(table.size() == size_t(N));
    REQUIRE(table.size() == t2.size());

    for (int i = 0; i < 10 * N; ++i) {
        auto result = table.insert(i, i + 3);
        if (i < N) {
            REQUIRE(result.second == Table::InsertResult::Present);
            REQUIRE(result.first.key() == i);
        } else {
            REQUIRE(result.second == Table::InsertResult::Inserted);
            REQUIRE(result.first.key() == i);
            REQUIRE(result.first.value() == i + 3);
        }
        auto r2 = t2.emplace(i, i + 3);
        REQUIRE(result.first.key() == r2.first->first);
        REQUIRE(result.first.val() == r2.first->second);
    }

    REQUIRE(table.size() == t2.size());
    for (auto p1 : table) {
        auto it2 = t2.find(p1.first);
        REQUIRE(it2 != t2.end());
        auto& p2 = *it2;
        REQUIRE(p1.first == p2.first);
        REQUIRE(p1.second == p2.second);
    }
}
