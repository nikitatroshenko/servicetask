//
// Created by nikita on 10.10.17.
//

#ifndef VALIDATIONTASK_ERR_HANDLING_H
#define VALIDATIONTASK_ERR_HANDLING_H

#define CUSTOM_ERR_MASK	0x97000000

// ERRORS
#define EBADARGS	(CUSTOM_ERR_MASK | 0x022)
#define EBADMD5		(CUSTOM_ERR_MASK | 0x0D5)
#define EBADCRC32	(CUSTOM_ERR_MASK | 0xC32)
#define EHASHMISMATCH	(CUSTOM_ERR_MASK | 0x111)
#define EOPENSSLFAIL	(CUSTOM_ERR_MASK | 0x551)

#define EBADARGS_MSG		"Invalid usage"
#define EBADMD5_MSG		"Wrong MD5 format"
#define EBADCRC32_MSG		"Wrong CRC32 format"
#define EHASHMISMATCH_MSG	"Hashes don't match"
#define EOPENSSLFAIL_MSG	"OpenSSL call failed"

// Unified approach to error logging. If errno is set to one of macros above,
// log_error() resolves error message itself, otherwise it delegates this job
// to perror(const char *). Logs error to stderr in format:
// <executable_name>
void log_error();

#endif //VALIDATIONTASK_ERR_HANDLING_H