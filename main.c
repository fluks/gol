#include "gol.h"
#include "options.h"
#include <stdlib.h>
#include <time.h>

int
main(int argc, char **argv) {
    int rows, cols;
    double probability_alive;
    if (!options_getopt(argc, argv, &rows, &cols, &probability_alive))
        exit(EXIT_FAILURE);
    int exit_value = EXIT_SUCCESS;
    srand(time(NULL));

    struct gol *g = gol_init(rows, cols, probability_alive);
    if (!g) {
        exit_value = EXIT_FAILURE;
        goto end;
    }
    gol_run(g);

    end:
        gol_free(g);
        return exit_value;
}
