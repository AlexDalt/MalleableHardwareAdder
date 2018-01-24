CC=gcc
CFLAGS=-O2 -Wall -std=c99

all:
	$(CC) $(CFLAGS) evolve.c -o evolve
