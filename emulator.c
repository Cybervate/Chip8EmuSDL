#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define TICK_INTERVAL 60

uint8_t ram[4096];

uint16_t pc;
uint16_t opcode;

uint8_t display[32][64];

uint8_t v[0x10];

uint16_t stack[16];
uint16_t I;

uint8_t delayTimer;

uint8_t soundTimer;

// Font
unsigned char font[80] = 

{0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80};  // F 

void loadGame(char *game) {
    FILE *fgame;

    fgame = fopen(game, "rb");

    if(fgame == NULL) {
        printf("LOAD GAME FAILED");
        exit(42);
    }

    fread(&ram[0x200], 1, (0x1000 - 0x200), fgame);

    fclose(fgame);
}

void draw(uint8_t x, uint8_t y, uint8_t n) {
    int row = y;
    int col = x;

    v[0xf] = 0;
    for (int i = 0; i < n; i++) {
        uint8_t spriteData = ram[I + i];

        for (int j = 0; j < 8; j++) {
            uint8_t curBit = (spriteData >> j) & 0x1;

            uint8_t *displayp = &display[(row + i) % 32][(col + (7 - j)) % 64];

            if (curBit == 1 && *displayp == 1) {
                v[0xf] = 1;
            }
            *displayp = *displayp ^ curBit;
        }
    }

}

void cycle() {
    // int i;
    uint8_t x, y, n;
    uint8_t nn;
    uint16_t nnn;

    // fetch
    opcode = ram[pc] << 8 | ram[pc + 1];
    x   = (opcode >> 8) & 0x000F; // the lower 4 bits of the high byte
    y   = (opcode >> 4) & 0x000F; // the upper 4 bits of the low byte
    n   = opcode & 0x000F; // the lowest 4 bits
    nn  = opcode & 0x00FF; // the lowest 8 bits
    nnn = opcode & 0x0FFF; // the lowest 12 bits

    printf("PC: %x, OPCODE: %x", pc, opcode);

    pc += 2;

    switch (opcode & 0xF000) {
        case 0x0000:
            switch(nn) {
                case 0x00E0:
                    printf("clear screen");
                    memset(display, 0, sizeof(uint8_t) * (32*64));
                    break;
                case 0x00EE:
                    printf("return from a subroutine");
                    break;
            }
            break;
        case 0x1000:
            printf("jump to mem address: %x", nnn);
            pc = nnn;
            break;
        case 0x2000:
            break;
        case 0x3000:
            break;
        case 0x4000:
            break;
        case 0x5000:
            break;
        case 0x6000:
            printf("set register VX: %x", nn);
            v[x] = nn;
            break;
        case 0x7000:
            printf("add value to register vx: %x", nn);
            v[x] += nn;
            break;
        case 0x8000:
            break;
        case 0x9000:
            break;
        case 0xA000:
            printf("set index register I: %x", nnn);
            I = nnn;
            break;
        case 0xB000:
            break;
        case 0xC000:
            break;
        case 0xD000:
            printf("draw vx: %d, vy: %d, n: %d", v[x], v[y], n);
            draw(v[x], v[y], n);
            // draw(v[x], v[y], n);
            break;
        case 0xE000:
            break;
        case 0xF000:
            break;
        default:
            exit(42);
            break;
        }
        

    }

void init() {
    pc = 0x200;
    opcode = 0;
    I = 0;

    memset(ram, 0, sizeof(uint8_t) * 4096);
    memset(v, 0, sizeof(uint8_t) * 0x10);
    memset(display, 0, sizeof(uint8_t) * 32*64);
    memset(stack, 0, sizeof(uint16_t) * 16);

    delayTimer = 0;
    soundTimer = 0;

    for (int i = 0; i < 80; i++) {
        ram[0x050 + i] = font[i];
    }
}