module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <expected>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>

export module awen.sdl.window;

import awen.core.engine;
import awen.sdl.event;

namespace
{
    auto MakeSdlWindowError(std::string_view message) -> std::string
    {
        return std::string{message} + ": " + SDL_GetError();
    }

    auto ToMouseButton(std::uint8_t button) -> awen::sdl::MouseButton
    {
        switch (button)
        {
            case SDL_BUTTON_LEFT:
                return awen::sdl::MouseButton::left;
            case SDL_BUTTON_RIGHT:
                return awen::sdl::MouseButton::right;
            case SDL_BUTTON_MIDDLE:
                return awen::sdl::MouseButton::middle;
            case SDL_BUTTON_X1:
                return awen::sdl::MouseButton::x1;
            case SDL_BUTTON_X2:
                return awen::sdl::MouseButton::x2;
            default:
                return awen::sdl::MouseButton::left;
        }
    }
}

export namespace awen::sdl
{
    /// @brief Creation parameters for an SDL-backed Window.
    struct WindowCreateInfo
    {
        const char* title{};
        int width{};
        int height{};
        int positionX{};
        int positionY{};
        bool resizable{};
        bool highDpi{};
        /// @brief When true, configure SDL_SetRenderLogicalPresentation with
        /// letterboxing so logical coordinates remain stable across resizes.
        /// When false, logical coordinates equal pixel coordinates and
        /// `getScreenWidth/Height` reflect the actual window size.
        bool useLogicalPresentation{true};
    };

    struct PollResult
    {
        bool open{true};
    };

