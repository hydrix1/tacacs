/*
 * Unit test program for get_optional_argument()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


void test_get_optional_argument()
{
    static const char* supplied_1 = "Supplied value 1";
    static const char* default_1 = "Default value 1";
    static const char* default_2 = "Default value 2";
    const char* result;

    unity_start_group("get_optional_argument");

    unity_start_test("presnt");
    optarg = (char*) supplied_1;
    result = get_optional_argument (default_1);
    unity_assert_ptr_equal (optarg, 0);
    unity_assert_ptr_not_equal (result, 0);
    unity_assert_ptr_not_equal (result, supplied_1);
    unity_assert_ptr_not_equal (result, default_1);
    unity_assert_str_equal (result, supplied_1);
    unity_end_test();

    unity_start_test("absent");
    optarg = (char*) 0;
    result = get_optional_argument (default_2);
    unity_assert_ptr_equal (optarg, 0);
    unity_assert_ptr_not_equal (result, 0);
    unity_assert_ptr_not_equal (result, default_2);
    unity_assert_str_equal (result, default_2);
    unity_end_test();

    unity_end_group();
}



void test_entry()
{
    test_get_optional_argument();
}

