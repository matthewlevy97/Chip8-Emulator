#include "cpu.h"
#include "memory.h"
#include "graphics.h"

#include <unistd.h>
#include <SDL2/SDL.h>

static SDL_Window *window;
static SDL_Renderer * renderer;

static char screen[GRAPHICS_SCREEN_WIDTH][GRAPHICS_SCREEN_HEIGHT];
static char keypresses[0xF];

static void draw_pixel(int x, int y);
static void clear_screen();

void graphics_init() {
    SDL_CreateWindowAndRenderer(64 * GRAPHICS_SCALE, 32 * GRAPHICS_SCALE,
        0, &window, &renderer);
}

void graphics_clear_screen() {
    memset(screen, 0, GRAPHICS_SCREEN_WIDTH * GRAPHICS_SCREEN_HEIGHT);
}

int graphics_draw_sprite(CHIP8_cpu *cpu, char x, char y, char bytes) {
    int ret;
    char *sprite, *ptr;
    
    ret = 0;
    sprite = cpu->memory + cpu->registers.I;
    
    for(int j = 0; j < bytes; j++) {
        for(int i = 0; i < 8; i++) {
            ptr = &screen[(x + i) % GRAPHICS_SCREEN_WIDTH][y % GRAPHICS_SCREEN_HEIGHT];
            ret |= *ptr;
            *ptr ^= ((sprite[j] >> (7 - i)) & 0x1);
        }
        y++;
    }
    return ret;
}

int graphics_update() {
    SDL_Event e;
    
    // Clear screen
    clear_screen();
    
    // Draw pixel
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(int i = 0; i < GRAPHICS_SCREEN_WIDTH; i++) {
        for(int j = 0; j < GRAPHICS_SCREEN_HEIGHT; j++) {
            if(screen[i][j]) draw_pixel(i, j);
        }
    }
    SDL_RenderPresent(renderer);
    
    if(!SDL_PollEvent(&e)) return 0;
    
    switch(e.type) {
    case SDL_QUIT:
        return 1;
    case SDL_KEYDOWN:
        switch(e.key.keysym.sym) {
        case SDLK_1:
            keypresses[0x1] = 1; break;
        case SDLK_2:
            keypresses[0x2] = 1; break;
        case SDLK_3:
            keypresses[0x3] = 1; break;
        case SDLK_4:
            keypresses[0xC] = 1; break;
        case SDLK_q:
            keypresses[0x4] = 1; break;
        case SDLK_w:
            keypresses[0x5] = 1; break;
        case SDLK_e:
            keypresses[0x6] = 1; break;
        case SDLK_r:
            keypresses[0xD] = 1; break;
        case SDLK_a:
            keypresses[0x7] = 1; break;
        case SDLK_s:
            keypresses[0x8] = 1; break;
        case SDLK_d:
            keypresses[0x9] = 1; break;
        case SDLK_f:
            keypresses[0xE] = 1; break;
        case SDLK_z:
            keypresses[0xA] = 1; break;
        case SDLK_x:
            keypresses[0x0] = 1; break;
        case SDLK_c:
            keypresses[0xB] = 1; break;
        case SDLK_v:
            keypresses[0xF] = 1; break;
        default: break;
        }
        break;
    case SDL_KEYUP:
        switch(e.key.keysym.sym) {
        case SDLK_1:
            keypresses[0x1] = 0; break;
        case SDLK_2:
            keypresses[0x2] = 0; break;
        case SDLK_3:
            keypresses[0x3] = 0; break;
        case SDLK_4:
            keypresses[0xC] = 0; break;
        case SDLK_q:
            keypresses[0x4] = 0; break;
        case SDLK_w:
            keypresses[0x5] = 0; break;
        case SDLK_e:
            keypresses[0x6] = 0; break;
        case SDLK_r:
            keypresses[0xD] = 0; break;
        case SDLK_a:
            keypresses[0x7] = 0; break;
        case SDLK_s:
            keypresses[0x8] = 0; break;
        case SDLK_d:
            keypresses[0x9] = 0; break;
        case SDLK_f:
            keypresses[0xE] = 0; break;
        case SDLK_z:
            keypresses[0xA] = 0; break;
        case SDLK_x:
            keypresses[0x0] = 0; break;
        case SDLK_c:
            keypresses[0xB] = 0; break;
        case SDLK_v:
            keypresses[0xF] = 0; break;
        default: break;
        }
        break;
    }
    
    return 0;
}

int graphics_keypressed(unsigned char keycode) {
    return keypresses[keycode];
}
int graphics_getkeypress() {
    char map[sizeof(keypresses)];
    int i;
    
    for(i = 0; i < sizeof(map); i++) {
        map[i] = keypresses[i];
    }
    
    while(1) {
        for(i = 0; i < sizeof(map); i++) {
            if(map[i] != keypresses[i]) return i;
        }
        graphics_update();
    }
}

static void clear_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
}

static void draw_pixel(int x, int y) {
    x *= GRAPHICS_SCALE;
    y *= GRAPHICS_SCALE;
    
    for(int i = 0; i < GRAPHICS_SCALE; i++) {
        for(int j = 0; j < GRAPHICS_SCALE; j++) {
            SDL_RenderDrawPoint(renderer, x + i, y + j);
        }
    }
}
