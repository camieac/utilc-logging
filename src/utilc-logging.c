/**
* @file utic-timing.c
* @author Cameron A. Craig
* @date 28 Dec 2016
* @version 0.1.0
* @copyright 2016 Cameron A. Craig
* @brief Measure time without thinking about the arithmetic.
* -- RULE_3_2_CD_do_not_use_special_characters_in_filename
* -- RULE_8_1_A_provide_file_info_comment
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utilc-logging.h"

#define MSG_TIMESTAMP_BYTES 29
#define NULL_TERMINATOR 1

static const char * ucl_err_string[] = {
	"Error",
	"Ok",
	"Invalid destination type"
};

const char * ucl_err_to_string(enum ucl_error_code_e err){
	if(err < 0 || (err > MAX_ERR_NUM)){
		return "Error";
	}

	return ucl_err_string[err];
}

static const char * ucl_dest_type_string[] = {
	"UCL_DEST_FILE",
	"UCL_DEST_STDOUT",
	"UCL_DEST_STDERR",
	"UCL_DEST_UDP"
};

char * ucl_dest_type_to_string(enum ucl_dest_type_e err){
	if(err < 0 || (err > MAX_DEST_NUM)){
		return "Error";
	}
	return ucl_dest_type_string[err];
}

static const char * ucl_log_level_string[] = {
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR"
};

char * ucl_log_level_to_string(enum ucl_log_level_e err){
	if(err < 0 || (err > MAX_LOG_LEVEL_NUM)){
		return "Error";
	}
	return ucl_log_level_string[err];
}

uint32_t ucl_init(ucl_h ucl){
	ucl->dests = NULL; //NULL indicates no dests
	ucl->num_dests = 0;

	ucl->flags = 0;
	ucl->flags |= UCL_FLAGS_TIMESTAMP;
	ucl->flags |= UCL_FLAGS_LOG_LEVEL;
}

static void tv_to_timestamp(char *m, struct timeval *tv){
	char timestamp[64];
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64];
	nowtime = tv->tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(timestamp, sizeof(timestamp), "%s.%06d", tmbuf, tv->tv_usec);
	strcat(m, "[");
	strcat(m, timestamp);
	strcat(m,"] ");
}

static uint32_t dest_log(ucl_h ucl, ucl_dest_h dest, struct ucl_message_s *message){
	if(!(dest->flags & UCL_FLAGS_DEST_ENABLED)){
		return UCL_ERROR_DEST_DISABLED;
	}

	const char* msg_user = message->message;
	size_t msg_user_size = strlen(msg_user);

	const char *msg_log_level;
	size_t msg_log_level_size = 0U;
	if(ucl->flags & UCL_FLAGS_LOG_LEVEL){
		msg_log_level = ucl_log_level_to_string(message->log_level);
		msg_log_level_size = strlen(msg_log_level);
	}

	char msg_timestamp[MSG_TIMESTAMP_BYTES];
	memset(msg_timestamp, 0, sizeof(msg_timestamp));
	size_t msg_timestamp_size = 0U;
	if(ucl->flags && UCL_FLAGS_TIMESTAMP){
		tv_to_timestamp(msg_timestamp, &message->tv);
		msg_timestamp_size = MSG_TIMESTAMP_BYTES;
	}

	//Work out the size of c-string we need to print
	char msg_full[msg_user_size + msg_timestamp_size + msg_log_level_size];
	memset(msg_full, 0, sizeof(msg_full));

	if(ucl->flags && UCL_FLAGS_TIMESTAMP){
		strcat(msg_full, msg_timestamp);
	}

	if(ucl->flags & UCL_FLAGS_LOG_LEVEL){
		strcat(msg_full, msg_log_level);
	}

	strcat(msg_full, ": ");
	strcat(msg_full, msg_user);

	char * filename;
	switch(dest->type){
		case UCL_DEST_FILE:
			filename = dest->conf.file.filename;

			//Try to open the specified log file
			FILE *log_file = fopen(filename, "ab+");
			if(log_file == NULL){
				return UCL_ERROR;
			}

			vfprintf(log_file, msg_full, message->args);

			fclose(log_file);
			break;
		case UCL_DEST_STDOUT:
			vfprintf(stdout, msg_full, message->args);
			break;
		case UCL_DEST_STDERR:
			vfprintf(stderr, msg_full, message->args);
			break;
		case UCL_DEST_UDP:
		default:
			return UCL_ERROR_INVALID_DEST;
	}
}

uint32_t ucl_log(ucl_h ucl, enum ucl_log_level_e log_level, const char *message, ...){
	//Create a message struct
	struct ucl_message_s m = {
		.message = message,
		.log_level = log_level
	};

	//Mark timestamp
	if(ucl->flags && UCL_FLAGS_TIMESTAMP){
		gettimeofday(&m.tv, NULL);
	}

	int i;
	for(i = 0; i < ucl->num_dests; i++){
		va_list args;
		va_start(args, message);
		va_copy(m.args, args);
		dest_log(ucl, &ucl->dests[i], &m);
		va_end(args);
		va_end(m.args);
	}
	return UCL_OK;
}

ucl_dest_h ucl_add_dest(ucl_h ucl, enum ucl_dest_type_e dest_type, ...){
	va_list args;
	va_start(args, dest_type);
	struct ucl_dest_s new_dest = {
		.log_level = UCL_LL_DEBUG,
		.type = dest_type,
		.flags = UCL_FLAGS_DEST_ENABLED
	};

	if (dest_type == UCL_DEST_FILE){
			//Make note of the filename for later use
			new_dest.conf.file.filename = va_arg(args, char*);

			//Attempt to open file (will attempt to create if it doesn't exist (+))
			FILE *f = fopen(new_dest.conf.file.filename, "a+");

			if((f == NULL) && (errno == ENOENT)){
				char * filepath = strdup(new_dest.conf.file.filename);

				if(mkdir(dirname(filepath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
					return UCL_ERROR;
				}
			} else {
				fclose(f);
			}

	}

	struct ucl_dest_s *tmp_ptr = realloc(ucl->dests, (ucl->num_dests + 1) * sizeof(struct ucl_dest_s));
	if(tmp_ptr == NULL){
		free(ucl->dests);
		return NULL;
	} else {
		ucl->dests = tmp_ptr;
	}

	memcpy(&ucl->dests[ucl->num_dests], &new_dest, sizeof(struct ucl_dest_s));
	ucl->num_dests++;
	va_end(args);

	return &ucl->dests[ucl->num_dests - 1];
}

uint32_t ucl_disable_dest(ucl_dest_h dest){
	dest->flags &= ~UCL_FLAGS_DEST_ENABLED;
	return UCL_OK;
}

uint32_t ucl_free(ucl_h ucl){
	//Free destinations.
	free(ucl->dests);
}
