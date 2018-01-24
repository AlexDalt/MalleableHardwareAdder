#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define MUTATION 0.05

typedef struct {
	unsigned char values[4];
	float eval;
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

float evaluate( Individual ind )
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

	return (float)score/32;
}

void evolve( Individual *pop )
{
	Individual most_fit;
	most_fit.eval = 0;

	for ( int i = 0 ; i < 50 ; i++ )
	{
		pop[ i ].eval = evaluate( pop[ i ] );

		if ( most_fit.eval < pop[ i ].eval )
		{
			most_fit = pop[ i ];
		}
	}

	printf( "Most fit bitstring : " );
	print_ind( most_fit );
	printf( " %.3f\n", most_fit.eval );
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

	evolve( pop );

	return 0;
}
