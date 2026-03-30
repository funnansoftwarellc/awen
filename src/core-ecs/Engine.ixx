module;

#include <awen/flecs.h>

#include <cstdlib>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

export module awen.core.ecs.engine;

export namespace awn::core
{
    /// @brief Owns an ECS world and runs update/render pipelines.
    class Engine
    {
    public:
        using hook_t = std::function<void(float)>;

        explicit Engine(std::unique_ptr<flecs::world> world) : world_{std::move(world)}
        {
        }

        auto on_pre_update(hook_t callback) -> void
        {
            pre_update_hooks_.push_back(std::move(callback));
        }

        auto on_post_update(hook_t callback) -> void
        {
            post_update_hooks_.push_back(std::move(callback));
        }

        auto set_update_pipeline(flecs::entity pipeline) -> void
        {
            update_pipeline_ = pipeline;
        }

        auto set_render_pipeline(flecs::entity pipeline) -> void
        {
            render_pipeline_ = pipeline;
        }

        auto stop() -> void
        {
            running_ = false;
        }

        auto run() -> int
        {
            while (running_)
            {
                constexpr auto dt = 0.1F;

                for (const auto& callback : pre_update_hooks_)
                {
                    callback(dt);
                }

                run_pipeline(update_pipeline_, dt);

                for (const auto& callback : post_update_hooks_)
                {
                    callback(dt);
                }

                run_pipeline(render_pipeline_, dt);
            }

            return EXIT_SUCCESS;
        }

    private:
        auto run_pipeline(flecs::entity pipeline, float dt) -> void
        {
            if (pipeline.is_valid())
            {
                world_->set_pipeline(pipeline);
            }

            world_->progress(dt);
        }

        std::unique_ptr<flecs::world> world_;
        flecs::entity update_pipeline_{};
        flecs::entity render_pipeline_{};
        std::vector<hook_t> pre_update_hooks_{};
        std::vector<hook_t> post_update_hooks_{};
        bool running_{true};
    };
}