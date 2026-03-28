module;
#include <compare>
#include <flecs.h>

export module awen.ecs;

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
        [[nodiscard]] auto is_alive() const noexcept -> bool
        {
            return entity_.is_alive();
        }

        /// @brief Destroys the entity and all its children.
        auto destroy() -> void
        {
            entity_.destruct();
        }

        /// @brief Reparents this entity under the given parent.
        /// @param parent The new parent entity.
        auto set_parent(Entity parent) -> void
        {
            entity_.child_of(parent.entity_);
        }

        /// @brief Returns the underlying flecs entity for direct flecs API access.
        [[nodiscard]] auto raw() const noexcept -> flecs::entity
        {
            return entity_;
        }

        [[nodiscard]] auto operator==(const Entity& rhs) const noexcept -> bool
        {
            return entity_.id() == rhs.entity_.id();
        }

        [[nodiscard]] auto operator<=>(const Entity& rhs) const noexcept -> std::strong_ordering
        {
            return entity_.id() <=> rhs.entity_.id();
        }

    private:
        friend class World;

        explicit Entity(flecs::entity e) noexcept : entity_(e)
        {
        }

        flecs::entity entity_{};
    };

    /// @brief Owns and manages the flecs ECS world.
    ///
    /// Entities, systems, queries, and observers are created through this class.
    /// Non-copyable — intended to be owned by Engine as a single instance.
    class World
    {
    public:
        World() = default;
        ~World() = default;

        World(const World&) = delete;
        auto operator=(const World&) -> World& = delete;

        /// @brief Creates an unnamed entity in this world.
        /// @return A handle to the new live entity.
        [[nodiscard]] auto create_entity() -> Entity
        {
            return Entity{world_.entity()};
        }

        /// @brief Creates a named entity in this world.
        /// @param name Unique name identifying the entity.
        /// @return A handle to the new live entity.
        [[nodiscard]] auto create_entity(const char* name) -> Entity
        {
            return Entity{world_.entity(name)};
        }

        /// @brief Returns whether the given entity handle is currently alive.
        /// @param e The entity to check.
        [[nodiscard]] auto is_alive(Entity e) const noexcept -> bool
        {
            return e.is_alive();
        }

        /// @brief Advances all registered systems by one tick.
        /// @param dt Delta time in seconds. Pass 0 to use wall-clock time.
        /// @return False if the world has been requested to terminate.
        auto progress(float dt = 0.0F) -> bool
        {
            return world_.progress(dt);
        }

        /// @brief Returns the underlying flecs world for direct flecs API access.
        [[nodiscard]] auto raw() noexcept -> flecs::world&
        {
            return world_;
        }

        /// @brief Returns the underlying flecs world for direct read-only access.
        [[nodiscard]] auto raw() const noexcept -> const flecs::world&
        {
            return world_;
        }

    private:
        flecs::world world_{};
    };

} // namespace awn
