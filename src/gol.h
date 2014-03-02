#ifndef GOL_H
    #define GOL_H
#include "options.h"
#include <stdbool.h>

struct object {
    bool alive_this_round, alive_next_round;
};

struct gol {
    struct object **table;
    int rows, columns;
};

struct gol*
gol_init(const struct options_opts *opts);

void
gol_free(struct gol *g);

void
gol_run(struct gol *g);

#endif // GOL_H
