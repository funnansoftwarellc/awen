module;

#include <functional>
#include <initializer_list>
#include <set>
#include <type_traits>
#include <vector>

#include <raylib.h>
#include <magic_enum/magic_enum.hpp>

export module awen.graphics.window;

export import awen.graphics.event;

export namespace awen::graphics
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

    class Window
    {
    public:
        Window(const char* title, int width, int height, std::initializer_list<ConfigFlag> flags = {}) : prev_width_{width}, prev_height_{height}
        {
            auto combined = 0U;
            for (const auto flag : flags)
            {
                combined |= static_cast<unsigned int>(flag);
            }
            SetConfigFlags(combined);
            InitWindow(width, height, title);
        }

        ~Window()
        {
            CloseWindow();
        }

        Window(const Window&) = delete;
        auto operator=(const Window&) -> Window& = delete;
        Window(Window&&) = delete;
        auto operator=(Window&&) -> Window& = delete;

        [[nodiscard]] static auto is_open() -> bool
        {
            return !WindowShouldClose();
        }

        static auto set_target_fps(int fps) -> void
        {
            SetTargetFPS(fps);
        }

        [[nodiscard]] static auto get_frame_time() -> float
        {
            return GetFrameTime();
        }

        [[nodiscard]] static auto get_screen_width() -> int
        {
            return GetScreenWidth();
        }

        [[nodiscard]] static auto get_screen_height() -> int
        {
            return GetScreenHeight();
        }

        [[nodiscard]] static auto is_key_down(EventKeyboard::Key key) -> bool
        {
            return IsKeyDown(static_cast<int>(key));
        }

        [[nodiscard]] static auto is_key_pressed(EventKeyboard::Key key) -> bool
        {
            return IsKeyPressed(static_cast<int>(key));
        }

        [[nodiscard]] static auto is_key_released(EventKeyboard::Key key) -> bool
        {
            return IsKeyReleased(static_cast<int>(key));
        }

        auto poll_events() -> void
        {
            poll_keyboard_events();
            poll_mouse_events();
            poll_touch_events();
            poll_joystick_events();
            poll_resize_events();
        }

        template <typename F>
        auto on_event(F&& handler) -> void
        {
            using EventType = callable_arg_t<F>;

            if constexpr (std::is_same_v<EventType, EventKeyboard>)
            {
                keyboard_handlers_.emplace_back(std::forward<F>(handler));
            }
            else if constexpr (std::is_same_v<EventType, EventMouse>)
            {
                mouse_handlers_.emplace_back(std::forward<F>(handler));
            }
            else if constexpr (std::is_same_v<EventType, EventTouch>)
            {
                touch_handlers_.emplace_back(std::forward<F>(handler));
            }
            else if constexpr (std::is_same_v<EventType, EventJoystick>)
            {
                joystick_handlers_.emplace_back(std::forward<F>(handler));
            }
            else if constexpr (std::is_same_v<EventType, EventWindowResize>)
            {
                resize_handlers_.emplace_back(std::forward<F>(handler));
            }
        }

    private:
        template <typename T>
        struct callable_arg;

        template <typename R, typename C, typename Arg>
        struct callable_arg<R (C::*)(Arg) const>
        {
            using type = std::decay_t<Arg>;
        };

        template <typename R, typename C, typename Arg>
        struct callable_arg<R (C::*)(Arg)>
        {
            using type = std::decay_t<Arg>;
        };

        template <typename F>
        using callable_arg_t = typename callable_arg<decltype(&std::decay_t<F>::operator())>::type;

        std::vector<std::function<void(const EventKeyboard&)>> keyboard_handlers_;
        std::vector<std::function<void(const EventMouse&)>> mouse_handlers_;
        std::vector<std::function<void(const EventTouch&)>> touch_handlers_;
        std::vector<std::function<void(const EventJoystick&)>> joystick_handlers_;
        std::vector<std::function<void(const EventWindowResize&)>> resize_handlers_;

        std::set<EventKeyboard::Key> pressed_keys_;
        std::set<int> prev_touches_;
        int prev_width_{};
        int prev_height_{};
        float prev_mouse_x_{};
        float prev_mouse_y_{};

        auto dispatch(const EventKeyboard& ev) -> void
        {
            for (const auto& handler : keyboard_handlers_)
            {
                handler(ev);
            }
        }

        auto dispatch(const EventMouse& ev) -> void
        {
            for (const auto& handler : mouse_handlers_)
            {
                handler(ev);
            }
        }

        auto dispatch(const EventTouch& ev) -> void
        {
            for (const auto& handler : touch_handlers_)
            {
                handler(ev);
            }
        }

        auto dispatch(const EventJoystick& ev) -> void
        {
            for (const auto& handler : joystick_handlers_)
            {
                handler(ev);
            }
        }

        auto dispatch(const EventWindowResize& ev) -> void
        {
            for (const auto& handler : resize_handlers_)
            {
                handler(ev);
            }
        }

        auto poll_keyboard_events() -> void
        {
            for (const auto key : magic_enum::enum_values<EventKeyboard::Key>())
            {
                if (IsKeyPressed(static_cast<int>(key)))
                {
                    pressed_keys_.insert(key);
                    dispatch(EventKeyboard{.key = key, .type = EventKeyboard::Type::pressed});
                }
                if (IsKeyReleased(static_cast<int>(key)))
                {
                    pressed_keys_.erase(key);
                    dispatch(EventKeyboard{.key = key, .type = EventKeyboard::Type::released});
                }
            }
        }

        auto poll_mouse_events() -> void
        {
            const auto pos = GetMousePosition();

            for (const auto btn : magic_enum::enum_values<EventMouse::MouseButton>())
            {
                if (IsMouseButtonPressed(static_cast<int>(btn)))
                {
                    dispatch(EventMouse{
                        .button = btn,
                        .type = EventMouse::Type::pressed,
                        .x = pos.x,
                        .y = pos.y,
                    });
                }

                if (IsMouseButtonReleased(static_cast<int>(btn)))
                {
                    dispatch(EventMouse{
                        .button = btn,
                        .type = EventMouse::Type::released,
                        .x = pos.x,
                        .y = pos.y,
                    });
                }
            }

            if (pos.x != prev_mouse_x_ || pos.y != prev_mouse_y_)
            {
                dispatch(EventMouse{
                    .button = {},
                    .type = EventMouse::Type::moved,
                    .x = pos.x,
                    .y = pos.y,
                });
                prev_mouse_x_ = pos.x;
                prev_mouse_y_ = pos.y;
            }

            const auto wheel = GetMouseWheelMoveV();
            if (wheel.x != 0.0F || wheel.y != 0.0F)
            {
                dispatch(EventMouse{
                    .button = {},
                    .type = EventMouse::Type::scrolled,
                    .x = pos.x,
                    .y = pos.y,
                    .scroll_x = wheel.x,
                    .scroll_y = wheel.y,
                });
            }
        }

        auto poll_touch_events() -> void
        {
            const auto count = GetTouchPointCount();
            auto current = std::set<int>{};

            for (auto idx = 0; idx < count; ++idx)
            {
                const auto id = GetTouchPointId(idx);
                const auto pos = GetTouchPosition(idx);
                current.insert(id);

                if (!prev_touches_.contains(id))
                {
                    dispatch(EventTouch{.id = id, .type = EventTouch::Type::pressed, .x = pos.x, .y = pos.y});
                }
                else
                {
                    dispatch(EventTouch{.id = id, .type = EventTouch::Type::moved, .x = pos.x, .y = pos.y});
                }
            }

            for (const auto id : prev_touches_)
            {
                if (!current.contains(id))
                {
                    dispatch(EventTouch{.id = id, .type = EventTouch::Type::released});
                }
            }

            prev_touches_ = current;
        }

        auto poll_joystick_events() -> void
        {
            constexpr auto max_gamepads = 4;

            for (auto gp = 0; gp < max_gamepads; ++gp)
            {
                if (!IsGamepadAvailable(gp))
                {
                    continue;
                }

                for (const auto btn : magic_enum::enum_values<EventJoystick::Button>())
                {
                    if (IsGamepadButtonPressed(gp, static_cast<int>(btn)))
                    {
                        dispatch(EventJoystick{
                            .id = gp,
                            .button = btn,
                            .type = EventJoystick::Type::pressed,
                            .axis_x = GetGamepadAxisMovement(gp, 0),
                            .axis_y = GetGamepadAxisMovement(gp, 1),
                        });
                    }
                    if (IsGamepadButtonReleased(gp, static_cast<int>(btn)))
                    {
                        dispatch(EventJoystick{
                            .id = gp,
                            .button = btn,
                            .type = EventJoystick::Type::released,
                            .axis_x = GetGamepadAxisMovement(gp, 0),
                            .axis_y = GetGamepadAxisMovement(gp, 1),
                        });
                    }
                }
            }
        }

        auto poll_resize_events() -> void
        {
            const auto w = GetScreenWidth();
            const auto h = GetScreenHeight();

            if (w != prev_width_ || h != prev_height_)
            {
                prev_width_ = w;
                prev_height_ = h;
                dispatch(EventWindowResize{.width = w, .height = h});
            }
        }
    };
}
