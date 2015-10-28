/*
 * Unit test program for get_next_option()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


struct test_set
{
    const char*       test_name;
    /* Inpurs to the test */
    int               argc;
    const char*       argv[10];
    /* Expected outputs from the test */
    int               count;
    int               opt_codes[10];
    const char*       values[10];
};

static const char* none = "--none--";


static struct test_set test_data_basic[] =
{
    { "zero",           0, { "nothing" },
                        0, { 0 } },
    { "blank",          1, { "prog.exe" },
                        0, { 0 } },
    { "one_word",       2, { "prog.exe", "something", "else" },
                        0, { 0 } },
    { "one_short",      2, { "-P", "-h", "-d" },
                        1, { 'h' } },
    { "two_short",      3, { "-P", "-h", "-v", "-d" },
                        2, { 'h', 'v' } },
    { "three_short",    4, { "-P", "-h", "-v", "-b", "-d" },
                        3, { 'h', 'v', 'b' } },
    { "merge_short",    2, { "-P", "-hvb", "-d" },
                        3, { 'h', 'v', 'b' } },
    { "one_long",       2, { "-P", "--help", "-d" },
                        1, { 'h' } },
    { "two_long",       3, { "-P", "--help", "--version", "-d" },
                        2, { 'h', 'v' } },
    { "three_long",     4, { "-P", "--help", "--version", "--background", "-d" },
                        3, { 'h', 'v', 'b' } },
    { "three_mixed",    4, { "-P", "--help", "-v", "--background", "-d" },
                        3, { 'h', 'v', 'b' } },
    { "one_unknown",    4, { "-P", "-h", "-Z", "-b", "-d" },
                        3, { 'h', '?', 'b' } },
    { "one_bad",        4, { "-P", "-h", "--ZOMBIE", "-b", "-d" },
                        3, { 'h', '?', 'b' } },
    { 0 }
};


static struct test_set test_data_plain[] =
{
    { "h",              2, { "-P", "-h", "-d" },
                        1, { 'h' } },
    { "v",              2, { "-P", "-v", "-d" },
                        1, { 'v' } },
    { "P",              2, { "-v", "-P", "-d" },
                        1, { 'P' } },
    { "1",              2, { "-P", "-1", "-d" },
                        1, { '1' } },
    { "f",              2, { "-P", "-f", "-d" },
                        1, { 'f' } },
    { "b",              2, { "-P", "-b", "-d" },
                        1, { 'b' } },
    { "help",           2, { "-P", "--help", "-d" },
                        1, { 'h' } },
    { "version",        2, { "-P", "--version", "-d" },
                        1, { 'v' } },
    { "check",          2, { "-P", "--check", "-d" },
                        1, { 'P' } },
    { "degraded",       2, { "-P", "--degraded", "-d" },
                        1, { '1' } },
    { "foreground",     2, { "-P", "--foreground", "-d" },
                        1, { 'f' } },
    { "background",     2, { "-P", "--background", "-d" },
                        1, { 'b' } },
    { "print",          2, { "-P", "--print", "-d" },
                        1, { lopt_print } },
    { "listen",         2, { "-P", "--listen", "-d" },
                        1, { lopt_listen } },
    { "user_junos",     2, { "-P", "--user_junos", "-d" },
                        1, { lopt_user_junos } },
    { 0 }
};


static struct test_set test_data_short_unwanted[] =
{
    { "h",              3, { "-P", "-h=v", "-i", "-d" },
                        4, { 'h', '?', 'v', 'i' } },
    { "v",              3, { "-P", "-v=P", "-i", "-d" },
                        4, { 'v', '?', 'P', 'i' } },
    { "P",              3, { "-v", "-P=1", "-i", "-d" },
                        4, { 'P', '?', '1', 'i' } },
    { "1",              3, { "-P", "-1=f", "-i", "-d" },
                        4, { '1', '?', 'f', 'i' } },
    { "f",              3, { "-P", "-f=b", "-i", "-d" },
                        4, { 'f', '?', 'b', 'i' } },
    { "b",              3, { "-P", "-b=h", "-i", "-d" },
                        4, { 'b', '?', 'h', 'i' } },
    { 0 }
};


