#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){

  char* path;
  DIR* dir;
  struct dirent** list;
  struct stat status;
  int error;

  if (argc > 1){
    path = argv[1];
  } else {
    path = getenv("PWD");
  }

  error = stat(path, &status);
  if (error) {
    fprintf(stderr, "%s: %s: %s\n", argv[0], path, strerror(errno));
    return 1;
  }

  if (S_ISDIR(status.st_mode)) {

    dir = opendir(path);
    if (dir) {

      int n = scandir(path, &list, NULL, alphasort);

      if (n == -1) {
        fprintf(stderr, "%s: %s: %s\n", argv[0], path, strerror(errno));
        return 2;
      }

      for (int i = 0; i < n; i++) {

        char* fullpath = malloc(sizeof(char*) * (strlen(path) + strlen(list[i]->d_name)));

        if (!fullpath) {
          fprintf(stderr, "%s: %s: %s\n", argv[0], path, strerror(errno));
          return 3;
        }

        sprintf(fullpath, "%s/%s", path, list[i]->d_name);

        error = stat(fullpath, &status);
        if (error) {
          fprintf(stderr, "%s: %s: %s\n", argv[0], fullpath, strerror(errno));
        }

        if (S_ISDIR(status.st_mode)) {
          if (*list[i]->d_name != '.') {
            printf("%s/\n", list[i]->d_name);
          }
        } else {
          printf("%s\n", list[i]->d_name);
        }

        free(fullpath);
        free(list[i]);

      }

      free(list);

      closedir(dir);

    } else {

      fprintf(stderr, "%s: %s: %s\n", argv[0], path, strerror(errno));

    }

  } else {
    printf("%s\n", path);
  }

  return 0;
}
