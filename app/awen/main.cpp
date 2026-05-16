#include <raylib.h>
#include <cstdlib>

auto main() -> int
{
    constexpr auto width{1280};
    constexpr auto height{720};
    constexpr auto fontSize{50};
    constexpr auto textPosX{350};
    constexpr auto testPosY{200};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "Awen");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello, Awen!", textPosX, testPosY, fontSize, ORANGE);
        DrawFPS(0, 0);
        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}