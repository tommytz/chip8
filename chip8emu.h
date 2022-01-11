#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 15
#define ON 0xFFFFFFFF
#define OFF 0x00000000
#define FONT_ADDRESS 0x50
#define FONT_SIZE 80

typedef struct Chip8State {
	uint8_t    V[16]; // The 16 variable 8-bit registers V0..VF
	uint16_t   I, PC; // The index register and program counter
	uint16_t stack[16];
	uint8_t SP; // The stack pointer
	uint8_t	   delay, sound;
	uint8_t	   *memory;
    uint32_t    *gfx;
    int     halt;
	int 	draw_flag;
} Chip8State;

uint8_t fontset[FONT_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


Chip8State* InitChip8(void);
void InitDisplay(void);
void terminate(int exit_code);
void updateDisplay(Chip8State* chip8);
void UnimplementedInstruction(Chip8State* state);
void EmulateChip8Op(Chip8State *state);
void displayState(Chip8State *state);

void op0(Chip8State *state, uint16_t opcode);
void op1(Chip8State *state, uint16_t opcode);
void op2(Chip8State *state, uint16_t opcode);
void op3(Chip8State *state, uint16_t opcode);
void op4(Chip8State *state, uint16_t opcode);
void op5(Chip8State *state, uint16_t opcode);
void op6(Chip8State *state, uint16_t opcode);
void op7(Chip8State *state, uint16_t opcode);
void op8(Chip8State *state, uint16_t opcode);
void op9(Chip8State *state, uint16_t opcode);
void opA(Chip8State *state, uint16_t opcode);
void opB(Chip8State *state, uint16_t opcode);
void opC(Chip8State *state, uint16_t opcode);
void opD(Chip8State *state, uint16_t opcode);
// void opE(Chip8State *state, uint16_t opcode);
void opF(Chip8State *state, uint16_t opcode);
