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
int cd(char**);
int pwd(char**);
int env(char**);
int help(char**);
int quit(char**);

typedef struct {
  char* cmd;
  int (*func)(char**);
  char* params;
  char* help;
} builtin;

int num_builtins();

char* read_line();
char** parse_line(char*);
char* get_token(char*, char*);

extern char** environ;
char* bin;
int exitcode;
int execute(char**);

builtin builtins[] = { { "cd", &cd, "[-] [DIR]", "change the working directory." },
                       { "pwd", &pwd, NULL, "print the working directory." },
                       { "env", &env, NULL, "print the environment variables." },
                       { "help", &help, NULL, "print help information." },
                       { "exit", &quit, "[EXITCODE]", "terminate process."} };

int num_builtins()
{
  return sizeof(builtins) / sizeof(builtin);
}

int cd(char** args)                                 // cd, change directory
{

  if (args[1] != NULL && *args[1] == '-') {
    return (pwd(args));
  }

  int buffer_size = _POSIX_PATH_MAX;
  char* dir = malloc(sizeof(char)* buffer_size);

  if (args[1] == NULL) {                            // no directory argument given
    char* HOME_PATH = getenv("HOME");               // change directory to $HOME if
                                                    // it exists. Do nothing otherwise
    if (HOME_PATH) {
      while (strlen(HOME_PATH) > buffer_size) {
        buffer_size += _POSIX_PATH_MAX;
        dir = realloc(dir, buffer_size);

        if(!dir){
          perror(args[0]);
          return 1;
        }
      }
      dir = strcpy(dir, HOME_PATH);
    } else {
      dir = '\0';
    }

  } else {

    if (*args[1] != '/') {                          // directory argument is relative
      char* PWD = getenv("PWD");
      if (PWD) {
        while (strlen(PWD) + strlen(args[1]) + 1 > buffer_size) {
          buffer_size += _POSIX_PATH_MAX;
          dir = realloc(dir, buffer_size);

          if(!dir){
            perror(args[0]);
            return 1;
          }
        }

        dir = strcpy(dir, PWD);
        if(PWD[strlen(PWD)-1] != '/'){
          dir = strcat(dir,"/");
        }
        dir = strcat(dir,args[1]);

      } else {
        fprintf(stderr, "%s: $PWD is not set", args[0]);
        free(dir);
        return 2;
      }
    } else {                                        // directory argument is absolute
      while (strlen(args[1]) > buffer_size) {
        buffer_size += _POSIX_PATH_MAX;
        dir = realloc(dir, buffer_size);

        if(!dir){
          perror(args[0]);
          return 1;
        }
      }
      dir = strcpy(dir, args[1]);
    }
  }

  // Attempt to change directory and update PWD on success
  if (chdir(dir) != 0) {
    fprintf(stderr, "%s: can't cd into %s: %s\n", bin, dir, strerror(errno));
    free(dir);
    return 1;
  } else {
    setenv("PWD", dir, 1);
  }

  free(dir);
  return 0;
}

// print the working directory
int pwd(char** args)
{
  // TODO: -P
  char* pwd = getenv("PWD");
  if(pwd){
    printf("%s\n", getenv("PWD"));
  } else {
    puts("PWD not set.");
    return 1;
  }

  return 0;
}

int env(char** args)
{
  for (int i = 0; environ[i] != NULL; i++) {
    printf("%s\n", environ[i]);
  }
  return 0;
}

int quit(char** args)
{
  if (args[1] == NULL) {
    exit(0);
  } else {
    int exitcode = atoi(args[1]);
    if (exitcode){
      exit(exitcode);
    } else {
      fprintf(stderr, "%s: illegal number %s\n", args[0], args[1]);
      return 1;
    }
  }
}

int help(char** args)
{
  char* params;
  for (int i = 0; i < num_builtins(); i++) {
    params = (builtins[i].params) ? builtins[i].params : "\t";
    printf("%s %s\t%s\n", builtins[i].cmd, params, builtins[i].help);
  }
  return 0;
}

char* read_line()   // Read characters from stdin to line buffer
{
  int buffer_size = _POSIX_MAX_CANON;                   // Initial buf size
  int position    = 0;
  char* buffer    = malloc(sizeof(char) * buffer_size); // Allocate buffer
  bool escaped    = false;                              // \ not used yet      
  int c;                                                // Int because EOF
                                                        // is an integer

  if (!buffer) {    // Exit when buffer failed to allocate
    perror(bin);
    exit(1);
  }

  for(;;) {
    c = getchar();  

    switch(c) {
      case '\\':                    // Escape character
        escaped = !escaped;
        buffer[position++] = c;
        break;

      case '\n':                    // New line
        if (!escaped) {
          buffer[position] = '\0';  // Terminate line if not escaped
          return buffer;
        } else {
          escaped = false;
        }
        break;

      case 0x0C:                    // ^L
        break;

      case EOF:                     // Kill shell with Ctrl+D
        free(buffer);
        exit(1);

      default:                      // Put character in line buffer
        buffer[position++] = c;
        escaped = false;            // Reset character escape
        break;

    }

    if (position >= buffer_size) {  // Dynamically resize line buffer
      buffer_size += _POSIX_MAX_CANON;
      buffer = realloc(buffer, buffer_size);

      if(!buffer){
        perror(bin);
        exit(1);
      }
    }
  }
}

char** parse_line(char* line)     // Parse line buffer to token array
{
  int position = 0;
  int buffer_size = CMD_BUFFER_SIZE;
  char **cmd = malloc(buffer_size * sizeof(char*));
  char *token;

  if (!cmd) {
    perror(bin);
    exit(1);
  }

  token = get_token(line, DELIMITERS);
  while (token != NULL) {
    cmd[position++] = token;

    if (position >= buffer_size) {      // Dynamically resize token buffer
      buffer_size += CMD_BUFFER_SIZE;
      cmd = realloc(cmd, buffer_size);

      if(!cmd){
        perror(bin);
        exit(1);
      }
    }

    token = get_token(NULL, DELIMITERS);
  }

  cmd[position] = NULL;
  return cmd;
}


// Get token from string
char* get_token(char* input, char* delimiters)
{
  bool quoted = false;                  // Ignore delimiter between quotes
  static char* s = NULL;                // Variable to remember the previous input
  char* token;                          // Pointer to the current token
  char* delimiter;                      // Pointer to the delimiters

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
    if (*input == '\\'){
      // Remove '\' from input string
      for(int i = 0; input[i] != '\0'; i++){
        input[i] = input[i+1];
      }
      // Skip processing the character following the '\'
      input++;
      continue;
    }

    // Compare with every delimiter
    delimiter = delimiters;
    while(*delimiter != '\0'){
      if (*input == *delimiter && !quoted){
        // Terminate input string a delimiter
        *input = '\0';
        s = input+1;

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

  return WEXITSTATUS(status);

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
