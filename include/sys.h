#ifndef SYS_H
#define SYS_H

int execveat(int dirfd, const char *pathname, char *argv[], char *envp[], int flags);
void exit(int);

#endif
