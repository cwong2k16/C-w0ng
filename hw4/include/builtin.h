#ifndef BUILTIN_H
#define BUILTIN_H

bool handle_builtin(char *input);
int getSize(char* tempInput, char *delim);
void tokenizer(char* input, char **arr, char *delim);
void pipeline(int pipeCount, char **arr, int count);
int char_count(int size, char **arr, char *myChar);
int offsetToBracket(char **arr);
int executor(char **arr, int count);
void sigchld_handler(int s);

#endif