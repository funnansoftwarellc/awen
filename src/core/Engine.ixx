module;

#include <Flecs.hpp>
#include <chrono>

export module awen.core.engine;
import awen.core.object;

namespace awn::core
{
    export class Engine : public Object
    {
    public:
        Engine()
        {
            if (singleton_ != nullptr)
            {
                throw std::runtime_error("Engine instance already exists");
            }

            singleton_ = this;
        }

        ~Engine()
        {
            singleton_ = nullptr;
        }

        static Engine* instance()
        {
            return singleton_;
        }

        auto run() -> int
        {
            running_ = true;
            start_ = std::chrono::steady_clock::now();

            startup();

            while (running_)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto delta = now - start_;
                accumulated_ += delta;
                start_ = now;

                on_event.emit();
                on_update.emit(delta);

                // Process fixed updates if enough time has accumulated.
                // To prevent spiral of death, we limit the number of fixed updates per frame.
                auto fixed_update_count = 0;

                while (accumulated_ >= fixed_update_interval_ && fixed_update_count < fixed_update_limit_)
                {
                    accumulated_ -= fixed_update_interval_;
                    ++fixed_update_count;
                    on_fixed_update.emit(fixed_update_interval_);
                }

                on_pre_render.emit();
                on_render.emit();
                on_post_render.emit();
            }

            return EXIT_SUCCESS;
        }

        auto stop() -> void
        {
            running_ = false;
        }

        [[nodiscard]] auto fixed_update_limit() const -> int
        {
            return fixed_update_limit_;
        }

        Signal<void()> on_event;
        Signal<void(std::chrono::duration<float> dt)> on_update;
        Signal<void(std::chrono::duration<float> dt)> on_fixed_update;
        Signal<void()> on_pre_render;
        Signal<void()> on_render;
        Signal<void()> on_post_render;

        [[nodiscard]] auto world() -> flecs::world&
        {
            return world_;
        }

    private:
        flecs::world world_;
        std::chrono::steady_clock::time_point start_;
        std::chrono::steady_clock::duration accumulated_{};
        const std::chrono::milliseconds fixed_update_interval_{10};
        const int fixed_update_limit_{5};
        bool running_ = false;

        static inline Engine* singleton_ = nullptr;
    };
}