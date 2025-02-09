#include <sys.h>
#include <sys/mount.h>
#include <bits/errno.h>

int main(int argc, char *argv[])
{
  if (argc < 4) {
    return -EINVAL;
  }

  return mount(argv[3], argv[2], argv[1], 0, 0);
}
