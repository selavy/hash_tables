#include <stdio.h>
#include <assert.h>
#include "linear_open_addressing.h"
#include "quadratic_open_addressing.h"
#include "qtable.h"

LOA_TABLE(s32, int, int)
QOA_TABLE(s32, int, int)

int main(int argc, char** argv) {
    loa_result_t r;
    loa_iter_t   k;
    loa_table(s32)*   t = loa_create(s32);

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
    return 0;
}
