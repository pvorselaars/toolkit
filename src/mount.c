#include <stdio.h>
#include <sys/mount.h>

int main(int argc, char *argv[])
{
  if (argc < 4) {
    return -1;
  }

  return mount(argv[3], argv[2], argv[1], 0, 0);
}
