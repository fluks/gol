#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#define WAIT_NSECS 300000000L

struct object {
    int alive_this_round, alive_next_round;
};

struct world {
    struct object **table;
    int rows, columns;
};

typedef void (*callback)(struct world*, int, int);

void world_foreach_object(struct world *w, callback cb) {
    for (int y = 0; y < w->rows; y++) {
        for (int x = 0; x < w->columns; x++) {
            cb(w, y, x);
        }
    }
}

int world_get_number_of_alive_neighbors(const struct world *w, int y, int x) {
    int alive_neighbors = 0;
    // Row above current object.
    if (y - 1 >= 0) {
        if (x - 1 >= 0)
            alive_neighbors += w->table[y - 1][x - 1].alive_this_round;
        alive_neighbors += w->table[y - 1][x].alive_this_round;
        if (x + 1 < w->columns)
            alive_neighbors += w->table[y - 1][x + 1].alive_this_round;
    }
    // Same row as current object.
    if (x - 1 >= 0)
        alive_neighbors += w->table[y][x - 1].alive_this_round;
    if (x + 1 < w->columns)
        alive_neighbors += w->table[y][x + 1].alive_this_round;
    // Row below current object.
    if (y + 1 < w->rows) {
        if (x - 1 >= 0)
            alive_neighbors += w->table[y + 1][x - 1].alive_this_round;
        alive_neighbors += w->table[y + 1][x].alive_this_round;
        if (x + 1 < w->columns)
            alive_neighbors += w->table[y + 1][x + 1].alive_this_round;
    }

    return alive_neighbors;
}

int world_alive_next_round(struct world *w, int y, int x) {
    int n = world_get_number_of_alive_neighbors(w, y, x);
    if (w->table[y][x].alive_this_round) {
        if (n < 2)
            return 0;
        else if (n == 2 || n == 3)
            return 1;
        else
            return 0;
    }
    else
        return n == 3 ? 1 : 0;
}

void world_set_alive_next_round_cb(struct world *w, int y, int x) {
    w->table[y][x].alive_next_round = world_alive_next_round(w, y, x);
}

void world_set_alive_this_round_cb(struct world *w, int y, int x) {
    w->table[y][x].alive_this_round = w->table[y][x].alive_next_round;
    w->table[y][x].alive_next_round = 0;
}

void world_print_cb(struct world *w, int y, int x) {
    printf("%c", w->table[y][x].alive_this_round ? 'o' : ' ');
    if (x == w->columns - 1)
        printf("\n");
}

struct world* world_init(int rows, int columns, double probability_alive) {
    struct world *w = malloc(sizeof(*w));
    if (!w)
        return NULL;
    w->table = malloc(sizeof(*(w->table)) * rows);
    if (!w->table)
        return NULL;
    for (int y = 0; y < rows; y++) {
        w->table[y] = malloc(sizeof(**(w->table)) * columns);
        if (!w->table[y])
            return NULL;
        for (int x = 0; x < columns; x++) {
             w->table[y][x].alive_this_round =
                 ((double) rand() / RAND_MAX) <= probability_alive ? 1 : 0;
             w->table[y][x].alive_next_round = 0; 
        }
    }
    w->rows = rows;
    w->columns = columns;

    return w;
}

void world_free(struct world *w) {
    if (!w)
        return;
    if (!w->table)
        goto free_w;
    for (int y = 0; y < w->rows; y++) {
        if (w->table[y])
            free(w->table[y]);
    }
    free(w->table);
   
    free_w:
        free(w);
}

int read_int_arg(const char *arg, int *result) {
    long l = strtol(arg, NULL, 10);
    if (errno == ERANGE || l > INT_MAX || l <= 0)
        return 0;
    *result = l;
    return 1;
}

int get_options(
    int argc,
    char **argv,
    int *rows,
    int *cols,
    double *probability_alive) {
    if (argc < 3 || argc > 4)
        return 0;
    if (!read_int_arg(argv[1], rows))
        return 0;
    if (!read_int_arg(argv[2], cols))
        return 0;
    if (argc == 4) {
        *probability_alive = strtod(argv[3], NULL);
        if (errno == ERANGE ||
            *probability_alive <= 0 ||
            *probability_alive >= 1)
            return 0;
    }
    else
        *probability_alive = 0.3;

    return 1;
}

void gol_sleep(long nanoseconds) {
    const struct timespec t = { .tv_sec = 0, .tv_nsec = nanoseconds };
    nanosleep(&t, NULL);
}

void world_run(struct world *w) {
    while (1) {
        system("clear");
        world_foreach_object(w, world_print_cb);
        world_foreach_object(w, world_set_alive_next_round_cb);
        world_foreach_object(w, world_set_alive_this_round_cb);
        gol_sleep(WAIT_NSECS);
    }
}

int main(int argc, char **argv) {
    int rows, cols;
    double probability_alive;
    if (!get_options(argc, argv, &rows, &cols, &probability_alive))
        return 1;
    int exit_value = 0;
    srand(time(NULL));

    struct world *w = world_init(rows, cols, probability_alive);
    if (!w) {
        exit_value = 1;
        goto end;
    }
    world_run(w);

    end:
        world_free(w);
        return exit_value;
}
