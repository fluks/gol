#include "ncurses_ui.h"
#include "gol.h"
#include <curses.h>
#include <stdbool.h>
#define KEY_QUIT 'q'
#define KEY_STOP 's'
#define KEY_SPEED_UP '+'
#define KEY_SPEED_DOWN '-'
#define WAIT_STEP 50000000L
#define WAIT_MAX 999999999L

static void
draw_object_cb(struct gol *g, void *data, int y, int x) {
    const cchar_t wc = g->table[y][x].alive_this_round ?
        g->ncurses_alive_character : g->ncurses_not_alive_character;
    add_wch(&wc);
    if (x == g->columns - 1)
        move(y + 1, 0);
}

static enum ncurses_return_value
wait_until_stop_or_quit() {
    nodelay(stdscr, false);
    flushinp();
    int key;
    do {
        if ((key = getch()) == KEY_QUIT)
            return NCURSES_QUIT;
    } while (key != KEY_STOP);
    nodelay(stdscr, true);

    return NCURSES_OK;
}

static void
wait_more(long *wait) {
    if (*wait + WAIT_STEP <= WAIT_MAX)
        *wait += WAIT_STEP;
}

static void
wait_less(long *wait) {
    if (*wait > 0)
        *wait -= WAIT_STEP;
}

static bool
validate_rows_and_columns(const struct gol *g) {
    if (LINES < g->rows) {
        ncurses_end();
        fprintf(stderr, "rows option is larger than screen\n");
        return false;
    }
    if (COLS < g->columns) {
        ncurses_end();
        fprintf(stderr, "columns option is larger than screen\n");
        return false;
    }
    return true;
}

bool
ncurses_init(struct gol *g) {
    initscr();
    if (!validate_rows_and_columns(g))
        return false;
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, true);
    clear();
    refresh();
    
    setcchar(&g->ncurses_alive_character,
        (wchar_t*) &g->alive_character, 0, 0, 0);
    setcchar(&g->ncurses_not_alive_character,
        (wchar_t*) &g->not_alive_character, 0, 0, 0);

    return true;
}

void
ncurses_draw(struct gol *g) {
    move(0, 0);
    refresh();
    gol_foreach_object(g, NULL, draw_object_cb);
    refresh();
}

enum ncurses_return_value
ncurses_handle_key(long *wait) {
    switch (getch()) {
        case KEY_QUIT:
            return NCURSES_QUIT;
        case KEY_SPEED_DOWN:
            wait_more(wait);
            break;
        case KEY_SPEED_UP:
            wait_less(wait);
            break;
        case KEY_STOP:
            return wait_until_stop_or_quit();
            break;
    }
    flushinp();

    return NCURSES_OK;
}

void
ncurses_end() {
    endwin();
}
