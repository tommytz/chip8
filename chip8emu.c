#include "chip8emu.h"
#include "chip8dis.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
int pitch = 64*4;

int main(int argc, char **argv) {
    Chip8State* c8 = InitChip8();
    InitDisplay();
    updateDisplay(c8);

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
    while (c8->PC < (fsize + 0x200)) {
        EmulateChip8Op(c8);
        displayState(c8);
        printf("\n");
        if (c8->halt) {
            terminate(EXIT_SUCCESS);
            break;
        }
        if(c8->draw_flag){
            updateDisplay(c8);
            c8->draw_flag = 0;
        }
        SDL_Delay(250);
    }
    return 0;
    free(c8->memory);
    free(c8->gfx);
    free(c8);
    terminate(EXIT_SUCCESS);
}

Chip8State* InitChip8(void)
{
	Chip8State* state = calloc(sizeof(Chip8State), 1);

	state->memory = calloc(1024*4, 1);
    state->gfx = calloc(64*32, sizeof(uint32_t));
	state->SP = 0xfa0;
	state->PC = 0x200;
    state->halt = 0;
    state->draw_flag = 0;

    return state;
}

void InitDisplay(void) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error: Failed to initialize SDL: %s\n", SDL_GetError());
        terminate(EXIT_FAILURE);
    }
    window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE, SDL_WINDOW_SHOWN);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // 64x32 pixels, regardless of window size. Pixels will be stretched to fit.
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

        if (!window || !renderer || !texture) {
            printf("Error: failed to initialise video: %s\n", SDL_GetError());
            terminate(EXIT_FAILURE);
        }
}

void terminate(int exit_code) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(exit_code);
}

void updateDisplay(Chip8State* chip8) {
    SDL_UpdateTexture(texture, NULL, chip8->gfx, pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void UnimplementedInstruction(Chip8State* state)
{
	//pc will have advanced one, so undo that
	printf ("Error: Unimplemented instruction\n");
}

void EmulateChip8Op(Chip8State *state)
{
    uint8_t *op = &state->memory[state->PC];
    disassembleChip8Opcode(state->memory, state->PC);
    state->PC += 2; // increment after fetching
    printf("\n");
    // int highnib = (*op & 0xf0) >> 4;
    uint16_t opcode = CONCAT(op[0], op[1]);
    uint8_t instr = INSTR_MASK(opcode);
    switch (instr)
    {
        case 0x00: op0(state, opcode); break;
        case 0x01: op1(state, opcode); break;
        case 0x02: UnimplementedInstruction(state); break;
        case 0x03: op3(state, opcode); break;
        case 0x04: op4(state, opcode); break;
        case 0x05: op5(state, opcode); break;
        case 0x06: op6(state, opcode); break;
        case 0x07: op7(state, opcode); break;
        case 0x08: UnimplementedInstruction(state); break;
        case 0x09: op9(state, opcode); break;
        case 0x0a: opA(state, opcode); break;
        case 0x0b: opB(state, opcode); break;
        case 0x0c: opC(state, opcode); break;
        case 0x0d: opD(state, opcode); break;
        case 0x0e: UnimplementedInstruction(state); break;
        case 0x0f: UnimplementedInstruction(state); break;
    }
}

void displayState(Chip8State * state) {
    for (int i = 0; i < 16; i++) {
        printf("V%01X: %02X  ", i, state->V[i]);
    }
    printf("\nI: %04X  PC: %04X  SP: %04X  *SP: %04X\n", state->I, state->PC, state->SP, state->memory[state->SP]);
}

void op0(Chip8State *state, uint16_t opcode) {
    switch (NN_MASK(opcode)) {
        case 0xe0: // clear screen
            memset(state->gfx, OFF, 64*32);
            break;
        case 0xee: printf("RETURN"); break; // return from subroutine
        default: printf("UNKNOWN 0"); break; // usually jump to machine code routine at NNN
    }
}

void op1(Chip8State *state, uint16_t opcode) {
    // Jump to address NNN
	uint16_t target = NNN_MASK(opcode);
	if (target == state->PC-2) {
		printf("INFINITE LOOP DETECTED...\n");
        state->halt = 1;
	}
	state->PC = target;
}

// void op2(Chip8State *state, uint16_t opcode);

void op3(Chip8State *state, uint16_t opcode) {
    // Skip the following instruction if VX == NN
    if (state->V[VX_MASK(opcode)] == NN_MASK(opcode)) {
        state->PC += 2;
    }
}

void op4(Chip8State *state, uint16_t opcode) {
    // Skip the following instruction if VX != NN
    if (state->V[VX_MASK(opcode)] != NN_MASK(opcode)) {
        state->PC += 2;
    }
}

void op5(Chip8State *state, uint16_t opcode) {
    // Skip the following instruction if VX == VY
    if (state->V[VX_MASK(opcode)] == state->V[VY_MASK(opcode)]) {
        state->PC += 2;
    }
}

void op6(Chip8State *state, uint16_t opcode) {
    // Store number NN in register VX
    state->V[VX_MASK(opcode)] = NN_MASK(opcode);
}

void op7(Chip8State *state, uint16_t opcode) {
    // Add the value NN to register VX
    state->V[VX_MASK(opcode)] += NN_MASK(opcode);
}

// void op8(Chip8State *state, uint16_t opcode);

void op9(Chip8State *state, uint16_t opcode) {
    // Skip the following instruction if VX != VY
    if (state->V[VX_MASK(opcode)] != state->V[VY_MASK(opcode)]) {
        state->PC += 2;
    }
}

void opA(Chip8State *state, uint16_t opcode) {
    // Store memory address NNN in register I
    state->I = NNN_MASK(opcode);
}

void opB(Chip8State *state, uint16_t opcode) {
    // Jump to address NNN + V0
    state->PC = state->V[0] + NNN_MASK(opcode);
}

void opC(Chip8State *state, uint16_t opcode) {
    // Set VX to a random number with a mask of NN
    // state->V[VX_MASK(opcode)] = random() & NN_MASK(opcode);
    state->V[VX_MASK(opcode)] = rand() & NN_MASK(opcode);
}

void opD(Chip8State *state, uint16_t opcode) {
    uint8_t x = state->V[VX_MASK(opcode)] % 64;
    uint8_t y = state->V[VY_MASK(opcode)] % 32;
    uint8_t height = LSN_MASK(opcode); // number of lines of sprite data
    uint8_t sprite_byte;
    uint8_t sprite_pixel;
    uint32_t *screen_pixel;

    state->V[0xF] = 0;
    for(int row = 0; row < height; row++){

        sprite_byte = state->memory[state->I + row];

        for(int col = 0; col < 8; col++){

            int address = (x+col) + (y + row) * SCREEN_WIDTH;
            sprite_pixel = sprite_byte & (0x80 >> col);
            screen_pixel = &state->gfx[address];

            // Sprite pixel is ON
            if(sprite_pixel){

                if(*screen_pixel) {
                    state->V[0xF] = 1;
                }
                *screen_pixel ^= ON;
            }
        }
    }
    state->draw_flag = 1;
}

// void opE(Chip8State *state, uint16_t opcode);
// void opF(Chip8State *state, uint16_t opcode);
