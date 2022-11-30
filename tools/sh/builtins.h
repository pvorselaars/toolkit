#ifndef BUILTINS_H
#define BUILTINS_H

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "parse.h"

// builtin functions
int cd(char**);
int ls(char**);
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

extern builtin builtins[];
extern char** environ;

// shell binary name
extern char* bin;

#endif
