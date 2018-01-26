CC=gcc
CFLAGS=-O2 -Wall -std=c99

all:
	$(CC) $(CFLAGS) evolve.c simulator.c -o evolve
