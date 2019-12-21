#include <benchmark/benchmark.h>
#include <pltables/linear_open_address.h>
#include <random>
#include <climits>
#include <iostream>

std::random_device rd;
std::mt19937_64 gen;
bool init = false;

void doinit()
{
    if (!init) {
        gen.seed(rd());
        init = true;
    }
}

using IntPairVec = std::vector<std::pair<int, int>>;

IntPairVec genData(size_t n)
{
    doinit();
    std::uniform_int_distribution<> dist(INT_MIN, INT_MAX);
    std::vector<std::pair<int, int>> vs;
    vs.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        vs.emplace_back(dist(gen), dist(gen));
    }
    return vs;
}

std::vector<int> sampleKeys(const IntPairVec& vs, size_t n)
{
    doinit();
    std::uniform_int_distribution<> dist(0, vs.size() - 1);
    std::vector<int> ks;
    for (size_t i = 0; i < n; ++i) {
        ks.push_back(vs[dist(gen)].first);
    }
    return ks;
}

using Table = loatable<int,int>;

static void insertData(Table& t, const IntPairVec& vs)
{
    for (auto&& v : vs) {
        t.insert(v.first, v.second);
    }
}

static void BM_TableFind(benchmark::State& state)
{
    Table table;
    auto data = genData(state.range(0));
    insertData(table, data);
    for (auto _ : state) {
        state.PauseTiming();
        auto keys = sampleKeys(data, state.range(1));
        state.ResumeTiming();
        for (auto key : keys) {
            benchmark::DoNotOptimize(table.find(key));
        }
    }
}
BENCHMARK(BM_TableFind)
    ->Args({ 1 << 10, 1 << 10 })
    ->Args({ 1 << 11, 1 << 10 })
    ->Args({ 1 << 12, 1 << 10 })
    ->Args({ 1 << 13, 1 << 10 })
    ->Args({ 1 << 14, 1 << 10 })
    ->Args({ 1 << 15, 1 << 10 })
    ;

BENCHMARK_MAIN();
