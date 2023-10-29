#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FONT_LOCATION 0x050

// system ram 4kb
uint8_t ram[4096];

uint16_t pc;
uint16_t opcode;

// display buffer, pixel map
uint8_t display[32][64];

// variable registers
uint8_t v[0x10];

uint16_t stack[0x10];
uint16_t I;

uint8_t delayTimer;
uint8_t soundTimer;

/*
Input pad
1 2 3 4
q w e r
a s d f
z x c v
*/
uint8_t keys[0x10];

// Font
unsigned char font[80] = 
{0xF0, 0x90, 0x90, 0x90, 0xF0, // 0x0
0x20, 0x60, 0x20, 0x20, 0x70,
0xF0, 0x10, 0xF0, 0x80, 0xF0,
0xF0, 0x10, 0xF0, 0x10, 0xF0,
0x90, 0x90, 0xF0, 0x10, 0x10,
0xF0, 0x80, 0xF0, 0x10, 0xF0,
0xF0, 0x80, 0xF0, 0x90, 0xF0,
0xF0, 0x10, 0x20, 0x40, 0x40,
0xF0, 0x90, 0xF0, 0x90, 0xF0,
0xF0, 0x90, 0xF0, 0x10, 0xF0,
0xF0, 0x90, 0xF0, 0x90, 0x90,
0xE0, 0x90, 0xE0, 0x90, 0xE0,
0xF0, 0x80, 0x80, 0x80, 0xF0,
0xE0, 0x90, 0x90, 0x90, 0xE0,
0xF0, 0x80, 0xF0, 0x80, 0xF0,
0xF0, 0x80, 0xF0, 0x80, 0x80}; // 0xf

// input file from argv
void loadGame(char *game) {
    FILE *fgame;

    fgame = fopen(game, "rb");

    if(fgame == NULL) {
        printf("LOAD GAME FAILED");
        exit(42);
    }

    // put game into ram at 0x200
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

            uint8_t *displayp = &display[(row + i) % 32][(col + (6 - j)) % 64];

            if (curBit == 1 && *displayp == 1) {
                v[0xf] = 1;
            }
            *displayp = *displayp ^ curBit;
        }
    }

}

void unknownInstruction() {
    // TODO
    exit(42);
}

