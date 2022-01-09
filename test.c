#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#define WIN_WIDTH 160
#define WIN_HEIGHT 144

int main(int argc, char **argv) {
    // SDL init
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // create SDL window
    SDL_Window *window = SDL_CreateWindow("sdl2_pixelbuffer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH * 4,
        WIN_HEIGHT * 4,
        SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return 1;
    }

    // create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(renderer,
        WIN_WIDTH, WIN_HEIGHT);

    // create texture
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        WIN_WIDTH,
        WIN_HEIGHT);
    if (texture == NULL) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return 1;
    }

    // array of pixels
    uint8_t pixels[WIN_WIDTH * WIN_HEIGHT * 4] = {0};
    pixels[4 * 0 + 1] = 255;
    pixels[4 * 1 + 1] = 255;
    pixels[4 * 2 + 1] = 255;
    pixels[46400 + 4 * 0 + 0] = 255;
    pixels[46400 + 4 * 1 + 1] = 255;
    pixels[46400 + 4 * 2 + 2] = 255;
    pixels[46400 + 4 * 3 + 0] = 255;
    pixels[46400 + 4 * 4 + 1] = 255;
    pixels[46400 + 4 * 5 + 2] = 255;
    pixels[WIN_WIDTH * WIN_HEIGHT * 4 - 4 * 1] = 255;
    pixels[WIN_WIDTH * WIN_HEIGHT * 4 - 4 * 2] = 255;
    pixels[WIN_WIDTH * WIN_HEIGHT * 4 - 4 * 3] = 255;

    // update texture with new data
    int texture_pitch = 0;
    void* texture_pixels = NULL;
    if (SDL_LockTexture(texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    }
    else {
        memcpy(texture_pixels, pixels, texture_pitch * WIN_HEIGHT);
    }
    SDL_UnlockTexture(texture);

    // main loop
    bool should_quit = false;
    SDL_Event e;
    while (!should_quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                should_quit = true;
            }
        }

        // render on screen
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
