# Aidan Bird 2021
CC=gcc
DEBUG_CFLAGS = -ggdb3 -O0 -Wall -fstrict-aliasing -std=c99
CFLAGS = -O2 -Wall -fstrict-aliasing -std=c99

debug:
	$(CC) $(DEBUG_CFLAGS) -o columnizer columnizer.c

all:
	$(CC) $(CFLAGS) -o columnizer columnizer.c

.PHONY: clean
clean:
	rm -f columnizer columnizer.o
