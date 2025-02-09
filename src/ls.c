#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

struct linux_dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};

int getdents(unsigned int fd, char buffer[], unsigned count);

int main(int argc, char *argv[], char *envp[])
{
    int fd, n, e;
    struct stat f;
    struct linux_dirent *de;
    char buffer[512];
    char *t = argc > 1 ? argv[1] : ".";

    e = fstatat(AT_FDCWD, t, &f, 0);

    if (e < 0) {
        return e;
    }

    if (S_ISDIR(f.st_mode))
    {

        fd = open(t, O_RDONLY | O_DIRECTORY);

        if (fd < 0) {
            return fd;
        }

        while ((n = getdents(fd, buffer, sizeof(buffer))) > 0) {

            for(int p = 0; p < n;) {
                de = (struct linux_dirent *) (buffer + p);

                int l = 0;
                while(de->d_name[l++] != 0);

                write(1, de->d_name, l);
                write(1, " ", 1);

                p += de->d_reclen;
            }
            
        }
        write(1, "\n", 1);
    }

    return 0;
}
