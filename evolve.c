#include "evolve.h"

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

int full_test( Individual *ind )
{
	FPGA fpga = ind->fpga;
	int score = 0;
	int add_fit = 0;
	int sub_fit = 0;

	for ( int i = 0 ; i < pow(2,FPGA_WIDTH) ; i++ )
	{
		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;
		int value = i;

		int v1 = value & mask;
		int v2 = ( value >> FPGA_WIDTH/2 ) & mask;
		int sum = v1 + v2;
		int dif = v1 - v2;

		for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
		{
			fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
			fpga.input[ j * 2 + 1 ] = ( v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
		}

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

	}

	score = add_weight * add_fit + sub_weight * sub_fit;

	ind->add_score = add_fit;
	ind->sub_score = sub_fit;

	return score/(add_weight + sub_weight);
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
		max = PARASITE_SIZE/2;
		for ( int i = 0 ; i < max ; i++ )
		{
			int mask = ( 1 << FPGA_WIDTH/2 ) - 1;
			int value = para->values[ i ];

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

			value = para->values[ i + PARASITE_SIZE/2 ];
			v1 = value & mask;
			v2 = ( value >> FPGA_WIDTH/2 ) & mask;
			sum = v1 + v2;
			dif = v1 - v2;

			for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
			{
				fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
				fpga.input[ j * 2 + 1 ] = ( v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
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
	}
	else
	{
		fitness = full_test( ind );
	}


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

	if ( DIVERSITY_WEIGHT != -1 )
	{
		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			int distance = ind_distance( *ind, pop[ i ] );
			diversity += pow ( distance, 2 );
		}

		diversity = diversity / POP_SIZE;
		diversity = (int)sqrt( (double)diversity );
	}

	ind->eval[ 0 ] = fitness;
	ind->eval[ 1 ] = size;
	ind->eval[ 2 ] = diversity;
}

void mod_parasite( Parasite *pop )
{
	float best = -1;
	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		if ( pop[ i ].score > best )
		{
			best = pop[ i ].score;
		}
	}

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		float score = pop[ i ].score/best;
		pop[ i ].score = (2 * score)/VIRULENCE - (pow( score, 2 ))/pow( VIRULENCE, 2 );
	}
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

void shuffle_individuals( Individual *pop )
{
	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		int j = i + rand() / (RAND_MAX / ( POP_SIZE - i ) + 1);
		Individual t = pop[ j ];
		pop[ j ] = pop[ i ];
		pop[ i ] = t;
	}
}

float ind_prob( int position )
{
	float b = PROB_SKEW;
	float a = ( 1 - b )/POP_SIZE;

	return a * pow( position, 2 ) + b * position + 1;
}

