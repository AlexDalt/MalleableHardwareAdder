#include "simulator.h"
WINDOW *add_win, *fpga_win, *sub_win;
int row,col;

WINDOW *create_win( int height, int width, int starty, int startx )
{
	WINDOW *win;
	win = newwin( height, width, starty, startx );
	box( win, 0, 0 );
	wrefresh( win );

	return win;
}

void delete_win( WINDOW* win )
{
	wborder( win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' );
	wrefresh( win );
	delwin( win );
}

WINDOW *create_subwin( WINDOW *orig, int height, int width, int starty, int startx )
{
	WINDOW *win;
	win = subwin( orig, height, width, starty, startx );
	box( win, 0, 0 );
	wrefresh( win );
	wrefresh( orig );

	return win;
}

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

			if ( j == 0 &&  i == 0 )
			{
				fpga->cells[ i ][ j ].w_in = fpga->control;
			}
			else if ( j == 0 )
			{
				fpga->cells[ i ][ j ].w_in = 2;
			}
			else
			{
				fpga->cells[ i ][ j ].w_in = fpga->cells[ i ][ j - 1 ].e_val;
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

	if ( FAULT_TYPE_CON )
	{
		for ( int i = 0 ; i < FAULT_NUM ; i++ )
		{
			if ( fpga->active_fault[ i ] )
			{
				Fault f = fpga->faults[ i ];
				switch ( f.dir )
				{
					case NORTH:
						fpga->cells[ f.y ][ f.x ].n_val = f.value;
						break;
					case EAST:
						fpga->cells[ f.y ][ f.x ].e_val = f.value;
						break;
					case SOUTH:
						fpga->cells[ f.y ][ f.x ].s_val = f.value;
						break;
					case WEST:
						fpga->cells[ f.y ][ f.x ].w_val = f.value;
						break;
					default:
						break;
				}
			}
		}
	}
	else
	{
		for ( int i = 0 ; i < FAULT_NUM ; i++ )
		{
			if ( fpga->active_fault[ i ] )
			{
				Fault f = fpga->faults[ i ];
				Cell c = fpga->cells[ f.y ][ f.x ];
				if ( c.n_out == F )
				{
					fpga->cells[ f.y ][ f.x ].n_val = f.value;
				}
				if ( c.e_out == F )
				{
					fpga->cells[ f.y ][ f.x ].e_val = f.value;
				}
				if ( c.s_out == F )
				{
					fpga->cells[ f.y ][ f.x ].s_val = f.value;
				}
				if ( c.w_out == F )
				{
					fpga->cells[ f.y ][ f.x ].w_val = f.value;
				}
			}
		}
	}
}

void evaluate_fpga ( FPGA *fpga )
{
	int changed = 1;
	int iteration = 0;
	unsigned char output[ FPGA_WIDTH ];

	for ( int i = 0 ; i < FPGA_WIDTH ; i++ )
	{
		output[ i ] = 3;
	}

	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			fpga->cells[ i ][ j ].n_val = 2;
			fpga->cells[ i ][ j ].e_val = 2;
			fpga->cells[ i ][ j ].w_val = 2;
			fpga->cells[ i ][ j ].s_val = 2;
		}
	}

	while ( changed )
	{
		tick ( fpga );
		tock ( fpga );

		changed = 0;
		for ( int i = 0 ; i < FPGA_WIDTH ; i++ )
		{
			if ( output[ i ] != fpga->cells[ FPGA_HEIGHT - 1 ][ i ].s_val
					|| fpga->cells[ FPGA_HEIGHT - 1 ][ i ].s_val == 2 )
			{
				changed = 1;
			}
			output[ i ] = fpga->cells[ FPGA_HEIGHT - 1 ][ i ].s_val;
		}

		iteration++;

		if ( iteration == 64 )
		{
			break;
		}
	}
}

void init_curses ()
{
	initscr();
	start_color();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	curs_set( 0 );
	getmaxyx( stdscr, row, col );

	refresh();

	init_pair( 1, COLOR_BLACK, COLOR_RED );
	init_pair( 2, COLOR_BLACK, COLOR_GREEN );

	add_win = create_win( row, col/4, 0, 0 );
	fpga_win = create_win( row, 2*col/4, 0, col/4 );
	sub_win = create_win( row, col/4, 0, 3*col/4 );

	refresh();
}

