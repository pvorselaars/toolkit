#include "builtins.h"

builtin builtins[] = { { "cd", &cd, "[-] [DIR]", "change the current directory." },
                       { "pwd", &pwd, NULL, "print the current/working directory." },
                       { "env", &env, NULL, "print the environment variables." },
                       { "help", &help, NULL, "print help information." },
                       { "exit", &quit, "[EXITCODE]", "terminate process."} };

int num_builtins()
{
  return sizeof(builtins) / sizeof(builtin);
}

int pwd(char** args)                   // pwd, print working directory
{
  // TODO: -P
  char* pwd = getenv("PWD");
  if(pwd){
    printf("%s\n", getenv("PWD"));
  }

  return 1;
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
          exit(1);
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
            exit(1);
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
        return 1;
      }
    } else {                                        // directory argument is absolute
      while (strlen(args[1]) > buffer_size) {
        buffer_size += _POSIX_PATH_MAX;
        dir = realloc(dir, buffer_size);

        if(!dir){
          perror(args[0]);
          exit(1);
        }
      }
      dir = strcpy(dir, args[1]);
    }
  }

  // rewrite to canonical form
  int path_buffer_size = PATH_BUFFER_SIZE;
  char** parts = malloc(sizeof(char*) * path_buffer_size);
  if (!parts) {
    fprintf(stderr, "%s: %s: %s", bin, args[0], strerror(errno));
  }

  int position = 0;
  char* part   = get_token(dir, "/");

  while (part) {
    if (strcmp(part, ".") == 0) { 
      // ignore
    } else if (strcmp(part, "..") == 0) {
      // remove preceding part
      if (position > 0) {
        position--;
        parts[position] = NULL;
      }
    } else {
      // add part to part list for path
      parts[position] = part;
      position++;
    }

    if (position > path_buffer_size) {
      path_buffer_size += PATH_BUFFER_SIZE;
      parts = realloc(parts, path_buffer_size);

      if(!parts){
        perror(args[0]);
        exit(1);
      }
    }

    part = get_token(NULL, "/");
  }
  parts[position] = NULL;

  char* path = malloc(sizeof(char)* buffer_size);
  path[0] = '/';
  for (int i = 0; parts[i] != NULL; i++){
    strcat(path, parts[i]);
    // Do not append the final /
    if(i != position - 1){
      strcat(path, "/");
    }
  }

  // Attempt to change directory and update PWD on success
  if (chdir(path) != 0) {
    fprintf(stderr, "%s: can't cd into %s: %s\n", bin, path, strerror(errno));
  } else {
    setenv("PWD", path, 1);
  }

  free(dir);
  free(path);
  free(parts);
  return 1;
}

int env(char** args)
{
  for (int i = 0; environ[i] != NULL; i++) {
    printf("%s\n", environ[i]);
  }
  return 1;
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
    }
  }
  return 1;
}

int help(char** args)
{
  char* params;
  for (int i = 0; i < num_builtins(); i++) {
    params = (builtins[i].params) ? builtins[i].params : "\t";
    printf("%s %s\t%s\n", builtins[i].cmd, params, builtins[i].help);
  }
  return 1;
}
