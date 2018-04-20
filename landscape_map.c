#include "evolve.h"
#define POP_NUM 12000

int lower(Individual x, Individual y)
{
	for ( int i = 32 ; i >= 0 ; i-- )
	{
		if ( !(x.values[ i ] == y.values[ i ]) )
		{
			if ( x.values[ i ] < y.values[ i ] )
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}

	return 0;
}

void ind_sort(Individual *pop, int low, int high)
{
	if ( low < high )
	{
		int index = low;
		Individual pivot = pop[ index ];

		for ( int i = low + 1 ; i < high ; i++ )
		{
			if ( lower( pop[ i ], pivot ) )
			{
				Individual disp = pop[ index + 1 ];
				Individual new = pop[ i ];
				pop[ i ] = disp;
				pop[ index + 1 ] = pivot;
				pop[ index ] = new;
				index++;
			}
		}
	}
}

int main()
{
	FPGA fpga;
	Individual ind[POP_NUM];
	int score[POP_NUM];
	int random;
	int peaked = 0;

	int rng = open("/dev/urandom", O_RDONLY );
	read( rng, &random, sizeof(int));
	close( rng );
	srand( random );

	while(!peaked)
	{
		for ( int j = 0 ; j < POP_NUM ; j++ )
		{
			for ( int i = 0 ; i < 32 ; i++ )
			{
				ind[ j ].values[ i ] = (unsigned char)rand()%256;
			}
		}

		ind_sort(ind, 0, POP_NUM);
		printf("sorted");

		for ( int j = 0 ; j < POP_NUM ; j++ )
		{
			score[ j ] = 0;
			bitstring_to_fpga( &fpga, ind[ j ].values );

			for ( int x = 0 ; x < 16 ; x++ )
			{
				int mask = 3;
				int v1 = x & mask;
				int v2 = (x >> 2) & mask;

				int sum = v1 + v2;

				for ( int y = 0 ; y < 2 ; y++ )
				{
					fpga.input[ y * 2 ] = (v1 >> (FPGA_WIDTH/2 - y - 1)) & 1;
					fpga.input[ y * 2 + 1 ] = (v2 >> (FPGA_WIDTH/2 - y - 1)) & 1;
				}

				fpga.control = 1;
				evaluate_fpga( &fpga );

				for ( int y = 0 ; y < 3 ; y++ )
				{
					if ( fpga.cells[ 3 ][ 3 - y ].s_val == ((sum >> y) & 1) )
					{
						score[ j ]++;
					}
				}
			}

			printf("[%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d] fitness = %d\n",
					ind[ j ].values[ 0 ], ind[ j ].values[ 1 ], ind[ j ].values[ 2 ], ind[ j ].values[ 3 ],
					ind[ j ].values[ 4 ], ind[ j ].values[ 5 ], ind[ j ].values[ 6 ], ind[ j ].values[ 7 ],
					ind[ j ].values[ 8 ], ind[ j ].values[ 9 ], ind[ j ].values[ 10 ], ind[ j ].values[ 11 ],
					ind[ j ].values[ 12 ], ind[ j ].values[ 13 ], ind[ j ].values[ 14 ], ind[ j ].values[ 15 ],
					ind[ j ].values[ 16 ], ind[ j ].values[ 17 ], ind[ j ].values[ 18 ], ind[ j ].values[ 19 ],
					ind[ j ].values[ 20 ], ind[ j ].values[ 21 ], ind[ j ].values[ 22 ], ind[ j ].values[ 23 ],
					ind[ j ].values[ 24 ], ind[ j ].values[ 25 ], ind[ j ].values[ 26 ], ind[ j ].values[ 27 ],
					ind[ j ].values[ 28 ], ind[ j ].values[ 29 ], ind[ j ].values[ 30 ], ind[ j ].values[ 31 ],
					score[ j ]);

			if( score[ j ] == 48 )
			{
				peaked = 1;
			}
		}
	}

	FILE *fp = fopen( "map.dat", "a" );
	for ( int i = 0 ; i < POP_NUM ; i++ )
	{
		fprintf(fp, "%d\n",score[i]);
	}
	fclose(fp);

	return 0;
}
