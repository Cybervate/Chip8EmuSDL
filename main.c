#include "emulator.c"


#define SCALING 16 // Original chip 8 is 64x32, Scaling multiplies this by a set amount in order to make the game bigger, I set default to 16 which is 1024x512
#define WINDOW_WIDTH (64 * SCALING)
#define WINDOW_HEIGHT (32 * SCALING)

// fps
#define TICK_INTERVAL 60

uint8_t running;
SDL_Renderer *renderer;
SDL_Window *window;

int frameCount, timerFPS, lastFrame, fps;

void drawPix(SDL_Renderer *renderer, int xPos, int yPos) {
    // for (int i = 0; i < SCALING; i++) {
    //     for (int j = 0; j < SCALING; j++) {
    //         SDL_RenderDrawPoint(renderer, (SCALING*xPos) + i, (SCALING*yPos) + j);
    //     }
    // }
    // SDL_Rect rect = {xPos*SCALING, yPos*SCALING, SCALING, SCALING};

    SDL_Rect rect = {xPos*SCALING, yPos*SCALING, SCALING, SCALING};
    SDL_RenderFillRect(renderer, &rect);
}

void update() {

}

void input() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            running = 0;
        }
    }
    const uint8_t *keystates = SDL_GetKeyboardState(NULL);
    if (keystates[SDL_SCANCODE_ESCAPE]) {
        running = 0;
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
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if(display[i][j]) {
                drawPix(renderer, j, i);
            }
        }
    }

    frameCount++;
    int timeFPS = SDL_GetTicks() - lastFrame;
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
    
    // game loop
    while(running) {
        cycle();

        lastFrame = SDL_GetTicks();
        if(lastFrame >= (lastTime + 1000)) {
            lastTime = lastFrame;
            fps = frameCount;
            frameCount = 0;
        }

        printf("FPS: %d\n", fps);

        update();
        input();
        drawSDL();

        
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