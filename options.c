#include "options.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static bool
read_int_arg(const char *arg, int *result) {
    long l = strtol(arg, NULL, 10);
    if (errno == ERANGE || l > INT_MAX || l <= 0)
        return false;
    *result = l;

    return true;
}

bool
options_getopt(int argc, char **argv, int *rows, int *cols, double *p) {
    if (argc < 3 || argc > 4)
        return false;
    if (!read_int_arg(argv[1], rows))
        return false;
    if (!read_int_arg(argv[2], cols))
        return false;
    if (argc == 4) {
        *p = strtod(argv[3], NULL);
        if (errno == ERANGE || *p <= 0 || *p >= 1)
            return false;
    }
    else
        *p = 0.3;

    return true;
}
