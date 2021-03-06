#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 20
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
	uint8_t key_state[16];
    int     halt;
	int 	draw_flag;
} Chip8State;

/*
Keypad         Keyboard       Array map
+-+-+-+-+      +-+-+-+-+      +-+-+-+-+
|1|2|3|C|      |1|2|3|4|      |X|1|2|3|
+-+-+-+-+      +-+-+-+-+      +-+-+-+-+
|4|5|6|D|      |Q|W|E|R|      |Q|W|E|A|
+-+-+-+-+  =>  +-+-+-+-+  =>  +-+-+-+-+
|7|8|9|E|      |A|S|D|F|      |S|D|Z|C|
+-+-+-+-+      +-+-+-+-+      +-+-+-+-+
|A|0|B|F|      |Z|X|C|V|      |4|R|F|V|
+-+-+-+-+      +-+-+-+-+      +-+-+-+-+
*/

// int SDL_keymap[16] = {
//     SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
//     SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
// 	SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
//     SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V
// };

int SDL_keymap[16] = {
    SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
	SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
    SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V
};

// uint8_t Chip8_keymap[16] = {
//     0x1, 0x2, 0x3, 0xC,
//     0x4, 0x5, 0x6, 0xD,
// 	0x7, 0x8, 0x9, 0xE,
//     0xA, 0x0, 0xB, 0xF
// };

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
void terminate(void);
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
void opE(Chip8State *state, uint16_t opcode);
void opF(Chip8State *state, uint16_t opcode);
