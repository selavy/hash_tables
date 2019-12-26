#include <cgreen/cgreen.h>
#include <pltables/quad_open_address.h>
#include <string.h>

QOA_INIT_INT(i32, int, qoa_i32_hash_identity);

QOA_INIT_STR(str, double, qoa_str_hash_X31);

Describe(QOATable);
BeforeEach(QOATable)
{
}
AfterEach(QOATable)
{
}

Ensure(QOATable, can_create_table_and_insert_values)
{
    qoatable_t(i32) *t = qoa_create(i32);
    assert_that(qoa_size(i32, t), is_equal_to(0));

    {
        qoaresult res = qoa_insert(i32, t, 42);
        assert_that(res.result, is_equal_to(QOA_NEW));
        assert_that(qoa_size(i32, t), is_equal_to(1));
        assert_that(qoa_valid(i32, t, res.iter), is_true);
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

Ensure(QOATable, can_lookup_inserted_values)
{
    int N = 128;
    qoatable_t(i32) *t = qoa_create(i32);
    qoaresult res;
    qoaiter iter;

    for (int i = 0; i < N; ++i) {
        res = qoa_insert(i32, t, i);
        assert_that(res.result, is_not_equal_to(QOA_ERROR));
        assert_that(*qoa_key(i32, t, res.iter), is_equal_to(i));
        *qoa_val(i32, t, res.iter) = i + 1;
    }
    assert_that(qoa_size(i32, t), is_equal_to(N));

    // lookup keys - present
    for (int i = 0; i < N; ++i) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_not_equal_to(qoa_end(i32, t)));
        assert_that(*qoa_key(i32, t, iter), is_equal_to(i));
        assert_that(*qoa_val(i32, t, iter), is_equal_to(i + 1));
    }

    // lookup keys - missing
    for (int i = N; i < 2 * N; ++i) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_equal_to(qoa_end(i32, t)));
    }

    // delete even keys
    for (int i = 0; i < N; i += 2) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_not_equal_to(qoa_end(i32, t)));
        qoa_del(i32, t, iter);

        assert_that(qoa_exist(i32, t, iter), is_false);
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_equal_to(qoa_end(i32, t)));
    }
    assert_that(qoa_size(i32, t), is_equal_to(N / 2));

    // lookup keys - even keys now missing
    for (int i = 0; i < N; ++i) {
        iter = qoa_find(i32, t, i);
        if (i % 2 == 0) {
            assert_that(iter, is_equal_to(qoa_end(i32, t)));
        } else {
            assert_that(iter, is_not_equal_to(qoa_end(i32, t)));
            assert_that(*qoa_key(i32, t, iter), is_equal_to(i));
            assert_that(*qoa_val(i32, t, iter), is_equal_to(i + 1));
        }
    }

    // re-insert even keys
    for (int i = 0; i < N; i += 2) {
        res = qoa_insert(i32, t, i);
        assert_that(res.result, is_not_equal_to(QOA_ERROR));
        *qoa_val(i32, t, res.iter) = i + 3;
    }
    assert_that(qoa_size(i32, t), is_equal_to(N));

    // lookup keys - present
    for (int i = 0; i < N; ++i) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_not_equal_to(qoa_end(i32, t)));
        assert_that(*qoa_key(i32, t, iter), is_equal_to(i));
        if (i % 2 == 0) {
            assert_that(*qoa_val(i32, t, iter), is_equal_to(i + 3));
        } else {
            assert_that(*qoa_val(i32, t, iter), is_equal_to(i + 1));
        }
    }

    // lookup keys - missing
    for (int i = N; i < 2 * N; ++i) {
        iter = qoa_find(i32, t, i);
        assert_that(iter, is_equal_to(qoa_end(i32, t)));
    }

    // erase odd keys
    for (int i = 1; i < N; i += 2) {
        int j = qoa_erase(i32, t, i);
        assert_that(j, is_equal_to(1));
    }
    assert_that(qoa_size(i32, t), is_equal_to(N / 2));

    // erase odd keys - missing
    for (int i = 1; i < N; i += 2) {
        int j = qoa_erase(i32, t, i);
        assert_that(j, is_equal_to(0));
    }

    qoa_destroy(i32, t);
}

void free_string_keys(char** key, double* val)
{
    free(*key);
}

Ensure(QOATable, can_insert_strings_and_lookup)
{
    int N = 8;
    qoatable_t(str) *t = qoa_create(str);
    qoaresult res;
    qoaiter iter;
    char buf[256];
    char* str;
    int ret;

    assert_that(qoa_isempty(str, t), is_true);

    /* insert keys */
    for (int i = 0; i < N; ++i) {
        ret = snprintf(&buf[0], sizeof(buf), "This is a string: %d", i);
        assert_that(ret, is_greater_than(0));
        str = strdup(&buf[0]);
        res = qoa_insert(str, t, str);
        assert_that(res.result, is_not_equal_to(QOA_ERROR));
        assert_that(*qoa_key(str, t, res.iter), is_equal_to_string(buf));
    }

    /* lookup keys - present */
    for (int i = 0; i < N; ++i) {
        ret = snprintf(&buf[0], sizeof(buf), "This is a string: %d", i);
        assert_that(ret, is_greater_than(0));
        iter = qoa_find(str, t, buf);
        assert_that(iter, is_not_equal_to(qoa_end(str, t)));
        assert_that(*qoa_key(str, t, iter), is_equal_to_string(buf));
    }

    /* lookup keys - missing */
    for (int i = N; i < 2*N; ++i) {
        ret = snprintf(&buf[0], sizeof(buf), "This is a string: %d", i);
        assert_that(ret, is_greater_than(0));
        iter = qoa_find(str, t, buf);
        assert_that(iter, is_equal_to(qoa_end(str, t)));
    }

    /* delete even keys */
    for (int i = 0; i < N; i += 2) {
        ret = snprintf(&buf[0], sizeof(buf), "This is a string: %d", i);
        assert_that(ret, is_greater_than(0));
        /* TODO: fix -- leaks memory */
        ret = qoa_erase(str, t, buf);
        assert_that(ret, is_equal_to(1));
    }

    /* lookup deleted keys */
    for (int i = 0; i < N; i += 2) {
        ret = snprintf(&buf[0], sizeof(buf), "This is a string: %d", i);
        assert_that(ret, is_greater_than(0));
        iter = qoa_get(str, t, buf);
        assert_that(iter, is_equal_to(qoa_end(str, t)));
    }

    /* lookup remaining keys */
    for (int i = 1; i < N; i += 2) {
        ret = snprintf(&buf[0], sizeof(buf), "This is a string: %d", i);
        assert_that(ret, is_greater_than(0));
        iter = qoa_get(str, t, buf);
        assert_that(iter, is_not_equal_to(qoa_end(str, t)));
        assert_that(*qoa_key(str, t, iter), is_equal_to_string(buf));
    }

    qoa_destroy2(str, t, free_string_keys);
}

TestSuite *qoatable_tests()
{
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, QOATable, can_create_table_and_insert_values);
    add_test_with_context(suite, QOATable, can_lookup_inserted_values);
    add_test_with_context(suite, QOATable, can_insert_strings_and_lookup);
    return suite;
}
