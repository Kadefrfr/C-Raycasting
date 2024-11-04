#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FOV 60
#define NUM_RAYS SCREEN_WIDTH
#define TILE_SIZE 64
#define MAP_WIDTH 10
#define MAP_HEIGHT 8

typedef struct {
    float x, y;
    float angle;
} Player;

// Simple 2D map where 1 represents a wall and 0 is empty space
int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Function to check for wall collision
bool isWall(int x, int y) {
    int mapX = x / TILE_SIZE;
    int mapY = y / TILE_SIZE;
    if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
        return map[mapY][mapX] == 1;
    }
    return false;
}

void renderRays(SDL_Renderer* renderer, Player* player) {
    // Clear renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Raycasting logic
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        float rayAngle = player->angle - (FOV / 2) + ((float)i / NUM_RAYS) * FOV;
        float rayX = player->x;
        float rayY = player->y;
        
        while (!isWall(rayX, rayY)) {
            rayX += cos(rayAngle * M_PI / 180);
            rayY += sin(rayAngle * M_PI / 180);
        }
        
        // Draw the ray until it hits a wall
        SDL_RenderDrawLine(renderer, player->x, player->y, rayX, rayY);
    }

    // Present the updated renderer
    SDL_RenderPresent(renderer);
}

void renderMap(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x] == 1) {
                SDL_Rect wallRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &wallRect);
            }
        }
    }
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

    Player player = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 0 };

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

        // Render map in main game window
        SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
        SDL_RenderClear(mainRenderer);
        renderMap(mainRenderer);

        // Draw player as a small circle
        SDL_SetRenderDrawColor(mainRenderer, 255, 0, 0, 255);
        SDL_Rect playerRect = { player.x - 5, player.y - 5, 10, 10 };
        SDL_RenderFillRect(mainRenderer, &playerRect);
        
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
