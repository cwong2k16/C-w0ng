#include "hw1.h"
#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */

int helper(char **arg1, int argPos, char *arg2){ // **arg1 is the  pointer to pointer of the command line arg, argPos is the position string array *arg2 is flag to compare
    int index = 0; // this is the starting point of the char indexes of the string
    while(*(*(arg1+argPos)+index) != '\0' || *(arg2+index) != '\0'){
        if(*(*(arg1+argPos)+index) != *(arg2+index)){
            return 0;
        }
        index++;
    }
    return 1;
}

int sizeOfAlphabet(int choose){
    const char *array;
    if(choose){
       array = polybius_alphabet;
    }
    else{
       array = fm_alphabet;
    }

    int count = 0;
    int index = 0;
    while(*(array+index) != '\0'){
        count++;
        index++;
    }
    return count;
}

int valid_key(char **str, int key_pos, int choose){
    char array;
    if(choose){ // if choose is 1, array = *fm_alphabet
        array = *polybius_alphabet;
    }
    else{   // if choose is 0, array = *polybius
        array = *fm_alphabet;
    }
    int size = sizeOfAlphabet(choose);
    char *str1 = *(str+key_pos); // this is our "key"
    // compare chars in str1 to the chars in fm_alphabet
    int boolValid = 0;
    int i = 0;
    //*(str1 + i) to get the char of key
    while(*(str1+i) != '\0'){
        int j = 0;
        while(j <= size){
            if(*(str1+i) == (array+j)){
                boolValid = 1;
                j = size;
            }
            else{
                j++;
            }
        }
        if(boolValid){      // letter of the key does appear in the fm_alphabet, valid char, so check if the letter repeats
            boolValid = 0;
            j = 0;
            int count = 0;
            int k = 0;
            while(*(str1+k) != '\0'){
                if(*(str1+i) == *(str1+k)){
                    count++;
                    k++;
                }
                else{
                    k++;
                }
            }
            if(count > 1){
                return 0; // the letter repeats, so return 0... invalid
            }
        }
        else{
            return 0; // letter of the key does not appear in the fm_alphabet... invalid char, return 0
        }
        i++;
    }
    return 1;
}

int myItoa(char *stri){
    /* LOGIC:
     * loop until index of array is "\0"
     * get curr index of string char, add it to num, times it by 10 for next num, repeat until hits "\0"
    */
    int num = 0;
    int index = 0;
    while(*(stri+index) != '\0'){
        num*=10;
        int num1 = *(stri+index);
        num1 = num1 - 48;
        num+= num1;
        index++;
    }
    return num;;
}

