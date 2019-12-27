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
QOA_INIT_INT(qoa, int, qoa_i32_hash_identity);
using QoaTable = qoatable_t(qoa)*;
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
    constexpr int M = 128;
    // constexpr int M = 64;
    // std::random_device rd;
    // std::mt19937 gen(rd());
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dist(INT_MIN, INT_MAX);

    StlTable  stl_table;
    KlibTable klib_table = kh_init_klib();
    QoaTable  qoa_table = qoa_create(qoa);

    qoa_resize(qoa, qoa_table, 1 << 10);

    for (int iter_ = 0; iter_ < M; ++iter_) {
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

            // QOA
            auto qoa_result = qoa_insert(qoa, qoa_table, key);
            assert(qoa_result.result != QOA_ERROR);
            assert(stl_result.second == (qoa_result.result != QOA_PRESENT));
            if (stl_result.second) {
                *qoa_val(qoa, qoa_table, qoa_result.iter) = val;
            }
        }

        // sizes should be same
        assert(kh_size(klib_table) == stl_table.size());
        assert(qoa_size(qoa, qoa_table) == stl_table.size());
        std::cout << stl_table.size() << "\t";

        // lookup keys
        for (const auto [key, val] : stl_table) {
            { // STL
                auto iter = stl_table.find(key);
                assert(iter != stl_table.end());
                assert(iter->first  == key);
                assert(iter->second == val);
            }

            { // KLIB
                auto iter = kh_get_klib(klib_table, key);
                assert(iter != kh_end(klib_table));
                assert(kh_key(klib_table, iter) == key);
                assert(kh_val(klib_table, iter) == val);
            }

            { // QOA
                auto iter = qoa_get(qoa, qoa_table, key);
                assert(iter != qoa_end(qoa, qoa_table));
                assert(*qoa_key(qoa, qoa_table, iter) == key);
                assert(*qoa_val(qoa, qoa_table, iter) == val);
            }
        }

        // lookup random keys
        for (int i = 0; i < 10*N; ++i) {
            int key = dist(gen);

            // STL
            auto stl_iter = stl_table.find(key);
            bool stl_found = stl_iter != stl_table.end();

            { // KLIB
                auto iter = kh_get_klib(klib_table, key);
                bool found = iter != kh_end(klib_table);
                assert(found == stl_found);
                if (found) {
                    assert(kh_key(klib_table, iter) == key);
                    assert(kh_val(klib_table, iter) == stl_iter->second);
                }
            }

            { // QOA
                auto iter = qoa_get(qoa, qoa_table, key);
                bool found = iter != qoa_end(qoa, qoa_table);
                assert(found == stl_found);
                if (found) {
                    assert(*qoa_key(qoa, qoa_table, iter) == key);
                    assert(*qoa_val(qoa, qoa_table, iter) == stl_iter->second);
                }
            }
        }

        // delete some keys
        int N2 = N / 10;
        for (int i = 0; i < N2; ++i) {
            auto stl_iter = stl_table.begin();
            assert(stl_iter != stl_table.end());
            int key = stl_iter->first;
            int val = stl_iter->second;

            { // STL
                int result = stl_table.erase(key);
                assert(result == 1);
            }

            { // KLIB
                auto iter = kh_get_klib(klib_table, key);
                assert(iter != kh_end(klib_table));
                kh_del_klib(klib_table, iter);
            }

            { // QOA
                int result = qoa_erase(qoa, qoa_table, key);
                assert(result == 1);
            }
        }

        // sizes should be same
        assert(kh_size(klib_table) == stl_table.size());
        assert(qoa_size(qoa, qoa_table) == stl_table.size());
        std::cout << stl_table.size() << "\n";
    }

    // clean up
    kh_destroy_klib(klib_table);
    qoa_destroy(qoa, qoa_table);

    printf("Passed.\n");

    return 0;
}
