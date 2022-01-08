#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Macros to extract the opcode values
#define CONCAT(x, y) ((x << 8) | y)
#define INSTR_MASK(x) (x >> 12)
#define VX_MASK(x) ((x & 0x0f00) >> 8)
#define VY_MASK(x) ((x & 0x00f0) >> 4)
#define NN_MASK(x) (x & 0x00ff)
#define NNN_MASK(x) (x & 0x0fff)
#define LSN_MASK(x) (x & 0x000f)

void disassembleChip8Opcode(uint8_t *buffer, int pc);
