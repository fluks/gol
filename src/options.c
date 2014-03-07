#include "options.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <math.h>
#define DEFAULT_PROBABILTY 0.3
#define OPTION_NOT_SET -1
#define DEFAULT_ALIVE_CHARACTER L'o'
#define DEFAULT_NOT_ALIVE_CHARACTER L' '

static void
read_int_arg(const char *arg, int *result, const char **error) {
    char *endptr;
    errno = 0;
    long l = strtol(arg, &endptr, 10);
    if (*arg == '\0' || *endptr != '\0') {
        *error = "is not a number";
        return;
    }
    if ((errno == ERANGE && l == LONG_MAX) || 
        l > INT_MAX) {
        *error = "too high";
        return;
    }
    if ((errno == ERANGE && l == LONG_MIN) ||
        l <= 0) {
        *error = "too low";
        return;
    }
    *result = l;
}

static void
read_double_arg(const char *arg, double *result, const char **error) {
    char *endptr;
    errno = 0;
    double d = strtod(arg, &endptr);
    if (*arg == '\0' || *endptr != '\0') {
        *error = "is not a number";
        return;
    }
    if ((errno == ERANGE && d == HUGE_VAL) || d >= 1) {
        *error = "too high";
        return;
    }
    if ((errno == ERANGE && d == -HUGE_VAL) || d <= 0) {
        *error = "too low";
        return;
    }
    *result = d;
}

static void
first_wide_char_in_str(const char *s, wint_t *wc, const char **error) {
    wchar_t wa[MB_LEN_MAX];
    size_t nbytes = mbrtowc(wa, s, MB_LEN_MAX, NULL);
    if (nbytes == (size_t) -2 || nbytes == (size_t) -1) {
        *error = "is invalid";
        return;
    }
    *wc = wa[0];
    nbytes = mbrtowc(wa, s + nbytes, MB_LEN_MAX, NULL);
    if (nbytes != 0)
        *error = "is invalid: more than one character";
}

static void
print_help(const char *program_name) {
    printf(
        "Usage: %s [OPTIONS] -r ROWS -c COLUMNS\n"
        "   -a, --alive-character       "
            "a character representing an alive object\n"
        "   -c, --columns\n"
        "   -h, --help                  print this help\n"
        "   -n, --not-alive-character   "
            "a character representing an object not alive\n"
        "   -p, --probability           default %g\n"
        "   -r, --rows\n"
        #ifdef HAVE_NCURSES
        "Keys:\n"
        "   s   stop\n"
        "   q   quit\n"
        "   +   speed up\n"
        "   -   speed down\n"
        #endif
        ,
        program_name, DEFAULT_PROBABILTY 
    );
}

static struct option*
init_longopts() {
    static struct option longopts[] = {
        { "alive-character",      1, NULL, 'a' },
        { "columns",              1, NULL, 'c' },
        { "help",                 0, NULL, 'h' },
        { "not-alive-character",  1, NULL, 'n' },
        { "probability",          1, NULL, 'p' },
        { "rows",                 1, NULL, 'r' },
        { 0,                      0, 0,    0   }
    };
    return longopts;
}

static enum options_return_value
validate_and_set_default_options(struct options_opts *opts) {
    if (opts->columns == OPTION_NOT_SET) {
        fprintf(stderr, "option columns is not set\n");
        return OPTIONS_ERROR;
    }
    if (opts->rows == OPTION_NOT_SET) {
        fprintf(stderr, "option rows is not set\n");
        return OPTIONS_ERROR;
    }
    if (opts->probability == OPTION_NOT_SET)
        opts->probability = DEFAULT_PROBABILTY;
    if (opts->alive_character == (wint_t) OPTION_NOT_SET)
        opts->alive_character = DEFAULT_ALIVE_CHARACTER;
    if (opts->not_alive_character == (wint_t) OPTION_NOT_SET)
        opts->not_alive_character = DEFAULT_NOT_ALIVE_CHARACTER;

    return OPTIONS_OK;
}

void
options_init(struct options_opts *opts) {
    opts->rows = OPTION_NOT_SET;
    opts->columns = OPTION_NOT_SET;
    opts->probability = OPTION_NOT_SET;
    opts->alive_character = OPTION_NOT_SET;
    opts->not_alive_character = OPTION_NOT_SET;
}

#define HANDLE_ERROR(error, format, return_value) do { \
    if (error) { \
        fprintf(stderr, format, error); \
        return return_value; \
    } \
} while(0)

enum options_return_value
options_getopt(int argc, char **argv, struct options_opts *opts) {
    const char *shortopts = "a:c:hn:p:r:";
    struct option *longopts = init_longopts();

    const char *error = NULL;
    int option;
    while ((option = getopt_long(argc, argv, shortopts, longopts, NULL))
            != -1) {
        switch (option) {
            case 'a':
                first_wide_char_in_str(optarg, &opts->alive_character, &error);
                HANDLE_ERROR(error, "option alive-character %s\n",
                    OPTIONS_ERROR);
                break;
            case 'c':
                read_int_arg(optarg, &(opts->columns), &error);
                HANDLE_ERROR(error, "option columns %s\n", OPTIONS_ERROR);
                break;
            case 'h':
                print_help(argv[0]);
                return OPTIONS_HELP;
            case 'n':
                first_wide_char_in_str(optarg, &opts->not_alive_character,
                    &error);
                HANDLE_ERROR(error,
                    "option not-alive-character %s\n", OPTIONS_ERROR);
                break;
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
