#include <unistd.h>

int main(int argc, char* argv[]){
  for (int i = 1; i < argc; i++) {

    int l = 0;
    while(argv[i][l] != 0) {
      l++;
    }

    write(1, argv[i], l);
    write(1, " ", 1);
  }

  write(1, "\n", 1);
  return 0;
}
