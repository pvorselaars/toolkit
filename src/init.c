#include <unistd.h>
#include <sys/wait.h>

void main()
{

	// fork and run shell with startup command in /etc/rc
	if (fork() == 0) {
		char *argv[] = { "/bin/sh", "/etc/rc", NULL };
		char *env[] = { "PWD=/", "HOME=/", NULL };
		execve(argv[0], argv, env);
	}

	waitid(P_ALL, 0, 0, WEXITED);

}
