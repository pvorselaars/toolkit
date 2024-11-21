#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#ifndef TOOLKIT_VERSION
#define TOOLKIT_VERSION "unknown"
#endif

void main()
{
	printf("Toolkit (%s)\n", TOOLKIT_VERSION);	// TODO: better name and versioning

	pid_t pid;

	switch (fork()) {
	case 0:		// The child process
		char *argv[2] = { "/bin/sh", NULL };
		char *env[] = { "PWD=/", "HOME=/", NULL };
		if (execve(argv[0], argv, env) == -1) {
			fprintf(stderr, "init: error executing %s: %s\n",
				argv[0], strerror(errno));
			exit(1);
		}

		break;

	case -1:		// Something bad happened
		printf("Failed to fork init process!\n");
		exit(1);

		break;

	default:		// The parent process
		for (;;) {	// Wait for child processes to exit
			pid = wait(NULL);
			if (pid == -1) {
				printf("init: %s. Goodbye!\n", strerror(errno));
				return;
			} else {
				printf("PID %d has exited.\n", pid);
			}
		}

		break;
	}

}
