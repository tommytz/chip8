#include "chip8emu.h"
#include "chip8dis.h"

int main(int argc, char **argv) {
    Chip8State* c8 = InitChip8();

    // Open rom from command line arguments. Rom name has to be in quotation marks.
    FILE *rom = fopen(argv[1], "rb"); // argv[1] is the rom name
    if (rom == NULL) {
        printf("Error: Couldn't open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Get the rom size
    fseek(rom, 0L, SEEK_END);
    int fsize = ftell(rom);
    fseek(rom, 0L, SEEK_SET);

    // CHIP-8 programs are put in memory at 0x200
    // Read the rom into memory at 0x200 and close it
    fread(&c8->memory[0x200], fsize, 1, rom);
    fclose(rom);

    // Set the program counter to 0x200 and iterate over buffer
    while (c8->pc < (fsize + 0x200)) {
        EmulateChip8Op(c8);
        c8->pc += 2;
        printf("\n");
    }
    return 0;
}

Chip8State* InitChip8(void)
{
	Chip8State* state = calloc(sizeof(Chip8State), 1);

	state->memory = calloc(1024*4, 1);
	state->sp = 0xfa0;
	state->pc = 0x200;

    return state;
}

void UnimplementedInstruction(Chip8State* state)
{
	//pc will have advanced one, so undo that
	printf ("Error: Unimplemented instruction\n");
}

void EmulateChip8Op(Chip8State *state)
{
    uint8_t *op = &state->memory[state->pc];
    disassembleChip8Opcode(state->memory, state->pc);
    printf("\n");
    int highnib = (*op & 0xf0) >> 4;
    switch (highnib)
    {
        case 0x00: UnimplementedInstruction(state); break;
        case 0x01: UnimplementedInstruction(state); break;
        case 0x02: UnimplementedInstruction(state); break;
        case 0x03: UnimplementedInstruction(state); break;
        case 0x04: UnimplementedInstruction(state); break;
        case 0x05: UnimplementedInstruction(state); break;
        case 0x06: UnimplementedInstruction(state); break;
        case 0x07: UnimplementedInstruction(state); break;
        case 0x08: UnimplementedInstruction(state); break;
        case 0x09: UnimplementedInstruction(state); break;
        case 0x0a: UnimplementedInstruction(state); break;
        case 0x0b: UnimplementedInstruction(state); break;
        case 0x0c: UnimplementedInstruction(state); break;
        case 0x0d: UnimplementedInstruction(state); break;
        case 0x0e: UnimplementedInstruction(state); break;
        case 0x0f: UnimplementedInstruction(state); break;
    }
}
