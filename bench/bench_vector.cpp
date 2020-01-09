#include <benchmark/benchmark.h>
#include <climits>
#include <iostream>
#include <pltables++/vector.h>
#include <random>
#include <string>
#include <vector>

#define APPEND_BENCHMARKS
// #define COPY_BENCHMARKS
// #define ERASE_BENCHMARKS

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

using PltIntVec = plt::Vector<int>;
using StlIntVec = std::vector<int>;
using PltStrVec = plt::Vector<std::string>;
using StlStrVec = std::vector<std::string>;

#ifdef COPY_BENCHMARKS
//----------------------------------------------------------------------------//
//
// Copy Benchmarks
//
//----------------------------------------------------------------------------//

// #define COPY_ASSIGN_TRIVIAL_TO_LARGER
// #define COPY_ASSIGN_TRIVIAL_TO_SMALLER
#define COPY_ASSIGN_NON_TRIVIAL_TO_LARGER
#define COPY_ASSIGN_NON_TRIVIAL_TO_SMALLER

// clang-format off
#define COPY_ASSIGN_ARGS                                                       \
    ->Arg(1 << 5)                                                              \
    ->Arg(1 << 7)                                                              \
    ->Arg(1 << 9)                                                              \
    ->Arg(1 << 10)                                                             \
    ->Arg(1 << 12)                                                             \
//       ->Arg(1 << 14)                                                           \
//       ->Arg(1 << 16)                                                           \
//       ->Arg(1 << 18)                                                           \
//       ->Arg(1 << 20)                                                           \
//       ->Arg(1 << 21)
// clang-format on

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
        Cont dst(N, 55);
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
        Cont src(N, 42);
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
// Copy Assign non-trivial type from smaller vector to larger vector. Can
// elide the memory allocation for destination vector.
//
template <class Cont>
static void BM_CopyAssignNonTriviallyCopyableToLarger(benchmark::State& state)
{
    std::string astring(27, 'a'); // NOTE: No SSO
    std::string bstring(32, 'b'); // NOTE: No SSO
    int N = (int)state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        Cont src(N / 2, astring);
        Cont dst(N, bstring);
        assert(src.size() < dst.size());
        state.ResumeTiming();
        benchmark::DoNotOptimize(dst = src);
    }
}
BENCHMARK_TEMPLATE(BM_CopyAssignNonTriviallyCopyableToLarger, PltStrVec)
COPY_ASSIGN_ARGS;
BENCHMARK_TEMPLATE(BM_CopyAssignNonTriviallyCopyableToLarger, StlStrVec)
COPY_ASSIGN_ARGS;
#endif

#ifdef COPY_ASSIGN_NON_TRIVIAL_TO_SMALLER
//
// Copy Assign non-trivial type from larger vector to smaller vector. Can
// elide the memory allocation for destination vector.
//

// TODO: not sure this is actually a great baseline
// static void BM_CopyAssignNonTriviallyCopyableToSmaller_Baseline(
//   benchmark::State& state)
// {
//     int N = (int)state.range(0);
//     std::vector<char> src(N * sizeof(std::string), '4');
//     std::vector<char> dst(N * sizeof(std::string), ' ');
//     for (auto _ : state) {
//         // benchmark::DoNotOptimize(dst = src);
//         memcpy(&dst[0], &src[0], sizeof(std::string) * N);
//     }
// }
// BENCHMARK(BM_CopyAssignNonTriviallyCopyableToSmaller_Baseline)
// COPY_ASSIGN_ARGS;

template <class Cont>
static void BM_CopyAssignNonTriviallyCopyableToSmaller(benchmark::State& state)
{
    std::string astring(27, 'a'); // NOTE: No SSO
    std::string bstring(32, 'b'); // NOTE: No SSO
    int N = (int)state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        Cont src(N, astring);
        Cont dst(N / 2, bstring);
        assert(src.size() < dst.size());
        state.ResumeTiming();
        benchmark::DoNotOptimize(dst = src);
    }
}
BENCHMARK_TEMPLATE(BM_CopyAssignNonTriviallyCopyableToSmaller, PltStrVec)
COPY_ASSIGN_ARGS;
BENCHMARK_TEMPLATE(BM_CopyAssignNonTriviallyCopyableToSmaller, StlStrVec)
COPY_ASSIGN_ARGS;

#endif // COPY_ASSIGN_NON_TRIVIAL_TO_SMALLER

#endif // COPY_BENCHMARKS

#ifdef APPEND_BENCHMARKS
//----------------------------------------------------------------------------//
//
// Append Benchmarks
//
//----------------------------------------------------------------------------//

#define APPEND_TRIVIAL

// clang-format off
#define APPEND_ARGS                                                            \
    ->Args({ 128, 128 })                                                       \
    ->Args({ 128, 256 })                                                       \
    ->Args({ 256, 256 })                                                       \
    ->Args({ 1024, 32 })
// clang-format on

#ifdef APPEND_TRIVIAL
template <class Cont>
static void BM_AppendTrivial(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        Cont vec(42, state.range(0));
        state.ResumeTiming();
        for (int i = 0; i < state.range(1); ++i) {
            vec.push_back(i);
        }
    }
}
static void BM_AppendTrivial_Unsafe(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        PltIntVec vec(42, state.range(0));
        vec.reserve(vec.size() + state.range(1));
        state.ResumeTiming();
        for (int i = 0; i < state.range(1); ++i) {
            vec.append_unsafe(i);
        }
    }
}
BENCHMARK_TEMPLATE(BM_AppendTrivial, PltIntVec) APPEND_ARGS;
BENCHMARK_TEMPLATE(BM_AppendTrivial, StlIntVec) APPEND_ARGS;
BENCHMARK(BM_AppendTrivial_Unsafe) APPEND_ARGS;
#endif // APPEND_TRIVIAL

#endif // APPEND_BENCHMARKS

#ifdef ERASE_BENCHMARKS
//----------------------------------------------------------------------------//
//
// Erase Benchmarks
//
//----------------------------------------------------------------------------//

#define ERASE_ARGS ->Arg(128)->Arg(256)->Arg(1024)->Arg(2048)->Arg(4096)->Arg(1<<16)
template <class Cont>
static void BM_EraseTrivial(benchmark::State& state)
{
    size_t ii = 0;
    for (auto _ : state) {
        state.PauseTiming();
        Cont vec(state.range(0), 42);
        state.ResumeTiming();
        benchmark::DoNotOptimize(vec.erase(vec.begin() + 1, vec.end() - 1));
        ii += vec.size();
    }
    if (ii == 0) {
        throw std::runtime_error{"failed"};
    }
}
BENCHMARK_TEMPLATE(BM_EraseTrivial, PltIntVec) ERASE_ARGS;
BENCHMARK_TEMPLATE(BM_EraseTrivial, StlIntVec) ERASE_ARGS;

template <class Cont>
static void BM_EraseNoThrowMove(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        Cont vec(state.range(0), "Hello World");
        state.ResumeTiming();
        benchmark::DoNotOptimize(vec.erase(vec.begin() + 1, vec.end() - 1));
    }
}
BENCHMARK_TEMPLATE(BM_EraseNoThrowMove, PltStrVec) ERASE_ARGS;
BENCHMARK_TEMPLATE(BM_EraseNoThrowMove, StlStrVec) ERASE_ARGS;

#endif // ERASE_BENCHMARKS

//
// -----------------------------------------------------------------------------
//
BENCHMARK_MAIN();