void redraw_add_win( FPGA fpga, int add_weight )
{
	int maxx, maxy;
	getmaxyx( add_win, maxy, maxx );
	werase( add_win );

	int num_values = pow( 2, FPGA_WIDTH );
	int total_correct = 0;

	for ( int i = 0 ; i < num_values ; i++ )
	{
		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;
		int v1 = i & mask;
		int v2 = ( i >> FPGA_WIDTH/2 ) & mask;

		for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
		{
			fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
			fpga.input[ j * 2 + 1 ] = (v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
		}

		fpga.control = 0;
		evaluate_fpga( &fpga );

		int correct = 1;
		int num_correct = FPGA_WIDTH/2 + 1;
		int sum = v1 + v2;

		for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val != (( sum >> j ) & 1 ) )
			{
				correct = 0;
				num_correct--;
			}
		}

		if ( correct )
		{
			wattron( add_win, COLOR_PAIR( 2 ) );
			mvwprintw( add_win, (maxy-num_values)/2 + i, (maxx - 24)/2, "%d + %d : %d/%d bits correct", v1, v2, num_correct, FPGA_WIDTH/2 + 1 );
			wattroff( add_win, COLOR_PAIR( 2 ) );
			total_correct++;
		}
		else if ( num_correct == 0 )
		{
			wattron( add_win, COLOR_PAIR( 1 ) );
			mvwprintw( add_win, (maxy-num_values)/2 + i, (maxx - 24)/2, "%d + %d : %d/%d bits correct", v1, v2, num_correct, FPGA_WIDTH/2 + 1 );
			wattroff( add_win, COLOR_PAIR( 1 ) );
		}
		else
		{
			mvwprintw( add_win, (maxy-num_values)/2 + i, (maxx - 24)/2, "%d + %d : %d/%d bits correct", v1, v2, num_correct, FPGA_WIDTH/2 + 1 );
		}
	}

	box( add_win, 0, 0 );
	mvwprintw ( add_win, maxy-1, (maxx-17)/2, "total correct=%2d", total_correct );
	mvwprintw ( add_win, 0, (maxx-13)/2, "ADD weight=%d", add_weight );
	wrefresh( add_win );
}

