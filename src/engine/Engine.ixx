module;

#include <initializer_list>
#include <utility>

#include <flecs.h>

export module awen.engine;

export import awen.graphics;
export import awen.scene;

export namespace awn
{
    /// @brief Owns the application window and drives the main game loop.
    ///
    /// Encapsulates window creation, event polling, rendering, and scene-graph
    /// traversal.  The caller registers event handlers with on_event() and then
    /// enters the loop via run(), which invokes the user-supplied update callback
    /// and submits the scene's draw list each frame.
    class Engine
    {
    public:
        /// @brief Creates the application window.
        /// @param title  Window title bar text.
        /// @param width  Initial width in pixels.
        /// @param height Initial height in pixels.
        /// @param flags  Optional window configuration flags.
        Engine(const char* title, int width, int height, std::initializer_list<graphics::ConfigFlag> flags = {})
            : window_{title, width, height, flags}
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
        /// @param color Colour written as a DrawClear command before scene commands.
        auto set_clear_color(graphics::Color color) -> void
        {
            clear_color_ = color;
        }

        /// @brief Registers an event handler forwarded to the underlying Window.
        /// @param handler Callable accepting a single event struct parameter.
        template <typename F>
        auto on_event(F&& handler) -> void
        {
            window_.on_event(std::forward<F>(handler));
        }

        /// @brief Runs the main loop until the window is closed.
        ///
        /// Each iteration polls events, calls @p on_update with the frame delta
        /// time, then builds the scene draw list and submits it to the renderer.
        /// @param scene     Scene whose draw list is built and submitted each frame.
        /// @param on_update Callback invoked once per frame with the delta time in seconds.
        template <typename F>
        auto run(scene::Scene& scene, const F& on_update) -> void
        {
            while (graphics::Window::is_open())
            {
                window_.poll_events();
                const auto dt = graphics::Window::get_frame_time();

                on_update(dt);

                draw_list_.clear();
                draw_list_.push(graphics::DrawClear{.color = clear_color_});
                scene.build_draw_list(draw_list_);

                graphics::Renderer::begin();
                graphics::Renderer::submit(draw_list_);
                graphics::Renderer::end();
            }
        }

    private:
        graphics::Window window_;
        graphics::DrawList draw_list_;
        graphics::Color clear_color_{};
    };
}
