CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L -std=c11 -Wall -Werror -pedantic -O2
objects = main.o gol.o options.o
executable = ../gol

.PHONY: all

all: $(objects)
	$(CC) $(CFLAGS) -o $(executable) $(objects)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm $(objects) $(executable)
