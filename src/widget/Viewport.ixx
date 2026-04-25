module;

#include <algorithm>
#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <utility>

export module awen.widget.viewport;

import awen.core.object;
import awen.widget.node;
import awen.widget.nodescissor;
import awen.widget.widget;

export namespace awen::widget
{
    /// @brief A clipping container that holds a single content widget and
    /// scrolls it vertically.  The viewport emits a scissor rectangle
    /// matching its world bounds and offsets its content by `scrollOffsetY`.
    class Viewport : public Widget
    {
    public:
        Viewport()
        {
            auto begin = std::make_unique<NodeScissorBegin>();
            scissorBegin_ = begin.get();
            this->addChild(std::move(begin));
        }

        /// @brief Replaces the current content widget.  The previous content
        /// (if any) is destroyed.
        /// @param content The new content widget to display inside the viewport.
        /// @return Pointer to the inserted content widget.
        template <typename W>
        auto setContent(std::unique_ptr<W> content) -> W*
        {
            // Remove existing content/end markers but keep scissorBegin.
            if (contentWidget_ != nullptr)
            {
                std::ignore = contentWidget_->remove();
                contentWidget_ = nullptr;
            }

            if (scissorEnd_ != nullptr)
            {
                std::ignore = scissorEnd_->remove();
                scissorEnd_ = nullptr;
            }

            auto* raw = content.get();
            contentWidget_ = raw;
            this->addChild(std::unique_ptr<awen::core::Object>{static_cast<awen::core::Object*>(content.release())});

            auto end = std::make_unique<NodeScissorEnd>();
            scissorEnd_ = end.get();
            this->addChild(std::move(end));

            return raw;
        }

        [[nodiscard]] auto getContent() const -> Widget*
        {
            return contentWidget_;
        }

        /// @brief Sets the vertical scroll offset.  Positive values scroll
        /// the content upward.
        auto setScrollOffsetY(float offset) -> void
        {
            scrollOffsetY_ = offset;
        }

        [[nodiscard]] auto getScrollOffsetY() const -> float
        {
            return scrollOffsetY_;
        }

        /// @brief Returns the measured height of the contained widget at
        /// the viewport's current width.  Used by ScrollView to drive
        /// scrollbar metrics.
        [[nodiscard]] auto contentSize() const -> Size
        {
            if (contentWidget_ == nullptr)
            {
                return Size{};
            }

            return contentWidget_->measure(Size{.width = bounds_.width, .height = 1.0e6F});
        }

        [[nodiscard]] auto measure(Size availableSize) -> Size override
        {
            return availableSize;
        }

        auto arrange(Rect bounds) -> void override
        {
            Widget::arrange(bounds);

            if (scissorBegin_ != nullptr)
            {
                scissorBegin_->setSize(glm::vec2{bounds.width, bounds.height});
            }

            if (contentWidget_ != nullptr)
            {
                const auto desired = contentWidget_->measure(Size{.width = bounds.width, .height = 1.0e6F});

                contentWidget_->arrange(Rect{
                    .x = bounds.x,
                    .y = bounds.y - scrollOffsetY_,
                    .width = bounds.width,
                    .height = std::max(bounds.height, desired.height),
                });
            }
        }

    private:
        NodeScissorBegin* scissorBegin_{nullptr};
        NodeScissorEnd* scissorEnd_{nullptr};
        Widget* contentWidget_{nullptr};
        float scrollOffsetY_{};
    };
}
