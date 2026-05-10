module;

#include <awen/flecs.h>

export module awen.widgets;

export import awen.scene;
export import awen.graphics;
export import awen.widgets.texture_cache;

export namespace awn::widgets
{
    /// @brief Global draw order for visual entities.
    struct DrawOrder
    {
        int z{};
    };

    /// @brief Marker component that identifies the root widget window node.
    struct Window
    {
    };

    /// @brief Flecs module that provides widget composition and render traversal queries.
    struct Module
    {
        /// @brief Pre-built query that emits all rect entities in ascending DrawOrder.z order.
        flecs::query<const awn::scene::WorldTransform, const awn::graphics::DrawRect, const DrawOrder> rect_query;

        /// @brief Pre-built query that emits all circle entities in ascending DrawOrder.z order.
        flecs::query<const awn::scene::WorldTransform, const awn::graphics::DrawCircle, const DrawOrder> circle_query;

        /// @brief Pre-built query that emits all text entities in ascending DrawOrder.z order.
        flecs::query<const awn::scene::WorldTransform, const awn::graphics::DrawText, const DrawOrder> text_query;

        /// @brief Pre-built query that emits all sprite entities in ascending DrawOrder.z order.
        flecs::query<const awn::scene::WorldTransform, const awn::graphics::DrawSprite, const DrawOrder> sprite_query;

        explicit Module(flecs::world& world)
        {
            world.module<Module>("awn::widgets");

            world.component<DrawOrder>("DrawOrder");
            world.component<Window>("Window");

            const auto ascending_z = [](flecs::entity_t, const DrawOrder* a, flecs::entity_t, const DrawOrder* b) -> int
            { return (a->z > b->z) - (a->z < b->z); };

            rect_query = world.query_builder<const awn::scene::WorldTransform, const awn::graphics::DrawRect, const DrawOrder>()
                             .order_by<DrawOrder>(ascending_z)
                             .build();

            circle_query = world.query_builder<const awn::scene::WorldTransform, const awn::graphics::DrawCircle, const DrawOrder>()
                               .order_by<DrawOrder>(ascending_z)
                               .build();

            text_query = world.query_builder<const awn::scene::WorldTransform, const awn::graphics::DrawText, const DrawOrder>()
                             .order_by<DrawOrder>(ascending_z)
                             .build();

            sprite_query = world.query_builder<const awn::scene::WorldTransform, const awn::graphics::DrawSprite, const DrawOrder>()
                               .order_by<DrawOrder>(ascending_z)
                               .build();
        }
    };

    /// @brief Creates the root widget window entity.
    /// @param world Flecs world that receives the root entity.
    /// @return The created root entity with Window, Transform, and DrawOrder components.
    [[nodiscard]] inline auto create_window(flecs::world& world) -> flecs::entity
    {
        return world.entity().set<Window>({}).set<awn::scene::Transform>({}).set<DrawOrder>({.z = 0});
    }

    /// @brief Appends render commands for all widget-compatible draw components.
    /// @param world Flecs world with widgets Module imported.
    /// @param textures Texture cache used to resolve sprite textures.
    /// @param out Draw list to append to.
    inline auto build_draw_list(flecs::world& world, TextureCache& textures, awn::graphics::DrawList& out) -> void
    {
        const auto& mod = world.get<Module>();

        mod.rect_query.each(
            [&](const awn::scene::WorldTransform& wt, const awn::graphics::DrawRect& rect, const DrawOrder&)
            {
                out.push(awn::graphics::RenderRect{
                    .x = wt.x,
                    .y = wt.y,
                    .width = rect.width,
                    .height = rect.height,
                    .color = rect.color,
                });
            });

        mod.circle_query.each(
            [&](const awn::scene::WorldTransform& wt, const awn::graphics::DrawCircle& circle, const DrawOrder&)
            {
                out.push(awn::graphics::RenderCircle{
                    .x = wt.x,
                    .y = wt.y,
                    .radius = circle.radius,
                    .color = circle.color,
                });
            });

        mod.text_query.each(
            [&](const awn::scene::WorldTransform& wt, const awn::graphics::DrawText& text, const DrawOrder&)
            {
                out.push(awn::graphics::RenderText{
                    .text = text.text,
                    .x = static_cast<int>(wt.x),
                    .y = static_cast<int>(wt.y),
                    .font_size = text.font_size,
                    .color = text.color,
                });
            });

        mod.sprite_query.each(
            [&](const awn::scene::WorldTransform& wt, const awn::graphics::DrawSprite& sprite, const DrawOrder&)
            {
                if (const auto* tex = textures.get(sprite.texture_id); tex != nullptr)
                {
                    out.push(awn::graphics::RenderSprite{
                        .texture =
                            {
                                .id = tex->id,
                                .width = tex->width,
                                .height = tex->height,
                                .mipmaps = tex->mipmaps,
                                .format = tex->format,
                            },
                        .x = wt.x,
                        .y = wt.y,
                        .width = sprite.width,
                        .height = sprite.height,
                        .tint = sprite.tint,
                    });
                }
            });
    }
}

export namespace awn::widget
{
    using Window = awn::widgets::Window;
}
