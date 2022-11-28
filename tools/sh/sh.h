#ifndef SH_H
#define SH_H

char* read_line();
char** parse_line(char*);
char* get_token(char*);
int execute(char**);

#endif
