#include <stdio.h>
#include <stdlib.h>

#define MUTATION 0.05

typedef struct {
	unsigned char values[4];
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

int main()
{
	Individual pop[50];
	
	for ( int i = 0 ; i < 50 ; i++ )
	{
		pop[ i ].values[ 0 ] = (unsigned char)(rand() % 255);
		pop[ i ].values[ 1 ] = (unsigned char)(rand() % 255);
		pop[ i ].values[ 2 ] = (unsigned char)(rand() % 255);
		pop[ i ].values[ 3 ] = (unsigned char)(rand() % 255);

		printf( "%2d : ", i );
		print_ind( pop[ i ] );
		printf( "\n" );
	}

	return 0;
}
