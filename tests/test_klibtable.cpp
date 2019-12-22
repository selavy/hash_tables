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
