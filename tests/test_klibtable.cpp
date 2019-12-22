#include <catch2/catch.hpp>
#include <pltables/klibtable.h>

TEST_CASE("KLIB - Default constructed table is empty", "[klib]")
{
    using Table = klibtable<int, int>;
    Table table;
    REQUIRE(table.size() == 0);
    REQUIRE(table.empty() == true);

    int result = table.resize(32);
    REQUIRE(result == 0);
}

TEST_CASE("KLIB - Insert and Find")
{
    using Table = klibtable<int, int>;
    Table table;
    int ret;

    {
        Table::iterator it = table.put(42, &ret);
        REQUIRE(ret == 1);
        REQUIRE(it != table.end());
        REQUIRE(it.key() == 42);
        REQUIRE(table.size() == 1);
    }

    {
        Table::iterator it = table.put(42, &ret);
        REQUIRE(ret == 0);
        REQUIRE(it != table.end());
        REQUIRE(it.key() == 42);
        REQUIRE(table.size() == 1);
    }

    {
        Table::iterator it = table.insert(44, 45);
        REQUIRE(it != table.end());
        REQUIRE(it.key() == 44);
        REQUIRE(it.val() == 45);
    }
}
