#include <stdio.h>

#include <SDL2/SDL.h>

#include "gtypes.h"
#include "tilemap_loader.h"

#define TITLE "Game the game"

#define FATAL(condition, output, label) \
    if((condition)){                    \
        fputs((output), stderr);        \
        goto label;                     \
    }

int main(void) {
    int buffer[256];
    FILE * f =fopen("assets/tilemap/test1.csv", "r");
    FATAL(!f, "Failed to open tilemap", ERROR);

    i32f sdl_init_retval = SDL_Init(SDL_INIT_EVERYTHING);
    FATAL(sdl_init_retval<0, "Failed to initialize SDL", ERROR);

    SDL_Window * win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    FATAL(!win, "Failed to create window", ERROR);

    u32f tiles = load_tilemap(f, buffer, 256);

    return 0;

ERROR:
    SDL_Quit();
    return 1;
}
