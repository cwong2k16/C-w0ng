#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "builtin.h"
#include "redirection.h"
#include "piping.h"

char *pwd1;

void setPwd(char* pwd2){
    pwd1 = pwd2;
}

char* checker(char *temp){
    return strtok_r(temp, " \n\t<>", &temp);
}

bool parser(char *input){

    /* Doesn't contain >, <, or |? Then there is no redirection needed. Just called the built-in command */
    if(!strstr(input, ">") && !strstr(input, "<") && !strstr(input, "|")){
        // This actually doesn't only handle built-ins. It also handles executables, if all built-in commands fail.
        return handle_builtin(input);
    }
    else if(!strstr(input, "|") && (strstr(input, "<") || strstr(input, ">"))){
        char* tempInput = calloc(strlen(input)+1, sizeof(char));
        strcpy(tempInput, input);
        char *help = checker(tempInput);
        // Call redirection handler
        if(strcmp(help, "help")==0){
            setFlag("help");
        }
        else if(strcmp(help, "pwd") ==0){
            setPwd2(pwd1);
            setFlag("pwd");
        }
        else{
            setFlag("");
        }

        free(tempInput);
        return handle_redirection(input);
    }
    else if(!strstr(input, "<") && !strstr(input, ">") && strstr(input, "|")){
        return handle_piping(input);
    }
    return true;
}