#include <stdlib.h>
#include "hw1.h"
#include "debug.h"
#include "polybius.h"
#include "fractionated.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;
    mode = validargs(argc, argv);
    debug("Mode: 0x%X", 0);

    if(mode == 0x0000){              // if mode returns "0" (0x0000), there is an error, thus we will have to print menu + EXIT_FAILURE
        USAGE(*argv, EXIT_FAILURE);
    }
    else if(mode & 0x8000) {        // if mode returns 1xxx xxxx xxxx xxxx, we print the menu + EXIT_SUCCESS
        USAGE(*argv, EXIT_SUCCESS);
    }
    else if(mode & 0x4000){         // if conditional is true, second most bit is 1, which indicates
        run_fractionated(mode);
        return EXIT_SUCCESS;
    }
    // at this point, can only be -p since we've already checked for invalid, -h, and -f flag.
    else{
        run_polybius(mode);
        return EXIT_SUCCESS;
    }
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */