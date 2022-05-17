#include <stdio.h>

#include <SDL2/SDL.h>

#include "tilemap_loader.h"

int main(void) {
    int buffer[256];
    FILE * f =fopen("assets/tilemap/test1.csv", "r");
    if (!f) return 1;
    u32f tiles = load_tilemap(f, buffer, 256);

    //TESTCODE
    for (u32f ct = 0; ct<tiles; ct++){
        printf("%d, ", buffer[ct]);
    }
    putchar('\n');
    //END OF TESTCODE

    return 0;
}
