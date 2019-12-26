#include <cgreen/cgreen.h>
#include <pltables/quad_open_address.h>

Describe(QOATable);
BeforeEach(QOATable) {}
AfterEach(QOATable) {}

Ensure(QOATable, qoa_create_table) {
    qoatable* t = qoa_create(i32);
    assert_that(qoa_size(i32, t), is_equal_to(0));
    int ret = qoa_resize(i32, t, 7);
    assert_that(ret, is_equal_to(0));
    /* TODO: remove these asserts */
    assert_that(t->asize, is_greater_than(6));
    assert_that(t->keys, is_not_equal_to(NULL));
    assert_that(t->vals, is_not_equal_to(NULL));
    assert_that(t->flags, is_not_equal_to(NULL));

    {
        qoaresult res = qoa_insert(i32, t, 42);
        assert_that(res.result, is_equal_to(QOA_NEW));
        assert_that(qoa_size(i32, t), is_equal_to(1));
        assert_that(qoa_valid(i32, t, res.iter),     is_true);
        assert_that(qoa_valid(i32, t, res.iter + 1), is_false);
        assert_that(*qoa_key(i32, t, res.iter), is_equal_to(42));
    }

    {
        qoaiter iter;
        qoaresult res = qoa_insert(i32, t, 42);
        assert_that(res.result, is_equal_to(QOA_PRESENT));
        assert_that(qoa_size(i32, t), is_equal_to(1));
        assert_that(qoa_valid(i32, t, res.iter), is_true);
        assert_that(*qoa_key(i32, t, res.iter), is_equal_to(42));
        iter = qoa_get(i32, t, 42);
        assert_that(iter, is_equal_to(res.iter));
        assert_that(*qoa_key(i32, t, iter), is_equal_to(42));
    }

    {
        int res;
        qoaiter iter;
        iter = qoa_put(i32, t, 44, &res);
        assert_that(res, is_equal_to(QOA_NEW));
        assert_that(qoa_size(i32, t), is_equal_to(2));
        assert_that(qoa_valid(i32, t, iter), is_true);
        assert_that(*qoa_key(i32, t, iter), is_equal_to(44));

        iter = qoa_put(i32, t, 44, &res);
        assert_that(res, is_equal_to(QOA_PRESENT));
        assert_that(qoa_size(i32, t), is_equal_to(2));
        assert_that(qoa_valid(i32, t, iter), is_true);
        assert_that(*qoa_key(i32, t, iter), is_equal_to(44));
    }

    qoa_destroy(i32, t);
}

Ensure(QOATable, qoa_lookups) {
    int N = 1024;
    qoatable* t = qoa_create(i32);
    qoaresult res;
    qoaiter iter;
    for (int i = 0; i < N; ++i) {
        res = qoa_insert(i32, t, i);
        assert_that(res.result, is_not_equal_to(QOA_ERROR));
        assert_that(*qoa_key(i32, t, res.iter), is_equal_to(i));
        *qoa_val(i32, t, res.iter) = i + 1;
    }
    // lookup keys - present
    for (int i = 0; i < N; ++i) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_not_equal_to(qoa_end(i32, t)));
        assert_that(*qoa_key(i32, t, iter), is_equal_to(i));
        assert_that(*qoa_val(i32, t, iter), is_equal_to(i + 1));
    }
    // lookup keys - missing
    for (int i = N; i < 2*N; ++i) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_equal_to(qoa_end(i32, t)));
    }
    qoa_destroy(i32, t);
}

TestSuite *qoatable_tests() {
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, QOATable,	qoa_create_table);
    return suite;
}
