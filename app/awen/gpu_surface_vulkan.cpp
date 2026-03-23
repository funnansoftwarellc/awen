#include "gpu_surface.h"

#include <SDL3/SDL.h>

// TODO: Vulkan backend for Windows, Linux, and Android.
//
// Requires vcpkg feature: skia[graphite,vulkan]
//
// Implementation outline:
//   1. SDL_Vulkan_GetInstanceExtensions / SDL_Vulkan_CreateSurface for the
//      window surface — both are pure C.
//   2. Create VkInstance, VkPhysicalDevice, VkDevice, VkQueue — all pure C.
//   3. Build skgpu::graphite::VulkanBackendContext and call
//      skgpu::graphite::ContextFactory::MakeVulkan().
//   4. Each frame: vkAcquireNextImageKHR → BackendTextures::MakeVulkan() →
//      SkSurfaces::WrapBackendTexture() → draw → snap → insert → submit →
//      vkQueuePresentKHR.
//
// No platform-specific language extensions are needed — Vulkan is pure C/C++.

namespace awen
{

    auto GpuSurface::create(SDL_Window *window) -> std::unique_ptr<GpuSurface>
    {
        (void)window;
        SDL_Log("Vulkan GpuSurface backend is not yet implemented");
        return nullptr;
    }

} // namespace awen
