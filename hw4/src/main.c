#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <signal.h>

#include "sfish.h"
#include "debug.h"
#include "builtin.h"
#include "parser.h"
#include "redirection.h"


void sigint_handler(int sig){
    // do nothing
}

void sigtstp_handler(int sig){
    signal(SIGTSTP, sigtstp_handler);
}

int main(int argc, char *argv[], char* envp[]) {
    signal(SIGINT, sigint_handler);
    // signal(SIGTSTP, sigtstp_handler);
    char* input;
    bool exited = false;
    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {
        long size;
        char *buf;
        char *ptr = NULL; // current directory
        char *str1 = ""; // squiggly character

        size = pathconf(".", _PC_PATH_MAX);
        buf = malloc((size_t)size);
        if(buf != NULL){
            ptr = getcwd(buf, (size_t)size);
        }

        char *homeDir = getenv("HOME");
        int totalLength = 0;
        if(strstr(ptr, homeDir)){
            int offset = strlen(homeDir);
            ptr = ptr + offset;
            totalLength += strlen(ptr) + 1; // length of pointer after offset
            str1 = "~";
        }
        else{
            totalLength += strlen(ptr); // pointer length by itself without any offset or squiggly
        }

        char *otherHalf = " :: chriwong >> "; // this will always exist
        totalLength += strlen(otherHalf); // total length of prompt will be whatever before + length of otherHalf

        char prompt[totalLength];

        snprintf(prompt, sizeof prompt, "%s%s%s", str1, ptr, otherHalf);

        setPwd(prompt);

        // printf(" %s ", prompt);
        input = readline(prompt);

        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }

        /* MY IMPLEMENTATION */
        else{ //
            exited = parser(input);
        }
        /* END OF MY IMPLEMENTATION */

        // Readline mallocs the space for input. You must free it.
        rl_free(input);
        free(buf);
        if(exited){
            exit(EXIT_SUCCESS);
        }
    } while(1);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}