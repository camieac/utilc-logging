/**
* @file utilc-logging-example.c
* @author Cameron A. Craig
* @date 27 Nov 2016
* @version 0.1.0
* @copyright 2016 Cameron A. Craig
* @brief Example usage of utilc-logging library.
* -- RULE_3_2_CD_do_not_use_special_characters_in_filename
* -- RULE_8_1_A_provide_file_info_comment
*/
#include <utilc-logging.h>

#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[]){
	struct ucl_s ucl;
	ucl_init(&ucl);
	uint32_t err;

	/* Add as many destinations as necessary,
	here we print to stdout and a log file.
	*/
	ucl_dest_h dest1 = ucl_add_dest(&ucl, UCL_DEST_STDOUT);
	ucl_dest_h dest2 = ucl_add_dest(&ucl, UCL_DEST_FILE, "/home/cameron/logs/test.log");

	/* For UCL_DEST_FILE destinations we can set a maximum nuber of bytes.
	   After the file reaches this size, messages from the end of beginning
		 of the file will be removed. */

	// Limit the dest 2 file to 10
	ucl_set_dest(dest2, UCL_DPROP_MAX_FILE_SIZE, 1024 * 10);
	//ucl_dest_h dest3 = ucl_add_dest(&ucl, UCL_DEST_STDOUT);

	/*	If destinations require modification after creation,
		keep track of the pointer returned by ucl_add_dest.
	*/
	//ucl_disable_dest(dest3);


	err = ucl_log(&ucl, UCL_LL_INFO, "This is some info, this is message %d\n", 1);
	if (err != UCL_OK){
		printf("Error: %s\r\n", ucl_err_to_string(err));
	}

	err = ucl_log(&ucl, UCL_LL_INFO, "Second message, %s message %d\n", "this is", 2);
	if (err != UCL_OK){
		printf("Error: %s\r\n", ucl_err_to_string(err));
	}
	return EXIT_SUCCESS;

	ucl_free(&ucl);
}
