module;

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <concepts>
#include <memory>
#include <ranges>
#include <sigslot/signal.hpp>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

export module awen.raylib.node;
import awen.core;
import awen.raylib.events;

export namespace awen::raylib
{
    class Node;

    template <typename T>
    concept TypeNode = std::derived_from<T, Node>;

    class Node : public core::Object
    {
    public:
        Node()
        {
            std::ignore = onChildAdd(
                [this](core::Object& child)
                {
                    auto* node = dynamic_cast<Node*>(&child);

                    if (node == nullptr)
                    {
                        return;
                    }

                    node->markDirty();
                    nodes_.emplace_back(node);
                });

            std::ignore = onChildRemove(
                [this](core::Object& child)
                {
                    auto* node = dynamic_cast<Node*>(&child);

                    if (node == nullptr)
                    {
                        return;
                    }

                    std::erase(nodes_, node);
                    node->markDirty();
                });
        }

        ~Node() override = default;

        Node(const Node&) = delete;
        auto operator=(const Node&) -> Node& = delete;

        Node(Node&&) noexcept = delete;
        auto operator=(Node&&) noexcept -> Node& = delete;

        auto setPosition(Vector2 position) noexcept -> void
        {
            position_ = position;
            markDirty();
        }

        [[nodiscard]] auto getPosition() const noexcept -> Vector2
        {
            return position_;
        }

        auto setScale(Vector2 scale) noexcept -> void
        {
            scale_ = scale;
            markDirty();
        }

        [[nodiscard]] auto getScale() const noexcept -> Vector2
        {
            return scale_;
        }

        auto setRotation(float rotation) noexcept -> void
        {
            rotation_ = rotation;
            markDirty();
        }

        [[nodiscard]] auto getRotation() const noexcept -> float
        {
            return rotation_;
        }

        [[nodiscard]] auto addNode(std::unique_ptr<Node> node) -> Node*
        {
            auto* ptr = node.get();
            std::ignore = addChild(std::move(node));
            return ptr;
        }

        template <TypeNode T, typename... Args>
        [[nodiscard]] auto addNode(Args&&... args) -> T*
        {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            auto* ptr = node.get();
            std::ignore = addChild(std::move(node));
            return ptr;
        }

        [[nodiscard]] auto getNodes() const -> std::vector<Node*>
        {
            return nodes_;
        }

        auto events(Event& x) -> void
        {
            for (auto* child : std::views::reverse(nodes_))
            {
                child->events(x);

                if (isHandled(x))
                {
                    return;
                }
            }

            if (!isHandled(x))
            {
                events_(x);
            }
        }

        auto renderPre() -> void
        {
            renderPre_();

            for (const auto& child : nodes_)
            {
                child->renderPre();
            }
        }

        auto render() -> void
        {
            rlPushMatrix();

            const auto matf = MatrixToFloatV(getLocalTransform());
            rlMultMatrixf(static_cast<const float*>(matf.v));

            render_();

            for (const auto& child : nodes_)
            {
                child->render();
            }

            rlPopMatrix();
        }

        auto renderPost() -> void
        {
            renderPost_();

            for (const auto& child : nodes_)
            {
                child->renderPost();
            }
        }

        auto onEvents(auto x) -> sigslot::connection
        {
            return events_.connect(x);
        }

        auto onRenderPre(auto x) -> sigslot::connection
        {
            return renderPre_.connect(x);
        }

        auto onRender(auto x) -> sigslot::connection
        {
            return render_.connect(x);
        }

        auto onRenderPost(auto x) -> sigslot::connection
        {
            return renderPost_.connect(x);
        }

        /// @brief Map a point in world coordinates into this node's local coordinate space.
        /// @param point The point in world coordinates to map.
        /// @return The point expressed in this node's local coordinate space.
        [[nodiscard]] auto mapToNode(Vector2 point) const -> Vector2
        {
            ensureWorldClean();
            return Vector2Transform(point, worldInverse_);
        }

        /// @brief Map a point in this node's local coordinate space into world coordinates.
        /// @param point The point in this node's local coordinate space.
        /// @return The point expressed in world coordinates.
        [[nodiscard]] auto mapToWorld(Vector2 point) const -> Vector2
        {
            ensureWorldClean();
            return Vector2Transform(point, worldTransform_);
        }

        /// @brief Get the node-local transform matrix (T * R * S), lazily recomputed if dirty.
        [[nodiscard]] auto getLocalTransform() const -> const Matrix&
        {
            if (localDirty_)
            {
                const auto t = MatrixTranslate(position_.x, position_.y, 0.0F);
                const auto r = MatrixRotateZ(rotation_ * DEG2RAD);
                const auto s = MatrixScale(scale_.x, scale_.y, 1.0F);

                // Match the render-time order: rlTranslatef, then rlRotatef, then rlScalef.
                localTransform_ = MatrixMultiply(MatrixMultiply(s, r), t);
                localDirty_ = false;
            }

            return localTransform_;
        }

        /// @brief Get the cached world transform matrix, lazily recomputed if dirty.
        [[nodiscard]] auto getWorldTransform() const -> const Matrix&
        {
            ensureWorldClean();
            return worldTransform_;
        }

    private:
        using awen::core::Object::addChild;

        [[nodiscard]] static auto isHandled(const Event& x) -> bool
        {
            return std::visit([](const auto& event) { return event.handled; }, x);
        }

        /// @brief Mark this node and all descendants as having stale world transforms.
        auto markDirty() -> void
        {
            localDirty_ = true;
            worldDirty_ = true;

            // Iteratively walk descendants to invalidate their world caches.
            std::vector<Node*> visitor;
            visitor.reserve(std::size(nodes_));

            for (auto* child : nodes_)
            {
                visitor.emplace_back(child);
            }

            while (!std::empty(visitor))
            {
                auto* node = visitor.back();
                visitor.pop_back();

                if (node->worldDirty_)
                {
                    continue;
                }

                node->worldDirty_ = true;

                for (auto* child : node->nodes_)
                {
                    visitor.emplace_back(child);
                }
            }
        }

        /// @brief Recompute the cached world transform (and its inverse) if stale.
        auto ensureWorldClean() const -> void
        {
            if (!worldDirty_ && !localDirty_)
            {
                return;
            }

            const auto& local = getLocalTransform();
            const auto* parentNode = getParent<Node>();

            if (parentNode != nullptr && parentNode != this)
            {
                worldTransform_ = MatrixMultiply(local, parentNode->getWorldTransform());
            }
            else
            {
                worldTransform_ = local;
            }

            worldInverse_ = MatrixInvert(worldTransform_);
            worldDirty_ = false;
        }

        std::vector<Node*> nodes_;
        Vector2 position_{};
        Vector2 scale_{.x = 1.0F, .y = 1.0F};
        float rotation_{};
        sigslot::signal_st<Event&> events_;
        sigslot::signal_st<> renderPre_;
        sigslot::signal_st<> render_;
        sigslot::signal_st<> renderPost_;

        mutable Matrix localTransform_{};
        mutable Matrix worldTransform_{};
        mutable Matrix worldInverse_{};
        mutable bool localDirty_{true};
        mutable bool worldDirty_{true};
    };
}