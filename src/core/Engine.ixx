module;

#include <awen/flecs/Flecs.hpp>
#include <chrono>

export module awen.core.engine;
import awen.core.object;

namespace awen::core
{
    export class Engine : public Object
    {
    public:
        Engine()
        {
            if (singleton != nullptr)
            {
                throw std::runtime_error("Engine instance already exists");
            }

            singleton = this;
        }

        ~Engine() override
        {
            singleton = nullptr;
        }

        Engine(const Engine&) = delete;
        auto operator=(const Engine&) -> Engine& = delete;
        Engine(Engine&&) = delete;
        auto operator=(Engine&&) -> Engine& = delete;

        static auto instance() -> Engine*
        {
            return singleton;
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

                onEvent_.emit();
                onUpdate_.emit(delta);

                // Process fixed updates if enough time has accumulated.
                // To prevent spiral of death, we limit the number of fixed updates per frame.
                auto fixedUpdateCount = 0;

                while (accumulated_ >= fixedUpdateInterval_ && fixedUpdateCount < fixedUpdateLimit_)
                {
                    accumulated_ -= fixedUpdateInterval_;
                    ++fixedUpdateCount;
                    onFixedUpdate_.emit(fixedUpdateInterval_);
                }

                onPreRender_.emit();
                onRender_.emit();
                onPostRender_.emit();
            }

            return EXIT_SUCCESS;
        }

        auto stop() -> void
        {
            running_ = false;
        }

        [[nodiscard]] auto fixedUpdateLimit() const -> int
        {
            return fixedUpdateLimit_;
        }

        [[nodiscard]] auto onEvent() -> Signal<void()>&
        {
            return onEvent_;
        }

        [[nodiscard]] auto onUpdate() -> Signal<void(std::chrono::duration<float>)>&
        {
            return onUpdate_;
        }

        [[nodiscard]] auto onFixedUpdate() -> Signal<void(std::chrono::duration<float>)>&
        {
            return onFixedUpdate_;
        }

        [[nodiscard]] auto onPreRender() -> Signal<void()>&
        {
            return onPreRender_;
        }

        [[nodiscard]] auto onRender() -> Signal<void()>&
        {
            return onRender_;
        }

        [[nodiscard]] auto onPostRender() -> Signal<void()>&
        {
            return onPostRender_;
        }

        [[nodiscard]] auto world() -> flecs::world&
        {
            return world_;
        }

    private:
        flecs::world world_;
        std::chrono::steady_clock::time_point start_;
        std::chrono::steady_clock::duration accumulated_{};
        static constexpr auto FixedUpdateMilliseconds = 10;
        static constexpr auto FixedUpdateStepLimit = 5;
        const std::chrono::milliseconds fixedUpdateInterval_{FixedUpdateMilliseconds};
        const int fixedUpdateLimit_{FixedUpdateStepLimit};
        bool running_ = false;
        Signal<void()> onEvent_;
        Signal<void(std::chrono::duration<float> dt)> onUpdate_;
        Signal<void(std::chrono::duration<float> dt)> onFixedUpdate_;
        Signal<void()> onPreRender_;
        Signal<void()> onRender_;
        Signal<void()> onPostRender_;

        static inline Engine* singleton = nullptr;
    };
}