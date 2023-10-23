#include "emulator.c"

uint8_t running;
SDL_Renderer *renderer;
SDL_Window *window;

int frameCount, timerFPS, lastFrame, fps;

void drawPix(SDL_Renderer *renderer, int xPos, int yPos) {
    int xScaled = 16 * xPos;
    int yScaled = 16 * yPos;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            SDL_RenderDrawPoint(renderer, xScaled + i, yScaled+ j);
        }
    }
    
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect;
    rect.x=rect.y=0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // drawPix(renderer, 0, 0);
    // SDL_RenderDrawPoint(renderer, 10, 10);

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            // printf("DISPLAY: %d\n", display[i][j]);
            if(display[i][j] > 0) {
                drawPix(renderer, j, i);
            }
        }
    }

    frameCount++;
    int timeFPS = SDL_GetTicks() - lastFrame;
    if(timerFPS < (1000/60)) {
        SDL_Delay((1000/60) - timerFPS);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {

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