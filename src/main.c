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

typedef struct Camera {
    f64 pos_x;
    f64 pos_y;
    f64 dir_x;
    f64 dir_y;
    f64 plane_x;
    f64 plane_y;
}Camera;


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
    Camera main_camera;
    
    main_camera.pos_x = 5;
    main_camera.pos_y = 5;
    main_camera.dir_x = -1;
    main_camera.dir_y = 0;
    main_camera.plane_x = 0;
    main_camera.plane_y = 0.66;

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
                            main_camera.pos_x +=main_camera.dir_x *movespeed;
                            main_camera.pos_y +=main_camera.dir_y *movespeed;
                            break;
                        case SDL_SCANCODE_DOWN:
                            main_camera.pos_x -= main_camera.dir_x *movespeed;
                            main_camera.pos_y -= main_camera.dir_y *movespeed;
                            break;

                        case SDL_SCANCODE_RIGHT:
                            {
                                f64 old_dir_x =main_camera.dir_x;
                                main_camera.dir_x = main_camera.dir_x * cos(-rotspeed) -main_camera.dir_y*sin(-rotspeed);
                                main_camera.dir_y = old_dir_x * sin(-rotspeed) +main_camera.dir_y*cos(-rotspeed);
                                f64 old_plane_x = main_camera.plane_x;
                                main_camera.plane_x = main_camera.plane_x * cos(-rotspeed) - main_camera.plane_y * sin(-rotspeed);
                                main_camera.plane_y = old_plane_x * sin(-rotspeed) + main_camera.plane_y * cos(-rotspeed);
                            }
                            break;
                        case SDL_SCANCODE_LEFT:
                            {
                                f64 old_dir_x =main_camera.dir_x;
                                main_camera.dir_x = main_camera.dir_x * cos(rotspeed) -main_camera.dir_y*sin(rotspeed);
                                main_camera.dir_y = old_dir_x * sin(rotspeed) +main_camera.dir_y*cos(rotspeed);
                                f64 old_plane_x = main_camera.plane_x;
                                main_camera.plane_x = main_camera.plane_x * cos(rotspeed) - main_camera.plane_y * sin(rotspeed);
                                main_camera.plane_y = old_plane_x * sin(rotspeed) + main_camera.plane_y * cos(rotspeed);
                            }
                            break;
                            
                    }
                break;
            }
        }
        for (i32f x =0; x<SCREEN_WIDTH; x++){
            f64 camx =2 * (f64)x/(f64)SCREEN_WIDTH-1;
            f64 ray_dir_x = main_camera.dir_x +main_camera.plane_x *camx;
            f64 ray_dir_y = main_camera.dir_y +main_camera.plane_y * camx;
            
            i32f mapx = (i32f) main_camera.pos_x;
            i32f mapy = (i32f) main_camera.pos_y;

            f64 sidedistx;
            f64 sidedisty;

            f64 deltadistx = (ray_dir_x == 0)? 1e30 : fabs(1.0/ray_dir_x);
            f64 deltadisty = (ray_dir_y == 0)? 1e30 : fabs(1.0/ray_dir_y);

            i32f stepy, stepx;

            b8 hit = 0;

            i8f side;

            if (ray_dir_x<0) {
                stepx =-1;
                sidedistx = (main_camera.pos_x -mapx) *deltadistx;

            }else{
                stepx =1;
                sidedistx = (mapx + 1.0 -main_camera.pos_x) *deltadistx;
            }
            if (ray_dir_y<0) {
                stepy =-1;
                sidedisty = (main_camera.pos_y -mapy) *deltadisty;

            }else{
                stepy =1;
                sidedisty = (mapy + 1.0 -main_camera.pos_y) *deltadisty;
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
