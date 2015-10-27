/*
 * Unit test framework
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "unity.h"

typedef struct ugs
{
    struct ugs*    parent;
    int            indent;
    char*          name;
} unity_group_t;


typedef struct uts
{
    unity_group_t* group;
    struct uts*    next;
    int            length;
    const char*    result;
    char*          name;
} unity_test_t;


typedef struct
{
    const char*    program;
    int            total;
    int            fails;
    int            skips;
    int            max_len;
    unity_group_t* context;
    unity_test_t*  first_test;
    unity_test_t*  last_test;
    unity_test_t*  this_test;
    int            skip_test;
    int            fail_test;
    struct timeval time_base;
    struct timeval last_time;
} unity_data_t;

static unity_data_t unity;


static void mark_time()
{
    double seconds;
    double microsecs;

    gettimeofday(&unity.last_time, 0);
    seconds = unity.last_time.tv_sec;
    microsecs = unity.last_time.tv_usec;
    printf ("@@@ %.3f\n", seconds + microsecs / 1000000.0);
}

static void print_context (unity_group_t* here)
{
    if (here != 0)
    {
	print_context (here->parent);
	printf ("%s/", here->name);
    }
}

static void print_n_chars (int n, char ch)
{
    while (--n >= 0)
    {
	printf ("%c", ch);
    }
}

static void unity_start (const char* program)
{
    unity.program = program;
    unity.total = 0;
    unity.fails = 0;
    unity.skips = 0;
    unity.max_len = 0;
    unity.context = 0;
    unity.first_test = 0;
    unity.this_test = 0;
    gettimeofday(&unity.time_base, 0);

    printf ("### Unit test program %s starting ###\n", unity.program);
    unity_start_group (unity.program)
}


static int unity_end()
{
    unity_test_t* test;

    unity_end_group()

    printf ("### Unit test program %s finished ###\n", unity.program);
    printf ("%d Tests %d Failures %d Ignored\n", unity.total, unity.fails, unity.skips);
    printf ("OK\n\nTest result summary\n");
    printf ("   |--------|-");
    print_n_chars (unity.max_len, '-');
    printf ("-|\n");
    printf ("   | Result | Test");
    print_n_chars (unity.max_len - 4, ' ');
    printf (" |\n");
    printf ("   |--------|-");
    print_n_chars (unity.max_len, '-');
    printf ("-|\n");
    for (test = unity.first_test; test != 0; test = test->next)
    {
	printf ("   |  %s  | ", test->result);
	print_context (test->group);
	printf ("%s", test->name);
	print_n_chars (unity.max_len - test->length, ' ');
	printf (" |\n");
    }
    printf ("   |--------|-");
    print_n_chars (unity.max_len, '-');
    printf ("-|\n");
    printf ("End of summary\n");

    return (unity.fails > 0) ? 1 : 0;
}

void unity_skip()
{
    if (unity.this_test == 0)
    {
	fprintf (stderr, "unity_skip() called outside a test!\n");
	exit(2);
    }

    unity.skip_test = 1;
}

void unity_fail()
{
    if (unity.this_test == 0)
    {
	fprintf (stderr, "unity_fail() called outside a test!\n");
	exit(2);
    }

    unity.fail_test = 1;
}

void unity_fail_at (const char* filename, int line_no)
{
    printf ("*** UNITY ASSERT FAILED in %s at line %d\n", filename, line_no);
    unity_fail();
}

void unity_start_group (const char* name)
{
    unity_group_t* group = malloc(sizeof(unity_group_t));

    if (group == 0)
    {
        fprintf (stderr, "Out of memory!\n");
	exit(3);
    }

    group->parent = unity.context;
    group->name = strdup(name);
    group->indent = strlen(name);
    if (unity.context)
    {
	group->indent += unity.context->indent + 1;
    }
    unity.context = group;
}

void unity_end_group()
{
    if (unity.this_test != 0)
    {
	fprintf (stderr, "unity_end_group() called inside test!\n");
    }

    if (unity.context == 0)
    {
	fprintf (stderr, "unity_end_group() called outside a group!\n");
	exit(2);
    }

    unity.context = unity.context->parent;
}

void unity_start_test (const char* name)
{
    unity_test_t* new_test = malloc(sizeof(unity_test_t));

    if (new_test == 0)
    {
        fprintf (stderr, "Out of memory!\n");
	exit(3);
    }

    if (unity.this_test != 0)
    {
	fprintf (stderr, "unity_start_test() called inside test!\n");
	unity_end_test();
    }

    if (unity.last_test == 0)
    {
	unity.first_test = new_test;
    }
    else
    {
	unity.last_test->next = new_test;
    }
    unity.last_test = new_test;
    unity.this_test = new_test;

    new_test->group = unity.context;
    new_test->name = strdup(name);
    new_test->next = 0;
    new_test->length = strlen(name);
    new_test->result = 0;

    if (unity.context)
    {
	new_test->length += unity.context->indent + 1;
    }

    if (new_test->length > unity.max_len)
    {
	unity.max_len = new_test->length;
    }

    unity.skip_test = 0;
    unity.fail_test = 0;

    mark_time();
    printf ("%%%%%% %s\n", name);
}

void unity_end_test()
{
    if (unity.this_test == 0)
    {
	fprintf (stderr, "unity_end_test() called outside test!\n");
    }
    else
    {
	const char* result;

	if (unity.skip_test)
	{
	    result = "SKIP";
	}
	else if (unity.fail_test)
	{
	    result = "FAIL";
	}
	else
	{
	    result = "PASS";
	}
        unity.this_test->result = result;
	mark_time();
	printf (":::");
        print_context (unity.this_test->group);
        printf ("%s::: %s\n", unity.this_test->name, result);
        unity.this_test = 0;
    }
}


/* *********************************************************************** */


