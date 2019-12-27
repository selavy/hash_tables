#include <iostream>
#include <pltables/quad_open_address.h>
#include <klib/khash.h>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <climits>
#include <cassert>

KHASH_MAP_INIT_INT(klib, int)
using KlibTable = khash_t(klib)*;
using StlTable = std::unordered_map<int, int>;

enum Klib
{
    ERROR   = -1,
    PRESENT =  0,
    EMPTY   =  1,
    DEL     =  2,
};

int main(int argc, char **argv)
{
    constexpr int N = 2056;
    // constexpr int M = 128;
    constexpr int M = 32;
    // std::random_device rd;
    // std::mt19937 gen(rd());
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dist(INT_MIN, INT_MAX);

    StlTable  stl_table;
    KlibTable klib_table = kh_init_klib();

    for (int iter = 0; iter < M; ++iter) {
        // insert keys
        for (int i = 0; i < N; ++i) {
            int key = dist(gen);
            int val = dist(gen);

            // STL
            auto stl_result = stl_table.emplace(key, val);

            // KLIB
            int rc;
            khiter_t klib_iter = kh_put_klib(klib_table, key, &rc);
            assert(rc != Klib::ERROR);
            assert(stl_result.second == (rc != Klib::PRESENT));
            if (stl_result.second) {
                kh_val(klib_table, klib_iter) = val;
            }
        }

        // sizes should be same
        assert(stl_table.size() == kh_size(klib_table));
        std::cout << stl_table.size() << "\t";

        // lookup keys
        for (auto [key, val] : stl_table) {
            // STL
            auto stl_iter = stl_table.find(key);
            assert(stl_iter != stl_table.end());
            assert(stl_iter->first  == key);
            assert(stl_iter->second == val);

            // KLIB
            auto klib_iter = kh_get_klib(klib_table, key);
            assert(klib_iter != kh_end(klib_table));
            assert(kh_key(klib_table, klib_iter) == key);
            assert(kh_val(klib_table, klib_iter) == val);
        }

        // lookup random keys
        for (int i = 0; i < 10*N; ++i) {
            int key = dist(gen);

            // STL
            auto stl_iter = stl_table.find(key);
            bool stl_found = stl_iter != stl_table.end();

            // KLIB
            auto klib_iter = kh_get_klib(klib_table, key);
            bool klib_found = klib_iter != kh_end(klib_table);
            assert(stl_found == klib_found);
        }

        // delete some keys
        int N2 = N / 10;
        for (int i = 0; i < N2; ++i) {
            auto stl_iter = stl_table.begin();
            assert(stl_iter != stl_table.end());
            int key = stl_iter->first;
            int val = stl_iter->second;

            // STL
            int stl_erased = stl_table.erase(key);
            assert(stl_erased == 1);

            // KLIB
            auto klib_iter = kh_get_klib(klib_table, key);
            assert(klib_iter != kh_end(klib_table));
            kh_del_klib(klib_table, klib_iter);
        }

        // sizes should be same
        assert(stl_table.size() == kh_size(klib_table));
        std::cout << stl_table.size() << "\n";
    }

    // clean up
    kh_destroy_klib(klib_table);

    printf("Passed.\n");

    return 0;
}
