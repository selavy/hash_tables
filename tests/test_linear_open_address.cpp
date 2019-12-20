#include <catch2/catch.hpp>
#include <pltables/linear_open_address.h>

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
        REQUIRE(*(*result.first).first  == 1);
        REQUIRE(*(*result.first).second == 42);
        REQUIRE(table.size() == 1u);
    }

    {
        auto result = table.insert(1, 43);
        REQUIRE(result.second == IntTable::InsertResult::Present);
        REQUIRE(result.first != table.end());
        REQUIRE(*(*result.first).first  == 1);
        REQUIRE(*(*result.first).second == 42); // NOTE: value *not* changed
        REQUIRE(table.size() == 1u);
    }

    for (int i = 2; i < 1024; ++i) {
        auto result = table.insert(i, i + 55);
        REQUIRE(result.second == IntTable::InsertResult::Inserted);
        REQUIRE(*(*result.first).first  == i);
        REQUIRE(*(*result.first).second == i + 55);
        REQUIRE(table.size() == size_t(i));
    }
}
