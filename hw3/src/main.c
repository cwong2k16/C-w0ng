#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    // void *x = sf_malloc(1);
    // sf_header* headerX = (sf_header*)((char*)x - 8);
    // // cr_assert(headerX->padded == 1, "Malloc(1) not padded correctly");
    // void *y = sf_realloc(x,16);
    // sf_header* headerY = (sf_header*)((char*)y - 8);
    // cr_assert(headerY->padded == 0, "Realloc(x,16) not padded correctly");
    // sf_free(y);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
