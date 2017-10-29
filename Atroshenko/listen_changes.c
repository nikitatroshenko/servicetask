#include "listen_changes.h"
#include "err_handling.h"

#include <sys/inotify.h>
#include <unistd.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct listen_ctx {
	int inotify_fd;
	int watch_fd;
};

static void free_preserve_errno(void *mem);
static void close_preserve_errno(int fd);

struct listen_ctx *start_listen_changes(
		const char *path,
		listen_callback callback,
		void *data)
{
	struct listen_ctx *ctx = calloc(1, sizeof *ctx);
	struct inotify_event event;

	ctx->inotify_fd = inotify_init();
	if (ctx->inotify_fd == -1) {
		free_preserve_errno(ctx);
		return NULL;
	}
	ctx->watch_fd = inotify_add_watch(
				ctx->inotify_fd,
				path,
				IN_CLOSE_WRITE);
	if (ctx->watch_fd == -1) {
		close_preserve_errno(ctx->inotify_fd);
		free_preserve_errno(ctx);
		return NULL;
	}

	while (1) {
		read(ctx->inotify_fd, &event, sizeof event + NAME_MAX + 1);
		callback(data);
	}
}

void stop_listen_changes(struct listen_ctx *ctx)
{
	inotify_rm_watch(ctx->inotify_fd, ctx->watch_fd);
	close(ctx->inotify_fd);
	free(ctx);
}

void free_preserve_errno(void *mem) {
	int err = errno;

	free(mem);
	errno = err;	
}

void close_preserve_errno(int fd) {
	int err = errno;

	close(fd);
	errno = err;
}