void unity_assert_ptr_equal_at (const char* filename,
                                int         line_no,
                                const void* expect,
                                const void* got)
{
    if (expect != got)
    {
	printf (" -- UNITY assert failed:\n");
	printf (" --  -- Expected: %p\n", expect);
	printf (" --  --      Got: %p\n", got);
        unity_fail_at (filename, line_no);
    }
}


void unity_assert_ptr_not_equal_at (const char* filename,
                                    int         line_no,
                                    const void* expect,
                                    const void* got)
{
    if (expect == got)
    {
	printf (" -- UNITY assert failed:\n");
	printf (" --  -- Expected: %p\n", expect);
	printf (" --  --      Got: %p\n", got);
        unity_fail_at (filename, line_no);
    }
}


void unity_assert_int_equal_at (const char* filename,
                                int         line_no,
                                long        expect,
                                long        got)
{
    if (expect != got)
    {
	printf (" -- UNITY assert failed:\n");
	printf (" --  -- Expected: %ld\n", expect);
	printf (" --  --      Got: %ld\n", got);
        unity_fail_at (filename, line_no);
    }
}


void unity_assert_int_not_equal_at (const char* filename,
                                    int         line_no,
                                    long        expect,
                                    long        got)
{
    if (expect == got)
    {
	printf (" -- UNITY assert failed:\n");
	printf (" --  -- Expected: %ld\n", expect);
	printf (" --  --      Got: %ld\n", got);
        unity_fail_at (filename, line_no);
    }
}


void unity_assert_str_equal_at (const char* filename,
                                int         line_no,
                                const char* expect,
                                const char* got)
{
    if (expect == 0)
    {
	if (got != 0)
	{
	    printf (" -- UNITY assert failed:\n");
	    printf (" --  -- Expected: <NULL>\n");
	    printf (" --  --      Got: \"%s\"\n", got);
	    unity_fail_at (filename, line_no);
	}
    }
    else if (got == 0)
    {
	printf (" -- UNITY assert failed:\n");
	printf (" --  -- Expected: \"%s\"\n", expect);
	printf (" --  --      Got: <NULL>\n");
	unity_fail_at (filename, line_no);
    }
    else if (strcmp(expect, got) != 0)
    {
	printf (" -- UNITY assert failed:\n");
	printf (" --  -- Expected: \"%s\"\n", expect);
	printf (" --  --      Got: \"%s\"\n", got);
        unity_fail_at (filename, line_no);
    }
}


/* *********************************************************************** */


int main (int argc, char* argv[])
{
    int exit_code;
    const char* program;

    if ((argc < 1) || (argv == 0) || (argv[0] == 0) || (argv[0][0] == '\0'))
    {
	program = "???";
    }
    else
    {
	program = argv[0];
	while (strchr(program, '/') != 0)
	{
	    program = strchr(program, '/') + 1;
	}
    }

    unity_start (program);
    test_entry();
    exit_code = unity_end();
    return exit_code;
}

