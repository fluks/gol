#ifndef NCURSES_UI_H
    #define NCURSES_UI_H
#include "gol.h"

enum ncurses_return_value {
    NCURSES_OK, NCURSES_ERR, NCURSES_QUIT
};

bool
ncurses_init(struct gol *g);

void
ncurses_draw(struct gol *g);

enum ncurses_return_value
ncurses_handle_key(long *wait);

void
ncurses_end();

#endif // NCURSES_UI_H
