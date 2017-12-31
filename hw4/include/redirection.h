#ifndef REDIR_H
#define REDIR_H

#define HELP "This is a help menu.\n\
Enter 'help' for this help menu.\n\
Enter 'exit' to exit this shell.\n\
Enter 'cd' change current working directory to the home directory.\n\
Enter 'cd -' change current working directory to the last directory you were in.\n\
Enter 'cd .' change ... nothing?\n\
Enter 'cd ..' change directory to current directory's predecessor's directory.\n"

extern char* pColor;
extern char* pColors[8];
extern int jobs[100];
extern char jobsComds[100];

char *getColor();
void setColor(char* color);
bool handle_redirection(char *input);
int count(char *input, char symbol);
void redirect(int leftCount, int rightCount, char *input);
int syntax_check(int leftCount, int rightCount, char **arr, int offset, int count, char **files);
void reHelp(int selector, int leftCount, int rightCount, char **arr, int offset, int count, char **files);
void split(char ** arr, char *input, char *symbol);
void split2(char ** arr, char *input, int fileOrder);
int determineOrder(char *input);
void setFlag(char *flag);
void setPwd2(char *pwd1);

#endif