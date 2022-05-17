#ifndef TILEMAP_LOADER_H
#define TILEMAP_LOADER_H

#include <stdio.h>

#include "gtypes.h"

typedef struct tilemap {

}Tilemap;

u32f load_tilemap(FILE * file, i32f * buffer, u32f buff_sz);
#endif /* TILEMAP_LOADER_H */
