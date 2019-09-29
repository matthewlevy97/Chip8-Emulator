#pragma once

#define GRAPHICS_SCREEN_WIDTH  64
#define GRAPHICS_SCREEN_HEIGHT 32
#define GRAPHICS_SCALE         4

void graphics_init();
void graphics_clear_screen();
int  graphics_draw_sprite(CHIP8_cpu *cpu, char x, char y, char bytes);
int  graphics_update();
int  graphics_keypressed(unsigned char keycode);
int graphics_getkeypress();
