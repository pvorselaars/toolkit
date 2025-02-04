#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  int f;
  char buf[4096];
  long n;

  for (int i = 1; i < argc; i++) {
    
    f = open(argv[i], O_RDONLY);

    if (f < 0) {
      perror(argv[0]);
    } else {
      while((n = read(f, buf, (long) sizeof(buf) )) > 0) {
        write(1, buf, n);
      }
    }
    
  }

  return 0;
}
