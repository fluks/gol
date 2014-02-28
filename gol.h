#ifndef GOL_H
    #define GOL_H

struct object {
    int alive_this_round, alive_next_round;
};

struct gol {
    struct object **table;
    int rows, columns;
};

struct gol*
gol_init(int rows, int columns, double probability_alive);

void
gol_free(struct gol *g);

void
gol_run(struct gol *g);

#endif // GOL_H
