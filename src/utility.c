#include "include/utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Command line parser
 */



char is_arg(char *pa, const char *pkey) {
    return strncmp(pa, pkey, strlen(pkey)) == 0;
}

arg parse_arg(char *parg, const char *pkey) {
    arg a;

    a.key = pkey; //first_part

    // if it is arg then the size must fit
    if (is_arg(parg, pkey)) {
        a.value = parg+strlen(pkey);
    } else {
        a.value = NULL;
        a.key = NULL;
    }

    return a;
}
