# getopt_long()
CFLAGS = -D_GNU_SOURCE
# nanosleep()
CFLAGS += -D_POSIX_C_SOURCE=199309L
# wcwidth()
CFLAGS += -D_XOPEN_SOURCE
CFLAGS += -std=c11 -Wall -Werror -pedantic -O2
LDLIBS = -lm
objects = main.o gol.o options.o
executable = ../gol

.PHONY: all

all: $(objects)
	$(CC) $(CFLAGS) $(LDLIBS) -o $(executable) $(objects)

ncurses: CFLAGS += -DHAVE_NCURSES $(shell pkg-config --cflags ncursesw)
ncurses: LDLIBS += $(shell pkg-config --libs ncursesw)
ncurses: objects += ncurses_ui.o
ncurses: ncurses_ui.o all

%.o: %.c
	$(CC) $(CFLAGS) $(LDLIBS) -c -o $@ $<

clean:
	-rm *.o $(executable)
