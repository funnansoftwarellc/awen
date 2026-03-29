module;

#include <initializer_list>
#include <string>
#include <utility>

#include <awen/flecs.h>

export module awen.core.engine;

export import awen.graphics;
export import awen.scene;
export import awen.widgets;

export namespace awn::core
{
    /// @brief Owns the application window and drives the main game loop.
    class Engine
    {
    public:
        /// @brief Creates the application window and binds the engine to an existing ECS world.
        /// @param world ECS world that already imported all required modules.
        /// @param title Window title bar text.
        /// @param width Initial width in pixels.
        /// @param height Initial height in pixels.
        /// @param flags Optional window configuration flags.
        Engine(flecs::world& world, const char* title, int width, int height, std::initializer_list<graphics::ConfigFlag> flags = {})
            : window_{title, width, height, flags}, world_{world}
        {
        }

        ~Engine() = default;

        Engine(const Engine&) = delete;
        auto operator=(const Engine&) -> Engine& = delete;
        Engine(Engine&&) = delete;
        auto operator=(Engine&&) -> Engine& = delete;

        /// @brief Sets the target frame rate.
        /// @param fps Desired frames per second.
        static auto set_target_fps(int fps) -> void
        {
            graphics::Window::set_target_fps(fps);
        }

        /// @brief Sets the background colour used to clear each frame.
        /// @param color Colour written as a RenderClear command before scene commands.
        auto set_clear_color(graphics::Color color) -> void
        {
            clear_color_ = color;
        }

        /// @brief Returns the ECS world reference provided at construction.
        [[nodiscard]] auto raw_world() noexcept -> flecs::world&
        {
            return world_;
        }

        /// @brief Loads a texture from @p path or returns the cached TextureId if already loaded.
        [[nodiscard]] auto load_texture(const std::string& path) -> graphics::TextureId
        {
            return textures_.load(path);
        }

        /// @brief Registers an event handler forwarded to the underlying Window.
        template <typename F>
        auto on_event(F&& handler) -> void
        {
            window_.on_event(std::forward<F>(handler));
        }

        /// @brief Runs the main loop until the window is closed.
        /// @param on_update Callback invoked once per frame with the delta time in seconds.
        template <typename F>
        auto run(const F& on_update) -> void
        {
            while (graphics::Window::is_open())
            {
                window_.poll_events();
                const auto dt = graphics::Window::get_frame_time();

                on_update(dt);
                world_.progress(dt);

                draw_list_.clear();
                draw_list_.push(graphics::RenderClear{.color = clear_color_});
                widgets::build_draw_list(world_, textures_, draw_list_);

                graphics::Renderer::begin();
                graphics::Renderer::submit(draw_list_);
                graphics::Renderer::end();
            }
        }

    private:
        graphics::Window window_;
        flecs::world& world_;
        widgets::TextureCache textures_;
        graphics::DrawList draw_list_;
        graphics::Color clear_color_{};
    };
}