static struct test_set test_data_long_unwanted[] =
{
    { "help",           3, { "-P", "--help=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "version",        3, { "-P", "--version=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "check",          3, { "-P", "--check=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "degraded",       3, { "-P", "--degraded=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "foreground",     3, { "-P", "--foreground=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "background",     3, { "-P", "--background=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "print",          3, { "-P", "--print=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "listen",         3, { "-P", "--listen=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { "user_junos",     3, { "-P", "--user_junos=unwanted", "-i", "-d" },
                        2, { '?', 'i' } },
    { 0 }
};


static struct test_set test_data_short_supplied_now[] =
{
    { "i",                     3, { "-P", "-i=supplied", "-f", "-d" },
                               2, { 'i', 'f' },
                                  { "=supplied", 0 } },
    { "d",                     3, { "-P", "-d=supplied", "-f", "-v" },
                               2, { 'd', 'f' },
                                  { "=supplied", 0 } },
    { "p",                     3, { "-P", "-p=supplied", "-f", "-d" },
                               2, { 'p', 'f' },
                                  { "=supplied", 0 } },
    { 0 }
};


static struct test_set test_data_short_supplied_later[] =
{
    { "i",                     4, { "-P", "-i", "supplied", "-f", "-d" },
                               2, { 'i', 'f' },
                                  { "supplied", 0 } },
    { "d",                     4, { "-P", "-d", "supplied", "-f", "-v" },
                               2, { 'd', 'f' },
                                  { "supplied", 0 } },
    { "p",                     4, { "-P", "-p", "supplied", "-f", "-d" },
                               2, { 'p', 'f' },
                                  { "supplied", 0 } },
    { 0 }
};


static struct test_set test_data_long_supplied_now[] =
{
    { "child-id",              3, { "-P", "--child-id=supplied", "-f", "-d" },      /*required_argument */
                               2, { 'i', 'f' },
                                  { "supplied", 0 } },
    { "debug-level",           3, { "-P", "--debug-level=supplied", "-f", "-d" },      /*required_argument */
                               2, { 'd', 'f' },
                                  { "supplied", 0 } },
    { "pid-file",              3, { "-P", "--pid-file=supplied", "-f", "-d" },      /*required_argument */
                               2, { 'p', 'f' },
                                  { "supplied", 0 } },
    { "debug",                 3, { "-P", "--debug=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_debug, 'f' },
                                  { "supplied", 0 } },
    { "listen_port",           3, { "-P", "--listen_port=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_listen_port, 'f' },
                                  { "supplied", 0 } },
    { "access_log",            3, { "-P", "--access_log=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_access_log, 'f' },
                                  { "supplied", 0 } },
    { "accounting_log",        3, { "-P", "--accounting_log=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_account_log, 'f' },
                                  { "supplied", 0 } },
    { "key",                   3, { "-P", "--key=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_key, 'f' },
                                  { "supplied", 0 } },
    { "host",                  3, { "-P", "--host=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host, 'f' },
                                  { "supplied", 0 } },
    { "host_address",          3, { "-P", "--host_address=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_host_address, 'f' },
                                  { "supplied", 0 } },
    { "host_key",              3, { "-P", "--host_key=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_host_key, 'f' },
                                  { "supplied", 0 } },
    { "host_enable",           3, { "-P", "--host_enable=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host_enable, 'f' },
                                  { "supplied", 0 } },
    { "host_enable_password",  3, { "-P", "--host_enable_password=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_host_enable_password, 'f' },
                                  { "supplied", 0 } },
    { "host_enable_deny",      3, { "-P", "--host_enable_deny=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host_enable_deny, 'f' },
                                  { "supplied", 0 } },
    { "host_enable_permit",    3, { "-P", "--host_enable_permit=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host_enable_permit, 'f' },
                                  { "supplied", 0 } },
    { "group",                 3, { "-P", "--group=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_group, 'f' },
                                  { "supplied", 0 } },
    { "group_enable",          3, { "-P", "--group_enable=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_group_enable, 'f' },
                                  { "supplied", 0 } },
    { "group_enable_password", 3, { "-P", "--group_enable_password=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_group_enable_password, 'f' },
                                  { "supplied", 0 } },
    { "group_enable_deny",     3, { "-P", "--group_enable_deny=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_group_enable_deny, 'f' },
                                  { "supplied", 0 } },
    { "group_enable_permit",   3, { "-P", "--group_enable_permit=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_group_enable_permit, 'f' },
                                  { "supplied", 0 } },
    { "user",                  3, { "-P", "--user=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user, 'f' },
                                  { "supplied", 0 } },
    { "user_password",         3, { "-P", "--user_password=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_password, 'f' },
                                  { "supplied", 0 } },
    { "user_deny",             3, { "-P", "--user_deny=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_deny, 'f' },
                                  { "supplied", 0 } },
    { "user_permit",           3, { "-P", "--user_permit=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_permit, 'f' },
                                  { "supplied", 0 } },
    { "user_default_cmd",      3, { "-P", "--user_default_cmd=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_default_cmd, 'f' },
                                  { "supplied", 0 } },
    { "user_cmd",              3, { "-P", "--user_cmd=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_cmd, 'f' },
                                  { "supplied", 0 } },
    { "user_cmd_deny",         3, { "-P", "--user_cmd_deny=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_cmd_deny, 'f' },
                                  { "supplied", 0 } },
    { "user_cmd_permit",       3, { "-P", "--user_cmd_permit=supplied", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_cmd_permit, 'f' },
                                  { "supplied", 0 } },
    { "user_group",            3, { "-P", "--user_group=supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_group, 'f' },
                                  { "supplied", 0 } },
    { 0 }
};


static struct test_set test_data_long_supplied_later[] =
{
    { "child-id",              4, { "-P", "--child-id", "supplied", "-f", "-d" },      /*required_argument */
                               2, { 'i', 'f' },
                                  { "supplied", 0 } },
    { "debug-level",           4, { "-P", "--debug-level", "supplied", "-f", "-d" },      /*required_argument */
                               2, { 'd', 'f' },
                                  { "supplied", 0 } },
    { "pid-file",              4, { "-P", "--pid-file", "supplied", "-f", "-d" },      /*required_argument */
                               2, { 'p', 'f' },
                                  { "supplied", 0 } },
    { "debug",                 4, { "-P", "--debug", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_debug, 'f' },
                                  { "supplied", 0 } },
    { "access_log",            4, { "-P", "--access_log", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_access_log, 'f' },
                                  { "supplied", 0 } },
    { "accounting_log",        4, { "-P", "--accounting_log", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_account_log, 'f' },
                                  { "supplied", 0 } },
    { "key",                   4, { "-P", "--key", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_key, 'f' },
                                  { "supplied", 0 } },
    { "host_address",          4, { "-P", "--host_address", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_host_address, 'f' },
                                  { "supplied", 0 } },
    { "host_key",              4, { "-P", "--host_key", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_host_key, 'f' },
                                  { "supplied", 0 } },
    { "host_enable_password",  4, { "-P", "--host_enable_password", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_host_enable_password, 'f' },
                                  { "supplied", 0 } },
    { "group",                 4, { "-P", "--group", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_group, 'f' },
                                  { "supplied", 0 } },
    { "group_enable_password", 4, { "-P", "--group_enable_password", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_group_enable_password, 'f' },
                                  { "supplied", 0 } },
    { "user",                  4, { "-P", "--user", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user, 'f' },
                                  { "supplied", 0 } },
    { "user_password",         4, { "-P", "--user_password", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_password, 'f' },
                                  { "supplied", 0 } },
    { "user_default_cmd",      4, { "-P", "--user_default_cmd", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_default_cmd, 'f' },
                                  { "supplied", 0 } },
    { "user_cmd",              4, { "-P", "--user_cmd", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_cmd, 'f' },
                                  { "supplied", 0 } },
    { "user_group",            4, { "-P", "--user_group", "supplied", "-f", "-d" },      /*required_argument */
                               2, { lopt_user_group, 'f' },
                                  { "supplied", 0 } },
    { 0 }
};


static struct test_set test_data_long_supplied_opt[] =
{
    { "listen_port",           3, { "-P", "--listen_port", "-f", "-d" },      /*optional_argument */
                               2, { lopt_listen_port, 'f' },
                                  { 0, 0 } },
    { "host",                  3, { "-P", "--host", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host, 'f' },
                                  { 0, 0 } },
    { "host_enable",           3, { "-P", "--host_enable", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host_enable, 'f' },
                                  { 0, 0 } },
    { "host_enable_deny",      3, { "-P", "--host_enable_deny", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host_enable_deny, 'f' },
                                  { 0, 0 } },
    { "host_enable_permit",    3, { "-P", "--host_enable_permit", "-f", "-d" },      /*optional_argument */
                               2, { lopt_host_enable_permit, 'f' },
                                  { 0, 0 } },
    { "group_enable",          3, { "-P", "--group_enable", "-f", "-d" },      /*optional_argument */
                               2, { lopt_group_enable, 'f' },
                                  { 0, 0 } },
    { "group_enable_deny",     3, { "-P", "--group_enable_deny", "-f", "-d" },      /*optional_argument */
                               2, { lopt_group_enable_deny, 'f' },
                                  { 0, 0 } },
    { "group_enable_permit",   3, { "-P", "--group_enable_permit", "-f", "-d" },      /*optional_argument */
                               2, { lopt_group_enable_permit, 'f' },
                                  { 0, 0 } },
    { "user_deny",             3, { "-P", "--user_deny", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_deny, 'f' },
                                  { 0, 0 } },
    { "user_permit",           3, { "-P", "--user_permit", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_permit, 'f' },
                                  { 0, 0 } },
    { "user_cmd_deny",         3, { "-P", "--user_cmd_deny", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_cmd_deny, 'f' },
                                  { 0, 0 } },
    { "user_cmd_permit",       3, { "-P", "--user_cmd_permit", "-f", "-d" },      /*optional_argument */
                               2, { lopt_user_cmd_permit, 'f' },
                                  { 0, 0 } },
    { 0 }
};


static struct test_set test_data_short_missing[] =
{
    { "i",        2, { "-P", "-i", "-d" },
                  1, { 'i' } },
    { "d",        2, { "-P", "-d", "-v" },
                  1, { 'd' } },
    { "p",        2, { "-P", "-p", "-d" },
                  1, { 'p' } },
    { 0 }
};


static struct test_set test_data_long_missing[] =
{
    { "child-id",              2, { "-P", "--child-id", "-d" },      /*required_argument */
                               1, { 'i' } },
    { "debug-level",           2, { "-P", "--debug-level", "-d" },      /*required_argument */
                               1, { 'd' } },
    { "pid-file",              2, { "-P", "--pid-file", "-d" },      /*required_argument */
                               1, { 'p' } },
    { "debug",                 2, { "-P", "--debug", "-d" },      /*required_argument */
                               1, { lopt_debug } },
    { "listen_port",           2, { "-P", "--listen_port", "-d" },      /*optional_argument */
                               1, { lopt_listen_port } },
    { "access_log",            2, { "-P", "--access_log", "-d" },      /*required_argument */
                               1, { lopt_access_log } },
    { "accounting_log",        2, { "-P", "--accounting_log", "-d" },      /*required_argument */
                               1, { lopt_account_log } },
    { "key",                   2, { "-P", "--key", "-d" },      /*required_argument */
                               1, { lopt_key } },
    { "host",                  2, { "-P", "--host", "-d" },      /*optional_argument */
                               1, { lopt_host } },
    { "host_address",          2, { "-P", "--host_address", "-d" },      /*required_argument */
                               1, { lopt_host_address } },
    { "host_key",              2, { "-P", "--host_key", "-d" },      /*required_argument */
                               1, { lopt_host_key } },
    { "host_enable",           2, { "-P", "--host_enable", "-d" },      /*optional_argument */
                               1, { lopt_host_enable } },
    { "host_enable_password",  2, { "-P", "--host_enable_password", "-d" },      /*required_argument */
                               1, { lopt_host_enable_password } },
    { "host_enable_deny",      2, { "-P", "--host_enable_deny", "-d" },      /*optional_argument */
                               1, { lopt_host_enable_deny } },
    { "host_enable_permit",    2, { "-P", "--host_enable_permit", "-d" },      /*optional_argument */
                               1, { lopt_host_enable_permit } },
    { "group",                 2, { "-P", "--group", "-d" },      /*required_argument */
                               1, { lopt_group } },
    { "group_enable",          2, { "-P", "--group_enable", "-d" },      /*optional_argument */
                               1, { lopt_group_enable } },
    { "group_enable_password", 2, { "-P", "--group_enable_password", "-d" },      /*required_argument */
                               1, { lopt_group_enable_password } },
    { "group_enable_deny",     2, { "-P", "--group_enable_deny", "-d" },      /*optional_argument */
                               1, { lopt_group_enable_deny } },
    { "group_enable_permit",   2, { "-P", "--group_enable_permit", "-d" },      /*optional_argument */
                               1, { lopt_group_enable_permit } },
    { "user",                  2, { "-P", "--user", "-d" },      /*required_argument */
                               1, { lopt_user } },
    { "user_password",         2, { "-P", "--user_password", "-d" },      /*required_argument */
                               1, { lopt_user_password } },
    { "user_deny",             2, { "-P", "--user_deny", "-d" },      /*optional_argument */
                               1, { lopt_user_deny } },
    { "user_permit",           2, { "-P", "--user_permit", "-d" },      /*optional_argument */
                               1, { lopt_user_permit } },
    { "user_default_cmd",      2, { "-P", "--user_default_cmd", "-d" },      /*required_argument */
                               1, { lopt_user_default_cmd } },
    { "user_cmd",              2, { "-P", "--user_cmd", "-d" },      /*required_argument */
                               1, { lopt_user_cmd } },
    { "user_cmd_deny",         2, { "-P", "--user_cmd_deny", "-d" },      /*optional_argument */
                               1, { lopt_user_cmd_deny } },
    { "user_cmd_permit",       2, { "-P", "--user_cmd_permit", "-d" },      /*optional_argument */
                               1, { lopt_user_cmd_permit } },
    { "user_group",            2, { "-P", "--user_group", "-d" },      /*required_argument */
                               1, { lopt_user_group } },
    { 0 }
};



void test_get_next_option(struct test_set* this_test)
{
    int result_count;

    unity_start_test(this_test->test_name);

    opts_argc = this_test->argc;
    opts_argv = (char**) this_test->argv;
    optind = 0;

    for (result_count = 0; result_count <= this_test->count; ++result_count)
    {
        char* actual_value = 0;
        const char* expected_value = none;
        int result = get_next_option();
        if (result == EOF)
        {
            break;
        }

        unity_assert_int_equal (this_test->opt_codes[result_count], result);

        if (this_test->values[result_count] != 0)
	{
            expected_value = this_test->values[result_count];
        }
        actual_value = get_optional_argument (none);
        unity_assert_str_equal (expected_value, actual_value);

        free (actual_value);
    }

    unity_assert_int_equal (this_test->count, result_count);

    unity_end_test();
}



void test_entry_group(const char* name, struct test_set* set)
{
    int test_index;

    unity_start_group(name);

    for (test_index = 0; set[test_index].test_name != 0; ++test_index)
    {
        test_get_next_option(&set[test_index]);
    }

    unity_end_group();
}



void test_entry()
{
    int test_index;

    unity_start_group("get_next_option");

    test_entry_group("basic",                test_data_basic);
    test_entry_group("plain",                test_data_plain);
    test_entry_group("short_unwanted",       test_data_short_unwanted);
    test_entry_group("long_unwanted",        test_data_long_unwanted);
    test_entry_group("short_supplied_now",   test_data_short_supplied_now);
    test_entry_group("short_supplied_later", test_data_short_supplied_later);
    test_entry_group("long_supplied_now",    test_data_long_supplied_now);
    test_entry_group("long_supplied_later",  test_data_long_supplied_later);
    test_entry_group("long_supplied_opt",    test_data_long_supplied_opt);
    test_entry_group("short_missing",        test_data_short_missing);
    test_entry_group("long_missing",         test_data_long_missing);

    unity_end_group();
}

