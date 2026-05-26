module;

#include <raylib.h>
#include <compare>
#include <sigslot/signal.hpp>
#include <typeinfo>

export module awen.raylib.ring;
import awen.raylib.color;
import awen.raylib.node;

export namespace awen::raylib
{
    class Ring : public Node
    {
    public:
        Ring()
        {
            onRender([this] { DrawRing(Vector2{}, innerRadius_, outerRadius_, startAngle_, endAngle_, segments_, ToRaylibColor(color_)); });
        }

        auto setInnerRadius(float x) -> void
        {
            innerRadius_ = x;
        }

        [[nodiscard]] auto getInnerRadius() const -> float
        {
            return innerRadius_;
        }

        auto setOuterRadius(float x) -> void
        {
            outerRadius_ = x;
        }

        [[nodiscard]] auto getOuterRadius() const -> float
        {
            return outerRadius_;
        }

        auto setStartAngle(float x) -> void
        {
            startAngle_ = x;
        }

        [[nodiscard]] auto getStartAngle() const -> float
        {
            return startAngle_;
        }

        auto setEndAngle(float x) -> void
        {
            endAngle_ = x;
        }

        [[nodiscard]] auto getEndAngle() const -> float
        {
            return endAngle_;
        }

        auto setSegments(int x) -> void
        {
            segments_ = x;
        }

        [[nodiscard]] auto getSegments() const -> int
        {
            return segments_;
        }

        auto setColor(const Color& color) -> void
        {
            color_ = color;
        }

        [[nodiscard]] auto getColor() const noexcept -> Color
        {
            return color_;
        }

    private:
        Color color_{};
        float innerRadius_{};
        float outerRadius_{};
        float startAngle_{};
        float endAngle_{};
        int segments_{};
    };
}
