CC=gcc
CFLAGS=-O -Wall -std=c99

all:
	$(CC) $(CFLAGS) evolve.c simulator.c -o evolve
