#ifndef OPTIONS_H
    #define OPTIONS_H
#include <stdbool.h>

bool
options_getopt(int argc, char **argv, int *rows, int *columns, double *p); 

#endif // OPTIONS_H
