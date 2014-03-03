#include "options.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#define DEFAULT_PROBABILTY 0.3
#define OPTION_NOT_SET -1

static void
read_int_arg(const char *arg, int *result, const char **error) {
    long l = strtol(arg, NULL, 10);
    if (errno == ERANGE || l > INT_MAX) {
        *error = "too high";
        return;
    }
    if (l <= 0) {
        *error = "too low";
        return;
    }
    *result = l;
}

static void
read_double_arg(const char *arg, double *result, const char **error) {
    double d = strtod(arg, NULL);
    if (errno == ERANGE || d > 1) {
        *error = "too high";
        return;
    }
    if (d <= 0) {
        *error = "too low";
        return;
    }
    *result = d;
}

static void
print_help(const char *program_name) {
    printf(
        "Usage: %s [OPTIONS] -r ROWS -c COLUMNS\n"
        "   -c, --columns\n"
        "   -h, --help          print this help\n"
        "   -p, --probability   default %g\n"
        "   -r, --rows\n",
        program_name, DEFAULT_PROBABILTY 
    );
}

static struct option*
init_longopts() {
    static struct option longopts[] = {
        { "columns",     1, NULL, 'c' },
        { "help",        0, NULL, 'h' },
        { "probability", 1, NULL, 'p' },
        { "rows",        1, NULL, 'r' },
    };
    return longopts;
}

static enum options_return_value
validate_and_set_default_options(struct options_opts *opts) {
    if (opts->columns == OPTION_NOT_SET) {
        fprintf(stderr, "option columns is not set\n");
        return OPTIONS_ERROR;
    }
    else if (opts->rows == OPTION_NOT_SET) {
        fprintf(stderr, "option rows is not set\n");
        return OPTIONS_ERROR;
    }
    if (opts->probability == OPTION_NOT_SET)
        opts->probability = DEFAULT_PROBABILTY;

    return OPTIONS_OK;
}

void
options_init(struct options_opts *opts) {
    opts->columns = opts->rows = opts->probability = OPTION_NOT_SET;
}

#define HANDLE_ERROR(error, format, return_value) do { \
    if (error) { \
        fprintf(stderr, format, error); \
        return return_value; \
    } \
} while(0)

enum options_return_value
options_getopt(int argc, char **argv, struct options_opts *opts) {
    const char *shortopts = "c:hp:r:";
    struct option *longopts = init_longopts();

    const char *error = NULL;
    int option;
    while ((option = getopt_long(argc, argv, shortopts, longopts, NULL))
            != -1) {
        switch (option) {
            case 'c':
                read_int_arg(optarg, &(opts->columns), &error);
                HANDLE_ERROR(error, "option columns %s\n", OPTIONS_ERROR);
                break;
            case 'h':
                print_help(argv[0]);
                return OPTIONS_HELP;
            case 'p':
                read_double_arg(optarg, &(opts->probability), &error);
                HANDLE_ERROR(error, "option probability %s\n", OPTIONS_ERROR);
                break;
            case 'r':
                read_int_arg(optarg, &(opts->rows), &error);
                HANDLE_ERROR(error, "option rows %s\n", OPTIONS_ERROR);
                break;
            case '?':
                return OPTIONS_ERROR;
        }
    }
    
    return validate_and_set_default_options(opts);
}