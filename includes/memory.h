#pragma once

#define FONTSET_OFFSET 0x50 
#define SPRITE_LEN     5

int memory_load_program(CHIP8_cpu *cpu, const char * filename);

char memory_read(CHIP8_cpu *cpu, short address);
void memory_write(CHIP8_cpu *cpu, short address, char value);
