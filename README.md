# Malleable Hardware Adder
Using evolutionary algorithms to generate and explore FPGA configurations to solve problems - in this case addition and subtraction.

## Getting Started
These instructions will get you a copy of the project up and running on your local machine.

### Prerequisites
The GUI uses ncurses. After this is installed

```
make
```
should compile everything. Run with
```
./evolve
```

### Configuration
The parameters for evolution can be modified at the top of evolve.c

The FPGA simulation parameters can be modified at the top of simulation.h (FPGA width/height etc). The scope of the addition/subtraction is defined by the width of the FPGA; the inputs span the width, for example a width of 4 corresponds with 2-bit addition.
