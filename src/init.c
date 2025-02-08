#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void main()
{

	pid_t pid;

	// fork and run command froms /etc/rc

	pid = fork();

	if (pid == 0) {
		execl("/bin/sh", "/bin/sh", "/etc/rc", NULL);
		exit(1);
	}

	wait(NULL);

	// fork and run shell

	switch (fork()) {
	case 0:
		char *argv[2] = { "/bin/sh", NULL };
		char *env[] = { "PWD=/", "HOME=/", NULL };
		if (execve(argv[0], argv, env) == -1) {
			fprintf(stderr, "init: error executing %s: %s\n", argv[0], strerror(errno));
			exit(1);
		}

		break;

	case -1:
		printf("Failed to fork init process!\n");
		exit(1);

		break;

	default:
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
