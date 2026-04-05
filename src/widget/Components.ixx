module;

#include <glm/vec2.hpp>
#include <vector>

export module awen.widget.components;

export namespace awen::widget::components
{
    struct Transform
    {
        glm::vec2 position{};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation{0.0f};
    };

    struct BoundingBox
    {
        glm::vec2 min{};
        glm::vec2 max{};
    };

    struct Polygon
    {
        std::vector<glm::vec2> vertices;
    };
}