#include <stdio.h>
#include <ncurses.h>
#include <curses.h>
#include <math.h>

#define FPGA_HEIGHT 4
#define FPGA_WIDTH 4
#define STRING_LENGTH_BYTES FPGA_WIDTH * FPGA_HEIGHT * 2
#define FAULT_NUM 1
#define FAULT_TYPE_CON 0

typedef enum {
	OFF,
	NOT,
	OR,
	AND,
	NAND,
	NOR,
	XOR,
	XNOR
} Gate;

typedef enum {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	F
} Direction;

typedef struct {
	int x, y;
	Direction dir;
	unsigned char value;
} Fault;

typedef struct {
	Direction n_out, e_out, s_out, w_out;
	Gate gate;
	Direction g_in1, g_in2;

	//3 values: 0, 1, 2 (2 represents undefined)
	unsigned char n_in, e_in, s_in, w_in;
	unsigned char n_val, e_val, s_val, w_val;
} Cell;

typedef struct {
	Cell cells[ FPGA_HEIGHT ][ FPGA_WIDTH ];
	unsigned char control;
	unsigned char input[ FPGA_WIDTH ];

	Fault faults[ FAULT_NUM ];
	int active_fault[ FAULT_NUM ];
} FPGA;

/*
 * FPGA is defined by a bitstring of the following format:
 * 		- the bitstring defines cells from left to right, top to bottom, row by row, one byte per cell
 * 		- the least significant 2 bits define the value pushed to n_out (F,EAST,SOUTH,WEST)
 * 		- the next 2 define the value pushed to e_out (NORTH,F,SOUTH,WEST)
 * 		- the next 2 define the value pushed to s_out (NORTH,EAST,F,WEST)
 * 		- the next 2 define the value pushed to w_out (NORTH,EAST,SOUTH,F)
 * 		- the next 2 define where the first input for F comes from (NORTH,EAST,SOUTH,WEST)
 * 		- the next 2 define where the second input for F comes from (NORTH,EAST,SOUTH,WEST)
 * 		- the next 3 define the function F performs (OFF,NOT,OR,AND,NAND,NOR,XOR,XNOR)
 * 		- the most significant bit is reserved
 */

void bitstring_to_fpga ( FPGA *fpga, unsigned char *bits );

void evaluate_fpga ( FPGA *fpga );

void init_curses ();

void redraw ( int test_loop, int iteration, FPGA fpga, int most_fit, int mean_fit, int mean_div, int add_weight, int sub_weight );

void tidy_up_curses();