void redraw_fpga_win ( int iteration, FPGA fpga, int most_fit, int mean_fit, int mean_div, int add_weight, int sub_weight )
{
	int maxx, maxy;
	getmaxyx( fpga_win, maxy, maxx );
	werase( fpga_win );

	int cell_x = maxx/(FPGA_WIDTH + 2);
	int cell_y = maxy/(FPGA_HEIGHT + 2);

	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			switch ( fpga.cells[ i ][ j ].gate )
			{
				case OFF:
					break;
				case NOT:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 3)/2, "NOT" );
					break;
				case OR:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 2)/2, "OR" );
					break;
				case AND:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 3)/2, "AND" );
					break;
				case NAND:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 4)/2, "NAND" );
					break;
				case NOR:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 3)/2, "NOR" );
					break;
				case XOR:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 3)/2, "XOR" );
					break;
				case XNOR:
					mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2 + 1, cell_x * ( j + 1 ) + (cell_x - 4)/2, "XNOR" );
					break;
			}
		}
	}

	for ( int i = 0 ; i < FPGA_WIDTH + 1 ; i++ )
	{
		mvwvline( fpga_win, cell_y, cell_x * ( i + 1 ), ACS_VLINE, cell_y * FPGA_HEIGHT );
	}
	for ( int i = 0 ; i < FPGA_HEIGHT + 1 ; i++ )
	{
		mvwhline( fpga_win, cell_y * (i + 1), cell_x, ACS_HLINE, cell_x * FPGA_WIDTH + 1 );
	}

	for ( int i = 0 ; i < FPGA_HEIGHT ; i++ )
	{
		for ( int j = 0 ; j < FPGA_WIDTH ; j++ )
		{
			if ( i != 0 )
			{
				mvwprintw( fpga_win, cell_y * (i + 1), cell_x * (j + 1) + cell_x/2, "X" );
				switch ( fpga.cells[ i ][ j ].n_out )
				{
					case NORTH:
						mvwprintw( fpga_win, cell_y * (i + 1) + 1, cell_x * (j + 1) + cell_x/2, "N" );
						break;
					case EAST:
						mvwprintw( fpga_win, cell_y * (i + 1) + 1, cell_x * (j + 1) + cell_x/2, "E" );
						break;
					case SOUTH:
						mvwprintw( fpga_win, cell_y * (i + 1) + 1, cell_x * (j + 1) + cell_x/2, "S" );
						break;
					case WEST:
						mvwprintw( fpga_win, cell_y * (i + 1) + 1, cell_x * (j + 1) + cell_x/2, "W" );
						break;
					case F:
						mvwprintw( fpga_win, cell_y * (i + 1) + 1, cell_x * (j + 1) + cell_x/2, "F" );
						break;
				}
			}
			if ( j != 0 )
			{
				mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 1), "X" );
				switch ( fpga.cells[ i ][ j ].w_out )
				{
					case NORTH:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 1) + 1, "N" );
						break;
					case EAST:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 1) + 1, "E" );
						break;
					case SOUTH:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 1) + 1, "S" );
						break;
					case WEST:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 1) + 1, "W" );
						break;
					case F:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 1) + 1, "F" );
						break;
				}
			}
			if ( i != FPGA_HEIGHT - 1 || j >= FPGA_WIDTH/2 - 1 )
			{
				switch ( fpga.cells[ i ][ j ].s_out )
				{
					case NORTH:
						mvwprintw( fpga_win, cell_y * (i + 2) - 1, cell_x * (j + 1) + cell_x/2, "N" );
						break;
					case EAST:
						mvwprintw( fpga_win, cell_y * (i + 2) - 1, cell_x * (j + 1) + cell_x/2, "E" );
						break;
					case SOUTH:
						mvwprintw( fpga_win, cell_y * (i + 2) - 1, cell_x * (j + 1) + cell_x/2, "S" );
						break;
					case WEST:
						mvwprintw( fpga_win, cell_y * (i + 2) - 1, cell_x * (j + 1) + cell_x/2, "W" );
						break;
					case F:
						mvwprintw( fpga_win, cell_y * (i + 2) - 1, cell_x * (j + 1) + cell_x/2, "F" );
						break;
				}
			}
			if ( j != FPGA_WIDTH - 1 )
			{
				switch ( fpga.cells[ i ][ j ].e_out )
				{
					case NORTH:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 2) - 1, "N" );
						break;
					case EAST:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 2) - 1, "E" );
						break;
					case SOUTH:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 2) - 1, "S" );
						break;
					case WEST:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 2) - 1, "W" );
						break;
					case F:
						mvwprintw( fpga_win, cell_y * (i + 1) + cell_y/2, cell_x * (j + 2) - 1, "F" );
						break;
				}
			}

			if ( fpga.cells[ i ][ j ].gate != OFF )
			{
				switch ( fpga.cells[ i ][ j ].g_in1 )
				{
					case NORTH:
						mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 - 1, "N" );
						break;
					case EAST:
						mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 - 1, "E" );
						break;
					case SOUTH:
						mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 - 1, "S" );
						break;
					case WEST:
						mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 - 1, "W" );
						break;
					case F:
						mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 - 1, "F" );
						break;
				}
				if ( fpga.cells[ i ][ j ].gate != NOT )
				{
					switch ( fpga.cells[ i ][ j ].g_in2 )
					{
						case NORTH:
							mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 + 1, "N" );
							break;
						case EAST:
							mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 + 1, "E" );
							break;
						case SOUTH:
							mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 + 1, "S" );
							break;
						case WEST:
							mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 + 1, "W" );
							break;
						case F:
							mvwprintw( fpga_win, cell_y * ( i + 1 ) + cell_y/2, cell_x * ( j + 1 ) + cell_x/2 + 1, "F" );
							break;
					}
				}
			}
		}
	}

	int y, x;

	if ( FAULT_TYPE_CON )
	{
		for ( int i = 0 ; i < FAULT_NUM ; i++ )
		{
			Fault f = fpga.faults[ i ];
			if ( fpga.active_fault[ i ] )
			{
				wattron( fpga_win, COLOR_PAIR( 1 ) );
				switch ( fpga.faults[ i ].dir )
				{
					case NORTH:
						y = cell_y * (f.y + 1) + 1;
						x = cell_x * (f.x + 1) + cell_x/2;
						break;
					case EAST:
						y = cell_y * (f.y + 1) + cell_y/2;
						x = cell_x * (f.x + 2) - 1;
						break;
					case SOUTH:
						y = cell_y * (f.y + 2) - 1;
						x = cell_x * (f.x + 1) + cell_x/2;
						break;
					case WEST:
						y = cell_y * (f.y + 1) + cell_y/2;
						x = cell_x * (f.x + 1) + 1;
						break;
					default:
						break;
				}
				mvwprintw( fpga_win, y, x, "%d", f.value );
				wattroff( fpga_win, COLOR_PAIR( 1 ) );
			}
		}
	}
	else
	{
		for ( int i = 0 ; i < FAULT_NUM ; i++ )
		{
			Fault f = fpga.faults[ i ];
			if ( fpga.active_fault[ i ] )
			{
				wattron( fpga_win, COLOR_PAIR( 1 ) );
				mvwprintw( fpga_win, cell_y * ( f.y + 1 ) + cell_y/2 + 1, cell_x * ( f.x + 1 ) + (cell_x - 5)/2, "  %d  ", f.value );
				wattroff( fpga_win, COLOR_PAIR( 1 ) );
			}
		}
	}

	mvwprintw( fpga_win, cell_y + cell_y/2, cell_x, ">" );
	mvwprintw( fpga_win, cell_y + cell_y/2, cell_x - 1, "-" );
	mvwprintw( fpga_win, cell_y + cell_y/2, cell_x - 8, "ADD/SUB" );

	for ( int i = 0 ; i < FPGA_WIDTH ; i++ )
	{
		if ( i % 2 == 0 )
		{
			mvwprintw( fpga_win, cell_y - 2, cell_x * ( i + 1 ) + (cell_x-3)/2, "y_%d", FPGA_WIDTH/2 - i/2 - 1);
		}
		else
		{
			mvwprintw( fpga_win, cell_y - 2, cell_x * ( i + 1 ) + (cell_x-3)/2, "x_%d", FPGA_WIDTH/2 - i/2 - 1);
		}
		mvwprintw( fpga_win, cell_y - 1, cell_x * ( i + 1 ) + cell_x/2, "|" );
		mvwprintw( fpga_win, cell_y, cell_x * ( i + 1 ) + cell_x/2, "V" );
		if ( i >= FPGA_WIDTH/2 - 1 )
		{
			mvwprintw( fpga_win, cell_y * (FPGA_HEIGHT + 1), cell_x * ( i + 1 ) + cell_x/2, "|" );
			mvwprintw( fpga_win, cell_y * (FPGA_HEIGHT + 1) + 1, cell_x * ( i + 1 ) + cell_x/2, "V" );
			if ( i >= FPGA_WIDTH/2 )
			{
			mvwprintw( fpga_win, cell_y * (FPGA_HEIGHT + 1) + 2, cell_x * ( i + 1 ) + (cell_x - 3)/2, "b_%d", FPGA_WIDTH - i - 1 );
			}
			else
			{
				mvwprintw( fpga_win, cell_y * (FPGA_HEIGHT + 1) + 2, cell_x * ( i + 1 ) + cell_x/2, "c" );
			}
		}
	}

	box( fpga_win, 0, 0 );
	float ratio = (float)add_weight / (float)(add_weight + sub_weight);
	mvwprintw( fpga_win, cell_y/2, cell_x - 1, "[" );
	mvwprintw( fpga_win, cell_y/2, cell_x + (cell_x * FPGA_WIDTH), "]" );
	mvwprintw( fpga_win, cell_y/2 - 1, (maxx - 15)/2, "ADD/SUB weights" );
	for( int i = 0 ; i < FPGA_WIDTH * cell_x ; i++ )
	{
		float check = (float)i / (float)(FPGA_WIDTH * cell_x);
		if ( check < ratio )
		{
			mvwprintw( fpga_win, cell_y/2, cell_x + i, "+" );
		}
		else
		{
			mvwprintw( fpga_win, cell_y/2, cell_x + i, "-" );
		}
	}

	mvwprintw( fpga_win, maxy-2, (maxx - 15)/2, "best fitness %2d", most_fit );
	mvwprintw( fpga_win, maxy-1, (maxx - 51)/2, "iteration %4d, mean fitness %3d, mean_diversity %2d", iteration, mean_fit, mean_div );
	wrefresh( fpga_win );
}

