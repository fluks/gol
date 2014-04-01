#include "options.h"
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#define DEFAULT_PROBABILTY 0.3
#define DEFAULT_ALIVE_CHARACTER L'o'
#define DEFAULT_NOT_ALIVE_CHARACTER L' '
#define OPTION_ROWS                1
#define OPTION_COLUMNS             2
#define OPTION_PROBABILITY         4
#define OPTION_ALIVE_CHARACTER     8
#define OPTION_NOT_ALIVE_CHARACTER 16
#define OPTION_FILE                32

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
    int columns_needed = wcwidth(*wc);
    if (columns_needed < 1)
        *error = "is invalid: less than one column wide";
    else if (columns_needed > 1)
        *error = "is invalid: more than one column wide";
}

static void
print_help(const char *program_name) {
    printf(
        "Usage: %s [OPTIONS] -r ROWS -c COLUMNS\n"
        "   -a, --alive-character       "
            "a character representing an alive object\n"
        "   -c, --columns\n"
        "   -f, --file                  read game starting position from file\n"
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
        { "file",                 1, NULL, 'f' },
        { "help",                 0, NULL, 'h' },
        { "not-alive-character",  1, NULL, 'n' },
        { "probability",          1, NULL, 'p' },
        { "rows",                 1, NULL, 'r' },
        { 0,                      0, 0,    0   }
    };
    return longopts;
}

static const char*
get_option_str(int flag) {
    if (flag & OPTION_ROWS)
        return "rows";
    if (flag & OPTION_COLUMNS)
        return "columns";
    if (flag & OPTION_PROBABILITY)
        return "probability";
    return "programming error: should not be reached!";
}

static enum options_return_value
validate_and_set_default_options(struct options_opts *opts) {
    if (!(opts->options_set & OPTION_ALIVE_CHARACTER))
        opts->alive_character = DEFAULT_ALIVE_CHARACTER;
    if (!(opts->options_set & OPTION_NOT_ALIVE_CHARACTER))
        opts->not_alive_character = DEFAULT_NOT_ALIVE_CHARACTER;

    if (opts->options_set & OPTION_FILE) {
        int flag = (opts->options_set & OPTION_ROWS)    |
                   (opts->options_set & OPTION_COLUMNS) |
                   (opts->options_set & OPTION_PROBABILITY);
        if (flag) {
            fprintf(stderr, "options file and %s are mutually exclusive\n",
                get_option_str(flag));
            return OPTIONS_ERROR;
        }
    }
    else {
        if (!(opts->options_set & OPTION_COLUMNS)) {
            fprintf(stderr, "option columns is not set\n");
            return OPTIONS_ERROR;
        }
        if (!(opts->options_set & OPTION_ROWS)) {
            fprintf(stderr, "option rows is not set\n");
            return OPTIONS_ERROR;
        }
        if (!(opts->options_set & OPTION_PROBABILITY))
            opts->probability = DEFAULT_PROBABILTY;
    }

    return OPTIONS_OK;
}

void
options_init(struct options_opts *opts) {
    memset(opts, 0, sizeof(*opts));
}

#define HANDLE_ERROR(error, format, return_value) do { \
    if (error) { \
        fprintf(stderr, format, error); \
        return return_value; \
    } \
} while(0)

enum options_return_value
options_getopt(int argc, char **argv, struct options_opts *opts) {
    const char *shortopts = "a:c:f:hn:p:r:";
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
                opts->options_set |= OPTION_ALIVE_CHARACTER;
                break;
            case 'c':
                read_int_arg(optarg, &(opts->columns), &error);
                HANDLE_ERROR(error, "option columns %s\n", OPTIONS_ERROR);
                opts->options_set |= OPTION_COLUMNS;
                break;
            case 'f':
                opts->file = optarg;
                opts->options_set |= OPTION_FILE;
                break;
            case 'h':
                print_help(argv[0]);
                return OPTIONS_HELP;
            case 'n':
                first_wide_char_in_str(optarg, &opts->not_alive_character,
                    &error);
                HANDLE_ERROR(error,
                    "option not-alive-character %s\n", OPTIONS_ERROR);
                opts->options_set |= OPTION_NOT_ALIVE_CHARACTER;
                break;
            case 'p':
                read_double_arg(optarg, &(opts->probability), &error);
                HANDLE_ERROR(error, "option probability %s\n", OPTIONS_ERROR);
                opts->options_set |= OPTION_PROBABILITY;
                break;
            case 'r':
                read_int_arg(optarg, &(opts->rows), &error);
                HANDLE_ERROR(error, "option rows %s\n", OPTIONS_ERROR);
                opts->options_set |= OPTION_ROWS;
                break;
            case '?':
                return OPTIONS_ERROR;
        }
    }
    
    return validate_and_set_default_options(opts);
}
