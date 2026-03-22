#include <cstdlib>

#include <raylib.h>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables, readability-identifier-naming)

auto main() -> int
{
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Hello Awen - Triangle");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTriangle({screenWidth / 2.0f, 50.0f}, {screenWidth / 2.0f - 150.0f, screenHeight - 100.0f},
                     {screenWidth / 2.0f + 150.0f, screenHeight - 100.0f}, RED);

        DrawText("Hello, Awen!", screenWidth / 2 - MeasureText("Hello, Awen!", 20) / 2, 20, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return EXIT_SUCCESS;
}

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables, readability-identifier-naming)