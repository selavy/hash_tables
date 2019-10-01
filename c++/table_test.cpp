#include "catch.hpp"
#include "table.h"
#include <unordered_map>
#include <utility>

// template <class K, class V>
// std::pair<const K, V>& foo(std::pair<K, V>& p) {
//     return p;
// }

TEST_CASE("size and empty", "[table]")
{
    Table<int, int> table;
    REQUIRE(table.size()  == 0u);
    REQUIRE(table.empty() == true);

    // std::pair<int, int> a{1, 2};
    // std::pair<const int, int>& b = foo(a);
    // REQUIRE(b.first  == 1);
    // REQUIRE(b.second == 2);

    // struct A {
    //     A(int aa, int bb) : a(aa), b(bb) {}
    //     int a;
    //     int b;
    // };

    // A a(1, 2);
    // std::pair<int, int> b{ a.a, }
}

struct A {
    explicit A(int x) noexcept : i(x) { ++_instances; ++_ctor; }
    A(const A& that) noexcept : i(that.i) { ++_instances; ++_copy; }
    A(A&& that) noexcept : i(that.i) { ++_instances; ++_move; }
    A& operator=(const A& other) noexcept { i = other.i; ++_instances; ++_casn; }
    A& operator=(A&& other) noexcept { i = other.i; ++_instances; ++_masn; }
    ~A() noexcept { ++_dtor; }

    int i;

    static void report() {
        printf("Instances   : %d\n", _instances);
        printf("Ctors       : %d\n", _ctor);
        printf("Dtors       : %d\n", _dtor);
        printf("Moves       : %d\n", _move);
        printf("Copyies     : %d\n", _copy);
        printf("Copy Assigns: %d\n", _casn);
        printf("Move Assigns: %d\n", _masn);
    }

    static int _instances;
    static int _ctor;
    static int _move;
    static int _dtor;
    static int _copy;
    static int _casn;
    static int _masn;
};

bool operator==(A lhs, A rhs) noexcept {
    return lhs.i == rhs.i;
}

bool operator!=(A lhs, A rhs) noexcept {
    return lhs.i != rhs.i;
}

int A::_instances = 0;
int A::_ctor = 0;
int A::_move = 0;
int A::_dtor = 0;
int A::_copy = 0;
int A::_casn = 0;
int A::_masn = 0;

namespace std {
template <>
struct hash<A> {
    size_t operator()(A a) const noexcept {
        return hash<int>{}(a.i);
    }
};
} // std


template <class K, class V>
union slot_type {
    // slot_type(K&& k, V&& v) : p1{std::forward<K>(k), std::forward<V>(v)} {}
    slot_type() {}
    ~slot_type() {}
    std::pair<K      , V> p1;
    std::pair<const K, V> p2;
    K                     key;
};

TEST_CASE("copy", "[table]")
{
    using Map = std::unordered_map<A, int>;
    Map m;

    printf("Initial:\n");
    A::report();
    printf("\n\n");

    using Slot = slot_type<A, int>;
    Slot s1; // {A(1), 2};
    s1.p1 = std::make_pair(A(1), 2);

    A::report();

    // // std::pair<A, int> p{A(1), 2};
    // // printf("After create first pair\n");
    // // A::report();
    // // printf("\n\n");

    // // m.insert(p);
    // m.emplace(1, 2);
    // printf("First insert:\n");
    // A::report();
    // printf("\n\n");
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
