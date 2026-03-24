#include "gpu_surface.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteUtils.h"

namespace awen
{
    namespace
    {

        class MetalGpuSurface final : public GpuSurface
        {
        public:
            ~MetalGpuSurface() override
            {
                recorder_.reset();
                context_.reset();
                queue_ = nil;
                device_ = nil;
                if (metalView_ != nullptr)
                {
                    SDL_Metal_DestroyView(metalView_);
                }
            }

            MetalGpuSurface(const MetalGpuSurface &) = delete;
            auto operator=(const MetalGpuSurface &) -> MetalGpuSurface & = delete;
            MetalGpuSurface(MetalGpuSurface &&) = delete;
            auto operator=(MetalGpuSurface &&) -> MetalGpuSurface & = delete;

            static auto make(SDL_Window *window) -> std::unique_ptr<MetalGpuSurface>
            {
                auto self = std::unique_ptr<MetalGpuSurface>(new MetalGpuSurface());

                self->metalView_ = SDL_Metal_CreateView(window);
                if (self->metalView_ == nullptr)
                {
                    SDL_Log("SDL_Metal_CreateView failed: %s", SDL_GetError()); // NOLINT(cppcoreguidelines-pro-type-vararg)
                    return nullptr;
                }

                self->layer_ = (__bridge CAMetalLayer *)SDL_Metal_GetLayer(self->metalView_);
                self->device_ = MTLCreateSystemDefaultDevice();
                self->queue_ = [self->device_ newCommandQueue];

                self->layer_.device = self->device_;
                self->layer_.pixelFormat = MTLPixelFormatBGRA8Unorm;
                self->layer_.framebufferOnly = NO;

                skgpu::graphite::MtlBackendContext backend_ctx;
                backend_ctx.fDevice.retain((__bridge CFTypeRef)self->device_);
                backend_ctx.fQueue.retain((__bridge CFTypeRef)self->queue_);

                const skgpu::graphite::ContextOptions opts;
                self->context_ = skgpu::graphite::ContextFactory::MakeMetal(backend_ctx, opts);
                if (!self->context_)
                {
                    SDL_Log("Failed to create Skia Graphite Metal context"); // NOLINT(cppcoreguidelines-pro-type-vararg)
                    return nullptr;
                }

                self->recorder_ = self->context_->makeRecorder();
                if (!self->recorder_)
                {
                    SDL_Log("Failed to create Skia Graphite recorder"); // NOLINT(cppcoreguidelines-pro-type-vararg)
                    return nullptr;
                }

                return self;
            }

            auto recorder() -> skgpu::graphite::Recorder * override { return recorder_.get(); }

            auto begin_frame() -> sk_sp<SkSurface> override
            {
                drawable_ = [layer_ nextDrawable];
                if (drawable_ == nil)
                {
                    return nullptr;
                }

                const CGSize size = layer_.drawableSize;
                const SkISize dims = {.fWidth = static_cast<int>(size.width), .fHeight = static_cast<int>(size.height)};

                backendTex_ = skgpu::graphite::BackendTextures::MakeMetal(
                    dims, (__bridge CFTypeRef)drawable_.texture);

                return SkSurfaces::WrapBackendTexture(
                    recorder_.get(), backendTex_, nullptr, nullptr);
            }

            void end_frame() override
            {
                @autoreleasepool
                {
                    auto recording = recorder_->snap();

                    skgpu::graphite::InsertRecordingInfo info;
                    info.fRecording = recording.get();
                    context_->insertRecording(info);
                    context_->submit();

                    const id<MTLCommandBuffer> cmd_buf = [queue_ commandBuffer];
                    [cmd_buf presentDrawable:drawable_];
                    [cmd_buf commit];

                    drawable_ = nil;
                }
            }

        private:
            MetalGpuSurface() = default;

            SDL_MetalView metalView_ = nullptr;
            id<MTLDevice> device_ = nil;
            id<MTLCommandQueue> queue_ = nil;
            CAMetalLayer *layer_ = nil;

            std::unique_ptr<skgpu::graphite::Context> context_;
            std::unique_ptr<skgpu::graphite::Recorder> recorder_;

            // Per-frame state
            id<CAMetalDrawable> drawable_ = nil;
            skgpu::graphite::BackendTexture backendTex_;
        };

    } // namespace

    auto GpuSurface::create(SDL_Window *window) -> std::unique_ptr<GpuSurface>
    {
        return MetalGpuSurface::make(window);
    }

} // namespace awen
