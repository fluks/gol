#include "gol.h"
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#define WAIT_NSECS 300000000L

typedef void (*callback)(struct gol*, int, int);

static void
foreach_object(struct gol *g, callback cb) {
    for (int y = 0; y < g->rows; y++) {
        for (int x = 0; x < g->columns; x++) {
            cb(g, y, x);
        }
    }
}

static int
get_number_of_alive_neighbors(const struct gol *g, int y, int x) {
    int alive_neighbors = 0;
    // Row above current object.
    if (y - 1 >= 0) {
        if (x - 1 >= 0)
            alive_neighbors += g->table[y - 1][x - 1].alive_this_round ? 1 : 0;
        alive_neighbors += g->table[y - 1][x].alive_this_round ? 1 : 0;
        if (x + 1 < g->columns)
            alive_neighbors += g->table[y - 1][x + 1].alive_this_round ? 1 : 0;
    }
    // Same row as current object.
    if (x - 1 >= 0)
        alive_neighbors += g->table[y][x - 1].alive_this_round ? 1 : 0;
    if (x + 1 < g->columns)
        alive_neighbors += g->table[y][x + 1].alive_this_round ? 1 : 0;
    // Row below current object.
    if (y + 1 < g->rows) {
        if (x - 1 >= 0)
            alive_neighbors += g->table[y + 1][x - 1].alive_this_round ? 1 : 0;
        alive_neighbors += g->table[y + 1][x].alive_this_round ? 1 : 0;
        if (x + 1 < g->columns)
            alive_neighbors += g->table[y + 1][x + 1].alive_this_round ? 1 : 0;
    }

    return alive_neighbors;
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
        return n == 3 ? true : false;
}

static void
set_alive_next_round_cb(struct gol *g, int y, int x) {
    g->table[y][x].alive_next_round = alive_next_round(g, y, x);
}

static void
set_alive_this_round_cb(struct gol *g, int y, int x) {
    g->table[y][x].alive_this_round = g->table[y][x].alive_next_round;
    g->table[y][x].alive_next_round = false;
}

static void
print_cb(struct gol *g, int y, int x) {
    printf("%c", g->table[y][x].alive_this_round ? 'o' : ' ');
    if (x == g->columns - 1)
        printf("\n");
}

static void
gsleep(long nanoseconds) {
    const struct timespec t = { .tv_sec = 0, .tv_nsec = nanoseconds };
    nanosleep(&t, NULL);
}

struct gol*
gol_init(int rows, int columns, double probability_alive) {
    struct gol *g = malloc(sizeof(*g));
    if (!g)
        return NULL;
    g->table = malloc(sizeof(*(g->table)) * rows);
    if (!g->table)
        return NULL;
    for (int y = 0; y < rows; y++) {
        g->table[y] = malloc(sizeof(**(g->table)) * columns);
        if (!g->table[y])
            return NULL;
        for (int x = 0; x < columns; x++) {
             g->table[y][x].alive_this_round =
                 ((double) rand() / RAND_MAX) <= probability_alive ? true : false;
             g->table[y][x].alive_next_round = false; 
        }
    }
    g->rows = rows;
    g->columns = columns;

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
    while (1) {
        system("clear");
        foreach_object(g, print_cb);
        foreach_object(g, set_alive_next_round_cb);
        foreach_object(g, set_alive_this_round_cb);
        gsleep(WAIT_NSECS);
    }
}