void new_pop( Individual *pop, Parasite *para_pop )
{
	int random;
	int total_score = 0;
	Individual new_pop[ POP_SIZE ];
	Parasite new_para_pop[ POP_SIZE ];
	Individual sub_pop[ TOURNAMENT_SIZE ];
	Parasite sub_para_pop[ TOURNAMENT_SIZE ];

	order( pop );

	if ( COEVOLVE )
	{
		mod_parasite( para_pop );
		order_parasite( para_pop );
	}

	if ( TOURNAMENT_SIZE < POP_SIZE )
	{
		for ( int i = 0 ; i < TOURNAMENT_SIZE ; i++ )
		{
			total_score += ind_prob( i );
		}
	}
	else
	{
		for ( int i = 0 ; i < POP_SIZE ; i++ )
		{
			total_score += ind_prob( i );
		}
	}

	for ( int i = 0 ; i < POP_SIZE ; i++ )
	{
		random = rand() % total_score;
		int score_count = 0;
		int index = 0;
		if ( TOURNAMENT_SIZE < POP_SIZE )
		{
			shuffle_individuals( pop );
			for ( int j = 0 ; j < TOURNAMENT_SIZE ; j++ )
			{
				sub_pop[ j ] = pop[ j ];
			}
			quicksort( sub_pop, 0, TOURNAMENT_SIZE );
			new_pop[ i ] = sub_pop[ TOURNAMENT_SIZE - 1];
		}
		else
		{
			while ( index < POP_SIZE && score_count + ind_prob( index ) < random )
			{
				score_count += ind_prob( index );
				index++;
			}

			new_pop[ i ] = pop[ index ];
		}

		if ( COEVOLVE )
		{
			random = rand() % total_score;
			int score_count = 0;
			int index = 0;
			if ( TOURNAMENT_SIZE < POP_SIZE )
			{
				shuffle_parasites( para_pop );
				for ( int j = 0 ; j < TOURNAMENT_SIZE ; j++ )
				{
					sub_para_pop[ j ] = para_pop[ j ];
				}
				quicksort_parasite( sub_para_pop, 0, TOURNAMENT_SIZE );
				new_para_pop[ i ] = sub_para_pop[ TOURNAMENT_SIZE - 1 ];
			}
			else
			{
				while ( index < POP_SIZE && score_count + ind_prob( index ) < random )
				{
					score_count += ind_prob( index );
					index++;
				}

				new_para_pop[ i ] = para_pop[ index ];
			}
		}
	}

	if ( CROSSOVER > 0 )
	{
		shuffle_individuals( new_pop );
		for( int i = 0 ; i < POP_SIZE ; i = i + 2 )
		{
			float cross_rand = (float)rand() / (float) RAND_MAX;
			if ( cross_rand > CROSSOVER )
			{
				int swap_pos = rand() % STRING_LENGTH_BYTES;
				for ( int j = 0 ; j < swap_pos ; j++ )
				{
					unsigned char temp = new_pop[ i ].values[ j ];
					new_pop[ i ].values[ j ] = new_pop[ i + 1 ].values[ j ];
					new_pop[ i + 1 ].values[ j ] = temp;
				}
			}
		}

		if ( COEVOLVE )
		{
			shuffle_parasites( new_para_pop );
			for( int i = 0 ; i < POP_SIZE ; i = i + 2 )
			{
				float cross_rand = (float)rand() / (float) RAND_MAX;
				if ( cross_rand > CROSSOVER )
				{
					int swap_pos = rand() % PARASITE_SIZE;
					for ( int j = 0 ; j < swap_pos ; j++ )
					{
						unsigned char temp = new_para_pop[ i ].values[ j ];
						new_para_pop[ i ].values[ j ] = new_para_pop[ i + 1 ].values[ j ];
						new_para_pop[ i + 1 ].values[ j ] = temp;
					}
				}
			}
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
				for ( int k = 0 ; k < 4 ; k++ )
				{
					float random_mut = (float)rand() / (float)RAND_MAX;
					if ( random_mut < MUTATION/(float)64 )
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
	int test_run = 0;
	int iteration = 0;
	int perfect_run = 0;
	int fault = 0;
	Individual most_fit;
	int most_fit_score = -1;
	add_weight = 10;
	sub_weight = 0;

	int avg_mean[ TEST_LOOP ];
	int avg_add[ TEST_LOOP ];
	int avg_sub[ TEST_LOOP ];
	int avg_div[ TEST_LOOP ];
	int avg_best[ TEST_LOOP ];

	for ( int i = 0 ; i < TEST_LOOP ; i++ )
	{
		avg_mean[ i ] = 0;
		avg_add[ i ] = 0;
		avg_sub[ i ] = 0;
		avg_best[ i ] = 0;
		avg_div[ i ] = 0;
	}

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

	clock_t begin = clock();

	while ( test_run < TEST_SIZE )
	{
		int mean_fit = 0;
		int mean_add_fit = 0;
		int mean_sub_fit = 0;
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
			mean_add_fit += pop[ i ].add_score;
			mean_sub_fit += pop[ i ].sub_score;

			if ( COEVOLVE && ELITISM )
			{
				if ( most_fit_score < full_test( &pop[ i ] ) )
				{
					most_fit = pop[ i ];
					most_fit_score = full_test( &pop[ i ] );
				}
			}
			else
			{
				if ( most_fit_score <= pop[ i ].eval[ 0 ] )
				{
					most_fit = pop[ i ];
					most_fit_score = pop[ i ].eval[ 0 ];
				}
			}
		}

		mean_fit = mean_fit/POP_SIZE;
		mean_size = mean_size/POP_SIZE;
		mean_div = mean_div/POP_SIZE;
		mean_para_fit = mean_para_fit/POP_SIZE;
		mean_add_fit = mean_add_fit/POP_SIZE;
		mean_sub_fit = mean_sub_fit/POP_SIZE;

		int test = full_test( &most_fit );

		if ( COEVOLVE )
		{
			mean_fit = 0;

			for ( int j = 0 ; j < POP_SIZE ; j++ )
			{
				mean_fit += full_test( &pop[ j ] );
			}

			mean_fit = mean_fit/POP_SIZE;
		}

		redraw( test_run, iteration, most_fit.fpga, test, mean_fit, mean_div, add_weight, sub_weight );

		if ( LOG )
		{
			log_data( iteration, mean_fit, most_fit.eval[ 0 ], test, mean_para_fit );
		}

		avg_mean[ iteration ] = avg_mean[ iteration ] * test_run + mean_fit;
		avg_mean[ iteration ] = avg_mean[ iteration ] / (test_run + 1);
		avg_add[ iteration ] = avg_add[ iteration ] * test_run + mean_add_fit;
		avg_add[ iteration ] = avg_add[ iteration ] / (test_run + 1);
		avg_sub[ iteration ] = avg_sub[ iteration ] * test_run + mean_sub_fit;
		avg_sub[ iteration ] = avg_sub[ iteration ] / (test_run + 1);
		avg_best[ iteration ] = avg_best[ iteration ] * test_run + test;
		avg_best[ iteration ] = avg_best[ iteration ] / (test_run + 1);
		avg_div[ iteration ] = avg_div[ iteration ] * test_run + mean_div;
		avg_div[ iteration ] = avg_div[ iteration ] / (test_run + 1);

		iteration++;

		new_pop( pop, para_pop );

		if ( ELITISM )
		{
			pop[ 0 ] = most_fit;
		}

		most_fit_score = -1;

		int c = getch();

		if ( c == 'f' || (FAULT_INJECTION > 0 && STICKY && iteration % FAULT_INJECTION == 0) || (FAULT_INJECTION > 0 && iteration == FAULT_INJECTION ))
		{
			fault = (fault + 1) % 2;
		}
		if ( c == 'r' || iteration == TEST_LOOP )
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

			if ( test == (FPGA_WIDTH/2 + 1) * pow(2,FPGA_WIDTH))
			{
				perfect_run++;
			}
			iteration = 0;
			test_run++;
			fault = 0;
		}
		else if ( c == 'd' || ((FAULT_INJECTION > 0 || WEIGHT_TEST > 0 ) && !STICKY && iteration == 2) )
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
		else if ( (c == KEY_LEFT || (WEIGHT_TEST > 0 && iteration % WEIGHT_TEST == 0)) && add_weight > 0)
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

	clock_t end = clock();
	double execution_time = (double)(end - begin) / CLOCKS_PER_SEC;
	execution_time = execution_time / TEST_SIZE;

	FILE *fp1 = fopen( "test.dat", "a" );
	if ( fp1 != NULL )
	{
		for ( int i = 0 ; i < TEST_LOOP ; i++ )
		{
			fprintf( fp1, "%d    %d    %d	%d	%d	%d\n", i, avg_mean[ i ], avg_best[ i ], avg_div[ i ], avg_add[ i ], avg_sub[ i ] );
		}
		fclose( fp1 );
	}

	FILE *fp2 = fopen( "summary.txt", "a" );
	if ( fp2 != NULL )
	{
		fprintf( fp2, "Number of perfect runs %d/%d, average time %f, average best case end fitness %d\n", perfect_run, TEST_SIZE, execution_time, avg_best[ TEST_LOOP - 1] );
		fclose( fp2 );
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
