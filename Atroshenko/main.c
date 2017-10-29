#include "err_handling.h"
#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <time.h>

#define DEFAULT_FILE_BUF_SIZE 512

static int get_file_md5(FILE *in, unsigned char *digest);
static void bytes_to_hex_str(unsigned char *bytes, size_t len, char *buf);
static void log_record(FILE *out, const char *record);

int main(int argc, char const **argv)
{
	size_t i;	/* loop counter */
	unsigned char digest[MD5_DIGEST_LENGTH];
	char digest_str[2 * MD5_DIGEST_LENGTH + 1];
	FILE *in = fopen(argv[1], "rb");
	struct configuration *conf = load_config(argv[2]);

	if (in == NULL) {
		log_error();
		return errno;
	}

	if (conf == NULL) {
		log_error();
		return errno;
	}

	if (!get_file_md5(in, digest)) {
		log_error();
		return errno;
	}

	bytes_to_hex_str(digest, MD5_DIGEST_LENGTH, digest_str);
	log_record(stderr, digest_str);
	log_record(stderr, conf->target_path);
	log_record(stderr, conf->log_path);

	free_config(conf);

	return 0;
}

int get_file_md5(FILE *in, unsigned char *digest)
{
	MD5_CTX context;
	size_t bytes;
	char buf[DEFAULT_FILE_BUF_SIZE];
	int success_call;

	success_call = MD5_Init(&context);

	if (!success_call)
		return 0;

	bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);

	while (bytes > 0) {
		success_call = MD5_Update(&context, buf, bytes);

		if (!success_call) 
			return 0;

		bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);
	}

	success_call = MD5_Final(digest, &context);

	return success_call;
}

void bytes_to_hex_str(unsigned char *bytes, size_t len, char *buf)
{
	size_t i;

	for (i = 0; i < len; i++) {
		sprintf(buf, "%2x", bytes[i]);
		buf += 2;
	}
}

void log_record(FILE *out, const char *record)
{
	time_t rawtime;
	struct tm *tm;

	time(&rawtime);
	tm = localtime(&rawtime);

	fprintf(out, "[%4d.%2d.%2d %2d:%2d:%2d] %s\n",
			tm->tm_year + 1900,
			tm->tm_mon + 1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			record);
}
