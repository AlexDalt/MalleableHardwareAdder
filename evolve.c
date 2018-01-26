#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "simulator.h"

#define MUTATION 0.003f

typedef struct {
	unsigned char values[4];
	int eval;
} Individual;

void print_ind( Individual ind )
{
	for ( int i = 0 ; i < 4 ; i++ )
	{
		for ( int j = 7 ; j >= 0 ; j-- )
		{
			printf( "%d", !!(ind.values[ i ] & ( 1 << j )) );
		}
	}
}

int evaluate( Individual ind )
{
	int score = 0;
	for ( int i = 0 ; i < 4 ; i++ )
	{
		for ( int j = 7 ; j >= 0 ; j-- )
		{
			if ( !!(ind.values[ i ] & ( 1 << j )) )
			{
				score++;
			}
		}
	}

	return score;
}

void new_pop( Individual most_fit, Individual *pop )
{
	int random;
	int total_score = 0;
	Individual new_pop[ 50 ];

	for ( int i = 0 ; i < 50 ; i++ )
	{
		total_score += pop[ i ].eval;
	}


	for ( int i = 0 ; i < 50 ; i++ )
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

	for ( int i = 0 ; i < 50 ; i++ )
	{
		for ( int j = 0 ; j < 4; j++ )
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

	for ( int  i = 0 ; i < 50 ; i++ )
	{
		pop[ i ] = new_pop[ i ];
	}
}

void evolve( Individual *pop )
{
	int iteration = 0;
	Individual most_fit;
	most_fit.eval = 0;

	while( most_fit.eval != 32 )
	{

		for ( int i = 0 ; i < 50 ; i++ )
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
}

int main()
{
	Individual pop[50];
	int random;

	int rng = open( "/dev/urandom", O_RDONLY );
	read( rng, &random, sizeof( int ) );
	close( rng );
	srand( random );

	for ( int i = 0 ; i < 50 ; i++ )
	{
		pop[ i ].values[ 0 ] = (unsigned char)(rand() % 255);
		pop[ i ].values[ 1 ] = (unsigned char)(rand() % 255);
		pop[ i ].values[ 2 ] = (unsigned char)(rand() % 255);
		pop[ i ].values[ 3 ] = (unsigned char)(rand() % 255);
	}

	//evolve( pop );

	FPGA fpga;
	pop[0].values[0] = 51;
	pop[0].values[1] = 0;
	pop[0].values[2] = 0;
	pop[0].values[3] = 26;
	bitstring_to_fpga ( &fpga, pop[0].values );
	fpga.cells[0][1].out = 0;
	fpga.cells[1][0].out = 0;
	print_fpga ( &fpga );
	evaluate_fpga ( &fpga );
	printf ( "output bits, &: %d, |: %d\n", fpga.cells[0][0].out, fpga.cells[1][1].out );

	return 0;
}
