module;

#include <awen/flecs.h>

export module awen.widgets;

export import awen.scene;
export import awen.graphics;
export import awen.widgets.texture_cache;

export namespace awn::widgets
{
    /// @brief Shared draw-order bands used by widget and game composition.
    namespace z_layers
    {
        inline constexpr auto game_background = 0;
        inline constexpr auto game_playfield = 100;
        inline constexpr auto game_overlay = 200;
        inline constexpr auto ui_base = 1000;
    }

    /// @brief Global draw order for visual entities.
    struct DrawOrder
    {
        int z{};
    };

    /// @brief Marker component that identifies the root widget window node.
    struct Window
    {
    };

    /// @brief Rendering context consumed by the widgets render system.
    struct RenderContext
    {
        TextureCache* textures{};
        awn::graphics::Color clear_color{};
        bool enabled{};
    };

    /// @brief Renders one frame by building a draw list and submitting it through graphics renderer.
    /// @param world Flecs world with widgets Module imported.
    /// @param textures Texture cache used to resolve sprite textures.
    /// @param clear_color Background clear color.
    inline auto render_frame(flecs::world& world, TextureCache& textures, awn::graphics::Color clear_color) -> void;

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
            world.component<RenderContext>("RenderContext");

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

            world.system<>("RenderFrame")
                .kind(flecs::PostUpdate)
                .each(
                    [&world]()
                    {
                        const auto* context = world.try_get<RenderContext>();

                        if (context == nullptr || !context->enabled || context->textures == nullptr)
                        {
                            return;
                        }

                        render_frame(world, *context->textures, context->clear_color);
                    });
        }
    };

    /// @brief Creates the root widget window entity.
    /// @param world Flecs world that receives the root entity.
    /// @return The created root entity with Window, Transform, and DrawOrder components.
    [[nodiscard]] inline auto create_window(flecs::world& world) -> flecs::entity
    {
        return world.entity().add<Window>().set<awn::scene::Transform>({}).set<DrawOrder>({.z = z_layers::ui_base});
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

    /// @brief Renders one frame by building a draw list and submitting it through graphics renderer.
    /// @param world Flecs world with widgets Module imported.
    /// @param textures Texture cache used to resolve sprite textures.
    /// @param clear_color Background clear color.
    inline auto render_frame(flecs::world& world, TextureCache& textures, awn::graphics::Color clear_color) -> void
    {
        auto draw_list = awn::graphics::DrawList{};
        draw_list.push(awn::graphics::RenderClear{.color = clear_color});
        build_draw_list(world, textures, draw_list);

        awn::graphics::Renderer::begin();
        awn::graphics::Renderer::submit(draw_list);
        awn::graphics::Renderer::end();
    }

    /// @brief Configures world-level render context for the widgets render system.
    /// @param world Flecs world with widgets Module imported.
    /// @param textures Texture cache used during render command generation.
    /// @param clear_color Background clear color.
    inline auto configure_rendering(flecs::world& world, TextureCache& textures, awn::graphics::Color clear_color) -> void
    {
        world.set<RenderContext>({
            .textures = &textures,
            .clear_color = clear_color,
            .enabled = true,
        });
    }
}

export namespace awn::widget
{
    using Window = awn::widgets::Window;
}
