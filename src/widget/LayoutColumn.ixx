module;

#include <algorithm>
#include <memory>

export module awen.widget.layoutcolumn;

import awen.core.object;
import awen.widget.widget;

export namespace awen::widget
{
    /// @brief Vertical layout that stacks children top-to-bottom with a
    /// fixed inter-child spacing.  Each child is given the full content
    /// width and its measured height.  No flex / proportional sizing.
    class ColumnLayout : public Widget
    {
    public:
        auto setSpacing(float spacing) -> void
        {
            spacing_ = spacing;
        }

        [[nodiscard]] auto getSpacing() const -> float
        {
            return spacing_;
        }

        [[nodiscard]] auto measure(Size availableSize) -> Size override
        {
            const auto contentWidth = std::max(0.0F, availableSize.width - getPadding().horizontal());
            auto totalHeight = 0.0F;
            auto count = 0;

            for (const auto& child : this->getChildren())
            {
                auto* widget = dynamic_cast<Widget*>(child.get());

                if (widget == nullptr)
                {
                    continue;
                }

                const auto childSize = widget->measure(Size{.width = contentWidth, .height = availableSize.height});
                totalHeight += childSize.height;
                ++count;
            }

            if (count > 1)
            {
                totalHeight += spacing_ * static_cast<float>(count - 1);
            }

            return Size{
                .width = availableSize.width,
                .height = totalHeight + getPadding().vertical(),
            };
        }

        auto arrange(Rect bounds) -> void override
        {
            Widget::arrange(bounds);

            const auto content = contentRect();
            auto y = content.y;
            auto first = true;

            for (const auto& child : this->getChildren())
            {
                auto* widget = dynamic_cast<Widget*>(child.get());

                if (widget == nullptr)
                {
                    continue;
                }

                if (!first)
                {
                    y += spacing_;
                }

                first = false;

                const auto childSize = widget->measure(Size{.width = content.width, .height = content.height});
                widget->arrange(Rect{
                    .x = content.x,
                    .y = y,
                    .width = content.width,
                    .height = childSize.height,
                });
                y += childSize.height;
            }
        }

    private:
        float spacing_{};
    };
}
