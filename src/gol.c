#include "gol.h"
#ifdef HAVE_NCURSES
    #include "ncurses_ui.h"
#endif
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#define WAIT_NSECS 300000000L

static int
get_number_of_alive_neighbors(const struct gol *g, int y, int x) {
    int n = 0;
    // Row above current object.
    if (y - 1 >= 0) {
        if (x - 1 >= 0)
            n += g->table[y - 1][x - 1].alive_this_round ? 1 : 0;
        n += g->table[y - 1][x].alive_this_round ? 1 : 0;
        if (x + 1 < g->columns)
            n += g->table[y - 1][x + 1].alive_this_round ? 1 : 0;
    }
    // Same row as current object.
    if (x - 1 >= 0)
        n += g->table[y][x - 1].alive_this_round ? 1 : 0;
    if (x + 1 < g->columns)
        n += g->table[y][x + 1].alive_this_round ? 1 : 0;
    // Row below current object.
    if (y + 1 < g->rows) {
        if (x - 1 >= 0)
            n += g->table[y + 1][x - 1].alive_this_round ? 1 : 0;
        n += g->table[y + 1][x].alive_this_round ? 1 : 0;
        if (x + 1 < g->columns)
            n += g->table[y + 1][x + 1].alive_this_round ? 1 : 0;
    }

    return n;
}

static bool
alive_next_round(struct gol *g, int y, int x) {
    int n = get_number_of_alive_neighbors(g, y, x);
    if (g->table[y][x].alive_this_round) {
        if (n < 2)
            return false;
        else if (n == 2 || n == 3)
            return true;
        else
            return false;
    }
    else
        return n == 3;
}

static void
set_alive_next_round_cb(struct gol *g, void *data, int y, int x) {
    g->table[y][x].alive_next_round = alive_next_round(g, y, x);
}

static void
set_alive_this_round_cb(struct gol *g, void *data, int y, int x) {
    int *objects_moved = data;
    if (g->table[y][x].alive_this_round != g->table[y][x].alive_next_round)
        (*objects_moved)++;

    g->table[y][x].alive_this_round = g->table[y][x].alive_next_round;
    g->table[y][x].alive_next_round = false;
}

#ifndef HAVE_NCURSES
static void
print_cb(struct gol *g, void *data, int y, int x) {
    printf("%c", g->table[y][x].alive_this_round ? 'o' : ' ');
    if (x == g->columns - 1)
        printf("\n");
}
#endif

static inline bool
is_object_alive_at_start(double probability) {
     return ((double) rand() / RAND_MAX) <= probability;
}

struct gol*
gol_init(const struct options_opts *opts) {
    struct gol *g = malloc(sizeof(*g));
    if (!g)
        return NULL;
    g->table = malloc(sizeof(*(g->table)) * opts->rows);
    if (!g->table)
        return NULL;
    srand(time(NULL));
    for (int y = 0; y < opts->rows; y++) {
        g->table[y] = malloc(sizeof(**(g->table)) * opts->columns);
        if (!g->table[y])
            return NULL;
        for (int x = 0; x < opts->columns; x++) {
             g->table[y][x].alive_this_round =
                 is_object_alive_at_start(opts->probability);
             g->table[y][x].alive_next_round = false; 
        }
    }
    g->rows = opts->rows;
    g->columns = opts->columns;

    return g;
}

void
gol_free(struct gol *g) {
    if (!g)
        return;
    if (!g->table)
        goto free_g;
    for (int y = 0; y < g->rows; y++) {
        if (g->table[y])
            free(g->table[y]);
    }
    free(g->table);
   
    free_g:
        free(g);
}

void
gol_run(struct gol *g) {
    int objects_moved = 0;
    long wait = WAIT_NSECS;
    #ifdef HAVE_NCURSES
        if (!ncurses_init(g))
            return;
    #endif
    while (true) {
        #ifdef HAVE_NCURSES
            ncurses_draw(g);
            if (ncurses_handle_key(&wait) == NCURSES_QUIT)
                break;
        #else
            system("clear");
            gol_foreach_object(g, NULL, print_cb);
        #endif
        gol_foreach_object(g, NULL, set_alive_next_round_cb);
        gol_foreach_object(g, &objects_moved, set_alive_this_round_cb);
        if (!objects_moved)
            break;
        objects_moved = 0;
        gol_sleep(wait);
    }
    #ifdef HAVE_NCURSES
        ncurses_end();
    #endif
}

void
gol_foreach_object(struct gol *g, void *data, callback cb) {
    for (int y = 0; y < g->rows; y++) {
        for (int x = 0; x < g->columns; x++) {
            cb(g, data, y, x);
        }
    }
}

void
gol_sleep(long wait) {
    const struct timespec t = { .tv_sec = 0, .tv_nsec = wait };
    nanosleep(&t, NULL);
}
