#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int execveat(int dirfd, const char *pathname, char *argv[], char *envp[], int flags);

void tokenize(char **tokens, char string[], char delimiters[])
{
	int t = 0;
	char *delimiter;
	tokens[t++] = string;

	while (*string != 0)
	{

		delimiter = delimiters;

		while (*delimiter != 0)
		{
			if (*string == *delimiter)
			{
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

	int bindir = open("/bin", O_RDONLY | O_DIRECTORY);

	if (bindir < 0)
		return bindir;

	for (;;)
	{
		write(1, "# ", 2);

		count = read(0, command, sizeof(command));
		command[count - 1] = 0;

		tokenize(tokens, command, " ");

		if (fork() == 0)
		{

			int e = execveat(bindir, tokens[0], tokens, envp, 0);

			write(2, argv[0], 7);

			if (e == -2)
			{
				write(2, ": no such file\n", 16);
			}
			else
			{
				write(2, ": error\n", 8);
			}
		}
		else
		{
			waitid(P_ALL, 0, 0, WEXITED);
		}
	}

	close(bindir);

	return 0;
}
