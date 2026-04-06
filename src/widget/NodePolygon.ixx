module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <vector>

export module awen.widget.nodepolygon;
import awen.core.engine;
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

            entity.set<components::Polygon>({
                .vertices = vertices_,
            });

            return entity;
        }

    private:
        std::vector<glm::vec2> vertices_;
    };
}
