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

    qoaresult res = qoa_insert(i32, t, 42);
    assert_that(res.result, is_equal_to(QOA_NEW));
    assert_that(qoa_size(i32, t), is_equal_to(1));

    // assert_that(qoa_valid(i32, t, 0), is_false);
    qoa_destroy(i32, t);
}

TestSuite *qoatable_tests() {
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, QOATable,	qoa_create_table);
    return suite;
}
