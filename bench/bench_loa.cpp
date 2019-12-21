#include <benchmark/benchmark.h>
#include <pltables/linear_open_address.h>
#include <klib/khash.h>
#include <unordered_map>
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

using LoaTable = loatable<int,int>;
KHASH_MAP_INIT_INT(i32, int)
using KlibTable = khash_t(i32);
using StlTable = std::unordered_map<int, int>;

static void insertData(LoaTable& t, const IntPairVec& vs)
{
    for (auto&& v : vs) {
        t.insert(v.first, v.second);
    }
}

static void insertData(KlibTable* t, const IntPairVec& vs)
{
    int ret;
    khiter_t it;
    for (auto&& v : vs) {
        it = kh_put(i32, t, v.first, &ret);
        kh_value(t, it) = v.second;
    }
}

static void insertData(StlTable& t, const IntPairVec& vs)
{
    for (auto&& v : vs) {
        t.emplace(v.first, v.second);
    }
}

static bool tableFind(const LoaTable& t, int key)
{
    return t.find(key) != t.end();
}

static bool tableFind(const KlibTable* t, int key)
{
    return kh_get(i32, t, key) != kh_end(t);
}

static bool tableFind(const StlTable& t, int key)
{
    return t.find(key) != t.end();
}

template <class Table>
void tableInit(Table& table) {}

template <>
void tableInit(KlibTable*& table)
{
    table = kh_init(i32);
}

#define TABLE_FIND_ARGS \
    ->Args({ 1 << 10, 1 << 10 }) \
    ->Args({ 1 << 11, 1 << 10 }) \
    ->Args({ 1 << 12, 1 << 10 }) \
    ->Args({ 1 << 13, 1 << 10 }) \
    ->Args({ 1 << 14, 1 << 10 }) \
    ->Args({ 1 << 15, 1 << 10 }) \


template <class Table>
static void BM_LoaTableFind(benchmark::State& state)
{
    Table table;
    tableInit(table);
    auto data = genData(state.range(0));
    insertData(table, data);
    for (auto _ : state) {
        state.PauseTiming();
        auto keys = sampleKeys(data, state.range(1));
        state.ResumeTiming();
        for (auto key : keys) {
            benchmark::DoNotOptimize(tableFind(table, key));
        }
    }
}
BENCHMARK_TEMPLATE(BM_LoaTableFind, LoaTable) TABLE_FIND_ARGS;
BENCHMARK_TEMPLATE(BM_LoaTableFind, KlibTable*) TABLE_FIND_ARGS;
BENCHMARK_TEMPLATE(BM_LoaTableFind, StlTable) TABLE_FIND_ARGS;

BENCHMARK_MAIN();
