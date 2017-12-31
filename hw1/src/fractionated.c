#include "fractionated.h"
#include "const.h"
#include <stdio.h>
#include <stdlib.h>

void run_fractionated(unsigned short mode){
    int keyExists = key != '\0';
    fillTable2(keyExists);      // table has been filled properly
    int cryption = mode & 0x2000; // if cryption is 8192, then -d (decryption) was used, else -e (encryption) was

    if(cryption){
        decrypt2();
    }
    else{
        encrypt2();
    }
}

void fillTable2(int keyExists){
    int size = sizeof(fm_key);
    int i = 0;
    if(keyExists){
        while(*(key+i) != '\0'){
            *(fm_key+i) = *(key+i);
            i++;
        }
    }

    int j = 0;
    while(*(fm_alphabet+j) != '\0'){
        char currChar = *(fm_alphabet+j);
        int alreadyHas = 0;
        int k = 0;
        while(*(fm_key + k) != '\0'){
            if(*(fm_key+k) == currChar){
                alreadyHas = 1;
                k = size;
            }
            else{
                k++;
            }
        }
        if(!alreadyHas){
            *(fm_key+i) = currChar;
            i++;
        }
        j++;
    }
}

void encrypt2(){
    char *holder = ""; // this simply keeps track if new line has nothing in it yet... useful for whitespace conditional
    char firstChar = fgetc(stdin);
    int whitespace = 0;
    int currIndex = 0; // currIndex is for putting inside the polybius, using it as a buffer
    while(firstChar >= 0){
        // printf(" %c ", firstChar);
        int firstAsciiNum = firstChar - 33;
        if(firstChar == ' ' || firstChar == '\t'){
            if(!whitespace){
                whitespace = 1;
                if(*(holder) != '\0'){ // if holder has something in it, that means something came before it, and it wasnt a whitespace
                    *(polybius_table+currIndex) = 'x'; // since holder is not empty, and there wasn't a whitespace before, add an 'x'
                    currIndex++;
                    if(getCurrSize() == 3){
                        printCorres();
                        currIndex = 0;
                        clearBuffer();
                    }
                }
            }
        }
        else if(firstChar == '\n'){
            if(!whitespace){
                whitespace = 1;
                if(getCurrSize()){
                    *(polybius_table+currIndex) = 'x';
                    currIndex++;
                    if(getCurrSize() == 3){
                        printCorres();
                        currIndex = 0;
                        clearBuffer();
                    }
                }
            }
            currIndex = 0;
            clearBuffer();
            putchar('\n');
            holder = "";
        }
        else if(*(*(morse_table + firstAsciiNum)) == '\0'){
            exit(EXIT_FAILURE);
        }
        else{
            if(*(morse_table+firstAsciiNum)){
                whitespace = 0;
                int index = 0;
                if(*(holder) == '\0'){
                    holder = "*";
                }
                while(*(*(morse_table+firstAsciiNum)+index) != '\0'){
                    *(polybius_table+currIndex) = *(*(morse_table+firstAsciiNum)+index);
                    index++;
                    currIndex++;
                    if(getCurrSize() == 3){
                        printCorres();
                        currIndex = 0;
                        clearBuffer();
                    }
                }
                *(polybius_table+currIndex) = 'x';
                currIndex++;
                if(getCurrSize() == 3){
                    printCorres();
                    currIndex = 0;
                    clearBuffer();
                }
            }
        }
        firstChar = fgetc(stdin);
    }
    *(polybius_table+currIndex) = 'x';
    if(getCurrSize()==3){
       printCorres();
    }
}

int getCurrSize(){  // function works as intended
    int i = 0;
    while(*(polybius_table+i) != '\0'){
        i++;
    }
    return i;
}

int getCurrSize2(const char *str){  // function works as intended
    int i = 0;
    while(*(str+i) != '\0'){
        i++;
    }
    return i;
}

void clearBuffer(){ // function worked as intended
    int destination = getCurrSize();
    int i = 0;
    while(i < destination){
        *(polybius_table+i) = '\0';
        i++;
    }
}