    /// @brief RAII wrapper around an SDL window + renderer with a global event watch
    ///        that forwards translated input events to `Engine::dispatchEvent`.
    class Window
    {
    public:
        static auto create(const WindowCreateInfo& createInfo) -> std::expected<Window, std::string>
        {
            if (refCount_ == 0)
            {
                if (!SDL_Init(SDL_INIT_VIDEO))
                {
                    return std::unexpected(MakeSdlWindowError("Failed to initialize SDL video subsystem"));
                }

                if (!TTF_Init())
                {
                    SDL_Quit();
                    return std::unexpected(MakeSdlWindowError("Failed to initialize SDL_ttf"));
                }
            }

            ++refCount_;

            auto flags = SDL_WindowFlags{};

            if (createInfo.resizable)
            {
                flags |= SDL_WINDOW_RESIZABLE;
            }

            if (createInfo.highDpi)
            {
                flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
            }

            auto* nativeWindow = SDL_CreateWindow(createInfo.title, createInfo.width, createInfo.height, flags);

            if (nativeWindow == nullptr)
            {
                shutdownSdl();
                return std::unexpected(MakeSdlWindowError("Failed to create SDL window"));
            }

            // Best-effort: Emscripten and Android do not support window positioning.
            std::ignore = SDL_SetWindowPosition(nativeWindow, createInfo.positionX, createInfo.positionY);

            auto* nativeRenderer = SDL_CreateRenderer(nativeWindow, nullptr);

            if (nativeRenderer == nullptr)
            {
                SDL_DestroyWindow(nativeWindow);
                shutdownSdl();
                return std::unexpected(MakeSdlWindowError("Failed to create SDL renderer"));
            }

            std::ignore = SDL_SetRenderVSync(nativeRenderer, 1);

            useLogicalPresentation_ = createInfo.useLogicalPresentation;

            if (useLogicalPresentation_)
            {
                // Use the requested size as a fixed logical coordinate space.
                std::ignore =
                    SDL_SetRenderLogicalPresentation(nativeRenderer, createInfo.width, createInfo.height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
                screenWidth_ = createInfo.width;
                screenHeight_ = createInfo.height;
            }
            else
            {
                // Track actual window pixel size; updated on resize events.
                auto pixelW = 0;
                auto pixelH = 0;
                std::ignore = SDL_GetWindowSizeInPixels(nativeWindow, &pixelW, &pixelH);
                screenWidth_ = pixelW;
                screenHeight_ = pixelH;
            }

            open_ = true;
            ownedWindowId_ = SDL_GetWindowID(nativeWindow);

            // Register a global event watch.  Watches fire synchronously from
            // inside SDL's message pump, including during the Win32 modal
            // resize/move loop, so layout/render can keep up while the main
            // loop is blocked.
            SDL_AddEventWatch(&Window::eventWatch, nullptr);

            return Window{nativeWindow, nativeRenderer};
        }

        ~Window()
        {
            if (window_ != nullptr || renderer_ != nullptr)
            {
                SDL_RemoveEventWatch(&Window::eventWatch, nullptr);
            }

            if (renderer_ != nullptr)
            {
                SDL_DestroyRenderer(renderer_);
            }

            if (window_ != nullptr)
            {
                SDL_DestroyWindow(window_);
            }

            if (renderer_ != nullptr || window_ != nullptr)
            {
                shutdownSdl();
            }
        }

        Window(const Window&) = delete;
        auto operator=(const Window&) -> Window& = delete;

        Window(Window&& other) noexcept : window_{std::exchange(other.window_, nullptr)}, renderer_{std::exchange(other.renderer_, nullptr)}
        {
        }

        auto operator=(Window&& other) noexcept -> Window&
        {
            if (this != &other)
            {
                this->~Window();
                window_ = std::exchange(other.window_, nullptr);
                renderer_ = std::exchange(other.renderer_, nullptr);
            }

            return *this;
        }

        [[nodiscard]] auto renderer() const -> SDL_Renderer*
        {
            return renderer_;
        }

        [[nodiscard]] auto isOpen() const -> bool
        {
            return open_;
        }

        /// @brief Drives SDL's event pump.  Translation and dispatch happen
        ///        inside the registered watch callback, so this only flushes
        ///        the queue and reports the open state.
        auto pollEvents() -> std::expected<PollResult, std::string>
        {
            // Reset per-frame edge-triggered keyboard sets.  The watch
            // re-populates them as events flow through SDL_PumpEvents.
            pressedKeys_.clear();
            releasedKeys_.clear();

            SDL_PumpEvents();

            // Drain any queued events; the watch already dispatched them.
            auto event = SDL_Event{};

            while (SDL_PollEvent(&event))
            {
                // No-op: dispatch happened in the watch callback.
            }

            return PollResult{.open = open_};
        }

        [[nodiscard]] static auto isKeyDown(EventKeyboard::Key key) -> bool
        {
            return downKeys_.contains(static_cast<SDL_Keycode>(key));
        }

        [[nodiscard]] static auto isKeyPressed(EventKeyboard::Key key) -> bool
        {
            return pressedKeys_.contains(static_cast<SDL_Keycode>(key));
        }

        [[nodiscard]] static auto isKeyReleased(EventKeyboard::Key key) -> bool
        {
            return releasedKeys_.contains(static_cast<SDL_Keycode>(key));
        }

        [[nodiscard]] static auto getScreenWidth() -> int
        {
            return screenWidth_;
        }

        [[nodiscard]] static auto getScreenHeight() -> int
        {
            return screenHeight_;
        }

    private:
        Window(SDL_Window* nativeWindow, SDL_Renderer* nativeRenderer) : window_{nativeWindow}, renderer_{nativeRenderer}
        {
        }

        static auto shutdownSdl() -> void
        {
            --refCount_;

            if (refCount_ == 0)
            {
                downKeys_.clear();
                pressedKeys_.clear();
                releasedKeys_.clear();
                TTF_Quit();
                SDL_Quit();
                screenWidth_ = 0;
                screenHeight_ = 0;
                open_ = false;
                ownedWindowId_ = 0;
                useLogicalPresentation_ = true;
            }
        }

        // SDL event watch: fires synchronously from inside SDL_PumpEvents,
        // including during the Win32 modal resize loop.
        static auto SDLCALL eventWatch(void* /*userdata*/, SDL_Event* event) -> bool
        {
            if (event == nullptr)
            {
                return true;
            }

            // Filter out events for windows we don't own.
            if (event->type >= SDL_EVENT_WINDOW_FIRST && event->type <= SDL_EVENT_WINDOW_LAST)
            {
                if (ownedWindowId_ != 0 && event->window.windowID != ownedWindowId_)
                {
                    return true;
                }
            }

            translateAndDispatch(*event);
            return true;
        }

        static auto translateAndDispatch(const SDL_Event& event) -> void
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    open_ = false;
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                {
                    if (!useLogicalPresentation_)
                    {
                        // event.window.data1/data2 are window coords; query pixels for accuracy.
                        if (auto* win = SDL_GetWindowFromID(event.window.windowID); win != nullptr)
                        {
                            auto pixelW = 0;
                            auto pixelH = 0;
                            std::ignore = SDL_GetWindowSizeInPixels(win, &pixelW, &pixelH);
                            screenWidth_ = pixelW;
                            screenHeight_ = pixelH;
                        }
                    }

                    dispatch(EventWindowResize{.width = screenWidth_, .height = screenHeight_});
                    break;
                }

                case SDL_EVENT_KEY_DOWN:
                {
                    const auto key = static_cast<EventKeyboard::Key>(event.key.key);

                    if (!event.key.repeat)
                    {
                        pressedKeys_.insert(event.key.key);
                        dispatch(EventKeyboard{.key = key, .type = EventKeyboard::Type::pressed});
                    }

                    downKeys_.insert(event.key.key);
                    break;
                }

                case SDL_EVENT_KEY_UP:
                {
                    const auto key = static_cast<EventKeyboard::Key>(event.key.key);
                    releasedKeys_.insert(event.key.key);
                    downKeys_.erase(event.key.key);
                    dispatch(EventKeyboard{.key = key, .type = EventKeyboard::Type::released});
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    dispatch(EventMouseButton{
                        .button = ToMouseButton(event.button.button),
                        .type = EventMouseButton::Type::pressed,
                        .x = event.button.x,
                        .y = event.button.y,
                    });
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    dispatch(EventMouseButton{
                        .button = ToMouseButton(event.button.button),
                        .type = EventMouseButton::Type::released,
                        .x = event.button.x,
                        .y = event.button.y,
                    });
                    break;
                }

                case SDL_EVENT_MOUSE_MOTION:
                {
                    dispatch(EventMouseMotion{
                        .x = event.motion.x,
                        .y = event.motion.y,
                        .dx = event.motion.xrel,
                        .dy = event.motion.yrel,
                    });
                    break;
                }

                case SDL_EVENT_MOUSE_WHEEL:
                {
                    dispatch(EventMouseWheel{
                        .dx = event.wheel.x,
                        .dy = event.wheel.y,
                        .x = event.wheel.mouse_x,
                        .y = event.wheel.mouse_y,
                    });
                    break;
                }

                default:
                    break;
            }
        }

        template <typename T>
        static auto dispatch(T&& payload) -> void
        {
            if (auto* engine = awen::core::Engine::instance(); engine != nullptr)
            {
                engine->dispatchEvent(Event{std::forward<T>(payload)});
            }
        }

        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;

        static inline int refCount_ = 0;                               // NOLINT(readability-identifier-naming)
        static inline bool open_ = false;                              // NOLINT(readability-identifier-naming)
        static inline int screenWidth_ = 0;                            // NOLINT(readability-identifier-naming)
        static inline int screenHeight_ = 0;                           // NOLINT(readability-identifier-naming)
        static inline SDL_WindowID ownedWindowId_ = 0;                 // NOLINT(readability-identifier-naming)
        static inline bool useLogicalPresentation_ = true;             // NOLINT(readability-identifier-naming)
        static inline std::unordered_set<SDL_Keycode> downKeys_{};     // NOLINT(readability-identifier-naming)
        static inline std::unordered_set<SDL_Keycode> pressedKeys_{};  // NOLINT(readability-identifier-naming)
        static inline std::unordered_set<SDL_Keycode> releasedKeys_{}; // NOLINT(readability-identifier-naming)
    };
}
