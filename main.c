#include "emulator.c"
#include <string.h>

// Original chip 8 is 64x32, Scaling multiplies this by a set amount in order to make the game bigger, I set default to 16 which is 1024x512
uint8_t scaling = 16;
#define WINDOW_WIDTH (64 * scaling)
#define WINDOW_HEIGHT (32 * scaling)

// fps
unsigned int speedMultipler = 4;
unsigned int tickInterval = 60 * speedMultipler;

uint8_t mute = 0;

enum Colors {White, Red, Green, Blue, Magenta};
enum Colors color = White;
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
    SDL_Rect rect = {xPos*scaling, yPos*scaling, scaling, scaling};
    SDL_RenderFillRect(renderer, &rect);
}

void playBeep(Mix_Chunk * beep) {
    if (!mute) Mix_PlayChannel(-1, beep, 0);
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
    keys[0]     = keystates[SDL_SCANCODE_1];
    keys[1]     = keystates[SDL_SCANCODE_2];
    keys[2]     = keystates[SDL_SCANCODE_3];
    keys[3]     = keystates[SDL_SCANCODE_4];
    keys[4]     = keystates[SDL_SCANCODE_Q];
    keys[5]     = keystates[SDL_SCANCODE_W];
    keys[6]     = keystates[SDL_SCANCODE_E];
    keys[7]     = keystates[SDL_SCANCODE_R];
    keys[8]     = keystates[SDL_SCANCODE_A];
    keys[9]     = keystates[SDL_SCANCODE_S];
    keys[10]    = keystates[SDL_SCANCODE_D];
    keys[11]    = keystates[SDL_SCANCODE_F];
    keys[12]    = keystates[SDL_SCANCODE_Z];
    keys[13]    = keystates[SDL_SCANCODE_X];
    keys[14]    = keystates[SDL_SCANCODE_C];
    keys[15]    = keystates[SDL_SCANCODE_V];

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
    if(timerFPS < (1000/tickInterval)) {
        SDL_Delay((1000/tickInterval) - timerFPS);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {

    // initalize
    init();
    loadGame(argv[1]);

    // Handle command line args
    if (argc > 2) {
        for(int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-m") == 0) {
                mute = 1;
            }
            else if(strcmp(argv[i], "-c") == 0) {
                if (argv[i + 1]) {
                    if(strcmp(strlwr(argv[i + 1]), "white") == 0) {
                        color = White;
                    }
                    else if(strcmp(strlwr(argv[i + 1]), "red") == 0) {
                        color = Red;
                    }
                    else if(strcmp(strlwr(argv[i + 1]), "green") == 0) {
                        color = Green;
                    }
                    else if(strcmp(strlwr(argv[i + 1]), "blue") == 0) {
                        color = Blue;
                    }
                    else if(strcmp(strlwr(argv[i + 1]), "magenta") == 0) {
                        color = Magenta;
                    }
                }
            }
            else if(strcmp(argv[i], "--speed") == 0) {
                if (argv[i + 1]) speedMultipler = atoi(argv[i + 1]);
            }
            else if(strcmp(argv[i], "--scaling") == 0) {
                if (argv[i + 1]) scaling = atoi(argv[i + 1]);
            }
            else if(strcmp(argv[i], "--fps") == 0) {
                if (argv[i + 1]) tickInterval = atoi(argv[i + 1]) * speedMultipler;
            }
        }
    }

    running = 1;
    static int lastTime = 0;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("\nFailed at SDL_Init()\n");
    }
    if (Mix_Init(0) < 0) {
        printf("\nFailed at Mix_Init\n");
    }
    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer) < 0) {
        printf("\nFailed at window and renderer\n");
    }
    SDL_SetWindowTitle(window, "Chip8Emu - cybervate");
    SDL_ShowCursor(1);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_Chunk * beep = Mix_LoadWAV("etherhit.wav");
    if (!beep) printf("loading sound effect failed");

    int ticks = 0;

    // game loop
    while(running) {
        
        if (ticks == speedMultipler - 1) {
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

        printf("FPS: %d, %d\n", fps, fps/speedMultipler);

        cycle();
        timerTick();

        if (soundFlag) {
            playBeep(beep);
            soundFlag = 0;
        }

        ticks++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeChunk(beep);
    Mix_CloseAudio();

    SDL_Quit();

    return 0;
}