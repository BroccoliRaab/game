#include <stdio.h>

#include <SDL2/SDL.h>

#include <math.h>

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
    i32f buffer[100];
    FILE * f =fopen("assets/tilemap/test1.csv", "r");
    FATAL(!f, "Failed to open tilemap", ERROR);

    const i32f mapdim = 10;

    i32f sdl_init_retval = SDL_Init(SDL_INIT_EVERYTHING);
    FATAL(sdl_init_retval<0, "Failed to initialize SDL", ERROR);


    SDL_Window * win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    FATAL(!win, "Failed to create window", ERROR);

    SDL_Renderer * renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    FATAL(!renderer, "Failed to create renderer", ERR_WIN);

    //u32f tiles = load_tilemap(f, buffer, 256);

    f64 posx = 5;
    f64 posy = 5;
    f64 dirx = -1;
    f64 diry = 0;
    f64 planex = 0;
    f64 planey = 0.66;

    f64 movespeed = 0.0;
    f64 rotspeed = 0.0;

    u64f last_frame = SDL_GetTicks64();
    b8 loop = 1;
    while (loop){
        u64f curr_frame = SDL_GetTicks64();
        f64 dt = (curr_frame - last_frame) /1000.0;
        last_frame = curr_frame;
        movespeed = dt * 5.0;
        rotspeed = dt * 3.0;
        printf("FPS: %lf\n", 1.0/dt);
        SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                    loop = 0;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.scancode){
                        case SDL_SCANCODE_UP:
                            posx +=dirx *movespeed;
                            posy +=diry *movespeed;
                            break;
                        case SDL_SCANCODE_DOWN:
                            posx -= dirx *movespeed;
                            posy -= diry *movespeed;
                            break;

                        case SDL_SCANCODE_RIGHT:
                            {
                                f64 olddirx =dirx;
                                dirx = dirx * cos(-rotspeed) -diry*sin(-rotspeed);
                                diry = olddirx * sin(-rotspeed) +diry*cos(-rotspeed);
                                f64 oldplanex = planex;
                                planex = planex * cos(-rotspeed) - planey * sin(-rotspeed);
                                planey = oldplanex * sin(-rotspeed) + planey * cos(-rotspeed);
                            }
                            break;
                        case SDL_SCANCODE_LEFT:
                            {
                                f64 olddirx =dirx;
                                dirx = dirx * cos(rotspeed) -diry*sin(rotspeed);
                                diry = olddirx * sin(rotspeed) +diry*cos(rotspeed);
                                f64 oldplanex = planex;
                                planex = planex * cos(rotspeed) - planey * sin(rotspeed);
                                planey = oldplanex * sin(rotspeed) + planey * cos(rotspeed);
                            }
                            break;
                            
                    }
                break;
            }
        }
        for (i32f x =0; x<SCREEN_WIDTH; x++){
            f64 camx =2 * (f64)x/(f64)SCREEN_WIDTH-1;
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
                if (buffer[mapy*mapdim+mapx]>0) hit = 1;
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
            SDL_SetRenderDrawColor(renderer, (side?100:200),0,0,255);
            i32f drawline_retval = SDL_RenderDrawLine(renderer, x, drawstart, x, drawend);
        }
        SDL_RenderPresent(renderer);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
    }

    return 0;

ERR_WIN:
    SDL_DestroyWindow(win);

ERROR:
    SDL_Quit();
    return 1;
}
