/*
 * Unit test program for get_user_shell()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


void test_get_user_shell()
{
    struct user_config test_user;
    struct service_config test_shell;
    struct service_config* result;

    unity_start_group("get_user_shell");

    unity_start_test("present");
    test_user.shell_service = &test_shell;
    result = get_user_shell (&test_user);
    unity_assert_str_equal (result, &test_shell);
    unity_end_test();

    unity_start_test("empty");
    test_user.shell_service = 0;
    test_user.service_head = 0;
    test_user.service_tail = 0;
    result = get_user_shell (&test_user);
    unity_assert_ptr_not_equal (result, 0);
    unity_assert_ptr_equal (result, test_user.shell_service);
    unity_assert_ptr_equal (result, test_user.service_head);
    unity_assert_ptr_equal (result, test_user.service_tail);
    unity_assert_ptr_equal (result->next_service, 0);
    unity_assert_str_equal (result->svc_name, "shell");
    unity_end_test();

    unity_start_test("another");
    test_user.shell_service = 0;
    test_user.service_head = &test_shell;
    test_user.service_tail = &test_shell;
    result = get_user_shell (&test_user);
    unity_assert_ptr_not_equal (result, 0);
    unity_assert_ptr_equal (result, test_user.shell_service);
    unity_assert_ptr_equal (&test_shell, test_user.service_head);
    unity_assert_ptr_equal (result, test_shell.next_service);
    unity_assert_ptr_equal (result, test_user.service_tail);
    unity_assert_ptr_equal (0, result->next_service);
    unity_assert_str_equal (result->svc_name, "shell");
    unity_end_test();

    unity_end_group();
}



void test_entry()
{
    test_get_user_shell();
}

