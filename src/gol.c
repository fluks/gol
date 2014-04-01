#include "gol.h"
#ifdef HAVE_NCURSES
    #include "ncurses_ui.h"
#endif
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
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
    printf("%lc", g->table[y][x].alive_this_round ? g->alive_character :
                                                    g->not_alive_character);
    if (x == g->columns - 1)
        printf("\n");
}
#endif

static inline bool
is_object_alive_at_start(double probability) {
     return ((double) rand() / RAND_MAX) <= probability;
}

static inline bool
file_is_stdin(const char *file) {
    return strncmp(file, "-", 1) == 0;
}

static FILE*
open_file(const char *file) {
    if (file_is_stdin(file))
        return stdin;
    else {
        errno = 0;
        FILE *fp = fopen(file, "r");
        if (!fp) {
            fprintf(stderr, "Can't open file: %s\n", strerror(errno));
            return NULL;
        }
        else
            return fp;
    }
}

static void
close_file(FILE *fp, const char *file) {
    if (!file_is_stdin(file))
        fclose(fp);
}

static bool
resize_wc_buf(wchar_t **buf, size_t size) {
    wchar_t *temp = realloc(*buf, size * sizeof(**buf));
    if (!temp)
        return false;
    *buf = temp;
    return true;
}

enum readline_error {
    READLINE_ERROR_MEMORY = -3, READLINE_ERROR_PARAM, READLINE_ERROR_ERRNO
};

static int
readline(FILE *fp, wchar_t **buf, size_t *size, int *error_errno) {
    if (*size == 0)
        return READLINE_ERROR_PARAM;
    if (!*buf) {
        if (!resize_wc_buf(buf, *size))
            return READLINE_ERROR_MEMORY;
    }
    wint_t wc;
    int n_chars = 0;
    errno = 0;
    while ((wc = fgetwc(fp)) != WEOF) {
        (*buf)[n_chars++] = wc;
        if (wc == L'\n') {
            (*buf)[n_chars - 1] = L'\0';
            return n_chars;
        }
        if (n_chars == *size) {
            *size *= 2;
            if (!resize_wc_buf(buf, *size))
                return READLINE_ERROR_MEMORY;
        }
    }
    if (errno != 0) {
        *error_errno = errno;
        return READLINE_ERROR_ERRNO;
    }
    return n_chars;
}

static bool
validate_and_copy_row(const wchar_t *row, int y, struct gol *g) {
    for (int x = 0; row[x] != L'\0'; x++) {
        if (row[x] == g->alive_character)
            g->table[y][x].alive_this_round = true;
        else if (row[x] == g->not_alive_character)
            g->table[y][x].alive_this_round = false;
        else
            return false;

        g->table[y][x].alive_next_round = false;
    }
    return true;
}

static bool
allocate_memory_for_rows(struct gol *g, int row, int columns) {
    struct object **temp_rows =
        realloc(g->table, sizeof(*(g->table)) * (row + 1));
    if (!temp_rows)
        return false;
    g->table = temp_rows;

    g->table[row] = malloc(sizeof(**(g->table)) * columns);
    if (!g->table[row])
        return false;
    return true;
}

#define FIRST_ROW -1

static bool
read_table(FILE *fp, struct gol *g) {
    wchar_t *buf = NULL;
    size_t size = 10;
    int error = 0;
    bool retval = true;
    int columns, row = 0, last_row_columns = FIRST_ROW;
    while ((columns = readline(fp, &buf, &size, &error)) > 0) {
        if (columns == 1) {
            fprintf(stderr, "empty row\n");
            retval = false;
            goto end;
        }
        if (!allocate_memory_for_rows(g, row, columns - 1)) {
            fprintf(stderr, "memory error\n");
            retval = false;
            goto end;
        }
        if (!validate_and_copy_row(buf, row, g)) {
            fprintf(stderr, "illegal character\n");
            retval = false;
            goto end;
        }

        if (last_row_columns != FIRST_ROW && last_row_columns != columns) {
            fprintf(stderr, "different number of columns\n");
            retval = false;
            goto end;
        }
        last_row_columns = columns;
        row++;
    }
    g->rows = row;
    g->columns = last_row_columns - 1;

    if (columns == READLINE_ERROR_MEMORY) {
        fprintf(stderr, "memory error\n");
        retval = false;
    }
    else if (columns == READLINE_ERROR_ERRNO) {
        fprintf(stderr, "fgetwc: %s\n", strerror(error));
        retval = false;
    }
    end:
        free(buf);
        return retval;
}

static bool
generate_table(struct gol *g, const struct options_opts *opts) {
    g->table = malloc(sizeof(*(g->table)) * opts->rows);
    if (!g->table)
        return false;

    srand(time(NULL));
 
    for (int y = 0; y < opts->rows; y++) {
        g->table[y] = malloc(sizeof(**(g->table)) * opts->columns);
        if (!g->table[y])
            return false;

        for (int x = 0; x < opts->columns; x++) {
             g->table[y][x].alive_this_round =
                 is_object_alive_at_start(opts->probability);
             g->table[y][x].alive_next_round = false; 
        }
    }
    return true;
}

struct gol*
gol_init(const struct options_opts *opts) {
    struct gol *g = malloc(sizeof(*g));
    if (!g)
        return NULL;
    memset(g, 0, sizeof(*g));

    g->alive_character = opts->alive_character;
    g->not_alive_character = opts->not_alive_character;

    if (opts->file) {
        errno = 0;
        FILE *fp = open_file(opts->file);
        if (!fp)
            return NULL;
        if (!read_table(fp, g)) {
            close_file(fp, opts->file);
            return NULL;
        }
        close_file(fp, opts->file);
    }
    else {
        if (!generate_table(g, opts))
            return NULL;
        g->rows = opts->rows;
        g->columns = opts->columns;
    }
    
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
