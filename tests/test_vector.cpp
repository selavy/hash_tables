#include <catch2/catch.hpp>
#include <initializer_list>
#include <pltables++/vector.h>
#include <stdexcept>
#include <string>
#include <vector>

using namespace plt;

TEST_CASE("Vector - construction")
{
    SECTION("Default construct empty vector")
    {
        Vector<int> vec;
        REQUIRE(vec.size() == 0);
        REQUIRE(vec.capacity() == 0);
    }

    SECTION("Default construct N IsTrivial elements")
    {
        Vector<int> vec{ 6, 42 };
        REQUIRE(vec.size() == 6);
        REQUIRE(vec.capacity() >= 6);
        for (int i = 0; i < vec.size(); ++i) {
            REQUIRE(vec[i] == 42);
        }

        const Vector<int>& cvec = vec;
        for (int i = 0; i < cvec.size(); ++i) {
            REQUIRE(cvec[i] == 42);
        }
    }

    SECTION("Copy constructor")
    {
        Vector<int> v1{ 6, 44 };
        Vector<int> v2{ v1 };
        REQUIRE(v1.size() == 6);
        REQUIRE(v2.size() == 6);
        for (int i = 0; i < v2.size(); ++i) {
            REQUIRE(v2[i] == 44);
            REQUIRE(v1[i] == 44);
        }
    }

    SECTION("Move constructor")
    {
        Vector<int> v1{ 6, 44 };
        Vector<int> v2{ std::move(v1) };
        REQUIRE(v1.size() == 0);
        REQUIRE(v2.size() == 6);
        for (int i = 0; i < v2.size(); ++i) {
            REQUIRE(v2[i] == 44);
        }
    }

    SECTION("Copy assignment")
    {
        Vector<int> v1{ 5, 41 };
        Vector<int> v2{ 10, 1 };

        REQUIRE(v1.size() == 5);
        REQUIRE(v2.size() == 10);
        REQUIRE(v1[0] == 41);
        REQUIRE(v2[0] == 1);

        v2 = v1;
        REQUIRE(v1.size() == 5);
        REQUIRE(v2.size() == 5);
        for (int i = 0; i < v2.size(); ++i) {
            REQUIRE(v2[i] == 41);
            REQUIRE(v1[i] == 41);
        }
    }

    SECTION("Move assignment")
    {
        Vector<int> v1{ 5, 41 };
        Vector<int> v2{ 10, 1 };

        REQUIRE(v1.size() == 5);
        REQUIRE(v2.size() == 10);
        REQUIRE(v1[0] == 41);
        REQUIRE(v2[0] == 1);

        v2 = std::move(v1);
        REQUIRE(v1.size() == 0);
        REQUIRE(v2.size() == 5);
        for (int i = 0; i < v2.size(); ++i) {
            REQUIRE(v2[i] == 41);
        }
    }
}

TEST_CASE("Vector - append")
{
    Vector<int> vec;
    constexpr int N = 1024;
    REQUIRE(vec.is_empty() == true);
    for (int i = 0; i < N; ++i) {
        REQUIRE(vec.size() == i);
        vec.append(i);
    }
    REQUIRE(vec.size() == N);

    for (int i = 0; i < N; ++i) {
        REQUIRE(vec[i] == i);
    }

    for (int i = 0; i < N / 2; ++i) {
        vec.pop();
    }
    REQUIRE(vec.size() == N / 2);

    for (int i = 0; i < N / 2; ++i) {
        vec.append(N / 2 + i + 1);
    }
    REQUIRE(vec.size() == N);

    for (int i = 0; i < N; ++i) {
        if (i < N / 2) {
            REQUIRE(vec[i] == i);
        } else {
            REQUIRE(vec[i] == i + 1);
        }
    }

    vec.shrink_to_fit();
    REQUIRE(vec.size() == vec.capacity());

    SECTION("Iterators")
    {
        Vector<int>::iterator itr = vec.begin();
        REQUIRE(itr != vec.end());
        REQUIRE(*itr == 0);
        itr += 3;
        REQUIRE(*itr == 3);

        itr = vec.begin();
        Vector<int>::iterator itr2 = itr + 3;
        REQUIRE(*itr2 == 3);
        REQUIRE(*itr == 0);
    }

    SECTION("ConstIterator")
    {
        Vector<int>& cvec = vec;
        Vector<int>::const_iterator it = cvec.cbegin();
        REQUIRE(it == cvec.begin());
        REQUIRE(it != cvec.end());
        REQUIRE(*it == 0);
        REQUIRE(*++it == 1);
        REQUIRE(*it++ == 1);
        REQUIRE(*++it == 3);
    }

    std::vector<int> other;
    for (auto it : vec) {
        other.push_back(it);
    }
    REQUIRE(vec.size() == (int)other.size());
    for (int i = 0; i < vec.size(); ++i) {
        REQUIRE(vec[i] == other[i]);
    }
}

