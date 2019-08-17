#include <stdio.h>
#include <assert.h>
#include "linear_open_addressing.h"

int main(int argc, char** argv) {
    loa_result_t r;
    loa_iter_t k;
    loa_table* t = loa_create();

    assert(loa_size(t) == 0);

    for (int i = 0; i < 100; ++i) {
        k = loa_get(t, i);
        assert(k == loa_end(t));
    }

    r = loa_put(t, 0);
    assert(r.rc == 0);
    loa_val(t, r.it) = 0;
    assert(loa_key(t, r.it) == 0);
    assert(loa_size(t) == 1);

    r = loa_put(t, 1);
    assert(r.rc == 0);
    loa_val(t, r.it) = 1;
    assert(loa_key(t, r.it) == 1);
    assert(loa_size(t) == 2);

    r = loa_put(t, 1);
    assert(r.rc == 1);
    assert(loa_key(t, r.it) == 1);
    assert(loa_size(t) == 2);

    int N = 1000000;
    loa_resize(t, N*2);

    for (int i = 2; i < N; ++i) {
        if (i % 10000 == 0) {
            printf("%d...\n", i);
        }
        r = loa_put(t, i);
        assert(r.rc == 0);
        assert(loa_key(t, r.it) == i);
        assert(loa_size(t) == i + 1);
    }

    loa_destroy(t);
    return 0;
}
