/*
 * Unit test framework
 */

#include <stdlib.h>


extern void unity_skip();
extern void unity_fail();
extern void unity_start_group (const char* name);
extern void unity_end_group();
extern void unity_start_test (const char* name);
extern void unity_end_test();

extern void unity_assert_ptr_equal_at (const char* filename, int line_no, const void* expect, const void* got);
extern void unity_assert_ptr_not_equal_at (const char* filename, int line_no, const void* expect, const void* got);
extern void unity_assert_int_equal_at (const char* filename, int line_no, long expect, long got);
extern void unity_assert_int_not_equal_at (const char* filename, int line_no, long expect, long got);
extern void unity_assert_str_equal_at (const char* filename, int line_no, const char* expect, const char* got);
extern void unity_assert_str_not_equal_at (const char* filename, int line_no, const char* expect, const char* got);

#define unity_assert_ptr_equal(expect, got) \
	 unity_assert_ptr_equal_at (__FILE__, __LINE__, expect, got);
#define unity_assert_ptr_not_equal(expect, got) \
	 unity_assert_ptr_not_equal_at (__FILE__, __LINE__, expect, got);
#define unity_assert_int_equal(expect, got) \
	 unity_assert_int_equal_at (__FILE__, __LINE__, expect, got);
#define unity_assert_int_not_equal(expect, got) \
	 unity_assert_int_not_equal_at (__FILE__, __LINE__, expect, got);
#define unity_assert_str_equal(expect, got) \
	 unity_assert_str_equal_at (__FILE__, __LINE__, expect, got);
#define unity_assert_str_not_equal(expect, got) \
	 unity_assert_str_not_equal_at (__FILE__, __LINE__, expect, got);

/* The program under test must provide this routine */
extern void test_entry();
