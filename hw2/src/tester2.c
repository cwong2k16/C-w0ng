#include <stdlib.h>
#include <stdio.h>

int recurse(int a, int b){
    printf("%d\n",b);
    if((a == 0) | (b == 0)){
        return 0;
    }
    else{
        return a + recurse(a, --b);
    }
}

int main(int argc, char **argv){

    printf("Number of arguments: %d \n", argc);

    char* first = argv[1];
    char* second = argv[2];


    printf("%d\n", recurse(atoi(first), atoi(second)));
    return 0;
}