struct NoThrowMove
{
    NoThrowMove(int i)
      : x{ new int(i) }
    {
    }

    NoThrowMove(const NoThrowMove& other) noexcept : x{ new int(*other.x) } {}

    ~NoThrowMove() noexcept { delete x; }

    NoThrowMove& operator=(NoThrowMove&& other) noexcept
    {
        x = other.x;
        other.x = nullptr;
        return *this;
    }

    NoThrowMove& operator=(const NoThrowMove& other)
    {
        throw std::runtime_error{ "No copy assign" };
    }

    int& operator*() { return *x; }

    int* x = nullptr;
};

struct NoThrowCopy
{
    NoThrowCopy(int i)
      : x{ new int(i) }
    {
    }

    NoThrowCopy(const NoThrowCopy& other) noexcept : x{ new int(*other.x) } {}

    ~NoThrowCopy() noexcept { delete x; }

    NoThrowCopy& operator=(const NoThrowCopy& other) noexcept
    {
        x = new int(*other.x);
        return *this;
    }

    NoThrowCopy& operator=(NoThrowCopy&& other)
    {
        throw std::runtime_error{ "No move assign" };
    }

    int& operator*() { return *x; }

    int* x = nullptr;
};

struct NotTrivial
{
    NotTrivial(int i)
      : x{ new int(i) }
    {
    }

    NotTrivial(const NotTrivial& other)
      : x{ new int(*other.x) }
    {
    }

    ~NotTrivial() noexcept { delete x; }

    NotTrivial& operator=(const NotTrivial& other) noexcept
    {
        x = new int(*other.x);
        return *this;
    }

    NotTrivial& operator=(NotTrivial&& other)
    {
        throw std::runtime_error{ "No move assign" };
    }

    int& operator*() { return *x; }

    int* x = nullptr;
};

TEST_CASE("Vector non-trivial types")
{
    SECTION("No throw move")
    {
        Vector<NoThrowMove> vec;
        for (int i = 0; i < 128; ++i) {
            REQUIRE(vec.size() == i);
            vec.append(i);
            REQUIRE(*vec[i] == i);
        }
    }

    SECTION("No throw copy")
    {
        Vector<NoThrowCopy> vec;
        for (int i = 0; i < 128; ++i) {
            REQUIRE(vec.size() == i);
            vec.append(i);
            REQUIRE(*vec[i] == i);
        }
    }

    SECTION("Throwable Copy")
    {
        Vector<NotTrivial> vec;
        for (int i = 0; i < 128; ++i) {
            REQUIRE(vec.size() == i);
            vec.append(i);
            REQUIRE(*vec[i] == i);
        }
    }

    SECTION("copy assign std::string larger to smaller")
    {
        std::string astr("aaaaaaaaaaaaaaaaaaaaaaaaaaa");     // No SSO
        std::string bstr("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"); // No SSO
        int N1 = 99;
        int N2 = 42;
        Vector<std::string> src(N1, astr);
        Vector<std::string> dst(N2, bstr);
        REQUIRE(src.size() == N1);
        REQUIRE(dst.size() == N2);
        REQUIRE(src[0] == astr);
        REQUIRE(src[src.size() - 1] == astr);
        REQUIRE(dst[0] == bstr);
        REQUIRE(dst[dst.size() - 1] == bstr);

        dst = src;
        REQUIRE(src.size() == dst.size());
        REQUIRE(dst.size() == N1);
        for (int i = 0; i < src.size(); ++i) {
            REQUIRE(dst[i] == src[i]);
        }
    }

    SECTION("copy assign std::string smaller to larger")
    {
        std::string astr("aaaaaaaaaaaaaaaaaaaaaaaaaaa");     // No SSO
        std::string bstr("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"); // No SSO
        int N1 = 42;
        int N2 = 99;
        Vector<std::string> src(N1, astr);
        Vector<std::string> dst(N2, bstr);
        REQUIRE(src.size() == N1);
        REQUIRE(dst.size() == N2);
        REQUIRE(src[0] == astr);
        REQUIRE(src[src.size() - 1] == astr);
        REQUIRE(dst[0] == bstr);
        REQUIRE(dst[dst.size() - 1] == bstr);

        dst = src;
        REQUIRE(src.size() == dst.size());
        REQUIRE(dst.size() == N1);
        for (int i = 0; i < src.size(); ++i) {
            REQUIRE(dst[i] == src[i]);
        }
    }
}

