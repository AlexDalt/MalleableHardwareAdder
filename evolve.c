#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "simulator.h"

#define POP_SIZE 400
#define MUTATION 1.5f
#define SIZE_WEIGHT 0
#define DIVERSITY_WEIGHT 4
#define ELITISM 1
#define FITNESS_WEIGHT 10

int add_weight, sub_weight;

typedef struct Individual {
	unsigned char values[ STRING_LENGTH_BYTES ];
	int eval[ 3 ];
	FPGA fpga;
} Individual;

void log_data( int iteration, int mean_fit, int most_fit )
{
	FILE *fp = fopen( "log.dat", "a" );
	if ( fp != NULL )
	{
		fprintf( fp, "%d    %d    %d\n", iteration, mean_fit, most_fit );
		fclose( fp );
	}
}

int ind_distance ( Individual x, Individual y )
{
	int distance = 0;
	FPGA fpga_x = x.fpga;
	FPGA fpga_y = y.fpga;

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
	FPGA fpga = ind->fpga;
	int fitness = 0;
	int size = 0;
	int diversity = 0;

	for ( int i = 0 ; i < pow(2,FPGA_WIDTH) ; i++ )
	{
		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;

		int v1 = i & mask;
		int v2 = ( i >> FPGA_WIDTH/2 ) & mask;
		int sum = v1 + v2;
		int dif = v1 - v2;

		for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
		{
			fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
			fpga.input[ j * 2 + 1 ] = ( v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
		}

		int add_fit = 0;
		int sub_fit = 0;
		fpga.control = 0;
		evaluate_fpga( &fpga );

		for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val == (( sum >> j ) & 1) )
			{
				add_fit++;
			}
		}

		fpga.control = 1;
		evaluate_fpga( &fpga );

		for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val == (( dif >> j ) & 1) )
			{
				sub_fit++;
			}
		}

		fitness += add_weight * add_fit + sub_weight * sub_fit;
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
		int pivot_score = (FITNESS_WEIGHT / (add_weight + sub_weight)) * pivot.eval[ 0 ] + SIZE_WEIGHT * pivot.eval[ 1 ] + DIVERSITY_WEIGHT * pivot.eval[ 2 ];
		for ( int i = low + 1 ; i < high ; i++ )
		{
			int pop_score = (FITNESS_WEIGHT / (add_weight + sub_weight)) * pop[ i ].eval[ 0 ] + SIZE_WEIGHT * pop[ i ].eval[ 1 ] + DIVERSITY_WEIGHT * pop[ i ].eval[ 2 ];
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

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		total_score += 1 + i;
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

	for ( int  i = 0 ; i < POP_SIZE ; i++ )
	{
		pop[ i ] = new_pop[ i ];
		bitstring_to_fpga( &pop[ i ].fpga, pop[ i ].values );
	}
}

void evolve( Individual *pop )
{
	int iteration = 0;
	Individual most_fit;
	int most_fit_score = -1;
	add_weight = 1;
	sub_weight = 0;

	Fault faults[ FAULT_NUM ];
	for ( int i = 0 ; i < FAULT_NUM ; i++ )
	{
		faults[ i ].x = rand() % FPGA_WIDTH;
		faults[ i ].y = rand() % FPGA_HEIGHT;
		faults[ i ].dir = rand() % 4;
		if ( faults[ i ].x == 0 && faults[ i ].dir == WEST )
		{
			faults[ i ].dir = EAST;
		}
		if ( faults[ i ].y == 0 && faults[ i ].dir == NORTH )
		{
			faults[ i ].dir = SOUTH;
		}
		if ( faults[ i ].x == FPGA_WIDTH - 1 && faults[ i ].dir == EAST )
		{
			faults[ i ].dir = WEST;
		}
		if ( faults[ i ].y == FPGA_HEIGHT - 1 && faults[ i ].dir == SOUTH )
		{
			faults[ i ].dir = NORTH;
		}
		faults[ i ].value = rand() % 3;
	}

	while( true )
	{
		int mean_fit = 0;
		int mean_size = 0;
		int mean_div = 0;

		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			for ( int j = 0 ; j < FAULT_NUM ; j++ )
			{
				if ( iteration % 1000 >= 500 )
				{
					pop[ i ].fpga.active_fault[ j ] = 0;
				}
				else
				{
					pop[ i ].fpga.active_fault[ j ] = 1;
				}
				pop[ i ].fpga.faults[ j ] = faults[ j ];
			}
		}

		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			evaluate( &(pop[ i ]), pop );

			mean_fit += pop[ i ].eval[ 0 ];
			mean_size += pop[ i ].eval[ 1 ];
			mean_div += pop[ i ].eval[ 2 ];

			if ( most_fit_score <= pop[ i ].eval[ 0 ] )
			{
				most_fit = pop[ i ];
				most_fit_score = pop[ i ].eval[ 0 ];
			}
		}

		mean_fit = mean_fit/POP_SIZE;
		mean_size = mean_size/POP_SIZE;
		mean_div = mean_div/POP_SIZE;

		redraw( iteration, most_fit.fpga, most_fit.eval[ 0 ], mean_fit, mean_div, add_weight, sub_weight );
		log_data( iteration, mean_fit, most_fit.eval[ 0 ] );

		iteration++;

		new_pop( pop );

		if ( ELITISM )
		{
			pop[ 0 ] = most_fit;
		}

		most_fit_score = -1;
	}
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
			bitstring_to_fpga( &pop[ i ].fpga, pop[ i ].values );
		}
	}

	init_curses();

	evolve( pop );

	tidy_up_curses();

	return 0;
}
