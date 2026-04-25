module;

#include <SDL3/SDL_rect.h>
#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

export module awen.widget.window;

import awen.core.overloaded;
import awen.core.engine;
import awen.core.object;
import awen.core.signal;
import awen.sdl.drawlist;
import awen.sdl.renderer;
import awen.sdl.window;
import awen.widget.components;
import awen.widget.color;
import awen.widget.event;
import awen.widget.node;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Root widget node that owns the SDL window, renderer, and synchronized flecs tree.
    class Window : public Node
    {
    public:
        /// @brief Creates a widget window that initializes SDL during startup.
        Window()
        {
            startupConnection_ = awen::core::ScopedConnection{this->onStartup().connect(
                [this]
                {
                    const auto width = static_cast<int>(this->getSize()[0]);
                    const auto height = static_cast<int>(this->getSize()[1]);
                    const auto x = static_cast<int>(this->getPosition()[0]);
                    const auto y = static_cast<int>(this->getPosition()[1]);

                    auto windowResult = awen::sdl::Window::create({
                        .title = title_.c_str(),
                        .width = width,
                        .height = height,
                        .positionX = x,
                        .positionY = y,
                        .resizable = true,
                        .highDpi = true,
                        .useLogicalPresentation = useLogicalPresentation_,
                    });

                    if (!windowResult)
                    {
                        handleError(windowResult.error());
                        return;
                    }

                    window_.emplace(std::move(windowResult.value()));

                    if (auto initializeResult = awen::sdl::Renderer::initialize(window_->renderer()); !initializeResult)
                    {
                        handleError(initializeResult.error());
                        window_.reset();
                        return;
                    }

                    // Dispatch an initial resize event so listeners (e.g.
                    // ScrollView) can perform their first layout pass against
                    // the actual window dimensions, which may differ from the
                    // requested size on high-DPI displays.
                    Engine::instance()->dispatchEvent(awen::core::EventWindowResize{
                        .width = awen::sdl::Window::getScreenWidth(),
                        .height = awen::sdl::Window::getScreenHeight(),
                    });
                })};

            eventConnection_ = awen::core::ScopedConnection{Engine::instance()->onEvent().connect(
                [this]
                {
                    if (!window_.has_value())
                    {
                        return;
                    }

                    auto pollResult = window_->pollEvents();

                    if (!pollResult)
                    {
                        handleError(pollResult.error());
                        return;
                    }

                    if (!pollResult->open)
                    {
                        Engine::instance()->stop();
                    }
                })};

            renderConnection_ = awen::core::ScopedConnection{Engine::instance()->onRender().connect(
                [this]
                {
                    if (!window_.has_value())
                    {
                        return;
                    }

                    renderOnce();
                })};

            // Re-render synchronously on resize.  This works during the
            // Win32 modal resize loop because SDL fires its event watch
            // synchronously from inside the message pump.
            inputConnection_ = awen::core::ScopedConnection{Engine::instance()->onInputEvent().connect(
                [this](const awen::core::Event& event)
                {
                    if (!window_.has_value())
                    {
                        return;
                    }

                    if (std::holds_alternative<awen::core::EventWindowResize>(event))
                    {
                        renderOnce();
                    }
                })};
        }

        ~Window() override = default;

        Window(const Window&) = delete;
        auto operator=(const Window&) -> Window& = delete;
        Window(Window&&) = delete;
        auto operator=(Window&&) -> Window& = delete;

        auto setPosition(glm::vec2 x) -> void
        {
            position_ = x;
        }

        [[nodiscard]] auto getPosition() const -> glm::vec2
        {
            return position_;
        }

        auto setSize(glm::vec2 x) -> void
        {
            size_ = x;
        }

        [[nodiscard]] auto getSize() const -> glm::vec2
        {
            return size_;
        }

        /// @brief Sets the title shown in the native window chrome.
        /// @param title The title text.
        auto setTitle(std::string_view title) -> void
        {
            title_ = title;
        }

        /// @brief Gets the current title string.
        /// @return The title text.
        [[nodiscard]] auto getTitle() const -> std::string_view
        {
            return title_;
        }

        /// @brief Sets the clear color used before drawing child widgets.
        /// @param color The color written to the SDL renderer each frame.
        auto setClearColor(Color color) -> void
        {
            clearColor_ = color;
        }

        /// @brief Gets the current clear color.
        /// @return The current clear color.
        [[nodiscard]] auto getClearColor() const -> Color
        {
            return clearColor_;
        }

        /// @brief Selects whether SDL's logical presentation (letterbox)
        ///        is enabled.  When disabled, the renderer reports actual
        ///        pixel dimensions and content is rendered at the window's
        ///        current size without aspect-ratio preservation.  Must be
        ///        called before the window starts up.
        /// @param value True to enable logical presentation; false to disable.
        auto setUseLogicalPresentation(bool value) -> void
        {
            useLogicalPresentation_ = value;
        }

        /// @brief Returns whether logical presentation is currently enabled.
        [[nodiscard]] auto getUseLogicalPresentation() const -> bool
        {
            return useLogicalPresentation_;
        }

        /// @brief Returns whether a key is currently held down.
        /// @param key The key to test.
        /// @return True if the key is down.
        [[nodiscard]] static auto isKeyDown(EventKeyboard::Key key) -> bool
        {
            return awen::sdl::Window::isKeyDown(key);
        }

        /// @brief Returns whether a key was pressed during the current event pump.
        /// @param key The key to test.
        /// @return True if the key was pressed this frame.
        [[nodiscard]] static auto isKeyPressed(EventKeyboard::Key key) -> bool
        {
            return awen::sdl::Window::isKeyPressed(key);
        }

        /// @brief Returns whether a key was released during the current event pump.
        /// @param key The key to test.
        /// @return True if the key was released this frame.
        [[nodiscard]] static auto isKeyReleased(EventKeyboard::Key key) -> bool
        {
            return awen::sdl::Window::isKeyReleased(key);
        }

        /// @brief Gets the current drawable width in pixels.
        /// @return The current renderer width.
        [[nodiscard]] static auto getScreenWidth() -> int
        {
            return awen::sdl::Window::getScreenWidth();
        }

        /// @brief Gets the current drawable height in pixels.
        /// @return The current renderer height.
        [[nodiscard]] static auto getScreenHeight() -> int
        {
            return awen::sdl::Window::getScreenHeight();
        }

        /// @brief Measures the width of text using the default SDL_ttf font.
        /// @param text The text to measure.
        /// @param fontSize The font size in points.
        /// @return The measured width in pixels.
        [[nodiscard]] static auto measureText(const char* text, int fontSize) -> int
        {
            auto width = awen::sdl::Renderer::measureText(text, fontSize);

            if (!width)
            {
                return 0;
            }

            return *width;
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Transform>({
                .position = glm::vec2{0.0F, 0.0F},
                .scale = glm::vec2{1.0F, 1.0F},
                .rotation = 0.0F,
            });

            return entity;
        }

        [[nodiscard]] auto getLastError() const -> const std::optional<std::string>&
        {
            return lastError_;
        }

    private:
        struct WorldTransform
        {
            glm::vec2 position{};
            glm::vec2 scale{1.0F, 1.0F};
            float rotation{};
        };

        static auto combineTransform(const WorldTransform& parent, const components::Transform& local) -> WorldTransform
        {
            return WorldTransform{
                .position = parent.position + (local.position * parent.scale),
                .scale = parent.scale * local.scale,
                .rotation = parent.rotation + local.rotation,
            };
        }

        auto handleError(std::string error) -> void
        {
            lastError_ = std::move(error);
            Engine::instance()->stop();
        }

        auto renderOnce() -> void
        {
            if (!window_.has_value())
            {
                return;
            }

            synchronizeTree();
            buildDrawList();

            if (auto beginResult = awen::sdl::Renderer::begin(); !beginResult)
            {
                handleError(beginResult.error());
                return;
            }

            if (auto clearResult = awen::sdl::Renderer::clear(clearColor_); !clearResult)
            {
                handleError(clearResult.error());
                return;
            }

            if (auto submitResult = awen::sdl::Renderer::submit(drawList_); !submitResult)
            {
                handleError(submitResult.error());
                return;
            }

            if (auto endResult = awen::sdl::Renderer::end(); !endResult)
            {
                handleError(endResult.error());
            }
        }

        auto ensureEntity(const Node* node) -> flecs::entity
        {
            if (const auto it = nodeEntities_.find(node); it != std::end(nodeEntities_))
            {
                return Engine::instance()->world().entity(it->second);
            }

            const auto entity = Engine::instance()->world().entity();
            nodeEntities_.emplace(node, entity.id());
            return entity;
        }

        auto synchronizeTree() -> void
        {
            auto visited = std::unordered_set<const Node*>{};
            auto stack = std::vector<std::pair<const awen::core::Object*, flecs::entity>>{};
            stack.emplace_back(this, flecs::entity{});

            while (!std::empty(stack))
            {
                const auto [object, parentEntity] = stack.back();
                stack.pop_back();

                auto nextParent = parentEntity;

                if (const auto* node = dynamic_cast<const Node*>(object); node != nullptr)
                {
                    auto entity = ensureEntity(node);

                    if (parentEntity.is_valid())
                    {
                        const auto currentParent = entity.parent();

                        if (!currentParent.is_valid() || currentParent != parentEntity)
                        {
                            if (currentParent.is_valid())
                            {
                                entity.remove(flecs::ChildOf, currentParent);
                            }

                            entity.child_of(parentEntity);
                        }
                    }
                    else
                    {
                        const auto currentParent = entity.parent();

                        if (currentParent.is_valid())
                        {
                            entity.remove(flecs::ChildOf, currentParent);
                        }
                    }

                    nextParent = node->synchronize(entity);
                    visited.insert(node);

                    if (node == this)
                    {
                        rootEntity_ = nextParent;
                    }
                }

                for (const auto& child : object->getChildren() | std::views::reverse)
                {
                    stack.emplace_back(child.get(), nextParent);
                }
            }

            for (auto it = std::begin(nodeEntities_); it != std::end(nodeEntities_);)
            {
                if (visited.contains(it->first))
                {
                    ++it;
                    continue;
                }

                Engine::instance()->world().entity(it->second).destruct();
                it = nodeEntities_.erase(it);
            }
        }

        auto buildDrawList() -> void
        {
            drawList_.clear();
            buildDrawListEntity(rootEntity_, WorldTransform{});
        }

        auto buildDrawListEntity(flecs::entity entity, const WorldTransform& parentTransform) -> void
        {
            if (!entity.is_valid())
            {
                return;
            }

            auto worldTransform = parentTransform;

            if (const auto* local = entity.try_get<components::Transform>(); local != nullptr)
            {
                worldTransform = combineTransform(parentTransform, *local);
            }

            if (const auto* drawable = entity.try_get<components::Drawable>(); drawable != nullptr)
            {
                if (drawable->value == nullptr)
                {
                    return;
                }

                std::visit(
                    awen::core::Overloaded{
                        [this, &worldTransform](const components::Rectangle& rectangle)
                        {
                            drawList_.emplace_back(awen::sdl::DrawRectangle{
                                .x = worldTransform.position.x,
                                .y = worldTransform.position.y,
                                .width = rectangle.size.x * worldTransform.scale.x,
                                .height = rectangle.size.y * worldTransform.scale.y,
                                .color = rectangle.color,
                            });
                        },
                        [this, &worldTransform](const components::Circle& circle)
                        {
                            drawList_.emplace_back(awen::sdl::DrawCircle{
                                .centerX = worldTransform.position.x,
                                .centerY = worldTransform.position.y,
                                .radius = circle.radius * ((worldTransform.scale.x + worldTransform.scale.y) * 0.5F),
                                .color = circle.color,
                            });
                        },
                        [this, &worldTransform](const components::Text& text)
                        {
                            drawList_.emplace_back(awen::sdl::DrawText{
                                .text = text.value,
                                .x = static_cast<int>(worldTransform.position.x),
                                .y = static_cast<int>(worldTransform.position.y),
                                .fontSize = text.fontSize,
                                .color = text.color,
                                .font = text.font,
                            });
                        },
                        [this, &worldTransform](const components::Polygon& polygon)
                        {
                            auto transformedVertices = std::vector<SDL_FPoint>{};
                            transformedVertices.reserve(polygon.vertices.size());

                            for (const auto& vertex : polygon.vertices)
                            {
                                transformedVertices.emplace_back(SDL_FPoint{
                                    .x = worldTransform.position.x + (vertex.x * worldTransform.scale.x),
                                    .y = worldTransform.position.y + (vertex.y * worldTransform.scale.y),
                                });
                            }

                            drawList_.emplace_back(awen::sdl::DrawPolygon{
                                .vertices = std::move(transformedVertices),
                                .color = polygon.color,
                            });
                        },
                        [this, &worldTransform](const components::ScissorBegin& scissor)
                        {
                            drawList_.emplace_back(awen::sdl::DrawScissorPush{
                                .x = static_cast<int>(worldTransform.position.x),
                                .y = static_cast<int>(worldTransform.position.y),
                                .width = static_cast<int>(scissor.size.x * worldTransform.scale.x),
                                .height = static_cast<int>(scissor.size.y * worldTransform.scale.y),
                            });
                        },
                        [this](const components::ScissorEnd&) { drawList_.emplace_back(awen::sdl::DrawScissorPop{}); },
                    },
                    *drawable->value);
            }

            entity.children([this, &worldTransform](flecs::entity child) { buildDrawListEntity(child, worldTransform); });
        }

        std::optional<awen::sdl::Window> window_;
        flecs::entity rootEntity_;
        awen::sdl::DrawList drawList_;
        std::unordered_map<const Node*, flecs::entity_t> nodeEntities_;
        awen::core::ScopedConnection startupConnection_;
        awen::core::ScopedConnection eventConnection_;
        awen::core::ScopedConnection renderConnection_;
        awen::core::ScopedConnection inputConnection_;
        std::string title_;
        glm::vec2 position_{0.0F, 0.0F};
        glm::vec2 size_{};
        Color clearColor_{colors::Orange};
        bool useLogicalPresentation_{true};
        std::optional<std::string> lastError_;
    };
}