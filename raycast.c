#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FOV 60
#define NUM_RAYS SCREEN_WIDTH

typedef struct {
    float x, y;
    float angle;
} Player;

void renderRays(SDL_Renderer* renderer, Player* player) {
    // Clear renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Raycasting logic
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        float rayAngle = player->angle - (FOV / 2) + ((float)i / NUM_RAYS) * FOV;
        float rayX = player->x + cos(rayAngle * M_PI / 180) * 100;
        float rayY = player->y + sin(rayAngle * M_PI / 180) * 100;
        
        // Draw ray
        SDL_RenderDrawLine(renderer, player->x, player->y, rayX, rayY);
    }

    // Present the updated renderer
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create main game window
    SDL_Window* mainWindow = SDL_CreateWindow("Main Game Window",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

    // Create raycasting output window
    SDL_Window* rayWindow = SDL_CreateWindow("Raycasting Output",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* rayRenderer = SDL_CreateRenderer(rayWindow, -1, SDL_RENDERER_ACCELERATED);

    Player player = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0 };

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        player.angle -= 5;
                        break;
                    case SDLK_RIGHT:
                        player.angle += 5;
                        break;
                    case SDLK_UP:
                        player.x += cos(player.angle * M_PI / 180) * 5;
                        player.y += sin(player.angle * M_PI / 180) * 5;
                        break;
                    case SDLK_DOWN:
                        player.x -= cos(player.angle * M_PI / 180) * 5;
                        player.y -= sin(player.angle * M_PI / 180) * 5;
                        break;
                }
            }
        }

        // Render main game window (can be used for the player view)
        SDL_SetRenderDrawColor(mainRenderer, 0, 0, 255, 255);
        SDL_RenderClear(mainRenderer);
        SDL_RenderPresent(mainRenderer);

        // Render raycasting output window
        renderRays(rayRenderer, &player);
    }

    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyRenderer(rayRenderer);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyWindow(rayWindow);
    SDL_Quit();

    return 0;
}
