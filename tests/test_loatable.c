#include <cgreen/cgreen.h>
#include <pltables/loatable.h>
#include <string.h>
#include <stdlib.h>
#include <klib/khash.h>

Describe(LOATable);
BeforeEach(LOATable)
{
}
AfterEach(LOATable)
{
}

KHASH_MAP_INIT_INT(m32, int)
typedef khash_t(m32)* bitstate;

Ensure(LOATable, can_set_and_check_flags)
{
    const int N = 8;
    const int fsize = loa_fsize(N);
    flg_t *flags = calloc(fsize, sizeof(*flags));

    /* start as all dead */
    for (int i = 0; i < N; ++i) {
        assert_that(loa_isdead(flags, i), is_true);
        assert_that(loa_islive(flags, i), is_false);
        assert_that(loa_istomb(flags, i), is_false);
    }

    /* set all live */
    for (int i = 0; i < N; ++i) {
        loa_setlive(flags, i);
    }
    for (int i = 0; i < N; ++i) {
        assert_that(loa_isdead(flags, i), is_false);
        assert_that(loa_islive(flags, i), is_true);
        assert_that(loa_istomb(flags, i), is_false);
    }

    /* set all tomb */
    for (int i = 0; i < N; ++i) {
        loa_settomb(flags, i);
    }
    for (int i = 0; i < N; ++i) {
        assert_that(loa_isdead(flags, i), is_false);
        assert_that(loa_islive(flags, i), is_false);
        assert_that(loa_istomb(flags, i), is_true);
    }

    /* set all dead */
    memset(flags, 0, sizeof(*flags)*fsize);
    for (int i = 0; i < N; ++i) {
        assert_that(loa_isdead(flags, i), is_true);
        assert_that(loa_islive(flags, i), is_false);
        assert_that(loa_istomb(flags, i), is_false);
    }

    enum {
        DEAD = 0,
        LIVE = 1,
        TOMB = 2,
        MAXVAL,
    };

    /* randomly set live */
    bitstate bs = kh_init(m32);
    for (int i = 0; i < N; ++i) {
        int ret;
        khiter_t k = kh_put(m32, bs, i, &ret);
        assert_that(ret, is_not_equal_to(-1));
        kh_val(bs, k) = DEAD;
        loa_setdead(flags, i);
    }

    typedef int (*flagfn_t)(const flg_t*, int);
    flagfn_t checkfns[MAXVAL] = {
        &loa_isdead,
        &loa_islive,
        &loa_istomb,
    };

    typedef void (*setfn_t)(flg_t*, int);
    setfn_t setfns[MAXVAL] = {
        &loa_setdead,
        &loa_setlive,
        &loa_settomb,
    };

    srand(42);
    for (int i = 0; i < 1000; ++i) {
        int choice = rand() % MAXVAL;
        int index = rand() % N;
        khiter_t k = kh_get(m32, bs, index);
        assert_that(k, is_not_equal_to(kh_end(bs)));
        assert_that(checkfns[kh_val(bs, k)](flags, index), is_true);
        setfns[choice](flags, index);
        kh_val(bs, k) = choice;
    }

    for (khiter_t k = 0; k < kh_end(bs); ++k) {
        if (!kh_exist(bs, k))
            continue;
        int i = kh_key(bs, k);
        int val = kh_val(bs, k);
        if (val == DEAD) {
            assert_that(loa_isdead(flags, i), is_true);
            assert_that(loa_islive(flags, i), is_false);
            assert_that(loa_istomb(flags, i), is_false);
        } else if (val == LIVE) {
            assert_that(loa_isdead(flags, i), is_false);
            assert_that(loa_islive(flags, i), is_true);
            assert_that(loa_istomb(flags, i), is_false);
        } else if (val == TOMB) {
            assert_that(loa_isdead(flags, i), is_false);
            assert_that(loa_islive(flags, i), is_false);
            assert_that(loa_istomb(flags, i), is_true);
        }
    }
}

TestSuite *loatable_tests()
{
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, LOATable, can_set_and_check_flags);
    return suite;
}
