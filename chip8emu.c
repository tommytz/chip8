#include "chip8emu.h"
#include "chip8dis.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
int pitch = 64*4;

SDL_AudioSpec *audioSpec = NULL;
SDL_AudioDeviceID audioDevice = 0;
int sampleNR;

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

    uint32_t previous_time = 0;
    uint32_t current_time, delta;
    int32_t time_accumulator;
    const float timer_rate = 1000.0f / 60.0f; // 60 Hz

    // Set the program counter to 0x200 and iterate over buffer
    while (c8->PC < (fsize + 0x200)) {
        SDL_Event event;

        current_time = SDL_GetTicks();
        delta = current_time - previous_time;
        previous_time = current_time;
        time_accumulator += delta;

        while(time_accumulator >= timer_rate){
            if(c8->delay){
                c8->delay--;
            }
            if(c8->sound){
                c8->sound--;
            }
            time_accumulator -= timer_rate;
        }

        while(SDL_PollEvent(&event)){
            switch(event.type) {
                case SDL_QUIT: c8->halt = 1; break;
                case SDL_KEYDOWN:
                {
                    if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                        c8->halt = 1;
                    }
                    for(int i = 0; i < 16; i++){
                        if(event.key.keysym.scancode == SDL_keymap[i]){
                            uint8_t key = Chip8_keymap[i];
                            c8->key_state[key] = 1;
                        }
                    }
                } break;
                case SDL_KEYUP:
                {
                    for(int i = 0; i < 16; i++){
                        if(event.key.keysym.scancode == SDL_keymap[i]){
                            uint8_t key = Chip8_keymap[i];
                            c8->key_state[key] = 0;
                        }
                    }
                } break;
            }
        }
        if(c8->sound){
            beepSound();
        }
        EmulateChip8Op(c8);
        displayState(c8);

        printf("\n");
        if (c8->halt) {
            break;
        }
        if(c8->draw_flag){
            updateDisplay(c8);
            c8->draw_flag = 0;
        }
        SDL_Delay(2);
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

    memcpy(&state->memory[FONT_ADDRESS], fontset, FONT_SIZE);

    state->SP = 0;
	state->PC = 0x200;
    state->halt = 0;
    state->draw_flag = 0;

    return state;
}

void InitDisplay(void) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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

void audio_callBack(void *userData, unsigned char *rawBuffer, int bytes)
{
    short *buffer = (short *)rawBuffer;
    int length = bytes / 2;
    int sampleNR = (*(int *) userData);

    for(int data = 0; data < length; data++, sampleNR++)
    {
        double time = (double)sampleNR / (double)SAMPLE_RATE;
        buffer[data] = (short)(AMPLITUDE * sin(2.0f * M_PI * 441.0f * time));
    }
}

void InitSound() {
    sampleNR = 0;

    audioSpec = (SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));
    audioSpec->freq = SAMPLE_RATE;
    audioSpec->format = AUDIO_S8;
    audioSpec->channels = 1;
    audioSpec->samples = 2048;
    audioSpec->callback = audio_callBack;
    audioSpec->userdata = &sampleNR;

    audioDevice = SDL_OpenAudioDevice(NULL, 0, audioSpec, NULL, 0);
    if(audioDevice != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s\n", SDL_GetError());
    }
}

void closeAudio()
{
    if(audioDevice != 0)
    {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }

    if(audioSpec != 0)
    {
        free(audioSpec);
        audioSpec = NULL;
    }
}

void beepSound()
{
    SDL_PauseAudioDevice(audioDevice, 0);
    SDL_Delay(40);
    SDL_PauseAudioDevice(audioDevice, 1);
}

void terminate(int exit_code) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    closeAudio();
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
	printf ("Error: Unimplemented instruction\n");
}

