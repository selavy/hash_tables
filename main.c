#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "linear_open_addressing.h"
#include "quadratic_open_addressing.h"
#include "qtable.h"

LOA_TABLE(s32, int, int)
LOA_TABLE(s64, int64_t, int*)
QOA_TABLE(s32, int, int)
// QOA_TABLE(s64, int64_t, int*)
uint32_t murmur3_hash_string(const void* s, size_t len) {
    uint32_t out;
    MurmurHash3_x86_32(s, strlen(s), /*seed*/0x42, &out);
    return out;
}
LOA_TABLE_INIT(str, char*, char*, murmur3_hash_string, strcmp, malloc, free)

int main(int argc, char** argv) {
    loa_result_t r;
    loa_iter_t k;
    loa_table_t(str)* t = loa_create(str);

    char* s = strdup("Hello, World");

    r = loa_put(str, t, s);
    assert(r.rc != loa_end(t));

    loa_val(t, r.it) = strdup("Something, else!");

    k = loa_get(str, t, s);
    assert(k != loa_end(t));
    assert(loa_exists(t, k));

    printf("key: %s\n", loa_key(t, k));
    printf("val: %s\n", loa_val(t, k));

    r = loa_put(str, t, strdup("Another key"));
    loa_val(t, r.it) = strdup("Another value!");

    // // XFORM #1:
    // k = loa_begin(str, t);
    // if (k != loa_end(t)) {
    //     do {
    //         printf("%s %s", loa_key(t, k), loa_val(t, k));
    //         k = loa_next(str, t, k);
    //     } while (k != loa_end(t));
    // }

    // // XFORM #2:
    // k = loa_str_next(t, 0);
    // if (k != t->asize) {
    //     do {
    //         printf("%s %s", t->keys[k], t->vals[k]);
    //         k = loa_str_next(str, t, k);
    //     } while (k != t->asize);
    // }

    // // XFORM #3:
    // do { ++k } while (k != t->asize && loa__live(t->msks, k);
    // if (k != t->asize) {
    //     do {
    //         printf("%s %s", t->keys[k], t->vals[k]);
    //         do { ++k } while (k != t->asize && loa__live(t->msks, k);
    //     } while (k != t->asize);
    // }

    for (k = loa_begin(str, t); k != loa_end(t); k = loa_next(str, t, k)) {
        printf("'%s' -> '%s'\n", loa_key(t, k), loa_val(t, k));
    }

    loa_destroy(str, t);

#if 0
    loa_result_t r;
    loa_iter_t   k;
    loa_table_t(s32)*   t = loa_create(s32);

    qresult_t c;
    qiter_t   s;
    qtable*   q = qcreate();

    // ------------------------------------------------------------------------

    assert(loa_size(t) == 0);
    assert(qsize(q)    == 0);

    // ------------------------------------------------------------------------

    for (int i = 0; i < 100; ++i) {
        k = loa_get(s32, t, i);
        assert(k == loa_end(t));

        s = qget(q, i);
        assert(s == qend(t));
    }

    // ------------------------------------------------------------------------

    r = loa_put(s32, t, 0);
    assert(r.rc == 0);
    loa_val(t, r.it) = 0;
    assert(loa_key(t, r.it) == 0);
    assert(loa_size(t) == 1);

    c = qput(q, 0);
    assert(c.rc          == 0);
    qval(q, c.it) = 0;
    assert(qkey(q, c.it) == 0);
    assert(qsize(q)      == 1);

    // ------------------------------------------------------------------------

    r = loa_put(s32, t, 1);
    assert(r.rc == 0);
    loa_val(t, r.it) = 1;
    assert(loa_key(t, r.it) == 1);
    assert(loa_size(t) == 2);

    c = qput(q, 1);
    assert(c.rc == 0);
    qval(q, c.it) = 1;
    assert(qkey(q, c.it) == 1);
    assert(qsize(q) == 2);

    // ------------------------------------------------------------------------

    r = loa_put(s32, t, 1);
    assert(r.rc == 1);
    assert(loa_key(t, r.it) == 1);
    assert(loa_size(t) == 2);

    c = qput(q, 1);
    assert(c.rc == 1);
    assert(qkey(q, c.it) == 1);
    assert(qsize(q) == 2);

    // ------------------------------------------------------------------------

    int N = 100000;
    loa_resize(s32, t, N*2);
    qresize(q, N*2);

    for (int i = 2; i < N; ++i) {
        // if (i % 10000 == 0) {
        //     printf("%d...\n", i);
        // }
        r = loa_put(s32, t, i);
        assert(r.rc             == 0);
        loa_val(t, r.it) = i;
        assert(loa_key(t, r.it) == i);
        assert(loa_size(t)      == i + 1);

        c = qput(q, i);
        assert(c.rc          == 0);
        assert(qkey(q, c.it) == i);
        assert(qsize(q)      == i + 1);
    }

    // ------------------------------------------------------------------------

    loa_destroy(s32, t);
    qdestroy(q);
#endif
    return 0;
}
