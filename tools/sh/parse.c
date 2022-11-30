#include "parse.h"

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
