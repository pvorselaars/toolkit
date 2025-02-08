#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]){

  int src, dst;
  struct stat fs;
  ssize_t n;

  char buffer[1024];

  if (argc != 3) {
    return -1;
  }

  src = open(argv[1], O_RDONLY);

  if (src == -1){
    return -1;
  }

  if(fstatat(src, 0, &fs, AT_EMPTY_PATH) == -1){
    return -1;
  }

  dst = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, fs.st_mode);

  if (dst == -1){
    return -1;
  }

  while ((n = read(src, buffer, 1024)) > 0) {
    if (write(dst, buffer, n) != n){
      return -1;
    }
  }

  if (close(src) == -1 || close(dst) == -1) {
    return -1;
  }

  return 0;
}
