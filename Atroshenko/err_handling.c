//
// Created by nikita on 10.10.17.
//

#include "err_handling.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef EXECUTABLE_NAME
#define EXECUTABLE_NAME "servicetask"
#endif

#define EXECUTEBLE_NAME_LEN strlen(EXECUTABLE_NAME)

// primitive error message lookup
static const char *lookup_custom_error(int err) {
	if (err == EBADARGS)
		return EBADARGS_MSG;
	else if (err == EBADMD5)
		return EBADMD5_MSG;
	else if (err == EBADCRC32)
		return EBADCRC32_MSG;
	else if (err == EHASHMISMATCH)
		return EHASHMISMATCH_MSG;
	else if (err == EOPENSSLFAIL)
		return EOPENSSLFAIL_MSG;
	else if (err == EFAILINIREAD)
		return EFAILINIREAD_MSG;
	else if (err == ENOCONFIG)
		return ENOCONFIG_MSG;
}

void log_error() {
	const char *err_format = "%s error (0x%08x)";
	const size_t err_msg_len = EXECUTEBLE_NAME_LEN
				   + strlen(err_format) + 8;
	char err_msg[err_msg_len];
	int preserved_errno = errno;	// errno may be overwritten by snprintf

	snprintf(err_msg, err_msg_len,
		 err_format, EXECUTABLE_NAME, preserved_errno);

	if (preserved_errno & CUSTOM_ERR_MASK) {
		fprintf(stderr, "%s: %s\n", err_msg,
			lookup_custom_error(preserved_errno));
	} else {
		errno = preserved_errno;
		perror(err_msg);
	}
}