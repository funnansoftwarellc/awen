module;

#include <awen/flecs.h>

#include <chrono>
#include <cstdlib>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

export module awen.core.ecs.engine;
export import awen.core.ecs.phases;

export namespace awn::core
{
    struct RunState
    {
        enum class Type : std::uint8_t
        {
            running,
            stopping,
        };

        Type type{Type::running};
    };

    /// @brief Owns an ECS world and runs update/render pipelines.
    class Engine
    {
    public:
        template <typename T>
        auto load() -> void
        {
            world_.import<T>();
        }

        auto stop() -> void
        {
            world_.set<RunState>({.type = RunState::Type::stopping});
        }

        auto world() -> flecs::world&
        {
            return world_;
        }

        auto run() -> int
        {
            start_ = std::chrono::steady_clock::now();

            while (world_.ensure<RunState>().type == RunState::Type::running)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto dt = now - start_;
                const auto dt_s = std::chrono::duration<float>(dt).count();

                start_ = now;
                accumulated_dt_ += dt;

                world_.run_pipeline(pipeline_event_, dt_s);
                world_.run_pipeline(pipeline_update_, dt_s);

                auto count = 0;

                while (accumulated_dt_ >= fixed_timestep_ && count < count_limit_)
                {
                    world_.run_pipeline(pipeline_update_fixed_, fixed_timestep_s_.count());
                    accumulated_dt_ -= fixed_timestep_;
                    ++count;
                }

                world_.run_pipeline(pipeline_render_, dt_s);
            }

            return EXIT_SUCCESS;
        }

    private:
        flecs::world world_;
        flecs::entity pipeline_event_{world_.pipeline().with(flecs::System).with<ecs::phases::OnEvent>().build()};
        flecs::entity pipeline_update_{world_.pipeline().with(flecs::System).with<ecs::phases::OnUpdate>().build()};
        flecs::entity pipeline_update_fixed_{world_.pipeline().with(flecs::System).with<ecs::phases::OnUpdateFixed>().build()};
        flecs::entity pipeline_render_{world_.pipeline().with(flecs::System).with<ecs::phases::OnRender>().build()};

        std::chrono::steady_clock::time_point start_{};
        std::chrono::steady_clock::duration accumulated_dt_{};
        static constexpr std::chrono::milliseconds fixed_timestep_{10};
        static constexpr std::chrono::duration<float> fixed_timestep_s_{std::chrono::duration<float>(fixed_timestep_).count()};
        static constexpr auto count_limit_{5};

        RunState run_state_;
    };
}