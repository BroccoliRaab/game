#include <stdio.h> 
#include <stdlib.h>

#include "gtypes.h"

#include "tilemap_loader.h"

u32f load_tilemap(FILE * file, Tilemap *map){
    i32f current_tile_id;
    u32f pos = 0;
    i32f scan_retval;
    scan_retval = fscanf(file, "%u,%u\n", &map->size_x, &map->size_y);
    if (scan_retval != 2){
        return 0;
    }
    map->size = map->size_x * map->size_y;
    map->buffer = (i32f *) malloc(map->size * (sizeof(i32f)));
    if (!map->buffer){
        return 0;
    }
    while(pos<map->size){
        scan_retval = fscanf(file, "%i", &current_tile_id);
        switch (scan_retval){
        
        case 1:
            map->buffer[pos] = current_tile_id;
            pos++;
            break;
        case EOF:
            return pos;
        case 0:
            fseek(file, 1, SEEK_CUR);
            break;
        default:
            fputs("Impossible fscanf return value reached. Undefined behavior!", stderr);
            break;
        }
    }
    return pos;
}

void free_tilemap(Tilemap *map){
    free(map->buffer);
    map->buffer = NULL;
}
