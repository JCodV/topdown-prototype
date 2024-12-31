#include "raylib.h"
#include "raymath.h"

struct Player {
    Vector2 position;
    Vector2 velocity;
};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "boss-rush");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
