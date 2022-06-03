#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

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


void move_player(Camera *cam, i32f map[100], f64 movespeed, f64 rotspeed);
void draw_walls(Camera *cam,SDL_Renderer *renderer, i32f map[100]);
i32f tile_from_position(i32f map[100], i32f pos_x, i32f pos_y);

int main(void) {
    i32f buffer[100];
    FILE * f =fopen("assets/tilemap/test1.csv", "r");
    FATAL(!f, "Failed to open tilemap", ERROR);

    const i32f mapdim = 10;

    i32f sdl_init_retval = SDL_Init(SDL_INIT_EVERYTHING);
    FATAL(sdl_init_retval<0, "Failed to initialize SDL", ERROR);
    
    i32f ttf_init_retval = TTF_Init();
    FATAL(ttf_init_retval<0, "Failed to initize TTF", ERROR);

    SDL_Window * win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    FATAL(!win, "Failed to create window", ERR_TTF);

    SDL_Renderer * renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    FATAL(!renderer, "Failed to create renderer", ERR_WIN);

    u32f tiles = load_tilemap(f, buffer, 100);

    TTF_Font *font = TTF_OpenFont("assets/font/sans.ttf", 25);

    Camera main_camera;
    
    main_camera.pos_x = 1.1;
    main_camera.pos_y = 5.1;
    main_camera.dir_x = -1;
    main_camera.dir_y = 0;
    main_camera.plane_x = 0;
    main_camera.plane_y = 0.66;

    f64 movespeed = 0.0;
    f64 rotspeed = 0.0;

    SDL_Surface *fps_counter;
    SDL_Surface *position_display;

    u64f last_frame = SDL_GetTicks64();
    b8 loop = 1;
    while (loop){
        u64f curr_frame = SDL_GetTicks64();
        f64 dt = (curr_frame - last_frame) /1000.0;
        last_frame = curr_frame;
        movespeed = dt * 3.0;
        rotspeed = dt * 2.0;
        //printf("FPS: %lf\n", 1.0/dt);
        SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                    loop = 0;
                    break;
            }
        }
        move_player(&main_camera, buffer, movespeed, rotspeed);
        draw_walls(&main_camera, renderer, buffer);

        char fps_str[20];
        char pos_str[25];
        sprintf(fps_str, "FPS: %.5lf", 1.0/dt);
        sprintf(pos_str, "X: %.5lf Y: %.5lf", main_camera.pos_x, main_camera.pos_y);

        fps_counter = TTF_RenderText_Solid(font,  fps_str, (SDL_Color){0xFF,0xFF,0xFF});
        SDL_Texture *fps_counter_texture = SDL_CreateTextureFromSurface(renderer, fps_counter);
        SDL_Rect counter_rect = (SDL_Rect) {0,0,fps_counter->w, fps_counter->h};

        position_display = TTF_RenderText_Solid(font,  pos_str, (SDL_Color){0xFF,0xFF,0xFF});

        SDL_Texture *position_display_texture = SDL_CreateTextureFromSurface(renderer, position_display);
        SDL_Rect position_rect = (SDL_Rect) {0, fps_counter->h+5, position_display->w, position_display->h};

        SDL_RenderCopy(renderer, fps_counter_texture, NULL, &counter_rect);
        SDL_RenderCopy(renderer, position_display_texture, NULL, &position_rect);

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);

        SDL_DestroyTexture(position_display_texture);
        SDL_FreeSurface(position_display);
        SDL_DestroyTexture(fps_counter_texture);
        SDL_FreeSurface(fps_counter);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    TTF_Quit();
    SDL_Quit();
    return 0;



ERR_WIN:
    SDL_DestroyWindow(win);
ERR_TTF:
    TTF_Quit();
