#pragma once

#include <memory>

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Recorder.h"

struct SDL_Window;

namespace awen
{

    class GpuSurface
    {
    public:
        virtual ~GpuSurface() = default;

        GpuSurface(const GpuSurface &) = delete;
        auto operator=(const GpuSurface &) -> GpuSurface & = delete;
        GpuSurface(GpuSurface &&) = delete;
        auto operator=(GpuSurface &&) -> GpuSurface & = delete;

        static auto create(SDL_Window *window) -> std::unique_ptr<GpuSurface>;

        virtual auto recorder() -> skgpu::graphite::Recorder * = 0;

        // Acquire the next drawable and return an SkSurface wrapping it.
        // Returns nullptr if no drawable is available (e.g. window minimized).
        virtual auto begin_frame() -> sk_sp<SkSurface> = 0;

        // Snap the recording, submit GPU work, and present the drawable.
        virtual void end_frame() = 0;

    protected:
        GpuSurface() = default;
    };

} // namespace awen
