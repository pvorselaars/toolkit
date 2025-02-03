#include <stdio.h>
#include <sys/mount.h>

int main(int argc, char *argv[])
{
  if (argc < 4) {
    printf("usage: %s type src dst\n", argv[0]);
  }

  if (mount(argv[3], argv[2], argv[1], 0, 0) != 0) {
    perror("mount");
    return -1;
  } else {
    return 0;
  }

}
