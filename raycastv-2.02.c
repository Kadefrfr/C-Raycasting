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
#define WALL_HEIGHT 3000  // Base height for wall rendering

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

float castRay(Player* player, float rayAngle) {
    float rayX = player->x;
    float rayY = player->y;
    float distance = 0;

    // Increment ray until it hits a wall or goes out of bounds
    while (!isWall(rayX, rayY) && distance < SCREEN_WIDTH) {
        rayX += cos(rayAngle * M_PI / 180);
        rayY += sin(rayAngle * M_PI / 180);
        distance += 1;
    }

    return distance;
}

void renderRays(SDL_Renderer* renderer, Player* player) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        float rayAngle = player->angle - (FOV / 2) + ((float)i / NUM_RAYS) * FOV;
        float distance = castRay(player, rayAngle);

        // Draw the ray on the main game window
        float endX = player->x + cos(rayAngle * M_PI / 180) * distance;
        float endY = player->y + sin(rayAngle * M_PI / 180) * distance;
        SDL_RenderDrawLine(renderer, player->x, player->y, endX, endY);
    }
}

void render3DView(SDL_Renderer* renderer, Player* player) {
    for (int i = 0; i < NUM_RAYS; i++) {
        float rayAngle = player->angle - (FOV / 2) + ((float)i / NUM_RAYS) * FOV;
        float distance = castRay(player, rayAngle);

        // Wall height inversely proportional to distance
        float wallHeight = WALL_HEIGHT / (distance * cos((rayAngle - player->angle) * M_PI / 180));

        // Calculate the position to center the wall strip vertically
        int wallTop = (SCREEN_HEIGHT / 2) - (wallHeight / 2);
        int wallBottom = wallTop + wallHeight;

        // Set color based on distance for a basic shading effect
        int shade = 255 - (int)(distance * 255 / SCREEN_WIDTH);
        shade = shade < 0 ? 0 : shade;  // Ensure shading value doesn't go below 0
        SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);

        // Render the wall strip
        SDL_RenderDrawLine(renderer, i, wallTop, i, wallBottom);
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

        // Clear main window
        SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
        SDL_RenderClear(mainRenderer);

        // Render map and player in main game window
        renderMap(mainRenderer);
        SDL_SetRenderDrawColor(mainRenderer, 255, 0, 0, 255);
        SDL_Rect playerRect = { player.x - 5, player.y - 5, 10, 10 };
        SDL_RenderFillRect(mainRenderer, &playerRect);

        // Render rays in main window
        renderRays(mainRenderer, &player);
        SDL_RenderPresent(mainRenderer);

        // Clear 3D view window and render 3D walls
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
