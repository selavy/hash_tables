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

TEST_CASE("Find key")
{
    loatable<int, int> table;
    loatable<int, int>::iterator it = table.find(42);
    REQUIRE(it == table.end());
    table.resize(8u);
    it = table.find(55);
    REQUIRE(it == table.end());
}
