module;

#include <awen/flecs/Flecs.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <cstdint>

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

    /// @brief World-space 2D transform produced by hierarchy propagation.
    /// @note Stored as a glm::mat3 so SDL_Render* call sites can transform
    ///       points uniformly regardless of how the parent chain composed.
    struct WorldTransform
    {
        glm::mat3 matrix{1.0F};
    };

    /// @brief Render layer. Lower values draw first.
    struct ZOrder
    {
        std::int32_t value{};
    };

    /// @brief Build the affine matrix for a local transform.
    auto toMatrix(const LocalTransform& local) -> glm::mat3
    {
        auto matrix = glm::mat3{1.0F};
        matrix = glm::translate(matrix, local.position);
        matrix = glm::rotate(matrix, local.rotation);
        matrix = glm::scale(matrix, local.scale);

        return matrix;
    }

    /// @brief Compose a parent world matrix with a child local transform.
    auto compose(const WorldTransform& parent, const LocalTransform& local) -> WorldTransform
    {
        return WorldTransform{.matrix = parent.matrix * toMatrix(local)};
    }

    /// @brief Transform a local-space point through a world matrix.
    auto applyWorld(const WorldTransform& world, glm::vec2 local) -> glm::vec2
    {
        const auto homogeneous = world.matrix * glm::vec3{local, 1.0F};

        return glm::vec2{homogeneous.x, homogeneous.y};
    }

    /// @brief Extract the world-space translation from a world matrix.
    auto worldPosition(const WorldTransform& world) -> glm::vec2
    {
        return glm::vec2{world.matrix[2].x, world.matrix[2].y};
    }

    /// @brief Extract a uniform scale factor from a world matrix's X basis.
    auto worldScaleX(const WorldTransform& world) -> float
    {
        return glm::length(glm::vec2{world.matrix[0].x, world.matrix[0].y});
    }

    /// @brief Extract the rotation angle (radians) from a world matrix.
    auto worldRotation(const WorldTransform& world) -> float
    {
        return std::atan2(world.matrix[0].y, world.matrix[0].x);
    }
}
