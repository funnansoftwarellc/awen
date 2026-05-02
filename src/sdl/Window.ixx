module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <expected>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

export module awen.sdl.window;

import awen.sdl.event;

namespace
{
    auto MakeSdlWindowError(std::string_view message) -> std::string
    {
        return std::string{message} + ": " + SDL_GetError();
    }
}

export namespace awen::sdl
{
    struct WindowCreateInfo
    {
        const char* title{};
        int width{};
        int height{};
        int positionX{};
        int positionY{};
        bool resizable{};
        bool highDpi{};
    };

    struct PollResult
    {
        bool open{true};
        std::vector<Event> events;
    };

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

            // Constrain Android/iOS to landscape orientations.  Without this
            // hint SDL passes an empty orientation string to the Java side,
            // which then requests SCREEN_ORIENTATION_FULL_USER and lets the
            // device's rotation-lock setting override the manifest.
            SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

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

            // Use the requested size as a fixed logical coordinate space.  SDL
            // maps these coordinates to whatever the physical output actually
            // is (CSS-controlled canvas on WASM, resizable window on desktop)
            // with aspect-ratio-preserving letterboxing.
            std::ignore = SDL_SetRenderLogicalPresentation(nativeRenderer, createInfo.width, createInfo.height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

            screenWidth_ = createInfo.width;
            screenHeight_ = createInfo.height;
            open_ = true;

            return Window{nativeWindow, nativeRenderer};
        }

        ~Window()
        {
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

        auto pollEvents() -> std::expected<PollResult, std::string>
        {
            pressedKeys_.clear();
            releasedKeys_.clear();
            pointerPressed_ = false;
            pointerReleased_ = false;

            auto result = PollResult{.open = open_, .events = {}};
            auto event = SDL_Event{};

            const auto updatePointerLogical = [this](float windowX, float windowY)
            {
                auto logicalX = windowX;
                auto logicalY = windowY;
                std::ignore = SDL_RenderCoordinatesFromWindow(renderer_, windowX, windowY, &logicalX, &logicalY);
                pointerX_ = logicalX;
                pointerY_ = logicalY;
            };

            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_EVENT_QUIT:
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        open_ = false;
                        result.open = false;
                        break;

                    case SDL_EVENT_WINDOW_RESIZED:
                    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                        // The logical coordinate space is fixed at the design
                        // resolution.  SDL automatically re-maps it to the new
                        // physical output via its internal event watcher.
                        result.events.emplace_back(EventWindowResize{.width = screenWidth_, .height = screenHeight_});
                        break;

                    case SDL_EVENT_KEY_DOWN:
                    {
                        const auto key = static_cast<EventKeyboard::Key>(event.key.key);

                        if (!event.key.repeat)
                        {
                            pressedKeys_.insert(event.key.key);
                            result.events.emplace_back(EventKeyboard{.key = key, .type = EventKeyboard::Type::pressed});
                        }

                        downKeys_.insert(event.key.key);
                        break;
                    }

                    case SDL_EVENT_KEY_UP:
                    {
                        const auto key = static_cast<EventKeyboard::Key>(event.key.key);
                        releasedKeys_.insert(event.key.key);
                        downKeys_.erase(event.key.key);
                        result.events.emplace_back(EventKeyboard{.key = key, .type = EventKeyboard::Type::released});
                        break;
                    }

                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                        if (event.button.button == SDL_BUTTON_LEFT)
                        {
                            pointerDown_ = true;
                            pointerPressed_ = true;
                            updatePointerLogical(event.button.x, event.button.y);
                        }
                        break;

                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        if (event.button.button == SDL_BUTTON_LEFT)
                        {
                            pointerDown_ = false;
                            pointerReleased_ = true;
                            updatePointerLogical(event.button.x, event.button.y);
                        }
                        break;

                    case SDL_EVENT_MOUSE_MOTION:
                        updatePointerLogical(event.motion.x, event.motion.y);
                        break;

                    default:
                        break;
                }
            }

            return result;
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

        [[nodiscard]] static auto isPointerDown() -> bool
        {
            return pointerDown_;
        }

        [[nodiscard]] static auto isPointerPressed() -> bool
        {
            return pointerPressed_;
        }

        [[nodiscard]] static auto isPointerReleased() -> bool
        {
            return pointerReleased_;
        }

        [[nodiscard]] static auto getPointerX() -> float
        {
            return pointerX_;
        }

        [[nodiscard]] static auto getPointerY() -> float
        {
            return pointerY_;
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
                pointerDown_ = false;
                pointerPressed_ = false;
                pointerReleased_ = false;
                pointerX_ = 0.0F;
                pointerY_ = 0.0F;
                TTF_Quit();
                SDL_Quit();
                screenWidth_ = 0;
                screenHeight_ = 0;
                open_ = false;
            }
        }

        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;

        static inline int refCount_ = 0;                               // NOLINT(readability-identifier-naming)
        static inline bool open_ = false;                              // NOLINT(readability-identifier-naming)
        static inline int screenWidth_ = 0;                            // NOLINT(readability-identifier-naming)
        static inline int screenHeight_ = 0;                           // NOLINT(readability-identifier-naming)
        static inline std::unordered_set<SDL_Keycode> downKeys_{};     // NOLINT(readability-identifier-naming)
        static inline std::unordered_set<SDL_Keycode> pressedKeys_{};  // NOLINT(readability-identifier-naming)
        static inline std::unordered_set<SDL_Keycode> releasedKeys_{}; // NOLINT(readability-identifier-naming)
        static inline bool pointerDown_ = false;                       // NOLINT(readability-identifier-naming)
        static inline bool pointerPressed_ = false;                    // NOLINT(readability-identifier-naming)
        static inline bool pointerReleased_ = false;                   // NOLINT(readability-identifier-naming)
        static inline float pointerX_ = 0.0F;                          // NOLINT(readability-identifier-naming)
        static inline float pointerY_ = 0.0F;                          // NOLINT(readability-identifier-naming)
    };
}