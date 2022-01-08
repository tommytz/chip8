#include "chip8dis.h"

// int main(int argc, char **argv) {
//     // Open rom from command line arguments. Rom name has to be in quotation marks.
//     FILE *rom = fopen(argv[1], "rb"); // argv[1] is the rom name
//     if (rom == NULL) {
//         printf("Error: Couldn't open %s\n", argv[1]);
//         exit(EXIT_FAILURE);
//     }
//
//     // Get the rom size
//     fseek(rom, 0L, SEEK_END);
//     int fsize = ftell(rom);
//     fseek(rom, 0L, SEEK_SET);
//
//     // CHIP-8 programs are put in memory at 0x200
//     // Read the rom into memory at 0x200 and close it
//     unsigned char *buffer = malloc(fsize + 0x200);
//     fread(buffer + 0x200, fsize, 1, rom);
//     fclose(rom);
//
//     // Set the program counter to 0x200 and iterate over buffer
//     int pc = 0x200;
//     while (pc < fsize + 0x200) {
//         disassembleChip8Opcode(buffer, pc);
//         pc += 2;
//         printf("\n");
//     }
//     free(buffer);
//     return 0;
// }

void disassembleChip8Opcode(uint8_t *buffer, int pc) {
    // pc == 0x200 because all chip-8 programs start at 0x200 in RAM
    uint8_t *op = &buffer[pc];
    // after reading the first byte in the buffer we can grab all our values
    uint16_t opcode = CONCAT(op[0], op[1]);
    uint8_t instr = INSTR_MASK(opcode);
    uint8_t vx = VX_MASK(opcode);
    uint8_t vy = VY_MASK(opcode);
    uint8_t nn = NN_MASK(opcode);
    uint16_t nnn = NNN_MASK(opcode);
    uint8_t lsn = LSN_MASK(opcode); // least significant 4 bits of opcode

    printf("%04X %04X : ", pc, opcode);
    switch(instr) {
        case 0x0:
            switch(op[1]) {
                case 0xe0: printf("CLS"); break; // clear screen
                case 0xee: printf("RETURN"); break; // return from subroutine
                default: printf("UNKNOWN 0"); break; // usually jump to machine code routine at NNN
            } break;
        case 0x1: printf("JUMP $%03X", nnn); break;
        case 0x2: printf("CALL $%03X", nnn); break;
        case 0x3: printf("SKIP V%01X == %02X", vx, nn); break;
        case 0x4: printf("SKIP V%01X != %02X", vx, nn); break;
        case 0x5: printf("SKIP V%01X == V%01X", vx, vy); break;
        case 0x6: printf("V%01X = %02X", vx, nn); break;
        case 0x7: printf("V%01X += %02X", vx, nn); break;
        case 0x8:
            switch(lsn) {
                case 0: printf("V%01X = V%01X", vx, vy); break;
                case 1: printf("V%01X = V%01X OR V%01X", vx, vx, vy); break;
                case 2: printf("V%01X = V%01X AND V%01X", vx, vx, vy); break;
                case 3: printf("V%01X = V%01X XOR V%01X", vx, vx, vy); break;
                case 4: printf("V%01X += V%01X, VF FLAG", vx, vy); break;
                case 5: printf("V%01X -= V%01X, VF FLAG", vx, vy); break;
                case 6: printf("V%01X >>= 1, VF FLAG", vx); break; // right bit shift
                case 7: printf("V%01X = V%01X - V%01X, VF FLAG", vx, vy, vx); break;
                case 0xe: printf("V%01X <<= 1, VF FLAG", vx); break; // left bit shift
                default: printf("UNKNOWN 8"); break;
            } break;
        case 0x9: printf("SKIP V%01X != V%01X", vx, vy); break;
        case 0xa: printf("I = %03X", nnn); break;
        case 0xb: printf("JUMP $%03X + V0", nnn); break;
        case 0xc: printf("V%01X = RND AND %02X", vx, nn); break;
        case 0xd: printf("DRAW %01X (V%01X, V%01X), VF FLAG", lsn, vx, vy); break;
        case 0xe:
            switch(nn) {
                case 0x9e: printf("SKIP V%01X P", vx); break; // skip if key with value of VX is pressed
                case 0xa1: printf("SKIP V%01X NP", vx); break; // skip if key with value of VX is NOT pressed
                default: printf("UNKNOWN E"); break;
            } break;
        case 0xf:
            switch(op[1]) {
                case 0x07: printf("V%01X = DELAY", vx); break;
                case 0x0a: printf("V%01X = WAITKEY", vx); break;
                case 0x15: printf("DELAY = V%01X", vx); break;
                case 0x18: printf("SOUND = V%01X", vx); break;
                case 0x1e: printf("I += V%01X", vx); break;
                case 0x29: printf("I = SPRITE, V%01X", vx); break;
                case 0x33: printf("I,I+1,I+2 = BCD V%01X", vx); break;
                case 0x55: printf("MEM[I..I+%01X] = V[0..%01X]", vx, vx); break;
                case 0x65: printf("V[0..%01X] = MEM[I..I+%01X]", vx, vx); break;
                default: printf("UNKNOWN F"); break;
            } break;
    }
}
