module;

#include <raylib.h>
#include <typeinfo>

export module awen.raylib.camera;
import awen.raylib.node;

export namespace awen::raylib
{
    class Camera : public Node
    {
    public:
        Camera()
        {
            onRenderPre([this] { BeginMode2D(camera_); });
            onRenderPost([] { EndMode2D(); });
        }

        auto setOffset(const Vector2& offset) noexcept -> void
        {
            camera_.offset = offset;
        }

        [[nodiscard]] auto getOffset() const noexcept -> Vector2
        {
            return camera_.offset;
        }

        auto setTarget(const Vector2& target) noexcept -> void
        {
            camera_.target = target;
        }

        [[nodiscard]] auto getTarget() const noexcept -> Vector2
        {
            return camera_.target;
        }

        auto setCameraRotation(float rotation) noexcept -> void
        {
            camera_.rotation = rotation;
        }

        [[nodiscard]] auto getCameraRotation() const noexcept -> float
        {
            return camera_.rotation;
        }

        auto setZoom(float zoom) noexcept -> void
        {
            camera_.zoom = zoom;
        }

        [[nodiscard]] auto getZoom() const noexcept -> float
        {
            return camera_.zoom;
        }

    private:
        Camera2D camera_{.offset = {}, .target = {}, .rotation = 0.0F, .zoom = 1.0F};
    };
}