module;

#include <raylib.h>
#include <compare>
#include <vector>

export module awen.raylib.trianglestrip;
import awen.raylib.color;
import awen.raylib.node;

export namespace awen::raylib
{
    class TriangleStrip : public Node
    {
    public:
        TriangleStrip()
        {
            onRender(
                [this]()
                {
                    if (std::size(vertices_) < 3)
                    {
                        return;
                    }
                    DrawTriangleStrip(vertices_.data(), static_cast<int>(std::size(vertices_)), ToRaylibColor(color_));
                });
        }

        auto setColor(const Color& color) -> void
        {
            color_ = color;
        }

        auto getColor() const -> Color
        {
            return color_;
        }

        auto setVertices(std::vector<Vector2> vertices) -> void
        {
            vertices_ = std::move(vertices);
        }

        [[nodiscard]] auto getVertices() const -> const std::vector<Vector2>&
        {
            return vertices_;
        }

    private:
        Color color_{colors::White};
        std::vector<Vector2> vertices_;
    };
}