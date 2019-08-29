#include "catch.hpp"
#include "table.h"
#include <unordered_map>

TEST_CASE("size and empty", "[table]")
{
    Table<int, int> table;
    REQUIRE(table.size()  == 0u);
    REQUIRE(table.empty() == true);
}

#if 0
TEST_CASE("size", "[table]")
{
    Table<int, int> t;
    Table<int, int>::iterator it;
    std::pair<bool, Table<int, int>::iterator> ret;
    REQUIRE(t.size() == 0);
    REQUIRE(t.empty() == true);

    ret = t.insert(1, 2);
    REQUIRE(ret.first == true);
    it = ret.second;
    REQUIRE((*it).first  == 1);
    REQUIRE((*it).second == 2);
    REQUIRE(t.size() == 1);

    int count = 0;
    for (it = t.begin(); it != t.end(); ++it) {
        printf("%d, ", (*it).first);
        ++count;
    }
    REQUIRE(count == 1);

    ret = t.insert(2, 3);
    REQUIRE(ret.first == true);
    it = ret.second;
    REQUIRE((*it).first  == 2);
    REQUIRE((*it).second == 3);
    REQUIRE(t.size() == 2);
}
#endif

TEST_CASE("umap", "[umap]")
{
    std::unordered_map<int, int> t;
    std::unordered_map<int, int>::iterator it;
    std::pair<decltype(it), bool> ret;

    REQUIRE(t.size() == 0u);
    REQUIRE(t.empty() == true);

    ret = t.insert(std::make_pair(1, 2));

    it = t.find(1);

     // iterator::operator* -> std::pair<const Key, Value>
    std::pair<const int, int>& a = *it;

    std::pair<const int, int>* b = it.operator->();
    b->second = 3;

    it = t.find(1);
    REQUIRE(it->second == 3);
}
