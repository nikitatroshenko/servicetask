#include "listen_changes.h"
#include "err_handling.h"

#include <sys/inotify.h>
#include <unistd.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define INOTIFY_EVENT_SIZE (sizeof (struct inotify_event) + NAME_MAX + 1)

struct listen_ctx {
	int inotify_fd;
	int watch_fd;
};

static void dispose_listen_ctx(struct listen_ctx *ctx);
static void dispose_listen_ctx_preserve_errno(struct listen_ctx *ctx);

struct listen_ctx *start_listen_changes(
		const char *path,
		listen_callback callback,
		void *data)
{
	struct listen_ctx *ctx = calloc(1, sizeof *ctx);
	struct inotify_event event;

	ctx->inotify_fd = inotify_init();
	if (ctx->inotify_fd == -1) {
		dispose_listen_ctx_preserve_errno(ctx);
		return NULL;
	}
	ctx->watch_fd = inotify_add_watch(
				ctx->inotify_fd,
				path,
				IN_CLOSE_WRITE);
	if (ctx->watch_fd == -1) {
		dispose_listen_ctx_preserve_errno(ctx);
		return NULL;
	}

	while (1) {
		int bytes_read = read(ctx->inotify_fd, &event, INOTIFY_EVENT_SIZE);

		if (bytes_read == -1 || bytes_read == 0) {
			dispose_listen_ctx_preserve_errno(ctx);
			return NULL;
		}

		callback(data);
	}

	return ctx;
}

void stop_listen_changes(struct listen_ctx *ctx)
{
	dispose_listen_ctx(ctx);
}

void dispose_listen_ctx(struct listen_ctx *ctx)
{
	close(ctx->inotify_fd);
	free(ctx);
}

void dispose_listen_ctx_preserve_errno(struct listen_ctx *ctx)
{
	int err = errno;

	dispose_listen_ctx(ctx);
	errno = err;
}
