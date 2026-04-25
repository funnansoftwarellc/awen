module;

#include <algorithm>
#include <glm/vec2.hpp>
#include <memory>
#include <variant>

export module awen.widget.scrollview;

import awen.core.engine;
import awen.core.event;
import awen.core.signal;
import awen.widget.scrollbar;
import awen.widget.viewport;
import awen.widget.widget;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Composite widget pairing a `Viewport` with a vertical
    /// `Scrollbar`.  Mouse-wheel input over the viewport adjusts the
    /// shared scroll offset, which is also driven by thumb dragging.
    class ScrollView : public Widget
    {
    public:
        ScrollView()
        {
            auto viewport = std::make_unique<Viewport>();
            viewport_ = viewport.get();
            this->addChild(std::move(viewport));

            auto scrollbar = std::make_unique<Scrollbar>();
            scrollbar_ = scrollbar.get();
            scrollbar_->setScrollOffsetPointer(&scrollOffsetY_);
            this->addChild(std::move(scrollbar));

            inputConnection_ = awen::core::ScopedConnection{
                Engine::instance()->onInputEvent().connect([this](const awen::core::Event& event) { handleInput(event); })};
        }

        /// @brief Replaces the scrollable content widget.
        template <typename W>
        auto setContent(std::unique_ptr<W> content) -> W*
        {
            return viewport_->setContent(std::move(content));
        }

        [[nodiscard]] auto getViewport() const -> Viewport*
        {
            return viewport_;
        }

        [[nodiscard]] auto getScrollbar() const -> Scrollbar*
        {
            return scrollbar_;
        }

        /// @brief When enabled, the ScrollView listens for window-resize
        /// events on `Engine::onInputEvent` and re-arranges itself to fill
        /// the new window dimensions.  This is the typical mode for a
        /// top-level scroll view that occupies the entire window.
        /// @param value True to enable auto-fill; false to disable.
        auto setFillWindowOnResize(bool value) -> void
        {
            fillWindowOnResize_ = value;
        }

        [[nodiscard]] auto getFillWindowOnResize() const -> bool
        {
            return fillWindowOnResize_;
        }

        auto setScrollOffsetY(float offset) -> void
        {
            scrollOffsetY_ = offset;
        }

        [[nodiscard]] auto getScrollOffsetY() const -> float
        {
            return scrollOffsetY_;
        }

        [[nodiscard]] auto measure(Size availableSize) -> Size override
        {
            return availableSize;
        }

        auto arrange(Rect bounds) -> void override
        {
            Widget::arrange(bounds);

            const auto contentHeight = (viewport_ != nullptr && viewport_->getContent() != nullptr)
                                           ? viewport_->getContent()->measure(Size{.width = bounds.width - scrollbarWidth_, .height = 1.0e6F}).height
                                           : 0.0F;

            const auto viewportHeight = bounds.height;
            const auto maxOffset = std::max(0.0F, contentHeight - viewportHeight);
            scrollOffsetY_ = std::clamp(scrollOffsetY_, 0.0F, maxOffset);

            viewport_->setScrollOffsetY(scrollOffsetY_);
            viewport_->arrange(Rect{
                .x = bounds.x,
                .y = bounds.y,
                .width = bounds.width - scrollbarWidth_,
                .height = bounds.height,
            });

            scrollbar_->setMetrics(contentHeight, viewportHeight);
            scrollbar_->arrange(Rect{
                .x = bounds.x + bounds.width - scrollbarWidth_,
                .y = bounds.y,
                .width = scrollbarWidth_,
                .height = bounds.height,
            });
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            // Keep viewport offset in sync each frame in case the scrollbar
            // mutated `scrollOffsetY_` from a drag.
            viewport_->setScrollOffsetY(scrollOffsetY_);
            return Widget::synchronize(entity);
        }

    private:
        auto handleInput(const awen::core::Event& event) -> void
        {
            if (const auto* resize = std::get_if<awen::core::EventWindowResize>(&event); resize != nullptr)
            {
                if (fillWindowOnResize_)
                {
                    arrange(Rect{
                        .x = 0.0F,
                        .y = 0.0F,
                        .width = static_cast<float>(resize->width),
                        .height = static_cast<float>(resize->height),
                    });
                }

                return;
            }

            const auto* wheel = std::get_if<awen::core::EventMouseWheel>(&event);

            if (wheel == nullptr || viewport_ == nullptr)
            {
                return;
            }

            const auto cursor = glm::vec2{static_cast<float>(wheel->x), static_cast<float>(wheel->y)};

            if (!viewport_->getBounds().contains(cursor))
            {
                return;
            }

            const auto contentHeight = (viewport_->getContent() != nullptr)
                                           ? viewport_->getContent()->measure(Size{.width = viewport_->getBounds().width, .height = 1.0e6F}).height
                                           : 0.0F;
            const auto maxOffset = std::max(0.0F, contentHeight - viewport_->getBounds().height);
            scrollOffsetY_ = std::clamp(scrollOffsetY_ - wheel->dy * wheelStep_, 0.0F, maxOffset);
            viewport_->setScrollOffsetY(scrollOffsetY_);

            // Re-arrange viewport content so its world position updates immediately.
            if (auto* content = viewport_->getContent(); content != nullptr)
            {
                content->arrange(Rect{
                    .x = viewport_->getBounds().x,
                    .y = viewport_->getBounds().y - scrollOffsetY_,
                    .width = viewport_->getBounds().width,
                    .height = std::max(viewport_->getBounds().height, contentHeight),
                });
            }
        }

        Viewport* viewport_{nullptr};
        Scrollbar* scrollbar_{nullptr};
        awen::core::ScopedConnection inputConnection_;
        float scrollOffsetY_{};
        float scrollbarWidth_{12.0F};
        float wheelStep_{40.0F};
        bool fillWindowOnResize_{false};
    };
}
