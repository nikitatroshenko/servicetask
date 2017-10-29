#include "config.h"
#include "ini.h"
#include "err_handling.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define CONF_LOG_PATH		"log_path"
#define CONF_TARGET_PATH	"target_path"

#define property_is(key) (!strncmp(name, key, strlen(key)))

static int config_prop_handler(
	void *user,
	const char *section,
	const char *name,
	const char *value)
{

	struct configuration *conf = (struct configuration *) user;

	if (strncmp(section, "", 1))
		return 0; /* Needed configuration is in default section */

	if (property_is(CONF_LOG_PATH))
		conf->log_path = strdup(value);
	else if (property_is(CONF_TARGET_PATH))
		conf->target_path = strdup(value);
	else
		return 0;

	return 1;
}

#undef property_is

struct configuration *load_config(const char *path)
{
	struct configuration *conf = calloc(1, sizeof *conf);
	int rc;

	rc = ini_parse(path, config_prop_handler, conf);
	if (rc == -1)
		errno = ENOCONFIG;
	else if (rc == -2)
		errno = ENOMEM;
	else if (rc != 0)
		errno = EFAILINIREAD;

	return (rc != 0) ? NULL : conf;
}

void free_config(struct configuration *conf)
{
	free(conf->log_path);
	free(conf->target_path);
}