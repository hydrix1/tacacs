/*
 * Unit test framework
 */

#include <stdlib.h>


void unity_skip();
void unity_fail();
void unity_start_group (const char* name);
void unity_end_group();
void unity_start_test (const char* name);
void unity_end_test();


/* The program under test must provide this routine */
extern void test_entry();
