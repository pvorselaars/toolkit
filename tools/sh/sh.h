#ifndef SH_H
#define SH_H

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "builtins.h"
#include "parse.h"

char* bin;
int execute(char**);

#endif
