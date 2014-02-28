#include "gol.h"
#include "options.h"
#include <stdlib.h>
#include <time.h>

int
main(int argc, char **argv) {
    int rows, cols;
    double probability_alive;
    if (!options_getopt(argc, argv, &rows, &cols, &probability_alive))
        return 1;
    int exit_value = 0;
    srand(time(NULL));

    struct gol *g = gol_init(rows, cols, probability_alive);
    if (!g) {
        exit_value = 1;
        goto end;
    }
    gol_run(g);

    end:
        gol_free(g);
        return exit_value;
}