void printCorres(){
    int i = 0;
    // int index = 0;
    while(i < getFTSize()){
        int com = compare(*(fractionated_table+i));
        if(com == 3){
            putchar(*(fm_key+i));
            i = getFTSize();
        }
        else{
           i++;
        }
    }
}

int compare(const char *str){
    int i = 0;
    int same = 0;
    while(i < 3){
        if(*(str+i) != *(polybius_table+i)){
            i++;
        }
        else{
            same++;
            i++;
        }
    }
    return same;
}

int getFTSize(){
    int i = 0;
    while(*(fractionated_table+i) != '\0'){
        i++;
    }
    return i;
}

/*
*
*   EVERYTHING BELOW THIS COMMENT SECTION HAS TO DO WITH DECRYPTION
*
*/

void decrypt2(){
    char firstChar = fgetc(stdin);
    int polybius_index = 0; // keeps track of polybius index and resets when find an x
    int xTracker = 0; // if 0, last character read was not an x, if 1, last character track was an x
    char charTracker = '\0';
    while(firstChar >= 0){
        if(firstChar == '\n'){
            if(*(polybius_table) != '\0'){
                int i = getCurrSize2(polybius_alphabet); // this will help us loop through *morse_table
                int currIndex = 0;
                int pos = 0;
                while(currIndex < i){
                    int comparison = compar(currIndex);
                    if(comparison){
                        pos = currIndex;
                        currIndex = i;
                    }
                    else{
                       currIndex++;
                    }
                }
                charTracker = *(polybius_alphabet+pos);
                printf("%c", *(polybius_alphabet+pos));
                polybius_index = 0;
                xTracker++;
                clearBuffer();
            }
                printf("%c", '\n');
                charTracker = '\n';
            // firstChar = fgetc(stdin);
        }
        else{
            int location = locate(firstChar);
            const char *str = *(fractionated_table+location);
            int index = 0;
            while(index < 3){
                // printf(" %d ", index);
                if(*(str+index) != 'x'){
                    if(xTracker == 2 && charTracker != '\n'){
                        printf(" ");
                    }
                    *(polybius_table+polybius_index) = *(str+index);
                    charTracker = *(str+index);
                    xTracker = 0;
                    polybius_index++;
                }
                else if(*(str+index) == 'x'){
                    charTracker = 'x';
                    // printf("%c", *(str+index));
                    if(xTracker == 0){
                        int i = getCurrSize2(polybius_alphabet); // this will help us loop through *morse_table
                        int currIndex = 0;
                        int pos = 0;
                        while(currIndex < i){
                            int comparison = compar(currIndex);
                            if(comparison){
                                pos = currIndex;
                                currIndex = i;
                            }
                            else{
                               currIndex++;
                            }
                        }
                        charTracker = *(polybius_alphabet+pos);
                        printf("%c", *(polybius_alphabet+pos));
                        polybius_index = 0;
                        xTracker++;
                        clearBuffer();
                    }
                    else{ // this means xTracker is 1, which indicates a word seperator and a space must be printed
                        xTracker++;
                    }
                }
                index++;
            }

        }
        firstChar = fgetc(stdin);
        if(firstChar < 0 && xTracker == 0){
            int i = getCurrSize2(polybius_alphabet); // this will help us loop through *morse_table
            int currIndex = 0;
            int pos = 0;
            while(currIndex < i){
                int comparison = compar(currIndex);
                if(comparison){
                    pos = currIndex;
                    currIndex = i;
                }
                else{
                   currIndex++;
                }
            }
            charTracker = *(polybius_alphabet+pos);
            printf("%c", *(polybius_alphabet+pos));
            xTracker = 2;
        }
    }
}


int locate(char firstChar){
    int a = sizeof(fm_key);
    int location = -1;
    int index = 0;
    while(index < a){
        if(*(fm_key+index) == firstChar){
            location = index;
            index = a;
        }
        else{
            index++;
        }
    }
    return location;
}

int compar(int argPos){ // argPos is the position string array is flag to compare
    int index = 0; // this is the starting point of the char indexes of the string
    while(*(*(morse_table+argPos)+index) != '\0' || *(polybius_table+index) != '\0'){
        if(*(*(morse_table+argPos)+index) != *(polybius_table+index)){
            return 0;
        }
        index++;
    }
    return 1;
}