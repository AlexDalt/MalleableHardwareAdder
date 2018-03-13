#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "simulator.h"

#define POP_SIZE 400
#define MUTATION 2.5f
#define SIZE_WEIGHT 0
#define DIVERSITY_WEIGHT 5
#define ELITISM 1
#define FITNESS_WEIGHT 10
#define COEVOLVE 1
#define STICKY 0
#define LOG 1

int add_weight, sub_weight;

typedef struct Individual {
	unsigned char values[ STRING_LENGTH_BYTES ];
	int eval[ 3 ];
	FPGA fpga;
} Individual;

typedef struct Parasite {
	unsigned char values[ 16 ];
	int score;
} Parasite;

void log_data( int iteration, int mean_fit, int most_fit, int performance, int mean_para_fit )
{
	FILE *fp = fopen( "log.dat", "a" );
	if ( fp != NULL )
	{
		fprintf( fp, "%d    %d    %d	%d	%d\n", iteration, mean_fit, most_fit, performance, mean_para_fit );
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

void evaluate( Individual *ind, Parasite *para, Individual *pop )
{
	FPGA fpga = ind->fpga;
	int fitness = 0;
	int size = 0;
	int diversity = 0;
	int max = pow( 2, FPGA_WIDTH );

	if ( COEVOLVE )
	{
		max = 16;
	}

	for ( int i = 0 ; i < max ; i++ )
	{
		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;
		int value;

		if ( COEVOLVE )
		{
			value = para->values[ i ];
		}
		else
		{
			value = i;
		}

		int v1 = value & mask;
		int v2 = ( value >> FPGA_WIDTH/2 ) & mask;
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

	fitness = fitness/(add_weight + sub_weight);
	para->score = 48 - fitness;

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

void quicksort_parasite( Parasite *pop, int low, int high )
{
	if ( low < high )
	{
		int index = low;
		Parasite pivot = pop[ index ];
		int pivot_score = pivot.score;
		for ( int i = low + 1 ; i < high ; i++ )
		{
			int pop_score = pop[ i ].score;

			if ( pop_score < pivot_score )
			{
				Parasite disp = pop[ index + 1 ];
				Parasite new = pop[ i ];
				pop[ i ] = disp;
				pop[ index + 1 ] = pivot;
				pop[ index ] = new;
				index++;
			}
		}

		quicksort_parasite( pop, low, index );
		quicksort_parasite( pop, index + 1, high );
	}
}

void order( Individual *pop )
{
	quicksort( pop, 0, POP_SIZE );
}

void order_parasite( Parasite *pop )
{
	quicksort_parasite( pop, 0, POP_SIZE );
}

void shuffle_parasites( Parasite *para_pop )
{
	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		int j = i + rand() / (RAND_MAX / ( POP_SIZE - i ) + 1);
		Parasite t = para_pop[ j ];
		para_pop[ j ] = para_pop[ i ];
		para_pop[ i ] = t;
	}
}

void new_pop( Individual *pop, Parasite *para_pop )
{
	int random;
	int total_score = 0;
	Individual new_pop[ POP_SIZE ];
	Parasite new_para_pop[ POP_SIZE ];

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		total_score += 1 + i;
	}

	order( pop );

	if ( COEVOLVE )
	{
		order_parasite( para_pop );
	}

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

		if ( COEVOLVE )
		{
			random = rand() % total_score;
			int score_count = 0;
			int index = 0;
			while ( index < POP_SIZE && score_count + 1 + index < random )
			{
				score_count += 1 + index;
				index++;
			}

			new_para_pop[ i ] = para_pop[ index ];
		}
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

		if ( COEVOLVE )
		{
			for ( int j = 0 ; j < 16 ; j++ )
			{
				for ( int k = 0 ; k < 8 ; k++ )
				{
					float random_mut = (float)rand() / (float)RAND_MAX;
					if ( random_mut < MUTATION/(float)8 )
					{
						new_para_pop[ i ].values[ j ] = new_para_pop[ i ].values[ j ] ^ (1 << k);
					}
				}
			}
		}
	}

	for ( int  i = 0 ; i < POP_SIZE ; i++ )
	{
		pop[ i ] = new_pop[ i ];
		bitstring_to_fpga( &pop[ i ].fpga, pop[ i ].values );

		if ( COEVOLVE )
		{
			para_pop[ i ] = new_para_pop[ i ];
		}
	}
}

void evolve( Individual *pop, Parasite *para_pop )
{
	int iteration = 0;
	int fault = 0;
	Individual most_fit;
	int most_fit_score = -1;
	add_weight = 10;
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

	while ( true )
	{
		int mean_fit = 0;
		int mean_size = 0;
		int mean_div = 0;
		int mean_para_fit = 0;

		if ( COEVOLVE )
		{
			shuffle_parasites( para_pop );
		}

		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			for ( int j = 0 ; j < FAULT_NUM ; j++ )
			{
				pop[ i ].fpga.active_fault[ j ] = fault;
				pop[ i ].fpga.faults[ j ] = faults[ j ];
			}

			evaluate( &pop[ i ], &para_pop[ i ], pop );

			mean_fit += pop[ i ].eval[ 0 ];
			mean_size += pop[ i ].eval[ 1 ];
			mean_div += pop[ i ].eval[ 2 ];
			mean_para_fit += para_pop[ i ].score;

			if ( most_fit_score <= pop[ i ].eval[ 0 ] )
			{
				most_fit = pop[ i ];
				most_fit_score = pop[ i ].eval[ 0 ];
			}
		}

		mean_fit = mean_fit/POP_SIZE;
		mean_size = mean_size/POP_SIZE;
		mean_div = mean_div/POP_SIZE;
		mean_para_fit = mean_para_fit/POP_SIZE;

		int test = 0;

		for ( int i = 0 ; i < pow( 2, FPGA_WIDTH ) ; i++ )
		{
			int mask = ( 1 << FPGA_WIDTH/2 ) - 1;
			int v1 = i & mask;
			int v2 = ( i >> FPGA_WIDTH/2 ) & mask;
			int sum = v1 + v2;
			int dif = v1 - v2;

			for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
			{
				most_fit.fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
				most_fit.fpga.input[ j * 2 + 1 ] = ( v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
			}

			int add_fit = 0;
			int sub_fit = 0;
			most_fit.fpga.control = 0;
			evaluate_fpga( &most_fit.fpga );

			for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
			{
				if ( most_fit.fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val == (( sum >> j ) & 1) )
				{
					add_fit++;
				}
			}

			most_fit.fpga.control = 1;
			evaluate_fpga( &most_fit.fpga );

			for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
			{
				if ( most_fit.fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val == (( dif >> j ) & 1) )
				{
					sub_fit++;
				}
			}

			test += add_weight * add_fit + sub_weight * sub_fit;
		}

		test = test / (add_weight + sub_weight);

		redraw( iteration, most_fit.fpga, test, mean_fit, mean_div, add_weight, sub_weight );

		if ( LOG )
		{
			log_data( iteration, mean_fit, most_fit.eval[ 0 ], test, mean_para_fit );
		}

		iteration++;

		new_pop( pop, para_pop );

		if ( ELITISM )
		{
			pop[ 0 ] = most_fit;
		}

		most_fit_score = -1;

		int c = getch();

		if ( c == 'f' || (STICKY && iteration % 500 == 0) )
		{
			fault = (fault + 1) % 2;
		}
		else if ( c == 'd' )
		{
			pop[ 0 ].values[ 0 ]  =  52; pop[ 0 ].values[ 1 ]  = 32;
			pop[ 0 ].values[ 2 ]  = 108; pop[ 0 ].values[ 3 ]  = 32;
			pop[ 0 ].values[ 4 ]  =  52; pop[ 0 ].values[ 5 ]  = 32;
			pop[ 0 ].values[ 6 ]  = 108; pop[ 0 ].values[ 7 ]  = 32;
			pop[ 0 ].values[ 8 ]  =   0; pop[ 0 ].values[ 9 ]  =  0;
			pop[ 0 ].values[ 10 ] =  52; pop[ 0 ].values[ 11 ] = 32;
			pop[ 0 ].values[ 12 ] = 108; pop[ 0 ].values[ 13 ] = 32;
			pop[ 0 ].values[ 14 ] =   0; pop[ 0 ].values[ 15 ] =  0;
			pop[ 0 ].values[ 16 ] =   0; pop[ 0 ].values[ 17 ] =  0;
			pop[ 0 ].values[ 18 ] =  44; pop[ 0 ].values[ 19 ] = 32;
			pop[ 0 ].values[ 20 ] =   0; pop[ 0 ].values[ 21 ] =  0;
			pop[ 0 ].values[ 22 ] =   0; pop[ 0 ].values[ 23 ] =  0;
			pop[ 0 ].values[ 24 ] =   0; pop[ 0 ].values[ 25 ] =  0;
			pop[ 0 ].values[ 26 ] =   0; pop[ 0 ].values[ 27 ] =  0;
			pop[ 0 ].values[ 28 ] =   0; pop[ 0 ].values[ 29 ] =  0;
			pop[ 0 ].values[ 30 ] =   0; pop[ 0 ].values[ 31 ] =  0;

			for ( int i = 0 ; i < FAULT_NUM ; i++ )
			{
				faults[ i ].x = 2;
				faults[ i ].y = 1;
				faults[ i ].value = 2;
			}
		}
		else if ( c == 'r' )
		{
			for ( int i = 0 ; i < POP_SIZE ; i++ )
			{
				for ( int j = 0 ; j < STRING_LENGTH_BYTES ; j++ )
				{
					pop[ i ].values[ j ] = (unsigned char)(rand() % 255);
					bitstring_to_fpga( &pop[ i ].fpga, pop[ i ].values );
				}
			}
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
		}
		else if ( c == KEY_LEFT && add_weight > 0 )
		{
			add_weight--;
			sub_weight++;
		}
		else if ( c == KEY_RIGHT && sub_weight > 0 )
		{
			add_weight++;
			sub_weight--;
		}
	}
}

int main()
{
	Individual pop[ POP_SIZE ];
	Parasite para_pop[ POP_SIZE ];
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

		for ( int j = 0 ; j < 16 ; j++ )
		{
			para_pop[ i ].values[ j ] = (unsigned char)(rand() % 255);
		}
	}

	init_curses();

	evolve( pop, para_pop );

	tidy_up_curses();

	return 0;
}
