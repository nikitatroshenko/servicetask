#ifndef SERVICETASK_LISTEN_CHANGES
#define SERVICETASK_LISTEN_CHANGES

typedef void(*listen_callback)(void *);

struct listen_ctx;

struct listen_ctx *start_listen_changes(
		const char *path,
		listen_callback callback,
		void *data);

void stop_listen_changes(struct listen_ctx*);

#endif