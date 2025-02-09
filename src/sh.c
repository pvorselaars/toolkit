#include <fcntl.h>
#include <unistd.h>
#include <sys.h>
#include <sys/wait.h>

const char *errors[] = {
	"unknown error",
	"operation not permitted",
	"no such file or directory",
	"no such process",
	"interrupted system call",
	"I/O error",
	"no such device or address",
	"argument list too big",
	"exec format error",
	"bad file descriptor",
	"no child process",
	"resource temporarily unvailable",
	"out of memory",
	"permission denied",
	"bad address",
	"block device required",
	"resource busy",
	"file exists",
	"cross-device link",
	"no such device",
	"not a directory",
	"is a directory",
	"invalid argument",
};

void error(const char *bin, char e)
{

	if (e < 0) {
		e = -e;
	}

	if (e > sizeof(errors) / sizeof(errors[0])) {
		e = 0;
	}

	int length = 0;
	while (bin[length++] != 0) ;
	write(2, bin, length);
	write(2, ": ", 2);

	length = 0;
	while (errors[e][length++] != 0) ;
	write(2, errors[e], length);
	write(2, "\n", 2);

}

void tokenize(char **tokens, char string[], char delimiters[])
{
	int t = 0;
	char *delimiter;
	tokens[t++] = string;

	while (*string != 0) {

		delimiter = delimiters;

		while (*delimiter != 0) {
			if (*string == *delimiter) {
				*string = 0;
				tokens[t++] = string + 1;
				continue;
			}
			delimiter++;
		}

		string++;
	}

	tokens[t++] = 0;
}

int main(int argc, char *argv[], char *envp[])
{
	int count;
	char command[255];
	char *tokens[64];
	siginfo_t info;

	int bindir = open("/bin", O_RDONLY | O_DIRECTORY);

	if (bindir < 0)
		return bindir;

	for (;;) {
		write(1, "# ", 2);

		count = read(0, command, sizeof(command));
		command[count - 1] = 0;

		tokenize(tokens, command, " ");

		if (fork() == 0) {

			int e = execveat(bindir, tokens[0], tokens, envp, 0);

			exit(e);
		} else {
			waitid(P_ALL, 0, &info, WEXITED);

			if ((char)info.si_status < 0) {
				error(tokens[0], (char)info.si_status);
			}

		}
	}

	close(bindir);

	return 0;
}
