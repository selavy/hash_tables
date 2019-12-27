#include <cgreen/cgreen.h>

extern TestSuite *qoatable_tests();
extern TestSuite *loatable_tests();

void add_all_tests(TestSuite *suite)
{
    add_suite(suite, qoatable_tests());
    add_suite(suite, loatable_tests());
}
