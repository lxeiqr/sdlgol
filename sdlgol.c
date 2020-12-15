#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <SDL2/SDL.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define GR_SCALE 20.f

SDL_Window *gr_window;
SDL_Renderer *gr_renderer;
struct tile fd_base, fd_temp;

const int TILE_COLORS[][3] = {
    {4, 4, 4},      // Empty 1
    {6, 6, 6},   // Empty 2
    {255, 255, 255} // Full
};

struct tile {
    bool *data;
    int w, h;
};

inline int sindex(struct tile fd, int x, int y) {
    return x + y * fd.w;
}

inline int val(struct tile fd, int x, int y) {
    return fd.data[sindex(fd, x, y)];
}

int neighbours(struct tile fd, int x, int y) {
    int neighbours = 0;

    for(int i = MAX(0, x - 1); i <= MIN(fd.w - 1, x + 1); i++)
        for(int j = MAX(0, y - 1); j <= MIN(fd.h - 1, y + 1); j++) {
            if(!(i == x && j == y) && fd.data[sindex(fd, i, j)])
                neighbours++;
        }

    return neighbours;
}
 
void eval_state(struct tile fd, struct tile new, int x, int y) {
    bool *ptr = &new.data[sindex(new, x, y)];
    
    int nh = neighbours(fd, x, y);

    if(val(fd, x, y) == true) {
        if(nh < 2 || nh > 3)
            *ptr = false;
        else 
            *ptr = true;
    }
    else {
        if(nh == 3)
            *ptr = true;
        else
            *ptr = false;
    }
}

void step(struct tile fd, struct tile temp) {
    for(int y = 0; y < fd.h; y++)
        for(int x = 0; x < fd.w; x++)
            eval_state(fd, temp, x, y);

    // Copy changed data
    memcpy(fd.data, temp.data, fd.w * fd.h * sizeof(bool));
}

void tiles_draw(struct tile fd) {

    for(int y = 0; y < fd.h; y++) 
        for(int x = 0; x < fd.w; x++) {

            const int *tile_color;

            // FULL
            if(fd.data[sindex(fd, x, y)])
                tile_color = TILE_COLORS[2];
            // EMPTY 1
            else if((x + (y % 2)) % 2 == 0) 
                tile_color = TILE_COLORS[0];
            // EMPTY 2
            else 
                tile_color = TILE_COLORS[1];
                
            SDL_SetRenderDrawColor(gr_renderer, 
                    tile_color[0], tile_color[1], tile_color[2],
                    SDL_ALPHA_OPAQUE);

            SDL_Rect rect = {
                x * GR_SCALE,
                y * GR_SCALE,
                GR_SCALE, GR_SCALE
            };
            SDL_RenderFillRect(gr_renderer, &rect);
        }
}

void setup_tiles(struct tile *base, struct tile *tempfd, int w, int h) {
    base->w = tempfd->w = w;
    base->h = tempfd->h = h;

    base->data = calloc(w * h, sizeof(bool));
    tempfd->data = calloc(w * h, sizeof(bool));
}

void setup_graphics(struct tile base) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(
        base.w * GR_SCALE * 2.f,
        base.h * GR_SCALE * 2.f,
        SDL_WINDOW_RESIZABLE,
        &gr_window,
        &gr_renderer
    );

    SDL_RenderSetLogicalSize(gr_renderer, base.w * GR_SCALE, base.h * GR_SCALE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_SetWindowTitle(gr_window, "sdlgol");
}

void finish_graphics(void) {
    SDL_DestroyRenderer(gr_renderer);
    SDL_DestroyWindow(gr_window);
    SDL_Quit();
}

inline void mouse_flip_cell(int x, int y) {
    bool *tile = &fd_base.data[sindex(fd_base, x, y)];
    *tile = !(*tile);
}

void usage(char *progname) {
    printf("%s <width> <height>\n"
            "\n"
            "ENTER - step\n"
            "LMB - flip state\n", progname);

    exit(1);
}

int main(int argc, char **argv) {
    if(argc != 3)
        usage(argv[0]);

    setup_tiles(&fd_base, &fd_temp, atoi(argv[1]), atoi(argv[2]));
    setup_graphics(fd_base);

    // Main loop
    SDL_Event event;
    bool main_loop = true;
    while(main_loop) {
        uint32_t frame_start = SDL_GetTicks();

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                main_loop = false;

            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_RETURN)
                    step(fd_base, fd_temp);
            }

            else if(event.type == SDL_MOUSEBUTTONDOWN) {
                if(event.button.x > 0 && event.button.x < GR_SCALE * fd_base.w &&
                   event.button.y > 0 && event.button.y < GR_SCALE * fd_base.h) {

                    mouse_flip_cell(floorf(event.button.x / GR_SCALE), 
                                    floorf(event.button.y / GR_SCALE));
                }
            }
        }


        SDL_SetRenderDrawColor(gr_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(gr_renderer);
        tiles_draw(fd_base);
        SDL_RenderPresent(gr_renderer);

        // Frame limit to 60 FPS
        int delay = (1000.f/60.f) - (SDL_GetTicks() - frame_start);
        SDL_Delay(delay > 0 ? delay : 0);
    }


    // Free resources
    free(fd_base.data);
    free(fd_temp.data);
    finish_graphics();
}
