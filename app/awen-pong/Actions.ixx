module;

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <vector>

export module awen.pong.actions;

export namespace pong
{
    /// @brief Logical input action identifiers used by the pong app.
    /// @note Values are also used as indices into ActionMap::states.
    enum class Action : std::uint8_t
    {
        Quit,
        ToggleAi,
        LeftPaddleAxis,
        RightPaddleAxis,
        MouseDragLeftPaddle,

        Count,
    };

    /// @brief Binds a keyboard scancode to a digital Action.
    struct ButtonBinding
    {
        Action action{};
        SDL_Scancode key{};
    };

    /// @brief Binds two keyboard scancodes to a 1D axis Action.
    /// @note Pressing positive yields +1, negative yields -1, both/none yields 0.
    struct AxisBinding
    {
        Action action{};
        SDL_Scancode positive{};
        SDL_Scancode negative{};
    };

    /// @brief Binds a mouse button (1..5) to a digital Action.
    struct MouseButtonBinding
    {
        Action action{};
        int button{};
    };

    /// @brief Per-frame state of a single Action.
    /// @note `value` is 0/1 for buttons and [-1, 1] for axes.
    ///       `down`/`pressed`/`released` track digital edges.
    struct ActionState
    {
        float value{};
        bool down{};
        bool pressed{};
        bool released{};
    };

    /// @brief Singleton holding action bindings and per-frame state.
    /// @note Populated by the pong::Module ctor; refreshed each frame by
    ///       the ActionSync system before game systems read it.
    struct ActionMap
    {
        std::array<ActionState, static_cast<std::size_t>(Action::Count)> states{};
        std::vector<ButtonBinding> buttons;
        std::vector<AxisBinding> axes;
        std::vector<MouseButtonBinding> mouseButtons;

        /// @brief Whether the action is currently active (digital).
        [[nodiscard]] auto isDown(Action action) const -> bool
        {
            return states[static_cast<std::size_t>(action)].down;
        }

        /// @brief Whether the action transitioned from inactive to active this frame.
        [[nodiscard]] auto wasPressed(Action action) const -> bool
        {
            return states[static_cast<std::size_t>(action)].pressed;
        }

        /// @brief Whether the action transitioned from active to inactive this frame.
        [[nodiscard]] auto wasReleased(Action action) const -> bool
        {
            return states[static_cast<std::size_t>(action)].released;
        }

        /// @brief Current axis value in [-1, 1] (or 0/1 for digital actions).
        [[nodiscard]] auto axis(Action action) const -> float
        {
            return states[static_cast<std::size_t>(action)].value;
        }
    };
}
