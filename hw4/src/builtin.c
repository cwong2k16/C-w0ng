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


char* pColors[8] = {"RED", "GRN", "YEL", "BLU", "MAG", "CYN", "WHT", "BWN"};
char* pColor = NULL;

char* getColor(){
    return pColor;
}

void setColor(char* color){
    pColor = color;
}

/* this will only be called if there are no <, >, or | found inside input string */
bool handle_builtin(char *input){

    /* Here I calloc tempInput to be same size as input string, and then copy over the contents from input onto tempInput*/
    char* tempInput = calloc(strlen(input)+1, sizeof(char));
    strcpy(tempInput, input);

    char* tempInput2 = calloc(strlen(input)+1, sizeof(char));
    strcpy(tempInput2, input);

    /* This is for the current directory ... ptr is a pointer to the current directory */
    long size;
    char *buf;
    char *ptr; // current directory

    size = pathconf(".", _PC_PATH_MAX);
    buf = malloc((size_t)size);
    if(buf != NULL){
        ptr = getcwd(buf, (size_t)size);
    }

    /* Here I make an array filled with string tokens extracted from the input */
    int count = getSize(tempInput, " \t\n");
    char *arr [count];
    tokenizer(tempInput2, arr, " \t\n");

    /* Here are the checks */
    bool help = strcmp(arr[0], "help") == 0;
    bool pwd = strcmp(arr[0], "pwd") == 0;
    bool exited = strcmp(arr[0], "exit") == 0;
    bool cd = strcmp(arr[0], "cd") == 0;
    bool job = strcmp(arr[0], "jobs")==0;
    bool fg = strcmp(arr[0], "fg")==0;
    bool kill = strcmp(arr[0], "kill")==0;
    bool color = strcmp(arr[0], "color")==0;

    if(help){
        printf(HELP);
    }
    else if (pwd){
        // don't remove this print statement. necessary to the homework.
        printf("%s\n", ptr);
    }
    else if(exited){
        free(buf);
        free(tempInput);
        free(tempInput2);
        return exited;
    }
    else if(job){
        // jobs[0] = 1;
    }
    else if(fg){
        if(count <3){
            printf(SYNTAX_ERROR, "1 more argument required!");
        }
    }
    else if(color){
        if(count < 3){
            printf(SYNTAX_ERROR, "1 more argument required");
        }
        else{
            char* color = arr[1];
            int i = 0;
            while(i < 8){
                if(strcmp(pColors[i], color)==0){
                    pColor = pColors[i];
                }
                i++;
            }
        }
    }
    else if (kill){
        if(count < 3){
            printf(SYNTAX_ERROR, "1 more argument required");
        }
    }
    /* This is the fun part: cd and all of its decision making */
    else if(cd){
        if(strcmp(arr[1], "\0") == 0){
            setenv("OLDPWD", ptr, 1);
            chdir(getenv("HOME"));
            ptr = getcwd(buf, (size_t)size);
        }
        else if(strcmp(arr[1], "-") == 0){
            chdir(getenv("OLDPWD"));
            setenv("OLDPWD", ptr, 1);
            ptr = getcwd(buf, (size_t)size);
        }
        else if(strcmp(arr[1], ".") == 0){
            setenv("OLDPWD", ptr, 1);
            chdir(".");
            ptr = getcwd(buf, (size_t)size);
        }
        else if(strcmp(arr[1], "..") == 0){
            setenv("OLDPWD", ptr, 1);
            chdir("..");
            ptr = getcwd(buf, (size_t)size);
        }
        else{
            // printf("Invalid cd command. \n");
            struct stat x;
            if(stat(arr[1], &x) == 0 && S_ISDIR(x.st_mode)){
                setenv("OLDPWD", ptr, 1);
                chdir(arr[1]);
                ptr = getcwd(buf, (size_t)size);
            }

        }
    }
    else{
        int retVal = executor(arr, count);
        if(retVal == -1){
            printf(EXEC_ERROR, arr[0]);
        }
        else if(retVal == -2){
            printf(EXEC_NOT_FOUND, arr[0]);
        }
    }
    free(buf);
    free(tempInput);
    free(tempInput2);
    return exited;
}

int getSize(char *tempInput, char *delim){
    int count = 0;
    char *pch = strtok_r(tempInput, delim, &tempInput);
    while(pch){
        count++;
        pch = strtok_r(tempInput, delim, &tempInput);
    }
    return count+1;
}

void tokenizer(char *input, char **arr, char *delim){
    int i = 0;
    char *pch = strtok_r(input, delim, &input);
    while(pch){
        arr[i] = pch;
        pch = strtok_r(input, delim, &input);
        i++;
    }
    arr[i] = "";
}

int char_count(int size, char **arr, char *myChar){
    int count = 0;
    for(int i = 0; i < size-1; i++){
        if(strcmp(arr[i], myChar) == 0){
            count++;
        }
    }
    return count;
}


int offsetToBracket(char **arr){
    int offsetAmt = 0;
    while((strcmp(arr[offsetAmt],"<") != 0) &&  (strcmp(arr[offsetAmt], ">") != 0)){
        offsetAmt++;
    }
    return offsetAmt;
}

volatile sig_atomic_t pid;

void sigchld_handler(int s){
    int olderrno = errno;
    pid = waitpid(-1, NULL, 0);
    errno = olderrno;
}

int executor(char **arr, int count){
    sigset_t mask, prev;
    signal(SIGCHLD, sigchld_handler);
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    arr[count-1] = (char*)NULL;
    pid = fork();
    if(!pid){
        execvp(arr[0], arr);
        return -1;
    }
    else if(pid < 0){
        return -2;
    }
    else{
        sigsuspend(&prev);
    }
    return 0;
}