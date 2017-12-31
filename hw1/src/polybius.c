#include "polybius.h"
#include "const.h"
#include <stdio.h>
#include <stdlib.h>

// only thing we need to parse out of the mode is the row, col, and if its decryption/encryption

int getKeySize(){
    int index = 0;
    while(*(key+index) != '\0'){
        index++;
    }
    return index;;
}

void fillTable(int keyExists){
    int keySize = 0;
    if(keyExists){
        keySize = getKeySize();
    }
    int keySize2 = keySize;
    int currPos = 0;
    while(currPos < keySize){
        *(polybius_table+currPos) = *(key+currPos);
        currPos++;
    }
    int index = 0; // position of polybius alphabet
// char a = *(key + index);
// char b = *(polybius_alphabet + 0);
    while(*(polybius_alphabet + index) != '\0'){
        int inAlphabet = 0;
        int j = 0;
        while(j < keySize2){
            if(*(polybius_alphabet + index) == *(key + j)){
                inAlphabet = 1;
                j = keySize;
            }
            j++;
        }
        if(!inAlphabet){ // this means current key char is not in the polybius alphabet, so add to the newString
            *(polybius_table + keySize) = *(polybius_alphabet + index);
            keySize++;
        }
        index++;
    }

    int polyTableSize = sizeof(polybius_table);
    while(keySize < polyTableSize){             // fill in the empty cells with the '\0'
        *(polybius_table + keySize) = '\0';
        keySize++;
    }
}

void run_polybius(unsigned short mode){
    // // key is the KEY
    int keyExists = key != '\0'; // if there is a key, keyExists will be true... else keyExists will be 0 (false)
    int col = mode & 0x000F;
    int row = mode & 0x00F0;
    row = row/16;
    int cryption = mode & 0x2000; // if cryption is 8192, then -d (decryption) was used, else -e (encryption) was
    fillTable(keyExists);

    if(cryption){       // cryption is not 0, therefore it is 8192 and we will need to decrypt
        decrypt(col);
    }

    else{               // cryption returned 0, therefore  we have to call encrypt
        encrypt(col);
    }
}

void decrypt(int col){
    int myRow = -1;
    int myCol = -1;

    int getC = getchar();
    while(getC >= 0){
        if(getC == '\t' || getC == ' ' || getC == '\n'){ // easy to check these first, and put them in the stdout if true
            putchar(getC);
        }
        else if(myRow == -1){
            myRow = getC;
        }
        else if(myCol == -1){
            myCol = getC;
        }
        if((myRow != -1) && (myCol != -1)){
            if(myRow > 47 && myRow < 58){
                myRow = myRow - 48;
            }
            else{
                myRow = myRow - 55;
            }
            if(myCol > 47 && myCol < 58){
                myCol = myCol - 48;
            }
            else{
                myCol = myCol - 55;
            }
            // printf("%01X", myRow);
            // printf("%01X", myCol);
            printChar(myRow, myCol, col);
            myCol = -1;
            myRow = -1;
        }
        getC = getchar();
    }
}

void printChar(int myRow, int myCol, int col){
    // printf(" %d ", col);
    // printf(" %d ", myRow);
    // printf(" %d ", myCol);
    int oneDConv = col * myRow + myCol;
    // printf("%d", oneDConv);
    int index = 0;
    while(index < oneDConv){
        index++;
    }
    if(index == oneDConv){
        printf("%c", *(polybius_table+oneDConv));
    }
}

void encrypt(int col){
    int getC = getchar();   // starts it off, get first char of the user input
    while(getC >= 0){       // if char isnt ctrl + d, then continue loop
        if(getC == '\t' || getC == ' ' || getC == '\n'){ // easy to check these first, and put them in the stdout if true
            putchar(getC);
        }
        else{                           // else getC does not equal tab or space, so let's check if they're valid (a part of the table)
            int checkExist = checkExists(getC);
            if(checkExist){ // the char exists inside the table, so "putchar"
                writeAddress(checkExist-1, col);
            }
            else{                  // the char does not exist in the table and it is not an acceptable non-white space, so return FAILURE
                // exit(EXIT_FAILURE);
                exit(EXIT_FAILURE);
            }
        }
        getC = getchar(); // read next char and continue checking
    }
}

int checkExists(char c){
    int i = 0;
    int size = sizeof(polybius_table);
    int boolExists = 0;
    while(i < size && !boolExists){
        if(c == *(polybius_table + i)){
            boolExists = 1;
        }
        else{
            i++;
        }
    }
    if(boolExists){
        return i+1;
    }
    return 0;
}

void writeAddress(int checkExist, int col){
    fprintf(stdout, "%01X", checkExist/col);
    fprintf(stdout, "%01X", checkExist%col);
}