void redraw_sub_win( FPGA fpga, int sub_weight )
{
	int maxx, maxy;
	getmaxyx( sub_win, maxy, maxx );
	werase( sub_win );

	int num_values = pow( 2, FPGA_WIDTH );
	int total_correct = 0;

	for ( int i = 0 ; i < num_values ; i++ )
	{
		int mask = ( 1 << FPGA_WIDTH/2 ) - 1;
		int v1 = i & mask;
		int v2 = ( i >> FPGA_WIDTH/2 ) & mask;

		for ( int j = 0 ; j < FPGA_WIDTH/2 ; j++ )
		{
			fpga.input[ j * 2 ] = ( v1 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
			fpga.input[ j * 2 + 1 ] = (v2 >> (FPGA_WIDTH/2 - j - 1) ) & 1;
		}

		fpga.control = 1;
		evaluate_fpga( &fpga );

		int correct = 1;
		int num_correct = FPGA_WIDTH/2 + 1;
		int dif = v1 - v2;

		for ( int j = 0 ; j < FPGA_WIDTH/2 + 1 ; j++ )
		{
			if ( fpga.cells[ FPGA_HEIGHT - 1 ][ FPGA_WIDTH - j - 1 ].s_val != (( dif >> j ) & 1 ) )
			{
				correct = 0;
				num_correct--;
			}
		}

		if ( correct )
		{
			wattron( sub_win, COLOR_PAIR( 2 ) );
			mvwprintw( sub_win, (maxy-num_values)/2 + i, (maxx - 24)/2, "%d - %d : %d/%d bits correct", v1, v2, num_correct, FPGA_WIDTH/2 + 1 );
			wattroff( sub_win, COLOR_PAIR( 2 ) );
			total_correct++;
		}
		else if ( num_correct == 0 )
		{
			wattron( sub_win, COLOR_PAIR( 1 ) );
			mvwprintw( sub_win, (maxy-num_values)/2 + i, (maxx - 24)/2, "%d - %d : %d/%d bits correct", v1, v2, num_correct, FPGA_WIDTH/2 + 1 );
			wattroff( sub_win, COLOR_PAIR( 1 ) );
		}
		else
		{
			mvwprintw( sub_win, (maxy-num_values)/2 + i, (maxx - 24)/2, "%d - %d : %d/%d bits correct", v1, v2, num_correct, FPGA_WIDTH/2 + 1 );
		}
	}

	box( sub_win, 0, 0 );
	mvwprintw ( sub_win, maxy-1, (maxx-17)/2, "total correct=%2d", total_correct );
	mvwprintw ( sub_win, 0, (maxx-13)/2, "SUB weight=%d", sub_weight );
	wrefresh( sub_win );
}

void redraw ( int iteration, FPGA fpga, int most_fit, int mean_fit, int mean_div, int add_weight, int sub_weight )
{
	redraw_add_win( fpga, add_weight );
	redraw_fpga_win( iteration, fpga, most_fit, mean_fit, mean_div, add_weight, sub_weight );
	redraw_sub_win( fpga, sub_weight );
	refresh();
}

void tidy_up_curses ()
{
	delete_win( add_win );
	endwin();
}
