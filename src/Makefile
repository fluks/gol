CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L -std=c11 -Wall -Werror -pedantic -O2
LDLIBS =
objects = main.o gol.o options.o
executable = ../gol

.PHONY: all

all: $(objects)
	$(CC) $(CFLAGS) $(LDLIBS) -o $(executable) $(objects)

ncurses: CFLAGS += -DHAVE_NCURSES
ncurses: LDLIBS += -lncurses
ncurses: objects += ncurses_ui.o
ncurses: ncurses_ui.o $(objects)
	$(CC) $(CFLAGS) $(LDLIBS) -o $(executable) $(objects)

ncurses_ui.o: ncurses_ui.c
	$(CC) $(CFLAGS) $(LDLIBS) -c -o $@ $<


%.o: %.c
	$(CC) $(CFLAGS) $(LDLIBS) -c -o $@ $<

clean:
	-rm *.o $(executable)
