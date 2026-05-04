module;

#include <awen/flecs/Flecs.hpp>
#include <cmath>
#include <cstdint>
#include <glm/vec2.hpp>

export module awen.sdl.transform;

export namespace awen::sdl
{
    /// @brief Local-space 2D transform of an entity, relative to its parent.
    struct LocalTransform
    {
        glm::vec2 position{};
        glm::vec2 scale{1.0F, 1.0F};
        float rotation{};
    };

    /// @brief World-space 2D transform produced by the hierarchy propagation system.
    /// @note Decomposed (not a matrix) so SDL_Render* calls can consume it directly.
    struct WorldTransform
    {
        glm::vec2 position{};
        glm::vec2 scale{1.0F, 1.0F};
        float rotation{};
    };

    /// @brief Render layer; lower values are drawn first.
    struct ZOrder
    {
        std::int32_t value{};
    };

    /// @brief Compose a parent world transform with a child local transform.
    /// @param parent Parent's WorldTransform.
    /// @param local Child's LocalTransform.
    /// @return Child's WorldTransform.
    inline auto compose(const WorldTransform& parent, const LocalTransform& local) -> WorldTransform
    {
        const auto cosR = std::cos(parent.rotation);
        const auto sinR = std::sin(parent.rotation);
        const auto scaledLocal = glm::vec2{local.position.x * parent.scale.x, local.position.y * parent.scale.y};
        const auto rotated = glm::vec2{(scaledLocal.x * cosR) - (scaledLocal.y * sinR), (scaledLocal.x * sinR) + (scaledLocal.y * cosR)};

        return WorldTransform{
            .position = parent.position + rotated,
            .scale = glm::vec2{parent.scale.x * local.scale.x, parent.scale.y * local.scale.y},
            .rotation = parent.rotation + local.rotation,
        };
    }

    /// @brief Make `child` a child of `parent` via flecs ChildOf relationship.
    inline auto setParent(flecs::entity child, flecs::entity parent) -> void
    {
        child.child_of(parent);
    }
}
