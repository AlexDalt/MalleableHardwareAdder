CC=gcc
CFLAGS=-O2 -Wall -std=c99

all: evolve
	$(CC) $(CFLAGS) evolve.c
