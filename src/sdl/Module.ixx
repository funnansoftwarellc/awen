module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <awen/flecs/Flecs.hpp>

#include <glm/vec2.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

export module awen.sdl;

export import awen.sdl.color;
export import awen.sdl.drawables;
export import awen.sdl.input;
export import awen.sdl.phases;
export import awen.sdl.renderer;
export import awen.sdl.resources;
export import awen.sdl.singletons;
export import awen.sdl.transform;
export import awen.sdl.window;

namespace awen::sdl::detail
{
    /// @brief Translate awen window flags to SDL window flags.
    auto toSdlFlags(const Window& window) -> SDL_WindowFlags
    {
        auto flags = SDL_WindowFlags{};

        if (window.flags.test(Window::Flags::Resizable))
        {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        if (window.flags.test(Window::Flags::HighPixelDensity))
        {
            flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
        }

        return flags;
    }

    /// @brief Iteratively propagate WorldTransform from roots through children.
    auto propagateTransforms(const flecs::query<const LocalTransform>& localQuery) -> void
    {
        auto stack = std::vector<std::pair<flecs::entity, WorldTransform>>{};

        localQuery.each(
            [&](flecs::entity entity, const LocalTransform&)
            {
                const auto parent = entity.parent();
                const auto isRoot = !parent.is_valid() || parent.try_get<LocalTransform>() == nullptr;

                if (isRoot)
                {
                    stack.emplace_back(entity, WorldTransform{});
                }
            });

        while (!stack.empty())
        {
            const auto [entity, parentWorld] = stack.back();
            stack.pop_back();

            auto myWorld = parentWorld;

            if (const auto* local = entity.try_get<LocalTransform>())
            {
                myWorld = compose(parentWorld, *local);
                entity.set<WorldTransform>(myWorld);
            }

            entity.children([&](flecs::entity child) { stack.emplace_back(child, myWorld); });
        }
    }

    /// @brief Pump SDL events and update the input + window-event singletons.
    auto pumpEvents(const flecs::world& world) -> void
    {
        auto& keyboard = world.ensure<KeyboardState>();
        auto& mouse = world.ensure<MouseState>();
        auto& windowEvents = world.ensure<WindowEvents>();
        auto& appState = world.ensure<AppState>();

        std::ranges::fill(keyboard.pressed, false);
        std::ranges::fill(keyboard.released, false);
        mouse.delta = glm::vec2{};
        mouse.wheel = glm::vec2{};
        mouse.buttonsPressed = 0;
        mouse.buttonsReleased = 0;
        windowEvents = WindowEvents{};

        auto event = SDL_Event{};

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    windowEvents.quitRequested = true;
                    appState.running = false;
                    break;

                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    windowEvents.closeRequested = true;
                    appState.running = false;
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                    windowEvents.resized = true;
                    break;

                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    windowEvents.focusGained = true;
                    break;

                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    windowEvents.focusLost = true;
                    break;

                case SDL_EVENT_KEY_DOWN:
                {
                    const auto idx = static_cast<std::size_t>(event.key.scancode);

                    if (idx < keyboard.down.size() && !event.key.repeat)
                    {
                        if (!keyboard.down[idx])
                        {
                            keyboard.pressed[idx] = true;
                        }

                        keyboard.down[idx] = true;
                    }

                    keyboard.modifiers = event.key.mod;
                    break;
                }

                case SDL_EVENT_KEY_UP:
                {
                    const auto idx = static_cast<std::size_t>(event.key.scancode);

                    if (idx < keyboard.down.size())
                    {
                        keyboard.down[idx] = false;
                        keyboard.released[idx] = true;
                    }

                    keyboard.modifiers = event.key.mod;
                    break;
                }

                case SDL_EVENT_MOUSE_MOTION:
                    mouse.position = glm::vec2{event.motion.x, event.motion.y};
                    mouse.delta += glm::vec2{event.motion.xrel, event.motion.yrel};
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    const auto bit = SDL_BUTTON_MASK(event.button.button);
                    mouse.buttonsPressed |= bit;
                    mouse.buttonsDown |= bit;
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    const auto bit = SDL_BUTTON_MASK(event.button.button);
                    mouse.buttonsReleased |= bit;
                    mouse.buttonsDown &= ~bit;
                    break;
                }

                case SDL_EVENT_MOUSE_WHEEL:
                    mouse.wheel += glm::vec2{event.wheel.x, event.wheel.y};
                    break;

                default:
                    break;
            }
        }
    }

    /// @brief Update FrameTiming from the most recent flecs delta.
    auto updateTiming(const flecs::world& world, float delta) -> void
    {
        auto& timing = world.ensure<FrameTiming>();

        timing.delta = delta;
        timing.elapsed += delta;

        const auto instantaneous = delta > 0.0F ? 1.0F / delta : 0.0F;

        if (timing.fps <= 0.0F)
        {
            timing.fps = instantaneous;
        }
        else
        {
            constexpr auto smoothing = 0.1F;
            timing.fps = (timing.fps * (1.0F - smoothing)) + (instantaneous * smoothing);
        }
    }

    /// @brief Apply size/position/title/colour changes to an existing SDL window.
    auto applyWindowProperties(SDL_Window* sdlWindow, const Window& window) -> void
    {
        SDL_SetWindowTitle(sdlWindow, window.title.c_str());
        SDL_SetWindowSize(sdlWindow, window.width, window.height);

        if (window.x != 0 || window.y != 0)
        {
            SDL_SetWindowPosition(sdlWindow, window.x, window.y);
        }
    }
}

