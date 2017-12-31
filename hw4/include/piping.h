#ifndef PIPE_H
#define PIPE_H

bool handle_piping(char *input);
char *wrapperTok(char *string, char *symbol);
int fillSizes(int numTokens, int *sizes, char **arr);
void fillTokens(int numTokens, int *sizes, char **tempArr, char **allTokens);
void fillPositions(int numTokens, int *sizes, int *positions);
int validateArr(int numTokens, int *size);
int getMax(int numTokens, int *sizes);
int getNumCmds(int numTokens, int *sizes);
void loop_pipe(char ***cmd);
void handle_pipe(char *allTokens[], int numTokens, int *sizes, int *positions);
#endif