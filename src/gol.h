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
    wint_t alive_character, not_alive_character;
};

typedef void (*callback)(struct gol*, void*, int, int);

struct gol*
gol_init(const struct options_opts *opts);

void
gol_free(struct gol *g);

void
gol_run(struct gol *g);

void
gol_foreach_object(struct gol *g, void *data, callback cb);

void
gol_sleep(long wait);

#endif // GOL_H
