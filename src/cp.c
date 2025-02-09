#include <bits/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

	int src, dst, e;
	struct stat fs;
	ssize_t n;

	char buffer[1024];

	if (argc != 3) {
		return -EINVAL;
	}

	if ((src = open(argv[1], O_RDONLY)) < 0) {
		return src;
	}

	if ((e = fstatat(src, 0, &fs, AT_EMPTY_PATH)) < 0) {
		return e;
	}

	if ((dst = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, fs.st_mode)) < 0) {
		return dst;
	}

	while ((n = read(src, buffer, 1024)) > 0) {
		if (write(dst, buffer, n) != n) {
			return n;
		}
	}

	return n;
}