void EmulateChip8Op(Chip8State *state)
{


    uint8_t *op = &state->memory[state->PC];
    disassembleChip8Opcode(state->memory, state->PC);
    state->PC += 2; // increment after fetching
    printf("\n");
    uint16_t opcode = CONCAT(op[0], op[1]);
    uint8_t instr = INSTR_MASK(opcode);
    switch (instr)
    {
        case 0x00: op0(state, opcode); break;
        case 0x01: op1(state, opcode); break;
        case 0x02: op2(state, opcode); break;
        case 0x03: op3(state, opcode); break;
        case 0x04: op4(state, opcode); break;
        case 0x05: op5(state, opcode); break;
        case 0x06: op6(state, opcode); break;
        case 0x07: op7(state, opcode); break;
        case 0x08: op8(state, opcode); break;
        case 0x09: op9(state, opcode); break;
        case 0x0a: opA(state, opcode); break;
        case 0x0b: opB(state, opcode); break;
        case 0x0c: opC(state, opcode); break;
        case 0x0d: opD(state, opcode); break;
        case 0x0e: opE(state, opcode); break;
        case 0x0f: opF(state, opcode); break;
    }
}

void displayState(Chip8State * state) {
    for (int i = 0; i < 8; i++) {
        printf("V%01X: %02X  ", i, state->V[i]);
    }
    printf("\n");
    for (int i = 8; i < 16; i++) {
        printf("V%01X: %02X  ", i, state->V[i]);
    }
    int stack_pointer;
    if(state->SP <= 0){
        stack_pointer = 0;
    } else {
        stack_pointer = (state->SP)-1;
    }
    printf("\nI: %04X  PC: %04X  SP: %04X  *SP: %04X\nDELAY: %02X  SOUND: %02X\n",
    state->I, state->PC, state->SP, state->stack[stack_pointer], state->delay, state->sound);
}

void op0(Chip8State *state, uint16_t opcode) {
    switch (NN_MASK(opcode)) {
        case 0xE0: // clear screen
            memset(state->gfx, OFF, 64*32);
            state->draw_flag = 1;
            break;
        case 0xEE: // Return from subroutine, pop the stack to the PC
            state->SP--;
            state->PC = state->stack[state->SP];
            break;
        default: printf("UNKNOWN 0 INSTRUCTION"); break;
    }
}

void op1(Chip8State *state, uint16_t opcode) {
    // Jump to address NNN
	uint16_t target = NNN_MASK(opcode);
	if (target == state->PC-2) {
		printf("INFINITE LOOP DETECTED...\n");
	}
	state->PC = target;
}

void op2(Chip8State *state, uint16_t opcode) {
    // Call subroutine at nnn, push the address of the next instruction to the stack
    state->stack[state->SP] = state->PC;
    state->SP++;
    state->PC = NNN_MASK(opcode);
}

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