ERROR:
    SDL_Quit();
    return 1;
}
void move_player(Camera *cam, i32f map[100], f64 movespeed, f64 rotspeed){
    f64 pos_x =0;
    f64 pos_y =0;
    const u8 *keyboard = SDL_GetKeyboardState(NULL);
    if(keyboard[SDL_SCANCODE_UP]){
        if(tile_from_position(map, (i32f) cam->pos_x+cam->dir_x*movespeed, (i32f) cam->pos_y)<0)
            pos_x = cam->dir_x*movespeed; 
        if(tile_from_position(map, (i32f) cam->pos_x, (i32f) cam->pos_y+cam->dir_y*movespeed)<0)
            pos_y = cam->dir_y*movespeed;
    }else if(keyboard[SDL_SCANCODE_DOWN]){
        if(tile_from_position(map, (i32f) cam->pos_x-cam->dir_x*movespeed, (i32f) cam->pos_y)<0)
            pos_x = -cam->dir_x*movespeed;
        if(tile_from_position(map, (i32f) cam->pos_x, (i32f) cam->pos_y-cam->dir_y*movespeed)<0)
            pos_y = -cam->dir_y*movespeed;
    }
    if(keyboard[SDL_SCANCODE_RIGHT]) {
        f64 old_dir_x =cam->dir_x;
        cam->dir_x = cam->dir_x * cos(-rotspeed) -cam->dir_y*sin(-rotspeed);
        cam->dir_y = old_dir_x * sin(-rotspeed) +cam->dir_y*cos(-rotspeed);
        f64 old_plane_x = cam->plane_x;
        cam->plane_x = cam->plane_x * cos(-rotspeed) - cam->plane_y * sin(-rotspeed);
        cam->plane_y = old_plane_x * sin(-rotspeed) + cam->plane_y * cos(-rotspeed);
    }else if(keyboard[SDL_SCANCODE_LEFT]){
        f64 old_dir_x =cam->dir_x;
        cam->dir_x = cam->dir_x * cos(rotspeed) -cam->dir_y*sin(rotspeed);
        cam->dir_y = old_dir_x * sin(rotspeed) +cam->dir_y*cos(rotspeed);
        f64 old_plane_x = cam->plane_x;
        cam->plane_x = cam->plane_x * cos(rotspeed) - cam->plane_y * sin(rotspeed);
        cam->plane_y = old_plane_x * sin(rotspeed) + cam->plane_y * cos(rotspeed);
    }

    cam->pos_x+= pos_x;
    cam->pos_y+= pos_y;
}
void draw_walls(Camera *cam,SDL_Renderer *renderer, i32f map[100]){
    for (i32f x =0; x<SCREEN_WIDTH; x++){
        f64 cam_x =2 * (f64)x/(f64)SCREEN_WIDTH-1;
        f64 ray_dir_x = cam->dir_x +cam->plane_x *cam_x;
        f64 ray_dir_y = cam->dir_y +cam->plane_y * cam_x;

        i32f map_x = (i32f) cam->pos_x;
        i32f map_y = (i32f) cam->pos_y;

        f64 side_dist_x;
        f64 side_dist_y;

        f64 delta_dist_x = (ray_dir_x == 0)? 1e30 : fabs(1.0/ray_dir_x);
        f64 delta_dist_y = (ray_dir_y == 0)? 1e30 : fabs(1.0/ray_dir_y);

        i32f stepy, stepx;

        b8 hit = 0;

        i8f side;

        if (ray_dir_x<0) {
            stepx =-1;
            side_dist_x = (cam->pos_x -map_x) *delta_dist_x;

        }else{
            stepx =1;
            side_dist_x = (map_x + 1.0 -cam->pos_x) *delta_dist_x;
        }
        if (ray_dir_y<0) {
            stepy =-1;
            side_dist_y = (cam->pos_y -map_y) *delta_dist_y;

        }else{
            stepy =1;
            side_dist_y = (map_y + 1.0 -cam->pos_y) *delta_dist_y;
        }

        while(!hit){
            if (side_dist_x <side_dist_y){
                side_dist_x += delta_dist_x;
                map_x += stepx;
                side =0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += stepy;
                side = 1;
            }
            if (tile_from_position(map, map_x, map_y)>0) hit = 1;
        } 
        f64 wall_dist;
        if (side == 0) {
            wall_dist = (side_dist_x -delta_dist_x);
        }else {
            wall_dist = (side_dist_y -delta_dist_y);
        }
        i32f lineheight  = (i32f)(SCREEN_HEIGHT /wall_dist);

        i32f line_start = -lineheight /2 +SCREEN_HEIGHT/2;
        if (line_start <0) line_start =0;
        i32f line_end = lineheight /2 + SCREEN_HEIGHT/2;
        if(line_end >=SCREEN_HEIGHT) line_end = SCREEN_HEIGHT-1;
        SDL_SetRenderDrawColor(renderer, (side?100:200),0,0,255);
        i32f drawline_retval = SDL_RenderDrawLine(renderer, x, line_start, x, line_end);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    }
}



i32f tile_from_position(i32f map[100],i32f pos_x, i32f pos_y){
    return map[pos_y*10+pos_x];
}
