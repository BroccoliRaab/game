#include <stdio.h>

#include <SDL2/SDL.h>

#include "gtypes.h"
#include "tilemap_loader.h"

#define TITLE "Game the game"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

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


    SDL_Window * win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    FATAL(!win, "Failed to create window", ERROR);

    SDL_Renderer * renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    FATAL(!renderer, "Failed to create renderer", ERR_WIN);

    u32f tiles = load_tilemap(f, buffer, 256);
    
    f64 posx = 5;
    f64 posy = 5;
    f64 dirx = 0;
    f64 diry = 0;
    f64 planex = 0;
    f64 planey = 0.66;
    
    u64f last_frame = SDL_GetTicks64();

    while (1){
        for (i32f x =0; x<SCREEN_WIDTH; x++){
            f64 camx =2 *x/(f64)SCREEN_WIDTH-1;
            f64 raydirx = dirx +planex *camx;
            f64 raydiry = diry +planey * camx;
            
            i32f mapx = (i32f) posx;
            i32f mapy = (i32f) posy;

            f64 sidedistx;
            f64 sidedisty;

            f64 deltadistx = (raydirx == 0)? 1e30 : fabs(1.0/raydirx);
            f64 deltadisty = (raydiry == 0)? 1e30 : fabs(1.0/raydiry);

            i32f stepy, stepx;

            b8 hit = 0;

            i8f side;

            if (raydirx<0) {
                stepx =-1;
                sidedistx = (posx -mapx) *deltadistx;

            }else{
                stepx =1;
                sidedistx = (mapx + 1.0 -posx) *deltadistx;
            }
            if (raydiry<0) {
                stepy =-1;
                sidedisty = (posy -mapy) *deltadisty;

            }else{
                stepy =1;
                sidedisty = (mapy + 1.0 -posy) *deltadisty;
            }

            while(!hit){
                if (sidedistx <sidedisty){
                    sidedistx += deltadistx;
                    mapx += stepx;
                    side =0;
                } else {
                    sidedisty += deltadisty;
                    mapy += stepy;
                    side = 1;
                }
                if (buffer[mapy*10+mapx]>0) hit =1;
            } 
            f64 perpwalldist;
            if (side == 0) {
                perpwalldist = (sidedistx -deltadistx);
            }else {
                perpwalldist = (sidedisty -deltadisty);
            }
            i32f lineheight  = (i32f)(SCREEN_HEIGHT /perpwalldist);

            i32f drawstart = -lineheight /2 +SCREEN_HEIGHT/2;
            if (drawstart <0) drawstart =0;
            i32f drawend = lineheight /2 + SCREEN_HEIGHT/2;
            if(drawend >=SCREEN_HEIGHT) drawend = SCREEN_HEIGHT-1;
            SDL_SetRenderDrawColor(renderer, 200,0,0,255);
            i32f drawline_retval = SDL_RenderDrawLine(renderer, x, drawstart, x, drawend);
        }
    SDL_RenderPresent(renderer);
    }

    return 0;

ERR_WIN:
    SDL_DestroyWindow(win);

ERROR:
    SDL_Quit();
    return 1;
}
