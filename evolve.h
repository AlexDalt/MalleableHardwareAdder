#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "simulator.h"

#define TEST_SIZE 30
#define TEST_LOOP 15000
#define POP_SIZE 500
#define MUTATION 2.7f
#define SIZE_WEIGHT 0
#define DIVERSITY_WEIGHT -1
#define ELITISM 1
#define FITNESS_WEIGHT 10
#define COEVOLVE 0
#define STICKY 0
#define LOG 0
#define PROB_SKEW 1.0f //between 0 or 1, 1 is linear
#define VIRULENCE 1.0f
#define PARASITE_SIZE 32
#define CROSSOVER 0.7f
#define TOURNAMENT_SIZE POP_SIZE

int add_weight, sub_weight;

typedef struct Individual {
	unsigned char values[ STRING_LENGTH_BYTES ];
	int eval[ 3 ];
	FPGA fpga;
} Individual;

typedef struct Parasite {
	unsigned char values[ PARASITE_SIZE ];
	float score;
} Parasite;

void evaluate( Individual *ind, Parasite *para, Individual *pop );
