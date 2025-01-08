#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>

#define CMD_BUFFER_SIZE 8
#define PATH_BUFFER_SIZE 8
#define DELIMITERS " \t\r\n\a"

// builtin functions
int cd(char **);
int pwd(char **);
int env(char **);
int help(char **);
int quit(char **);

typedef struct {
	char *cmd;
	int (*func)(char **);
	char *params;
	char *help;
} builtin;

int num_builtins();

void read_line(char *);
void parse_line(char *, char **);
char *get_token(char *, char *);

extern char **environ;
char *bin;
char *line;			// the input string
char **cmd;			// the input string as token array
int exitcode;
int execute(char **);

builtin builtins[] = {
	{"cd", &cd, "[-] [DIR]", "change the working directory."},
	{"pwd", &pwd, NULL, "print the working directory."},
	{"env", &env, NULL, "print the environment variables."},
	{"help", &help, NULL, "print help information."},
	{"exit", &quit, "[EXITCODE]", "terminate process."}
};

int num_builtins()
{
	return sizeof(builtins) / sizeof(builtin);
}

int cd(char **args)		// cd, change directory
{

	char dir[_POSIX_PATH_MAX] = { 0 };
	char path[_POSIX_PATH_MAX] = { 0 };
	char *PWD = getenv("PWD");

	if (args[1] == NULL) {
		// no directory argument given
		// change directory to $HOME if
		// it exists. Do nothing otherwise.

		char *HOME_PATH = getenv("HOME");
		strncat(dir, HOME_PATH, _POSIX_PATH_MAX - strlen(dir));

	} else {

		if (*args[1] == '-') {
			char *OLDPWD = getenv("OLDPWD");
			strncat(dir, OLDPWD, _POSIX_PATH_MAX - strlen(dir));
			strncat(dir, "/", _POSIX_PATH_MAX - strlen(dir));
		} else if (*args[1] != '/') {
			// directory argument is relative to current working directory

			strncat(dir, PWD, _POSIX_PATH_MAX - strlen(dir));
			strncat(dir, "/", _POSIX_PATH_MAX - strlen(dir));
			strncat(dir, args[1], _POSIX_PATH_MAX - strlen(dir));
		}

	}

	// rewrite input to a valid path

	char *part = get_token(dir, "/");

	if (part == NULL)
		path[0] = '/';

	while (part != NULL) {

		if (*part == '.') {

			if (*part++ == '.') {
				char *pos = &path[strlen(path) - 1];
				while (*pos != '/')
					pos--;

				if (pos != path) {
					*pos = '\0';
				} else {
					*++pos = '\0';
				}
			}

		} else {

			strncat(path, "/", _POSIX_PATH_MAX - strlen(path));
			strncat(path, part, _POSIX_PATH_MAX - strlen(path));

		}

		part = get_token(NULL, "/");
	}

	// Attempt to change directory and update PWD on success
	if (chdir(path) != 0) {
		fprintf(stderr, "%s: %s: %s\n", bin, path, strerror(errno));
		return 1;
	} else {
		setenv("OLDPWD", PWD, 1);
		setenv("PWD", path, 1);
	}

	return 0;
}

// print the working directory
int pwd(char **args)
{
	// TODO: -P
	char *pwd = getenv("PWD");
	if (pwd) {
		printf("%s\n", getenv("PWD"));
	} else {
		puts("PWD not set.");
		return 1;
	}

	return 0;
}

int env(char **args)
{
	for (int i = 0; environ[i] != NULL; i++) {
		printf("%s\n", environ[i]);
	}
	return 0;
}

int quit(char **args)
{
	if (args[1] == NULL) {
		exit(0);
	} else {
		int exitcode = atoi(args[1]);
		if (exitcode) {
			exit(exitcode);
		} else {
			fprintf(stderr, "%s: illegal number %s\n", args[0], args[1]);
			return 1;
		}
	}
}

int help(char **args)
{
	char *params;
	for (int i = 0; i < num_builtins(); i++) {
		params = (builtins[i].params) ? builtins[i].params : "\t";
		printf("%s %s\t%s\n", builtins[i].cmd, params, builtins[i].help);
	}
	return 0;
}

