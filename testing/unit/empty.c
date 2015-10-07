/*
 * Empty unit test program, to check the unity test mechanism
 */

#include "unity.h"


void test_entry()
{
    unity_start_test ("root");
    unity_end_test();
    unity_start_group ("first");
    unity_start_test ("primo");
    unity_end_test();
    unity_start_group ("second");
    unity_start_group ("third");
    unity_start_test ("1-2-3");
    unity_end_test();
    unity_end_group();
    unity_start_group ("branch");
    unity_start_group ("leaf");
    unity_start_test ("1-2-b-l");
    unity_end_test();
    unity_end_group();
    unity_end_group();
    unity_end_group();
}

