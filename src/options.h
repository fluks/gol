#ifndef OPTIONS_H
    #define OPTIONS_H
#include <wchar.h>

struct options_opts {
    int rows, columns;
    double probability;
    wint_t alive_character, not_alive_character;
    char *file;
    int options_set;
};

enum options_return_value {
    OPTIONS_OK, OPTIONS_HELP, OPTIONS_ERROR
};

void
options_init(struct options_opts *opts);

enum options_return_value
options_getopt(int argc, char **argv, struct options_opts *opts); 

#endif // OPTIONS_H
