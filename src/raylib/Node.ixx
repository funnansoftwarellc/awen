module;

#include <raylib.h>
#include <rlgl.h>
#include <concepts>
#include <sigslot/signal.hpp>
#include <typeinfo>
#include <vector>

export module awen.raylib.node;
import awen.core;

export namespace awen::raylib
{
    class Node;

    template <typename T>
    concept TypeNode = std::derived_from<T, Node>;

    class Node : public core::Object
    {
    public:
        Node() = default;
        ~Node() override = default;

        Node(const Node&) = delete;
        auto operator=(const Node&) -> Node& = delete;

        Node(Node&&) noexcept = delete;
        auto operator=(Node&&) noexcept -> Node& = delete;

        auto setPosition(Vector2 position) noexcept -> void
        {
            position_ = position;
        }

        [[nodiscard]] auto getPosition() const noexcept -> Vector2
        {
            return position_;
        }

        auto setScale(Vector2 scale) noexcept -> void
        {
            scale_ = scale;
        }

        [[nodiscard]] auto getScale() const noexcept -> Vector2
        {
            return scale_;
        }

        auto setRotation(float rotation) noexcept -> void
        {
            rotation_ = rotation;
        }

        [[nodiscard]] auto getRotation() const noexcept -> float
        {
            return rotation_;
        }

        [[nodiscard]] auto addNode(std::unique_ptr<Node> node) -> Node*
        {
            if (node == nullptr)
            {
                return nullptr;
            }

            auto* nodePtr = node.get();
            addChild(std::move(node));
            nodes_.emplace_back(nodePtr);
            return nodePtr;
        }

        template <TypeNode T, typename... Args>
        [[nodiscard]] auto addNode(Args&&... args) -> T*
        {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            auto* nodePtr = node.get();
            addChild(std::move(node));
            nodes_.emplace_back(nodePtr);
            return nodePtr;
        }

        [[nodiscard]] auto getNodes() const -> std::vector<Node*>
        {
            return nodes_;
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
            rlTranslatef(position_.x, position_.y, 0.0F);
            rlRotatef(rotation_, 0.0F, 0.0F, 1.0F);
            rlScalef(scale_.x, scale_.y, 1.0F);

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

        [[nodiscard]] auto getEngine() noexcept -> core::Engine*
        {
            if (engine_ != nullptr)
            {
                return engine_;
            }

            auto* parent = getParent();

            while (parent != nullptr)
            {
                if (auto* engine = dynamic_cast<core::Engine*>(parent))
                {
                    engine_ = engine;
                    break;
                }

                parent = parent->getParent();
            }

            return engine_;
        }

    private:
        using awen::core::Object::addChild;
        std::vector<Node*> nodes_;
        Vector2 position_{};
        Vector2 scale_{.x = 1.0F, .y = 1.0F};
        float rotation_{};
        sigslot::signal_st<> renderPre_;
        sigslot::signal_st<> render_;
        sigslot::signal_st<> renderPost_;
        awen::core::Engine* engine_{};
    };
}