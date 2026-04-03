module;

#include <raylib.h>
#include <string>
#include <vector>

export module awen.graphics.ecs.components;
export import awen.graphics.ecs.color;

export namespace awn::graphics::ecs
{
    struct DrawRect
    {
        float x;
        float y;
        float width;
        float height;
        Color color;
    };

    struct DrawCircle
    {
        float x;
        float y;
        float radius;
        Color color;
    };

    struct DrawText
    {
        std::string text;
        float x;
        float y;
        int font_size;
        Color color;
    };

    // struct DrawSprite
    // {
    //     TextureId texture_id;
    //     float x;
    //     float y;
    //     float width;
    //     float height;
    //     Color tint{255, 255, 255, 255};
    // };

    struct Window
    {
        enum class ConfigFlag : unsigned int
        {
            vsync = FLAG_VSYNC_HINT,
            fullscreen = FLAG_FULLSCREEN_MODE,
            resizable = FLAG_WINDOW_RESIZABLE,
            undecorated = FLAG_WINDOW_UNDECORATED,
            hidden = FLAG_WINDOW_HIDDEN,
            minimized = FLAG_WINDOW_MINIMIZED,
            maximized = FLAG_WINDOW_MAXIMIZED,
            unfocused = FLAG_WINDOW_UNFOCUSED,
            topmost = FLAG_WINDOW_TOPMOST,
            always_run = FLAG_WINDOW_ALWAYS_RUN,
            transparent = FLAG_WINDOW_TRANSPARENT,
            high_dpi = FLAG_WINDOW_HIGHDPI,
            mouse_passthrough = FLAG_WINDOW_MOUSE_PASSTHROUGH,
            borderless_windowed = FLAG_BORDERLESS_WINDOWED_MODE,
            msaa_4x = FLAG_MSAA_4X_HINT,
            interlaced = FLAG_INTERLACED_HINT,
        };

        std::string title{"Awen"};
        int width{800};
        int height{600};
        std::vector<ConfigFlag> flags{};
    };

    struct WindowRuntime
    {
        bool created{false};
        bool should_close{false};
    };
}