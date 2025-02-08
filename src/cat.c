#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  int f, i;
  char buf[4096];
  long n;

  for (i = 1; i < argc; i++) {
    
    f = open(argv[i], O_RDONLY);

    if (f < 0) {
      return -1;
    } else {
      while((n = read(f, buf, sizeof(buf) )) > 0) {
        write(1, buf, n);
      }
    }
    
  }

  return 0;
}
