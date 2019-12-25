#include <cgreen/cgreen.h>
#include <pltables/quad_open_address.h>

Describe(QOATable);
BeforeEach(QOATable) {}
AfterEach(QOATable) {}

Ensure(QOATable, qoa_create_table) {
    assert_that(1, is_equal_to(1));
    assert_that(1, is_equal_to(2));
    qoatable* t = qoatable_create();
    qoatable_destroy(t);
}

TestSuite *qoatable_tests() {
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, QOATable,	qoa_create_table);
    return suite;
}
