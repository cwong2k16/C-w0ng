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
#include "piping.h"

bool handle_piping(char *input){
    int numTokens = count(input, '|') + 1;
    char *tempArr[numTokens];
    char *arr[numTokens];
    int sizes[numTokens];
    int positions[numTokens];
    int size = 0;

    char* tempInput = calloc(strlen(input)+1, sizeof(char));
    strcpy(tempInput, input);

    char* tempInput2 = calloc(strlen(input)+1, sizeof(char));
    strcpy(tempInput2, input);

    tokenizer(tempInput, arr, "|");
    tokenizer(tempInput2, tempArr, "|");

    size = fillSizes(numTokens, sizes, arr);

    int valid = validateArr(numTokens, sizes);

    if(valid == -1){
        free(tempInput);
        free(tempInput2);
        printf(SYNTAX_ERROR, "Lacking enough arguments per pipes");
        return false;
    }

    char *allTokens[size];

    fillTokens(numTokens, sizes, tempArr, allTokens);

    fillPositions(numTokens, sizes, positions);

    handle_pipe(allTokens, numTokens, sizes, positions);

    free(tempInput);
    free(tempInput2);
    return false;
}

void handle_pipe(char *allTokens[], int numTokens, int *sizes, int *positions){
}

void fillTokens(int numTokens, int *sizes, char **tempArr, char **allTokens){
    int j = 0;
    int z = 0;
    while(j < numTokens){
        int k = 0;
        while(k < sizes[j]){
            allTokens[z] = strtok_r(tempArr[j], " \n\t", &tempArr[j]);
            k++;
            z++;
        }
        j++;
    }
}

int fillSizes(int numTokens, int *sizes, char **arr){
    int i = 0;
    int size = 0;
    while(i < numTokens){
        sizes[i] = getSize(arr[i], " \t\n");
        size+=sizes[i];
        i++;
    }
    return size;
}

void fillPositions(int numTokens, int *sizes, int *positions){
    int pos = 0;
    int index = 0;

    while(index < numTokens){
        positions[index] = pos;
        pos+= sizes[index];
        index++;
    }
}

int validateArr(int numTokens, int *size){
    int i = 0;
    while(i < numTokens){
        if(size[i] == 1){
            return -1;
        }
        i++;
    }
    return 0;
}
