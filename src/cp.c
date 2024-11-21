#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

int main(int argc, char* argv[]){

  int src, dst;
  struct stat fs;
  ssize_t n;

  char buffer[BUFFER_SIZE];

  if (argc != 3 || !strcmp(argv[1], "--help")) {
    fprintf(stderr,"%s src dest\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  src = open(argv[1], O_RDONLY);

  if (src == -1){
    fprintf(stderr,"%s: %s, %s\n", argv[0], argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  if(stat(argv[1], &fs) == -1){
    fprintf(stderr,"%s: %s, %s\n", argv[0], argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  dst = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, fs.st_mode);

  if (src == -1){
    fprintf(stderr,"%s: %s, %s\n", argv[0], argv[2], strerror(errno));
    exit(EXIT_FAILURE);
  }

  while ((n = read(src, buffer, BUFFER_SIZE)) > 0)
    if (write(dst, buffer, n) != n){
      fprintf(stderr,"%s: %s\n", argv[0], strerror(errno));
      exit(EXIT_FAILURE);
    }

  if (n == -1){
    fprintf(stderr,"%s: %s, %s\n", argv[0], argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if (close(src) == -1) {
    fprintf(stderr,"%s: %s, %s\n", argv[0], argv[1], strerror(errno));
  }

  if (close(dst) == -1) {
    fprintf(stderr,"%s: %s, %s\n", argv[0], argv[2], strerror(errno));
  }

  exit(EXIT_SUCCESS);
}
