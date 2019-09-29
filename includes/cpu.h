#pragma once

#define CHIP8_MEMORY_SIZE     0xFFF
#define CHIP8_STACK_ADDRESSES 16

typedef struct {
    unsigned char  V[16]; // General purpose registers
    unsigned short I;     // Stores memory addresses
    unsigned char  R[8];  // Super CHIP-8 user flag registers
    
    unsigned char DT;
    unsigned char ST;
    
    unsigned short PC;
    unsigned char  SP;
} CHIP8_registers;

typedef struct {
    // Registers
    CHIP8_registers registers;
    
    // Memory
    char  memory[CHIP8_MEMORY_SIZE];
    short stack[CHIP8_STACK_ADDRESSES];
} CHIP8_cpu;
