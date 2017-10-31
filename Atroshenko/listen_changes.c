#include "listen_changes.h"
#include "err_handling.h"

#include <signal.h>		/* sigaction(), struct sigaction, siginfo_t */
#include <sys/inotify.h>	/* inotify_init(), inotify_add_watch() */
#include <unistd.h>		/* read(), close() */
#include <fcntl.h>		/* fcntl(), F_SETXXX, F_GETXXX macros */

#include <errno.h>
#include <limits.h>		/* NAME_MAX */
#include <stdlib.h>
#include <string.h>		/* memset() */

#define INOTIFY_EVENT_SIZE (sizeof (struct inotify_event) + NAME_MAX + 1)

struct listen_ctx {
	int inotify_fd;
	int watch_fd;
	listen_callback callback;
	void *udata;
};

static void add_async_signal_handler(struct listen_ctx *ctx);
static void dispose_listen_ctx(struct listen_ctx *ctx);
static void dispose_listen_ctx_preserve_errno(struct listen_ctx *ctx);

/*
 * A little bit of magic if it is really necessary not to use loops
 * to poll the I/O events
 * Here we will allocate context as a global static variable to have
 * access to it from signal function
 */
#if	_SERVICETASK_USE_SIGNALS

static struct listen_ctx listen_changes_context = {0};

static void inotify_event_handler(
		int signum,
		siginfo_t *siginfo,
		void *ucontext)
{
	if (signum == SIGIO
		&& siginfo->si_fd == listen_changes_context.inotify_fd
		&& siginfo->si_code == POLL_IN) {

		struct inotify_event event;

		read(listen_changes_context.inotify_fd,
			&event,
			INOTIFY_EVENT_SIZE);
		listen_changes_context.callback(listen_changes_context.udata);
	} else if (signum == SIGINT || signum == SIGTERM) {
		listen_changes_context.callback(listen_changes_context.udata);
		dispose_listen_ctx(&listen_changes_context);
		exit(EXIT_SUCCESS);
	}
}

/*
 * Simply forbid calling this function more than once because we use 
 * global variable to store changes listen context
 */
static struct listen_ctx *internal_listen_ctx_alloc()
{
	static int ctx_is_allocated = 0;

	if (!ctx_is_allocated) {
		ctx_is_allocated = 1;
		return &listen_changes_context;
	} else {
		errno = EBUSY;
		return NULL;
	}
}

void dispose_listen_ctx(struct listen_ctx *ctx) {
	close(ctx->inotify_fd);
}

void add_async_signal_handler(struct listen_ctx *ctx)
{
	struct sigaction sig_action;
	struct sigaction old_action;

	memset(&sig_action, 0, sizeof sig_action);
	sig_action.sa_sigaction = inotify_event_handler;
	// sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = SA_SIGINFO | SA_RESTART;

	/*
	 * set up async flag on the given inotify fd
	 * and make the given inotify instance
	 * send SIGIO signals to this process
	 */

	fcntl(ctx->inotify_fd, F_SETFL, O_ASYNC, 1);
	fcntl(ctx->inotify_fd, F_SETOWN, getpid());
	fcntl(ctx->inotify_fd, F_SETSIG, SIGIO);

	sigaction(SIGINT, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGINT, &sig_action, NULL);
	sigaction(SIGTERM, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGTERM, &sig_action, NULL);
	sigaction(SIGIO, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGIO, &sig_action, NULL);
}

#else	/* !_SERVICETASK_USE_SIGNALS */

#error Servicetask without signals and global variables is not yet implemented

static struct listen_ctx *internal_listen_ctx_alloc()
{
	return calloc(1, sizeof (struct listen_ctx));
}

void dispose_listen_ctx(struct listen_ctx *ctx)
{
	close(ctx->inotify_fd);
	free(ctx);
}

#endif	/* _SERVICETASK_USE_SIGNALS */

struct listen_ctx *start_listen_changes(
		const char *path,
		listen_callback callback,
		void *data)
{
	struct listen_ctx *ctx = internal_listen_ctx_alloc();
	struct inotify_event event;

	if (ctx == NULL) {
		return NULL;
	}

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
	ctx->udata = data;
	ctx->callback = callback;

	add_async_signal_handler(ctx);

	while (1) {
		// int bytes_read = read(ctx->inotify_fd, &event, INOTIFY_EVENT_SIZE);

		// if (bytes_read == -1 || bytes_read == 0) {
		// 	dispose_listen_ctx_preserve_errno(ctx);
		// 	return NULL;
		// }

		// callback(data);
	}

	return ctx;
}

void stop_listen_changes(struct listen_ctx *ctx)
{
	dispose_listen_ctx(ctx);
}

void dispose_listen_ctx_preserve_errno(struct listen_ctx *ctx)
{
	int err = errno;

	dispose_listen_ctx(ctx);
	errno = err;
}
