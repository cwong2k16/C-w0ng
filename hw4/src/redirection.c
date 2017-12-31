#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "builtin.h"
#include "debug.h"
#include "sfish.h"
#include "redirection.h"

char *flag = "";
char *myPwd;

bool handle_redirection(char *input){
    int leftCount = count(input, '<');
    int rightCount = count(input, '>');
    if((leftCount == 1 || rightCount == 1)){
        redirect(leftCount, rightCount, input);
    }
    else{
        printf(SYNTAX_ERROR, "invalid number of brackets");
        return false;
    }
    return false;
}

void redirect(int leftCount, int rightCount, char *input){
    /* Algorithm: Parse out the symbol */

    if((leftCount == 1 && rightCount == 0) || (leftCount == 0 && rightCount == 1)){
        /* Here I calloc tempInput to be same size as input string, and then copy over the contents from input onto tempInput*/
        char* tempInput = calloc(strlen(input)+1, sizeof(char));
        strcpy(tempInput, input);

        char *arr[2];
        if(rightCount){
            split(arr, tempInput, ">");
        }
        else{
            split(arr, tempInput, "<");
        }

        if(arr[0] == 0 || arr[1] == 0){ // this accounts for if format is like "<blahblah", "blahblah<", "<   blahblah", or "blahblah   <"
            printf(SYNTAX_ERROR, "Wrong formatting for brackets.");
            free(tempInput);
            return;
        }

        /* arr[0] is the element that contains possible program exec as well as the arguments, here I make a copy of this for arr1 */
        char* arr1 = calloc(strlen(arr[0])+1, sizeof(char));
        strcpy(arr1, arr[0]);

        /* file is the first strtok_r token received from calling strtok_r... there can only be one instance of this */
        char *file = strtok_r(arr[1], " ", &arr[1]);

        /* if file is null, then there is no file so throw a syntax error */
        if(file == 0){
            printf(SYNTAX_ERROR, "Wrong formatting/missing input for file.");
            free(arr1);
            free(tempInput);
            return;
        }

        /* extra accounts if there is more than one string inputted in the place for file*/
        char *extra = strtok_r(arr[1], " ", &arr[1]);
        if(extra != 0){
            printf(SYNTAX_ERROR, "Too many arguments");
            free(arr1);
            free(tempInput);
            return;
        }

        /* now here we must parse the prog exec along with its possible arguments */
        int progCount = getSize(arr[0], " ");
        char *args[progCount];
        tokenizer(arr1, args, " ");

        if(args == 0){
            printf(SYNTAX_ERROR, "Missing program exec/args!");
            free(arr1);
            free(tempInput);
            return;
        }

        /* now we can set all the file descriptors, fork, waitpid and all that stuff */
        int fd;
        if(rightCount){
           fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        }
        else{
           fd = open(file, O_RDONLY);
            if(fd == -1){
                printf(SYNTAX_ERROR, "Invalid file.");
                free(arr1);
                free(tempInput);
                return;
            }
        }

        int retVal = 0;

        if(rightCount){
            int saveStdin = dup(1);
            dup2(fd, STDOUT_FILENO);
            if(strcmp(flag, "help")==0){
                dprintf(STDOUT_FILENO, HELP);
            }
            else if(strcmp(flag, "pwd")==0){
                dprintf(STDOUT_FILENO, "%s", myPwd);
            }
            else{
                retVal = executor(args, progCount);
            }
            close(fd);

            dup2(saveStdin, 1);
        }
        else{
            int saveStdin = dup(0);
            dup2(fd, STDIN_FILENO);
            close(fd);
            if(strcmp(flag, "help") == 0){
                dprintf(STDIN_FILENO, HELP);
            }
            else if(strcmp(flag, "pwd")==0){
                dprintf(STDIN_FILENO, "%s", myPwd);
            }
            else{
                retVal = executor(args, progCount);
            }
            dup2(saveStdin, 0);
        }

        if(retVal == -1){
            printf("sfish: %s: command not found\n", args[0]);

        }
        else if(retVal == -2){
            printf("sfish exec error: %s\n", EXEC_ERROR);
        }
        free(arr1);
        free(tempInput);
    }

    else{
        char* tempInput = calloc(strlen(input)+1, sizeof(char));
        strcpy(tempInput, input);

        int fileOrder = determineOrder(tempInput); // if fileOrder is zero, then input first, aka "<" comes before the ">" in the input command

        char *argv[3];
        split2(argv, tempInput, fileOrder);
        if((argv[0]==0) || (argv[1]==0) || (argv[2]==0)){
            printf(SYNTAX_ERROR, "Wrong placement of brackets.");
            return;
        }
        char *in;
        char *out;
        char *extra;
        char *extra2;
        if(!fileOrder){
            in = strtok_r(argv[1], " ", &argv[1]);
            out = strtok_r(argv[2], " ", &argv[2]);
        }
        else{
            in = strtok_r(argv[2], " ", &argv[2]);
            out = strtok_r(argv[1], " ", &argv[1]);
        }

        if ((in == 0) || (out == 0)){
            printf(SYNTAX_ERROR, "No input/output files were provided.");
            return;
        }
        extra = strtok_r(argv[1], " ", &argv[1]);
        extra2 = strtok_r(argv[2], " ", &argv[2]);

        if((extra != 0) || (extra2 != 0)){
            printf(SYNTAX_ERROR, "Wrong placement of brackets.");
            return;
        }

        char* arr1 = calloc(strlen(argv[0])+1, sizeof(char));
        strcpy(arr1, argv[0]);

        /* Check for stuff like you did in the last conditional */
        int progCount = getSize(argv[0], " ");
        char *args[progCount];
        tokenizer(arr1, args, " ");
        if(args == 0){
            printf(SYNTAX_ERROR, "Missing program exec/args!");
            return;
        }

        /* now we can set all the file descriptors, fork, waitpid and all that stuff */
        int fdInput;
        int fdOutput;

        int saveStdin = dup(1);
        int saveStdout = dup(0);

        fdInput = open(in, O_RDONLY);
        if(fdInput == -1){
            printf(SYNTAX_ERROR, "Invalid file.");
            return;
        }
        fdOutput = open(out, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

        dup2(fdInput, STDIN_FILENO);
        dup2(fdOutput, STDOUT_FILENO);
        close(fdInput);
        close(fdOutput);
        int retVal = 0;
        if(strcmp(flag, "help") == 0){
            if(fileOrder == 1){
                dprintf(STDOUT_FILENO, HELP);
            }
            else{
                dprintf(STDIN_FILENO, HELP);
            }
        }
        else if(strcmp(flag, "pwd")==0){
            if(fileOrder == 1){
                dprintf(STDOUT_FILENO, "%s", myPwd);
            }
            else{
                dprintf(STDIN_FILENO, "%s", myPwd);
            }
        }
        else{
            retVal = executor(args, progCount);
        }
        dup2(saveStdin, 0);
        dup2(saveStdout, 1);
        if(retVal == -1){
            printf("sfish: %s: command not found\n", args[0]);

        }
        else if(retVal == -2){
            printf("sfish exec error: %s\n", EXEC_ERROR);
        }
        free(arr1);
        free(tempInput);
    }
}

int count(char *input, char symbol){
    int count = 0;
    for(int i = 0; i < strlen(input); i++){
        if(input[i] == symbol){
            count++;
        }
    }
    return count;
}

void split(char ** arr, char *input, char *symbol){
    char *pch = strtok_r(input, symbol, &input);
    arr[0] = pch;
    pch = strtok_r(input, symbol, &input);
    arr[1] = pch;
}

void split2(char ** arr, char *input, int fileOrder){
    char *symbol;
    char *otherSymbol;
    if(!fileOrder){
        symbol = "<";
        otherSymbol = ">";
    }
    else{
        symbol = ">";
        otherSymbol = "<";
    }
    char *pch = strtok_r(input, symbol, &input);
    arr[0] = pch;
    pch = strtok_r(input, otherSymbol, &input);
    arr[1] = pch;
    pch = strtok_r(input, otherSymbol, &input);
    arr[2] = pch;
}

int determineOrder(char *input){
    char* left = strstr(input, "<");
    char* right = strstr(input, ">");
    if(left < right){
        return 0;
    }
    else{
        return 1; // if fileOrder is 1, then opposite case
    }
}

void setFlag(char *str){
    flag = str;
}
void setPwd2(char *pwd1){
    myPwd = pwd1;
}