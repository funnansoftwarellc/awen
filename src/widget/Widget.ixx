module;

#include <algorithm>
#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <utility>

export module awen.widget.widget;

import awen.core.engine;
import awen.core.object;
import awen.widget.components;
import awen.widget.node;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Two-dimensional size in pixels.
    struct Size
    {
        float width{};
        float height{};

        auto operator<=>(const Size&) const noexcept = default;
    };

    /// @brief Axis-aligned rectangle in pixels.  Widget bounds are stored in
    /// world (top-level window) coordinates.
    struct Rect
    {
        float x{};
        float y{};
        float width{};
        float height{};

        [[nodiscard]] auto right() const noexcept -> float
        {
            return x + width;
        }

        [[nodiscard]] auto bottom() const noexcept -> float
        {
            return y + height;
        }

        [[nodiscard]] auto contains(glm::vec2 point) const noexcept -> bool
        {
            return point.x >= x && point.x < right() && point.y >= y && point.y < bottom();
        }

        auto operator<=>(const Rect&) const noexcept = default;
    };

    /// @brief Padding around a rectangle expressed as four edges.
    struct Insets
    {
        float left{};
        float top{};
        float right{};
        float bottom{};

        [[nodiscard]] auto horizontal() const noexcept -> float
        {
            return left + right;
        }

        [[nodiscard]] auto vertical() const noexcept -> float
        {
            return top + bottom;
        }

        [[nodiscard]] static auto all(float value) noexcept -> Insets
        {
            return Insets{.left = value, .top = value, .right = value, .bottom = value};
        }

        auto operator<=>(const Insets&) const noexcept = default;
    };

    /// @brief Base class for widgets that participate in measure/arrange layout
    /// and emit transform components into the flecs draw tree.
    ///
    /// Widget extends Node so it can act as both a layout-aware container and
    /// an entry in the synchronized scene graph.  Each widget stores its
    /// final world-space rectangle (assigned by `arrange`) and emits a
    /// `Transform` component whose position is offset relative to the nearest
    /// widget ancestor.  Sub-classes override `measure` and `arrange` to
    /// implement specific layouts.
    class Widget : public Node
    {
    public:
        auto setPadding(Insets padding) -> void
        {
            padding_ = padding;
        }

        [[nodiscard]] auto getPadding() const -> Insets
        {
            return padding_;
        }

        [[nodiscard]] auto getBounds() const -> Rect
        {
            return bounds_;
        }

        /// @brief Adds a child widget and returns a non-owning pointer.
        template <typename W>
        auto addWidget(std::unique_ptr<W> widget) -> W*
        {
            auto* raw = widget.get();
            this->addChild(std::unique_ptr<awen::core::Object>{static_cast<awen::core::Object*>(widget.release())});
            return raw;
        }

        /// @brief Computes the desired size of the widget given an available
        /// budget.  Default returns zero.
        [[nodiscard]] virtual auto measure(Size availableSize) -> Size
        {
            (void)availableSize;
            return Size{};
        }

        /// @brief Assigns the final world rectangle.  Sub-classes override
        /// this to lay out children inside `contentRect()`.
        virtual auto arrange(Rect bounds) -> void
        {
            bounds_ = bounds;
        }

        /// @brief Returns the content rectangle (bounds shrunk by padding).
        [[nodiscard]] auto contentRect() const -> Rect
        {
            return Rect{
                .x = bounds_.x + padding_.left,
                .y = bounds_.y + padding_.top,
                .width = std::max(0.0F, bounds_.width - padding_.horizontal()),
                .height = std::max(0.0F, bounds_.height - padding_.vertical()),
            };
        }

        /// @brief Returns the world position of the nearest widget ancestor,
        /// or `{0,0}` if there is none.
        [[nodiscard]] auto parentWorldPosition() const -> glm::vec2
        {
            for (const auto* parent = this->getParent(); parent != nullptr; parent = parent->getParent())
            {
                if (const auto* widget = dynamic_cast<const Widget*>(parent); widget != nullptr)
                {
                    return glm::vec2{widget->bounds_.x, widget->bounds_.y};
                }
            }

            return glm::vec2{0.0F, 0.0F};
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            const auto parentWorld = parentWorldPosition();
            entity.set<components::Transform>({
                .position = glm::vec2{bounds_.x - parentWorld.x, bounds_.y - parentWorld.y},
                .scale = glm::vec2{1.0F, 1.0F},
                .rotation = 0.0F,
            });

            return entity;
        }

    private:
        Rect bounds_{};
        Insets padding_{};
    };
}