unsigned short validargs(int argc, char **argv) {
    unsigned short val = 0x0000;
    if(argc == 1){  // only program executable was provided but no flags (-h, -f, -p...), aka return 0
        return 0x0000;
    }
    else if (helper(argv, 1, "-h")){    // since argc != 1, that means there is at least 2 arguments... if the second argument, argv[1], is "-h", display USAGE  + return EXIT_SUCCESS
        return 0x8000;
    }
    else if(helper(argv, 1, "-p")){ // since argc != 1 and second arg is not "-h", let's check if it's "-p"... if it is, parse the rest of the arguments + begin polybius cipher if valid
        int k = 0; // this keeps track if "-k" is already used
        int r = 0; // this keeps track if "-r" is already used
        int c = 0; // this keeps track if "-c" is already used
        int row = 10;
        int col = 10;
        if(argc > 9){
            return 0x0000; // too many arguments, only 9 arguments in total for command line for -p flag
        }
        val = val | 0x00aa;
        if(argc == 2){ // if there are only 2 arguments, then there is only the program executable and the "-p" ... this is invalid so return 0x0000
            return 0x0000;
        }
        else{   // since argc is not only 2 arguments, there has to be at least one more, aka the required argument following "-p" which is either "-e" or "-d"
            if(helper(argv, 2, "-d") || helper(argv, 2, "-e")){
                if(helper(argv, 2, "-d")){
                    val = val | 0x2000;
                }
            }
            else{
                return 0x0000;
            }
            // now that we got the -p and -e|-d out of the way, let's check for optional arguments
            // optional arguments can come in any order, so checking for all 3 at the same time is essential
            // else{ // LOGIC loop from index 3 to argc, checking each index if any of the flags -r, -c, or -k
            for(int i = 3; i < argc; i++){
                if(helper(argv, i, "-k") || helper(argv, i, "-r") || helper(argv, i, "-c")){
                    if(helper(argv, i, "-k")){
                        if(k == 0){
                            k = 1;
                            if(valid_key(argv, i+1, 1)){
                                // printf("this key is %s", "valid");
                                i++;
                                key = *(argv+i);
                            }
                            else{
                                // printf("this key is %s", "invalid");
                                return 0x0000;
                            }
                        }
                        else{
                            return 0x0000; // since k is already used as one of the flags, return 0, invalid arg, can't repeat flag
                        }
                    }
                    else if (helper(argv, i, "-r")){
                        if(r == 0){
                            r = 1;
                            int num = myItoa(*(argv+i+1));
                            if(num < 9 || num > 15){
                                return 0x0000;
                            }
                            row = num;
                            num = num * 16;
                            val = val & 0xFF0F; // only make row 0... originally row is set to a, but we want to replace with flag num
                            val = val | num;
                            i++;
                        }
                        else{
                            return 0x0000; // if r != 0, it is == 1. r is the current flag, but it was already used. return 0;
                        }
                    }
                    else if (helper(argv, i, "-c")){
                        if(c == 0){
                            c = 1;
                            int num = myItoa(*(argv+i+1));
                            if(num < 9 || num > 15){
                                return 0x0000;
                            }
                            col = num;
                            val = val & 0xFFF0; // make col section 0, we want to replace it with num
                            val = val | num;
                            i++;
                        }
                        else{
                            return 0x0000; // if c != 0, it is == 1. c is the current flag, but it was already used. return 0;
                        }
                    }
                }
                else{ // the arg is none of k, r, or c, so invalid
                    return 0x0000;
                }
            }
            // }
        }
        int rowTimesCol = row * col;
        int polybiusSize = sizeOfAlphabet(1);
        if(rowTimesCol < polybiusSize){
            return 0x0000;
        }
    }
    else if(helper(argv, 1, "-f")){ // since argc != 1 and second arg is not "-p", let's check if it's "-f"... if it is, parse the rest of the arguments + begin fractionated morse cipher
        val = val + 0x4000; // second bit of the 2 byte is set to 1 since it's "-f"
        if(argc == 2){
            return 0x0000; // since its only "-f" and nothing after, return 0 since invalid args
        }
        else{ // it's not only -f, meaning there is at least one more argument after
            if(helper(argv, 2, "-d") || helper(argv, 2, "-e")){
                if(helper(argv, 2, "-d")){
                    val = val + 0x2000; // since -d is the value after -f, set 1 to the bit right after
                }
            }
            else{ // neither -e or -d, return 0 since invalid args, can only be -e or -d at this position
                return 0x0000;
            }
            // let's check for the optional argument now, just "-k"
            if(argc == 3){ // there is no optional argument, so just return the val
                return val;
            }

            // there is a "-f" flag provided, and there is a "-d"/"-e" flag provided... since length of string arg array is longer than 3, there has to be a 4th arg, can only be "-k"
            else if(helper(argv, 3, "-k")){
                // as stated in the homework documentation, if there is an optional argument, it will 100% be followed by an argument
                // since "-k" is in the proper position, and we know 100% it is followed by a key, let's check if the key is valid (no repeating letters, and characters only)
                if(valid_key(argv, 4, 0)){
                    // printf("this key is %s", "valid");
                    // store the valid key into the fm_key array
                    key = *(argv+4);
                }
                else{
                    // printf("this key is %s", "invalid");
                    return 0x0000;
                }
            }
            else{
                return 0x0000; // since there is a 3rd argument, that happens not to be "-k", return 0 because it would be invalid arg
            }
        }
    }
    else{ // this means something other than -h, -p, or -f was provided as argv[1], which will result in displaying USAGE + returning EXIT_FAILURE, aka return 0
        return 0x0000;
    }
    return val;
}