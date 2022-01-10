#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10
#define ON 0xFFFFFFFF
#define OFF 0x00000000

typedef struct Chip8State {
	uint8_t    V[16]; // The 16 variable 8-bit registers V0..VF
	uint16_t   I, SP, PC; // index register, stack pointer, and program counter
	uint8_t	   delay, sound;
	uint8_t	   *memory;
    uint32_t    *gfx;
    int     halt;
	int 	draw_flag;
} Chip8State;

Chip8State* InitChip8(void);
void InitDisplay(void);
void terminate(int exit_code);
void updateDisplay(Chip8State* chip8);
void UnimplementedInstruction(Chip8State* state);
void EmulateChip8Op(Chip8State *state);
void displayState(Chip8State *state);

void op0(Chip8State *state, uint16_t opcode);
void op1(Chip8State *state, uint16_t opcode);
// void op2(Chip8State *state, uint16_t opcode);
void op3(Chip8State *state, uint16_t opcode);
void op4(Chip8State *state, uint16_t opcode);
void op5(Chip8State *state, uint16_t opcode);
void op6(Chip8State *state, uint16_t opcode);
void op7(Chip8State *state, uint16_t opcode);
// void op8(Chip8State *state, uint16_t opcode);
void op9(Chip8State *state, uint16_t opcode);
void opA(Chip8State *state, uint16_t opcode);
void opB(Chip8State *state, uint16_t opcode);
void opC(Chip8State *state, uint16_t opcode);
void opD(Chip8State *state, uint16_t opcode);
// void opE(Chip8State *state, uint16_t opcode);
// void opF(Chip8State *state, uint16_t opcode);