export namespace awen::sdl
{
    /// @brief Awen ECS module. Import with `world.import<Module>()` to register
    ///        all phases, singletons, observers, and systems.
    class Module
    {
    public:
        explicit Module(flecs::world& world)
        {
            world.module<Module>();

            phases::registerAll(world);

            initializeSdl(world);
            initializeDefaults(world);

            registerWindowObservers(world);
            registerResourceObservers(world);
            registerDrawableObservers(world);

            buildQueries(world);

            registerEventSystem(world);
            registerTransformSystem(world);
            registerRenderSystem(world);
            registerPostRenderSystem(world);
        }

    private:
        flecs::query<const LocalTransform> localTransforms_{};
        flecs::query<const Drawable, const WorldTransform, const ZOrder> orderedDrawables_{};

        static auto initializeSdl(flecs::world& world) -> void
        {
            auto& flags = world.ensure<InitFlags>();

            if (!flags.sdlInitialized)
            {
                if (SDL_WasInit(SDL_INIT_VIDEO) == 0U)
                {
                    SDL_InitSubSystem(SDL_INIT_VIDEO);
                }

                flags.sdlInitialized = true;
            }

            if (!flags.ttfInitialized)
            {
                if (TTF_WasInit() == 0)
                {
                    TTF_Init();
                }

                flags.ttfInitialized = true;
            }
        }

        static auto initializeDefaults(flecs::world& world) -> void
        {
            world.set<FrameTiming>(FrameTiming{});
            world.set<FrameRate>(FrameRate{});
            world.set<AppState>(AppState{.running = true});
            world.set<KeyboardState>(KeyboardState{});
            world.set<MouseState>(MouseState{});
            world.set<WindowEvents>(WindowEvents{});
        }

        static auto registerWindowObservers(flecs::world& world) -> void
        {
            world.observer<Window>("awen::sdl::WindowOnSet")
                .event(flecs::OnSet)
                .each(
                    [](flecs::entity entity, Window& window)
                    {
                        if (const auto* handles = entity.try_get<WindowHandles>())
                        {
                            if (handles->window != nullptr)
                            {
                                detail::applyWindowProperties(handles->window, window);
                            }

                            return;
                        }

                        const auto sdlFlags = detail::toSdlFlags(window);
                        auto* sdlWindow = SDL_CreateWindow(window.title.c_str(), window.width, window.height, sdlFlags);

                        if (sdlWindow == nullptr)
                        {
                            entity.world().get_mut<AppState>().running = false;
                            return;
                        }

                        if (window.x != 0 || window.y != 0)
                        {
                            SDL_SetWindowPosition(sdlWindow, window.x, window.y);
                        }

                        auto* sdlRenderer = SDL_CreateRenderer(sdlWindow, nullptr);

                        if (sdlRenderer == nullptr)
                        {
                            SDL_DestroyWindow(sdlWindow);
                            entity.world().get_mut<AppState>().running = false;
                            return;
                        }

                        entity.set<WindowHandles>(WindowHandles{.window = sdlWindow, .renderer = sdlRenderer});
                    });

            world.observer<WindowHandles>("awen::sdl::WindowOnRemove")
                .event(flecs::OnRemove)
                .each(
                    [](flecs::entity, WindowHandles& handles)
                    {
                        if (handles.renderer != nullptr)
                        {
                            SDL_DestroyRenderer(handles.renderer);
                            handles.renderer = nullptr;
                        }

                        if (handles.window != nullptr)
                        {
                            SDL_DestroyWindow(handles.window);
                            handles.window = nullptr;
                        }
                    });
        }