void cycle() {
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

    // debug to show current instruction
    printf("PC: %x, OPCODE: %x", pc, opcode);

    // go to next set of instructions
    pc += 2;

    // instruction list
    switch (opcode & 0xF000) {
        case 0x0000:
            switch(nn) {
                case 0x00E0:
                    printf("clear screen");
                    memset(display, 0, sizeof(uint8_t) * (32*64));
                    break;
                case 0x00EE:
                    printf("return from a subroutine");
                    pc = stack[0];
                    break;
            }
            break;
        case 0x1000:
            printf("jump to mem address: %x", nnn);
            pc = nnn;
            break;
        case 0x2000:
            printf("Go to address: %x", nnn);
            stack[0] = pc;
            pc = nnn;
            break;
        case 0x3000:
            printf("Skip if vx: %x == nn: %x", v[x], nn);
            if (v[x] == nn) pc += 2;
            break;
        case 0x4000:
            printf("Skip if vx: %x != nn: %x", v[x], nn);
            if (v[x] != nn) pc += 2;
            break;
        case 0x5000:
            printf("Skip if vx: %x == vy: %x", v[x], v[y]);
            if (v[x] == v[y]) pc += 2;
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
            switch (n) {
                case 0x0:
                    printf("Set VX: %x, to VY: %x", v[x], v[y]);
                    v[x] = v[y];
                    break;
                case 0x1:
                    printf("VX | VY");
                    v[x] = v[x] | v[y];
                    break;
                case 0x2:
                    printf("VX & VY");
                    v[x] = v[x] & v[y];
                    break;
                case 0x3:
                    printf("VX ^ VY");
                    v[x] = v[x] ^ v[y];
                    break;
                case 0x4:
                    printf("Set VX to VX: %x + VY: %x", v[x], v[y]);
                    if (v[x] + v[y] > 255) v[0xf] = 1;
                    else v[0xf] = 0;
                    v[x] = v[x] + v[y];
                    break;
                case 0x5:
                    printf("Set VX to VX: %x - VY: %x", v[x], v[y]);
                    if (v[x] > v[y]) v[0xf] = 1;
                    else v[0xf] = 0;
                    v[x] = v[x] - v[y];
                    break;
                case 0x7:
                    printf("Set VX to VY: %x - VX: %x", v[y], v[x]);
                    if (v[y] > v[x]) v[0xf] = 1;
                    else v[0xf] = 0l;
                    v[x] = v[y] - v[x];
                    break;
                case 0x6:
                    // TODO 0x6 and 0xe ambiguos
                    printf("Shift right");
                    v[x] = v[y];
                    v[0xf] = v[x] & 1;
                    v[x] = v[x] >> 1;
                    break;
                case 0xe:
                    printf("Shift left");
                    v[x] = v[y];
                    v[0xf] = (v[x] >> 7) & 1;
                    v[x] = v[x] << 1;
                    break;
            }
            break;
        case 0x9000:
            printf("Skip if vx: %x != vy: %x", v[x], v[y]);
            if (v[x] != v[y]) pc += 2;
            break;
        case 0xA000:
            printf("set index register I: %x", nnn);
            I = nnn;
            break;
        case 0xB000:
            // TODO some implementations may not accounted for
            pc = nnn + v[0];
            break;
        case 0xC000:
            v[x] = rand() & nn;
            break;
        case 0xD000:
            printf("draw vx: %d, vy: %d, n: %d", v[x], v[y], n);
            draw(v[x], v[y], n);
            break;
        case 0xE000:
            switch (nn) {
                case 0x9E:
                    if (keys[v[x]]) pc += 2;
                    break;
                case 0xA1:
                    if(!keys[v[x]]) pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch (nn) {
                case 0x07:
                    printf("VX set to delayTimer: %x", delayTimer);
                    v[x] = delayTimer;
                    break;
                case 0x15:
                    printf("delayTimer set to VX: %x", v[x]);
                    delayTimer = v[x];
                    break;
                case 0x18:
                    printf("soundTimer set to VX: %x", v[x]);
                    soundTimer = v[x];
                    break;
                case 0x1E:
                    printf("Add to Index VX: %x", v[x]);
                    if (I + v[x > 0xfff]) v[0xf] = 1;
                    else v[0xf] = 0;
                    I += v[x];
                    break;
                case 0x0a:
                    printf("Get Key");
                    // TODO should activate on release, not press
                    for (int i = 0; i < 16; i++) {
                        if (keys[i]) {
                            v[x] = i;
                            goto keyBreak;
                        }
                    }
                    pc -= 2;
                    keyBreak:
                    break;
                case 0x29:
                    printf("Font character");
                    I = FONT_LOCATION + (5 * v[x]);
                    break;
                case 0x33:
                    printf("Binary Coded Decimal Conversion");
                    ram[I] = v[x] / 100;
                    ram[I + 1] = (v[x] % 100) / 10;
                    ram[I + 2] = v[x] % 10;
                    break;
                case 0x55:
                    // 0x55 and 0x65 are ambiguos
                    for (int i = 0; i <= x; i++) {
                        ram[I + i] = v[i];
                    }
                    break;
                case 0x65:
                    for (int i = 0; i <= x; i++) {
                        v[i] = ram[I + i];
                    }
                    break;
            }
            break;
        default:
            unknownInstruction();
            break;
        }
        
    }

void timerTick() {
    if (delayTimer > 0) {
        delayTimer--;
    }
    if (soundTimer > 0) {
        soundTimer--;
        if (!soundTimer)
        {
            // TODO
            printf("SOUNDTIMERWENTOFF");
        }
        
    }
    
}

void init() {
    pc = 0x200;
    opcode = 0;
    I = 0;

    srand(time(NULL)); 

    memset(ram, 0, sizeof(uint8_t) * 4096);
    memset(v, 0, sizeof(uint8_t) * 0x10);
    memset(display, 0, sizeof(uint8_t) * 32*64);
    memset(stack, 0, sizeof(uint16_t) * 16);

    delayTimer = 0;
    soundTimer = 0;

    // load font into memory
    for (int i = 0; i < 80; i++) {
        ram[FONT_LOCATION + i] = font[i];
    }
}