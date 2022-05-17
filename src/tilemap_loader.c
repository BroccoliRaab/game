#include <stdio.h>

#include "gtypes.h"
#include "tilemap_loader.h"

u32f load_tilemap(FILE * file, i32f * buffer, u32f buff_sz){
    i32f current_tile_id;
    u32f pos = 0;
    i32f scan_retval;
    while(pos<buff_sz){
        scan_retval = fscanf(file, "%d", &current_tile_id);
        switch (scan_retval){
        
        case 1:
            buffer[pos] = current_tile_id;
            pos++;
            break;
        case EOF:
            return pos;
        case 0:
            fseek(file, 1, SEEK_CUR);
            break;
        default:
            //TODO: LOGGER?
            fputs("Impossible fscanf return value reached. Undefined behavior!", stderr);
            break;
        }
    }
    return pos;
}
