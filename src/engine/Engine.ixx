module;

#include <functional>
#include <utility>
#include <vector>

#include <awen/flecs.h>

export module awen.core.engine;

export namespace awn::core
{
    /// @brief Owns the frame loop and executes world pipelines and hooks.
    class Engine
    {
    public:
        using hook_t = std::function<void(float)>;

        /// @brief Binds the engine to an existing ECS world.
        /// @param world ECS world that already imported all required modules.
        explicit Engine(flecs::world& world) : world_{world}
        {
        }

        ~Engine() = default;

        Engine(const Engine&) = delete;
        auto operator=(const Engine&) -> Engine& = delete;
        Engine(Engine&&) = delete;
        auto operator=(Engine&&) -> Engine& = delete;

        /// @brief Returns the ECS world reference provided at construction.
        [[nodiscard]] auto raw_world() noexcept -> flecs::world&
        {
            return world_;
        }

        /// @brief Sets the update pipeline used for the simulation phase.
        /// @param pipeline Pipeline entity to run for update.
        auto set_update_pipeline(flecs::entity pipeline) -> void
        {
            update_pipeline_ = pipeline;
        }

        /// @brief Sets the render pipeline used for the render phase.
        /// @param pipeline Pipeline entity to run for render.
        auto set_render_pipeline(flecs::entity pipeline) -> void
        {
            render_pipeline_ = pipeline;
        }

        /// @brief Registers a callback executed before update phase.
        auto on_pre_update(hook_t hook) -> void
        {
            pre_update_hooks_.push_back(std::move(hook));
        }

        /// @brief Registers a callback executed after update phase.
        auto on_post_update(hook_t hook) -> void
        {
            post_update_hooks_.push_back(std::move(hook));
        }

        /// @brief Registers a callback executed before render phase.
        auto on_pre_render(hook_t hook) -> void
        {
            pre_render_hooks_.push_back(std::move(hook));
        }

        /// @brief Registers a callback executed after render phase.
        auto on_post_render(hook_t hook) -> void
        {
            post_render_hooks_.push_back(std::move(hook));
        }

        /// @brief Runs one frame tick using configured hooks and pipelines.
        /// @param dt Delta time in seconds.
        auto tick(float dt) -> void
        {
            run_hooks(pre_update_hooks_, dt);
            run_pipeline(update_pipeline_, dt);
            run_hooks(post_update_hooks_, dt);

            run_hooks(pre_render_hooks_, dt);
            run_pipeline(render_pipeline_, dt);
            run_hooks(post_render_hooks_, dt);
        }

        /// @brief Runs the main loop while @p should_continue returns true.
        /// @param should_continue Callback that controls loop lifetime.
        /// @param get_delta_time Callback that provides the frame delta time in seconds.
        template <typename ContinueFn, typename DeltaFn>
        auto run(const ContinueFn& should_continue, const DeltaFn& get_delta_time) -> void
        {
            while (should_continue())
            {
                tick(get_delta_time());
            }
        }

    private:
        static auto run_hooks(const std::vector<hook_t>& hooks, float dt) -> void
        {
            for (const auto& hook : hooks)
            {
                hook(dt);
            }
        }

        auto run_pipeline(flecs::entity pipeline, float dt) -> void
        {
            if (pipeline.is_valid())
            {
                world_.set_pipeline(pipeline);
            }

            world_.progress(dt);
        }

        flecs::world& world_;
        flecs::entity update_pipeline_{};
        flecs::entity render_pipeline_{};
        std::vector<hook_t> pre_update_hooks_{};
        std::vector<hook_t> post_update_hooks_{};
        std::vector<hook_t> pre_render_hooks_{};
        std::vector<hook_t> post_render_hooks_{};
    };
}
