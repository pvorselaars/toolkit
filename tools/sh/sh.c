#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>

#include "sh.h"

#define LINE_BUFFER_SIZE 64
#define CMD_BUFFER_SIZE 8

static char* bin;

char* read_line()   // Read characters from stdin to line buffer
{
  int buffer_size = LINE_BUFFER_SIZE;                   // Initial buf size
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
        if (escaped) {
          buffer[position++] = c;
          escaped = false;
        } else {
          escaped = true;
        }
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
      buffer_size += LINE_BUFFER_SIZE;
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

  token = get_token(line);
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

    token = get_token(NULL);
  }

  cmd[position] = NULL;
  return cmd;
}

char* get_token(char* input)            // Get token from string
{
  bool quoted = false;                  // Ignore delimiter between quotes
  static char* s = NULL;                // Variable to remember the previous input
  char* token;                          // Pointer to the current token

  if (input == NULL) {                  // If no input is given, use the old input
    input = s;
  }

  if (input == NULL) {                  // No further tokens
    return NULL;
  }

  if (*input == '\0') {                 // Terminate on an empty string
    s = NULL;
    return NULL;
  }

  token = input;                        // Point token to start of input
  for(int i = 0; input[i]!='\0'; i++){  // Scan input until null terminator
    if (input[i] == '"' ) {             // When a quote is encountered:
      quoted = !quoted;                 // ignore subsequent delimiters
      continue;                         // until next quote
    }

    if ((input[i] == ' ' || \
         input[i] == '\t') && !quoted) {
      input[i] = '\0';                  // Terminate input at delimiter
      s = &input[i+1];                  // Point old input to next character
      return token;                     // Return the token
    }
  }
  s = NULL;                             // No further tokens to be found
  return token;
}

int execute(char** args){

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

void help()
{
  printf("Ctrl+D to quit\n");
}

int main(int argc, char *argv[])
{
  bin = argv[0];                        // name of the shell binary
  char* line;                           // the input string
  char** cmd;                           // the input string as token array
  int status;                           // exit code from command

  help();
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
