/*
 * Unit test program for get_required_argument()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


void test_get_required_argument_existing()
{
    static const char* sample_1 = "Sample value";
    const char* result;

    unity_start_group("existing");

    unity_start_test("normal");
    optarg = (char*) sample_1;
    result = get_required_argument ("Prompt %d text", 1);
    unity_assert_ptr_equal (optarg, 0);
    unity_assert_ptr_not_equal (result, 0);
    unity_assert_ptr_not_equal (result, sample_1);
    unity_assert_str_equal (result, sample_1);
    unity_end_test();

    unity_end_group();
}



void test_get_required_argument()
{
    unity_start_group("get_required_argument");
    test_get_required_argument_existing();
    unity_end_group();
}



void test_entry()
{
    test_get_required_argument();
}

