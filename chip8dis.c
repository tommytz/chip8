#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void disassembleChip8Opcode(uint8_t *codebuffer, int pc);

int main(int argc, char **argv) {
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
    unsigned char *buffer = malloc(fsize + 0x200);
    fread(buffer + 0x200, fsize, 1, rom);
    fclose(rom);

    // Set the program counter to 0x200 and iterate over buffer
    int pc = 0x200;
    while (pc < fsize + 0x200) {
        disassembleChip8Opcode(buffer, pc);
        pc += 2;
        printf("\n");
    }
    free(buffer);
    return 0;
}

void disassembleChip8Opcode(uint8_t *codebuffer, int pc) {
    // pc == 0x200 because all chip-8 programs start at 0x200 in RAM
    uint8_t *opcode = &codebuffer[pc];
    // Most significant nibble. Designates instruction type.
    uint8_t instruction_type = (opcode[0] >> 4);
    printf("%04X %02X %02X -- ", pc, opcode[0], opcode[1]);
    switch(instruction_type) {
        case 0x00:
            switch(opcode[1]) {
                case 0xe0: printf("CLS"); break; // clear screen
                case 0xee: printf("RETURN"); break; // return from subroutine
                default: printf("UNKNOWN 0"); break; // usually jump to machine code routine at NNN
            } break;
        case 0x01: printf("JUMP %01X%02X", opcode[0]&0xf, opcode[1]); break;
        case 0x02: printf("CALL %01X%02X", opcode[0]&0xf, opcode[1]); break;
        case 0x03: printf("SKIP V%01X == %02X", opcode[0]&0xf, opcode[1]); break;
        case 0x04: printf("SKIP V%01X != %02X", opcode[0]&0xf, opcode[1]); break;
        case 0x05: printf("SKIP VX == VY"); break;
        case 0x06: printf("V%01X = %02X", opcode[0]&0xf, opcode[1]); break;
        case 0x07: printf("V%01X += %02X", opcode[0]&0xf, opcode[1]); break;
        case 0x08: {
            uint8_t lsNibble = (opcode[1]&0xf); // least significant nibble
            switch(lsNibble) {
                case 0: printf("V%01X = V%01X", opcode[0]&0xf, opcode[1]>>4); break;
                case 1: printf("V%01X = V%01X OR V%01X", opcode[0]&0xf, opcode[0]&0xf, opcode[1]>>4); break;
                case 2: printf("V%01X = V%01X AND V%01X", opcode[0]&0xf, opcode[0]&0xf, opcode[1]>>4); break;
                case 3: printf("V%01X = V%01X XOR V%01X", opcode[0]&0xf, opcode[0]&0xf, opcode[1]>>4); break;
                case 4: printf("V%01X += V%01X, VF FLAG", opcode[0]&0xf, opcode[1]>>4); break;
                case 5: printf("V%01X -= V%01X, VF FLAG", opcode[0]&0xf, opcode[1]>>4); break;
                case 6: printf("V%01X >>= 1, VF FLAG", opcode[0]&0xf); break; // right bit shift
                case 7: printf("V%01X = V%01X - V%01X, VF FLAG", opcode[0]&0xf, opcode[1]>>4, opcode[0]&0xf); break;
                case 0xe: printf("V%01X <<= 1, VF FLAG", opcode[0]&0xf); break; // left bit shift
                default: printf("UNKNOWN 8"); break;
            }
        } break;
        case 0x09: printf("SKIP VX != VY"); break;
        case 0x0a: printf("I = %01X%02X", opcode[0]&0xf, opcode[1]); break;
        case 0x0b: printf("JUMP %01X%02X + V0", opcode[0]&0xf, opcode[1]); break;
        case 0x0c: printf("V%01X = RND AND %02X", opcode[0]&0xf, opcode[1]); break;
        case 0x0d: printf("DRAW %01X (V%01X, V%01X), VF FLAG", opcode[1]&0xf, opcode[0]&0xf, opcode[1]>>4); break;
        case 0x0e:
            switch(opcode[1]) {
                case 0x9e: printf("SKIP V%01X P", opcode[0]&0xf); break; // skip if key with value of VX is pressed
                case 0xa1: printf("SKIP V%01X NP", opcode[0]&0xf); break; // skip if key with value of VX is NOT pressed
                default: printf("UNKNOWN E"); break;
            } break;
        case 0x0f:
            switch(opcode[1]) {
                case 0x07: printf("V%01X = DELAY", opcode[0]&0xf); break;
                case 0x0a: printf("V%01X = WAITKEY", opcode[0]&0xf); break;
                case 0x15: printf("DELAY = V%01X", opcode[0]&0xf); break;
                case 0x18: printf("SOUND = V%01X", opcode[0]&0xf); break;
                case 0x1e: printf("I += V%01X", opcode[0]&0xf); break;
                case 0x29: printf("I = SPRITE, V%01X", opcode[0]&0xf); break;
                case 0x33: printf("I,I+1,I+2 = BCD V%01X", opcode[0]&0xf); break;
                case 0x55: printf("MEM[I..I+%01X] = V[0..%01X]", opcode[0]&0xf, opcode[0]&0xf); break;
                case 0x65: printf("V[0..%01X] = MEM[I..I+%01X]", opcode[0]&0xf, opcode[0]&0xf); break;
                default: printf("UNKNOWN F"); break;
            } break;
    }
}
