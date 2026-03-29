module;

#include <initializer_list>
#include <string>
#include <utility>

#include <awen/flecs.h>

export module awen.engine;

export import awen.graphics;
export import awen.scene;

export namespace awn
{
    /// @brief Owns the application window, the ECS world, and drives the main game loop.
    ///
    /// Encapsulates window creation, event polling, rendering, and scene-graph
    /// traversal. The caller builds entities via raw_world() and root(), registers
    /// event handlers with on_event(), and then enters the loop via run(), which
    /// invokes the user-supplied update callback and submits the scene draw list
    /// each frame.
    class Engine
    {
    public:
        /// @brief Creates the application window and initialises the ECS world.
        /// @param title  Window title bar text.
        /// @param width  Initial width in pixels.
        /// @param height Initial height in pixels.
        /// @param flags  Optional window configuration flags.
        Engine(const char* title, int width, int height, std::initializer_list<graphics::ConfigFlag> flags = {})
            : window_{title, width, height, flags}
        {
            world_.import <scene::SceneModule>();
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

        /// @brief Returns a reference to the underlying flecs world for direct entity creation and mutation.
        [[nodiscard]] auto raw_world() noexcept -> flecs::world&
        {
            return world_;
        }

        /// @brief Loads a texture from @p path or returns the cached TextureId if already loaded.
        /// @param path File path of the image to load.
        /// @return TextureId that can be passed to a DrawSprite component.
        [[nodiscard]] auto load_texture(const std::string& path) -> graphics::TextureId
        {
            return textures_.load(path);
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
        /// @param on_update Callback invoked once per frame with the delta time in seconds.
        template <typename F>
        auto run(const F& on_update) -> void
        {
            while (graphics::Window::is_open())
            {
                window_.poll_events();
                const auto dt = graphics::Window::get_frame_time();

                on_update(dt);

                // Run registered systems (PropagateWorldTransforms and any user-added systems).
                world_.progress(dt);

                draw_list_.clear();
                draw_list_.push(graphics::RenderClear{.color = clear_color_});
                scene::build_draw_list(world_, textures_, draw_list_);

                graphics::Renderer::begin();
                graphics::Renderer::submit(draw_list_);
                graphics::Renderer::end();
            }
        }

    private:
        // window_ must be declared first so the OpenGL context is available for the
        // full lifetime of textures_, which calls UnloadTexture in its destructor.
        graphics::Window window_;
        flecs::world world_;
        scene::TextureCache textures_;
        graphics::DrawList draw_list_;
        graphics::Color clear_color_{};
    };
}
