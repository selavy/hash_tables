#include <benchmark/benchmark.h>
#include <climits>
#include <iostream>
#include <pltables++/vector.h>
#include <random>
#include <vector>

// append
// pop
// copy
// erase

template <class Cont>
static void BM_VectorCopyAssign(benchmark::State& state)
{
    int N = (int)state.range(0);
    Cont src(N, 42);
    Cont dst(N, 55);
    for (auto _ : state) {
        benchmark::DoNotOptimize(dst = src);
    }
}
#define COPY_ASSIGN_ARGS                                                       \
    ->Arg(1 << 5)                                                              \
      ->Arg(1 << 7)                                                            \
      ->Arg(1 << 9)                                                            \
      ->Arg(1 << 10)                                                           \
      ->Arg(1 << 12)                                                           \
      ->Arg(1 << 14)                                                           \
      ->Arg(1 << 16)                                                           \
      ->Arg(1 << 18)

using PltIntVec = plt::Vector<int>;
using StlIntVec = std::vector<int>;
BENCHMARK_TEMPLATE(BM_VectorCopyAssign, StlIntVec) COPY_ASSIGN_ARGS;
BENCHMARK_TEMPLATE(BM_VectorCopyAssign, PltIntVec) COPY_ASSIGN_ARGS;

BENCHMARK_MAIN();
