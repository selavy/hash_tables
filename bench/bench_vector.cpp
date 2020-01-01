#include <benchmark/benchmark.h>
#include <climits>
#include <iostream>
#include <pltables++/vector.h>
#include <random>
#include <vector>
#include <string>


// #define COPY_ASSIGN_TRIVIAL_TO_LARGER
// #define COPY_ASSIGN_TRIVIAL_TO_SMALLER
#define COPY_ASSIGN_NON_TRIVIAL_TO_LARGER

// append
// pop
// copy
// erase

#define _mkstr(x) #x

#define _assert_helper(cond, linum)                                            \
    do {                                                                       \
        if (!cond) {                                                           \
            char buf[2048];                                                    \
            snprintf(buf, sizeof(buf), "assert failed (%d): %s", linum,        \
                     _mkstr(cond));                                            \
            throw std::runtime_error{ buf };                                   \
        }                                                                      \
    } while (0)

#define myassert(cond) _assert_helper(cond, __LINE__)

#define COPY_ASSIGN_ARGS                                                       \
    ->Arg(1 << 5)                                                              \
      ->Arg(1 << 7)                                                            \
      ->Arg(1 << 9)                                                            \
      ->Arg(1 << 10)                                                           \
//       ->Arg(1 << 12)                                                           \
//       ->Arg(1 << 14)                                                           \
//       ->Arg(1 << 16)                                                           \
//       ->Arg(1 << 18)                                                           \
//       ->Arg(1 << 20)                                                           \
//       ->Arg(1 << 21)

using PltIntVec = plt::Vector<int>;
using StlIntVec = std::vector<int>;
using PltStrVec = plt::Vector<std::string>;
using StlStrVec = std::vector<std::string>;

#ifdef COPY_ASSIGN_TRIVIAL_TO_LARGER
//
// Copy Assign smaller vector to larger vector. Possible to elide memory
// allocation for destination vector.
//
template <class Cont>
static void BM_CopyAssignTriviallyCopyableToLarger(benchmark::State& state)
{
    int N = (int)state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        Cont src(N / 2, 42);
        Cont dst(N    , 55);
        state.ResumeTiming();
        benchmark::DoNotOptimize(dst = src);
    }
}
BENCHMARK_TEMPLATE(BM_CopyAssignTriviallyCopyableToLarger, PltIntVec)
COPY_ASSIGN_ARGS;
BENCHMARK_TEMPLATE(BM_CopyAssignTriviallyCopyableToLarger, StlIntVec)
COPY_ASSIGN_ARGS;
#endif

#ifdef COPY_ASSIGN_TRIVIAL_TO_SMALLER
//
// Copy Assign larger vector to smaller vector. Not possible to elide
// memory allocation for destination vector.
//
template <class Cont>
static void BM_CopyAssignTriviallyCopyableToSmaller(benchmark::State& state)
{
    int N = (int)state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        Cont src(N    , 42);
        Cont dst(N / 2, 55);
        state.ResumeTiming();
        benchmark::DoNotOptimize(dst = src);
    }
}
BENCHMARK_TEMPLATE(BM_CopyAssignTriviallyCopyableToSmaller, PltIntVec)
COPY_ASSIGN_ARGS;
BENCHMARK_TEMPLATE(BM_CopyAssignTriviallyCopyableToSmaller, StlIntVec)
COPY_ASSIGN_ARGS;
#endif

#ifdef COPY_ASSIGN_NON_TRIVIAL_TO_LARGER
//
// Copy Assign non-trivial type from larger vector to smaller vector. Can
// elide the memory allocation for destination vector.
//
template <class Cont>
static void BM_CopyAssignNonTriviallyCopyableToLarger(benchmark::State& state)
{
    std::string astring(27, 'a');    // NOTE: No SSO
    std::string bstring(32, 'b'); // NOTE: No SSO
    int N = (int)state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        Cont src(N / 2, astring);
        Cont dst(N    , bstring);
        state.ResumeTiming();
        benchmark::DoNotOptimize(dst = src);
    }
}
BENCHMARK_TEMPLATE(BM_CopyAssignNonTriviallyCopyableToLarger, PltStrVec)
COPY_ASSIGN_ARGS;
BENCHMARK_TEMPLATE(BM_CopyAssignNonTriviallyCopyableToLarger, StlStrVec)
COPY_ASSIGN_ARGS;
#endif

//
// ----------------------------------------------------------------------------
//
BENCHMARK_MAIN();
