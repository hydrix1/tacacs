/*
 * Unit test program for get_missing_argument()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


static const char* call_get_missing_argument(const char* prompt, ...)
{
    char* result;
    va_list args;

    va_start (args, prompt);
    result = get_missing_argument(prompt, args);
    va_end (args);

    return result;
}



static int trace_system(const char* cmd)
{
    int result;
    fprintf (stderr, "-- running ``%s''\n", cmd);
    result = system (cmd);
    fprintf (stderr, "    -- returned %d\n", result);
    return result;
}



static int catch_test_get_missing_argument(const char* file_base,
                                           const char* prompt, ...)
{
    pid_t parent;
    pid_t child;
    int   result = 0;
    char* in_file = 0;
    char* expected_output = 0;
    char* expected_error = 0;
    char* actual_output = 0;
    char* actual_error = 0;
    char* make_actual_dir = 0;
    char* cmp_outputs = 0;
    char* cmp_errors = 0;

    parent = getpid();

    asprintf(&in_file, "unit/inputs/%s.input", file_base);
    asprintf(&expected_output, "unit/expect/%s.output", file_base);
    asprintf(&expected_error, "unit/expect/%s.error", file_base);
    asprintf(&actual_output, "unit/got_%d/%s.output", parent, file_base);
    asprintf(&actual_error, "unit/got_%d/%s.error", parent, file_base);
    asprintf(&make_actual_dir, "mkdir -p unit/got_%d", parent);
    asprintf(&cmp_outputs, "diff -bw %s %s", expected_output, actual_output);
    asprintf(&cmp_errors, "diff -bw %s %s", expected_error, actual_error);

    trace_system(make_actual_dir);

    child = fork();
    if (child == -1)
    {
        /* fork() failed! */
        fprintf(stderr, "fork() failed!\n");
        unity_fail();
        result = -1;
    }
    else if (child == 0)
    {
        /* we are the child process */
        char* result;
        va_list args;

        freopen (in_file, "r", stdin);
        freopen (actual_output, "w", stdout);
        freopen (actual_error, "w", stderr);

        va_start (args, prompt);
        result = get_missing_argument(prompt, args);
        va_end (args);

        // We were not suppoed to reach here!
        exit(0);
    }
    else
    {
        /* we are the parent */
        int status = 0;
        int delta;
        pid_t from = waitpid (child, &status, 0);
        fprintf(stderr, "waitpid(%d) response from %d\n", child, from);
        unity_assert_int_equal(child, from);
        fprintf(stderr, "wait() exited with code %d\n", status);
        /* Check exit status is "exited" ...*/
        unity_assert_int_equal(1, WIFEXITED(status));
        /* ... and that the return value is non-zero */
        unity_assert_int_not_equal(0, WEXITSTATUS(status));
        /* Confirm the output and error files are as expected */
        delta = trace_system(cmp_outputs);
        unity_assert_int_equal(0, delta);
        delta = trace_system(cmp_errors);
        unity_assert_int_equal(0, delta);
    }

    return result;
}



void test_get_missing_argument_eof()
{
    static const char* prompt1 = "First prompt";
    static const char* file_1 =  "get_missing_argument_eof_nothing";
    int result;

    unity_start_group("EOF");

    unity_start_test("nothing");
    result = catch_test_get_missing_argument(file_1, prompt1);
    //unity_assert_ptr_not_equal (base_1, 0);
   //unity_assert_ptr_not_equal (base_1, easy);
    //unity_assert_str_equal (base_1, easy);
    unity_end_test();

    unity_end_group();
}



void test_get_missing_argument()
{
    unity_start_group("get_missing_argument");
    test_get_missing_argument_eof();
    unity_end_group();
}



void test_entry()
{
    test_get_missing_argument();
}

