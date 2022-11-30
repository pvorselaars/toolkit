#include "sh.h"

int execute(char** args)
{

  // no command entered
  if (args[0] == NULL) {
    return 0;
  }

  // check for builtin functions
  for (int i = 0; i < num_builtins(); i++){
    if (strcmp(args[0], builtins[i].cmd) == 0) {
      return (*builtins[i].func)(args);
    }
  }

  pid_t pid = fork();
  int status;
  switch (pid) {
    // child process
    case 0:
      // Execute the arg array with the current environment
      if(execvp(args[0], args) == -1){
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
      } while ( !WIFEXITED(status) && !WIFSIGNALED(status) );

      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      }

  }

}

void main(int argc, char *argv[])
{
  bin = argv[0];                        // name of the shell binary
  char* line;                           // the input string
  char** cmd;                           // the input string as token array

  while (1) {
    printf("# ");                       // print the prompt
    fflush(stdout);

    line     = read_line();               // read from input
    cmd      = parse_line(line);          // parse the input to a token array
    exitcode = execute(cmd);              // try to execute the token array

    free(line);
    free(cmd);
  }

}
