#include <stdio.h>
#include "simulator.h"

void bitstring_to_fpga ( FPGA *fpga, unsigned char *bits )
{
	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			unsigned char byte = bits[ i * FPGA_WIDTH + j ];

			unsigned char gate = byte & 7;
			unsigned char in1 = (byte >> 3) & 3;
			unsigned char in2 = (byte >> 5) & 3;

			fpga->cells[ i ][ j ].gate = (Gate)gate;
			fpga->cells[ i ][ j ].in1_d = (Direction)in1;
			fpga->cells[ i ][ j ].in2_d = (Direction)in2;
			fpga->cells[ i ][ j ].out = 2;
		}
	}
}

void tick ( FPGA *fpga )
{
	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			switch ( fpga->cells[ i ][ j ].in1_d )
			{
				case NORTH:
					if ( i == 0 )
					{
						fpga->cells[ i ][ j ].in1 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in1 = fpga->cells[ i-1 ][ j ].out;
					}
					break;
				case SOUTH:
					if ( i == FPGA_HEIGHT - 1 )
					{
						fpga->cells[ i ][ j ].in1 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in1 = fpga->cells[ i+1 ][ j ].out;
					}
					break;
				case EAST:
					if ( j == FPGA_WIDTH )
					{
						fpga->cells[ i ][ j ].in1 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in1 = fpga->cells[ i ][ j+1 ].out;
					}
					break;
				case WEST:
					if ( j == 0 )
					{
						fpga->cells[ i ][ j ].in1 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in1 = fpga->cells[ i ][ j-1 ].out;
					}
					break;
			}

			switch ( fpga->cells[ i ][ j ].in2_d )
			{
				case NORTH:
					if ( i == 0 )
					{
						fpga->cells[ i ][ j ].in2 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in2 = fpga->cells[ i-1 ][ j ].out;
					}
					break;
				case SOUTH:
					if ( i == FPGA_HEIGHT - 1 )
					{
						fpga->cells[ i ][ j ].in2 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in2 = fpga->cells[ i+1 ][ j ].out;
					}
					break;
				case EAST:
					if ( j == FPGA_WIDTH )
					{
						fpga->cells[ i ][ j ].in2 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in2 = fpga->cells[ i ][ j+1 ].out;
					}
					break;
				case WEST:
					if ( j == 0 )
					{
						fpga->cells[ i ][ j ].in2 = 2;
					}
					else
					{
						fpga->cells[ i ][ j ].in2 = fpga->cells[ i ][ j-1 ].out;
					}
					break;
			}
		}
	}
}

void tock ( FPGA *fpga )
{
	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			unsigned char in1,in2;
			in1 = fpga->cells[ i ][ j ].in1;
			in2 = fpga->cells[ i ][ j ].in2;

			switch ( fpga->cells[ i ][ j ].gate )
			{
				case NOT:
					if ( in1 != 2 )
					{
						fpga->cells[ i ][ j ].out = !in1;
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				case OR:
					if ( in1 != 2 && in2 != 2 )
					{
						fpga->cells[ i ][ j ].out = in1 | in2;
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				case AND:
					if ( in1 != 2 && in2 != 2 )
					{
						fpga->cells[ i ][ j ].out = in1 & in2;
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				case NAND:
					if ( in1 != 2 && in2 != 2 )
					{
						fpga->cells[ i ][ j ].out = !(in1 & in2);
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				case NOR:
					if ( in1 != 2 && in2 != 2 )
					{
						fpga->cells[ i ][ j ].out = !(in1 | in2);
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				case XOR:
					if ( in1 != 2 && in2 != 2 )
					{
						fpga->cells[ i ][ j ].out = in1 ^ in2;
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				case XNOR:
					if ( in1 != 2 && in2 != 2 )
					{
						fpga->cells[ i ][ j ].out = !(in1 | in2);
					}
					else
					{
						fpga->cells[ i ][ j ].out = 2;
					}
					break;
				default:
					break;
			}
		}
	}
}

void evaluate_fpga ( FPGA *fpga )
{
	for ( int i = 0 ; i < CLOCK_CYCLES ; i++ )
	{
		tick ( fpga );
		tock ( fpga );
	}
}

void print_fpga ( FPGA *fpga )
{
	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			printf( "-----" );
		}
		printf("\n");
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga->cells[ i ][ j ].in1_d == NORTH || fpga->cells[ i ][ j ].in2_d == NORTH )
			{
				printf( "| v |" );
			}
			else
			{
				printf( "|   |" );
			}
		}
		printf("\n");
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga->cells[ i ][ j ].in1_d == WEST || fpga->cells[ i ][ j ].in2_d == WEST )
			{
				printf( "|>" );
			}
			else
			{
				printf( "| " );
			}

			switch ( fpga->cells[ i ][ j ].gate )
			{
				case OFF:
					printf( "%d", fpga->cells[ i ][ j ].out );
					break;
				case NOT:
					printf( "!" );
					break;
				case OR:
					printf( "|" );
					break;
				case AND:
					printf( "&" );
					break;
				case NAND:
					printf( "N" );
					break;
				case NOR:
					printf( "n" );
					break;
				case XOR:
					printf( "x" );
					break;
				case XNOR:
					printf( "~" );
					break;
			}

			if ( fpga->cells[ i ][ j ].in1_d == EAST || fpga->cells[ i ][ j ].in2_d == EAST )
			{
				printf( "<|" );
			}
			else
			{
				printf( " |" );
			}
		}
		printf("\n");
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( fpga->cells[ i ][ j ].in1_d == SOUTH || fpga->cells[ i ][ j ].in2_d == SOUTH )
			{
				printf( "| ^ |" );
			}
			else
			{
				printf( "|   |" );
			}
		}
		printf("\n");
	}
	for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
	{
		printf( "-----" );
	}
	printf("\n");
}