void read_line(char *buffer)	// Read characters from stdin to line buffer
{
	int position = 0;
	bool escaped = false;	// \ not used yet
	int c;			// Int because EOF
	// is an integer

	for (;;) {
		c = getchar();

		switch (c) {
		case '\\':	// Escape character
			escaped = !escaped;
			buffer[position++] = c;
			break;

		case '\n':	// New line
			if (!escaped) {
				buffer[position] = '\0';	// Terminate line if not escaped
				return;
			} else {
				escaped = false;
			}
			break;

		case 0x0C:	// ^L
			break;

		case EOF:	// Kill shell with Ctrl+D
			exit(1);

		default:	// Put character in line buffer
			buffer[position++] = c;
			escaped = false;	// Reset character escape
			break;

		}

		if (position > _POSIX_MAX_CANON) {
			fprintf(stderr, "Input buffer overflow\n");
			return;
		}
	}
}

void parse_line(char *line, char **cmd)	// Parse line buffer to token array
{
	int position = 0;
	char *token;

	token = get_token(line, DELIMITERS);
	while (token != NULL) {
		cmd[position++] = token;

		token = get_token(NULL, DELIMITERS);
	}

	cmd[position] = NULL;
}

// Get token from string
char *get_token(char *input, char *delimiters)
{
	bool quoted = false;	// Ignore delimiter between quotes
	static char *s = NULL;	// Variable to remember the previous input
	char *token;
	char *delimiter;

	// If no input is given, use the old input
	if (input == NULL) {
		input = s;
	}
	// No further tokens
	if (input == NULL) {
		return NULL;
	}
	// Terminate on an empty string
	if (*input == '\0') {
		s = NULL;
		return NULL;
	}
	// Point to start of token
	token = input;

	// Loop through input string
	while (*input != '\0') {

		// Toggle quotation and skip quote character
		if (*input == '"') {
			quoted = !quoted;
			input++;
			continue;
		}
		// Escape next character
		if (*input == '\\') {
			// Remove '\' from input string
			for (int i = 0; input[i] != '\0'; i++) {
				input[i] = input[i + 1];
			}
			// Skip processing the character following the '\'
			input++;
			continue;
		}
		// Compare with every delimiter
		delimiter = delimiters;
		while (*delimiter != '\0') {
			if (*input == *delimiter && !quoted) {
				// Terminate input string a delimiter
				*input = '\0';
				s = input + 1;

				// Only return non empty tokens
				if (*token == '\0') {
					token++;
				} else {
					return token;
				}
			}
			delimiter++;
		}

		input++;
	}

	// No further tokens found
	s = NULL;
	token = *token == '\0' ? NULL : token;
	return token;
}

int execute(char **args)
{

	// no command entered
	if (args[0] == NULL) {
		return 0;
	}
	// check for builtin functions
	for (int i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtins[i].cmd) == 0) {
			return (*builtins[i].func) (args);
		}
	}

	pid_t pid = fork();
	int status;
	switch (pid) {
		// child process
	case 0:
		// Execute the arg array with the current environment
		if (execvp(args[0], args) == -1) {
			perror(bin);
			exit(errno);
		}
		// Something went wrong
	case -1:
		perror(bin);
		return errno;

		// Parent process
	default:
		// Wait on child process to exit or be killed
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		return WEXITSTATUS(status);

	}

}

void run(const char *filename)  {

	char *buf = NULL;
	size_t len = 0;
	ssize_t num_bytes;

	char **cmd = malloc(_POSIX_MAX_CANON / 4 * sizeof(char *));

	// open file name
	FILE *f = fopen(filename, "r");

	if (f == NULL) {
		printf("Error: %s\n", strerror(errno));
		free(cmd);
		return;
	}

	// read file line by line
	while((num_bytes = getline(&buf, &len, f)) != -1 ){
		parse_line(buf, cmd);	    // parse the input to a token array
		exitcode = execute(cmd);	// try to execute the token array
	}
	
	fclose(f);
	free(buf);
	free(cmd);
	return;
}

void main(int argc, char *argv[])
{
	bin = argv[0]; // name of the shell binary

	if (argc > 1) {
		// interpret arguments as shell scripts to run

		for (int script = 1; script < argc; script++) {
			run(argv[script]);
		}

		return;
	}

	char *line = malloc(_POSIX_MAX_CANON * sizeof(char));
	char **cmd = malloc(_POSIX_MAX_CANON / 4 * sizeof(char *));

	while (1) {
		printf("# ");	// print the prompt
		fflush(stdout);

		read_line(line);	// read from input
		parse_line(line, cmd);	// parse the input to a token array
		exitcode = execute(cmd);	// try to execute the token array

	}
	free(line);
	free(cmd);

}
