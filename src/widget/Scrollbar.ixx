module;

#include <algorithm>
#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <variant>

export module awen.widget.scrollbar;

import awen.core.engine;
import awen.core.event;
import awen.core.signal;
import awen.widget.color;
import awen.widget.noderectangle;
import awen.widget.nodetransform;
import awen.widget.widget;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Vertical scrollbar that drives an external scroll-offset value.
    ///
    /// The scrollbar owns a track rectangle and a thumb rectangle (positioned
    /// via a child transform).  It listens to mouse events through
    /// `Engine::onInputEvent` to support thumb dragging.
    class Scrollbar : public Widget
    {
    public:
        static constexpr auto DefaultBarWidth = 12.0F;
        static constexpr auto MinThumbHeight = 20.0F;

        Scrollbar()
        {
            auto track = std::make_unique<NodeRectangle>();
            track->setColor(Color{.r = 40, .g = 40, .b = 40, .a = 255});
            track_ = track.get();
            this->addChild(std::move(track));

            auto thumbTransform = std::make_unique<NodeTransform>();
            thumbTransform_ = thumbTransform.get();

            auto thumb = std::make_unique<NodeRectangle>();
            thumb->setColor(Color{.r = 120, .g = 120, .b = 120, .a = 255});
            thumb_ = thumb.get();
            thumbTransform_->addChild(std::move(thumb));

            this->addChild(std::move(thumbTransform));

            inputConnection_ = awen::core::ScopedConnection{
                Engine::instance()->onInputEvent().connect([this](const awen::core::Event& event) { handleInput(event); })};
        }

        /// @brief Sets a pointer to the scroll-offset value updated by drags.
        /// The pointer is borrowed and must outlive the scrollbar.
        auto setScrollOffsetPointer(float* offset) -> void
        {
            scrollOffsetPtr_ = offset;
        }

        /// @brief Updates the metrics that determine thumb size and position.
        /// @param contentHeight Total height of scrollable content.
        /// @param viewportHeight Visible viewport height.
        auto setMetrics(float contentHeight, float viewportHeight) -> void
        {
            contentHeight_ = std::max(0.0F, contentHeight);
            viewportHeight_ = std::max(0.0F, viewportHeight);
        }

        [[nodiscard]] auto measure(Size availableSize) -> Size override
        {
            return Size{.width = barWidth_, .height = availableSize.height};
        }

        auto arrange(Rect bounds) -> void override
        {
            Widget::arrange(bounds);
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            const auto bounds = getBounds();
            track_->setSize(glm::vec2{bounds.width, bounds.height});

            const auto thumbHeightPx = computeThumbHeight();
            const auto thumbYPx = computeThumbY(thumbHeightPx);

            thumb_->setSize(glm::vec2{bounds.width, thumbHeightPx});
            thumbTransform_->setPosition(glm::vec2{0.0F, thumbYPx});

            return Widget::synchronize(entity);
        }

    private:
        [[nodiscard]] auto computeThumbHeight() const -> float
        {
            if (contentHeight_ <= 0.0F || viewportHeight_ >= contentHeight_)
            {
                return getBounds().height;
            }

            const auto ratio = viewportHeight_ / contentHeight_;
            return std::max(MinThumbHeight, getBounds().height * ratio);
        }

        [[nodiscard]] auto computeThumbY(float thumbHeightPx) const -> float
        {
            if (contentHeight_ <= viewportHeight_)
            {
                return 0.0F;
            }

            const auto maxOffset = contentHeight_ - viewportHeight_;
            const auto offset = scrollOffsetPtr_ != nullptr ? std::clamp(*scrollOffsetPtr_, 0.0F, maxOffset) : 0.0F;
            const auto travel = getBounds().height - thumbHeightPx;
            return (offset / maxOffset) * travel;
        }

        auto handleInput(const awen::core::Event& event) -> void
        {
            if (const auto* button = std::get_if<awen::core::EventMouseButton>(&event); button != nullptr)
            {
                if (button->button != awen::core::MouseButton::left)
                {
                    return;
                }

                if (button->type == awen::core::EventMouseButton::Type::pressed)
                {
                    const auto thumbHeightPx = computeThumbHeight();
                    const auto thumbYPx = computeThumbY(thumbHeightPx);
                    const auto bounds = getBounds();
                    const auto thumbRect = Rect{
                        .x = bounds.x,
                        .y = bounds.y + thumbYPx,
                        .width = bounds.width,
                        .height = thumbHeightPx,
                    };

                    if (thumbRect.contains(glm::vec2{static_cast<float>(button->x), static_cast<float>(button->y)}))
                    {
                        dragging_ = true;
                        dragStartMouseY_ = static_cast<float>(button->y);
                        dragStartOffset_ = scrollOffsetPtr_ != nullptr ? *scrollOffsetPtr_ : 0.0F;
                    }
                }
                else
                {
                    dragging_ = false;
                }
            }
            else if (const auto* motion = std::get_if<awen::core::EventMouseMotion>(&event); motion != nullptr)
            {
                if (!dragging_ || scrollOffsetPtr_ == nullptr || contentHeight_ <= viewportHeight_)
                {
                    return;
                }

                const auto thumbHeightPx = computeThumbHeight();
                const auto travel = std::max(1.0F, getBounds().height - thumbHeightPx);
                const auto maxOffset = contentHeight_ - viewportHeight_;
                const auto deltaPx = static_cast<float>(motion->y) - dragStartMouseY_;
                const auto deltaOffset = (deltaPx / travel) * maxOffset;
                *scrollOffsetPtr_ = std::clamp(dragStartOffset_ + deltaOffset, 0.0F, maxOffset);
            }
        }

        NodeRectangle* track_{nullptr};
        NodeTransform* thumbTransform_{nullptr};
        NodeRectangle* thumb_{nullptr};
        awen::core::ScopedConnection inputConnection_;
        float* scrollOffsetPtr_{nullptr};
        float contentHeight_{};
        float viewportHeight_{};
        float barWidth_{DefaultBarWidth};
        float dragStartMouseY_{};
        float dragStartOffset_{};
        bool dragging_{false};
    };
}
