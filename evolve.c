#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "simulator.h"

#define POP_SIZE 400
#define MUTATION 2.7f
#define FITNESS_WEIGHT 3
#define SIZE_WEIGHT 0
#define DIVERSITY_WEIGHT 1
#define ELITISM 1

typedef struct Individual {
	unsigned char values[ STRING_LENGTH_BYTES ];
	int eval[ 3 ];
} Individual;

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

int ind_distance ( Individual x, Individual y )
{
	int distance = 0;
	FPGA fpga_x, fpga_y;

	bitstring_to_fpga( &fpga_x, x.values );
	bitstring_to_fpga( &fpga_y, y.values );

	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga_x.cells[ i ][ j ].gate != fpga_y.cells[ i ][ j ].gate )
			{
				distance++;
			}
			if ( fpga_x.cells[ i ][ j ].n_out != fpga_y.cells[ i ][ j ].n_out )
			{
				distance++;
			}
			if ( fpga_x.cells[ i ][ j ].e_out != fpga_y.cells[ i ][ j ].e_out )
			{
				distance++;
			}
			if ( fpga_x.cells[ i ][ j ].s_out != fpga_y.cells[ i ][ j ].s_out )
			{
				distance++;
			}
			if ( fpga_x.cells[ i ][ j ].w_out != fpga_y.cells[ i ][ j ].w_out )
			{
				distance++;
			}
			if ( fpga_x.cells[ i ][ j ].g_in1 != fpga_y.cells[ i ][ j ].g_in1 )
			{
				distance++;
			}
			if ( fpga_x.cells[ i ][ j ].g_in2 != fpga_y.cells[ i ][ j ].g_in2 )
			{
				distance++;
			}
		}
	}

	return distance;
}

void evaluate( Individual *ind, Individual *pop )
{
	FPGA fpga;
	int fitness = 1;
	int size = 1;
	int diversity = 1;

	for ( int i = 0 ; i < pow(2,FPGA_WIDTH) ; i++ )
	{
		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;

		int v1 = i & mask;
		int v2 = ( i >> FPGA_WIDTH/2 ) & mask;

		bitstring_to_fpga( &fpga, ind->values );

		for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
		{
			fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
		}

		for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
		{
			fpga.input[ j * 2 + 1 ] = ( v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
		}

		evaluate_fpga( &fpga );

		int sum = v1 + v2;

		for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val == (( sum >> j ) & 1) )
			{
				fitness++;
			}
		}
	}

	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga.cells[ i ][ j ].gate == OFF )
			{
				size++;
			}
		}
	}

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{

		int distance = ind_distance( *ind, pop[ i ] );
		diversity += pow ( distance, 2 );
	}

	diversity = diversity / POP_SIZE;
	diversity = (int)sqrt( (double)diversity );

	ind->eval[ 0 ] = fitness;
	ind->eval[ 1 ] = size;
	ind->eval[ 2 ] = diversity;
}

void quicksort( Individual *pop, int low, int high )
{
	if ( low < high )
	{
		int index = low;
		Individual pivot = pop[ index ];
		int pivot_score = FITNESS_WEIGHT * pivot.eval[ 0 ] + SIZE_WEIGHT * pivot.eval[ 1 ] + DIVERSITY_WEIGHT * pivot.eval[ 2 ];
		for ( int i = low + 1 ; i < high ; i++ )
		{
			int pop_score = FITNESS_WEIGHT * pop[ i ].eval[ 0 ] + SIZE_WEIGHT * pop[ i ].eval[ 1 ] + DIVERSITY_WEIGHT * pop[ i ].eval[ 2 ];
			if ( pop_score < pivot_score )
			{
				Individual disp = pop[ index + 1 ];
				Individual new = pop[ i ];
				pop[ i ] = disp;
				pop[ index + 1 ] = pivot;
				pop[ index ] = new;
				index++;
			}
		}

		quicksort( pop, low, index );
		quicksort( pop, index + 1, high );
	}
}

void order( Individual *pop )
{
	quicksort( pop, 0, POP_SIZE );
}

void new_pop( Individual *pop )
{
	int random;
	int total_score = 0;
	Individual new_pop[ POP_SIZE ];
	Individual elite;
	elite.eval[ 0 ] = 0;

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		total_score += 1 + i;
	}

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		if ( pop[ i ].eval[ 0 ] >= elite.eval[ 0 ] )
		{
			elite = pop[ i ];
		}
	}

	order( pop );

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		random = rand() % total_score;
		int score_count = 0;
		int index = 0;
		while ( index < POP_SIZE && score_count + 1 + index < random )
		{
			score_count += 1 + index;
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
				if( random_mut < MUTATION / (float)(8*STRING_LENGTH_BYTES) )
				{
					new_pop[ i ].values[ j ] = new_pop[ i ].values[ j ] ^ (1 << k);
				}
			}
		}
	}

	if ( ELITISM )
	{
		new_pop[ 0 ] = elite;
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
	int most_fit_score = -1;

	while( most_fit.eval[ 0 ] != (FPGA_WIDTH/2 + 1) * pow( 2, FPGA_WIDTH ) + 1 )
	{
		most_fit_score = -1;
		int mean_fit = 0;
		int mean_size = 0;
		int mean_div = 0;

		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			evaluate( &(pop[ i ]), pop );

			int score = FITNESS_WEIGHT * pop[ i ].eval[ 0 ] + SIZE_WEIGHT * pop[ i ].eval[ 1 ] + DIVERSITY_WEIGHT * pop[ i ].eval[ 2 ];
			mean_fit += pop[ i ].eval[ 0 ];
			mean_size += pop[ i ].eval[ 1 ];
			mean_div += pop[ i ].eval[ 2 ];

			if ( most_fit_score < score )
			{
				most_fit = pop[ i ];
				most_fit_score = score;
			}
		}

		mean_fit = mean_fit/POP_SIZE;
		mean_size = mean_size/POP_SIZE;
		mean_div = mean_div/POP_SIZE;

		printf( "Itteration %2d top 10 :\n", iteration );
		for ( int i = 0 ; i < 10 ; i++ )
		{
			Individual ind = pop[ POP_SIZE - 1 - i ];
			printf( "    %d : fitness %3d, size %3d, diversity %3d\n", i, ind.eval[ 0 ] - 1, FPGA_WIDTH * FPGA_HEIGHT + 1 - ind.eval[ 1 ], ind.eval[ 2 ] );
		}
		printf( "Population mean scores: fitness %d, size %d, diversity %d\n", mean_fit, FPGA_WIDTH * FPGA_HEIGHT + 1 - mean_size, mean_div);

		iteration++;

		new_pop( pop );

		if ( ELITISM )
		{
			printf( "Elite fitness: %d\n", pop[ 0 ].eval[ 0 ] );
		}
	}

	printf("final fpga:\n");
	bitstring_to_fpga( &fpga, most_fit.values );
	for ( int i = 0 ; i < FPGA_WIDTH ; i++ )
	{
		fpga.input[ i ] = rand() & 1;
	}
	evaluate_fpga( &fpga );

	print_ind( most_fit );
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
