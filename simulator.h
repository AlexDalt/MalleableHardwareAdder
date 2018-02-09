#define FPGA_HEIGHT 2
#define FPGA_WIDTH 2
#define STRING_LENGTH_BYTES FPGA_WIDTH * FPGA_HEIGHT * 2

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
	Direction n_out, e_out, s_out, w_out;
	Gate gate;
	Direction g_in1, g_in2;

	//3 values: 0, 1, 2 (2 represents undefined)
	unsigned char n_in, e_in, s_in, w_in;
	unsigned char n_val, e_val, s_val, w_val;
} Cell;

typedef struct {
	Cell cells[ FPGA_HEIGHT ][ FPGA_WIDTH ];
	unsigned char input[ FPGA_WIDTH ];
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

//void print_fpga ( FPGA *fpga );
