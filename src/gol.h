#ifndef GOL_H
    #define GOL_H
#include "options.h"
#include <stdbool.h>
#ifdef HAVE_NCURSES
    #include <curses.h>
#endif

struct object {
    bool alive_this_round, alive_next_round;
};

struct gol {
    struct object **table;
    int rows, columns;
    wint_t alive_character, not_alive_character;
    #ifdef HAVE_NCURSES
        cchar_t ncurses_alive_character, ncurses_not_alive_character;
    #endif
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
