#include "err_handling.h"
#include "config.h"
#include "daemonize.h"
#include "listen_changes.h"

#include <errno.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <time.h>

#define DEFAULT_FILE_BUF_SIZE 512

#define CONFIG_ARG "--config"

struct file_change_handling_data {
	FILE *log;
	FILE *target;
};

static int get_file_md5(FILE *in, unsigned char *digest);
static void bytes_to_hex_str(unsigned char *bytes, size_t len, char *buf);
static void log_record(FILE *out, const char *record);
static void file_change_handling_routine(void *data);

int main(int argc, char const **argv)
{
	size_t i;	/* loop counter */
	struct file_change_handling_data routine_data;
	struct listen_ctx *listen_ctx;
	struct configuration *conf = NULL;
	size_t config_arg = -1;

	go_background();

	for (i = 0; i < argc; i++) {
		fprintf(stderr, "Argument '%s'\n", argv[i]);
		if (!strncmp(CONFIG_ARG, argv[i], strlen(CONFIG_ARG)))
			config_arg = i + 1;
	}

	fprintf(stderr, "Config arg: %d\n", config_arg);

	if (config_arg < argc) {
		fprintf(stderr, "loading config %s\n", argv[1]);
		conf = load_config(argv[config_arg]);	
	} else {
		fprintf(stderr, "loading default config\n");
		conf = load_default_config();
	}

	fprintf(stderr, "Loaded\n");
	fprintf(stderr, "Read config: log_path=%s\ntarget_path=%s\n", conf->log_path, conf->target_path);

	if (conf == NULL || conf->log_path == NULL
			|| conf->target_path == NULL) {

		log_error();
		fprintf(stderr, "Config is null\n");
		return errno;
	}


	routine_data.log = fopen(conf->log_path, "at");
	routine_data.target = fopen(conf->target_path, "rt");

	if (routine_data.log == NULL || routine_data.target == NULL) {
		fprintf(stderr, "Config data null\n");
		log_error();
		return errno;
	}

	fprintf(stderr, "Starting listen\n");

	listen_ctx = start_listen_changes(
			conf->target_path,
			file_change_handling_routine,
			&routine_data);

	while (1) {}

	fprintf(stderr, "Stopping listen\n");
	stop_listen_changes(listen_ctx);
	free_config(conf);

	fprintf(stderr, "Exiting\n");

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
		sprintf(buf, "%02x", bytes[i]);
		buf += 2;
	}
}

void log_record(FILE *out, const char *record)
{
	time_t rawtime;
	struct tm *tm;

	time(&rawtime);
	tm = localtime(&rawtime);

	fprintf(out, "[%4d.%02d.%02d %02d:%02d:%02d] %s\n",
			tm->tm_year + 1900,
			tm->tm_mon + 1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			record);
}

void file_change_handling_routine(void *data)
{
	struct file_change_handling_data *routine_data = data;
	unsigned char digest[MD5_DIGEST_LENGTH];
	char digest_str[2 * MD5_DIGEST_LENGTH + 1];

	if (!get_file_md5(routine_data->target, digest))
		log_error();

	bytes_to_hex_str(digest, MD5_DIGEST_LENGTH, digest_str);
	log_record(routine_data->log, digest_str);

	fflush(routine_data->log);
}