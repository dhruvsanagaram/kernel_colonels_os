#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/* Divide Error Fault
 * 
 * Throws divide error fault when dividing by zero.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Divide 1 by 0
 * Files: x86_desc.h/S, idt.c
 */

 int divide_test() {
	TEST_HEADER;
	int i,j,k;
	j = 1;
	k = 0;
	i = j/k;
	return FAIL;
}


/* Paging Test 1
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if accessing NULL throws page fault
 * Files: x86_desc.h/S, page.c
 */

int page_test_NULL() {
	TEST_HEADER;
	char * x;
	char y;
	// char test;
	// test = *(char*)NULL;

	x = NULL;
	y = *x;
	return FAIL;
}

/* Paging Test 2
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if accessing before start of kernel memory throws page fault
 * Files: x86_desc.h/S, page.c
 */

int page_test_kbefore() {
	TEST_HEADER;
	char test;
	test = *(char*)0x3FFFFF;
	return FAIL;
}

/* Paging Test 3
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if accessing after end of kernel memory throws page fault
 * Files: x86_desc.h/S, page.c
 */

int page_test_kafter() {
	TEST_HEADER;
	char test;
	test = *(char*)0x800000;
	return FAIL;
}

/* Paging Test 4
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if accessing before start of video memory throws page fault
 * Files: x86_desc.h/S, page.c
 */

int page_test_vbefore() {
	TEST_HEADER;
	char test;
	test = *(char*)0xB7FFF;
	return FAIL;
}

/* Paging Test 5
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if accessing after end of kernel memory throws page fault
 * Files: x86_desc.h/S, page.c
 */

int page_test_vafter() {
	TEST_HEADER;
	char test;
	test = *(char*)0xB9000;
	return FAIL;
}

/* KEYB Test
 * 
 * 
 * Inputs: None
 * Outputs: poll char
 * Side Effects: None
 * Coverage: Sends int line ffrom keyb
 * Files: x86_desc.h/S, page.c
 */
void kbtest() {
	TEST_HEADER;
	while(1) {
		keyb_main();
	}
}

/* Paging Test 6
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if anything within virtual and kernel memory is accessible
 * Files: x86_desc.h/S, page.c
 */

int page_test_all() {
	TEST_HEADER;
	char test;
	test = *(char*)0x400000;
	test = *(char*)0x7FFFFF;
	test = *(char*)0xB8000;
	test = *(char*)0xB8FFF;

	return PASS;
}

/* Paging Test 7
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Test if anything within virtual and kernel memory is accessible
 * Files: x86_desc.h/S, page.c
 */


int alltest() {
	TEST_HEADER;
	int i;
	char test;
	for (i=0x400000; i < 0x800000; i++) {
		test = *(char*)i;
	}
	return PASS;
}

/* RTC Test
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Sends int line
 * Files: x86_desc.h/S, page.c
 */


void rtc_test() {
	asm volatile ("int	$0x28");
	while(1){
		test_interrupts();
	}
}



/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	///////////////////// IDT TESTS /////////////////////
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("divide_test", divide_test());

	///////////////////// DEVICE TESTS /////////////////////
	// rtc_test();
	

	///////////////////// PAGING TESTS /////////////////////
	// TEST_OUTPUT("alltest", alltest());
	// TEST_OUTPUT("page_test_all", page_test_all());
	// TEST_OUTPUT("page_test_null", page_test_NULL());
	// TEST_OUTPUT("page_test_kbefore", page_test_kbefore());
	// TEST_OUTPUT("page_test_kafter", page_test_kafter());
	// TEST_OUTPUT("page_test_vbefore", page_test_vbefore());
	// TEST_OUTPUT("page_test_vafter", page_test_vafter());



	kbtest();
}