void op8(Chip8State *state, uint16_t opcode) {
    uint8_t x = VX_MASK(opcode);
    uint8_t y = VY_MASK(opcode);

    switch (LSN_MASK(opcode)) {
        case 0x0: // VX = VY
            state->V[x] = state->V[y]; break;
        case 0x1: // VX = VX OR VY
            state->V[x] |= state->V[y]; break;
        case 0x2: // VX = VX AND VY
            state->V[x] &= state->V[y]; break;
        case 0x3: // VX = VX XOR VY
            state->V[x] ^= state->V[y]; break;
        case 0x4: // VX += VY, VF = Carry (ie result > 255, 8-bit overflow)
        {
            uint16_t result = state->V[x] + state->V[y];
            if (result & 0xFF00) {
                state->V[0xF] = 1;
            } else {
                state->V[0xF] = 0;
            }
            // Only set the 8-bit result in the register
            state->V[x] = (result&0xFF); break;
        }
        case 0x5: // VX -= VY, VF = !borrow (0 if a borrow occurs on underflow, otherwise 1)
        {
            uint8_t borrow = (state->V[x] > state->V[y]);
            state->V[x] -= state->V[y];
            state->V[0xF] = borrow; break;
        }
        case 0x6: // VX = VY >> 1, VF = LSB
            state->V[x] = state->V[y];
            uint8_t lsb = state->V[x] & 0x1; // least-significant bit
			state->V[x] = state->V[x] >> 1;
			state->V[0xF] = lsb;
            break;
        case 0x7: // VX = VY - VX, VF = !borrow (0 if a borrow occurs on underflow, otherwise 1)
        {
            uint8_t borrow = (state->V[y] > state->V[x]);
            state->V[x] = state->V[y] - state->V[x];
            state->V[0xF] = borrow; break;
        }
        case 0xE: // VX = VY << 1, VF = MSB
            state->V[x] = state->V[y];
            uint8_t msb = state->V[x] & 0x80; // most-significant bit
			state->V[x] = state->V[x] << 1;
			state->V[0xF] = msb;
            break;
        default: printf("UNKNOWN 8 INSTRUCTION"); break;
    }
}

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
    // Load N bytes of 8-bit sprite data into video memory at (VX,VY) coordinates
    uint8_t x = state->V[VX_MASK(opcode)] % 64;
    uint8_t y = state->V[VY_MASK(opcode)] % 32;
    uint8_t height = LSN_MASK(opcode); // number of lines of sprite data

    state->V[0xF] = 0;
    for(int row = 0; row < height; row++){

        uint8_t sprite_byte = state->memory[state->I + row];

        for(int col = 0; col < 8; col++){

            int address = (x+col) + (y + row) * SCREEN_WIDTH;
            uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
            uint32_t *screen_pixel = &state->gfx[address];

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

void opE(Chip8State *state, uint16_t opcode) {
    uint8_t x = VX_MASK(opcode);
    switch (NN_MASK(opcode)) {
        case 0x9E: // Skip if key with the value of VX is pressed
            if(state->key_state[state->V[x]] != 0){
                state->PC += 2;
            }
            break;
        case 0xA1: // Skip if key with the value of VX is not pressed
            if(state->key_state[state->V[x]] == 0){
                state->PC += 2;
            }
            break;
        default: printf("UNKNOWN E INSTRUCTION"); break;
    }
}

void opF(Chip8State *state, uint16_t opcode) {
    uint8_t x = VX_MASK(opcode);
    switch (NN_MASK(opcode)) {
        case 0x07: // VX = Delay timer
            state->V[x] = state->delay; break;
        case 0x0A: // Wait for keypress, then VX = KEY
        {
            int key_pressed = 0;
            for (int i = 0; i < 16; i++) {
                if (state->key_state[i]){ // A key is pressed
                    state->V[x] = i; // VX = KEY
                    key_pressed = 1;
                }
            }
            if(!key_pressed){
                state->PC -= 2;
            }
        } break;
        case 0x15: // Delay timer = VX
            state->delay = state->V[x]; break;
        case 0x18: // Sound timer = VX
            state->sound = state->V[x]; break;
        case 0x1E: // I += VX
            state->I += state->V[x]; break;
        case 0x29: // Set I = memory address of the font sprite of the value in VX
            state->I = FONT_ADDRESS + (state->V[VX_MASK(opcode)] * 5) ; break;
        case 0x33: // Store the binary coded decimal equivalent of VX in memory[I..I+2]
        {
            uint8_t ones, tens, hundreds;
            uint8_t value = state->V[x];
            ones = value % 10;
            value = value / 10;
            tens = value % 10;
            hundreds = value / 10;
            state->memory[state->I] = hundreds;
            state->memory[state->I+1] = tens;
            state->memory[state->I+2] = ones;
            break;
        }
        case 0x55: // Store V[0..X] in memory at memory[I..I+X]
        {
            for(uint8_t reg = 0; reg <= x; reg++){
                state->memory[state->I + reg] = state->V[reg];
            }
            break;
        }
        case 0x65: // Set V[0..X] from the values at memory[I..I+X]
        {
            for(uint8_t reg = 0; reg <= x; reg++){
                state->V[reg] = state->memory[state->I + reg];
            }
            break;
        }
        default: printf("UNKNOWN F INSTRUCTION"); break;
    }
}
