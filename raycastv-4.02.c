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

    // Render the player
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for player
    SDL_Rect playerRect = { (int)(player->x - 5), (int)(player->y - 5), 10, 10 };
    SDL_RenderFillRect(renderer, &playerRect);

    // Render rays
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for rays
    for (int i = 0; i < NUM_RAYS; i++) {
        float rayAngle = player->angle - (FOV / 2) + ((float)i / NUM_RAYS) * FOV;
        float distance = castRay(player, rayAngle);
        float rayX = player->x + cos(rayAngle * M_PI / 180) * distance;
        float rayY = player->y + sin(rayAngle * M_PI / 180) * distance;
        SDL_RenderDrawLine(renderer, player->x, player->y, rayX, rayY);
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

void renderMapEditor(SDL_Renderer* renderer) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            SDL_Rect tileRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            if (map[y][x] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Wall (1) - Red
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Empty (0) - White
            }
            SDL_RenderFillRect(renderer, &tileRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black border
            SDL_RenderDrawRect(renderer, &tileRect);
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
    
    // Create map editor window
    SDL_Window* editorWindow = SDL_CreateWindow("Map Editor",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, SDL_WINDOW_SHOWN);
    SDL_Renderer* editorRenderer = SDL_CreateRenderer(editorWindow, -1, SDL_RENDERER_ACCELERATED);

    Player player = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 0 };

    bool quit = false;
    const Uint8* keystate;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Map editor mouse click handling
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                int gridX = mouseX / TILE_SIZE;
                int gridY = mouseY / TILE_SIZE;
                if (gridX >= 0 && gridX < MAP_WIDTH && gridY >= 0 && gridY < MAP_HEIGHT) {
                    // Toggle between wall (1) and empty (0)
                    map[gridY][gridX] = (map[gridY][gridX] == 1) ? 0 : 1;
                }
            }
        }

        keystate = SDL_GetKeyboardState(NULL);

        // Handle player movement
        float speed = 2.0f;
        if (keystate[SDL_SCANCODE_W]) {
            float deltaX = cos(player.angle * M_PI / 180) * speed;
            float deltaY = sin(player.angle * M_PI / 180) * speed;
            if (canMoveTo(&player, deltaX, deltaY)) {
                player.x += deltaX;
                player.y += deltaY;
            }
        }
        if (keystate[SDL_SCANCODE_S]) {
            float deltaX = -cos(player.angle * M_PI / 180) * speed;
            float deltaY = -sin(player.angle * M_PI / 180) * speed;
            if (canMoveTo(&player, deltaX, deltaY)) {
                player.x += deltaX;
                player.y += deltaY;
            }
        }
        if (keystate[SDL_SCANCODE_A]) {
            player.angle -= 2.0f; // Rotate left
        }
        if (keystate[SDL_SCANCODE_D]) {
            player.angle += 2.0f; // Rotate right
        }

        // Render the 3D view
        SDL_SetRenderDrawColor(viewRenderer, 0, 0, 0, 255);
        SDL_RenderClear(viewRenderer);
        render3DView(viewRenderer, &player);
        SDL_RenderPresent(viewRenderer);

        // Render the main game window
        SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
        SDL_RenderClear(mainRenderer);
        renderMap(mainRenderer);
        SDL_RenderPresent(mainRenderer);

        // Render the map editor
        SDL_SetRenderDrawColor(editorRenderer, 255, 255, 255, 255);
        SDL_RenderClear(editorRenderer);
        renderMapEditor(editorRenderer);
        SDL_RenderPresent(editorRenderer);
    }

    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyRenderer(viewRenderer);
    SDL_DestroyWindow(viewWindow);
    SDL_DestroyRenderer(editorRenderer);
    SDL_DestroyWindow(editorWindow);
    SDL_Quit();

    return 0;
}
