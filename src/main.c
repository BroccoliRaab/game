#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf.h>

#include "gtypes.h"
#include "tilemap_loader.h"

#define TITLE "Game the game"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct Camera {
    f64 pos_x;
    f64 pos_y;
    f64 dir_x;
    f64 dir_y;
    f64 plane_x;
    f64 plane_y;
}Camera;


void move_player(Camera *cam, Tilemap *map, f64 movespeed, f64 rotspeed);
void draw_walls(Camera *cam,SDL_Renderer *renderer, Tilemap *map, SDL_Surface * tile_img);
i32f tile_from_position(Tilemap *map, i32f pos_x, i32f pos_y);
i32f DrawVTexLine(SDL_Renderer *renderer, SDL_Surface *tex, i32f scr_x, i32f line_start, i32f line_end, i32f tex_x);

int main(void) {
    Tilemap map;
    i32f main_ret = 0;
    FILE * f =fopen("assets/tilemap/test1.csv", "r");
    if (!f){
        fputs("Failed to open tilemap\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }
    u32f tiles = load_tilemap(f, &map);
    if (!tiles){
        fputs("Failed to open tilemap\n", stderr);
        main_ret =1;
        goto EXIT_main;
    }
    fclose(f);

    SDL_Surface * tile_img = IMG_Load("assets/tileset/brks.bmp");
    tile_img = SDL_ConvertSurfaceFormat(tile_img, SDL_PIXELFORMAT_ABGR8888, 0);
    if (!tile_img){
        fputs("Failed to open tile image\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }

    i32f sdl_init_retval = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdl_init_retval<0){
        fputs("Failed to initialize SDL\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }
    
    i32f ttf_init_retval = TTF_Init();
    if (ttf_init_retval<0){
        fputs("Failed to initize TTF\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }

    SDL_Window * win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!win){
        fputs("Failed to create window\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }

    SDL_Renderer * renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    //|SDL_RENDERER_PRESENTVSYNC
    if(!renderer) {
        fputs("Failed to create renderer\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }

    TTF_Font *font = TTF_OpenFont("assets/font/sans.ttf", 25);
    if (!renderer){
        fputs("Failed to load font\n", stderr);
        main_ret = 1;
        goto EXIT_main;
    }

    SDL_Texture *tile_img_tex = SDL_CreateTextureFromSurface(renderer, tile_img);

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
    for (;;){
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
                    goto EXIT_main;
            }
        }
        move_player(&main_camera, &map, movespeed, rotspeed);
        draw_walls(&main_camera, renderer, &map, tile_img);

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
        SDL_Rect img_position_rect = (SDL_Rect) {0, position_rect.h+32, tile_img->w, tile_img->h};

        SDL_RenderCopy(renderer, fps_counter_texture, NULL, &counter_rect);
        SDL_RenderCopy(renderer, position_display_texture, NULL, &position_rect);
        SDL_RenderCopy(renderer, tile_img_tex, NULL, &img_position_rect);

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);

        SDL_DestroyTexture(position_display_texture);
        SDL_FreeSurface(position_display);
        SDL_DestroyTexture(fps_counter_texture);
        SDL_FreeSurface(fps_counter);
    }

EXIT_main:

    TTF_CloseFont(font);

    SDL_FreeSurface(tile_img);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    free_tilemap(&map);
    return main_ret;
}
void move_player(Camera *cam, Tilemap *map, f64 movespeed, f64 rotspeed){
    f64 pos_x =0.0;
    f64 pos_y =0.0;
    const u8 *keyboard = SDL_GetKeyboardState(NULL);
    if(keyboard[SDL_SCANCODE_UP]){
        pos_x = cam->dir_x*movespeed; 
        pos_y = cam->dir_y*movespeed;
    }else if(keyboard[SDL_SCANCODE_DOWN]){
        pos_x = -cam->dir_x*movespeed;
        pos_y = -cam->dir_y*movespeed;
    }
    if(tile_from_position(map, (i32f) (cam->pos_x+pos_x), (i32f) (cam->pos_y))>-1){
        pos_x =0.0;
    }
    if(tile_from_position(map, (i32f) (cam->pos_x), (i32f) (cam->pos_y+pos_y))>-1){
        pos_y =0.0;
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
void draw_walls(Camera *cam,SDL_Renderer *renderer, Tilemap *map, SDL_Surface* tile_img){
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
        /*
        SDL_SetRenderDrawColor(renderer, (side?100:200),0,0,255);
        i32f drawline_retval = SDL_RenderDrawLine(renderer, x, line_start, x, line_end);
        if(drawline_retval)
            fprintf(stderr, "WARN: Failed to draw line in column %d.\n%s\n", x, SDL_GetError());
        */
        f64 wall_x;
        
        //calculate value of wallX
        if (side == 0) wall_x = cam->pos_y + wall_dist * ray_dir_y;
        else           wall_x = cam->pos_x + wall_dist * ray_dir_x;
        wall_x -= floor((wall_x));

        //x coordinate on the texture
        i32f tex_x = (wall_x * tile_img->w );
        if(side == 0 && ray_dir_x > 0) tex_x = tile_img->w - tex_x - 1;
        if(side == 1 && ray_dir_y < 0) tex_x = tile_img->w - tex_x - 1;

        DrawVTexLine(renderer,
            tile_img,
            x,
            line_start,
            line_end,
            tex_x);

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    }
}

i32f DrawVTexLine(SDL_Renderer *renderer, SDL_Surface *tex, i32f scr_x, i32f line_start, i32f line_end, i32f tex_x){
    SDL_PixelFormat *fmt = tex->format;
    SDL_Surface *surface = tex;
    Uint32 temp, pixel;
    Uint8 red, green, blue, alpha;
    Uint32 tex_y;
    f64 tex_y_f;
    Uint32 *pixel_buf = (tex->pixels);
    for (int i=0; i<line_end-line_start ; i++)
    {

        fmt = surface->format;
        SDL_LockSurface(surface);
        tex_y_f = (((f64)i)/(f64)(line_end-line_start)) * (f64)surface->h;
        tex_y = (Uint32) floor(tex_y_f);
        pixel = pixel_buf[tex_x+tex_y*tex->h];

        
        SDL_UnlockSurface(surface);

        /* Get Red component */
        temp = pixel & fmt->Rmask;  /* Isolate red component */
        temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
        temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
        red = (Uint8)temp;

        /* Get Green component */
        temp = pixel & fmt->Gmask;  /* Isolate green component */
        temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
        temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
        green = (Uint8)temp;

        /* Get Blue component */
        temp = pixel & fmt->Bmask;  /* Isolate blue component */
        temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
        temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
        blue = (Uint8)temp;

        /* Get Alpha component */
        temp = pixel & fmt->Amask;  /* Isolate alpha component */
        temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
        temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
        alpha = (Uint8)temp;


        SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
        SDL_RenderDrawPoint(renderer, scr_x, line_start+i);
    }
    return 0;

}


i32f tile_from_position(Tilemap *map,i32f pos_x, i32f pos_y){
    return map->buffer[pos_y*(map->size_x)+pos_x];
}
