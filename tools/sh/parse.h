#ifndef PARSE_H
#define PARSE_H

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define CMD_BUFFER_SIZE 8
#define PATH_BUFFER_SIZE 8
#define DELIMITERS " \t\r\n\a"

char* read_line();
char** parse_line(char*);
char* get_token(char*, char*);

// shell binary name
extern char* bin;

#endif
