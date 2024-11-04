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
#define WALL_HEIGHT 4000  // Set wall height to 4000 for taller walls

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

// Check if the player can move to the new position
bool canMoveTo(Player* player, float deltaX, float deltaY) {
    float newX = player->x + deltaX;
    float newY = player->y + deltaY;
    return !isWall(newX, newY);
}

float castRay(Player* player, float rayAngle) {
    float rayX = player->x;
    float rayY = player->y;
    float distance = 0;

    while (!isWall(rayX, rayY) && distance < SCREEN_WIDTH) {
        rayX += cos(rayAngle * M_PI / 180);
        rayY += sin(rayAngle * M_PI / 180);
        distance += 1;
    }

    return distance;
}

void render3DView(SDL_Renderer* renderer, Player* player) {
    for (int i = 0; i < NUM_RAYS; i++) {
        float rayAngle = player->angle - (FOV / 2) + ((float)i / NUM_RAYS) * FOV;
        float distance = castRay(player, rayAngle);

        float wallHeight = WALL_HEIGHT / (distance * cos((rayAngle - player->angle) * M_PI / 180));
        int wallTop = (SCREEN_HEIGHT / 2) - (wallHeight / 2);
        int wallBottom = wallTop + wallHeight;

        int shade = 255 - (int)(distance * 255 / SCREEN_WIDTH);
        shade = shade < 0 ? 0 : shade;

        // Render ceiling (roof)
        SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);
        SDL_RenderDrawLine(renderer, i, 0, i, wallTop);

        // Render wall
        SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);
        SDL_RenderDrawLine(renderer, i, wallTop, i, wallBottom);

        // Render floor
        SDL_SetRenderDrawColor(renderer, 100, 50, 50, 255);
        SDL_RenderDrawLine(renderer, i, wallBottom, i, SCREEN_HEIGHT);
    }
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
    SDL_Window* mainWindow = SDL_CreateWindow("Raycasting Game",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

    // Create 3D view window
    SDL_Window* viewWindow = SDL_CreateWindow("3D View",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* viewRenderer = SDL_CreateRenderer(viewWindow, -1, SDL_RENDERER_ACCELERATED);

    Player player = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 0 };

    bool quit = false;
    const Uint8* keystate;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        keystate = SDL_GetKeyboardState(NULL);
        float deltaX = 0;
        float deltaY = 0;

        if (keystate[SDL_SCANCODE_LEFT]) {
            player.angle -= 5;
        }
        if (keystate[SDL_SCANCODE_RIGHT]) {
            player.angle += 5;
        }
        if (keystate[SDL_SCANCODE_UP]) {
            deltaX = cos(player.angle * M_PI / 180) * 3;
            deltaY = sin(player.angle * M_PI / 180) * 3;
            if (canMoveTo(&player, deltaX, deltaY)) {
                player.x += deltaX;
                player.y += deltaY;
            }
        }
        if (keystate[SDL_SCANCODE_DOWN]) {
            deltaX = -cos(player.angle * M_PI / 180) * 3;
            deltaY = -sin(player.angle * M_PI / 180) * 3;
            if (canMoveTo(&player, deltaX, deltaY)) {
                player.x += deltaX;
                player.y += deltaY;
            }
        }

        SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
        SDL_RenderClear(mainRenderer);

        renderMap(mainRenderer);
        SDL_SetRenderDrawColor(mainRenderer, 255, 0, 0, 255);
        SDL_Rect playerRect = { player.x - 5, player.y - 5, 10, 10 };
        SDL_RenderFillRect(mainRenderer, &playerRect);

        SDL_RenderPresent(mainRenderer);

        SDL_SetRenderDrawColor(viewRenderer, 0, 0, 0, 255);
        SDL_RenderClear(viewRenderer);
        render3DView(viewRenderer, &player);
        SDL_RenderPresent(viewRenderer);
    }

    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyRenderer(viewRenderer);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyWindow(viewWindow);
    SDL_Quit();

    return 0;
}
