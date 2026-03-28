module;

#include <flecs.h>

module awen.ecs;

namespace awn
{

    auto Entity::is_alive() const noexcept -> bool
    {
        if (id_ == 0 || world_ == nullptr)
        {
            return false;
        }

        return ecs_is_alive(world_, id_);
    }

    auto Entity::destroy() -> void
    {
        ecs_delete(world_, id_);
    }

    auto Entity::set_parent(Entity parent) -> void
    {
        ecs_add_pair(world_, id_, EcsChildOf, parent.id_);
    }

    World::World() : world_(ecs_init())
    {
    }

    World::~World()
    {
        if (world_ != nullptr)
        {
            ecs_fini(world_);
        }
    }

    auto World::create_entity() -> Entity
    {
        return Entity{ecs_new(world_), world_};
    }

    auto World::create_entity(const char* name) -> Entity
    {
        const auto e = ecs_new(world_);
        ecs_set_name(world_, e, name);

        return Entity{e, world_};
    }

    auto World::is_alive(Entity e) const noexcept -> bool
    {
        return ecs_is_alive(world_, e.id());
    }

    auto World::progress(float dt) -> bool
    {
        return ecs_progress(world_, dt);
    }

} // namespace awn
