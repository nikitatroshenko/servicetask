#include "daemonize.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

void go_background()
{
	pid_t pid, sid;
	
	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		/* Log failure (use syslog if possible) */
		perror("fork");
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		perror("Leave parent");
		exit(EXIT_SUCCESS);
	}

	perror("Now child");

	/* Change the file mode mask */
	umask(0);
}