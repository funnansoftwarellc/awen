module;

#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

export module awen.widget.components;

import awen.widget.color;

export namespace awen::widget::components
{
    struct Transform
    {
        glm::vec2 position{};
        glm::vec2 scale{1.0F, 1.0F};
        float rotation{0.0F};
    };

    struct BoundingBox
    {
        glm::vec2 min{};
        glm::vec2 max{};
    };

    struct Polygon
    {
        std::vector<glm::vec2> vertices;
        Color color{};

        Polygon() = default;

        Polygon(std::vector<glm::vec2> verticesIn, Color colorIn) : vertices(std::move(verticesIn)), color(colorIn)
        {
        }
    };

    struct Rectangle
    {
        glm::vec2 size{};
        Color color{};

        Rectangle() = default;

        Rectangle(glm::vec2 sizeIn, Color colorIn) : size(sizeIn), color(colorIn)
        {
        }
    };

    struct Circle
    {
        float radius{};
        Color color{};

        Circle() = default;

        Circle(float radiusIn, Color colorIn) : radius(radiusIn), color(colorIn)
        {
        }
    };

    struct Text
    {
        std::string value;
        int fontSize{};
        Color color{};

        Text() = default;

        Text(std::string valueIn, int fontSizeIn, Color colorIn) : value(std::move(valueIn)), fontSize(fontSizeIn), color(colorIn)
        {
        }
    };

    using DrawableVariant = std::variant<Rectangle, Circle, Text, Polygon>;

    struct Drawable
    {
        std::shared_ptr<DrawableVariant> value;
    };
}