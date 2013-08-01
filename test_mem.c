/*
 * =====================================================================================
 *
 *       Filename:  test_mem.c
 *
 *    Description:  Tests for memory
 *
 *        Version:  1.0
 *        Created:  13-07-29 10:51:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alan Kydd (), akydd@ualberta.net
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include "memory.h"


#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
	if (message) return message; } while (0)

int tests_run = 0;

struct memory *memory;

static char *test_MEM_init()
{
	memory = MEM_init();
	mu_assert("memory is NULL!", memory != NULL);
	MEM_delete(&memory);
	mu_assert("memory is not NULL!", memory == NULL);

	return 0;
}

static char *test_MEM_write_non_mirrored()
{
	memory = MEM_init();

	MEM_write(memory, 0x6000, 123);
	MEM_write(memory, 0x7FFF, 99);

	mu_assert("Wrong value at 0x6000", MEM_read(memory, 0x6000) == 123);
	mu_assert("Wrong value at 0x7FFF", MEM_read(memory, 0x7FFF) == 99);

	MEM_delete(&memory);
	return 0;
}

static char *test_MEM_write_mirrored()
{
	memory = MEM_init();

	MEM_write(memory, 0x0200, 123);

	mu_assert("No mirrow at 0x0200 + 1*0x0800", MEM_read(memory, 0x0200 + 0x0800) == 123);
	mu_assert("No mirrow at 0x0200 + 2*0x0800", MEM_read(memory, 0x0200 + 2*0x0800) == 123);
	mu_assert("No mirrow at 0x0200 + 3*0x0800", MEM_read(memory, 0x0200 + 3*0x0800) == 123);

	MEM_delete(&memory);
	return 0;
}

static char *all_tests()
{
	mu_run_test(test_MEM_init);
	mu_run_test(test_MEM_write_mirrored);
	mu_run_test(test_MEM_write_non_mirrored);
	return 0;
}

int main()
{
	char *result = all_tests();
	if (result != 0) {
		(void) printf("%s\n", result);
	} else {
		(void) printf("All tests passed!\n");
	}
	(void) printf("Tests run: %d\n", tests_run);

	return result != 0;
}
