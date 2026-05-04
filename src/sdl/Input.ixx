module;

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>
#include <glm/vec2.hpp>

export module awen.sdl.input;

export namespace awen::sdl
{
    /// @brief Maximum scancode value tracked by KeyboardState.
    inline constexpr auto kMaxScancodes = static_cast<std::size_t>(SDL_SCANCODE_COUNT);

    /// @brief Per-frame keyboard state singleton.
    struct KeyboardState
    {
        std::array<bool, kMaxScancodes> down{};
        std::array<bool, kMaxScancodes> pressed{};
        std::array<bool, kMaxScancodes> released{};
        std::uint16_t modifiers{};

        /// @brief Whether the given scancode is currently held down.
        [[nodiscard]] auto isDown(SDL_Scancode code) const -> bool
        {
            return static_cast<std::size_t>(code) < down.size() && down[static_cast<std::size_t>(code)];
        }

        /// @brief Whether the given scancode transitioned from up to down this frame.
        [[nodiscard]] auto wasPressed(SDL_Scancode code) const -> bool
        {
            return static_cast<std::size_t>(code) < pressed.size() && pressed[static_cast<std::size_t>(code)];
        }

        /// @brief Whether the given scancode transitioned from down to up this frame.
        [[nodiscard]] auto wasReleased(SDL_Scancode code) const -> bool
        {
            return static_cast<std::size_t>(code) < released.size() && released[static_cast<std::size_t>(code)];
        }
    };

    /// @brief Per-frame mouse state singleton.
    struct MouseState
    {
        glm::vec2 position{};
        glm::vec2 delta{};
        glm::vec2 wheel{};
        std::uint32_t buttonsDown{};
        std::uint32_t buttonsPressed{};
        std::uint32_t buttonsReleased{};

        /// @brief Whether the given SDL mouse button (1..5) is currently held down.
        [[nodiscard]] auto isButtonDown(int button) const -> bool
        {
            return (buttonsDown & SDL_BUTTON_MASK(button)) != 0U;
        }

        /// @brief Whether the given SDL mouse button transitioned from up to down this frame.
        [[nodiscard]] auto wasButtonPressed(int button) const -> bool
        {
            return (buttonsPressed & SDL_BUTTON_MASK(button)) != 0U;
        }

        /// @brief Whether the given SDL mouse button transitioned from down to up this frame.
        [[nodiscard]] auto wasButtonReleased(int button) const -> bool
        {
            return (buttonsReleased & SDL_BUTTON_MASK(button)) != 0U;
        }
    };

    /// @brief Per-frame transient window-event flags.
    struct WindowEvents
    {
        bool quitRequested{};
        bool closeRequested{};
        bool resized{};
        bool focusGained{};
        bool focusLost{};
    };
}
