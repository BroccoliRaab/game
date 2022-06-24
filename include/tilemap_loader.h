#ifndef TILEMAP_LOADER_H
#define TILEMAP_LOADER_H

#include <stdio.h>

#include "gtypes.h"

typedef struct Tilemap {
   i32f *buffer;
   u32f size;
   u32f size_x;
   u32f size_y;

}Tilemap;

u32f load_tilemap(FILE * file, Tilemap *map);
void free_tilemap(Tilemap *map);

#endif /* TILEMAP_LOADER_H */
