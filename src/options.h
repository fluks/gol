#ifndef OPTIONS_H
    #define OPTIONS_H

struct options_opts {
    int rows, columns;
    double probability;
};

enum options_return_value {
    OPTIONS_OK, OPTIONS_HELP, OPTIONS_ERROR
};

void
options_init(struct options_opts *opts);

enum options_return_value
options_getopt(int argc, char **argv, struct options_opts *opts); 

#endif // OPTIONS_H