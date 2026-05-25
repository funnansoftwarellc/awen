module;

#include <chrono>
#include <cstdlib>

export module awen.core.engine;
import awen.core.object;

export namespace awen::core
{
    class Engine : public Object
    {
    public:
        static constexpr auto DefaultUpdateFixedLimit{5};
        static constexpr auto DefaultUpdateFixedInterval{std::chrono::milliseconds(10)};

        Engine() = default;
        ~Engine() override = default;

        Engine(const Engine&) = delete;
        auto operator=(const Engine&) -> Engine& = delete;

        Engine(Engine&&) noexcept = delete;
        auto operator=(Engine&&) noexcept -> Engine& = delete;

        /// @brief Sets the limit of fixed updates that can be performed in a single run loop iteration. This is used to prevent the "spiral of death"
        /// problem, where the engine gets stuck in an infinite loop of fixed updates if the update fixed interval is too small or if the system is
        /// too slow to keep up with the updates. By setting a limit, the engine will skip any remaining fixed updates if the limit is reached,
        /// allowing the main update loop to continue and preventing the engine from getting stuck.
        /// @param x The maximum number of fixed updates that can be performed in a single run loop iteration.
        auto setUpdateFixedLimit(int x) noexcept -> void
        {
            updateFixedLimit_ = x;
        }

        /// @brief Gets the limit of fixed updates that can be performed in a single run loop iteration.
        /// @return The maximum number of fixed updates that can be performed in a single run loop iteration.
        [[nodiscard]] auto getUpdateFixedLimit() const noexcept -> int
        {
            return updateFixedLimit_;
        }

        /// @brief Sets the interval at which fixed updates are performed. The engine will perform a fixed update every time the accumulated time
        /// since the last fixed update exceeds this interval. This allows for a consistent update rate for physics and other time-sensitive systems,
        /// regardless of the performance of the main update loop. By adjusting this interval, developers can balance the need for accurate physics
        /// updates with the performance constraints of their application.
        /// @param x The interval at which fixed updates are performed.
        auto setUpdateFixedInterval(std::chrono::steady_clock::duration x) noexcept -> void
        {
            updateFixedInterval_ = x;
        }

        /// @brief Gets the interval at which fixed updates are performed.
        /// @return The interval at which fixed updates are performed.
        [[nodiscard]] auto getUpdateFixedInterval() const noexcept -> std::chrono::steady_clock::duration
        {
            return updateFixedInterval_;
        }

        /// @brief Runs the engine's main loop, which continuously updates the engine and performs fixed updates at the specified intervals. The main
        /// loop will continue running until the engine is stopped, either by calling a stop function (not implemented in this snippet) or by closing
        /// the application. During each iteration of the main loop, the engine calculates the elapsed time since the last update, updates the engine
        /// with that elapsed time, and performs fixed updates as needed based on the accumulated time and the specified fixed update interval. This
        /// function is the entry point for the engine's execution and is responsible for managing the timing and execution of updates.
        auto run() -> int
        {
            start_ = std::chrono::steady_clock::now();
            running_ = true;

            while (running_)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto elapsed = now - start_;
                start_ = now;
                accumulate_ += elapsed;

                updatePre();
                update(elapsed);

                auto count = 0;

                while (accumulate_ >= updateFixedInterval_ && count < updateFixedLimit_)
                {
                    updateFixed(updateFixedInterval_);
                    accumulate_ -= updateFixedInterval_;
                    ++count;
                }

                updatePost();
            }

            return EXIT_SUCCESS;
        };

        auto stop() noexcept -> void
        {
            running_ = false;
        }

    private:
        std::chrono::steady_clock::time_point start_;
        std::chrono::steady_clock::duration accumulate_{};
        std::chrono::steady_clock::duration updateFixedInterval_{DefaultUpdateFixedInterval};
        int updateFixedLimit_{DefaultUpdateFixedLimit};
        bool running_{};
    };
}
