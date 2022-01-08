#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Chip8State {
	uint8_t		v[16];
	uint16_t	i, sp, pc; // index register, stack pointer, and program counter
	uint8_t		delay, sound;
	uint8_t		*memory;
} Chip8State;

Chip8State* InitChip8(void);
void UnimplementedInstruction(Chip8State* state);
void EmulateChip8Op(Chip8State *state);
