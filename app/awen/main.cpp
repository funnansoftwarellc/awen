#include <raylib.h>
#include <cstdlib>

auto main() -> int
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Awen");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello, Awen!", 350, 200, 50, ORANGE);
        DrawFPS(1150, 700);
        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}