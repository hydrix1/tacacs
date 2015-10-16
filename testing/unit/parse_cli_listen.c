/*
 * Unit test program for parse_cli_listen()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


struct test_case
{
    const char*  name;
    int          start_config;
    int          arg_count;
    const char*  arg_values[10];
};



static void dump_spawnd_config (const struct spawnd_config* spawnd)
{
    const struct listen_config* lc;

    printf ("spawnd config:\n");
    if ((spawnd->listen_head == 0) && (spawnd->listen_tail == 0))
    {
        printf ("    listen list is empty\n");
    }
    else if (spawnd->listen_head == spawnd->listen_tail)
    {
        printf ("    listen list has one entry\n");
    }
    else
    {
        printf ("    listen list has multiple entries\n");
    }
    printf ("----\n");

    for (lc = spawnd->listen_head; lc != 0; lc = lc->next_listen)
    {
        printf ("Listen config:\n");
        if (lc->next_listen == spawnd->listen_tail)
        {
            printf ("    next_listen is last\n");
        }
        else if (lc->next_listen == 0)
        {
            printf ("    next_listen is null\n");
        }
        else
        {
            printf ("    next_listen is not null\n");
        }
        printf ("    port = ``%s''\n", lc->port);
        printf ("----\n");
    }
}


static void dump_tacplus_config (const struct tacplus_config* tacplus)
{
    printf ("tacplus config:\n");
    printf ("    debug = ``%s''\n", tacplus->debug_opts);
    printf ("    access = ``%s''\n", tacplus->access_log);
    printf ("    account = ``%s''\n", tacplus->account_log);
    printf ("    key = ``%s''\n", tacplus->secret_key);
    printf ("----\n");

    //dump_host_list (tacplus->host_head, tacplus->host_tail);
    //dump_group_list (tacplus->group_head, tacplus->group_tail);
    //dump_user_list (tacplus->user_head, tacplus->user_tail);
}


static void dump_cli_config (const struct cli_config* cli)
{
    printf ("CLI config:\n");
    printf ("    used  = %d\n", cli->used);
    printf ("    print = %d\n", cli->print);
    printf ("----\n");

    dump_spawnd_config (&cli->spawnd);
    dump_tacplus_config (&cli->tacplus);
}


static int trace_system(const char* cmd)
{
    int result;
    fprintf (stderr, "-- running ``%s''\n", cmd);
    result = system (cmd);
    fprintf (stderr, "    -- returned %d\n", result);
    return result;
}



static void build_initial_context(int format, struct cli_config* context)
{
    memset (context, 0, sizeof(*context));

    if (format == 1)
    {
	struct listen_config* lc = Xcalloc(1, sizeof(*lc));

        lc->port = "99";
        context->spawnd.listen_head = lc;
        context->spawnd.listen_tail = lc;
    }

    if (format == 2)
    {
	struct listen_config* lc1 = Xcalloc(1, sizeof(*lc1));
	struct listen_config* lc2 = Xcalloc(1, sizeof(*lc2));

        lc1->port = "123";
        lc2->port = "456";

        lc1->next_listen = lc2;

        context->spawnd.listen_head = lc1;
        context->spawnd.listen_tail = lc2;
    }
}



static void catch_test_parse_cli_listen(const struct test_case* this_test)
{
    pid_t parent;
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

    unity_start_test(this_test->name);
    parent = getpid();

    asprintf(&file_base, "parse_cli_listen_%s", this_test->name);
    asprintf(&in_file, "unit/inputs/%s.input", file_base);
    asprintf(&expected_result, "unit/expect/%s.result", file_base);
    asprintf(&expected_output, "unit/expect/%s.output", file_base);
    asprintf(&expected_error, "unit/expect/%s.error", file_base);
    asprintf(&actual_result, "unit/got_%d/%s.result", parent, file_base);
    asprintf(&actual_output, "unit/got_%d/%s.output", parent, file_base);
    asprintf(&actual_error, "unit/got_%d/%s.error", parent, file_base);
    asprintf(&make_actual_dir, "mkdir -p unit/got_%d", parent);
    asprintf(&cmp_results, "diff -bw %s %s", expected_result, actual_result);
    asprintf(&cmp_outputs, "diff -bw %s %s", expected_output, actual_output);
    asprintf(&cmp_errors, "diff -bw %s %s", expected_error, actual_error);

    trace_system(make_actual_dir);

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
        struct cli_config  context;
        int                next_sym;

        unlink (actual_result);
        freopen (in_file, "r", stdin);
        freopen (actual_output, "w", stdout);
        freopen (actual_error, "w", stderr);

        opts_argc = this_test->arg_count;
        opts_argv = (char**) this_test->arg_values;
        optind = 0;
        build_initial_context(this_test->start_config, &context);
        next_sym = parse_cli_listen(&context);

        /* We may nor may not get here! */
        freopen (actual_result, "w", stdout);
        printf ("Next symbol = %d\n", next_sym);
        dump_cli_config (&context);
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



static const struct test_case case_table[] =
{
    { "null",         0, 1, { "prog", 0 } },
    { "nothing",      0, 2, { "prog", "--user", 0 } },
    { "prompted",     0, 2, { "prog", "--user", 0 } },
    { "correct",      0, 3, { "prog", "--listen_port=9876", "--user", 0 } },
    { "no_value",     0, 3, { "prog", "--listen_port", "--user", 0 } },
    { "duplicate",    0, 4, { "prog", "--listen_port=9876", "--listen_port=1234", "--user", 0 } },
    { "two_good",     1, 3, { "prog", "--listen_port=1234", "--user", 0 } },
    { "three_good",   2, 3, { "prog", "--listen_port=1234", "--user", 0 } },
    { 0 }
};



void test_entry()
{
    int test_index;

    /* Test parse_cli_listen() */
    unity_start_group("parse_cli_listen");
    for (test_index = 0; case_table[test_index].name != 0; ++test_index)
    {
        catch_test_parse_cli_listen(&case_table[test_index]);
    }
    unity_end_group();
}

