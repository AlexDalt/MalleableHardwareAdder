#define FPGA_HEIGHT 4
#define FPGA_WIDTH 4
#define STRING_LENGTH_BYTES (FPGA_WIDTH * FPGA_HEIGHT)
#define CLOCK_CYCLES 64

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
	SOUTH,
	EAST,
	WEST
} Direction;

typedef struct {
	Direction in1_d, in2_d;
	Gate gate;
	unsigned char in1, in2;
	unsigned char out; //3 values: 0, 1, 2 (2 represents undefined)
} Cell;

typedef struct {
	Cell cells[ FPGA_HEIGHT ][ FPGA_WIDTH ];
	unsigned char input[ FPGA_WIDTH ];
} FPGA;

/*
 * FPGA is defined by a bitstring of the following format:
 * 		- the bitstring defines cells from left to right, top to bottom, row by row, one byte per cell
 * 		- the least significant 3 bits are the gate (OFF,NOT,OR,AND,NAND,NOR,XOR,XNOR)
 * 		- the next 2 bits define in1 (NORTH,SOUTH,EAST,WEST)
 * 		- the next 2 bits define in2 (NORTH,SOUTH,EAST,WEST)
 * 		- the final bit is reserved
 */

void bitstring_to_fpga ( FPGA *fpga, unsigned char *bits );

void evaluate_fpga ( FPGA *fpga );

void print_fpga ( FPGA *fpga );
