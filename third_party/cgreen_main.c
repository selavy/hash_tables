#include <cgreen/cgreen.h>

extern void add_all_tests(TestSuite* suite);

int
main(int argc, char** argv)
{
    TestSuite* suite = create_test_suite();
    add_all_tests(suite);
    if (argc > 1) {
        return run_single_test(suite, argv[1], create_text_reporter());
    }
    return run_test_suite(suite, create_text_reporter());
}
