#include "gol.h"
#include "options.h"
#include <stdlib.h>
#include <locale.h>

int
main(int argc, char **argv) {
    setlocale(LC_CTYPE, "");

    struct options_opts opts;
    options_init(&opts);
    enum options_return_value retval = options_getopt(argc, argv, &opts);
    if (retval == OPTIONS_ERROR)
        exit(EXIT_FAILURE);
    else if (retval == OPTIONS_HELP)
        exit(EXIT_SUCCESS);

    int exit_value = EXIT_SUCCESS;

    struct gol *g = gol_init(&opts);
    if (!g) {
        exit_value = EXIT_FAILURE;
        goto end;
    }
    gol_run(g);

    end:
        gol_free(g);
        exit(exit_value);
}
