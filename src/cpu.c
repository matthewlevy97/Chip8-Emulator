#include "cpu.h"
#include "memory.h"
#include "graphics.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#define MIN(x, y) ((x) < (y) ? x : y)

CHIP8_cpu cpu;

void cpu_init(CHIP8_cpu *cpu) {
    int i;
    
    for(i = 0; i < 16; i++) cpu->registers.V[i] = 0;
    cpu->registers.PC = 0x200;
}

#ifdef DEBUG
void disassemble(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    fprintf(stderr, "0x%04x: ", cpu.registers.PC-2);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}
#else
void disassemble(const char *fmt, ...) {}
#endif

void dump_registers(CHIP8_cpu *cpu) {
    for(int i = 0; i < 16; i++) {
        fprintf(stderr, "v%01x -> %2x\n", i, cpu->registers.V[i]);
    }
}

// Fetch-Decode-Execute
void cpu_fde(CHIP8_cpu *cpu) {
    int tmp;
    short opcode;
    char * memory;
    
    // Get opcode
    memory = cpu->memory + cpu->registers.PC;
    opcode = (*memory << 8) | (*(memory + 1) & 0xFF);
    cpu->registers.PC += 2;
    
    switch(opcode & 0xF000) {
    case 0x0000:
        switch(opcode & 0xFF) {
        case 0xE0:
            // Clear screen
            disassemble("cls");
            graphics_clear_screen();
            break;
        case 0xEE:
            // Return
            disassemble("ret");
            cpu->registers.PC = cpu->stack[--cpu->registers.SP];
            break;
        default:
            // Jump to address NNN
            disassemble("jmp 0x%04x", opcode);
            cpu->registers.PC = opcode;
        }
        break;
    case 0x1000:
        // Jump to address NNN
        disassemble("jmp 0x%04x", opcode & 0xFFF);
        cpu->registers.PC = opcode & 0xFFF;
        break;
    case 0x2000:
        // Call address NNN
        disassemble("call 0x%04x", opcode & 0xFFF);
        cpu->stack[cpu->registers.SP++] = cpu->registers.PC;
        cpu->registers.PC = opcode & 0xFFF;
        break;
    case 0x3000:
        disassemble("se v%01x, 0x%02x", ((opcode >> 8) & 0xF), opcode & 0xFF);
        // Skip next instruction if Vx = kk.
        if(cpu->registers.V[(opcode >> 8) & 0xF] == (opcode & 0xFF))
            cpu->registers.PC += 2;
        break;
    case 0x4000:
        disassemble("sne v%01x, 0x%02x", ((opcode >> 8) & 0xF), opcode & 0xFF);
        // Skip next instruction if Vx != kk.
        if(cpu->registers.V[(opcode >> 8) & 0xF] != (opcode & 0xFF))
            cpu->registers.PC += 2;
        break;
    case 0x5000:
        disassemble("se v%01x, v%01x", (opcode >> 8) & 0xF, (opcode >> 4) & 0xFF);
        // Skip next instruction if Vx = Vy.
        opcode >>= 4;
        if(cpu->registers.V[(opcode >> 4) & 0xF] == (opcode & 0xF))
            cpu->registers.PC += 2;
        break;
    case 0x6000:
        disassemble("ld v%01x, 0x%02x", (opcode >> 8) & 0xF, opcode & 0xFF);
        cpu->registers.V[(opcode >> 8) & 0xF] = opcode & 0xFF;
        break;
    case 0x7000:
        disassemble("add v%01x, 0x%02x", (opcode >> 8) & 0xF, opcode & 0xFF);
        cpu->registers.V[(opcode >> 8) & 0xF] += (opcode & 0xFF);
        break;
    case 0x8000:
        switch(opcode & 0xF) {
        case 0x0:
            opcode >>= 4;
            disassemble("ld v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            cpu->registers.V[(opcode >> 4) & 0xF] = cpu->registers.V[opcode & 0xF];
            break;
        case 0x1:
            opcode >>= 4;
            disassemble("or v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            cpu->registers.V[(opcode >> 4) & 0xF] |= cpu->registers.V[opcode & 0xF];
            break;
        case 0x2:
            opcode >>= 4;
            disassemble("and v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            cpu->registers.V[(opcode >> 4) & 0xF] &= cpu->registers.V[opcode & 0xF];
            break;
        case 0x3:
            opcode >>= 4;
            disassemble("xor v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            cpu->registers.V[(opcode >> 4) & 0xF] ^= cpu->registers.V[opcode & 0xF];
            break;
        case 0x4:
            opcode >>= 4;
            disassemble("add v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            tmp = cpu->registers.V[(opcode >> 4) & 0xF] + cpu->registers.V[(opcode & 0xF)];
            cpu->registers.V[(opcode >> 4) & 0xF] = tmp;
            // Set carry flag
            if(tmp >> 8)
                cpu->registers.V[0xF] = 1;
            else
                cpu->registers.V[0xF] = 0;
            
            break;
        case 0x5:
            opcode >>= 4;
            disassemble("sub v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            
            cpu->registers.V[(opcode >> 4) & 0xF] -= cpu->registers.V[opcode & 0xF];
            tmp = cpu->registers.V[(opcode >> 4) & 0xF];
            // Set NOT borrow flag
            if(cpu->registers.V[opcode & 0xF] > (unsigned char)tmp)
                cpu->registers.V[0xF] = 1;
            else
                cpu->registers.V[0xF] = 0;
            break;
        case 0x6:
            opcode >>= 4;
            disassemble("shr v%01x", (opcode >> 4) & 0xF);
            cpu->registers.V[0xF] = cpu->registers.V[(opcode >> 4) & 0xF] & 0x1;
            cpu->registers.V[(opcode >> 4) & 0xF] /= 2;
            break;
        case 0x7:
            opcode >>= 4;
            disassemble("subn v%01x, v%01x", (opcode >> 4) & 0xF, opcode & 0xF);
            
            cpu->registers.V[(opcode >> 4) & 0xF] = cpu->registers.V[opcode & 0xF] 
                - cpu->registers.V[(opcode >> 4) & 0xF];
            tmp = cpu->registers.V[(opcode >> 4) & 0xF];
            // Set NOT borrow flag
            if(cpu->registers.V[opcode & 0xF] > (unsigned char)tmp)
                cpu->registers.V[0xF] = 1;
            else
                cpu->registers.V[0xF] = 0;
            break;
        case 0xE:
            opcode >>= 4;
            disassemble("shl v%01x", (opcode >> 4) & 0xF);
            cpu->registers.V[0xF] = cpu->registers.V[(opcode >> 4) & 0xF] >> 7;
            cpu->registers.V[(opcode >> 4) & 0xF] *= 2;
            break;
        default:
            goto unknown_opcode;
        }
        break;
    case 0x9000:
        disassemble("sne v%01x, v%01x", (opcode >> 8) & 0xF, (opcode >> 4) & 0xFF);
        // Skip next instruction if Vx != Vy.
        opcode >>= 4;
        if(cpu->registers.V[(opcode >> 4) & 0xF] != (opcode & 0xF))
            cpu->registers.PC += 2;
        break;
    case 0xA000:
        disassemble("ld I, 0x%04x", opcode & 0xFFF);
        cpu->registers.I = opcode & 0xFFF;
        break;
    case 0xB000:
        disassemble("jmp v0, 0x%04x", opcode & 0xFFF);
        cpu->registers.PC = cpu->registers.V[0] + (opcode & 0xFFF);
        break;
    case 0xC000:
        disassemble("rnd v%01x, %02x", (opcode >> 8) & 0xF, opcode & 0xFF);
        cpu->registers.V[(opcode >> 8) & 0xF] = rand() & (opcode & 0xFF);
        break;
    case 0xD000:
        // DRW Vx, Vy, nibble
        disassemble("DRW v%01x, v%01x, %01x",
            (opcode >> 8) & 0xF, (opcode >> 4) & 0xF,
            opcode & 0xF);
        cpu->registers.V[0xF] = graphics_draw_sprite(cpu,
            cpu->registers.V[(opcode >> 8) & 0xF],
            cpu->registers.V[(opcode >> 4) & 0xF],
            opcode & 0xF);
        break;
    case 0xE000:
        switch(opcode & 0xFF) {
        case 0x9E:
             // Is key down
            opcode = (opcode >> 8) & 0xF;
            disassemble("skp v%01x", opcode);
            
            if(graphics_keypressed(cpu->registers.V[opcode]))
                cpu->registers.PC += 2;
            break;
        case 0xA1:
            // Is key up
            opcode = (opcode >> 8) & 0xF;
            disassemble("sknp v%01x", opcode);
            
            if(!graphics_keypressed(cpu->registers.V[opcode]))
                cpu->registers.PC += 2;
            break;
        default:
            goto unknown_opcode;
        }
        break;
    case 0xF000:
        switch(opcode & 0xFF) {
        case 0x07:
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld v%01x, DT", opcode);
            cpu->registers.V[opcode] = cpu->registers.DT;
            break;
        case 0x0A:
            opcode = (opcode >> 8) & 0xF;
            cpu->registers.V[opcode] = graphics_getkeypress();
            break;
        case 0x15:
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld DT, v%01x", opcode);
            cpu->registers.DT = cpu->registers.V[opcode];
            break;
        case 0x18:
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld ST, v%01x", opcode);
            cpu->registers.ST = cpu->registers.V[opcode];
            break;
        case 0x1E:
            opcode = (opcode >> 8) & 0xF;
            disassemble("add I, v%01x", opcode);
            
            cpu->registers.I += cpu->registers.V[opcode];
            
            // Set overflow flag
            if(cpu->registers.I < cpu->registers.V[opcode])
                cpu->registers.V[0xF] = 0;
            else
                cpu->registers.V[0xF] = 1;
            break;
        case 0x29:
            // Location of sprite for digit Vx
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld F, v%01x", opcode);
            
            cpu->registers.I = (cpu->registers.V[opcode] * SPRITE_LEN);
            cpu->registers.I += FONTSET_OFFSET;
            
            break;
        case 0x33:
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld B, v%01x", opcode);
            
            // Save in BCD format
            memory_write(cpu, cpu->registers.I,
                cpu->registers.V[opcode] / 100);
            memory_write(cpu, cpu->registers.I+1,
                (cpu->registers.V[opcode] % 100) / 10);
            memory_write(cpu, cpu->registers.I+2,
                cpu->registers.V[opcode] % 10);
            break;
        case 0x55:
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld [I], v%01x", opcode);
            for(int i = 0; i <= opcode; i++) {
                memory_write(cpu, cpu->registers.I + i, cpu->registers.V[i]);
            };
            break;
        case 0x65:
            opcode = (opcode >> 8) & 0xF;
            disassemble("ld v%01x, [I]", opcode);
            for(int i = 0; i <= opcode; i++) {
                cpu->registers.V[i] = memory_read(cpu, cpu->registers.I + i);
            };
            break;
        case 0x75:
            opcode = MIN((opcode >> 8) & 0xF, 7);
            disassemble("ld R, v%01x", opcode);
            for(int i = 0; i <= opcode; i++) {
                cpu->registers.R[i] = cpu->registers.V[i];
            }
            break;
        case 0x85:
            opcode = MIN((opcode >> 8) & 0xF, 7);
            disassemble("ld v%01x, R", opcode);
            for(int i = 0; i <= opcode; i++) {
                cpu->registers.V[i] = cpu->registers.R[i];
            }
            break;
        default:
            goto unknown_opcode;
        }
        break;
    default:
        goto unknown_opcode;
    }
    
    return;
    
unknown_opcode:
    printf("Unknown Opcode: 0x%04x\n", opcode);
    printf("\tLocation: 0x%04x\n", cpu->registers.PC - 2);
    exit(0);
}

int main(int argc, char ** argv) {
    if(argc < 2) {
        printf("usage: %s [program file]\n", argv[0]);
        return 0;
    }
    
    memory_load_program(&cpu, argv[1]);
    
    graphics_init();
    cpu_init(&cpu);
    
    clock_t start_time;
    while(cpu.registers.PC) {
        start_time = clock();
        
        if(graphics_update()) break;
        
        cpu_fde(&cpu);
        
        if(cpu.registers.DT)
            cpu.registers.DT--;
        
        while(clock() < start_time + 800);
    }
}
