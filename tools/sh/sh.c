#include "sh.h"

int execute(char** args)
{

  if (args[0] == NULL) {
    return 1;                           // No command entered
  }

  for (int i = 0; i < num_builtins(); i++){       // check for builtin functions
    if (strcmp(args[0], builtins[i].cmd) == 0) {
      return (*builtins[i].func)(args);
    }
  }

  pid_t pid = fork();
  int status;
  switch (pid) {
    case 0:                             // Child process
      if(execvp(args[0], args) == -1){  // Execute the arg array with 
        perror(bin);                    // with current environment
      }
      exit(1);

    case -1:                            // Something went wrong
      perror(bin);

    default:                            // Parent process
      do {                              // Wait on child process exit or termination
        waitpid(pid, &status, WUNTRACED);
      } while ( !WIFEXITED(status) && !WIFSIGNALED(status) );

  }

  return 1;
}

int main(int argc, char *argv[])
{
  bin = argv[0];                        // name of the shell binary
  char* line;                           // the input string
  char** cmd;                           // the input string as token array
  int status;                           // exit code from command

  do {
    printf("# ");                       // print the prompt
    fflush(stdout);

    line   = read_line();               // read from input
    cmd    = parse_line(line);          // parse the input to a token array
    status = execute(cmd);              // try to execute the token array

    free(line);
    free(cmd);
  } while (status);

  exit(0);
}
