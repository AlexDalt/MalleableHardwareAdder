#include <stdio.h>
#include "simulator.h"

void bitstring_to_fpga ( FPGA *fpga, unsigned char *bits )
{
	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			unsigned char high_byte = bits[ (i * FPGA_WIDTH + j) * 2 ];
			unsigned char low_byte = bits[ (i * FPGA_WIDTH + j) * 2 + 1 ];

			unsigned char n_out = low_byte & 3;
			unsigned char e_out = (low_byte >> 2) & 3;
			unsigned char s_out = (low_byte >> 4) & 3;
			unsigned char w_out = (low_byte >> 6) & 3;

			unsigned char g_in1 = high_byte & 3;
			unsigned char g_in2 = (high_byte >> 2) & 3;
			unsigned char gate = (high_byte >> 4) & 7;

			if ( n_out == NORTH )
			{
				fpga->cells[ i ][ j ].n_out = F;
			}
			else
			{
				fpga->cells[ i ][ j ].n_out = (Direction)n_out;
			}
			if ( e_out == EAST )
			{
				fpga->cells[ i ][ j ].e_out = F;
			}
			else
			{
				fpga->cells[ i ][ j ].e_out = (Direction)e_out;
			}
			if ( s_out == SOUTH )
			{
				fpga->cells[ i ][ j ].s_out = F;
			}
			else
			{
				fpga->cells[ i ][ j ].s_out = (Direction)s_out;
			}
			if ( w_out == WEST )
			{
				fpga->cells[ i ][ j ].w_out = F;
			}
			else
			{
				fpga->cells[ i ][ j ].w_out = (Direction)w_out;
			}

			fpga->cells[ i ][ j ].gate = (Gate)gate;

			fpga->cells[ i ][ j ].g_in1 = (Direction)g_in1;
			fpga->cells[ i ][ j ].g_in2 = (Direction)g_in2;
		}
	}
}

void tick ( FPGA *fpga )
{
	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( i == 0 )
			{
				fpga->cells[ i ][ j ].n_in = fpga->input[ j ];
			}
			else
			{
				fpga->cells[ i ][ j ].n_in = fpga->cells[ i-1 ][ j ].s_val;
			}

			if ( j == FPGA_WIDTH - 1)
			{
				fpga->cells[ i ][ j ].e_in = 2;
			}
			else
			{
				fpga->cells[ i ][ j ].e_in = fpga->cells[ i ][ j + 1 ].w_val;
			}

			if ( i == FPGA_HEIGHT - 1)
			{
				fpga->cells[ i ][ j ].s_in = 2;
			}
			else
			{
				fpga->cells[ i ][ j ].s_in = fpga->cells[ i + 1 ][ j ].n_val;
			}

			if ( j == 0 )
			{
				fpga->cells[ i ][ j ].w_in = 2;
			}
			else
			{
				fpga->cells[ i ][ j ].w_in = fpga->cells[ i + 1 ][ j ].e_val;
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
			unsigned char in1, in2, f;
			unsigned char n_in = fpga->cells[ i ][ j ].n_in;
			unsigned char e_in = fpga->cells[ i ][ j ].e_in;
			unsigned char s_in = fpga->cells[ i ][ j ].s_in;
			unsigned char w_in = fpga->cells[ i ][ j ].w_in;

			switch ( fpga->cells[ i ][ j ].g_in1 )
			{
				case NORTH:
					in1 = n_in;
					break;
				case EAST:
					in1 = e_in;
					break;
				case SOUTH:
					in1 = s_in;
					break;
				case WEST:
					in1 = w_in;
					break;
				default:
					break;
			}

			switch ( fpga->cells[ i ][ j ].g_in2 )
			{
				case NORTH:
					in2 = n_in;
					break;
				case EAST:
					in2 = e_in;
					break;
				case SOUTH:
					in2 = s_in;
					break;
				case WEST:
					in2 = w_in;
					break;
				default:
					break;
			}

			switch ( fpga->cells[ i ][ j ].gate )
			{
				case NOT:
					if ( in1 != 2 )
					{
						f = !in1;
					}
					else
					{
						f = 2;
					}
					break;
				case OR:
					if ( in1 != 2 && in2 != 2 )
					{
						f = in1 | in2;
					}
					else
					{
						f = 2;
					}
					break;
				case AND:
					if ( in1 != 2 && in2 != 2 )
					{
						f = in1 & in2;
					}
					else
					{
						f = 2;
					}
					break;
				case NAND:
					if ( in1 != 2 && in2 != 2 )
					{
						f = !(in1 & in2);
					}
					else
					{
						f = 2;
					}
					break;
				case NOR:
					if ( in1 != 2 && in2 != 2 )
					{
						f = !(in1 | in2);
					}
					else
					{
						f = 2;
					}
					break;
				case XOR:
					if ( in1 != 2 && in2 != 2 )
					{
						f = in1 ^ in2;
					}
					else
					{
						f = 2;
					}
					break;
				case XNOR:
					if ( in1 != 2 && in2 != 2 )
					{
						f = !(in1 ^ in2);
					}
					else
					{
						f = 2;
					}
					break;
				default:
					break;
			}

			switch ( fpga->cells[ i ][ j ].n_out )
			{
				case F:
					fpga->cells[ i ][ j ].n_val = f;
					break;
				case EAST:
					fpga->cells[ i ][ j ].n_val = e_in;
					break;
				case SOUTH:
					fpga->cells[ i ][ j ].n_val = s_in;
					break;
				case WEST:
					fpga->cells[ i ][ j ].n_val = w_in;
					break;
				default:
					fpga->cells[ i ][ j ].n_val = 2;
					break;
			}

			switch ( fpga->cells[ i ][ j ].e_out )
			{
				case NORTH:
					fpga->cells[ i ][ j ].e_val = n_in;
					break;
				case F:
					fpga->cells[ i ][ j ].e_val = f;
					break;
				case SOUTH:
					fpga->cells[ i ][ j ].e_val = s_in;
					break;
				case WEST:
					fpga->cells[ i ][ j ].e_val = w_in;
					break;
				default:
					fpga->cells[ i ][ j ].e_val = 2;
					break;
			}

			switch ( fpga->cells[ i ][ j ].s_out )
			{
				case NORTH:
					fpga->cells[ i ][ j ].s_val = n_in;
					break;
				case EAST:
					fpga->cells[ i ][ j ].s_val = e_in;
					break;
				case F:
					fpga->cells[ i ][ j ].s_val = f;
					break;
				case WEST:
					fpga->cells[ i ][ j ].s_val = w_in;
					break;
				default:
					fpga->cells[ i ][ j ].s_val = 2;
					break;
			}

			switch ( fpga->cells[ i ][ j ].w_out )
			{
				case NORTH:
					fpga->cells[ i ][ j ].w_val = n_in;
					break;
				case EAST:
					fpga->cells[ i ][ j ].w_val = e_in;
					break;
				case SOUTH:
					fpga->cells[ i ][ j ].w_val = s_in;
					break;
				case F:
					fpga->cells[ i ][ j ].w_val = f;
					break;
				default:
					fpga->cells[ i ][ j ].w_val = 2;
					break;
			}
		}
	}
}

void evaluate_fpga ( FPGA *fpga )
{
	for ( int i = 0 ; i < FPGA_WIDTH * FPGA_HEIGHT ; i++ )
	{
		tick ( fpga );
		tock ( fpga );
	}
}
