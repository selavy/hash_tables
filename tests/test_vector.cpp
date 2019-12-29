#include <catch2/catch.hpp>
#include <pltables++/vector.h>
#include <stdexcept>
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

    NotTrivial(const NotTrivial& other) : x{ new int(*other.x) } {}

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
}
