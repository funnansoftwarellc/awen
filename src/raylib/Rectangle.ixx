module;

#include <raylib.h>
#include <variant>

export module awen.raylib.rectangle;
import awen.raylib.color;
import awen.raylib.node;
import awen.raylib.events;

export namespace awen::raylib
{
    class Rectangle : public Node
    {
    public:
        Rectangle()
        {
            onRender(
                [this]
                {
                    DrawRectangle(static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(width_), static_cast<int>(height_),
                                  ToRaylibColor(color_));
                });

            onEvents(
                [this](Event& event)
                {
                    auto* e = std::get_if<EventMouse>(&event);

                    if (e == nullptr)
                    {
                        return;
                    }

                    if (e->type == EventMouse::Type::ButtonPressed)
                    {
                        const auto mouseX = e->x;
                        const auto mouseY = e->y;

                        const auto local = mapToNode({.x = mouseX, .y = mouseY});

                        if (local.x >= x_ && local.x <= x_ + width_ && local.y >= y_ && local.y <= y_ + height_)
                        {
                            color_ = colors::Green;
                            e->handled = true;
                        }
                    }
                });
        }

        Rectangle(const Rectangle&) = delete;
        auto operator=(const Rectangle&) -> Rectangle& = delete;

        Rectangle(Rectangle&&) noexcept = delete;
        auto operator=(Rectangle&&) noexcept -> Rectangle& = delete;

        ~Rectangle() override = default;

        auto setX(float x) noexcept -> void
        {
            x_ = x;
        }

        [[nodiscard]] auto getX() const noexcept -> float
        {
            return x_;
        }

        auto setY(float y) noexcept -> void
        {
            y_ = y;
        }

        [[nodiscard]] auto getY() const noexcept -> float
        {
            return y_;
        }

        auto setWidth(float width) noexcept -> void
        {
            width_ = width;
        }

        [[nodiscard]] auto getWidth() const noexcept -> float
        {
            return width_;
        }

        auto setHeight(float height) noexcept -> void
        {
            height_ = height;
        }

        [[nodiscard]] auto getHeight() const noexcept -> float
        {
            return height_;
        }

        auto setColor(const Color& color) noexcept -> void
        {
            color_ = color;
        }

        [[nodiscard]] auto getColor() const noexcept -> Color
        {
            return color_;
        }

    private:
        Color color_{colors::Magenta};
        float x_{};
        float y_{};
        float width_{};
        float height_{};
    };
}