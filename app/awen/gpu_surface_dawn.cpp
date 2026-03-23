#include "gpu_surface.h"

#include <SDL3/SDL.h>

// TODO: Dawn/WebGPU backend for Emscripten (WebAssembly).
//
// Requires vcpkg feature: skia[graphite,dawn]  (or link against system Dawn)
//
// Implementation outline:
//   1. Use emscripten_webgpu_get_device() to obtain a wgpu::Device.
//   2. Build skgpu::graphite::DawnBackendContext and call
//      skgpu::graphite::ContextFactory::MakeDawn().
//   3. Obtain the WebGPU swap-chain texture from the canvas element.
//   4. Each frame: wrap the swap-chain texture with
//      BackendTextures::MakeDawn() → SkSurfaces::WrapBackendTexture() →
//      draw → snap → insert → submit.  Presentation is implicit when the
//      browser composites the canvas.
//
// No platform-specific language extensions are needed — Dawn/WebGPU is C/C++.

namespace awen
{

    auto GpuSurface::create(SDL_Window *window) -> std::unique_ptr<GpuSurface>
    {
        (void)window;
        SDL_Log("Dawn/WebGPU GpuSurface backend is not yet implemented");
        return nullptr;
    }

} // namespace awen
