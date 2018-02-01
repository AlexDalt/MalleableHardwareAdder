#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "simulator.h"

#define MUTATION 0.05f
#define POP_SIZE 100

typedef struct {
	unsigned char values[STRING_LENGTH_BYTES];
	int eval;
} Individual;

void print_fpga_exhaustive ( Individual ind )
{
	FPGA fpga;
	for ( int i = 0 ; i < pow(2,FPGA_WIDTH) ; i++ )
	{
		bitstring_to_fpga( &fpga, ind.values );

		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			fpga.input[ j ] = ( i >> (FPGA_WIDTH - j - 1) ) & 1;
		}

		evaluate_fpga( &fpga );
		print_fpga( &fpga );
		int score = 0;

		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;

		int v1 = i & mask;
		int v2 = ( i >> FPGA_WIDTH/2 ) & mask;

		evaluate_fpga( &fpga );

		int sum = v1 + v2;

		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].out == (( sum >> j ) & 1) )
			{
				score++;
			}
		}
		printf("score: %d\n",score);
	}
}

void print_ind( Individual ind )
{
	for ( int i = 0 ; i < STRING_LENGTH_BYTES ; i++ )
	{
		for ( int j = 7 ; j >= 0 ; j-- )
		{
			printf( "%d", !!(ind.values[ i ] & ( 1 << j )) );
		}
		printf( " " );
	}
}

int evaluate( Individual ind )
{
	FPGA fpga;
	int score = 0;
	for ( int i = 0 ; i < pow(2,FPGA_WIDTH) ; i++ )
	{
		bitstring_to_fpga( &fpga, ind.values );

		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			fpga.input[ j ] = ( i >> (FPGA_WIDTH - j - 1) ) & 1;
		}

		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;

		int v1 = i & mask;
		int v2 = ( i >> FPGA_WIDTH/2 ) & mask;

		evaluate_fpga( &fpga );

		int sum = v1 + v2;

		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].out == (( sum >> j ) & 1) )
			{
				score++;
			}
		}
	}

	if ( score == 0 )
	{
		score = 1;
	}

	return score;
}

void new_pop( Individual most_fit, Individual *pop )
{
	int random;
	int total_score = 0;
	Individual new_pop[ POP_SIZE ];

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		total_score += pop[ i ].eval;
	}

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		random = rand() % total_score;
		int score_count = 0;
		int index = 0;
		while ( score_count + pop[ index ].eval < random )
		{
			score_count += pop[ index ].eval;
			index++;
		}

		new_pop[ i ] = pop[ index ];
	}

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		for ( int j = 0 ; j < STRING_LENGTH_BYTES; j++ )
		{
			for ( int k = 0 ; k < 8 ; k++ )
			{
				float random_mut = (float)rand() / (float)RAND_MAX;
				if( random_mut < MUTATION )
				{
					new_pop[ i ].values[ j ] = new_pop[ i ].values[ j ] ^ (1 << k);
				}
			}
		}
	}

	for ( int  i = 0 ; i < POP_SIZE ; i++ )
	{
		pop[ i ] = new_pop[ i ];
	}
}

void evolve( Individual *pop )
{
	FPGA fpga;
	int iteration = 0;
	Individual most_fit;
	most_fit.eval = 0;

	while( most_fit.eval != FPGA_WIDTH * pow( 2, FPGA_WIDTH ) )
	{
		most_fit.eval = 0;
		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			pop[ i ].eval = evaluate( pop[ i ] );

			if ( most_fit.eval < pop[ i ].eval )
			{
				most_fit = pop[ i ];
			}
		}

		printf( "Most fit bitstring %2d : ", iteration );
		print_ind( most_fit );
		printf( " %d\n", most_fit.eval );

		iteration++;

		new_pop( most_fit, pop );
	}

	printf("final fpga:\n");
	bitstring_to_fpga( &fpga, most_fit.values );
	evaluate_fpga( &fpga );
	print_fpga( &fpga );
}

int main()
{
	Individual pop[POP_SIZE];
	int random;

	int rng = open( "/dev/urandom", O_RDONLY );
	read( rng, &random, sizeof( int ) );
	close( rng );
	srand( random );

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		for ( int j = 0 ; j < STRING_LENGTH_BYTES ; j++ )
		{
			pop[ i ].values[ j ] = (unsigned char)(rand() % 255);
		}
	}

	evolve( pop );
	return 0;
}
