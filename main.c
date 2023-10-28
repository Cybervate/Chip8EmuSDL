#include "emulator.c"

// Original chip 8 is 64x32, Scaling multiplies this by a set amount in order to make the game bigger, I set default to 16 which is 1024x512
#define SCALING 16 
#define WINDOW_WIDTH (64 * SCALING)
#define WINDOW_HEIGHT (32 * SCALING)

// fps
#define SPEED_MULTIPLIER 4
#define TICK_INTERVAL (60 * SPEED_MULTIPLIER)

enum Colors {White, Red, Green, Blue, Magenta};
enum Colors color = Magenta;
uint8_t colors[][4] = {
    {255, 255, 255, 255},
    {255, 0, 0, 255},
    {0, 255, 0, 255},
    {0, 0, 255, 255},
    {255, 0, 255, 255}
};

// set false to exit
uint8_t running;

SDL_Renderer *renderer;
SDL_Window *window;

int frameCount, timerFPS, lastFrame, fps;

const uint8_t *keystates = SDL_GetKeyboardState(NULL);

void drawPix(SDL_Renderer *renderer, int xPos, int yPos) {
    SDL_Rect rect = {xPos*SCALING, yPos*SCALING, SCALING, SCALING};
    SDL_RenderFillRect(renderer, &rect);
}

void input() {
    memset(keys, 0, sizeof keys);

    /*
    Input pad
    1 2 3 4
    q w e r
    a s d f
    z x c v
    */
    keys[0] = keystates[SDL_SCANCODE_1];
    keys[1] = keystates[SDL_SCANCODE_2];
    keys[2] = keystates[SDL_SCANCODE_3];
    keys[3] = keystates[SDL_SCANCODE_4];
    keys[4] = keystates[SDL_SCANCODE_Q];
    keys[5] = keystates[SDL_SCANCODE_W];
    keys[6] = keystates[SDL_SCANCODE_E];
    keys[7] = keystates[SDL_SCANCODE_R];
    keys[8] = keystates[SDL_SCANCODE_A];
    keys[9] = keystates[SDL_SCANCODE_S];
    keys[10] = keystates[SDL_SCANCODE_D];
    keys[11] = keystates[SDL_SCANCODE_F];
    keys[12] = keystates[SDL_SCANCODE_Z];
    keys[13] = keystates[SDL_SCANCODE_X];
    keys[14] = keystates[SDL_SCANCODE_C];
    keys[15] = keystates[SDL_SCANCODE_V];

    if (keystates[SDL_SCANCODE_ESCAPE]) {
        running = 0;
    }

    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            running = 0;
        }
    }
}

void drawSDL() {
    // draw background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect;
    rect.x=rect.y=0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;
    SDL_RenderFillRect(renderer, &rect);

    // loop through display buffer pixel map, and draw all pixels
    SDL_SetRenderDrawColor(renderer, colors[color][0], colors[color][1], colors[color][2], colors[color][3]);
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if(display[i][j]) {
                drawPix(renderer, j, i);
            }
        }
    }

    frameCount++;
    int timerFPS = SDL_GetTicks() - lastFrame;
    if(timerFPS < (1000/TICK_INTERVAL)) {
        SDL_Delay((1000/TICK_INTERVAL) - timerFPS);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {

    // initalize
    init();
    loadGame(argv[1]);

    running = 1;
    static int lastTime = 0;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("\nFailed at SDL_Init()\n");
    }
    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer) < 0) {
        printf("\nFailed at window and renderer\n");
    }
    SDL_SetWindowTitle(window, "Chip8Emu - cybervate");
    SDL_ShowCursor(1);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    int ticks = 0;

    // game loop
    while(running) {
        
        if (ticks == SPEED_MULTIPLIER - 1) {
            input();
            drawSDL();
            ticks = 0;
        }

        lastFrame = SDL_GetTicks();
        if(lastFrame >= (lastTime + 1000)) {
            lastTime = lastFrame;
            fps = frameCount;
            frameCount = 0;
        }

        printf("FPS: %d, %d\n", fps, fps/SPEED_MULTIPLIER);

        cycle();
        timerTick();
        ticks++;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // debug
    // for(int i = 0; i < 4096; i++) {
    //     printf("%x: %x ", i, ram[i]);
    // }
    // for(int i = 0; i < 64; i++) {
    //     for (int j = 0; j < 32; j++) {
    //         printf("%d", display[i][j]);
    //     }
    // }
    return 0;
}