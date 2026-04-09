module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>

#include <memory>
#include <variant>
#include <vector>

export module awen.widget.nodepolygon;
import awen.core.engine;
import awen.widget.color;
import awen.widget.node;
import awen.widget.components;

using awen::core::Engine;

export namespace awen::widget
{
    class NodePolygon : public awen::widget::Node
    {
    public:
        auto setVertices(const std::vector<glm::vec2>& vertices) -> void
        {
            vertices_ = vertices;
        }

        auto setColor(Color color) -> void
        {
            color_ = color;
        }

        [[nodiscard]] auto getColor() const -> Color
        {
            return color_;
        }

        [[nodiscard]] auto getVertices() const -> const std::vector<glm::vec2>&
        {
            return vertices_;
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Drawable>(components::Drawable{
                .value = std::make_shared<components::DrawableVariant>(std::in_place_type<components::Polygon>, vertices_, color_),
            });

            return entity;
        }

    private:
        std::vector<glm::vec2> vertices_;
        Color color_{colors::White};
    };
}
