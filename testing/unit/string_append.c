/*
 * Unit test program for string_append()
 */

#include <stdlib.h>
#include "unity.h"
#include "wrap_spawnd_main.h"


void test_string_append_new()
{
    static const char* easy = "Easy";
    static const char* assm = "as Sunday morning";
    char* base_1 = 0;
    char* base_2 = 0;

    unity_start_group("new");

    unity_start_test("Easy");
    string_append (&base_1, easy);
    unity_assert_ptr_not_equal (base_1, 0);
    unity_assert_ptr_not_equal (base_1, easy);
    unity_assert_str_equal (base_1, easy);
    unity_end_test();

    unity_start_test("Again");
    string_append (&base_2, assm);
    unity_assert_ptr_not_equal (base_2, 0);
    unity_assert_ptr_not_equal (base_2, easy);
    unity_assert_ptr_not_equal (base_2, assm);
    unity_assert_ptr_not_equal (base_2, base_1);
    unity_assert_str_equal (base_2, assm);
    unity_end_test();

    unity_start_test("free");
    free (base_1);
    free (base_2);
    unity_end_test();

    unity_end_group();
}



void test_string_append_existing()
{
    static const char* easy = "Easy";
    static const char* as   = " as";
    static const char* assm = " a very lazy Sunday morning";
    static const char* not  = "Not";
    static const char* so   = " so";
    static const char* mon  = " lazy as Monday morning back at work";
    char* base_1 = 0;
    char* base_2 = 0;
    char* base_3 = 0;
    char* base_4 = 0;

    unity_start_group("existing");

    unity_start_test("short");
    string_append (&base_1, easy);
    string_append (&base_1, as);
    unity_assert_ptr_not_equal (base_1, 0);
    unity_assert_ptr_not_equal (base_1, easy);
    unity_assert_ptr_not_equal (base_1, as);
    unity_assert_str_equal (base_1, "Easy as");
    unity_end_test();

    unity_start_test("again");
    string_append (&base_2, not);
    string_append (&base_2, so);
    unity_assert_ptr_not_equal (base_2, 0);
    unity_assert_ptr_not_equal (base_2, not);
    unity_assert_ptr_not_equal (base_2, so);
    unity_assert_ptr_not_equal (base_2, base_1);
    unity_assert_str_equal (base_2, "Not so");
    unity_end_test();

    unity_start_test("long");
    base_3 = base_1;
    string_append (&base_3, assm);
    unity_assert_ptr_not_equal (base_3, 0);
    unity_assert_ptr_not_equal (base_3, base_2);
    unity_assert_ptr_not_equal (base_3, assm);
    unity_assert_str_equal (base_3, "Easy as a very lazy Sunday morning");
    unity_end_test();

    unity_start_test("longer");
    base_4 = base_2;
    string_append (&base_4, mon);
    unity_assert_ptr_not_equal (base_4, 0);
    unity_assert_ptr_not_equal (base_4, base_3);
    unity_assert_ptr_not_equal (base_4, mon);
    unity_assert_str_equal (base_4, "Not so lazy as Monday morning back at work");
    unity_end_test();

    unity_start_test("free");
    free (base_3);
    free (base_4);
    unity_end_test();

    unity_end_group();
}



void test_string_append_missing()
{
    static const char* easy = "Easy";
    static const char* assm = "as Sunday morning:";
    char* base_1 = 0;
    char* base_2 = 0;
    char* base_3 = 0;

    unity_start_group("missing");

    unity_start_test("null-null");
    string_append (&base_1, 0);
    unity_assert_ptr_equal (base_1, 0);
    unity_end_test();

    unity_start_test("basea-null");
    string_append (&base_2, easy);
    unity_assert_ptr_not_equal (base_2, 0);
    unity_assert_ptr_not_equal (base_2, easy);
    unity_assert_str_equal (base_2, easy);
    unity_end_test();

    unity_start_test("extra-null");
    string_append (&base_3, assm);
    string_append (&base_3, 0);
    unity_assert_ptr_not_equal (base_3, 0);
    unity_assert_ptr_not_equal (base_3, base_2);
    unity_assert_ptr_not_equal (base_3, assm);
    unity_assert_str_equal (base_3, assm);
    unity_end_test();

    unity_start_test("free");
    free (base_3);
    free (base_2);
    unity_end_test();

    unity_end_group();
}



void test_string_append()
{
    unity_start_group("string_append");
    test_string_append_new();
    test_string_append_existing();
    test_string_append_missing();
    unity_end_group();
}



void test_entry()
{
    test_string_append();
}

