/**
* @file test-utilc-logging.c
* @author Cameron A. Craig
* @date 28 Dec 2016
* @version 0.1.0
* @copyright 2016 Cameron A. Craig
* @brief Unit tests for utilc-logging.
* -- RULE_3_2_CD_do_not_use_special_characters_in_filename
* -- RULE_8_1_A_provide_file_info_comment

* @note Check unit test documentation can be cound here:
* https://libcheck.github.io/check/doc/check_html/index.html#Top
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <check.h>

#include "utilc-logging.h"

struct ucl_s ucl;
uint32_t err;

/************************
* Test Case Setup and teardown
************************/

/**
* @brief Setup
* @details Checked fixture: Runs for each unit test.
*/
void uc_logging_test_setup(void){
	ucl_init(&ucl);
}

/**
* @brief Teardown
* @details Checked fixture: Runs for each unit test.
*/
void uc_logging_test_teardown(void){
	ucl_free(&ucl);
}

/************************
* Unit Tests
************************/

START_TEST (test_basic_stdout){
	// Add stdout destination
	ucl_dest_h dest1 = ucl_add_dest(&ucl, UCL_DEST_STDOUT);

	// Log a single string (most basic)
	fail_if(ucl_log(&ucl, UCL_LL_INFO, "If at first you don't succeed; call it version %.1f.\r\n", 1.0f) != UCL_OK);
}
END_TEST

START_TEST (test_basic_file){
	// Add stdout destination
	ucl_dest_h dest1 = ucl_add_dest(&ucl, UCL_DEST_FILE, "testfile.txt");

	//Define expected output
	const char EXPECTED_VALUE[] = "If at first you don't succeed; call it version 1.0.";

	//Define the offset from the log message start, where the actual message string starts
	int LOG_MSG_OFFSET = 35;

	// Log a single string (most basic)
	fail_if(ucl_log(&ucl, UCL_LL_INFO, "If at first you don't succeed; call it version %.1f.\r\n", 1.0f) != UCL_OK);

	//Open the file just written to
	FILE *f = fopen("testfile.txt", "r");
	fail_if(f == NULL);

	size_t str_len = strlen(EXPECTED_VALUE) + 1;
	char buffer[str_len + LOG_MSG_OFFSET];

	//Read the first line in the file
	fgets(buffer, str_len + LOG_MSG_OFFSET, f);
	fclose(f);

	//Compare the first line in the file with the expected string value.
	fail_if(strncmp(buffer + LOG_MSG_OFFSET, EXPECTED_VALUE, str_len) != 0);

	//Delete the log file just produced
	fail_if(remove("testfile.txt") != 0);
}
END_TEST



/**
* @brief Defines test suite for utilc-template lib.
*/
Suite* utilc_test_suite (void) {
				Suite *suite = suite_create("utilc_loggong_test_suite");

				TCase *basic_tcase = tcase_create("Basic Test Case");
				tcase_add_checked_fixture(basic_tcase, uc_logging_test_setup, uc_logging_test_teardown);
				tcase_add_test(basic_tcase, test_basic_stdout);
				tcase_add_test(basic_tcase, test_basic_file);
				suite_add_tcase(suite, basic_tcase);

				return suite;
}

/**
* @brief Run the full test suite.
* @returns Number of tests failed.
*/
uint32_t main (uint32_t argc, char *argv[]) {
				uint32_t number_failed;
				Suite *suite = utilc_test_suite();
				SRunner *runner = srunner_create(suite);
				srunner_run_all(runner, CK_NORMAL);
				number_failed = srunner_ntests_failed(runner);
				srunner_free(runner);
				return number_failed;
}