        static auto registerResourceObservers(flecs::world& world) -> void
        {
            world.observer<Font>("awen::sdl::FontOnRemove")
                .event(flecs::OnRemove)
                .each(
                    [](flecs::entity, Font& font)
                    {
                        if (font.handle != nullptr)
                        {
                            TTF_CloseFont(font.handle);
                            font.handle = nullptr;
                        }
                    });

            world.observer<Texture>("awen::sdl::TextureOnRemove")
                .event(flecs::OnRemove)
                .each(
                    [](flecs::entity, Texture& texture)
                    {
                        if (texture.handle != nullptr)
                        {
                            SDL_DestroyTexture(texture.handle);
                            texture.handle = nullptr;
                        }
                    });
        }

        static auto registerDrawableObservers(flecs::world& world) -> void
        {
            // Ensure every drawable carries a ZOrder so the ordered render
            // query can sort by it (flecs requires the order_by component
            // to be a non-optional query term).
            world.observer<Drawable>("awen::sdl::DrawableOnAdd")
                .event(flecs::OnAdd)
                .each(
                    [](flecs::entity entity, Drawable&)
                    {
                        if (!entity.has<ZOrder>())
                        {
                            entity.set<ZOrder>(ZOrder{});
                        }
                    });
        }

        auto buildQueries(flecs::world& world) -> void
        {
            localTransforms_ = world.query_builder<const LocalTransform>().build();

            orderedDrawables_ = world.query_builder<const Drawable, const WorldTransform, const ZOrder>()
                                    .order_by<ZOrder>(
                                        [](flecs::entity_t, const ZOrder* a, flecs::entity_t, const ZOrder* b)
                                        {
                                            const auto av = a != nullptr ? a->value : 0;
                                            const auto bv = b != nullptr ? b->value : 0;

                                            return (av > bv) - (av < bv);
                                        })
                                    .build();
        }

        static auto registerEventSystem(flecs::world& world) -> void
        {
            world.system("awen::sdl::EventPump")
                .kind<phases::OnEvent>()
                .run(
                    [](flecs::iter& it)
                    {
                        const auto& world = it.world();
                        detail::updateTiming(world, it.delta_time());
                        detail::pumpEvents(world);
                    });
        }

        auto registerTransformSystem(flecs::world& world) -> void
        {
            world.system("awen::sdl::TransformPropagation")
                .kind<phases::OnPreRender>()
                .run([this](flecs::iter&) { detail::propagateTransforms(localTransforms_); });
        }

        auto registerRenderSystem(flecs::world& world) -> void
        {
            world.system<const Window, const WindowHandles>("awen::sdl::Render")
                .kind<phases::OnRender>()
                .each(
                    [this](flecs::entity entity, const Window& window, const WindowHandles& handles)
                    {
                        if (handles.renderer == nullptr)
                        {
                            return;
                        }

                        SDL_SetRenderDrawColor(handles.renderer, window.color.r, window.color.g, window.color.b, window.color.a);
                        SDL_RenderClear(handles.renderer);

                        const auto& world = entity.world();

                        orderedDrawables_.each([&](flecs::entity drawable, const Drawable& shape, const WorldTransform& transform, const ZOrder&)
                                               { drawEntity(world, handles.renderer, drawable, shape, transform); });

                        SDL_RenderPresent(handles.renderer);
                    });
        }

        static auto registerPostRenderSystem(flecs::world& world) -> void
        {
            world.system("awen::sdl::PostRender")
                .kind<phases::OnPostRender>()
                .run(
                    [](flecs::iter& it)
                    {
                        auto& timing = it.world().get_mut<FrameTiming>();
                        ++timing.frameCount;
                    });
        }
    };
}
