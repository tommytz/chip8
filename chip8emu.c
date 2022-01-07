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

typedef struct Chip8State {
    uint8_t V[16]; // Registers V0 to VF
    uint16_t I; // Index register
    uint16_t SP; // Stack pointer
    uint16_t PC; // Program counter
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t *memory;
} Chip8State;

void displayState(Chip8State *state) {
    printf("V0: %02X, V1: %02X, V2: %02X, V3: %02X, V4 %02X, V5: %02X, V6: %02X, V7: %02X\n",
    state->V[0], state->V[1], state->V[2], state->V[3], state->V[4],
    state->V[5], state->V[6], state->V[7]);
    printf("V8: %02X, V9: %02X, VA: %02X, VB: %02X, VC: %02X, VD: %02X, VE %02X, VF: %02X\n",
    state->V[8], state->V[9], state->V[0xa], state->V[0xb], state->V[0xc],
    state->V[0xd], state->V[0xe], state->V[0xf]);
    printf("I: x, PC: x")
}

void unimplementedInstruction() {
    printf("Unimplemented Instruction\n");
}

Chip8State* initChip8(void) {
    Chip8State *state = calloc(sizeof(Chip8State), 1);
    state->memory = calloc(4096, 1);
    state->PC = 0x200;

    return state;
}

void emulateChip8Op(Chip8State *state) {
    uint8_t *op = &state->memory[state->PC];

    uint16_t opcode = CONCAT(op[0], op[1]); // get 16 bit opcode
    uint8_t instr = INSTR_MASK(opcode);
    uint8_t vx = VX_MASK(opcode);
    uint8_t vy = VY_MASK(opcode);
    uint8_t nn = NN_MASK(opcode);
    uint16_t nnn = NNN_MASK(opcode);
    uint8_t lsn = LSN_MASK(opcode); // least significant 4 bits of opcode

    switch (lsn) {
        case 0x0: unimplementedInstruction(); break;
        case 0x1: unimplementedInstruction(); break;
        case 0x2: unimplementedInstruction(); break;
        case 0x3: unimplementedInstruction(); break;
        case 0x4: unimplementedInstruction(); break;
        case 0x5: unimplementedInstruction(); break;
        case 0x6: unimplementedInstruction(); break;
        case 0x7: unimplementedInstruction(); break;
        case 0x8: unimplementedInstruction(); break;
        case 0x9: unimplementedInstruction(); break;
        case 0xa: unimplementedInstruction(); break;
        case 0xb: unimplementedInstruction(); break;
        case 0xc: unimplementedInstruction(); break;
        case 0xd: unimplementedInstruction(); break;
        case 0xe: unimplementedInstruction(); break;
        case 0xf: unimplementedInstruction(); break;
    }
    displayState(state);
}
