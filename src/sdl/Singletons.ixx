module;

#include <cstdint>

export module awen.sdl.singletons;

export namespace awen::sdl
{
    /// @brief Per-frame timing data populated by the timing system on OnEvent.
    struct FrameTiming
    {
        /// @brief Variable frame delta in seconds.
        float delta{};

        /// @brief Total elapsed seconds since the application started.
        float elapsed{};

        /// @brief Monotonic frame counter (incremented on OnPostRender).
        std::uint64_t frameCount{};

        /// @brief Smoothed frames-per-second estimate.
        float fps{};
    };

    /// @brief Frame-rate / fixed-step configuration.
    struct FrameRate
    {
        /// @brief Target frames per second (0 = uncapped).
        float targetFps{60.0F};

        /// @brief Target fixed-update frequency in Hz.
        float fixedHz{60.0F};

        /// @brief Maximum fixed-update steps that may run in a single frame
        ///        (prevents the spiral-of-death after long stalls).
        int maxFixedStepsPerFrame{5};
    };

    /// @brief Application-level run state.
    struct AppState
    {
        /// @brief When false, the host loop should stop calling `world.progress()`.
        bool running{true};

        /// @brief Exit code reported back to the host.
        int exitCode{};
    };

    /// @brief One-shot SDL initialization tracking.
    /// @note Set automatically by the awen.sdl Module on import.
    struct InitFlags
    {
        bool sdlInitialized{};
        bool ttfInitialized{};
    };
}
