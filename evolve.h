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
#define WEIGHT_TEST 1500
#define FAULT_INJECTION 0
#define POP_SIZE 50
#define MUTATION 3.0f
#define SIZE_WEIGHT 0
#define DIVERSITY_WEIGHT 2
#define ELITISM 1
#define FITNESS_WEIGHT 10
#define COEVOLVE 0
#define STICKY 0
#define LOG 0
#define PROB_SKEW 0.0f //between 0 or 1, 1 is linear
#define VIRULENCE 1.0f
#define PARASITE_SIZE 16
#define CROSSOVER 0.7f
#define TOURNAMENT_SIZE 20 //set to POP_SIZE for rank-based selection

int add_weight, sub_weight;

typedef struct Individual {
	unsigned char values[ STRING_LENGTH_BYTES ];
	int eval[ 3 ];
	int add_score, sub_score;
	FPGA fpga;
} Individual;

typedef struct Parasite {
	unsigned char values[ PARASITE_SIZE ];
	float score;
} Parasite;

void evaluate( Individual *ind, Parasite *para, Individual *pop );
