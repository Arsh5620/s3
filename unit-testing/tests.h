#include <string.h>

#define BEGIN_TEST                                                                                 \
    struct                                                                                         \
    {                                                                                              \
        int failed_tests;                                                                          \
        int passed_tests;                                                                          \
        char *file_name;                                                                           \
    } global_tests = {0, 0, __FILE__};

#define END_TEST                                                                                   \
    {                                                                                              \
        printf (                                                                                   \
          "\n********\nFor file name \"%s\": \nTotal tests run: %d\n"                              \
          "Tests passed: %d\n"                                                                     \
          "Tests failed: %d\n********\n",                                                          \
          global_tests.file_name,                                                                  \
          global_tests.failed_tests + global_tests.passed_tests,                                   \
          global_tests.passed_tests,                                                               \
          global_tests.failed_tests);                                                              \
    }

#define TEST_NAME_STMT ">> Test name \"%s\" %s (Value expected : %d, Value returned: %d)\n"

#define TEST_PASSED 1
#define TEST_FAILED 0

#define PRINT_TEST_RESULTS(test_name, x, y, success_code)                                          \
    {                                                                                              \
        char *result = "passed";                                                                   \
        if (TEST_FAILED == success_code)                                                           \
        {                                                                                          \
            result = "failed";                                                                     \
        }                                                                                          \
        printf (TEST_NAME_STMT, test_name, result, x, y);                                          \
    }

#define ASSERT_UTEST_UPDATE(compare, x, y, test_name)                                              \
    {                                                                                              \
        if (compare)                                                                               \
        {                                                                                          \
            PRINT_TEST_RESULTS (test_name, x, y, TEST_PASSED);                                     \
            global_tests.passed_tests++;                                                           \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            PRINT_TEST_RESULTS (test_name, x, y, TEST_FAILED);                                     \
            global_tests.failed_tests++;                                                           \
        }                                                                                          \
    }

#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

#define ASSERT_UTEST_PRINTMEM(x_mem, y_mem, x_len, y_len)                                          \
    {                                                                                              \
        puts ("x memory,\ty memory");                                                              \
        for (size_t i = 0; i < (x_len > y_len ? y_len : x_len); i++)                               \
        {                                                                                          \
            int x = *(char *) (x_mem + i), y = *(char *) (y_mem + i);                              \
            printf ("%s%d\t\t%d%s\n", x == y ? "" : COLOR_RED, x, y, COLOR_RESET);                 \
        }                                                                                          \
        puts ("x memory,\ty memory");                                                              \
    }

#define ASSERT_TEST_EQUALS(x, y, test_name)                                                        \
    {                                                                                              \
        ASSERT_UTEST_UPDATE (x == y, x, y, test_name);                                             \
    }

#define ASSERT_TEST_NOTEQUALS(x, y, test_name)                                                     \
    {                                                                                              \
        ASSERT_UTEST_UPDATE (x != y, x, y, test_name);                                             \
    }

#define ASSERT_MEMTEST_EQUALS(x_mem, y_mem, x_len, y_len, test_name)                               \
    {                                                                                              \
        int result = x_len == y_len;                                                               \
        if (result == 1)                                                                           \
        {                                                                                          \
            result = memcmp (x_mem, y_mem, x_len);                                                 \
            result = result == 0;                                                                  \
        }                                                                                          \
        ASSERT_UTEST_UPDATE (result, x_len, y_len, test_name);                                     \
        ASSERT_UTEST_PRINTMEM (x_mem, y_mem, x_len, y_len);                                        \
    }

int
test_fecc ();