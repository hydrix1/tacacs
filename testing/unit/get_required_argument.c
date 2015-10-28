/*
 * Unit test program for get_required_argument()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


static int trace_system(const char* cmd)
{
    int result;
    fprintf (stderr, "-- running ``%s''\n", cmd);
    result = system (cmd);
    fprintf (stderr, "    -- returned %d\n", result);
    return result;
}



static void catch_test_get_required_argument(const char* test_name,
                                             const char* value,
                                             const char* prompt)
{
    pid_t child;
    char* file_base = 0;
    char* in_file = 0;
    char* expected_result = 0;
    char* expected_output = 0;
    char* expected_error = 0;
    char* actual_result = 0;
    char* actual_output = 0;
    char* actual_error = 0;
    char* make_actual_dir = 0;
    char* cmp_results = 0;
    char* cmp_outputs = 0;
    char* cmp_errors = 0;

    unity_start_test(test_name);

    asprintf(&file_base, "get_required_argument_%s", test_name);
    asprintf(&in_file, "unit/inputs/%s.input", file_base);
    asprintf(&expected_result, "unit/expect/%s.result", file_base);
    asprintf(&expected_output, "unit/expect/%s.output", file_base);
    asprintf(&expected_error, "unit/expect/%s.error", file_base);
    asprintf(&actual_result, "unit/got/%s.result", file_base);
    asprintf(&actual_output, "unit/got/%s.output", file_base);
    asprintf(&actual_error, "unit/got/%s.error", file_base);
    asprintf(&make_actual_dir, "mkdir -p unit/got");
    asprintf(&cmp_results, "diff -bw %s %s", expected_result, actual_result);
    asprintf(&cmp_outputs, "diff -bw %s %s", expected_output, actual_output);
    asprintf(&cmp_errors, "diff -bw %s %s", expected_error, actual_error);

    trace_system(make_actual_dir);
    fflush(stdout);

    child = fork();
    if (child == -1)
    {
        /* fork() failed! */
        fprintf(stderr, "fork() failed!\n");
        unity_fail();
    }
    else if (child == 0)
    {
        /* we are the child process */
        char* result;

        unlink (actual_result);
        freopen (in_file, "r", stdin);
        freopen (actual_output, "w", stdout);
        freopen (actual_error, "w", stderr);

        optarg = value;
        result = get_required_argument(prompt, test_name, 42);

        /* We were not suppoed to reach here! */
        freopen (actual_result, "w", stdout);
        printf ("%d:%d:%s\n", (int)optarg, result == value, result);
        exit(0);
    }
    else
    {
        /* we are the parent */
        int status = 0;
        int delta;
        int expect_to_work = 0;
        struct stat file_stat;

        /* Do we expect this oprtation to work? */
        delta = stat(expected_result, &file_stat);
        if (delta < 0)
        {
            /* No expected result file ==> expect failure */
             expect_to_work = 0;
        }
        else
        {
            /* There ia an expected result file ==> expect success */
             expect_to_work = 1;
        }

        /* wait for the child to exit */
        pid_t from = waitpid (child, &status, 0);
        /* confirm status is from our child */
        //fprintf(stderr, "waitpid(%d) response from %d\n", child, from);
        unity_assert_int_equal(child, from);
        //fprintf(stderr, "wait() exited with code %d\n", status);
        /* Check exit status is "exited" ...*/
        unity_assert_int_equal(1, WIFEXITED(status));
        /* ... and that the return value is as expected */
        if (expect_to_work)
        {
            unity_assert_int_equal(0, WEXITSTATUS(status));
            delta = trace_system(cmp_results);
            unity_assert_int_equal(0, delta);
        }
        else
        {
            unity_assert_int_not_equal(0, WEXITSTATUS(status));
        }

        /* Confirm the output and error files are as expected */
        delta = trace_system(cmp_outputs);
        unity_assert_int_equal(0, delta);
        delta = trace_system(cmp_errors);
        unity_assert_int_equal(0, delta);
    }

    free(file_base);
    free(in_file);
    free(expected_result);
    free(expected_output);
    free(expected_error);
    free(actual_result);
    free(actual_output);
    free(actual_error);
    free(make_actual_dir);
    free(cmp_results);
    free(cmp_outputs);
    free(cmp_errors);

    unity_end_test();
}



void test_entry()
{
    /* Test get_required_argument() */
    unity_start_group("get_required_argument");
    catch_test_get_required_argument("supplied", "here I am", "no prompt");
    catch_test_get_required_argument("one_word", 0, "one word prompt");
    catch_test_get_required_argument("four_words", 0, "four words prompt");
    catch_test_get_required_argument("padded", 0, "padded prompt");
    catch_test_get_required_argument("spaced", 0, "spaced prompt");
    catch_test_get_required_argument("format", 0, "format %s prompt #%04d");
    catch_test_get_required_argument("eventually", 0, "eventually prompt");
    catch_test_get_required_argument("eof_nothing", 0, "First prompt");
    catch_test_get_required_argument("eof_spaces", 0, "second prompt");
    unity_end_group();
}

