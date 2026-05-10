module;

#include <compare>
#include <cstdint>

export module awen.ecs;

// Forward-declare the opaque flecs C world type with external C linkage.
// Not exported — internal bridge type allowing Entity/World to store pointers
// without including <flecs.h> in the module interface.
extern "C"
{
    struct ecs_world_t; // NOLINT(readability-identifier-naming)
}

export namespace awn
{

    /// @brief Lightweight non-owning handle to a flecs entity.
    ///
    /// Default-constructed instances represent a null/invalid entity.
    /// Copyable and cheaply passed by value.
    class Entity
    {
    public:
        Entity() = default;

        /// @brief Returns true if the entity is currently alive in its world.
        [[nodiscard]] auto is_alive() const noexcept -> bool;

        /// @brief Destroys the entity and all its children.
        auto destroy() -> void;

        /// @brief Reparents this entity under the given parent.
        /// @param parent The new parent entity.
        auto set_parent(Entity parent) -> void;

        /// @brief Returns the underlying flecs entity id.
        [[nodiscard]] auto id() const noexcept -> std::uint64_t
        {
            return id_;
        }

        /// @brief Returns a pointer to the world this entity belongs to.
        [[nodiscard]] auto world_ptr() const noexcept -> ecs_world_t*
        {
            return world_;
        }

        auto operator<=>(const Entity&) const noexcept = default;

    private:
        friend class World;

        Entity(std::uint64_t id, ecs_world_t* world) noexcept : id_(id), world_(world)
        {
        }

        std::uint64_t id_{};
        ecs_world_t* world_{};
    };

    /// @brief Owns and manages the flecs ECS world.
    ///
    /// Entities, systems, queries, and observers are created through this class.
    /// Non-copyable — intended to be owned by Engine as a single instance.
    class World
    {
    public:
        World();
        ~World();

        World(const World&) = delete;
        auto operator=(const World&) -> World& = delete;

        /// @brief Creates an unnamed entity in this world.
        /// @return A handle to the new live entity.
        [[nodiscard]] auto create_entity() -> Entity;

        /// @brief Creates a named entity in this world.
        /// @param name Unique name identifying the entity.
        /// @return A handle to the new live entity.
        [[nodiscard]] auto create_entity(const char* name) -> Entity;

        /// @brief Returns whether the given entity handle is currently alive.
        /// @param e The entity to check.
        [[nodiscard]] auto is_alive(Entity e) const noexcept -> bool;

        /// @brief Advances all registered systems by one tick.
        /// @param dt Delta time in seconds. Pass 0 to use wall-clock time.
        /// @return False if the world has been requested to terminate.
        auto progress(float dt = 0.0F) -> bool;

        /// @brief Returns the underlying flecs world pointer for direct flecs API access.
        [[nodiscard]] auto raw() noexcept -> ecs_world_t*
        {
            return world_;
        }

        /// @brief Returns the underlying flecs world pointer for direct read-only access.
        [[nodiscard]] auto raw() const noexcept -> const ecs_world_t*
        {
            return world_;
        }

    private:
        ecs_world_t* world_{};
    };

} // namespace awn