TEST_CASE("Erase")
{
    SECTION("Erase single trivial")
    {
        Vector<int> vec;
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i);
        }
        REQUIRE(vec.size() == 10);

        // remove 5
        Vector<int>::iterator iter = vec.erase(vec.begin() + 5);
        REQUIRE(vec.size() == 9);
        REQUIRE(*iter == 6);
        REQUIRE(vec[5] == 6);

        // remove 9
        iter = vec.erase(vec.end() - 1);
        REQUIRE(vec.size() == 8);
        REQUIRE(iter == vec.end());

        // remove 0
        iter = vec.erase(vec.begin());
        REQUIRE(vec.size() == 7);
        REQUIRE(*iter == 1);

        std::vector<int> expect = { 1, 2, 3, 4, 6, 7, 8 };
        REQUIRE(vec.size() == expect.size());
        for (int i = 0; i < vec.size(); ++i) {
            REQUIRE(vec[i] == expect[i]);
        }
    }

    SECTION("Erase single non-trivial")
    {
        Vector<std::string> vec;
        for (int i = 0; i < 10; ++i) {
            vec.push_back(std::to_string(i));
        }
        REQUIRE(vec.size() == 10);

        // remove 5
        Vector<std::string>::iterator iter = vec.erase(vec.begin() + 5);
        REQUIRE(vec.size() == 9);
        REQUIRE(*iter == "6");
        REQUIRE(vec[5] == "6");

        // remove 9
        iter = vec.erase(vec.end() - 1);
        REQUIRE(vec.size() == 8);
        REQUIRE(iter == vec.end());

        // remove 0
        iter = vec.erase(vec.begin());
        REQUIRE(vec.size() == 7);
        REQUIRE(*iter == "1");

        std::vector<std::string> expect = { "1", "2", "3", "4", "6", "7", "8" };
        REQUIRE(vec.size() == expect.size());
        for (int i = 0; i < vec.size(); ++i) {
            REQUIRE(vec[i] == expect[i]);
        }
    }

    SECTION("std::vector erase range")
    {
        std::vector<int> vs = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        auto iter = vs.erase(vs.begin() + 2, vs.begin() + 7);
        REQUIRE(vs[0] == 0);
        REQUIRE(vs[1] == 1);
        REQUIRE(vs[2] == 7);
        REQUIRE(vs[3] == 8);
        REQUIRE(vs[4] == 9);
        REQUIRE(*iter == 7);
    }

    SECTION("erase range trivial")
    {
        static_assert(std::is_trivial_v<int>);
        auto vec = Vector<int>::make({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
        REQUIRE(vec.size() == 10);

        // expect { 0, 1, 7, 8, 9 }
        Vector<int>::iterator iter =
          vec.erase(vec.begin() + 2, vec.begin() + 7);
        REQUIRE(vec.size() == 5);
        REQUIRE(vec[0] == 0);
        REQUIRE(vec[1] == 1);
        REQUIRE(vec[2] == 7);
        REQUIRE(vec[3] == 8);
        REQUIRE(vec[4] == 9);
        REQUIRE(*iter == 7);
    }

    SECTION("erase range is_nothrow_move_assignable")
    {
        static_assert(std::is_nothrow_move_assignable_v<std::string>);
        Vector<std::string> vec;
        for (int i = 0; i < 10; ++i) {
            vec.append(std::to_string(i));
        }

        // start  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
        // expect { 0, 1, 7, 8, 9 }
        Vector<std::string>::iterator iter =
          vec.erase(vec.begin() + 2, vec.begin() + 7);
        REQUIRE(vec.size() == 5);
        REQUIRE(vec[0] == "0");
        REQUIRE(vec[1] == "1");
        REQUIRE(vec[2] == "7");
        REQUIRE(vec[3] == "8");
        REQUIRE(vec[4] == "9");
        printf("%zu\n", (&*iter - &*vec.begin()));
        REQUIRE(*iter == "7");
    }
}
