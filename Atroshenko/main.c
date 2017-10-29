#include "err_handling.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>

#define DEFAULT_FILE_BUF_SIZE 512

static int get_file_md5(FILE *in, unsigned char *digest);

int main(int argc, char const **argv)
{
	size_t i;	/* loop counter */
	unsigned char digest[MD5_DIGEST_LENGTH];
	FILE *in = fopen(argv[1], "rb");

	if (in == NULL) {
		return errno;
	}

	if (!get_file_md5(in, digest)) {
		log_error();
		return errno;
	}

	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		printf("%2x", digest[i]);
	}
	printf("\n");

	return EXIT_SUCCESS;
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